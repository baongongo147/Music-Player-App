#include "../../include/controller/StatisticsManager.hpp"
#include <QDateTime>
#include <QDebug>
#include <QSaveFile> // Dùng cái này an toàn hơn QFile thường cho Oto

StatisticsManager::StatisticsManager() {
    loadSession();
}

StatisticsManager::~StatisticsManager() {
    saveSession();
}

// ---------------------------------------------------------
// LOGIC GHI NHẬN (CORE)
// ---------------------------------------------------------
void StatisticsManager::addSongPlayback(int songId, int durationSec) {
    // Nếu nghe quá ít (dưới 10s) thì bỏ qua, không tính rác
    if (durationSec < 10) return;

    PlayRecord rec;
    rec.songId = songId;
    rec.timestamp = QDateTime::currentSecsSinceEpoch();
    rec.durationSec = durationSec;

    // Thêm vào cuối
    m_records.push_back(rec);

    // QUAN TRỌNG: Cơ chế Rolling Buffer (Xoay vòng)
    // Nếu vượt quá giới hạn, xóa bớt các bài cũ nhất ở đầu
    if (m_records.size() > MAX_HISTORY_SIZE) {
        // Xóa phần tử đầu tiên (cũ nhất)
        m_records.erase(m_records.begin());
    }

    // Lưu ngay (hoặc có thể dùng timer để lưu định kỳ cho đỡ hại bộ nhớ)
    saveSession();
}

// ---------------------------------------------------------
// LOGIC FILE I/O (JSON)
// ---------------------------------------------------------
void StatisticsManager::saveSession() {
    QJsonArray arr;
    for (const auto& rec : m_records) {
        QJsonObject obj;
        obj["id"] = rec.songId;
        obj["ts"] = rec.timestamp; // Viết tắt cho gọn file
        obj["d"]  = rec.durationSec;
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    
    // Dùng QSaveFile: Nó ghi ra file tạm, ghi xong mới đổi tên đè lên file cũ
    // Rất an toàn cho hệ thống nhúng nếu lỡ mất điện giữa chừng
    QSaveFile file(FILE_PATH);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Compact)); // Compact để tiết kiệm dung lượng
        file.commit(); // Xác nhận ghi xong
    }
}

void StatisticsManager::loadSession() {
    QFile file(FILE_PATH);
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray arr = doc.array();

    m_records.clear();
    for (const auto& val : arr) {
        QJsonObject obj = val.toObject();
        PlayRecord rec;
        rec.songId = obj["id"].toInt();
        rec.timestamp = obj["ts"].toVariant().toLongLong();
        rec.durationSec = obj["d"].toInt();
        m_records.push_back(rec);
    }
    file.close();
}
// =========================================================
// LOGIC KIỂM TRA THỜI GIAN 
// =========================================================
bool StatisticsManager::isTimeInMillisInRange(qint64 timestamp, StatsPeriod period) const {
    // Nếu chọn ALL_TIME thì luôn đúng, không cần tính toán
    if (period == ALL_TIME) return true;

    QDateTime recordTime = QDateTime::fromSecsSinceEpoch(timestamp);
    QDate recordDate = recordTime.date();
    QDate today = QDate::currentDate();

    if (period == TODAY) {
        return recordDate == today;
    }
    // Yêu cầu 1: 3-7 ngày gần đây
    if (period == LAST_7_DAYS) {
        // Lấy ngày hiện tại trừ đi 6 ngày trước (tổng là 7 ngày)
        return recordDate >= today.addDays(-6); 
    }
    // Yêu cầu 3, 4: Tuần này (Từ Thứ 2 -> Chủ Nhật)
    if (period == THIS_WEEK) {
        // Tìm ngày Thứ 2 của tuần này
        // dayOfWeek(): 1=Mon, 7=Sun
        int currentDayOfWeek = today.dayOfWeek(); 
        // Tính ngày đầu tuần (Thứ 2)
        QDate monday = today.addDays(-(currentDayOfWeek - 1));
        // Tính ngày cuối tuần (Chủ nhật)
        QDate sunday = monday.addDays(6);

        return (recordDate >= monday && recordDate <= sunday);
    }

    return true;
}
// ---------------------------------------------------------
// LOGIC TÍNH TOÁN THỐNG KÊ
// ---------------------------------------------------------

// A. Gần đây (Duyệt ngược từ cuối về đầu)
std::vector<PlayRecord> StatisticsManager::getRecentlyPlayed(int limit) const {
    std::vector<PlayRecord> result;
    int count = 0;
    // Duyệt ngược (rbegin) để lấy bài mới nhất trước
    for (auto it = m_records.rbegin(); it != m_records.rend(); ++it) {
        result.push_back(*it);
        count++;
        if (count >= limit) break;
    }
    return result;
}

// B. Số phút nghe (Duyệt xuôi và lọc theo ngày)
int StatisticsManager::getListeningMinutes(const QDate& targetDate, int mode) const {
    int totalSeconds = 0;
    // 1. Duyệt qua tất cả record
    for (const auto& rec : m_records) {
        QDateTime dt = QDateTime::fromSecsSinceEpoch(rec.timestamp);
        QDate recDate = dt.date();

        bool match = false;
        if (mode == 0) { // Theo ngày
            if (recDate == targetDate) match = true;
        } else { // Theo tháng (Cùng tháng và cùng năm)
            if (recDate.month() == targetDate.month() && recDate.year() == targetDate.year()) {
                match = true;
            }
        }

        if (match) {
            totalSeconds += rec.durationSec;
        }
    }
    return totalSeconds / 60; // Trả về phút
}

// C. Top Bài Hát (Dùng Map để đếm)
std::vector<std::pair<int, int>> StatisticsManager::getTopSongs(int limit, StatsPeriod period) const {
    std::map<int, int> counts; 
    // 1. Duyệt qua tất cả record
    for (const auto& rec : m_records) {
        // 2. Kiểm tra thời gian (Nếu thuộc tuần này/7 ngày qua thì mới đếm)
        if (isTimeInMillisInRange(rec.timestamp, period)) {
                counts[rec.songId]++; 
            }
        }

    // 3. Chuyển map sang vector để sắp xếp
    std::vector<std::pair<int, int>> sortedList(counts.begin(), counts.end());
    std::sort(sortedList.begin(), sortedList.end(), 
        [](const auto& a, const auto& b) { return a.second > b.second; }
    );

    if (sortedList.size() > (size_t)limit) sortedList.resize(limit);
    return sortedList;
}


// D. Top Nghệ Sĩ (Cần tra cứu Library)
std::vector<std::pair<std::string, int>> StatisticsManager::getTopArtists( models::MusicLibrary& lib, int limit,StatsPeriod period) const {
    std::map<std::string, int> artistCounts;

    for (const auto& rec : m_records) {
        // 1. Kiểm tra thời gian trước
        if (isTimeInMillisInRange(rec.timestamp, period)) {
            // 2. Tra cứu ID bài hát sang thông tin Ca sĩ
                models::Song* song = lib.findSongByID(rec.songId);
                if (song) {
            artistCounts[song->artist]++;
            }
        }
    }

    // Các bước sort tương tự như Top Song
    std::vector<std::pair<std::string, int>> sortedList(artistCounts.begin(), artistCounts.end());
    
    std::sort(sortedList.begin(), sortedList.end(), 
        [](const auto& a, const auto& b) { return a.second > b.second; }
    );

    if (sortedList.size() > (size_t)limit) sortedList.resize(limit);
    
    return sortedList;
}