#include "../../include/view/PlaylistView.hpp"
#include <QListWidgetItem>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>

PlaylistView::PlaylistView(controller::MusicPlayer* player, QWidget *parent) 
    : QWidget(parent), m_player(player) 
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Tiêu đề
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    QLabel* title = new QLabel("My Playlists", this);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");
    headerLayout->addWidget(title);
    
    headerLayout->addStretch(); // Đẩy nút sang phải

    // --- NÚT TẠO PLAYLIST ---
    m_btnCreate = new QPushButton("+ New Playlist", this);
    m_btnCreate->setCursor(Qt::PointingHandCursor);
    m_btnCreate->setStyleSheet(
        "QPushButton { background-color: #1DB954; color: white; font-weight: bold; border-radius: 20px; padding: 10px 20px; }"
        "QPushButton:hover { background-color: #1ed760; }"
    );
    // Kết nối nút bấm
    connect(m_btnCreate, &QPushButton::clicked, this, &PlaylistView::handleCreatePlaylist);
    headerLayout->addWidget(m_btnCreate);

    layout->addLayout(headerLayout);

    // Danh sách Playlist
    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet(	
        "QListWidget { background-color: transparent; border: none; }"
        "QListWidget::item { background-color: #282828; color: white; border-radius: 8px; padding: 15px; margin-bottom: 10px; }"
        "QListWidget::item:hover { background-color: #3E3E3E; }"
    );
    layout->addWidget(m_listWidget);

    // Load dữ liệu
    loadPlaylists();

    // Kết nối sự kiện click
    connect(m_listWidget, &QListWidget::itemClicked, this, &PlaylistView::onItemClicked);
}

void PlaylistView::loadPlaylists() {
    if (!m_player) return;
	m_listWidget->clear(); 

	// Bật chế độ tự động xuống dòng cho ListWidget
    m_listWidget->setWordWrap(true); 
    
    const auto& playlists = m_player->getPlaylistLibrary().getPlaylists();
    
    for (size_t i = 0; i < playlists.size(); ++i) {
        const auto& pl = playlists[i];
        
        // Tạo text hiển thị: Tên dòng 1, Mô tả dòng 2
        QString displayText = pl.name + "\n" + pl.description;
        
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setFont(QFont("Segoe UI", 12));

        // Lưu index của playlist trong vector vào item để lát lấy ra cho dễ
        item->setData(Qt::UserRole, (int)i);
        
        m_listWidget->addItem(item);
    }
}

void PlaylistView::onItemClicked(QListWidgetItem* item) {
    if (!item) return;
    
    // Lấy lại index đã lưu
    int index = item->data(Qt::UserRole).toInt();
    
    // Lấy thông tin playlist từ Controller
    const auto& playlists = m_player->getPlaylistLibrary().getPlaylists();
    if (index >= 0 && index < (int)playlists.size()) {
        const auto& pl = playlists[index];
        
        // Bắn tín hiệu ra ngoài MainWindow
        emit playlistClicked(pl.songIDs, pl.name);
    }
}

void PlaylistView::handleCreatePlaylist() {
    if (!m_player) return;

    // 1. Nhập Tên Playlist
    bool ok;
    QString name = QInputDialog::getText(this, "New Playlist", 
                                         "Playlist Name:", QLineEdit::Normal, 
                                         "My Favorite Songs", &ok);
    if (!ok || name.isEmpty()) return;

    // 2. Nhập Mô tả (Optional)
    QString desc = QInputDialog::getText(this, "New Playlist", 
                                         "Description:", QLineEdit::Normal, 
                                         "Created by me", &ok);
    if (!ok) return;

    // 3. Chọn Bài Hát (Tạo Dialog chứa danh sách bài hát có Checkbox)
    QDialog selectDialog(this);
    selectDialog.setWindowTitle("Select Songs");
    selectDialog.setMinimumSize(400, 500);
    selectDialog.setStyleSheet("background-color: #2b2b2b; color: white;");

    QVBoxLayout* dlgLayout = new QVBoxLayout(&selectDialog);
    
    QListWidget* songList = new QListWidget(&selectDialog);
    songList->setStyleSheet("QListWidget { border: 1px solid #444; border-radius: 4px; background-color: #1a1a1a; }"
                            "QListWidget::item { padding: 5px; }");
    
    // Lấy tất cả bài hát từ thư viện để hiển thị
    const auto& allSongs = m_player->getLibrary().getAllSongs();
    for (const auto& s : allSongs) {
        QString text = QString::fromStdString(s.title) + " - " + QString::fromStdString(s.artist);
        QListWidgetItem* item = new QListWidgetItem(text, songList);
        
        // Thêm Checkbox
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); 
        item->setCheckState(Qt::Unchecked);
        
        // Lưu ID bài hát
        item->setData(Qt::UserRole, s.id);
    }
    dlgLayout->addWidget(songList);

    // Nút OK/Cancel cho Dialog
    QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &selectDialog);
    connect(btnBox, &QDialogButtonBox::accepted, &selectDialog, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &selectDialog, &QDialog::reject);
    dlgLayout->addWidget(btnBox);

    // 4. Hiển thị Dialog và xử lý kết quả
    if (selectDialog.exec() == QDialog::Accepted) {
        std::vector<int> selectedIDs;
        
        // Duyệt qua danh sách để tìm các bài được tick
        for (int i = 0; i < songList->count(); ++i) {
            QListWidgetItem* item = songList->item(i);
            if (item->checkState() == Qt::Checked) {
                selectedIDs.push_back(item->data(Qt::UserRole).toInt());
            }
        }

        if (selectedIDs.empty()) {
            QMessageBox::warning(this, "Empty Playlist", "Please select at least one song.");
            return;
        }

        // 5. Gọi Controller để tạo Playlist
        m_player->createUserPlaylist(name, desc, selectedIDs);

        // 6. Reload lại giao diện Playlist
        loadPlaylists();
        
        QMessageBox::information(this, "Success", "Playlist created successfully!");
    }
}