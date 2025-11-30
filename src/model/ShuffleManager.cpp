#include "model/ShuffleManager.hpp"
#include <random>
#include <algorithm>

ShuffleManager::ShuffleManager() : indexShuffle(0) {}

void ShuffleManager::load(const std::list<Song>& queue) {
	shuffleList.assign(queue.begin(), queue.end());
	std::random_device random;
	std::mt19937 g(random());
	shuffle(shuffleList.begin(), shuffleList.end(), g);
	indexShuffle = 0;
}

Song ShuffleManager::getNextSong() {
	// tra ve rong neu shuffle list khong co bai hat nao
	if (shuffleList.empty()) return Song();
	while (true) {
		if (indexShuffle >= shuffleList.size()) {
			indexShuffle = 0;
		}
		const Song& song = shuffleList[indexShuffle];
		// Neu chua nghe bai hat nay thi se phat no
		if (playedID.find(song.id) == playedID.end()) {		// tra ve iterator neu tim thay, tra ve .end() neu khong tim thay
			playedID.insert(song.id);
			++indexShuffle;
			return song;
		}		
		// Bo qua neu da nghe bai hat nay
		++indexShuffle;
		// Neu tat ca bai hat thi se reset chu ky shuffle
		if (playedID.size() == shuffleList.size()) resetCycle();
	}
}

void ShuffleManager::resetCycle() {
	playedID.clear();
}