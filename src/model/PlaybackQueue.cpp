#include <stdexcept>
#include "model/PlaybackQueue.hpp"

PlaybackQueue::PlaybackQueue() : currentIndex(queue.end()) {}

void PlaybackQueue::addSong(const Song& song) {
	bool wasEmpty = queue.empty(); 
	queue.push_back(song);
	if (wasEmpty) currentIndex = queue.begin();
}

void PlaybackQueue::removeSong(int songID) {
	for (auto i = queue.begin(); i != queue.end(); ++i) {
		if (i->id == songID) {
			if (i == currentIndex) currentIndex = queue.erase(i);
			else queue.erase(i);
			break;
		}
	}
	if (queue.empty()) currentIndex = queue.end();
}

Song PlaybackQueue::getCurrentSong() {
	if (queue.empty() || currentIndex == queue.end()) throw std::runtime_error("Empty Queue");
	return *currentIndex;
}

void PlaybackQueue::playNext() {
	if (queue.empty()) throw std::runtime_error("Empty Queue");
	++currentIndex;
	if (currentIndex == queue.end()) currentIndex = queue.begin();
}

std::list<Song>& PlaybackQueue::getQueue() {return queue;}