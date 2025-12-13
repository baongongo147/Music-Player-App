#include <stdexcept>
#include "../../include/model/PlaybackQueue.hpp"

namespace models{
	PlaybackQueue::PlaybackQueue(): queue{}, currentSong{queue.end()} {}

	void PlaybackQueue::ensureCurrentOnFirst() {
		if (currentSong == queue.end() && !queue.empty()) {
			currentSong = queue.begin();
		}
	}

	void PlaybackQueue::addSong(const Song& song) {
		queue.push_back(song);
		// Nếu trước đó rỗng -> current_ trỏ về begin
		ensureCurrentOnFirst();
	}

	void PlaybackQueue::removeSong(int songID) {
		if (empty()){
			std::cout << "Playback Queue is empty. Cannot remove song.\n";
			return;
		} 
		for (auto it = queue.begin(); it != queue.end(); ++it) {
			if (it->id == songID) {
				// Nếu xóa đúng bài hiện tại
				if (it == currentSong) {
					// Tiến tới phần tử kế tiếp nếu có
					auto next = std::next(it);
					if (next !=queue.end()) {
						currentSong = next;
					} else {
						// Không có phần tử kế tiếp thì băt đầu lại từ đầu nếu còn phần tử
						if (size() > 1) {
							currentSong = queue.begin();
						} else {
							// Chỉ còn một phần tử và nó bị xóa
							currentSong = queue.end();
						}
					}
				}
				// Thực sự xóa phần tử khỏi list
				std::cout<<"Song name: "<<it->title<<" has been deleted"<<std::endl;
				queue.erase(it);

				// Nếu sau khi xóa queue rỗng -> current_ = end
				if (empty()) {
					currentSong = queue.end();
					std::cout<<"Playback Queue is emty \n";
				}
				return; // Xóa lần xuất hiện đầu tiên, rồi thoát
			}
		}
	}

	Song PlaybackQueue::getCurrentSong() const {
		if (empty() || currentSong == queue.end()) {
			throw std::runtime_error("PlaybackQueue is empty: no current song.");
		}
		return *currentSong; 
	}
	
	void PlaybackQueue::playNext() {
		if (empty() || currentSong == queue.end())
		{
			std::cout << "Playback Queue is empty. Cannot play next song.\n";
			return;
		} 
		auto next = std::next(currentSong);
		if (next != queue.end()) {
			currentSong= next;
		} else {
			currentSong = queue.begin();
		}
		std::cout << "Now playing: " << currentSong->title << " by " << currentSong->artist << std::endl;
	}

	void PlaybackQueue::setCurrentSongByID(int id) {
        if (queue.empty()) return;

        // Duyệt qua từng phần tử để tìm ID khớp
        for (auto it = queue.begin(); it != queue.end(); ++it) {
            if (it->id == id) {
                currentSong = it; // Đặt lại con trỏ iterator vào đúng bài cũ
                std::cout << "Restored current song: " << it->title << "\n";
                return;
            }
        }

        // Trường hợp đặc biệt: Nếu ID lưu trong file không tìm thấy trong list 
        // (ví dụ bài đó bị xóa khỏi thư viện rồi), thì reset về bài đầu tiên cho an toàn.
        std::cout << "Saved song ID not found, resetting to start.\n";
        currentSong = queue.begin();
    }

	std::list<Song>& PlaybackQueue::getQueue() {return queue;}
}