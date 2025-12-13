#pragma once
#include <vector>
#include <set>
#include <list>
#include "Song.hpp"

namespace models{
	class ShuffleManager {
	private:
		std::vector<Song> shuffleList;
		std::set<int> playedID;
		size_t indexShuffle;
	public:
		ShuffleManager();
		void load(const std::list<Song>& queue);
		Song getNextSong();
		void resetCycle();
	};
}