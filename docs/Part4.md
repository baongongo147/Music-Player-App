# Part 4: Integration and Advanced Algorithm
 
## Mục tiêu
Tích hợp các thành phần đã xây dựng (MusicLibrary, PlaybackQueue, PlaybackHistory) và triển khai thuật toán **Smart Playlist** dựa trên BFS để tạo danh sách phát thông minh.
 
---
 
## Phân tích
 
1. **System Integration (MusicPlayer Class)**:
   - Quản lý các đối tượng: `MusicLibrary`, `PlaybackQueue`, `PlaybackHistory`.
   - Phương thức chính: `void selectAndPlaySong(int songID)`:
     - Tìm bài hát bằng `findSongByID` từ MusicLibrary.
     - Nếu có bài đang phát, đẩy vào PlaybackHistory.
     - Đặt bài mới làm bài hiện tại và thêm vào PlaybackQueue.
 
2. **Smart Playlist Algorithm**:
   - Hàm: `PlaybackQueue generateSmartPlaylist(const Song& startSong, const MusicLibrary& library, int maxSize)`.
   - Ý tưởng: Tạo playlist thông minh gồm bài hát cùng nghệ sĩ hoặc cùng album.
   - Thuật toán:
     - Dùng cấu trúc **Breadth-First Search (BFS)**.
     - Khởi tạo với `startSong`.
     - Sử dụng `std::queue` để lưu các bài cần khám phá.
     - Sử dụng `std::set<int>` để đánh dấu ID bài hát đã thêm vào playlist (tránh trùng lặp).
     - "Neighbor" của một bài hát: tất cả bài cùng nghệ sĩ hoặc cùng album (tra cứu nhanh bằng index từ Part 2).
     - Lặp BFS cho đến khi playlist đạt `maxSize`.
 
---
 
## Giải thích lựa chọn cấu trúc dữ liệu
 
### Vì sao dùng `std::queue` trong BFS
`std::queue` tuân theo nguyên tắc **FIFO (First-In, First-Out)**, phù hợp với BFS vì ta cần xử lý các bài hát theo thứ tự lớp (level order). Mỗi bài hát được thêm vào hàng đợi sẽ được khám phá sau khi các bài trước đó hoàn tất. Thao tác `push` và `pop` đều có độ phức tạp **O(1)**, đảm bảo hiệu năng tốt cho thuật toán.
 
### Vì sao dùng `std::set<int>` để đánh dấu bài hát đã thêm
`std::set` cho phép kiểm tra sự tồn tại và thêm phần tử với độ phức tạp **O(log n)**. Điều này giúp tránh thêm trùng lặp vào playlist, đồng thời duy trì tính duy nhất của ID bài hát. Khi playlist đạt kích thước tối đa hoặc tất cả bài đã được thêm, ta có thể dễ dàng kiểm soát và reset tập hợp.
 
---
 
## Độ phức tạp (Big-O)
- **selectAndPlaySong**:
  - Tìm kiếm bài hát theo ID: O(1) trung bình (unordered_map).
  - Thêm vào PlaybackQueue: O(1).
  - Đẩy vào PlaybackHistory: O(1).
- **generateSmartPlaylist (BFS)**:
  - Mỗi bài hát được xử lý một lần → O(n) với n là số bài hát trong playlist.
  - Kiểm tra và thêm vào set: O(log n) mỗi lần.
  - Tổng thể: O(n log n) trong trường hợp xấu nhất.
 
 
 