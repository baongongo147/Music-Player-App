#include "../../include/controller/MusicPlayer.hpp"
#include <stdexcept>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDebug>


namespace controller{
	void MusicPlayer::initializePlaybackIterator(std::list<models::Song>::iterator it) {
		playBack.currentSong = it; 
		lastSequentialPosition = it; // Khởi tạo vị trí tuần tự cuối cùng
	}

	void MusicPlayer::insertSongToPlaybackQueue(const models::Song& song) {
		auto& q = playBack.getQueue();
		
		// BƯỚC 1: Kiểm tra xem bài hát đã tồn tại trong hàng đợi chưa
		for (auto it = q.begin(); it != q.end(); ++it) {
			if (it->id == song.id) {
				// Nếu tìm thấy: Chỉ cần dời con trỏ currentSong về vị trí bài hát đó
				playBack.currentSong = it;
				
				std::cout << "Song existed in queue. Moved current pointer to: " << song.title << std::endl;
				return; // Kết thúc hàm, không chèn thêm
			}
		}
		// BƯỚC 2: Nếu chưa có (logic cũ) -> Thì mới chèn vào vào vị trí currentSong, và bài mới thành currentSong
		if (q.empty()) {
			q.push_back(song);
			playBack.currentSong = q.begin();
		} else {
			auto pos = playBack.currentSong; 
			++pos;
			playBack.currentSong = q.insert(pos, song); 
		}
	}

	void MusicPlayer::selectAndPlaySong(int songID) {
		models::Song* found = library.findSongByID(songID);
		if (!found) {
			std::cout << "Song (ID: " << songID << ") not found in library." << std::endl;
			return;
		}
		
		models::Song currentSong;
		try {
			currentSong = playBack.getCurrentSong();
		} catch (const std::runtime_error& e) {
		}

		if (currentSong.id != 0) {
			history.push(currentSong);
		}
		
		auto& q = playBack.getQueue();
		
		insertSongToPlaybackQueue(*found);
		
		if (shuffleEnable) {
			shuffle.load(q);
		}
	}

	// Lưu và Khôi phục vị trí Tuần tự
	void MusicPlayer::enableShuffle(bool enable) {
		shuffleEnable = enable;
		auto& q = playBack.getQueue();

		if (enable) {
			// Lưu currentSong hiện tại vào lastSequentialPosition
			if (!q.empty()) {
				lastSequentialPosition = playBack.currentSong;
				std::cout << "Sequential position saved: " << lastSequentialPosition->title << std::endl;
			} 
			shuffle.load(q);
		} else {
			// KHÔI PHỤC VỊ TRÍ TUẦN TỰ: Đặt currentSong về vị trí đã lưu
			if (!q.empty()) {
				// Chuyển sang bài tiếp theo (P4 sau P3)
				auto nextSequential = lastSequentialPosition;
				++nextSequential; 
				if (nextSequential == q.end()) {
					nextSequential = q.begin();
				}

				playBack.currentSong = nextSequential; 
				std::cout << "Sequential position restored to: " << playBack.currentSong->title << std::endl;
			}
			shuffle.resetCycle();
		}
	}

	// Đảm bảo chỉ đẩy bài hát VỪA KẾT THÚC vào History
	void MusicPlayer::playSong() {
		auto& q = playBack.getQueue();
		if(q.empty()) {
			std::cout << "No song to play" << std::endl;
			return;
		}
		
		// Xác định bài hát vừa kết thúc (sẽ được lưu vào history)
		models::Song previousSong;
		try {
			previousSong = playBack.getCurrentSong();
		} catch (const std::runtime_error& e) {
			// Nếu không có bài nào đang phát, previousSong có id=0
		}

		models::Song nextToPlay;
		bool playedFromQueue = false;

		if (!playNext.empty()) {
			nextToPlay = playNext.popNextSong();
			std::cout << "[Priority PlayNext] ";
		}

		else if (shuffleEnable) {
			nextToPlay = shuffle.getNextSong();
			std::cout << "[Shuffle Mode] ";

			for (auto i = q.begin(); i != q.end(); ++i) {
				if (i->id == nextToPlay.id) {
					playBack.currentSong = i;
					break;
				}
			}
			playedFromQueue = true; // Đánh dấu đã chuyển bài
		}
		else {
			// Chuyển index sang bài tiếp theo cho lần phát sau
			playBack.playNext();
			std::cout << "[Sequential Play] ";
			try {
				nextToPlay = playBack.getCurrentSong();
			} catch (const std::runtime_error&) {
				return;
			}
			// Đánh dấu đã chuyển bài (để lưu vào history)
			playedFromQueue = true; 
		}

		// Thêm bài hát VỪA KẾT THÚC (previousSong) vào history 
		// Chỉ lưu nếu nó là một bài hát hợp lệ VÀ việc chuyển bài là tự động (Menu 1)
		if (previousSong.id != 0 && playedFromQueue) { 
			history.push(previousSong); 
		}
		
		// 3. Hiển thị bài hát mới
		if (nextToPlay.id != 0) {
			std::cout << "Now Playing: " << nextToPlay.title << " by " << nextToPlay.artist << std::endl;
		}
	}

	models::PlaybackQueue MusicPlayer::createSmartPlaylist(int startSongID, int maxSize) {
		models::Song* startSong = library.findSongByID(startSongID);
		if (!startSong) {
			std::cout << "Start song (ID: " << startSongID << ") not found." << std::endl;
			return models::PlaybackQueue();
		}
		std::cout<<"Smart playlist has been created \n";
		return models::SmartPlaylist::generateSmartPlaylist(*startSong, library, maxSize);
	}

	// Hàm lưu session
	void MusicPlayer::saveSession(const QString& filename) {
		QJsonObject root;
		
		// 1. Lưu Playback Queue
		QJsonArray playbackArray;
		for(const auto& s : playBack.getQueue()) {
			playbackArray.append(s.id); 
		}
		root["playbackQueue"] = playbackArray;

		// 2. Lưu History
		QJsonArray historyArray;
		for(const auto& s : history.getHistory()) {
			historyArray.append(s.id);
		}
		root["history"] = historyArray;

		// 3. Lưu Current Song ID
		try {
			if (!playBack.empty()) {
				root["currentSongID"] = playBack.getCurrentSong().id;
			} else {
				root["currentSongID"] = -1;
			}
		} catch (...) {
			root["currentSongID"] = -1;
		}

		// 4. Lưu volume
		root["volume"] = m_volume;

		// 5. Lưu Playlist: Gọi hàm toJson của library để lấy mảng playlist và lưu vào root
        root["customPlaylists"] = playlistLib.toJson();

		// Ghi ra file
		QFile file(filename);
		if (file.open(QIODevice::WriteOnly)) {
			QJsonDocument doc(root);
			file.write(doc.toJson());
			file.close();
		}
	}

	// Hàm lấy Session đã lưu
	void MusicPlayer::restoreSession(const QString& filename) {
		QFile file(filename);
		if (!file.open(QIODevice::ReadOnly)) return;

		QByteArray data = file.readAll();
		QJsonDocument doc = QJsonDocument::fromJson(data);
		QJsonObject root = doc.object();

		// 1. Khôi phục Playback Queue
		if (root.contains("playbackQueue")) {
			QJsonArray arr = root["playbackQueue"].toArray();
			for (const auto& val : arr) {
				int id = val.toInt();
				
				// --- DÙNG HÀM CỦA BẠN: findSongByID ---
				models::Song* sPtr = library.findSongByID(id); 
				
				if (sPtr != nullptr) { // Kiểm tra tìm thấy không
					// Dùng *sPtr để lấy giá trị Song truyền vào addSong
					playBack.addSong(*sPtr); 
				}
			}
		}

		// 2. Khôi phục History
		if (root.contains("history")) {
			QJsonArray arr = root["history"].toArray();
			for (const auto& val : arr) {
				int id = val.toInt();
				
				// --- DÙNG HÀM CỦA BẠN: findSongByID ---
				models::Song* sPtr = library.findSongByID(id);
				
				if (sPtr != nullptr) {
					// Dùng *sPtr để lấy giá trị Song truyền vào push
					history.push(*sPtr); 
				}
			}
		}
		
		// 3. Khôi phục Current Song (Giữ nguyên)
		if (root.contains("currentSongID")) {
			int currentID = root["currentSongID"].toInt();
			if (currentID != -1) {
				playBack.setCurrentSongByID(currentID);
			}
		}

		// 4. Khôi phục volume
		if (root.contains("volume")) {
			m_volume = root["volume"].toInt();
		}

		// 5.Khôi phục playlists
        if (root.contains("customPlaylists")) {
            QJsonArray plArray = root["customPlaylists"].toArray();
            // Nạp dữ liệu vào PlaylistLibrary
            // Hàm này sẽ xóa playlist mặc định và thay bằng danh sách trong file
            playlistLib.fromJson(plArray); 
        }
	}

	/** Hàm tìm kiếm:
	 * Input: từ khóa, loại tìm kiếm (0: ID, 1: Title, 2: Artist, 3: Album)
	 * Output: Danh sách các bài hát thỏa mãn
	*/
	std::vector<models::Song> MusicPlayer::searchSongs(const QString& keyword, int searchType) {
        std::vector<models::Song> results;
        
        // Lấy toàn bộ bài hát từ thư viện
        const auto& allSongs = library.getAllSongs();

        // Duyệt và lọc
        for (const auto& s : allSongs) {
            bool isMatch = false;

            // Chuyển đổi dữ liệu sang QString để xử lý
            QString qTitle = QString::fromStdString(s.title);
            QString qArtist = QString::fromStdString(s.artist);
            QString qAlbum = QString::fromStdString(s.album);

            switch (searchType) {
                case 0: // By ID (Tìm chính xác)
                    if (QString::number(s.id) == keyword) {
                        isMatch = true;
                    }
                    break;
                case 1: // By Title (Partial Match)
                    if (qTitle.contains(keyword, Qt::CaseInsensitive)) {
                        isMatch = true;
                    }
                    break;
                case 2: // By Artist
                    if (qArtist.contains(keyword, Qt::CaseInsensitive)) {
                        isMatch = true;
                    }
                    break;
                case 3: // By Album
                    if (qAlbum.contains(keyword, Qt::CaseInsensitive)) {
                        isMatch = true;
                    }
                    break;
            }

            // Nếu khớp thì thêm vào danh sách kết quả
            if (isMatch) {
                results.push_back(s);
            }
        }
        return results;
    }

	int MusicPlayer::addPlaylistToQueue(const std::vector<int>& songIDs) {
        auto& queue = playBack.getQueue();
        int addedCount = 0;

        // 1. Tạo danh sách các ID đang tồn tại trong hàng đợi để đối chiếu
        std::set<int> existingIDs;
        for (const auto& s : queue) {
            existingIDs.insert(s.id);
        }

        // 2. Duyệt qua playlist cần thêm
        for (int id : songIDs) {
            // Kiểm tra: ID này có trong existingIDs chưa?
            if (existingIDs.find(id) == existingIDs.end()) {
                // Nếu CHƯA có -> Tìm bài hát trong thư viện
                models::Song* s = library.findSongByID(id);
                
                if (s) {
                    // Thêm vào cuối hàng đợi
                    playBack.addSong(*s);
                    
                    // Thêm ID vào set (để tránh trường hợp trong chính playlist đầu vào có 2 bài giống nhau)
                    existingIDs.insert(id);
                    
                    addedCount++;
                }
            }
            // Nếu ĐÃ có (else) -> Bỏ qua, không làm gì cả
        }

        return addedCount; // Trả về số bài thực tế đã thêm
    }

	 void MusicPlayer::createUserPlaylist(const QString& name, const QString& desc, const std::vector<int>& songIDs) {
        models::PlaylistInfo newPl;
        newPl.name = name;
        newPl.description = desc;
        newPl.songIDs = songIDs;

        // Thêm vào thư viện
        playlistLib.addPlaylist(newPl);
    }

	// --- 1. LOGIC THÊM VÀO PLAY NEXT (Kiểm tra trùng) ---
    bool MusicPlayer::addToPlayNext(int songID) {
        models::Song* song = library.findSongByID(songID);
        if (!song) return false;

        // A. Kiểm tra trùng trong Main Queue
        const auto& mainQueue = playBack.getQueue();
        for (const auto& s : mainQueue) {
            if (s.id == songID) return false; // Trùng
        }

        // B. Kiểm tra trùng trong Play Next Queue
        const auto& nextQueue = playNext.getNextQueue();
        for (const auto& s : nextQueue) {
            if (s.id == songID) return false; // Trùng
        }

        // C. Thêm vào
        playNext.addSong(*song);
        return true;
    }

    // --- 2. LOGIC TÌM BÀI TIẾP THEO (NEXT) ---
    // (Logic này được copy từ handleNext của bạn và chuẩn hóa lại)
    models::Song MusicPlayer::playNextSongLogic() {
        // ƯU TIÊN 1: Kiểm tra Play Next Queue
        if (!playNext.empty()) {
            models::Song s = playNext.popNextSong(); 
            // Lưu ý: Logic cũ của bạn là lấy ra và phát luôn, không lưu vào history ngay lúc này
            return s; 
        }

        // ƯU TIÊN 2: Kiểm tra Playback Queue (Main)
        if (!playBack.empty()) {
            // Logic cũ của bạn: player.playSong() (tức là playNext bên trong model)
            // rồi lấy currentSong ra.
            
            // 1. Lưu bài cũ vào history trước khi chuyển
            try {
                models::Song prev = playBack.getCurrentSong();
                if (prev.id != 0) history.push(prev);
            } catch(...) {}

            // 2. Chuyển iterator sang bài tiếp theo
            // (Copy logic từ hàm playSong cũ vào đây để quy về 1 mối)
            if (shuffleEnable) {
                models::Song nextToPlay = shuffle.getNextSong();
                auto& q = playBack.getQueue();
                for (auto i = q.begin(); i != q.end(); ++i) {
                    if (i->id == nextToPlay.id) {
                        playBack.currentSong = i;
                        break;
                    }
                }
            } else {
                try {
                    playBack.playNext();
                } catch (...) {
                    return models::Song(); // Hết bài -> Trả về rỗng
                }
            }

            // 3. Trả về bài hát mới tại con trỏ hiện tại
            try {
                return playBack.getCurrentSong();
            } catch (...) {
                return models::Song();
            }
        }

        return models::Song(); // Cả 2 hàng đợi đều rỗng
    }

    // --- 3. LOGIC XÓA BÀI ---
    models::Song MusicPlayer::removeSong(int songID) {
        // A. Xác định xem có phải bài đang hát trong PlaybackQueue không
        bool isDeletingCurrent = false;
        try {
            if (!playBack.empty() && playBack.getCurrentSong().id == songID) {
                isDeletingCurrent = true;
            }
        } catch (...) {}

        // B. Xóa khỏi backend
        playBack.removeSong(songID);

        // C. Xử lý kết quả trả về
        if (isDeletingCurrent) {
            // Nếu xóa bài đang hát -> Phải tìm bài mới ngay
            
            // ƯU TIÊN 1: Play Next Queue
            if (!playNext.empty()) {
                return playNext.popNextSong();
            }

            // ƯU TIÊN 2: Playback Queue
            // Vì hàm removeSong của model PlaybackQueue thường đã đẩy iterator sang bài kế tiếp
            // nên ta chỉ cần lấy nó ra.
            try {
                if (!playBack.empty()) {
                    return playBack.getCurrentSong();
                }
            } catch (...) {}

            // Không còn bài nào
            return models::Song(); 
        } 
        else {
            // Nếu xóa bài khác -> Vẫn trả về bài đang hát hiện tại để View biết mà update (nếu cần)
            // hoặc View có thể bỏ qua.
            try {
                if (!playBack.empty()) return playBack.getCurrentSong();
            } catch(...) {}
            
            // Trường hợp đặc biệt: Nếu đang hát bài từ PlayNext (không nằm trong Queue)
            // thì logic này cần bạn truyền bài đang hát vào tham số. 
            // Nhưng với cấu trúc hiện tại, ta tạm trả về rỗng hoặc bài hiện tại của Queue.
            return models::Song(); 
        }
    }
}