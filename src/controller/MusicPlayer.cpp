#include "controller/MusicPlayer.hpp"

void MusicPlayer::selectAndPlaySong(int songID) {
	Song* found = library.findSongByID(songID);
	if (!found) {
		std::cout << "Song not found" << std::endl;
		return;
	}
	Song currentSong = playBack.getCurrentSong();
	
	if (!currentSong.title.empty()) {
		history.push(currentSong);
	}
	currentSong = *found;
	// playBack.addSong(currentSong);
	// playBack.playNext();
	auto& q = playBack.getQueue();
	if (q.empty()) {
		q.push_back(currentSong);
		playBack.currentIndex = q.begin();
		return;
	}
	auto pos = playBack.currentIndex;
	++pos;
	playBack.currentIndex = q.insert(pos, currentSong);
	playSong();
	// shuffle 
	if (shuffleEnable) {
		shuffle.load(q);
	}
}

void MusicPlayer::enableShuffle(bool enable) {
	shuffleEnable = enable;
	if (enable) shuffle.load(playBack.getQueue());
	else shuffle.resetCycle();
}

void MusicPlayer::playSong() {
	auto& q = playBack.getQueue();
	if(q.empty()) {
		std::cout << "No song to play" << std::endl;
		return;
	}
	Song currentSong;
	if (!playNext.empty()) {
		currentSong = playNext.popNextSong();
	}
	else if (shuffleEnable) {
		currentSong = shuffle.getNextSong();

		for (auto i = q.begin(); i != q.end(); ++i) {
			if (i->id == currentSong.id) {
				playBack.currentIndex = i;
				break;
			}
		}
	}
	else {
		currentSong = playBack.getCurrentSong();
		playBack.playNext();
	}
	std::cout << "Now Playing: " << currentSong.title << " by " << currentSong.artist << std::endl;
	history.push(currentSong);
}

PlaybackQueue MusicPlayer::createSmartPlaylist(int startSongID, int maxSize) {
	Song* startSong = library.findSongByID(startSongID);
	if (!startSong) {
		std::cout << "Start Song not found so cannot create Smart Playlist" << std::endl;
		return PlaybackQueue();
	}
	return SmartPlaylist::generateSmartPlaylist(*startSong, library, maxSize);
}
