#pragma once
#include <queue>
#include "Song.hpp"

namespace models{
	class PlayNextQueue {
	private:
		std::queue<Song> playNextSongs;
	public:
		void addSong (const Song& song);
		void clear();
		bool empty();
		Song pickNextSong() const;
		Song popNextSong();
		std::vector<Song> getNextQueue() const;
	};
}