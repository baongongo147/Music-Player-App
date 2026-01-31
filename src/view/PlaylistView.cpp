#include "ui_PlaylistView.h"
#include "../../include/view/PlaylistView.hpp"

#include <QListWidgetItem>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QScrollBar>
#include <QCheckBox>

// =================================================================================
// CONSTRUCTOR & DESTRUCTOR
// =================================================================================
PlaylistView::PlaylistView(controller::MusicPlayer* player, QWidget *parent) 
    : QWidget(parent), ui(new Ui::PlaylistView), m_player(player) 
{
    ui->setupUi(this); 

    // Kết nối nút tạo mới
    connect(ui->btnCreate, &QPushButton::clicked, this, &PlaylistView::handleCreatePlaylist);

    // Tinh chỉnh thanh cuộn cho List (nếu chưa chỉnh trong UI)
    ui->listPlaylists->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { background: #121212; width: 8px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #444; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    // Load dữ liệu ban đầu
    loadPlaylists();
}

PlaylistView::~PlaylistView() {
    delete ui;
}

// =================================================================================
// HÀM TIỆN ÍCH: HIỂN THỊ DIALOG ĐẸP + MÀN ĐEN (ĐỒNG BỘ MAINWINDOW)
// =================================================================================
void PlaylistView::showCustomDialog(const QString& title, const QString& content, QMessageBox::Icon icon) {
    // 1. Tạo Overlay (Màn đen che phủ View này)
    QWidget* overlay = new QWidget(this);
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    overlay->show();

    // 2. Tạo MessageBox
    QMessageBox msgBox(this); 
    msgBox.setWindowTitle(title);
    msgBox.setText(content);
    msgBox.setIcon(icon);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    // (StyleSheet sẽ tự động kế thừa từ MainWindow/App nếu đã set global)
    // Nhưng để chắc chắn, ta có thể set cứng ở đây nếu cần thiết (hoặc bỏ qua nếu đã set ở file UI chính)
    
    msgBox.exec();

    // 3. Xóa màn đen
    delete overlay;
}

// =================================================================================
// LOGIC HIỂN THỊ DANH SÁCH (CARD STYLE)
// =================================================================================
void PlaylistView::loadPlaylists() {
    if (!m_player) return;
    ui->listPlaylists->clear(); 
    
    const auto& playlists = m_player->getPlaylistLibrary().getPlaylists();
    
    for (const auto& pl : playlists) {
        // 1. Tạo Item gốc
        QListWidgetItem* item = new QListWidgetItem(ui->listPlaylists);
        
        // 2. Tạo Widget Container (Card)
        QWidget* cardWidget = new QWidget();
        
        // --- STYLE CARD ---
        cardWidget->setStyleSheet(
            "QWidget { background-color: #181818; border: 1px solid #282828; border-radius: 8px; }"
            "QWidget:hover { background-color: #202020; border-color: #333; }"
            "QLabel { border: none; background: transparent; }" 
            "QPushButton { border: none; }" 
        );

        // --- LAYOUT ---
        QHBoxLayout* cardLayout = new QHBoxLayout(cardWidget);
        cardLayout->setContentsMargins(20, 15, 20, 15);
        cardLayout->setSpacing(20);

        // Cột Trái: Text
        QVBoxLayout* textLayout = new QVBoxLayout();
        textLayout->setSpacing(5);
        
        QLabel* lblName = new QLabel(pl.name);
        lblName->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
        
        QLabel* lblDesc = new QLabel(pl.description);
        lblDesc->setStyleSheet("color: #B3B3B3; font-size: 13px;");
        lblDesc->setWordWrap(true);

        textLayout->addWidget(lblName);
        textLayout->addWidget(lblDesc);
        
        // Cột Phải: Actions
        QHBoxLayout* actionLayout = new QHBoxLayout();
        
        // Nút Play
        QPushButton* btnPlay = new QPushButton("Play");
        btnPlay->setCursor(Qt::PointingHandCursor);
        btnPlay->setFixedSize(70, 32);
        btnPlay->setStyleSheet(
            "QPushButton { background-color: #1DB954; color: white; border-radius: 16px; font-weight: bold; font-size: 13px; }"
            "QPushButton:hover { background-color: #1ed760; }"
        );
        connect(btnPlay, &QPushButton::clicked, [this, pl]() {
            emit playlistClicked(pl.songIDs, pl.name);
        });

        // Nút Delete
        QPushButton* btnDelete = new QPushButton();
        btnDelete->setIcon(QIcon(":/icons/delete.svg")); 
        btnDelete->setFixedSize(32, 32);
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setStyleSheet(
            "QPushButton { background-color: #2a2a2a; border-radius: 16px; border: 1px solid #444; }"
            "QPushButton:hover { background-color: rgba(255, 59, 48, 0.2); border-color: #ff3b30; }"
        );
        
        connect(btnDelete, &QPushButton::clicked, [this, pl]() {
            // ... (Code tạo dialog xóa giữ nguyên như cũ) ...
            QWidget* overlay = new QWidget(this);
            overlay->setGeometry(this->rect());
            overlay->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
            overlay->show();

            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Delete Playlist");
            msgBox.setText("Are you sure you want to delete '" + pl.name + "'?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.setIcon(QMessageBox::Warning);
            
            if (msgBox.exec() == QMessageBox::Yes) {
                if (m_player->deletePlaylist(pl.name)) {
                    loadPlaylists(); 
                }
            }
            delete overlay;
        });

        actionLayout->addWidget(btnPlay);
        actionLayout->addWidget(btnDelete);

        // Ráp Layout
        cardLayout->addLayout(textLayout, 1); 
        cardLayout->addLayout(actionLayout, 0); 

        // --- [QUAN TRỌNG NHẤT: XỬ LÝ KÍCH THƯỚC] ---

        // 1. Ép chiều cao Card Widget CỐ ĐỊNH là 90px
        // Dùng setFixedHeight tốt hơn setMinimumHeight vì nó giữ form chuẩn, ko bị giãn dọc
        cardWidget->setFixedHeight(90); 

        // 2. Set SizeHint cho Item chứa nó
        // - Tham số Width = 0 (hoặc 1): Để Qt tự động tính toán giãn full chiều ngang (Adjust Mode).
        // - Tham số Height = 90: Để khớp với chiều cao của Widget.
        item->setSizeHint(QSize(0, 90)); 
        
        ui->listPlaylists->setItemWidget(item, cardWidget);
    }
}

// Helper: Setup style cho các Dialog nhập liệu
void PlaylistView::setupDialogStyle(QDialog* dialog) {
    dialog->setStyleSheet(
        "QDialog { background-color: #202020; border: 2px solid #1DB954; border-radius: 10px; }"
        "QLabel { background-color: transparent; color: white; font-size: 14px; font-weight: 500; font-family: 'Segoe UI'; }"
        "QLineEdit { background-color: #333; color: white; border: 1px solid #555; border-radius: 5px; padding: 6px; }"
        "QLineEdit:focus { border: 1px solid #1DB954; }"
        "QListWidget { background-color: #1a1a1a; border: 1px solid #444; color: white; border-radius: 5px; }"
        "QListWidget::item { padding: 5px; }"
        "QListWidget::item:hover { background-color: #333; }"
        
        // Style nút bấm giống hộp thoại thông báo
        "QPushButton { background-color: transparent; color: #B3B3B3; border: 1px solid #555; border-radius: 18px; padding: 6px 25px; font-weight: bold; min-width: 80px; }"
        "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); color: white; border: 1px solid #1DB954; }"
        "QPushButton:pressed { background-color: rgba(29, 185, 84, 0.2); }"
    );
}

// =================================================================================
// LOGIC TẠO PLAYLIST MỚI (CÓ OVERLAY)
// =================================================================================
void PlaylistView::handleCreatePlaylist() {
    if (!m_player) return;

    // 1. Tạo Màn đen bao trùm
    QWidget* overlay = new QWidget(this);
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    overlay->show();

    // --- PHASE 1: NHẬP TÊN & MÔ TẢ ---
    QDialog dialog(this);
    dialog.setWindowTitle("New Playlist");
    dialog.setMinimumWidth(400);
    setupDialogStyle(&dialog);

    QFormLayout* form = new QFormLayout(&dialog);
    form->setSpacing(15);
    form->setContentsMargins(20, 20, 20, 20);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    nameEdit->setPlaceholderText("Enter playlist name...");
    
    QLineEdit* descEdit = new QLineEdit(&dialog);
    descEdit->setPlaceholderText("Description (Optional)...");

    form->addRow("Name:", nameEdit);
    form->addRow("Description:", descEdit);

    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(btnBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    form->addRow(btnBox);

    // Chờ người dùng nhập
    if (dialog.exec() != QDialog::Accepted) {
        delete overlay; // Hủy nếu bấm Cancel
        return;
    }

    QString name = nameEdit->text().trimmed();
    QString desc = descEdit->text().trimmed();

    if (name.isEmpty()) {
        delete overlay; return; 
    }

    // Check trùng tên
    if (m_player->checkPlaylistExists(name)) {
        // Dùng custom message box, vẫn giữ overlay hiện tại
        QMessageBox warningBox(this);
        warningBox.setWindowTitle("Error");
        warningBox.setText("A playlist with this name already exists.");
        warningBox.setIcon(QMessageBox::Warning);
        // Style box này sẽ ăn theo global hoặc set cứng
        warningBox.exec();
        
        delete overlay; 
        return;
    }

    // --- PHASE 2: CHỌN BÀI HÁT ---
    QDialog selectDialog(this);
    selectDialog.setWindowTitle("Select Songs");
    selectDialog.resize(500, 600);
    setupDialogStyle(&selectDialog);

    QVBoxLayout* vLayout = new QVBoxLayout(&selectDialog);
    vLayout->addWidget(new QLabel("Select songs to add:", &selectDialog));

    QListWidget* songList = new QListWidget(&selectDialog);
    
    const auto& allSongs = m_player->getLibrary().getAllSongs();
    for (const auto& s : allSongs) {
        QString text = QString::fromStdString(s.title) + " - " + QString::fromStdString(s.artist);
        QListWidgetItem* item = new QListWidgetItem(text, songList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, s.id);
    }
    vLayout->addWidget(songList);

    QDialogButtonBox* btnBox2 = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &selectDialog);
    connect(btnBox2, &QDialogButtonBox::accepted, &selectDialog, &QDialog::accept);
    connect(btnBox2, &QDialogButtonBox::rejected, &selectDialog, &QDialog::reject);
    vLayout->addWidget(btnBox2);

    if (selectDialog.exec() == QDialog::Accepted) {
        std::vector<int> selectedIDs;
        for (int i = 0; i < songList->count(); ++i) {
            if (songList->item(i)->checkState() == Qt::Checked) {
                selectedIDs.push_back(songList->item(i)->data(Qt::UserRole).toInt());
            }
        }

        if (selectedIDs.empty()) {
            QMessageBox::warning(this, "Empty", "Please select at least one song.");
        } else {
            // TẠO THÀNH CÔNG
            m_player->createUserPlaylist(name, desc, selectedIDs);
            loadPlaylists();
            // showCustomDialog("Success", "Playlist created!", QMessageBox::Information);
        }
    }

    // Cuối cùng: Xóa Overlay
    delete overlay;
}