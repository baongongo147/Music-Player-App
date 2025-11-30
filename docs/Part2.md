# Part 2: Accelerating Searches and Indexing Metadata

##  Mục tiêu
Tăng tốc độ tìm kiếm và lập chỉ mục metadata cho kho nhạc lớn (hàng chục nghìn bài hát) bằng cách sử dụng các cấu trúc dữ liệu tối ưu:
- **std::unordered_map<int, Song*>**: Tìm kiếm bài hát theo ID với hiệu năng cao.
- **std::map<std::string, Song*>**: Tìm kiếm bài hát theo tiêu đề và duy trì thứ tự sắp xếp.
- **std::unordered_map<std::string, std::vector<Song*>>**: Lập chỉ mục nghệ sĩ để truy xuất nhanh tất cả bài hát của một nghệ sĩ.

---

##  Phân tích

1. **Search by ID (Hiệu năng quan trọng)**:
   - Thêm thành viên dữ liệu: `std::unordered_map<int, Song*> songIndexByID;`.
   - Key: ID bài hát, Value: con trỏ tới đối tượng `Song` trong `std::vector`.
   - Phương thức: `Song* findSongByID(int id)`.

2. **Search by Title và Sorting**:
   - Thêm thành viên dữ liệu: `std::map<std::string, Song*> songIndexByTitle;`.
   - Key: tiêu đề bài hát, Value: con trỏ tới `Song`.
   - Phương thức: `Song* findSongByTitle(const std::string& title)`.

3. **Search by Artist (Nhiều bài hát)**:
   - Thêm thành viên dữ liệu: `std::unordered_map<std::string, std::vector<Song*>> artistIndex;`.
   - Key: tên nghệ sĩ, Value: danh sách con trỏ tới các bài hát.

---

##  Giải thích lựa chọn cấu trúc dữ liệu

Trong Part 2, mỗi loại truy vấn có yêu cầu khác nhau về hiệu năng và thứ tự:
- **ID và nghệ sĩ**: chỉ cần tốc độ, không yêu cầu sắp xếp → dùng `std::unordered_map` để đạt độ phức tạp trung bình **O(1)** cho tìm kiếm.
- **Tiêu đề bài hát**: cần tìm kiếm và hiển thị theo thứ tự alphabet → dùng `std::map` để vừa tìm kiếm **O(log n)** vừa duy trì thứ tự khóa tự động.

Cụ thể:
- `songIndexByID: std::unordered_map<int, Song*>` → tra cứu ID cực nhanh, không cần thứ tự.
- `songIndexByTitle: std::map<std::string, Song*>` → tra cứu theo tiêu đề và hiển thị đã sắp xếp.
- `artistIndex: std::unordered_map<std::string, std::vector<Song*>>` → lấy nhanh danh sách bài hát của nghệ sĩ, không cần thứ tự toàn cục.

### Vì sao chọn `std::unordered_map<int, Song*>` cho tìm kiếm theo ID
`std::unordered_map` là cấu trúc dữ liệu dựa trên bảng băm (hash table), cung cấp độ phức tạp trung bình **O(1)** cho thao tác tìm kiếm, chèn và xóa. Với yêu cầu tìm kiếm theo ID diễn ra thường xuyên và cần hiệu năng cao, việc sử dụng hash map giúp giảm đáng kể thời gian truy xuất so với tìm kiếm tuyến tính trong `vector` (O(n)). Đây là lựa chọn tối ưu cho bài toán hiệu năng quan trọng.

### Vì sao chọn `std::map<std::string, Song*>` cho tìm kiếm theo tiêu đề
`std::map` được hiện thực bằng cây tìm kiếm cân bằng (thường là Red-Black Tree), đảm bảo độ phức tạp **O(log n)** cho thao tác tìm kiếm và chèn. Ngoài ra, `map` tự động duy trì thứ tự sắp xếp của các khóa, rất hữu ích khi cần hiển thị danh sách bài hát theo thứ tự bảng chữ cái. Điều này giúp kết hợp tìm kiếm và sắp xếp trong một cấu trúc duy nhất, tối ưu cho yêu cầu quản lý tiêu đề.

### Vì sao chọn `std::unordered_map<std::string, std::vector<Song*>>` cho lập chỉ mục nghệ sĩ
Một nghệ sĩ có thể có nhiều bài hát, do đó cần một cấu trúc cho phép truy xuất nhanh danh sách bài hát theo tên nghệ sĩ. `std::unordered_map` cung cấp độ phức tạp trung bình **O(1)** cho việc tìm kiếm theo khóa (tên nghệ sĩ), trong khi giá trị là một `std::vector<Song*>` lưu trữ tất cả bài hát của nghệ sĩ đó. Cách tiếp cận này giúp giảm chi phí tìm kiếm và hỗ trợ duyệt nhanh danh sách bài hát liên quan.

---

##  Độ phức tạp (Big-O)
- **Tìm kiếm theo ID**: `unordered_map` → O(1) trung bình.
- **Tìm kiếm theo tiêu đề**: `map` → O(log n).
- **Tìm kiếm theo nghệ sĩ**: `unordered_map` → O(1) trung bình, duyệt danh sách bài hát O(k) với k là số bài hát của nghệ sĩ.


