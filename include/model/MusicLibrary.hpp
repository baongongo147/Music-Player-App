#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include "Song.hpp"

namespace models{
	class MusicLibrary {
	private:
		std::vector<Song> songs;
		// Tao unordered_map voi key la int, value la Song*
		std::unordered_map<int, Song*> songIndexByID;
		// tao map voi key la string, value la Song*
		std::map<std::string, Song*> songIndexByTitle;
		// tao unordered_map voi key la string, value la vector<Song*>
		std::unordered_map<std::string, std::vector<Song*>> artistIndex;
		std::unordered_map<std::string, std::vector<Song*>> albumIndex;
	public:
		explicit MusicLibrary(const std::string& path);
		const std::vector<Song>& getAllSongs() const;
		bool loadFromFile(const std::string& filename);
		void buildIndexes();
		Song* findSongByID(int id);
		Song* findSongByTitle(const std::string& title);
		const std::vector<Song*> findSongsByArtist(const std::string& artist) const;
		const std::vector<Song*> findSongsByAlbum(const std::string& album) const;
	};	
}
