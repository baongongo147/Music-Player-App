#pragma once
#include <iostream>
#include "model/Song.hpp"
#include "model/MusicLibrary.hpp"
#include "model/PlaybackQueue.hpp"
#include "model/PlaybackHistory.hpp"
#include "model/PlayNextQueue.hpp"
#include "model/ShuffleManager.hpp"
#include "model/SmartPlaylist.hpp"

class MusicPlayer {	
private:
	MusicLibrary library;
	PlaybackQueue playBack;
	PlaybackHistory history;
	PlayNextQueue playNext;
	ShuffleManager shuffle;
	bool shuffleEnable = false;
public:
	//getter
	MusicLibrary& getLibrary() { return library; };
	PlaybackQueue& getPlayBack() { return playBack; };
	PlaybackHistory& getHistory() { return history; };
	PlayNextQueue& getPlayNext() { return playNext; }
	ShuffleManager& getShuffle() { return shuffle; };
	
	void selectAndPlaySong(int songID);
	void enableShuffle (bool enable);
	void playSong();
	PlaybackQueue createSmartPlaylist(int startSongID, int maxSize);
};
