#include "../../include/view/PersonalView.hpp"
#include "ui_PersonalView.h" 

#include <QDateTime>
#include <QHeaderView>      // Cần thiết để chỉnh độ rộng cột
#include <QTableWidgetItem> // Cần thiết để thêm dòng vào bảng
#include <QDebug>

PersonalView::PersonalView(controller::MusicPlayer* player,StatisticsManager* stats, QWidget *parent) 
    : QWidget(parent)
    , ui(new Ui::PersonalView)
    , m_player(player) 
    , m_stats(stats)
{
    ui->setupUi(this); // Load giao diện từ file .ui XML
    
    // --- CẤU HÌNH BẢNG LỊCH SỬ (TABLE WIDGET) ---
    // Đảm bảo tableHistory đã được tạo trong file .ui (kéo thả QTableWidget vào)
    // Thiết lập số cột là 3 (Title, Artist, Time)
    if (ui->tableHistory->columnCount() < 3) {
        ui->tableHistory->setColumnCount(3);
        ui->tableHistory->setHorizontalHeaderLabels({"Title", "Artist", "Time"});
    }
    // Tinh chỉnh độ rộng cột
    // Cột 0 (Title): Tự động giãn
    ui->tableHistory->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    // Cột 1 (Artist): Co giãn theo nội dung
    ui->tableHistory->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    // Cột 2 (Time): Cố định kích thước
    ui->tableHistory->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableHistory->setColumnWidth(2, 120);

    // Ẩn cột số thứ tự bên trái (Vertical Header) cho đẹp
    ui->tableHistory->verticalHeader()->setVisible(false);

    // Cấu hình selection: Chọn cả dòng, không chọn từng ô
    ui->tableHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
    // ui->tableHistory->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableHistory->setEditTriggers(QAbstractItemView::NoEditTriggers); // Không cho sửa

    // Load dữ liệu lần đầu
    refreshData(); 
}

PersonalView::~PersonalView() {
    delete ui;
}

void PersonalView::refreshData() {
    // Nếu chưa có player thì không làm gì cả để tránh crash
    if (!m_player||!m_stats) return;

    updateStatistics();
    updateHistoryTable();
    updateTopLists();
}

// --- PHẦN 1: CẬP NHẬT 3 Ô THỐNG KÊ TRÊN CÙNG ---
void PersonalView::updateStatistics() {
    // 1. Tổng số bài đã nghe (Lấy dữ liệu thật từ History)
    // Lưu ý: getHistory() trả về PlaybackHistory object, gọi tiếp .getHistory() để lấy vector
    auto totalSongs = m_stats->getRecentlyPlayed(1000); 
    ui->lblTotalSongs->setText(QString::number(totalSongs.size()));

    // 2. Số phút nghe HÔM NAY (Req 2)
    // Mode 0 = Theo ngày
    int minsToday = m_stats->getListeningMinutes(QDate::currentDate(), 0); 
    ui->lblStatToday->setText(QString::number(minsToday) + " mins");

    // 3. Số phút nghe THÁNG NAY (Req 2)
    // Mode 1 = Theo tháng
    int minsMonth = m_stats->getListeningMinutes(QDate::currentDate(), 1);
    // Logic hiển thị đẹp: Nếu trên 60 phút thì đổi sang giờ
    if (minsMonth > 60) {
        double hours = minsMonth / 60.0;
        ui->lblStatMonth->setText(QString::number(hours, 'f', 1) + " hrs");
    } else {
        ui->lblStatMonth->setText(QString::number(minsMonth) + " mins");
    }
}

// --- PHẦN 2: CẬP NHẬT BẢNG LỊCH SỬ ---
void PersonalView::updateHistoryTable() {
    // Xóa dữ liệu cũ
    ui->tableHistory->setRowCount(0);
     // Lấy 50 bài nghe gần nhất (Không quan tâm tuần/tháng, chỉ cần mới nhất)
    std::vector<PlayRecord> recentRecords = m_stats->getRecentlyPlayed(50);
    
    // Duyệt ngược để bài mới nghe nhất nằm trên cùng (dùng rbegin/rend)
    for (const auto& rec : recentRecords) {
        // Lấy ID từ record để tìm thông tin bài hát
        models::Song* song = m_player->getLibrary().findSongByID(rec.songId);

        if (song) {
            int row = ui->tableHistory->rowCount();
            ui->tableHistory->insertRow(row);
            // Cột 1: Title
            ui->tableHistory->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(song->title)));
            // Cột 2: Artist
            ui->tableHistory->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(song->artist)));
            // --- CỘT 3: TIME
            QDateTime timeObj = QDateTime::fromSecsSinceEpoch(rec.timestamp);
            // 2. Format thành chuỗi
            QString timeStr = timeObj.toString("dd/MM HH:mm");

            QTableWidgetItem* timeItem = new QTableWidgetItem(timeStr);
            timeItem->setTextAlignment(Qt::AlignCenter); // Căn giữa cho đẹp
            ui->tableHistory->setItem(row, 2, timeItem);
        }
}
}

// --- PHẦN 3: CẬP NHẬT TOP LIST (DEMO) ---
void PersonalView::updateTopLists() {
    ui->listTopSongs->clear();
    ui->listTopArtists->clear();
     // ---------------------------------------------------------
    // A. TOP BÀI HÁT (YÊU CẦU 3: TRONG TUẦN NÀY)
    // Sử dụng tham số THIS_WEEK (Từ Thứ 2 -> Chủ Nhật tuần hiện tại)
    // ---------------------------------------------------------
    auto topSongsWeek = m_stats->getTopSongs(10, THIS_WEEK); 
    int rank = 1;
    for (const auto& pair : topSongsWeek) {
        int id = pair.first;
        int count = pair.second;

        // Tìm bài hát
        models::Song* song = m_player->getLibrary().findSongByID(id);
        if (song) {
            // Format: "#1. Ten Bai Hat (5 plays)"
            QString text = QString("#%1. %2 (%3 plays)")
                           .arg(rank)
                           .arg(QString::fromStdString(song->title))
                           .arg(count);
            ui->listTopSongs->addItem(text);
            rank++;
        }
    }
     // Nếu tuần này chưa nghe gì thì báo
    if (topSongsWeek.empty()) {
        ui->listTopSongs->addItem("(No data for this week)");
    }
    // ---------------------------------------------------------
    // B. TOP NGHỆ SĨ (YÊU CẦU 4: TRONG TUẦN NÀY)
    // Cũng dùng tham số THIS_WEEK
    // ---------------------------------------------------------
    auto topArtistsWeek = m_stats->getTopArtists(m_player->getLibrary(), 5, THIS_WEEK);
    
    rank = 1;
    for (const auto& pair : topArtistsWeek) {
        QString artistName = QString::fromStdString(pair.first);
        int count = pair.second;

        // Format: "#1. Son Tung M-TP (10 plays)"
        QString text = QString("#%1. %2 (%3 plays)")
                       .arg(rank)
                       .arg(artistName)
                       .arg(count);
        ui->listTopArtists->addItem(text);
        rank++;
    }

    if (topArtistsWeek.empty()) {
        ui->listTopArtists->addItem("(No data for this week)");
    }
}