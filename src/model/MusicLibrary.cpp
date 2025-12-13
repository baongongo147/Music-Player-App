#include "../../include/model/MusicLibrary.hpp"
#include <iostream>
#include <fstream>
#include <sstream> 
#include <stdexcept>

namespace models{
	MusicLibrary::MusicLibrary(const std::string& path) {
		loadFromFile(path);
		buildIndexes();
	}

	const std::vector<Song>& MusicLibrary::getAllSongs() const {
		return songs;
	}

	Song* MusicLibrary::findSongByID(int id) {
		auto i = songIndexByID.find(id);
		if (i != songIndexByID.end()) return i->second;
		return nullptr;
	}

	Song* MusicLibrary::findSongByTitle(const std::string& title) {
		auto i = songIndexByTitle.find(title);
		if (i != songIndexByTitle.end()) return i->second;
		return nullptr;
	}
	
	const std::vector<Song*> MusicLibrary::findSongsByArtist(const std::string& artist) const {
		auto i = artistIndex.find(artist);
		if (i != artistIndex.end()) return i->second;
		static const std::vector<Song*> empty;
		return empty;
	}

	const std::vector<Song*> MusicLibrary::findSongsByAlbum(const std::string& album) const {
		auto i = albumIndex.find(album);
		if (i != albumIndex.end()) return i->second;
		static const std::vector<Song*> empty;
		return empty;
	}

	bool MusicLibrary::loadFromFile(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Error: Could not open file " << filename << std::endl;
			return false;
		}
		std::string line;
		while (std::getline(file, line)) {
			if (line.empty()) continue;
			std::stringstream ss(line);
			std::string segment;
			std::vector<std::string> parts;
			while (std::getline(ss, segment, '|')) {
				parts.push_back(segment);
			}
			if (parts.size() == 5) {
				try {
					int id = std::stoi(parts[0]);
					std::string title = parts[1];
					std::string artist = parts[2];
					std::string album = parts[3];
					std::string filePath = parts[4];
					Song newSong(id, title, artist, album, filePath);
					songs.push_back(newSong); // Add to vector for main storage
				} catch (const std::exception& e) {
					std::cerr << "Error parsing line: " << line << " (" << e.what() << ")" << std::endl;
					continue;
				}
			} else {
				std::cerr << "Warning: Skipping malformed line with " << parts.size() << " parts: " << line << std::endl;
			}
		}
		return true;
	}
	
	void MusicLibrary::buildIndexes() {
		songIndexByID.clear();
		songIndexByTitle.clear();
		artistIndex.clear();
		albumIndex.clear(); 
		songIndexByID.reserve(songs.size()); 
		for (size_t i = 0; i < songs.size(); ++i) {
			Song* p = &songs[i];
			songIndexByID[p->id]     = p;
			songIndexByTitle[p->title] = p;
			artistIndex[p->artist].push_back(p);
			albumIndex[p->album].push_back(p);
		}
	}
}