#include "model/PlaybackHistory.hpp"
#include <stdexcept>

void PlaybackHistory::push (const Song& song) {
	history.push(song);
}

Song PlaybackHistory::playPreviousSong() {
	if (history.empty()) throw std::runtime_error("No History to play");
	Song song = history.top();
	history.pop();
	return song;
}

std::vector<Song> PlaybackHistory::getHistory() const {
	std::vector<Song> list;
	std::stack<Song> temp = history;

	while (!temp.empty()) {
		list.push_back(temp.top());
		temp.pop();
	}
	return list;
}