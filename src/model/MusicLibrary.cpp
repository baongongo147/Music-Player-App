#include "model/MusicLibrary.hpp"
#include <iostream>
#include <memory>
#include <fstream>
#include <sstream> 

void MusicLibrary::addSong(const Song& song) {
	songs.push_back(song);

	Song* ptr = &songs.back();
	songIndexByID[song.id] = ptr;
	songIndexByTitle[song.title] = ptr;
	artistIndex[song.artist].push_back(ptr);
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
    // Bỏ qua dòng tiêu đề nếu có (giả sử file data bắt đầu ngay bằng dữ liệu)
    
    // Đọc từng dòng
    while (std::getline(file, line)) {
        if (line.empty()) continue; // Bỏ qua dòng trống

        // Sử dụng stringstream để tách các trường bằng dấu '|'
        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> parts;
        
        while (std::getline(ss, segment, '|')) {
            parts.push_back(segment);
        }

        // Đảm bảo có đúng 5 trường: id, title, artist, album, duration
        if (parts.size() == 5) {
            try {
                int id = std::stoi(parts[0]);
                std::string title = parts[1];
                std::string artist = parts[2];
                std::string album = parts[3];
                int duration = std::stoi(parts[4]);

                // Tạo đối tượng Song mới và thêm vào thư viện
                // Lưu ý: Song() của đồng nghiệp bạn có constructor 5 tham số
                Song newSong(id, title, artist, album, duration);
                this->addSong(newSong);

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