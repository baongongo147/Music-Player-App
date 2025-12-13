#pragma once
#include "../model/Song.hpp"      
#include "../controller/MusicPlayer.hpp" 
#include "PlaylistView.hpp"
#include "PersonalView.hpp"

#include <QMainWindow>
#include <QMediaPlayer>
#include <QListWidgetItem> 
#include <QMessageBox> 
#include <QUrl>
#include <QDebug>
#include <QTime>
#include <QCloseEvent> 

// Khai báo Namespace Ui
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


// --------------------------------------------

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const std::string& songFile, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleNext();
    void handlePrev();
    void handleShuffle();
    void handlePlayByID();        
    void handleSmartPlaylist();   
    void handleSongDoubleClick(QListWidgetItem* item); 
	void handleVolumeChanged(int value);
	void handleSearch();
    // xóa khỏi back queue
    void handleDeleteFromQueue();
	// Slot xử lý click vào danh sách thư viện (Library) 
    void handleLibraryDoubleClick(QListWidgetItem* item);
	// Slots xử lý Progress Bar 
	// Xử lý khi biết tổng thời gian bài hát
    void handleDurationChanged(qint64 duration); 
	// Xử lý khi bài hát đang chạy
    void handlePositionChanged(qint64 position); 
    // Override sự kiện đóng cửa sổ
    void closeEvent(QCloseEvent *event) override;
	// Hàm xử lý Play/Pause
	void handlePlayPause(); 
	// Hàm xử lí Stop
	void handleStop(); 
    // Kiểm tra trạng thái Play/Pause
	void onMediaStateChanged(QMediaPlayer::State state); 
	// Hàm bật tắt Loop
    void handleLoopToggle();
    // Hàm xử lý khi hết
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
	// Xử lý tua nhạc
    void handleSeek(int position); 
	// Xử lý khi chọn Playlist
    void handlePlaylistSelected(const std::vector<int>& songIds, QString playlistName);
    // Xử lý khi bấm nút Loa
	void on_btnVolume_clicked();
	void logCurrentSongListening();
    void showStatistics();
	// Thêm vào Play Next
	void handleAddSongToPlayNext(int songID);
    // Hàm xóa trực tiếp bài hát trong Queue
	void handleRemoveSongDirectly(int songID);
private:
    // Biến đệm (Cache)
	int m_lastVolume; 
	// Trạng thái Mute hiện tại
    bool m_isMuted; 
	// Biến lưu trạng thái Loop
	bool isLoopenabled; 
    Ui::MainWindow *ui; 
    controller::MusicPlayer player; 
    QMediaPlayer* media;
	// Biến lưu tên bài hát hiện tại để hiển thị ổn định
    QString m_currentSongTitle;
	PlaylistView* m_playlistView;
    PersonalView* m_personalView;
	StatisticsManager statsManager; // Instance quản lý thống kê
    QTime songStartTime; // Để đo thời gian nghe thực tế

    void loadLibraryToUI();
    void updateNowPlaying(const models::Song& s);
    void playAudioFile(const std::string& path);
    void refreshPlaybackQueueUI(); 
    void refreshHistoryUI();
	void refreshPlayNextUI();
	// Hàm cập nhật giao diện tổng hợp
    void refreshUI();
    // Hàm hiển thị thông báo đẹp + làm tối nền
    void showCustomDialog(const QString& title, const QString& content, QMessageBox::Icon icon);
};

