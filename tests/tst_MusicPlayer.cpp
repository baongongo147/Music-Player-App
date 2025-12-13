#include <gtest/gtest.h>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <vector>

// Include Controller & Models
#include "../include/controller/MusicPlayer.hpp"

// Cấu hình môi trường giả lập
const QString TEST_DB_FILE = "test_songs.txt";
const QString TEST_MEDIA_DIR = "media";

class MusicPlayerTest : public ::testing::Test {
protected:
    controller::MusicPlayer* player;

    // --- SETUP: Chạy trước mỗi TEST_F ---
    void SetUp() override {
        // 1. Tạo thư mục media giả và file mp3 rỗng (để MusicLibrary load được)
        if (!QDir(TEST_MEDIA_DIR).exists()) {
            QDir().mkdir(TEST_MEDIA_DIR);
        }

        // Danh sách file nhạc từ songs.txt của bạn
        std::vector<QString> mp3Files = {
            "Attention.mp3", "Dangerously.mp3", "haru_haru.mp3", 
            "Lemon.mp3", "Lemon_Tree.mp3", "Rolling_In_The_Deep.mp3",
            "See_You_Again.mp3", "Set_Fire_To_The_Rain.mp3", "shape_of_you.mp3",
            "Skyfall.mp3", "Someone_Like_You.mp3", "The_Nights.mp3",
            "Waiting_For_Love.mp3", "We_Don_t_Talk_Anymore.mp3", "Yummy.mp3"
        };

        for (const auto& filename : mp3Files) {
            createDummyFile(TEST_MEDIA_DIR + "/" + filename);
        }

        // 2. Tạo file songs.txt giả chứa dữ liệu thật
        QFile file(TEST_DB_FILE);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "1|Attention|Charlie Puth|Voicenotes|" << TEST_MEDIA_DIR << "/Attention.mp3\n";
            out << "2|Dangerously|Charlie Puth|Nine Track Mind|" << TEST_MEDIA_DIR << "/Dangerously.mp3\n";
            out << "3|Haru Haru|Big Bang|Stand Up|" << TEST_MEDIA_DIR << "/haru_haru.mp3\n";
            out << "4|Lemon|Kenshi Yonezu|Unknown|" << TEST_MEDIA_DIR << "/Lemon.mp3\n";
            out << "5|Lemon Tree|Fools Garden|Dish of the Day|" << TEST_MEDIA_DIR << "/Lemon_Tree.mp3\n";
            out << "6|Rolling In The Deep|Adele|21|" << TEST_MEDIA_DIR << "/Rolling_In_The_Deep.mp3\n";
            out << "7|See You Again|Charlie Puth|Nine Track Mind|" << TEST_MEDIA_DIR << "/See_You_Again.mp3\n";
            out << "8|Set Fire To The Rain|Adele|21|" << TEST_MEDIA_DIR << "/Set_Fire_To_The_Rain.mp3\n";
            out << "9|Shape of You|Ed Sheeran|Divide|" << TEST_MEDIA_DIR << "/shape_of_you.mp3\n";
            out << "10|Skyfall|Adele|Unknown|" << TEST_MEDIA_DIR << "/Skyfall.mp3\n";
            out << "11|Someone Like You|Adele|21|" << TEST_MEDIA_DIR << "/Someone_Like_You.mp3\n";
            out << "12|The Nights|Avicii|The Day - Nights|" << TEST_MEDIA_DIR << "/The_Nights.mp3\n";
            out << "13|Waiting for Love|Avicii|Stories|" << TEST_MEDIA_DIR << "/Waiting_For_Love.mp3\n";
            out << "14|We Don't Talk Anymore|Charlie Puth|Nine Track Mind|" << TEST_MEDIA_DIR << "/We_Don_t_Talk_Anymore.mp3\n";
            out << "15|Yummy|Justin Bieber|Changes|" << TEST_MEDIA_DIR << "/Yummy.mp3\n";
            file.close();
        }

        // 3. Khởi tạo Controller
        player = new controller::MusicPlayer(TEST_DB_FILE.toStdString());
    }

    // --- TEARDOWN: Chạy sau mỗi TEST_F ---
    void TearDown() override {
        delete player;
        QFile::remove(TEST_DB_FILE);
        QFile::remove("session.json");
        QDir dir(TEST_MEDIA_DIR);
        dir.removeRecursively();
    }

    void createDummyFile(const QString& path) {
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) f.close();
    }
};

// ========================================================
// 1. TEST LOAD THƯ VIỆN
// ========================================================
TEST_F(MusicPlayerTest, LibraryLoadCorrectly) {
    const auto& songs = player->getLibrary().getAllSongs();
    ASSERT_EQ(songs.size(), 15);
    EXPECT_EQ(songs[0].title, "Attention");
    EXPECT_EQ(songs[14].title, "Yummy");
}

// ========================================================
// 2. TEST LOGIC: THÊM VÀO PLAY NEXT (addToPlayNext)
// ========================================================
TEST_F(MusicPlayerTest, AddToPlayNextLogic) {
    // 1. Queue chính đang có bài 1 (Attention)
    models::Song s1 = *player->getLibrary().findSongByID(1);
    player->getPlayBack().addSong(s1);

    // 2. Thêm bài 2 (Dangerously) vào Play Next -> Thành công
    bool res1 = player->addToPlayNext(2);
    EXPECT_TRUE(res1);
    EXPECT_FALSE(player->getPlayNext().empty());

    // 3. Thêm lại bài 2 vào Play Next -> Thất bại (Trùng trong Play Next)
    bool res2 = player->addToPlayNext(2);
    EXPECT_FALSE(res2);

    // 4. Thêm bài 1 vào Play Next -> Thất bại (Trùng trong Main Queue)
    bool res3 = player->addToPlayNext(1);
    EXPECT_FALSE(res3);
}

// ========================================================
// 3. TEST LOGIC: CHUYỂN BÀI (playNextSongLogic)
// ========================================================
TEST_F(MusicPlayerTest, PlayNextSongPriority) {
    // Kịch bản: Queue chính [1, 3]. Play Next [2].
    // Đang ở bài 1. Bấm Next -> Phải ra bài 2. Bấm Next nữa -> Ra bài 3.

    // Setup
    player->selectAndPlaySong(1); // Main Queue có 1
    player->getPlayBack().addSong(*player->getLibrary().findSongByID(3)); // Main Queue có [1, 3]
    
    // Thêm bài 2 vào Play Next
    player->addToPlayNext(2); 

    // --- LẦN 1: Bấm Next ---
    // Mong đợi: Lấy từ Play Next (ID 2)
    models::Song nextSong = player->playNextSongLogic();
    EXPECT_EQ(nextSong.id, 2);
    EXPECT_EQ(nextSong.title, "Dangerously");
    
    // Play Next phải rỗng sau khi lấy
    EXPECT_TRUE(player->getPlayNext().empty());

    // --- LẦN 2: Bấm Next ---
    // Mong đợi: Lấy từ Main Queue (ID 3) - Haru Haru
    nextSong = player->playNextSongLogic();
    EXPECT_EQ(nextSong.id, 3);
    EXPECT_EQ(nextSong.title, "Haru Haru");
}

// ========================================================
// 4. TEST LOGIC: XÓA BÀI (removeSong)
// ========================================================
TEST_F(MusicPlayerTest, RemoveSongLogic) {
    // Kịch bản: Queue [1, 2, 3]. Đang hát bài 2.
    // Xóa bài 2 -> Phải tự động trả về bài 3.
    // Xóa bài 1 -> Nhạc không đổi (vẫn trả về bài đang hát hoặc giữ nguyên).

    models::Song s1 = *player->getLibrary().findSongByID(1);
    models::Song s2 = *player->getLibrary().findSongByID(2);
    models::Song s3 = *player->getLibrary().findSongByID(3);

    player->getPlayBack().addSong(s1);
    player->getPlayBack().addSong(s2);
    player->getPlayBack().addSong(s3);

    // Set đang hát bài 2
    player->getPlayBack().setCurrentSongByID(2);

    // --- TEST 1: Xóa bài không hát (Bài 1) ---
    models::Song res1 = player->removeSong(1);
    // Hàm removeSong logic mới trả về bài hiện tại nếu không xóa bài đang hát
    EXPECT_EQ(res1.id, 2); 
    EXPECT_EQ(player->getPlayBack().getQueue().size(), 2); // Còn [2, 3]

    // --- TEST 2: Xóa bài ĐANG hát (Bài 2) ---
    // Mong đợi: Tự động chuyển sang bài 3
    models::Song res2 = player->removeSong(2);
    EXPECT_EQ(res2.id, 3);
    EXPECT_EQ(res2.title, "Haru Haru");
    EXPECT_EQ(player->getPlayBack().getQueue().size(), 1); // Còn [3]
}

// ========================================================
// 5. TEST ADD PLAYLIST & LỌC TRÙNG (addPlaylistToQueue)
// ========================================================
TEST_F(MusicPlayerTest, AddPlaylistFiltering) {
    // Queue đang có bài 1. Thêm playlist [1, 2].
    models::Song s1 = *player->getLibrary().findSongByID(1);
    player->getPlayBack().addSong(s1);

    std::vector<int> newIds = {1, 2};
    int added = player->addPlaylistToQueue(newIds);

    EXPECT_EQ(added, 1); // Chỉ thêm được bài 2
    EXPECT_EQ(player->getPlayBack().getQueue().size(), 2); // Tổng [1, 2]
}

// ========================================================
// 6. TEST SHUFFLE
// ========================================================
TEST_F(MusicPlayerTest, ShuffleLogic) {
    player->enableShuffle(true);
    EXPECT_TRUE(player->isShuffleEnabled());
    
    player->enableShuffle(false);
    EXPECT_FALSE(player->isShuffleEnabled());
}

// ========================================================
// 7. TEST VOLUME SAVE/RESTORE
// ========================================================
TEST_F(MusicPlayerTest, SessionVolume) {
    player->setVolume(88);
    player->saveSession("test_session.json");

    player->setVolume(0);
    player->restoreSession("test_session.json");

    EXPECT_EQ(player->getVolume(), 88);
}

// Entry Point
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}