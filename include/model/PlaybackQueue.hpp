#pragma once
#include <list>
#include <memory>
#include "Song.hpp"

class PlaybackQueue {
private:
	std::list<Song> queue;
	std::list<Song>::iterator currentIndex;
	friend class MusicPlayer;
public:
	PlaybackQueue();

	void addSong(const Song& song);
	void removeSong(int songID);
	Song getCurrentSong();
	void playNext();
	std::list<Song>& getQueue();
};