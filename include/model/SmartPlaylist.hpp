#pragma once
#include <set>
#include <queue>
#include "PlaybackQueue.hpp"
#include "MusicLibrary.hpp"

namespace models{
class SmartPlaylist {
public:
    static PlaybackQueue generateSmartPlaylist(const Song& startSong, const MusicLibrary& library, int maxSize);
};
}