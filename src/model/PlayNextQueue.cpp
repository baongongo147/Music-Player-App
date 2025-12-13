#include "../../include/model/PlayNextQueue.hpp"
#include <stdexcept>

namespace models{
void PlayNextQueue::addSong (const Song& song) {
    playNextSongs.push(song);
    std::cout << "Added song: " << song.title << " to Play Next Queue.\n";
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
    std::cout << "Play Next Queue cleared." << std::endl;
}

bool PlayNextQueue::empty(){
    return playNextSongs.empty();
}

std::vector<Song> PlayNextQueue::getNextQueue() const {
    std::vector<Song> list;
    std::queue<Song> temp = playNextSongs;
    while (!temp.empty()) {
        list.push_back(temp.front());
        temp.pop();
    }
    return list;
}
}