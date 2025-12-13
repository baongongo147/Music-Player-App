#pragma once
#include <list>
#include <memory>
#include "Song.hpp"

namespace controller{
	class MusicPlayer;
}

namespace models{
	class PlaybackQueue {
	private:
		std::list<Song> queue;
		std::list<Song>::iterator currentSong;
		// Cập nhật current_ về begin nếu queue lần đầu có phần tử
		void ensureCurrentOnFirst();
		friend controller::MusicPlayer;
	public:
		
		PlaybackQueue();
		bool empty() const { return queue.empty(); }
		size_t size() const { return queue.size(); }
		void addSong(const Song& song);
		void removeSong(int songID);
		Song getCurrentSong() const;
		void playNext();
		std::list<Song>& getQueue();
		void setCurrentSongByID(int id);
	};
}