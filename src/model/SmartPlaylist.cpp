#include "../../include/model/SmartPlaylist.hpp"
namespace models{
PlaybackQueue SmartPlaylist::generateSmartPlaylist(const Song& startSong, const MusicLibrary& library, int maxSize) {
    PlaybackQueue smartQueue;
    if (maxSize <= 0) return smartQueue;
    std::set<int> songIDs;
    std::queue<const Song*> queue;
    queue.push(&startSong);
    songIDs.insert(startSong.id);
    smartQueue.addSong(startSong);

    while (!queue.empty() && smartQueue.getQueue().size() < maxSize) {
        const Song* currentSong = queue.front();
        queue.pop();
        // Xet cung artist
        auto artistNeighbors = library.findSongsByArtist(currentSong->artist); 
        for (Song* neighbor : artistNeighbors) {
            if ((int)smartQueue.getQueue().size() >= maxSize) break;
            if (songIDs.insert(neighbor->id).second) {
                smartQueue.addSong(*neighbor);
                queue.push(neighbor);
            }
        }
        // Xet cung album
        auto albumNeighbors = library.findSongsByAlbum(currentSong->album); 
        for (Song* neighbor : albumNeighbors) {
            if ((int)smartQueue.getQueue().size() >= maxSize) break;
            if (songIDs.insert(neighbor->id).second) {
                smartQueue.addSong(*neighbor);
                queue.push(neighbor);
            }
        }
    }
    return smartQueue;
}
}