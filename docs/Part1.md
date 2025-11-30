# Part 1: Music Library and Playback Queue Management

## Mục tiêu
Xây dựng hai lớp cốt lõi cho hệ thống phát nhạc:
- **MusicLibrary Class**: Quản lý toàn bộ kho nhạc đã nạp khi khởi động, sử dụng `std::vector`.
- **PlaybackQueue Class**: Quản lý hàng đợi phát động, hỗ trợ chèn/xóa linh hoạt, sử dụng `std::list`.

---

## Phân tích

1. **Struct `Song`**: Cấu trúc dữ liệu lưu trữ thông tin cơ bản của một bài hát, gồm các thuộc tính: `id`, `title`, `artist`, `album`, `duration`.

2. **MusicLibrary Class**: Lưu toàn bộ bài hát đã nạp, sử dụng `std::vector<Song>` để hỗ trợ truy cập nhanh theo chỉ số.

3. **PlaybackQueue Class**: Quản lý danh sách bài hát đang chờ phát theo thứ tự phát; hỗ trợ chèn/xóa giữa danh sách hiệu quả, cập nhật bài hiện tại, chuyển bài tiếp theo. Sử dụng `std::list<Song>`. Các phương thức chính:
   - `void addSong(const Song& song)` — thêm bài vào cuối hàng đợi.
   - `void removeSong(int songID)` — xóa bài theo `id` ở bất kỳ vị trí.
   - `Song getCurrentSong()` — trả về bài hiện tại.
   - `void playNext()` — chuyển sang bài kế tiếp.

4. **Hàm `addAlbumToQueue()`**: Nạp toàn bộ bài hát thuộc một album vào `PlaybackQueue`.

---

## Giải thích lựa chọn cấu trúc dữ liệu

### Vì sao chọn `std::vector<Song>` cho MusicLibrary
`std::vector` là lựa chọn tối ưu để lưu trữ kho nhạc vì nó cung cấp khả năng truy cập ngẫu nhiên với độ phức tạp **O(1)**, cho phép lấy bài hát theo chỉ số tức thời. Dữ liệu được lưu trong bộ nhớ liên tục, tận dụng hiệu quả cache CPU, giúp tăng tốc độ duyệt toàn bộ danh sách khi hiển thị hoặc tìm kiếm. Ngoài ra, kho nhạc được nạp một lần khi khởi động và hầu như không thay đổi trong quá trình sử dụng, nên chi phí chèn/xóa ở giữa (O(n)) không ảnh hưởng đáng kể. Việc thêm bài hát vào cuối cũng chỉ tốn **O(1)** trung bình, đảm bảo hiệu năng tốt cho yêu cầu hệ thống.

### Vì sao chọn `std::list<Song>` cho PlaybackQueue
`std::list` phù hợp cho hàng đợi phát nhạc vì nó hỗ trợ chèn và xóa ở bất kỳ vị trí nào với độ phức tạp **O(1)** khi đã có iterator, điều này rất quan trọng khi người dùng thường xuyên thêm hoặc xóa bài hát trong danh sách đang phát. Một ưu điểm nổi bật của `list` là iterator và reference tới các phần tử không bị vô hiệu hóa khi thêm hoặc xóa phần tử khác, giúp duy trì trạng thái bài hát hiện tại một cách an toàn. Mặc dù `list` không hỗ trợ truy cập ngẫu nhiên, điều này không phải vấn đề vì hàng đợi phát được xử lý tuần tự. Do đó, `std::list` đáp ứng tốt yêu cầu về tính linh hoạt và ổn định cho việc quản lý hàng đợi phát động.


