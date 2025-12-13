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
      ui(new Ui::MainWindow), // Khởi tạo giao diện
      player(songFile),
      media(new QMediaPlayer(this)),
	  isLoopenabled(false), // Khởi tạo mặc định là Không LOOP
      m_lastVolume(50), // Âm lượng mặc định là 50%
	  m_isMuted(false) // Mặc định là có tiếng
{
	// Vẽ giao diện từ Designer lên cửa sổ
    ui->setupUi(this); 

    // --- THÊM ĐOẠN NÀY ĐỂ CHIA 3 CỘT ĐỀU NHAU ---
    // setStretch(index, factor) -> index 0, 1, 2 đều có tỷ lệ 1
    ui->homeLayout->setStretch(0, 1);
    ui->homeLayout->setStretch(1, 1);
    ui->homeLayout->setStretch(2, 1);

	// --- CẤU HÌNH STACKED WIDGET (SIDEBAR) ---
    
    // 1. Khởi tạo các trang con
    m_playlistView = new PlaylistView(&player,this);
    m_personalView = new PersonalView(&player, &statsManager, this); // Truyền &player vào để lấy thống kê

    // 2. Thêm vào StackedWidget (vào chỗ pagePlaylist và pagePersonal đã khai báo trong XML)
    // Lưu ý: ui->mainStack đã có sẵn pageHome (index 0), pagePlaylist (index 1), pagePersonal (index 2)
    // Ta thay thế widget placeholder bằng widget thật của ta
    
    // Xóa widget rỗng cũ trong XML đi
    ui->mainStack->removeWidget(ui->pagePlaylist);
    ui->mainStack->removeWidget(ui->pagePersonal);

    // Thêm widget code tay vào
    ui->mainStack->addWidget(m_playlistView); // Index 1
    ui->mainStack->addWidget(m_personalView); // Index 2

    // 3. Kết nối nút bấm Sidebar với Stack
    connect(ui->btnNavHome, &QPushButton::clicked, [this](){
        ui->mainStack->setCurrentIndex(0); // Về trang Home
    });

    connect(ui->btnNavPlaylist, &QPushButton::clicked, [this](){
        ui->mainStack->setCurrentIndex(1); // Sang Playlist
    });

    connect(ui->btnNavPersonal, &QPushButton::clicked, [this](){
        m_personalView->refreshData(); // Cập nhật số liệu mới nhất
        ui->mainStack->setCurrentIndex(2); // Sang Personal
    });

    // Mặc định vào Home
    ui->mainStack->setCurrentIndex(0);





	// chức năng âm lượng
    ui->horizontalSlider->setRange(0, 100);

    // Khi bấm nút Loa -> Gọi hàm on_btnVolume_clicked
    connect(ui->btnVolume, &QPushButton::clicked, this, &MainWindow::on_btnVolume_clicked);

    // Load dữ liệu bài hát vào listPlayback
    loadLibraryToUI();

	// Đặt giá trị mặc định (ví dụ 50%)
    ui->horizontalSlider->setValue(50);
    if(media) {
        media->setVolume(50); 
    }

    // KẾT NỐI TÍN HIỆU (Signal) & KHE (Slot)
    // Cú pháp: connect(ui->Tên_Widget_Trong_Designer, ...)

    // Nút Next / Previous / Shuffle
    connect(ui->btnNext, &QPushButton::clicked, this, &MainWindow::handleNext);
    connect(ui->btnPrevious, &QPushButton::clicked, this, &MainWindow::handlePrev);
    connect(ui->btnShuffle, &QPushButton::clicked, this, &MainWindow::handleShuffle);

    // Nút Play by ID (Nhập ID rồi bấm Play)
    // connect(ui->btnPlayID, &QPushButton::clicked, this, &MainWindow::handlePlayByID);

    // Nút Smart Playlist (Nhập ID, Size rồi bấm Create)
    connect(ui->btnSmart, &QPushButton::clicked, this, &MainWindow::handleSmartPlaylist);

    // Sự kiện click đúp vào bài hát trong listPlayback
    connect(ui->listPlayback, &QListWidget::itemDoubleClicked, this, &MainWindow::handleSongDoubleClick);

    // Sự kiện click đúp vào bài hát trong listSearchResult,listAllSongs
    connect(ui->listSearchResult, &QListWidget::itemDoubleClicked, this, &MainWindow::handleSongDoubleClick);

	// Sự kiện click đúp vào All Songs Library (Danh sách thư viện)
    // Lưu ý: ui->listAllSongs là tên widget ta đã thêm trong file XML ở bước trước
    connect(ui->listAllSongs, &QListWidget::itemDoubleClicked, this, &MainWindow::handleLibraryDoubleClick);

	// Kết nối tín hiệu kéo thanh trượt với hàm xử lý
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::handleVolumeChanged);

	// Kết nối Slider Duration
    // 1. Khi độ dài bài hát thay đổi -> Set Range cho slider
    connect(media, &QMediaPlayer::durationChanged, this, &MainWindow::handleDurationChanged);
    // 2. Khi bài hát đang chạy (mỗi giây) -> Cập nhật vị trí slider tự động
    connect(media, &QMediaPlayer::positionChanged, this, &MainWindow::handlePositionChanged);
	// 3. Khi người dùng kéo slider -> tua nhạc (seek)
	// sliderMoved: Gọi khi đang kéo (để tua mượt mà)
    connect(ui->sliderDuration, &QSlider::sliderMoved, this, &MainWindow::handleSeek);
    // sliderReleased: Gọi khi thả chuột ra (để chốt vị trí cuối cùng)
    connect(ui->sliderDuration, &QSlider::sliderReleased, [this]() {
        handleSeek(ui->sliderDuration->value());
    });

    // Xử lý khi click đúp vào kết quả tìm kiếm (để phát nhạc)
    connect(ui->listSearchResult, &QListWidget::itemDoubleClicked, this, &MainWindow::handleSongDoubleClick);
    // Kết nối sự kiện thay đổi text với hàm xử lý tìm kiếm
    connect(ui->lineSearch, &QLineEdit::textChanged, this, &MainWindow::handleSearch);
    
    // Kết nối cho nút Delete 
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::handleDeleteFromQueue);
    // Nếu đang gõ trong ô ID Delete mà ấn Enter thì cũng xóa luôn
    connect(ui->lineIDdelete, &QLineEdit::returnPressed, this, &MainWindow::handleDeleteFromQueue);

	// Nút Play/Pause
	connect(ui->btnPlayPause, &QPushButton::clicked, this, &MainWindow::handlePlayPause);
	connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::handleStop);

	// GIẢI QUYẾT VẤN ĐỀ ĐỒNG BỘ NÚT BẤM (Cho Double Click, Next, Prev...)
    // Khi trạng thái media thay đổi (Playing <-> Stopped), tự động đổi chữ trên nút
    connect(media, &QMediaPlayer::stateChanged, this, &MainWindow::onMediaStateChanged);

	// Kết nối cho nút loop
    connect(ui->btnLoop, &QPushButton::clicked, this, &MainWindow::handleLoopToggle);
 
    // Xử lý sự kiện khi bài hát kết thúc
    connect(media, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::onMediaStatusChanged);

	// Kết nối tín hiệu từ Playlist View
    connect(m_playlistView, &PlaylistView::playlistClicked, this, &MainWindow::handlePlaylistSelected);

    // --- LOGIC KHÔI PHỤC SESSION ---
    QFile sessionFile("session.json");
    if (sessionFile.exists()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Restore Session");
        msgBox.setText("Do you want to continue from the last session?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setIcon(QMessageBox::Question);

        // 2. Ép Style Dark Mode, Viền Xanh, Nút To
        msgBox.setStyleSheet(
            "QMessageBox {"
            "   background-color: #202020;"  /* Nền tối (sáng hơn nền app 1 xíu để nổi bật) */
            "   border: 2px solid #1DB954;"   /* VIỀN XANH LÁ RÕ RÀNG */
            "   border-radius: 10px;"         /* Bo tròn góc */
            "}"
            "QLabel {"
            "   color: #FFFFFF;"              /* Chữ trắng */
            "   font-size: 14px;"             /* Chữ to hơn mặc định */
            "   font-weight: 500;"
            "   font-family: 'Segoe UI';"
            "   margin-bottom: 10px;"         /* Dãn cách giữa chữ và nút */
            "   margin-right: 15px;"          /* Dãn cách icon và chữ */
            "}"
            "QPushButton {"
            "   background-color: #333333;"   /* Nền nút xám */
            "   color: white;"
            "   border: 1px solid #444;"
            "   border-radius: 6px;"
            "   padding: 8px 25px;"           /* NÚT TO, DÃN CÁCH RỘNG */
            "   font-size: 13px;"
            "   min-width: 80px;"             /* Chiều rộng tối thiểu cho đẹp */
            "}"
            "QPushButton:hover {"
            "   background-color: #1DB954;"   /* Hover vào thành màu xanh */
            "   color: black;"                /* Chữ chuyển đen cho dễ đọc */
            "   border: none;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #1ed760;"
            "}"
        );

        // 3. Hiện hộp thoại và lấy kết quả
        int reply = msgBox.exec();
        
        if (reply == QMessageBox::Yes) {
            // Thực hiện load lại dữ liệu
            player.restoreSession();
            
            // Vẽ lại giao diện sau khi load dữ liệu ngầm
            refreshPlaybackQueueUI(); 
            refreshHistoryUI();
			m_playlistView->loadPlaylists();
            
			// khôi phục volume
            int savedVol = player.getVolume();
            ui->horizontalSlider->setValue(savedVol); // Set vị trí thanh trượt
            if(media) media->setVolume(savedVol); // Set âm lượng thực tế

            // Cập nhật bài đang hát lên Label
            try {
                auto current = player.getPlayBack().getCurrentSong();
                updateNowPlaying(current);
                // Nếu muốn auto play luôn thì gọi: media->play();
            } catch(...) {}
        } else {
            // Nếu chọn No -> Xóa file session cũ đi để làm mới
            sessionFile.remove();
			player.setVolume(50);
        }
    } else {
		// Nếu không có file session, set mặc định 50 vào player
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
        QString text = QString::number(s.id) + " - " + QString::fromStdString(s.title);
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
            // "QPushButton:pressed { background-color: #1ed760; }" ////////////////////////////////////////////////////////////////
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

        QString displayText = QString::number(index) + " - " + QString::fromStdString(s.title);
        QLabel* lblName = new QLabel(displayText);

        // [QUAN TRỌNG]
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
    m_currentSongTitle = QString::fromStdString(s.title);
    ui->labelCurrent->setText("Current Song: " + m_currentSongTitle);
    playAudioFile(s.filePath);
}

// -------------------------------------------------------------
// EVENT: PLAY BY ID
// -------------------------------------------------------------
void MainWindow::handlePlayByID() {
    // Lấy text từ ô nhập lineSongID
    QString textID = ui->lineSongID->text();
    bool ok;
    int id = textID.toInt(&ok);

    if (!ok) {
        showCustomDialog("Input Error", "Please enter a valid Song ID (number).", QMessageBox::Warning);
        return;
    }

    try {
        player.selectAndPlaySong(id);
        auto current = player.getPlayBack().getCurrentSong();
        updateNowPlaying(current);
		refreshUI();
    } catch (...) {
        showCustomDialog("Error", "Song ID not found!", QMessageBox::Warning);
    }
}

// -------------------------------------------------------------
// EVENT: NEXT
// -------------------------------------------------------------
void MainWindow::handleNext() {
    models::Song s = player.playNextSongLogic();

    if (s.id != 0) {
        // Có bài -> Phát
        // Log thống kê bài cũ TRƯỚC KHI phát bài mới
        logCurrentSongListening(); 
        songStartTime = QTime::currentTime();

        updateNowPlaying(s);
        refreshPlayNextUI();      // Cập nhật nếu vừa pop PlayNext
        refreshPlaybackQueueUI(); // Cập nhật nếu vừa playNext main queue
        
        qDebug() << "Playing: " << QString::fromStdString(s.title);
    } else {
        // Hết bài -> Stop
        m_currentSongTitle = ""; 
        media->stop();
        ui->labelCurrent->setText("Ready to Play");
        ui->labelDuration->setText("--:-- / --:--");
        ui->sliderDuration->setValue(0);
        qDebug() << "Queue finished.";
    }
}

// -------------------------------------------------------------
// EVENT: PREVIOUS
// -------------------------------------------------------------
void MainWindow::handlePrev() {
	logCurrentSongListening();
    songStartTime = QTime::currentTime();
    auto histVec = player.getHistory().getHistory();
    if (histVec.empty()) {
        showCustomDialog("Info", "No history available.", QMessageBox::Information);
        return;
    }
    try {
        auto s = player.getHistory().playPreviousSong();
        player.selectAndPlaySong(s.id);
        player.getHistory().removeLastAddedSong();
        updateNowPlaying(s);
		refreshUI();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", e.what());
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
// EVENT: SMART PLAYLIST (Logic Mới lấy từ UI)
// -------------------------------------------------------------
void MainWindow::handleSmartPlaylist() {
    // 1. Lấy Start ID từ lineSmartID
    QString startStr = ui->lineSmartID->text();
    bool ok1;
    int startID = startStr.toInt(&ok1);

    // 2. Lấy Size từ lineSmartSize
    QString sizeStr = ui->lineSmartSize->text();
    bool ok2;
    int size = sizeStr.toInt(&ok2);

    if (!ok1 || !ok2) {
        QMessageBox::warning(this, "Input Error", "Please enter valid numbers for ID and Size.");
        return;
    }

    // 3. Gọi logic tạo playlist
    auto smartQ = player.createSmartPlaylist(startID, size);

    if (smartQ.size() == 0) {
        QMessageBox::information(this, "Info", "Could not generate Smart Playlist (check ID range).");
        return;
    }

	// Kiểm tra xem hàng đợi có đang rỗng trước khi thêm không
    bool wasEmpty = player.getPlayBack().empty();

    // 4. Đẩy vào playback queue
    for (const auto& s : smartQ.getQueue()) {
        player.getPlayBack().addSong(s);
    }

    // 5. Nếu queue đang rỗng thì init iterator
    if (wasEmpty && !player.getPlayBack().empty()) {
        auto it = player.getPlayBack().getQueue().begin();
        player.initializePlaybackIterator(it);
        
        // (Tùy chọn) Cập nhật bài hiện tại lên Label ngay lập tức
        try {
             updateNowPlaying(*it);
        } catch(...) {}
    }
    refreshPlaybackQueueUI();
    QMessageBox::information(this, "Success", "Smart Playlist added to queue!");
}

// -------------------------------------------------------------
// EVENT: DOUBLE CLICK ITEM
// -------------------------------------------------------------
void MainWindow::handleSongDoubleClick(QListWidgetItem* item) {
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    logCurrentSongListening();          // 1. Lưu thống kê bài cũ
    songStartTime = QTime::currentTime(); // 2. Reset giờ bắt đầu cho bài mới
    player.selectAndPlaySong(id);
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
            QMessageBox::information(this, "Notification", "This song is already in the playback queue.");
            return; // Thoát hàm ngay lập tức
        }
    }

    // 3. Nếu không trùng -> Logic: Thêm vào cuối Playback Queue và phát ngay lập tức
    // Hoặc bạn có thể chỉ play mà không add, tuỳ logic controller của bạn.
    // Ở đây ta dùng selectAndPlaySong như logic Play By ID
    try {
        models::Song* foundSong = player.getLibrary().findSongByID(id);
		if (foundSong) {
			// Thêm vào backend (hàm này đã tự xử lý việc set currentSong nếu list rỗng)
			player.getPlayBack().addSong(*foundSong);
			
			// Vẽ lại danh sách hiển thị
			refreshPlaybackQueueUI();  

			if (wasEmpty) {
				// Nếu danh sách đang rỗng thì phát luôn
                songStartTime = QTime::currentTime(); 
				player.getPlayBack().setCurrentSongByID(foundSong->id);
				updateNowPlaying(*foundSong);
			}
		}

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", "Could not play song from library.");
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
        
        // [FIX LỖI RANDOM GIÁ TRỊ NHỎ]
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
                // Tùy bạn chọn, ở đây mình giữ nguyên logic cũ của bạn (Exact Match)
                if (QString::number(s.id) == keyword) {
                    isMatch = true;
                }
                
                // GỢI Ý: Nếu muốn tìm ID kiểu 'gần đúng' luôn thì dùng dòng dưới:
                // if (QString::number(s.id).contains(keyword)) isMatch = true;
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
            QString displayText = QString::number(s.id) + " - " + qTitle + " (" + qArtist + ")";
            QListWidgetItem* item = new QListWidgetItem(displayText, ui->listSearchResult);
            item->setData(Qt::UserRole, s.id);
            foundAny = true;
        }
    }
}

// -------------------------------------------------------------
// EVENT: Xóa bài hát từ Playback Queue
// -------------------------------------------------------------
void MainWindow::handleDeleteFromQueue() {
    // 1. Lấy text từ ô nhập liệu
    QString textIndex = ui->lineIDdelete->text();
    bool ok;
    int visualIndex = textIndex.toInt(&ok);

    // 2. Kiểm tra dữ liệu nhập vào
    if (!ok || visualIndex < 1) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid Song ID (number).");
        return;
    }

    // 3. Truy cập vào Queue để tìm ID gốc tại vị trí đó
    // Lưu ý: PlaybackQueue trả về reference, nhưng ta cần iterator để duyệt
    const auto& queue = player.getPlayBack().getQueue();

    // Kiểm tra nếu số nhập vào lớn hơn độ dài danh sách
    if (visualIndex > queue.size()) {
        QMessageBox::warning(this, "Error", "Index out of range.");
        return;
    }
    // 4. Tìm bài hát tại vị trí (visualIndex - 1)
    // Vì danh sách hiển thị từ 1, nhưng mảng/list tính từ 0
    auto it = queue.begin();
    std::advance(it, visualIndex - 1); // Di chuyển con trỏ đến đúng bài đó

    int originalID = it->id; // Đây là ID gốc
    // 5. Gọi hàm xóa từ Backend
    // Lưu ý: player.getPlayBack() trả về tham chiếu đến đối tượng PlaybackQueue
    player.getPlayBack().removeSong(originalID);

    // 6. CẬP NHẬT GIAO DIỆN 
    
    // a. Vẽ lại danh sách Playback Queue (để dòng bị xóa biến mất khỏi list)
    refreshPlaybackQueueUI(); 

    // b. Xử lý trường hợp bài đang hát bị xóa
    // Vì logic removeSong của bạn đã tự động chuyển `currentSong` sang bài kế tiếp (next),
    // nên ta cần cập nhật lại nhãn "Current Song" và phát bài mới đó.
    if (player.getPlayBack().empty()) {
        // Nếu xóa hết sạch thì dừng nhạc và reset giao diện
        media->stop();
        ui->labelCurrent->setText("Ready to play");
        ui->labelDuration->setText("--:-- / --:--");
        ui->sliderDuration->setValue(0);
    } else {
        // Nếu còn bài, cập nhật thông tin bài hiện tại mới (do con trỏ đã dịch chuyển)
        try {
            auto current = player.getPlayBack().getCurrentSong();
            updateNowPlaying(current); // Hàm này sẽ hiển thị tên bài mới và play audio
        } catch (...) {
            // Trường hợp xóa xong con trỏ current bị lỗi 
        }
    }

    // 7. Xóa trắng ô nhập liệu để nhập cái tiếp theo cho nhanh
    ui->lineIDdelete->clear();
    
    // (Tùy chọn) Focus lại vào ô nhập để nhập tiếp không cần click chuột
    ui->lineIDdelete->setFocus();
}

// -------------------------------------------------------------
// EVENT: Lưu trạng thái app khi đóng
// -------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *event) {
    // Trước khi đóng, gọi hàm lưu session
    logCurrentSongListening(); // Ghi nhận bài đang nghe dở trước khi tắt
    player.saveSession();
    
    // Chấp nhận đóng cửa sổ
    event->accept();
}

// -------------------------------------------------------------
// XỬ LÝ PLAY/PAUSE (LOGIC TOGGLE)
// -------------------------------------------------------------
void MainWindow::handlePlayPause() {
	if (!media) return;
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
// HÀM CẬP NHẬT GIAO DIỆN HISTORY ////////////////////////////////////////////////////////////////////////////////////
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

        QString displayText = QString::number(q.id) + " - " + QString::fromStdString(q.title);
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
        QMessageBox::information(this, "Info", "This playlist is empty.");
        return;
    }

    // 1. Kiểm tra trạng thái hàng đợi TRƯỚC KHI thêm
    bool wasEmpty = player.getPlayBack().empty();

    // 2. Gọi hàm Add playlist đã xử lí lọc trùng
    int addedCount = player.addPlaylistToQueue(songIds);

    // 3. Kiểm tra kết quả
    if (addedCount == 0) {
        QMessageBox::information(this, "Info", "No new songs added. All songs in this playlist are already in the queue.");
        return;
    }

    // 4. Logic tự động phát nếu trước đó hàng đợi rỗng
    if (wasEmpty && !player.getPlayBack().empty()) {
        auto it = player.getPlayBack().getQueue().begin();
        player.initializePlaybackIterator(it);
        
        try {
            updateNowPlaying(*it);
            // media->play(); // Bỏ comment nếu muốn tự động phát nhạc luôn
        } catch (...) {}
    }

    // 5. Cập nhật giao diện
    refreshPlaybackQueueUI();
    
    QMessageBox::information(this, "Playlist Added", QString("Added %1 new songs from '%2' to Queue.").arg(addedCount).arg(playlistName));
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
    
    // Thay vì ui->labelMinsToday->setText(...), ta dùng qDebug:
    qDebug() << "Minutes listened today:" << minsToday;

    // Lấy Top Artist
    // Lưu ý: Đã xóa const ở tham số lib như hướng dẫn trước
    auto topArtists = statsManager.getTopArtists(player.getLibrary(), 5); 
    
    qDebug() << "--- TOP ARTISTS ---";
    for(const auto& p : topArtists) {
        // Thay vì ui->listTopArtist->addItem(...), ta dùng qDebug:
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
        QMessageBox::information(this, "Info", "This song is already in a queue or not found.");
    }
}

// -------------------------------------------------------------
// Hàm xóa trực tiếp bài hát trong Queue
// -------------------------------------------------------------
void MainWindow::handleRemoveSongDirectly(int songID) {
    // 1. Kiểm tra xem có phải xóa bài đang hiển thị trên Label không (Logic View)
    // Lưu ý: Logic này ở View vẫn cần để biết có nên Stop nhạc hay không
    // Tuy nhiên, việc "tìm bài tiếp theo" thì để Controller lo.
    
    // Gọi Controller xóa và lấy bài tiếp theo (nếu cần thiết)
    models::Song nextSong = player.removeSong(songID);

    // Cập nhật danh sách
    refreshPlaybackQueueUI();

    // Kiểm tra kết quả
    if (nextSong.id != 0) {
        // Controller trả về 1 bài hát.
        // Kiểm tra xem bài này có khác bài đang hát không?
        if (m_currentSongTitle != QString::fromStdString(nextSong.title)) {
             // Khác -> Nghĩa là bài đang hát đã bị xóa, đây là bài mới
             updateNowPlaying(nextSong);
             qDebug() << "Auto switched to: " << QString::fromStdString(nextSong.title);
        }
        // Nếu giống -> Nghĩa là xóa bài khác, nhạc vẫn chạy tiếp, không làm gì cả.
    } 
    else {
        // Controller trả về rỗng -> Có thể là xóa hết sạch rồi
        // Hoặc xóa bài đang hát và không còn bài nào nữa
        // Ta kiểm tra xem hàng đợi còn không để chắc chắn
        if (player.getPlayBack().empty() && player.getPlayNext().empty()) {
             handleStop();
             ui->labelCurrent->setText("Ready to Play");
             ui->labelDuration->setText("--:-- / --:--");
             ui->sliderDuration->setValue(0);
             m_currentSongTitle = "";
        }
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
    QMessageBox msgBox(this); // Quan trọng: Phải có 'this' để nó kế thừa CSS từ MainWindow
    msgBox.setWindowTitle(title);
    msgBox.setText(content);
    msgBox.setIcon(icon);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    // KHÔNG CẦN setStyleSheet() Ở ĐÂY NỮA
    // Vì nó sẽ tự động nhận style từ XML (Mục 12 vừa thêm)

    msgBox.exec();

    // 3. Xóa Overlay
    delete overlay;
}