#pragma once
#include <queue>
#include "Song.hpp"

class PlayNextQueue {
private:
    std::queue<Song> playNextSongs;
public:
    void addSong (const Song& song);
    bool empty() const;
    void clear();
    Song pickNextSong() const;
    Song popNextSong();
};