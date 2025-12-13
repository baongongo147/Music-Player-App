#pragma once
#include <stack>
#include <vector>
#include "Song.hpp"

namespace models{
	class PlaybackHistory {
	private:
		std::stack<Song> history;
	public:
		void push(const Song& song);
		Song playPreviousSong();
		void clear();
		std::vector<Song> getHistory() const;
		void removeLastAddedSong();
	};
}