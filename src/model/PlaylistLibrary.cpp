#include "../../include/model/PlaylistLibrary.hpp"

namespace models {

    PlaylistLibrary::PlaylistLibrary() {
        initDefaultPlaylists();
    }

    const std::vector<PlaylistInfo>& PlaylistLibrary::getPlaylists() const {
        return m_playlists;
    }

    void PlaylistLibrary::initDefaultPlaylists() {        
        // Playlist 1
        PlaylistInfo energy;
        energy.name = "Energy Boost Playlist";
        energy.description = "EDM and Pop hits to start your day.";
        energy.songIDs = {12, 13, 9, 15, 1}; 
        m_playlists.push_back(energy);

        // Playlist 2
        PlaylistInfo sad;
        sad.name = "Deep Emotions Playlist";
        sad.description = "Sad songs for rainy days.";
        sad.songIDs = {11, 7, 14, 3, 8, 10, 2}; 
        m_playlists.push_back(sad);

        // Playlist 3
        PlaylistInfo mix;
        mix.name = "Mix Playlist";
        mix.description = "J-Pop, Oldies and Powerful Vocals.";
        mix.songIDs = {4, 5, 2, 6}; 
        m_playlists.push_back(mix);
    }

	void PlaylistLibrary::addPlaylist(const PlaylistInfo& playlist) {
        m_playlists.push_back(playlist);
    }

	QJsonArray PlaylistLibrary::toJson() const {
        QJsonArray array;
        for (const auto& pl : m_playlists) {
            QJsonObject obj;
            obj["name"] = pl.name;
            obj["desc"] = pl.description;
            
            // Chuyển vector<int> sang QJsonArray
            QJsonArray ids;
            for (int id : pl.songIDs) {
                ids.append(id);
            }
            obj["ids"] = ids;

            array.append(obj);
        }
        return array;
    }

	void PlaylistLibrary::fromJson(const QJsonArray& jsonArray) {
        // Xóa danh sách hiện tại (bao gồm cả default vừa init trong constructor)
        // để tránh bị trùng lặp khi load lại từ file.
        m_playlists.clear(); 

        for (const auto& val : jsonArray) {
            QJsonObject obj = val.toObject();
            
            PlaylistInfo pl;
            pl.name = obj["name"].toString();
            pl.description = obj["desc"].toString();
            
            QJsonArray ids = obj["ids"].toArray();
            for (const auto& idVal : ids) {
                pl.songIDs.push_back(idVal.toInt());
            }

            m_playlists.push_back(pl);
        }
    }
}