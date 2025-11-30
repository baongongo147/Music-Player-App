#include "model/PlayNextQueue.hpp"
#include <stdexcept>

void PlayNextQueue::addSong (const Song& song) {
    playNextSongs.push(song);
}

bool PlayNextQueue::empty() const {
    return playNextSongs.empty();
}

Song PlayNextQueue::pickNextSong() const {
    if (playNextSongs.empty()) throw std::runtime_error("No songs in Play Next Queue");
    return playNextSongs.front();
}

Song PlayNextQueue::popNextSong() {
    if (playNextSongs.empty()) throw std::runtime_error("No songs in Play Next Queue");
    Song nextSong = playNextSongs.front();
    playNextSongs.pop();
    return nextSong;
}

void PlayNextQueue::clear() {
    while (!playNextSongs.empty()) {
        playNextSongs.pop();
    }
}

