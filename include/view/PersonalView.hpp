#pragma once
#include <QWidget>
#include "../../include/controller/MusicPlayer.hpp"
#include "../../include/controller/StatisticsManager.hpp" 
namespace Ui {
class PersonalView;
}

class PersonalView : public QWidget {
    Q_OBJECT

public:
    explicit PersonalView(controller::MusicPlayer* player, StatisticsManager* stats, QWidget *parent = nullptr);
    ~PersonalView();

    // Hàm quan trọng nhất: Gọi hàm này mỗi khi muốn cập nhật số liệu
    void refreshData();

private:
    Ui::PersonalView *ui;
    controller::MusicPlayer* m_player;
    StatisticsManager* m_stats;
    // Helper functions (tùy chọn)
    void updateStatistics();
    void updateHistoryTable();
    void updateTopLists();
};