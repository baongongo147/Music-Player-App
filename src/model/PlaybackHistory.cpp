#include "../../include/model/PlaybackHistory.hpp"
#include <stdexcept>

namespace models{
void PlaybackHistory::push (const Song& song) {
	history.push(song);
	std::cout << "Song added to history: " << song.title << std::endl;
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

void PlaybackHistory::clear() {
	while (!history.empty()) {
		history.pop();
	}
	std::cout << "Playback history cleared." << std::endl;
}

// [Hàm xóa phần tử cuối cùng của Stack/Vector
void PlaybackHistory::removeLastAddedSong() {
    if (!history.empty()) { 
        history.pop(); 
    }
}
}
