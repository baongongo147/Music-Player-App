#include "../../include/model/ShuffleManager.hpp"
#include <random>
#include <algorithm>

namespace models{
ShuffleManager::ShuffleManager() : indexShuffle(0) {}

void ShuffleManager::load(const std::list<Song>& queue) {
	shuffleList.assign(queue.begin(), queue.end());
	std::random_device random;
	std::mt19937 g(random());
	std::shuffle(shuffleList.begin(), shuffleList.end(), g); 
	indexShuffle = 0;
    resetCycle(); 
}

Song ShuffleManager::getNextSong() {
	if (shuffleList.empty()) return Song();
    int iterations = 0; 
    const int max_iterations = shuffleList.size() * 2; 
	while (iterations < max_iterations) {
		if (indexShuffle >= shuffleList.size()) {
			indexShuffle = 0;
            if (playedID.size() == shuffleList.size()) {
                 resetCycle(); 
                 std::cout << "--- Shuffle Cycle Reset ---" << std::endl;
            }
		}
		const Song& song = shuffleList[indexShuffle];
		if (playedID.find(song.id) == playedID.end()) {		
			playedID.insert(song.id);
			++indexShuffle;
			return song;
		}		
		++indexShuffle;
        ++iterations;
	}
    return Song(); 
}

void ShuffleManager::resetCycle() {
	playedID.clear();
	std::cout << "Shuffle Cycle Reset " << std::endl;
}
}