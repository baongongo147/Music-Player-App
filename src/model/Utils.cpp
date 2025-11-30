#include "model/Utils.hpp"

void addAlbumToQueue(const std::string& albumName, const MusicLibrary& library, PlaybackQueue& queue) {
	for (const auto& song : library.getAllSongs()) {
		if (song.album == albumName) queue.addSong(song);
	}
}