#include <iostream>
#include "model/Utils.hpp"
#include "controller/MusicPlayer.hpp"

// const std::string DATA_FILE = "songs.txt"; 

// int main () {
// 	MusicPlayer player;
// 	MusicLibrary& library = player.getLibrary();
// 	PlaybackQueue& playBack = player.getPlayBack();
// 	PlaybackHistory& history = player.getHistory();
// 	PlayNextQueue& playNext = player.getPlayNext();

//     // ------------------ NẠP DỮ LIỆU TỪ FILE (Thay thế các dòng addSong thủ công) ------------------ 
//     std::cout << "Loading data from " << DATA_FILE << "..." << std::endl;
//     if (!library.loadFromFile(DATA_FILE)) {
//         std::cerr << "Failed to load music library. Exiting." << std::endl;
//         return 1; 
//     }
//     std::cout << "Successfully loaded " << library.getAllSongs().size() << " songs." << std::endl;
    
//     // Khởi tạo trạng thái PlaybackQueue để có dữ liệu test
//     addAlbumToQueue("25", library, playBack); // Thêm Album '25' vào queue

// 	/* ------------------ Test Part 1 ------------------ */
// 	std::cout << "\nTesting Part 1" << std::endl;
// 	std::cout << "Songs in queue after adding '25': "<< std::endl;
// 	for (const auto& song : playBack.getQueue()) {
// 		std::cout << song.title << std::endl;
// 	}
// 	player.playSong();
	
// 	/* ------------------ Test Part 2 ------------------ */
// 	std::cout << "\nTesting Part 2" << std::endl;
//     // Sử dụng ID 1, 6, và Artist 'Adele' từ file 200 bài hát
// 	std::cout << "Find by ID (1): ";
// 	if (auto* s = library.findSongByID(1)) std::cout << s->title << std::endl;
// 	std::cout << "Find by Title (Hello): ";
// 	if (auto* s = library.findSongByTitle("Hello")) std::cout << s->title << std::endl;
// 	std::cout << "Find by Artist (Adele):" << std::endl;
// 	auto findSongs = library.findSongsByArtist("Adele");
// 	for (auto* s : findSongs) {
// 		std::cout << "- " << s->title << std::endl;
// 	}
// 	/* ------------------ Test Part 3 ------------------ */
// 	std::cout << "Testing Part 3" << std::endl;
// 	addAlbumToQueue("21", library, playBack);
// 	std::cout << "Play previous song: " << history.playPreviousSong().title << std::endl;
// 	playNext.addSong(*library.findSongByID(6));
// 	player.playSong();
// 	player.playSong();
// 	std::cout << "Test Shuffle on: " << std::endl;
// 	player.enableShuffle(true);
// 	player.playSong();
// 	player.playSong();
// 	player.playSong();
// 	player.playSong();
// 	player.playSong();
// 	player.playSong();
// 	player.enableShuffle(false);
// 	std::cout << "Test Shuffle off: " << std::endl;
// 	player.playSong();
// 	player.playSong();
// 	player.playSong();
// 	player.playSong();
// 	player.playSong();

// 	/* ------------------ Test Part 4 ------------------ */
// 	std::cout << "Testing Part 4" << std::endl;
// 	player.selectAndPlaySong(5);
// 	std::cout << "Current Song: " << playBack.getCurrentSong().title << std::endl;
// 	std::cout << "Play previous song: " << history.playPreviousSong().title << std::endl;
// 	player.playSong();
// 	std::cout << "Song in queues: "<< std::endl;
// 	for (const auto& song : playBack.getQueue()) {
// 		std::cout << song.title << std::endl;
// 	}
// 	player.playSong();
// 	PlaybackQueue smart = player.createSmartPlaylist(7, 10);
// 	std::cout << "Smart playlist:" << std::endl;
// 	for (const auto& s : smart.getQueue()) {
// 		std::cout << "- " << s.title << " (" << s.artist << ")" << std::endl;
// 	}
	
// 	return 0;
// }







using namespace std;
const string DATA_FILE = "songs.txt";
void showMainMenu() {
   cout << "\n=============== MUSIC PLAYER MENU ===============\n";
   cout << "1. Select Song to Play\n";
   cout << "2. Add Album to Queue\n";
   cout << "3. Next Song\n";
   cout << "4. Previous Song (History)\n";
   cout << "5. Toggle Shuffle\n";
   cout << "6. Smart Playlist\n";
   cout << "7. Find Song\n";
   cout << "8. Show Information\n";
   cout << "0. Exit\n";
   cout << "=================================================\n";
   cout << "Your choice: ";
}
void showFindMenu() {
   cout << "\n--- FIND SONG ---\n";
   cout << "1. By ID\n";
   cout << "2. By Title\n";
   cout << "3. By Artist\n";
   cout << "4. By Album\n";
   cout << "0. Back\n";
   cout << "Choice: ";
}
void showInfoMenu() {
   cout << "\n--- SHOW INFORMATION ---\n";
   cout << "1. Playback Queue\n";
   cout << "2. PlayNext Queue\n";
   cout << "3. Current Song\n";
   cout << "4. Playback History\n";
   cout << "0. Back\n";
   cout << "Choice: ";
}
int main() {
   MusicPlayer player;
   MusicLibrary& library = player.getLibrary();
   PlaybackQueue& playBack = player.getPlayBack();
   PlaybackHistory& history = player.getHistory();
   PlayNextQueue& playNext = player.getPlayNext();
   cout << "Loading data...\n";
   if (!library.loadFromFile(DATA_FILE)) {
       cerr << "Failed to load songs.\n";
       return 1;
   }
   cout << "Loaded " << library.getAllSongs().size() << " songs.\n";
   bool running = true;
   bool shuffleState = false;
   while (running) {
       showMainMenu();
       int choice;
       cin >> choice;
       switch (choice) {
       case 1: { // Select song
           int id;
           cout << "Enter Song ID: ";
           cin >> id;
           player.selectAndPlaySong(id);
           break;
       }
       case 2: { // Add album
           string album;
           cout << "Enter Album Name: ";
           cin.ignore();
           getline(cin, album);
           addAlbumToQueue(album, library, playBack);
           cout << "Added album to queue.\n";
           break;
       }
       case 3: { // Next song
           player.playSong();
           break;
       }
       case 4: { // Previous
           try {
               Song prev = history.playPreviousSong();
               cout << "Previous: " << prev.title << endl;
           } catch (...) {
               cout << "No previous song.\n";
           }
           break;
       }
       case 5: { // Shuffle toggle
           shuffleState = !shuffleState;
           player.enableShuffle(shuffleState);
           cout << "Shuffle is now " << (shuffleState ? "ON" : "OFF") << endl;
           break;
       }
       case 6: { // Smart playlist
           int id, size;
           cout << "Start Song ID: ";
           cin >> id;
           cout << "Max playlist size: ";
           cin >> size;
           PlaybackQueue smart = player.createSmartPlaylist(id, size);
           cout << "\nSMART PLAYLIST:\n";
           for (auto& s : smart.getQueue())
               cout << "- " << s.title << " (" << s.artist << ")\n";
           break;
       }
       case 7: { // Find Song
           showFindMenu();
           int sub;
           cin >> sub;
           switch (sub) {
           case 1: {
               int id;
               cout << "Enter ID: ";
               cin >> id;
               if (auto* s = library.findSongByID(id))
                   cout << s->title << " - " << s->artist << endl;
               else cout << "Not found.\n";
               break;
           }
           case 2: {
               string title;
               cout << "Enter Title: ";
               cin.ignore();
               getline(cin, title);
               if (auto* s = library.findSongByTitle(title))
                   cout << s->title << " - " << s->artist << endl;
               else cout << "Not found.\n";
               break;
           }
           case 3: {
               string artist;
               cout << "Enter artist: ";
               cin.ignore();
               getline(cin, artist);
               auto list = library.findSongsByArtist(artist);
               for (auto* s : list)
                   cout << "- " << s->title << endl;
               break;
           }
           case 4: {
               string album;
               cout << "Enter album: ";
               cin.ignore();
               getline(cin, album);
               auto list = library.findSongsByAlbum(album);
               for (auto* s : list)
                   cout << "- " << s->title << endl;
               break;
           }
           }
           break;
       }
       case 8: { // Show info
           showInfoMenu();
           int sub;
           cin >> sub;
           switch (sub) {
           case 1: { // Playback Queue
               cout << "\nPLAYBACK QUEUE:\n";
               for (auto& s : playBack.getQueue())
                   cout << "- " << s.title << endl;
               break;
           }
           case 2: { // PlayNext Queue
               cout << "\nPLAY NEXT QUEUE:\n";
               PlayNextQueue temp = playNext;
               while (!temp.empty()) {
                   Song s = temp.popNextSong();
                   cout << "- " << s.title << endl;
               }
               break;
           }
           case 3: { // Current Song
               try {
                   Song s = playBack.getCurrentSong();
                   cout << "Current: " << s.title << " - " << s.artist << endl;
               } catch (...) {
                   cout << "No current song.\n";
               }
               break;
           }
           case 4: { // History
               cout << "\nHISTORY:\n";
               for (auto& s : history.getHistory())
                   cout << "- " << s.title << endl;
               break;
           }
           }
           break;
       }
       case 0:
           running = false;
           break;
       default:
           cout << "Invalid choice.\n";
       }
   }
   cout << "Exiting...\n";
   return 0;
}