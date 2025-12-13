#include "../../include/model/Utils.hpp"

namespace models{
void addAlbumToQueue(const std::string& albumName,const models::MusicLibrary& library,models::PlaybackQueue& queue) {
    const auto& all = library.getAllSongs();
	int i = 0;
    for (const auto& s : all) {
        if (s.album == albumName) {
            queue.addSong(s);
			++i;
        }
    }
	std::cout <<"The number of the song has been added is "<<i<<std::endl;
}
}