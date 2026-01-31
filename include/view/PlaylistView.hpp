#ifndef PLAYLISTVIEW_HPP
#define PLAYLISTVIEW_HPP

#include <QWidget>
#include <QMessageBox>
#include "../../include/controller/MusicPlayer.hpp"

// Namespace UI
namespace Ui {
class PlaylistView;
}

class PlaylistView : public QWidget {
    Q_OBJECT

public:
    explicit PlaylistView(controller::MusicPlayer* player, QWidget *parent = nullptr);
    ~PlaylistView();

    void loadPlaylists();

signals:
    // Signal bắn ra khi chọn playlist để MainWindow biết mà phát nhạc
    void playlistClicked(const std::vector<int>& songIDs, QString playlistName);

private slots:
    void handleCreatePlaylist();

private:
    Ui::PlaylistView *ui; // Con trỏ UI
    controller::MusicPlayer* m_player;
    
    // Hàm tiện ích nội bộ
    void setupDialogStyle(QDialog* dialog); 

	// Hàm hiển thị thông báo có màn đen
	void showCustomDialog(const QString& title, const QString& content, QMessageBox::Icon icon);
};

#endif // PLAYLISTVIEW_HPP