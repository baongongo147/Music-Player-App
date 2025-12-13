#pragma once
#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton> 
#include "../../include/controller/MusicPlayer.hpp"

class PlaylistView : public QWidget {
    Q_OBJECT
public:
    explicit PlaylistView(controller::MusicPlayer* player, QWidget *parent = nullptr);
	void loadPlaylists();
signals:
    // Tín hiệu bắn ra khi người dùng chọn playlist, gửi kèm danh sách ID bài hát
    void playlistClicked(const std::vector<int>& songIds, QString playlistName);
private slots:
    void onItemClicked(QListWidgetItem* item);
    void handleCreatePlaylist(); 
private:
    controller::MusicPlayer* m_player;
    QListWidget* m_listWidget;
	QPushButton* m_btnCreate;
};