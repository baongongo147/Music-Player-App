#pragma once
#include "../model/Song.hpp"
#include "../model/MusicLibrary.hpp"
#include "../model/PlaybackQueue.hpp"
#include "../model/PlaybackHistory.hpp"
#include "../model/PlayNextQueue.hpp"
#include "../model/ShuffleManager.hpp"
#include "../model/SmartPlaylist.hpp"
#include "../model/PlaylistLibrary.hpp" 
#include <QString> // Cần thiết cho tham số filename

namespace controller {
	class MusicPlayer {	
	private:
		models::MusicLibrary library;
		models::PlaybackQueue playBack;
		models::PlaybackHistory history;
		models::PlayNextQueue playNext;
		models::ShuffleManager shuffle;
		models::PlaylistLibrary playlistLib;
		// Cờ check trạng thái để kiểm tra xem có bật shuffle không
		bool shuffleEnable = false;
		// Lưu vị trí tuần tự trước khi bật shuffle 
		std::list<models::Song>::iterator lastSequentialPosition; 
		// Biến lưu volume
		int m_volume = 50;

		// Biến lưu ID của bài hát cuối cùng trong hàng đợi và trạng thái xuất phát của nó
		int m_restoredLastQueueID = -1;
		// Biến kiểm tra bài đang hát là từ PlayNext hay Queue
    	bool m_restoredIsFromNext = false;
		
	public:
		MusicPlayer(const std::string& songFile) : library(songFile) {}

		// Getter 
		models::MusicLibrary& getLibrary() { return library; };
		models::PlaybackQueue& getPlayBack() { return playBack; };
		models::PlaybackHistory& getHistory() { return history; };
		models::PlayNextQueue& getPlayNext() { return playNext; }
		models::ShuffleManager& getShuffle() { return shuffle; };
		models::PlaylistLibrary& getPlaylistLibrary() { return playlistLib; }
		// Getter cho trạng thái Shuffle
		bool isShuffleEnabled() const { return shuffleEnable; }
		// Getter và Setter cho Volume
        void setVolume(int v) { m_volume = v; }
        int getVolume() const { return m_volume; }
		//Hàm thiết lập iterator ban đầu
		void initializePlaybackIterator(std::list<models::Song>::iterator it); 
		// Hàm chèn bài hát từ History 
		void insertSongToPlaybackQueue(const models::Song& song);
		void selectAndPlaySong(int songID);
		void enableShuffle (bool enable);
		void playSong();

		//hàm lưu dữ liệu và khôi phục dữ liệu
		void saveSession(int currentPlayingID, int lastQueueID, bool isFromNext, const QString& filename = "session.json");
    	void restoreSession(const QString& filename = "session.json");
		// Thêm getter để MainWindow lấy dữ liệu sau khi restore
		int getRestoredLastQueueID() const { return m_restoredLastQueueID; }
		bool getRestoredIsFromNext() const { return m_restoredIsFromNext; }

		// Hàm tìm kiếm
		std::vector<models::Song> searchSongs(const QString& keyword, int searchType);

		models::PlaybackQueue createSmartPlaylist(int startSongID, int maxSize);

		// Hàm thêm một danh sách ID vào hàng đợi, tự động lọc trùng
        // Trả về số lượng bài hát thực tế đã được thêm
        int addPlaylistToQueue(const std::vector<int>& songIDs);
		
		// Hàm tạo playlist từ người dùng
		void createUserPlaylist(const QString& name, const QString& desc, const std::vector<int>& songIDs);

		// 1. Logic thêm vào Play Next (có kiểm tra trùng lặp)
        // Trả về: true nếu thành công, false nếu trùng
        bool addToPlayNext(int songID);

        // 2. Logic xóa bài (xử lý cả trường hợp xóa bài đang hát)
        // Trả về: Bài hát tiếp theo cần phát (nếu xóa bài đang hát). 
        // Trả về: Bài hiện tại (nếu xóa bài khác).
        // Trả về: Song rỗng (id=0) nếu hết nhạc.
        models::Song removeSong(int songID);

        // 3. Logic chuyển bài tiếp theo (Gộp logic PlayNextQueue và MainQueue)
        // Thay thế cho việc gọi lẻ tẻ ở MainWindow
        models::Song playNextSongLogic();
		bool checkPlaylistExists(const QString& name) const;
		bool deletePlaylist(const QString& name);
	};
}

