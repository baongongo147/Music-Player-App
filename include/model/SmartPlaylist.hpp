#pragma once
#include <set>
#include <queue>
#include "Song.hpp"
#include "PlaybackQueue.hpp"
#include "MusicLibrary.hpp"

class SmartPlaylist {
public:
    static PlaybackQueue generateSmartPlaylist(const Song& startSong, const MusicLibrary& library, int maxSize);
};