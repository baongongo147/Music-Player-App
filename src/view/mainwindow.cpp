#include "ui_MainWindow.h" 
#include "../../include/view/mainwindow.hpp"
#include "../../include/controller/MusicPlayer.hpp"

// Hàm chuyển đổi mili-giây sang chuỗi "mm:ss" hoặc "hh:mm:ss"
static QString formatTime(qint64 ms) {
    if (ms <= 0) return "00:00";
    
    QTime time(0, 0, 0);
    time = time.addMSecs(ms);
    
    QString format = "mm:ss";
    if (ms > 3600000) format = "hh:mm:ss"; // Nếu dài hơn 1 giờ
    
    return time.toString(format);
}

// -------------------------------------------------------------
// CONSTRUCTOR
// -------------------------------------------------------------
MainWindow::MainWindow(const std::string& songFile, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      player(songFile),
      media(new QMediaPlayer(this)),
      // 1. KHỞI TẠO MẶC ĐỊNH 
      isLoopenabled(false),           // Mặc định tắt loop
      m_lastVolume(50),               // Mặc định vol 50
      m_isMuted(false),               
      m_isPlayingFromPlayNext(false), // Mặc định không phải từ Next
      m_lastPlayedQueueID(-1),        // Mặc định chưa hát bài nào
      m_currentSongID(-1)             // Mặc định không có bài đang chọn
{
    // 2. SETUP UI CƠ BẢN
    ui->setupUi(this); 
    ui->homeLayout->setStretch(0, 1);
    ui->homeLayout->setStretch(1, 1);
    ui->homeLayout->setStretch(2, 1);
    ui->horizontalSlider->setRange(0, 100);
    ui->horizontalSlider->setValue(50);

    // 3. SETUP SIDEBAR & PAGES
    m_playlistView = new PlaylistView(&player, this);
    m_personalView = new PersonalView(&player, &statsManager, this);
    
    ui->mainStack->removeWidget(ui->pagePlaylist);
    ui->mainStack->removeWidget(ui->pagePersonal);
    ui->mainStack->addWidget(m_playlistView);
    ui->mainStack->addWidget(m_personalView);
    ui->mainStack->setCurrentIndex(0);

    // 4. SETUP KẾT NỐI (CONNECTIONS)
    connect(ui->btnNavHome, &QPushButton::clicked, [this](){ ui->mainStack->setCurrentIndex(0); });
    connect(ui->btnNavPlaylist, &QPushButton::clicked, [this](){ ui->mainStack->setCurrentIndex(1); });
    connect(ui->btnNavPersonal, &QPushButton::clicked, [this](){ m_personalView->refreshData(); ui->mainStack->setCurrentIndex(2); });
    
    connect(ui->btnVolume, &QPushButton::clicked, this, &MainWindow::on_btnVolume_clicked);
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::handleVolumeChanged);
    
    connect(ui->btnNext, &QPushButton::clicked, this, &MainWindow::handleNext);
    connect(ui->btnPrevious, &QPushButton::clicked, this, &MainWindow::handlePrev);
    connect(ui->btnShuffle, &QPushButton::clicked, this, &MainWindow::handleShuffle);
    connect(ui->btnPlayPause, &QPushButton::clicked, this, &MainWindow::handlePlayPause);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::handleStop);
    connect(ui->btnLoop, &QPushButton::clicked, this, &MainWindow::handleLoopToggle);
    
    connect(ui->btnSmart, &QPushButton::clicked, this, &MainWindow::handleSmartPlaylist);
    connect(ui->lineSearch, &QLineEdit::textChanged, this, &MainWindow::handleSearch);
    
    // Double click events
    connect(ui->listPlayback, &QListWidget::itemDoubleClicked, this, &MainWindow::handleSongDoubleClick);
    connect(ui->listSearchResult, &QListWidget::itemDoubleClicked, this, &MainWindow::handleSongDoubleClick);
    connect(ui->listAllSongs, &QListWidget::itemDoubleClicked, this, &MainWindow::handleLibraryDoubleClick);
    
    // Media events
    connect(media, &QMediaPlayer::durationChanged, this, &MainWindow::handleDurationChanged);
    connect(media, &QMediaPlayer::positionChanged, this, &MainWindow::handlePositionChanged);
    connect(media, &QMediaPlayer::stateChanged, this, &MainWindow::onMediaStateChanged);
    connect(media, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::onMediaStatusChanged);
    connect(ui->sliderDuration, &QSlider::sliderMoved, this, &MainWindow::handleSeek);
    connect(ui->sliderDuration, &QSlider::sliderReleased, [this]() { handleSeek(ui->sliderDuration->value()); });
    
    connect(m_playlistView, &PlaylistView::playlistClicked, this, &MainWindow::handlePlaylistSelected);

    // 5. LOAD DỮ LIỆU CƠ BẢN (LIBRARY)
    // Luôn load thư viện gốc trước, dù có restore hay không
    loadLibraryToUI(); 
    if(media) media->setVolume(50); // Set âm lượng phần cứng mặc định

    // ==========================================================
    // 6. LOGIC RESTORE SESSION (XỬ LÝ CUỐI CÙNG)
    // ==========================================================
    QFile sessionFile("session.json");
    if (sessionFile.exists()) {
        // Dùng hàm showCustomDialog không tiện ở đây vì ta cần lấy kết quả Yes/No
        // Nên dùng lại code tạo MessageBox thủ công nhưng dùng style đẹp
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Restore Session");
        msgBox.setText("Do you want to continue from the last session?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setIcon(QMessageBox::Question);

        // Copy Style đẹp vào đây
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #202020; border: 2px solid #1DB954; border-radius: 10px; }"
            "QLabel { color: #FFFFFF; font-size: 14px; font-weight: 500; font-family: 'Segoe UI'; margin: 10px 15px; }"
            "QPushButton { background-color: #333333; color: white; border: 1px solid #444; border-radius: 6px; padding: 6px 25px; min-width: 80px; }"
            "QPushButton:hover { background-color: #1DB954; color: black; border: none; }"
        );

        int reply = msgBox.exec();
        
        if (reply == QMessageBox::Yes) {
            // --- A. NGƯỜI DÙNG CHỌN RESTORE ---
            
            // 1. Load dữ liệu từ file vào Backend
            player.restoreSession(); 

            // 2. Cập nhật các biến Logic Frontend (QUAN TRỌNG)
            // Lấy giá trị từ Backend đắp vào biến của MainWindow
            m_lastPlayedQueueID = player.getRestoredLastQueueID();
            m_isPlayingFromPlayNext = player.getRestoredIsFromNext();

            // 3. Vẽ lại giao diện
            refreshPlaybackQueueUI(); 
            refreshHistoryUI();
            refreshPlayNextUI(); 
            m_playlistView->loadPlaylists();

            // 4. Khôi phục Volume
            int savedVol = player.getVolume();
            m_lastVolume = savedVol; // Cập nhật biến nhớ volume
            ui->horizontalSlider->setValue(savedVol); 
            if(media) media->setVolume(savedVol); 

            // 5. Khôi phục bài đang hát dở (Hiển thị UI thôi, không auto play)
            // Đọc lại file JSON để lấy "actualPlayingID" 
            QFile file("session.json");
            if (file.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                QJsonObject root = doc.object();
                
                int playingID = -1;
                if (root.contains("actualPlayingID")) playingID = root["actualPlayingID"].toInt();
                
                if (playingID != -1) {
                    models::Song* sPtr = player.getLibrary().findSongByID(playingID);
                    if (sPtr) {
                        m_currentSongID = sPtr->id; // Cập nhật biến nhớ ID
                        m_currentSongTitle = QString::fromStdString(sPtr->title);
                        
                        // Hiển thị trạng thái Stopped
                        ui->labelCurrent->setText("Stopped: " + m_currentSongTitle);
                        
                        // Nạp file sẵn sàng
                        QString qpath = QString::fromStdString(sPtr->filePath);
                        media->setMedia(QUrl::fromLocalFile(qpath));
                    }
                }
                file.close();
            }

        } else {
            // --- B. NGƯỜI DÙNG CHỌN NO (RESET) ---
            sessionFile.remove(); // Xóa file save cũ
            
            // Các biến đã được khởi tạo mặc định ở bước 1 (Initializer List) 
            // nên không cần làm gì thêm, chỉ cần đảm bảo volume về chuẩn.
            player.setVolume(50);
            if(media) media->setVolume(50);
        }
    } else {
        // --- C. KHÔNG CÓ FILE SESSION ---
        // Chạy mặc định như mới
        player.setVolume(50);
    }
}

MainWindow::~MainWindow()
{
    delete ui; // Giải phóng bộ nhớ giao diện
}

// -------------------------------------------------------------
// LOGIC: LOAD LIBRARY
// -------------------------------------------------------------
void MainWindow::loadLibraryToUI() {
    const auto& lib = player.getLibrary().getAllSongs();
    ui->listAllSongs->clear(); 
    
    for (const auto& s : lib) {
        // 1. Tạo Item
        QListWidgetItem* item = new QListWidgetItem(ui->listAllSongs);
        
        // 2. Tạo Widget Container
        QWidget* widget = new QWidget();
        // Quan trọng: Set nền trong suốt để khi click chọn dòng vẫn thấy màu highlight của ListWidget
        widget->setStyleSheet("background-color: transparent;"); 
        
        // 3. Setup Layout
        QHBoxLayout* layout = new QHBoxLayout(widget);
        
		// Left = 5: Để khớp với padding-left của các list kia
        // Top/Bottom = 0: Để chiều cao khít với sizeHint
        layout->setContentsMargins(5, 0, 0, 0);
        layout->setSpacing(10);

        // 4. Label Tên bài hát
        QString text = QString::number(s.id) + " - " + QString::fromStdString(s.title) + " - " + QString::fromStdString(s.artist);;
        QLabel* lblName = new QLabel(text);
        lblName->setObjectName("lblCardTitle"); 
        lblName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred); 
        
        // 5. Nút Add
        QPushButton* btnAdd = new QPushButton();
        btnAdd->setIcon(QIcon(":/icons/add.svg")); 
        btnAdd->setFixedSize(26, 26);       
        btnAdd->setCursor(Qt::PointingHandCursor); 

        // - Bình thường: Trong suốt (hoặc xám rất nhạt) để không rối mắt.
        // - Hover: Nền chuyển màu Xanh lá (Spotify Green #1DB954), bo tròn.
        btnAdd->setStyleSheet(
            "QPushButton { border: none; background: transparent; border-radius: 13px; }"
            "QPushButton:hover { background-color: #1DB954; }"
        );

        // Kết nối nút bấm
        connect(btnAdd, &QPushButton::clicked, [this, s]() { handleAddSongToPlayNext(s.id); });

        // 6. Thêm vào layout (Căn giữa dọc tuyệt đối)
        layout->addWidget(lblName, 1, Qt::AlignVCenter); // Số 1 là stretch factor (chiếm hết chỗ trống)
        layout->addWidget(btnAdd, 0, Qt::AlignVCenter);  // Căn giữa nút theo chiều dọc

        // 7. Set chiều cao cố định cho dòng (Tạo độ thoáng)
        item->setSizeHint(QSize(widget->sizeHint().width(), 36)); 
        item->setData(Qt::UserRole, s.id);
        
        ui->listAllSongs->setItemWidget(item, widget);
    }
}

// -------------------------------------------------------------
// LOGIC: REFRESH PLAYBACK QUEUE (Cập nhật danh sách đang phát)
// -------------------------------------------------------------
// Hàm này dùng để đồng bộ dữ liệu từ player.getPlayBack() lên UI listPlayback
void MainWindow::refreshPlaybackQueueUI() {
    ui->listPlayback->clear();
    const auto& queue = player.getPlayBack().getQueue();
    int index = 1;
    for (const auto& s : queue) {
        QListWidgetItem* item = new QListWidgetItem(ui->listPlayback);
        QWidget* widget = new QWidget();
        widget->setStyleSheet("background-color: transparent;"); 
        
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(5, 0, 0, 0); 
        layout->setSpacing(10);

        QString displayText = QString::number(index) + " - " + QString::fromStdString(s.title) + " - " + QString::fromStdString(s.artist);
        QLabel* lblName = new QLabel(displayText);

        lblName->setObjectName("lblCardTitle"); 
        
        lblName->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred); 

        QPushButton* btnRemove = new QPushButton();
        btnRemove->setIcon(QIcon(":/icons/removeSong.svg")); 
        btnRemove->setFixedSize(24, 24);       
        btnRemove->setCursor(Qt::PointingHandCursor);
        btnRemove->setStyleSheet(
            "QPushButton { border: none; background: transparent; border-radius: 12px; }"
            "QPushButton:hover { background-color: rgba(255, 59, 48, 0.2); }"
        );
        connect(btnRemove, &QPushButton::clicked, [this, s]() { handleRemoveSongDirectly(s.id); });

        layout->addWidget(lblName, 1, Qt::AlignVCenter);
        layout->addWidget(btnRemove, 0, Qt::AlignVCenter);

        item->setSizeHint(QSize(widget->sizeHint().width(), 36)); 
        item->setData(Qt::UserRole, s.id); 
        ui->listPlayback->setItemWidget(item, widget);
        index++;
    }
}

// -------------------------------------------------------------
// LOGIC: PLAY AUDIO
// -------------------------------------------------------------
void MainWindow::playAudioFile(const std::string& path) {
    QString qpath = QString::fromStdString(path);
    media->setMedia(QUrl::fromLocalFile(qpath));
    media->play();
}

// -------------------------------------------------------------
// LOGIC: UPDATE UI (LABEL)
// -------------------------------------------------------------
void MainWindow::updateNowPlaying(const models::Song& s) {
	m_currentSongID = s.id;

    m_currentSongTitle = QString::fromStdString(s.title);
    ui->labelCurrent->setText("Current Song: " + m_currentSongTitle);
    playAudioFile(s.filePath);
}

// -------------------------------------------------------------
// EVENT: NEXT
// -------------------------------------------------------------
void MainWindow::handleNext() {
    // 1. ƯU TIÊN UP NEXT
    if (!player.getPlayNext().empty()) {
        auto nextQueue = player.getPlayNext().getNextQueue();
        models::Song s = nextQueue.front();
        player.getPlayNext().popNextSong();
        refreshPlayNextUI();

        // Setup trạng thái
        m_isPlayingFromPlayNext = true; 

        logCurrentSongListening();
        songStartTime = QTime::currentTime();
        updateNowPlaying(s);
        
        qDebug() << "Playing from UP NEXT:" << QString::fromStdString(s.title);
        return;
    }

    // 2. PLAYBACK QUEUE (MAIN)
    if (!player.getPlayBack().empty()) {
        try {
            auto currentIteratorSong = player.getPlayBack().getCurrentSong();

            // Nếu bài Iterator đang trỏ tới (currentIteratorSong.id)
            // TRÙNG với bài cuối cùng từng hát ở Queue (m_lastPlayedQueueID)
            // -> Nghĩa là bài này hát rồi -> Phải Next (+1)
            if (currentIteratorSong.id == m_lastPlayedQueueID) {
                player.playSong(); // Iterator++
                currentIteratorSong = player.getPlayBack().getCurrentSong();
				qDebug() << "Skipping played song. Next is:" << QString::fromStdString(currentIteratorSong.title);
            } else {
				qDebug() << "Resume/Start at iterator:" << QString::fromStdString(currentIteratorSong.title);
			}

            // Setup trạng thái
            m_isPlayingFromPlayNext = false;

			// Cập nhật biến nhớ: "Đây là bài Main Queue mới nhất đã hát"
            m_lastPlayedQueueID = currentIteratorSong.id;

            logCurrentSongListening();
            songStartTime = QTime::currentTime();
            updateNowPlaying(currentIteratorSong);
            refreshPlaybackQueueUI();

        } catch (...) {
            // Hết bài
            handleStop();
            ui->labelCurrent->setText("Ready to Play");
            ui->labelDuration->setText("--:-- / --:--");
            ui->sliderDuration->setValue(0);
            m_currentSongTitle = "";
            m_lastPlayedQueueID = -1;

			// Reset biến nhớ để lần sau play lại từ đầu
            m_lastPlayedQueueID = -1; 
            qDebug() << "Queue finished.";
        }
    } else {
        handleStop();

		// Gỡ file nhạc ra khỏi player
        media->setMedia(QMediaContent()); 

        ui->labelCurrent->setText("Ready to Play");
        m_currentSongTitle = "";
        m_lastPlayedQueueID = -1;
    }
}

// -------------------------------------------------------------
// EVENT: PREVIOUS
// -------------------------------------------------------------
void MainWindow::handlePrev() {
    logCurrentSongListening();
    songStartTime = QTime::currentTime();

    if (player.getHistory().getHistory().empty()) {
        showCustomDialog("Info", "No history available.", QMessageBox::Information);
        return;
    }

    try {
        // 1. Lấy bài cũ ra (Pop)
        auto prevSong = player.getHistory().playPreviousSong();

        // 2. Reset cờ Next (Vì Prev thì ko tính là Up Next)
        m_isPlayingFromPlayNext = false;
        
        // Cập nhật lại ID Queue để logic Next sau này chạy đúng
        // Nếu bài prev này nằm trong queue thì cập nhật ID, nếu ko thì thôi
        m_lastPlayedQueueID = prevSong.id; 

        // 3. Phát bài cũ
        // selectAndPlaySong sẽ tự động PUSH bài hiện tại (bài đang nghe dở) vào History
        try {
            player.selectAndPlaySong(prevSong.id);
        } catch (...) {
            qDebug() << "Song not in queue, playing anyway.";
        }
        
        // 4.
        // Xóa cái bài vừa bị selectAndPlaySong đẩy vào History
        // Để đảm bảo ta đang "Lùi lại" chứ không phải "Tiến tới bài cũ"
        player.getHistory().removeLastAddedSong();

        // 5. Update UI
        updateNowPlaying(prevSong);
        refreshUI();

    } catch (const std::exception& e) {
        showCustomDialog("Error", e.what(), QMessageBox::Warning);
    }
}

// -------------------------------------------------------------
// EVENT: SHUFFLE
// -------------------------------------------------------------
void MainWindow::handleShuffle() {
    // 1. Cập nhật biến logic từ trạng thái nút
    bool isShuffleOn = ui->btnShuffle->isChecked();

	player.enableShuffle(isShuffleOn);
	
	// 2. Debug hoặc xử lý logic playlist (nếu có)
    // Ví dụ: console log để kiểm tra
	qDebug() << "Shuffle:" << (isShuffleOn ? "ON" : "OFF");
}

// -------------------------------------------------------------
// EVENT: SMART PLAYLIST (Đã thêm bộ lọc trùng lặp)
// -------------------------------------------------------------
void MainWindow::handleSmartPlaylist() {
    // 1. Lấy Start ID
    QString startStr = ui->lineSmartID->text();
    bool ok1;
    int startID = startStr.toInt(&ok1);

    // 2. Lấy Size
    QString sizeStr = ui->lineSmartSize->text();
    bool ok2;
    int size = sizeStr.toInt(&ok2);

    if (!ok1 || !ok2) {
        showCustomDialog("Input Error", "Please enter valid numbers for ID and Size", QMessageBox::Warning);
        return;
    }

    // 3. Gọi logic tạo playlist từ Backend (Backend cứ việc tạo ra danh sách thô)
    auto smartQ = player.createSmartPlaylist(startID, size);

    if (smartQ.size() == 0) {
        showCustomDialog("Info", "Could not generate Smart Playlist (check ID range)", QMessageBox::Information);
        return;
    }

    // --- BẮT ĐẦU LOGIC LỌC TRÙNG ---

    // A. Tạo danh sách các ID đang có sẵn trong Playback Queue
    std::set<int> existingIDs;
    const auto& currentQueue = player.getPlayBack().getQueue();
    for (const auto& s : currentQueue) {
        existingIDs.insert(s.id);
    }

    // B. Kiểm tra trạng thái Queue trước khi thêm
    bool wasEmpty = player.getPlayBack().empty();
    int addedCount = 0;

    // C. Duyệt qua Smart Playlist và chỉ thêm bài CHƯA CÓ
    for (const auto& s : smartQ.getQueue()) {
        // Nếu ID chưa tồn tại trong set existingIDs
        if (existingIDs.find(s.id) == existingIDs.end()) {
            player.getPlayBack().addSong(s);
            
            // Thêm vào set luôn để tránh trường hợp SmartPlaylist bị lỗi dup nội bộ (phòng hờ)
            existingIDs.insert(s.id); 
            addedCount++;
        }
    }

    // --- KẾT THÚC LOGIC LỌC ---

    // Kiểm tra kết quả
    if (addedCount == 0) {
        showCustomDialog("Info", "All songs in the Smart Playlist are already in the Queue.", QMessageBox::Information);
        return;
    }

    // 5. Logic tự động phát nếu trước đó rỗng (Giữ nguyên logic chuẩn của bạn)
    if (wasEmpty && !player.getPlayBack().empty()) {
        auto it = player.getPlayBack().getQueue().begin();
        player.initializePlaybackIterator(it);
        
        try {
            // Cập nhật các biến nhớ để tránh lỗi lặp bài khi Next
            m_isPlayingFromPlayNext = false;
            m_lastPlayedQueueID = it->id;
            
            updateNowPlaying(*it);
        } catch(...) {}
    }

    refreshPlaybackQueueUI();
    
    // Thông báo chi tiết hơn
    showCustomDialog("Success", QString("Added %1 new songs to queue.").arg(addedCount), QMessageBox::Information);
}

// -------------------------------------------------------------
// EVENT: DOUBLE CLICK ITEM
// -------------------------------------------------------------
void MainWindow::handleSongDoubleClick(QListWidgetItem* item) {
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    
    // 1. Lưu thống kê bài cũ
    logCurrentSongListening(); 
    songStartTime = QTime::currentTime(); 

    // 2. Phát bài mới
    player.selectAndPlaySong(id);

    // 3. Cập nhật biến nhớ (Quan trọng để logic Next hoạt động đúng)
    m_lastPlayedQueueID = id;
    m_isPlayingFromPlayNext = false;

    // 4. Update UI
    try {
        auto current = player.getPlayBack().getCurrentSong();
        updateNowPlaying(current);
        refreshUI(); 
    } catch (...) {}
}

// -------------------------------------------------------------
// EVENT: DOUBLE CLICK ITEM (LIBRARY)
// -------------------------------------------------------------
void MainWindow::handleLibraryDoubleClick(QListWidgetItem* item) {
    if (!item) return;
    
    // 1. Lấy ID bài hát từ item được click
    int id = item->data(Qt::UserRole).toInt();

	// 2. Kiểm tra trùng lặp
	// Lấy tham chiếu danh sách queue để kiểm tra nhanh
    const auto& queue = player.getPlayBack().getQueue();
    bool wasEmpty = queue.empty(); // kiểm tra backqueue
    for (auto it = queue.begin(); it != queue.end(); ++it) {
        if (it->id == id) {
            // --- NẾU TÌM THẤY TRÙNG ---
            // Chỉ hiện thông báo và dừng lại, không làm gì thêm.
			showCustomDialog("Notification", "This song is already in the playback queue", QMessageBox::Information);
            return; // Thoát hàm ngay lập tức
        }
    }

    // 3. Nếu không trùng -> Logic: Thêm vào cuối Playback Queue và phát ngay lập tức
    try {
        models::Song* foundSong = player.getLibrary().findSongByID(id);
		if (foundSong) {
			// Thêm vào backend (hàm này đã tự xử lý việc set currentSong nếu list rỗng)
			player.getPlayBack().addSong(*foundSong);
			
			// Vẽ lại danh sách hiển thị
			refreshPlaybackQueueUI();  

			// Kiểm tra máy có đang rảnh không? (Stopped = Rảnh, Playing/Paused = Bận)
            bool isPlayerIdle = (media->state() == QMediaPlayer::StoppedState);

			// Chỉ tự động phát khi: Danh sách cũ Rỗng VÀ Máy đang Rảnh
            if (wasEmpty && isPlayerIdle) {
                songStartTime = QTime::currentTime(); 
                player.getPlayBack().setCurrentSongByID(foundSong->id);
                
				m_isPlayingFromPlayNext = false;
                m_lastPlayedQueueID = foundSong->id;

				updateNowPlaying(*foundSong);
            }
            // Nếu máy đang bận (ví dụ đang hát bài của Up Next), 
            // thì bài mới chỉ âm thầm nằm vào danh sách Playback chờ đến lượt.
		}

    } catch (const std::exception& e) {
		showCustomDialog("Error", "Could not play song from library", QMessageBox::Warning);
    }
}

// -------------------------------------------------------------
// HÀM XỬ LÝ ÂM LƯỢNG
// -------------------------------------------------------------
void MainWindow::handleVolumeChanged(int value)
{
    // 1. Cập nhật âm lượng thực tế
    if (media) media->setVolume(value);     // QMediaPlayer nhận giá trị 0-100
    player.setVolume(value);

    // 2. Logic đồng bộ giao diện
    if (value == 0) {
        // --- KÉO VỀ 0: TỰ ĐỘNG MUTE ---
        if (!m_isMuted) {
            if (media) media->setMuted(true);
            m_isMuted = true;
            
            // Đổi trạng thái nút sang Unchecked (Mute icon)
            // Block signals để tránh gọi lại on_btnVolume_clicked
            const QSignalBlocker blocker(ui->btnVolume);
            ui->btnVolume->setChecked(false); 
        }
    } 
    else {
        // --- KÉO LÊN > 0: TỰ ĐỘNG CÓ TIẾNG ---
        
        // a. Thoát chế độ Mute nếu đang bị Mute
        if (m_isMuted) {
            if (media) media->setMuted(false);
            m_isMuted = false;
            
            // Đổi trạng thái nút sang Checked (Sound icon)
            const QSignalBlocker blocker(ui->btnVolume);
            ui->btnVolume->setChecked(true); 
        }

        // b. Cập nhật biến đệm
        // Chỉ cập nhật khi giá trị kéo tay > 0
        m_lastVolume = value;
    }
}

// HÀM XỬ LÝ MUTE/UNMUTE (NHẤN VÀO NÚT VOLUME)
void MainWindow::on_btnVolume_clicked() {
    // Checked (True) = Có tiếng (Icon VolumeOn)
    // Unchecked (False) = Mute (Icon VolumeOff)
    
    bool isSoundOn = ui->btnVolume->isChecked(); 

    if (!isSoundOn) {
        // --- NGƯỜI DÙNG MUỐN MUTE (TẮT TIẾNG) ---
        
        int currentVal = ui->horizontalSlider->value();
        
        // Chỉ lưu lại giá trị nếu nó đủ lớn (> 0)
        if (currentVal > 0) {
            m_lastVolume = currentVal;
        } 
        
        // Phòng hờ: Nếu biến đệm đang quá nhỏ hoặc bằng 0 thì gán mặc định 50
        if (m_lastVolume < 10) m_lastVolume = 50;

        // Kéo Slider về 0
        ui->horizontalSlider->setValue(0);
    } 
    else {
        // --- NGƯỜI DÙNG MUỐN UNMUTE (MỞ LẠI TIẾNG) ---

        // Nếu giá trị cũ quá bé (do lúc kéo tay về 0 bị dính số 1, 2...)
        // Thì ta ép nó lên mức 30 cho dễ nghe.
        if (m_lastVolume < 10) {
            m_lastVolume = 30;
        }

        // Khôi phục lại Slider
        ui->horizontalSlider->setValue(m_lastVolume);
    }
}

// -------------------------------------------------------------
// HÀM XỬ LÝ DURATION
// -------------------------------------------------------------
void MainWindow::handleDurationChanged(qint64 duration) {
    // duration tính bằng mili-giây
    // Đặt giá trị lớn nhất cho slider bằng tổng thời gian bài hát
    ui->sliderDuration->setRange(0, duration);
}

// -------------------------------------------------------------
// HÀM XỬ LÝ POSITION
// -------------------------------------------------------------
void MainWindow::handlePositionChanged(qint64 position) {
    // 1. Cập nhật Slider (Luôn cần thiết)
    if (!ui->sliderDuration->isSliderDown()) {
        ui->sliderDuration->setValue(position);
    }

    // 2. Nếu đang STOP thì KHÔNG cập nhật Label nữa
    // Để tránh việc nó ghi đè chữ "00:00" lên chữ "--:--" hoặc làm mất chữ "Ready to Play"
    if (media->state() == QMediaPlayer::StoppedState) {
        return; 
    }

    // 3. Cập nhật thời gian khi đang Play/Pause
    qint64 duration = media->duration();
    if (duration > 0) {
        QString currentStr = formatTime(position);
        QString totalStr = formatTime(duration);
        
        // Cập nhật Label bên phải
        ui->labelDuration->setText(currentStr + " / " + totalStr);
    }
}

// -------------------------------------------------------------
// HÀM XỬ LÍ TUA NHẠC
// -------------------------------------------------------------
void MainWindow::handleSeek(int position) {
    // Gọi hàm setPosition của QMediaPlayer để nhảy đến vị trí mili-giây tương ứng
    media->setPosition(position);
}

// -------------------------------------------------------------
// LOGIC: SEARCH (Áp dụng Partial Match giống Console)
// -------------------------------------------------------------
void MainWindow::handleSearch() {
    // 1. Dọn dẹp danh sách kết quả cũ
    ui->listSearchResult->clear();

    // 2. Lấy dữ liệu đầu vào
    QString keyword = ui->lineSearch->text().trimmed(); // Lấy từ khóa, xóa khoảng trắng thừa đầu đuôi
    int searchType = ui->comboSearch->currentIndex();   // 0: ID, 1: Title, 2: Artist, 3: Album

    // Nếu không nhập gì thì không tìm
    if (keyword.isEmpty()) {
        return;
    }

    // 3. Lấy TOÀN BỘ bài hát
    const auto& allSongs = player.getLibrary().getAllSongs();
    
    // Biến này có thể giữ lại nếu bạn muốn dùng để hiển thị label trạng thái (VD: "Found 5 songs")
    bool foundAny = false; 

    // 4. Duyệt và kiểm tra
    for (const auto& s : allSongs) {
        bool isMatch = false;

        QString qTitle = QString::fromStdString(s.title);
        QString qArtist = QString::fromStdString(s.artist);
        QString qAlbum = QString::fromStdString(s.album);

        switch (searchType) {
            case 0: // By ID
            {
                // Với Real-time search, ID thường cũng nên dùng contains (Partial Match)
                // Ví dụ gõ "1" sẽ ra ID 1, 10, 12... thay vì bắt buộc gõ đúng "10"
                if (QString::number(s.id) == keyword) {
                    isMatch = true;
                }
                break;
            }
            case 1: // By Title
            {
                if (qTitle.contains(keyword, Qt::CaseInsensitive)) {
                    isMatch = true;
                }
                break;
            }
            case 2: // By Artist
            {
                if (qArtist.contains(keyword, Qt::CaseInsensitive)) {
                    isMatch = true;
                }
                break;
            }
            case 3: // By Album
            {
                if (qAlbum.contains(keyword, Qt::CaseInsensitive)) {
                    isMatch = true;
                }
                break;
            }
        }

        // 5. Thêm vào list
        if (isMatch) {
            QString displayText = QString::number(s.id) + " - " + qTitle;
            
			// Logic hiển thị linh hoạt:
            if (searchType == 3) {
                // Nếu đang tìm theo ALBUM -> Phải hiện tên Album
                // Format: ID - Tên Bài (Ca Sĩ) - [Tên Album]
                displayText += " (" + qArtist + ") - [" + qAlbum + "]";
            }
            else {
                // Các trường hợp khác (ID, Tên, Ca sĩ) -> Hiện Ca sĩ là đủ
                displayText += "  -  " + qArtist;
            }
            QListWidgetItem* item = new QListWidgetItem(displayText, ui->listSearchResult);
           
            // Thêm Tooltip để nếu dài quá thì di chuột vào vẫn thấy hết
            item->setToolTip(displayText);
           
            item->setData(Qt::UserRole, s.id);
        }
    }
}

// -------------------------------------------------------------
// EVENT: Lưu trạng thái app khi đóng
// -------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *event) {
    // Trước khi đóng, gọi hàm lưu session
    logCurrentSongListening(); // Ghi nhận bài đang nghe dở trước khi tắt
    player.saveSession(m_currentSongID, m_lastPlayedQueueID, m_isPlayingFromPlayNext);
    
    // Chấp nhận đóng cửa sổ
    event->accept();
}

// -------------------------------------------------------------
// XỬ LÝ PLAY/PAUSE (LOGIC TOGGLE)
// -------------------------------------------------------------
void MainWindow::handlePlayPause() {
	if (!media) return;

	// Kiểm tra xem có bài nào đang được nạp không?
	// Nếu m_currentSongTitle rỗng (tức là đang "Ready to Play") -> Khóa nút Play
    if (m_currentSongTitle.isEmpty()) {
		ui->btnPlayPause->setChecked(false);
        return; 
    }

    if (media->state() == QMediaPlayer::PlayingState) {
        media->pause();
    } else {
        media->play();
    }
}

// -------------------------------------------------------------
// XỬ LÝ STOP: LOGIC DỪNG HẲN (QUAY LẠI ĐẦU BÀI)
// -------------------------------------------------------------
void MainWindow::handleStop() {
	logCurrentSongListening(); 
	if (!media) return;

	// Dừng nhạc (QMediaPlayer tự động reset file về 0s)
	media->stop();
}

// Hàm này tự động chạy bất cứ khi nào nhạc Bắt đầu hoặc Dừng
// (Dù là do bấm nút, do Double Click, hay do tự chuyển bài)
void MainWindow::onMediaStateChanged(QMediaPlayer::State state) {
    // Logic nút Play/Pause
    ui->btnPlayPause->setChecked(state == QMediaPlayer::PlayingState);
    // Logic hiển thị trạng thái
	if (state == QMediaPlayer::StoppedState) {
		// --- KHI DỪNG NHẠC ---
		
		// Reset thanh Slider về 0
		ui->sliderDuration->setValue(0);

		if (m_currentSongTitle.isEmpty()) {
			// TRƯỜNG HỢP 1: Chưa nạp bài nào (Mới mở App hoặc đã Clear)
            ui->labelCurrent->setText("Ready to Play");
            ui->labelDuration->setText("--:-- / --:--");
		} else {
			// TRƯỜNG HỢP 2: Đã có bài, nhưng đang Stop
            ui->labelCurrent->setText("Stopped: " + m_currentSongTitle);
            
            // Lấy tổng thời gian bài hát
            qint64 totalMs = media->duration();
            
            // Format thành: "00:00 / 04:35"
            QString timeStr = "00:00 / " + formatTime(totalMs);
            ui->labelDuration->setText(timeStr);
		}
	} else if (state == QMediaPlayer::PlayingState) {
		// Khi Play: Chỉ hiện tên bài (Thời gian sẽ do hàm Position lo)
        ui->labelCurrent->setText("Playing: " + m_currentSongTitle);
	} else if (state == QMediaPlayer::PausedState) {
        // Khi Pause: Hiện chữ Paused
        ui->labelCurrent->setText("Paused: " + m_currentSongTitle);
    }
}

// -------------------------------------------------------------
// HÀM CẬP NHẬT GIAO DIỆN HISTORY
// -------------------------------------------------------------
void MainWindow::refreshHistoryUI() {
    // Không cần tự vẽ listHistory nữa (vì đã xóa widget đó ở UI Main)
    
    // Gọi hàm refreshData của PersonalView
    // Hàm này sẽ tự gọi updateHistoryTable() bên trong nó
    if (m_personalView) {
        m_personalView->refreshData(); 
    }
}

// -------------------------------------------------------------
// HÀM: CẬP NHẬT GIAO DIỆN PLAY NEXT 
// -------------------------------------------------------------
void MainWindow::refreshPlayNextUI() {
	ui->listPlayNext->clear();
	const auto& queue = player.getPlayNext().getNextQueue();
	for (const auto& q : queue) {
		QListWidgetItem* item = new QListWidgetItem(ui->listPlayNext);
        QWidget* widget = new QWidget();
        widget->setStyleSheet("background-color: transparent;"); 
        
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(5, 0, 0, 0); 
        layout->setSpacing(10);

        QString displayText = QString::number(q.id) + " - " + QString::fromStdString(q.title)  + " - " + QString::fromStdString(q.artist);
        QLabel* lblName = new QLabel(displayText);
        
        lblName->setObjectName("lblCardTitle"); 
        
        lblName->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred); 

        layout->addWidget(lblName, 1, Qt::AlignVCenter); // Số 1: Label được co giãn

        item->setSizeHint(QSize(widget->sizeHint().width(), 36)); 
        item->setData(Qt::UserRole, q.id);

        ui->listPlayNext->setItemWidget(item, widget);
	}
}

// -------------------------------------------------------------
// HÀM TỔNG HỢP: CẬP NHẬT TOÀN BỘ GIAO DIỆN
// -------------------------------------------------------------
void MainWindow::refreshUI() {
	refreshPlaybackQueueUI();
    refreshHistoryUI();
    refreshPlayNextUI();
}

// -------------------------------------------------------------
// Hàm này để đổi trạng thái biến `isLoopEnabled` và đổi chữ trên nút.
// -------------------------------------------------------------
void MainWindow::handleLoopToggle() {
	// 1. Cập nhật biến logic từ trạng thái nút
    isLoopenabled = ui->btnLoop->isChecked();
	
	// 2. Debug hoặc xử lý logic playlist (nếu có)
    // Ví dụ: console log để kiểm tra
    if (isLoopenabled) { // LOOP: ON
        qDebug() << "Loop Mode: ON";
    } else {
        qDebug() << "Loop Mode: OFF";
    }
}

// -------------------------------------------------------------
// Hàm này quyết định xem sẽ phát lại bài cũ hay sang bài mới khi nhạc dừng.
// -------------------------------------------------------------
void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        if (isLoopenabled) {
            // Nếu đang Loop: Phát lại bài hiện tại
            media->setPosition(0); 
            media->play();
        } else {
            // Nếu không Loop: Chuyển bài tiếp theo
            handleNext(); 
        }
    }
}

// -------------------------------------------------------------
// Hàm xử lý chọn Playlist từ Playlist view
// -------------------------------------------------------------
void MainWindow::handlePlaylistSelected(const std::vector<int>& songIds, QString playlistName) {
    if (songIds.empty()) {
		showCustomDialog("Info", "This playlist is empty", QMessageBox::Information);
        return;
    }

    // 1. Kiểm tra trạng thái hàng đợi TRƯỚC KHI thêm
    bool wasEmpty = player.getPlayBack().empty();

    // 2. Gọi hàm Add playlist đã xử lí lọc trùng
    int addedCount = player.addPlaylistToQueue(songIds);

    // 3. Kiểm tra kết quả
    if (addedCount == 0) {
        showCustomDialog("Info", "No new songs added. All songs in this playlist are already in the queue", QMessageBox::Information);
		return;
    }

    // 4. Logic tự động phát nếu trước đó hàng đợi rỗng
    if (wasEmpty && !player.getPlayBack().empty()) {
        auto it = player.getPlayBack().getQueue().begin();
        player.initializePlaybackIterator(it);
        
        try {
			m_isPlayingFromPlayNext = false;
            m_lastPlayedQueueID = it->id;
            updateNowPlaying(*it);
            // media->play(); // Bỏ comment nếu muốn tự động phát nhạc luôn
        } catch (...) {}
    }

    // 5. Cập nhật giao diện
    refreshPlaybackQueueUI();
	showCustomDialog("Playlist Added", QString("Added %1 new songs from '%2' to Queue.").arg(addedCount).arg(playlistName), QMessageBox::Information);
}

// -------------------------------------------------------------
// Hàm xử lý lưu bài hát vào lịch sử
// -------------------------------------------------------------
void MainWindow::logCurrentSongListening() {
    try {
        auto currentSong = player.getPlayBack().getCurrentSong();
        
        // 1. Tính thời gian nghe
        int secondsListened = songStartTime.secsTo(QTime::currentTime());
        
        // 2. Ghi vào database thống kê (StatsManager)
        // Hàm này sẽ lưu vào file json/db
        statsManager.addSongPlayback(currentSong.id, secondsListened);
        
        // 3. Cập nhật giao diện Personal View ngay lập tức
        // Để người dùng qua tab Personal là thấy bài vừa nghe hiện lên luôn
        refreshHistoryUI(); 
        
        // Reset giờ
        songStartTime = QTime::currentTime();
    } catch (...) {
        // Không có bài đang hát
    }
}

void MainWindow::showStatistics() {
    // Lấy số phút nghe hôm nay
    int minsToday = statsManager.getListeningMinutes(QDate::currentDate(), 0);
    
    qDebug() << "Minutes listened today:" << minsToday;

    // Lấy Top Artist
    auto topArtists = statsManager.getTopArtists(player.getLibrary(), 5); 
    
    qDebug() << "--- TOP ARTISTS ---";
    for(const auto& p : topArtists) {
        qDebug() << QString::fromStdString(p.first) << " - Listens:" << p.second;
    }
} 

// -------------------------------------------------------------
// HÀM ADD SONG VÀO PLAY NEXT
// -------------------------------------------------------------
void MainWindow::handleAddSongToPlayNext(int songID) {
    bool success = player.addToPlayNext(songID);

    if (success) {
        refreshPlayNextUI();
        
        // Logic phát ngay nếu đang dừng
        if (media->state() == QMediaPlayer::StoppedState) {
            handleNext();
        } else {
            qDebug() << "Added to Play Next queue.";
        }
    } else {
		showCustomDialog("Info", "This song is already in a queue or not found", QMessageBox::Information);
    }
}

// -------------------------------------------------------------
// Hàm xóa trực tiếp bài hát trong Queue
// -------------------------------------------------------------
void MainWindow::handleRemoveSongDirectly(int songID) {
    // 1. Lấy ID bài đang hát (Dùng biến m_currentSongID của UI cho chính xác)
    int currentPlayingID = m_currentSongID;

    // 2. Xóa khỏi Backend
    player.getPlayBack().removeSong(songID);

    // 3. Cập nhật giao diện danh sách
    refreshPlaybackQueueUI();

    // 4. KIỂM TRA ĐIỀU KIỆN DỪNG NHẠC (QUAN TRỌNG)
    // Điều kiện: 
    //   a. ID bị xóa == ID đang hát (currentPlayingID)
    //   b. VÀ Bài đang hát KHÔNG PHẢI lấy từ Up Next (!m_isPlayingFromPlayNext)
    //      (Nghĩa là nó đang hát từ chính Playback Queue -> Xóa nó tức là xóa bài đang hát)
    
    if (songID == currentPlayingID && !m_isPlayingFromPlayNext) {
        
        // --- TRƯỜNG HỢP: XÓA ĐÚNG BÀI ĐANG HÁT TRONG QUEUE ---
        qDebug() << "Deleting currently playing song from Queue...";
        media->stop();

        // Logic chuyển bài: Ưu tiên Play Next -> Sau đó mới đến Playback
        if (!player.getPlayNext().empty()) {
            // Gọi handleNext để nó tự lo logic lấy bài từ Up Next
            handleNext(); 
        } 
        else if (!player.getPlayBack().empty()) {
            // Lấy bài kế tiếp trong Playback (iterator đã tự nhảy khi removeSong)
            try {
                auto newCurrent = player.getPlayBack().getCurrentSong();
                
                // Đánh dấu lại 
                m_isPlayingFromPlayNext = false; 
                
                updateNowPlaying(newCurrent);
                qDebug() << "Auto switched to next in Queue.";
            } catch (...) {
                handleStop();
            }
        } 
        else {
            // Hết sạch bài
            handleStop();

			media->setMedia(QMediaContent());

            ui->labelCurrent->setText("Ready to Play");
            ui->labelDuration->setText("--:-- / --:--");
            ui->sliderDuration->setValue(0);
            m_currentSongTitle = "";
        }
    } 
    else {
        // --- TRƯỜNG HỢP: XÓA BÀI NỀN (BACKGROUND DELETE) ---
        // Bao gồm:
        // 1. Xóa bài ID khác.
        // 2. Xóa bài ID trùng (ví dụ ID=10), NHƯNG máy đang hát bài ID=10 của nguồn Up Next.
        // -> Nhạc không bị ngắt quãng.
        qDebug() << "Deleted background song. Playback continues.";
    }
}

// -------------------------------------------------------------
// HÀM TIỆN ÍCH: HIỂN THỊ DIALOG ĐẸP + LÀM TỐI NỀN
// -------------------------------------------------------------
void MainWindow::showCustomDialog(const QString& title, const QString& content, QMessageBox::Icon icon) {
    // 1. Tạo Overlay (Màn đen)
    QWidget* overlay = new QWidget(this);
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 180);");
    overlay->show();

    // 2. Tạo MessageBox
    QMessageBox msgBox(this); // Phải có 'this' để nó kế thừa CSS từ MainWindow
    msgBox.setWindowTitle(title);
    msgBox.setText(content);
    msgBox.setIcon(icon);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    msgBox.exec();

    // 3. Xóa Overlay
    delete overlay;
}