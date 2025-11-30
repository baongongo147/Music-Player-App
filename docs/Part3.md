# Part 3: Implementing History and Shuffle Features

##  Mục tiêu

Triển khai các tính năng nâng cao cho hệ thống phát nhạc:
- **Playback History**: Cho phép người dùng quay lại bài hát trước đó (Back button).
- **Play Next Queue**: Quản lý danh sách bài hát được đánh dấu để phát tiếp theo.
- **Smart Shuffle**: Phát ngẫu nhiên nhưng không lặp lại ngay lập tức.

---

##  Phân tích

1. **Playback History (Back Button)**:
   - Sử dụng `std::stack<Song>` để lưu lịch sử phát nhạc.
   - Khi một bài hát kết thúc, push vào stack.
   - Phương thức: `Song playPreviousSong()` → pop từ stack và trả về bài hát trước.

2. **Play Next Queue**:
   - Sử dụng `std::queue<Song>` để quản lý danh sách bài hát người dùng muốn phát ngay sau bài hiện tại.
   - Phương thức: `void addToNextQueue(const Song& song)` và `Song getNextSong()`.

3. **Smart Shuffle (Không lặp ngay)**:
   - Tạo một `ShuffleManager`:
     - Tạo `std::vector<Song>` tạm thời và trộn bằng `std::shuffle`.
     - Dùng `std::set<int>` để lưu ID các bài đã phát trong chu kỳ hiện tại.
     - Khi tất cả bài đã phát (set.size() == playlist.size()), xóa set để bắt đầu chu kỳ mới.

---

##  Giải thích lựa chọn cấu trúc dữ liệu

### Vì sao chọn `std::stack` cho Playback History
`std::stack` hoạt động theo nguyên tắc **LIFO (Last-In, First-Out)**, hoàn toàn phù hợp với chức năng "Back". Bài hát vừa phát xong sẽ được đưa lên đỉnh stack, và khi người dùng nhấn quay lại, ta chỉ cần pop phần tử trên cùng để lấy bài hát trước đó. Độ phức tạp cho thao tác push và pop là **O(1)**, đảm bảo hiệu năng tối ưu.

### Vì sao chọn `std::queue` cho Play Next Queue
`std::queue` tuân theo nguyên tắc **FIFO (First-In, First-Out)**, phù hợp với logic "phát tiếp theo". Bài hát được thêm vào hàng đợi sẽ được phát theo thứ tự người dùng thêm vào. Các thao tác `push` (thêm vào cuối) và `pop` (lấy từ đầu) đều có độ phức tạp **O(1)**, giúp quản lý danh sách phát tiếp theo một cách đơn giản và hiệu quả.

### Vì sao chọn `std::set` cho Smart Shuffle
`std::set` được hiện thực bằng cây tìm kiếm cân bằng, cho phép kiểm tra sự tồn tại của một phần tử với độ phức tạp **O(log n)**. Khi phát ngẫu nhiên, ta cần đảm bảo không phát lại bài hát đã phát trong chu kỳ hiện tại. Việc lưu ID bài hát trong `std::set` giúp kiểm tra nhanh và duy trì tính duy nhất. Khi tất cả bài hát đã phát, chỉ cần xóa set để bắt đầu chu kỳ mới. Đây là giải pháp vừa đơn giản vừa hiệu quả cho yêu cầu không lặp ngay.

---

##  Độ phức tạp (Big-O)
- **Playback History (Stack)**: push/pop → O(1).
- **Play Next Queue (Queue)**: enqueue/dequeue → O(1).
- **Smart Shuffle (Set)**: kiểm tra tồn tại → O(log n), thêm/xóa → O(log n).

