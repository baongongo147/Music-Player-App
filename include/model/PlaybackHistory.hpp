#pragma once
#include <stack>
#include <vector>
#include "Song.hpp"

class PlaybackHistory {
private:
	std::stack<Song> history;
public:
	void push(const Song& song);
	Song playPreviousSong();
	std::vector<Song> getHistory() const;
};