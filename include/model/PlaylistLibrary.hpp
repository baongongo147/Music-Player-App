#pragma once
#include <vector>
#include <QString>
#include <vector>
#include <QJsonArray>
#include <QJsonObject>

namespace models {

    // Di chuyển struct PlaylistInfo sang đây
    struct PlaylistInfo {
        QString name;
        QString description;
        std::vector<int> songIDs;
    };

    class PlaylistLibrary {
    public:
        PlaylistLibrary(); 
        // Getter lấy danh sách
        const std::vector<PlaylistInfo>& getPlaylists() const;
		// Hàm thêm playlist do người dùng tạo
		void addPlaylist(const PlaylistInfo& playlist);
		QJsonArray toJson() const;               // Chuyển danh sách hiện tại thành JSON Array
        void fromJson(const QJsonArray& json);   // Nạp dữ liệu từ JSON Array vào list
    private:
        std::vector<PlaylistInfo> m_playlists;
		// Hàm tạo dữ liệu mẫu
        void initDefaultPlaylists(); 
		
    };
}