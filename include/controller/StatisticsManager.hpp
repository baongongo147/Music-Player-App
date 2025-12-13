#pragma once
#include <vector>
#include <map>
#include <string>
#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <algorithm>
#include "../model/Song.hpp" 
#include "../model/MusicLibrary.hpp"

// Cấu trúc bản ghi: Bài nào, Nghe lúc nào, Nghe bao lâu
struct PlayRecord {
    int songId;
    qint64 timestamp;
    int durationSec;
};

// Enum chọn khoảng thời gian
enum StatsPeriod {
    ALL_TIME,       // Tất cả
    LAST_7_DAYS,    // 7 ngày qua (Yêu cầu 1)
    THIS_WEEK,      // Tuần này (Yêu cầu 3, 4)
    TODAY           // Hôm nay
};

class StatisticsManager {
private:
    std::vector<PlayRecord> m_records;
    const QString FILE_PATH = "usage_history.json";
    const size_t MAX_HISTORY_SIZE = 1000;

    void loadSession();
    
    // Hàm kiểm tra thời gian (Core logic cho việc lọc)
    bool isTimeInMillisInRange(qint64 timestamp, StatsPeriod period) const;

public:
    StatisticsManager();
    ~StatisticsManager();

    // Ghi nhận lịch sử
    void addSongPlayback(int songId, int durationSec);
    void saveSession();

    // --- CÁC TÍNH NĂNG HIỂN THỊ ---

    // A. Lịch sử nghe (Timeline): Dùng cho bảng History
    // Trả về danh sách ID bài hát, mới nhất nằm đầu
    std::vector<PlayRecord> getRecentlyPlayed(int limit = 50) const;

    // B. Thống kê thời gian (Yêu cầu 2)
    int getListeningMinutes(const QDate& date, int mode) const;

    // C. Top Bài Hát (Yêu cầu 1 & 3)
    // Trả về: <ID Bài Hát, Số lần nghe>
    std::vector<std::pair<int, int>> getTopSongs(int limit = 10, StatsPeriod period = ALL_TIME) const;

    // D. Top Nghệ Sĩ (Yêu cầu 4)
    // Trả về: <Tên Nghệ Sĩ, Số lần nghe>
    std::vector<std::pair<std::string, int>> getTopArtists(models::MusicLibrary& lib, int limit = 5, StatsPeriod period = ALL_TIME) const;
};