# Part 1: Music Library and Playback Queue Management

## Mục tiêu
Xây dựng hai lớp cốt lõi cho hệ thống phát nhạc:
- **MusicLibrary Class**: Quản lý toàn bộ kho nhạc đã nạp khi khởi động, sử dụng `std::vector`.
- **PlaybackQueue Class**: Quản lý hàng đợi phát động, hỗ trợ chèn/xóa linh hoạt, sử dụng `std::list`.

## Phân tích

1. **Struct `Song`**: Cấu trúc dữ liệu lưu trữ thông tin cơ bản của một bài hát, gồm các thuộc tính: `id`, `title`, `artist`, `album`, `duration`.

2. **MusicLibrary Class**: Lưu toàn bộ bài hát đã nạp, sử dụng `std::vector<Song>` để hỗ trợ truy cập nhanh theo chỉ số.

3. **PlaybackQueue Class**: Quản lý danh sách bài hát đang chờ phát theo thứ tự phát; hỗ trợ chèn/xóa giữa danh sách hiệu quả, cập nhật bài hiện tại, chuyển bài tiếp theo. Sử dụng `std::list<Song>`. Các phương thức chính:
   - `void addSong(const Song& song)` — thêm bài vào cuối hàng đợi.
   - `void removeSong(int songID)` — xóa bài theo `id` ở bất kỳ vị trí.
   - `Song getCurrentSong()` — trả về bài hiện tại.
   - `void playNext()` — chuyển sang bài kế tiếp.

4. **Hàm `addAlbumToQueue()`**: Nạp toàn bộ bài hát thuộc một album vào `PlaybackQueue`.


## Giải thích lựa chọn cấu trúc dữ liệu

### Vì sao chọn `std::vector<Song>` cho MusicLibrary
`std::vector` là lựa chọn tối ưu để lưu trữ kho nhạc vì nó cung cấp khả năng truy cập ngẫu nhiên với độ phức tạp **O(1)**, cho phép lấy bài hát theo chỉ số tức thời. Dữ liệu được lưu trong bộ nhớ liên tục, tận dụng hiệu quả cache CPU, giúp tăng tốc độ duyệt toàn bộ danh sách khi hiển thị hoặc tìm kiếm. Ngoài ra, kho nhạc được nạp một lần khi khởi động và hầu như không thay đổi trong quá trình sử dụng, nên chi phí chèn/xóa ở giữa (O(n)) không ảnh hưởng đáng kể. Việc thêm bài hát vào cuối cũng chỉ tốn **O(1)** trung bình, đảm bảo hiệu năng tốt cho yêu cầu hệ thống.

### Vì sao chọn `std::list<Song>` cho PlaybackQueue
`std::list` phù hợp cho hàng đợi phát nhạc vì nó hỗ trợ chèn và xóa ở bất kỳ vị trí nào với độ phức tạp **O(1)** khi đã có iterator, điều này rất quan trọng khi người dùng thường xuyên thêm hoặc xóa bài hát trong danh sách đang phát. Một ưu điểm nổi bật của `list` là iterator và reference tới các phần tử không bị vô hiệu hóa khi thêm hoặc xóa phần tử khác, giúp duy trì trạng thái bài hát hiện tại một cách an toàn. Mặc dù `list` không hỗ trợ truy cập ngẫu nhiên, điều này không phải vấn đề vì hàng đợi phát được xử lý tuần tự. Do đó, `std::list` đáp ứng tốt yêu cầu về tính linh hoạt và ổn định cho việc quản lý hàng đợi phát động.

---

# Part 2: Accelerating Searches and Indexing Metadata

##  Mục tiêu
Tăng tốc độ tìm kiếm và lập chỉ mục metadata cho kho nhạc lớn (hàng chục nghìn bài hát) bằng cách sử dụng các cấu trúc dữ liệu tối ưu:
- **std::unordered_map<int, Song*>**: Tìm kiếm bài hát theo ID với hiệu năng cao.
- **std::map<std::string, Song*>**: Tìm kiếm bài hát theo tiêu đề và duy trì thứ tự sắp xếp.
- **std::unordered_map<std::string, std::vector<Song*>>**: Lập chỉ mục nghệ sĩ để truy xuất nhanh tất cả bài hát của một nghệ sĩ.


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


##  Độ phức tạp (Big-O)
- **Tìm kiếm theo ID**: `unordered_map` → O(1) trung bình.
- **Tìm kiếm theo tiêu đề**: `map` → O(log n).
- **Tìm kiếm theo nghệ sĩ**: `unordered_map` → O(1) trung bình, duyệt danh sách bài hát O(k) với k là số bài hát của nghệ sĩ.

---

# Part 3: Implementing History and Shuffle Features

##  Mục tiêu

Triển khai các tính năng nâng cao cho hệ thống phát nhạc:
- **Playback History**: Cho phép người dùng quay lại bài hát trước đó (Back button).
- **Play Next Queue**: Quản lý danh sách bài hát được đánh dấu để phát tiếp theo.
- **Smart Shuffle**: Phát ngẫu nhiên nhưng không lặp lại ngay lập tức.


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



##  Giải thích lựa chọn cấu trúc dữ liệu

### Vì sao chọn `std::stack` cho Playback History
`std::stack` hoạt động theo nguyên tắc **LIFO (Last-In, First-Out)**, hoàn toàn phù hợp với chức năng "Back". Bài hát vừa phát xong sẽ được đưa lên đỉnh stack, và khi người dùng nhấn quay lại, ta chỉ cần pop phần tử trên cùng để lấy bài hát trước đó. Độ phức tạp cho thao tác push và pop là **O(1)**, đảm bảo hiệu năng tối ưu.

### Vì sao chọn `std::queue` cho Play Next Queue
`std::queue` tuân theo nguyên tắc **FIFO (First-In, First-Out)**, phù hợp với logic "phát tiếp theo". Bài hát được thêm vào hàng đợi sẽ được phát theo thứ tự người dùng thêm vào. Các thao tác `push` (thêm vào cuối) và `pop` (lấy từ đầu) đều có độ phức tạp **O(1)**, giúp quản lý danh sách phát tiếp theo một cách đơn giản và hiệu quả.

### Vì sao chọn `std::set` cho Smart Shuffle
`std::set` được hiện thực bằng cây tìm kiếm cân bằng, cho phép kiểm tra sự tồn tại của một phần tử với độ phức tạp **O(log n)**. Khi phát ngẫu nhiên, ta cần đảm bảo không phát lại bài hát đã phát trong chu kỳ hiện tại. Việc lưu ID bài hát trong `std::set` giúp kiểm tra nhanh và duy trì tính duy nhất. Khi tất cả bài hát đã phát, chỉ cần xóa set để bắt đầu chu kỳ mới. Đây là giải pháp vừa đơn giản vừa hiệu quả cho yêu cầu không lặp ngay.


##  Độ phức tạp (Big-O)
- **Playback History (Stack)**: push/pop → O(1).
- **Play Next Queue (Queue)**: enqueue/dequeue → O(1).
- **Smart Shuffle (Set)**: kiểm tra tồn tại → O(log n), thêm/xóa → O(log n).

---

# Part 4: Integration and Advanced Algorithm
 
## Mục tiêu
Tích hợp các thành phần đã xây dựng (MusicLibrary, PlaybackQueue, PlaybackHistory) và triển khai thuật toán **Smart Playlist** dựa trên BFS để tạo danh sách phát thông minh.
 
 
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
 
 
## Giải thích lựa chọn cấu trúc dữ liệu
 
### Vì sao dùng `std::queue` trong BFS
`std::queue` tuân theo nguyên tắc **FIFO (First-In, First-Out)**, phù hợp với BFS vì ta cần xử lý các bài hát theo thứ tự lớp (level order). Mỗi bài hát được thêm vào hàng đợi sẽ được khám phá sau khi các bài trước đó hoàn tất. Thao tác `push` và `pop` đều có độ phức tạp **O(1)**, đảm bảo hiệu năng tốt cho thuật toán.
 
### Vì sao dùng `std::set<int>` để đánh dấu bài hát đã thêm
`std::set` cho phép kiểm tra sự tồn tại và thêm phần tử với độ phức tạp **O(log n)**. Điều này giúp tránh thêm trùng lặp vào playlist, đồng thời duy trì tính duy nhất của ID bài hát. Khi playlist đạt kích thước tối đa hoặc tất cả bài đã được thêm, ta có thể dễ dàng kiểm soát và reset tập hợp.
 
 
## Độ phức tạp (Big-O)
- **selectAndPlaySong**:
  - Tìm kiếm bài hát theo ID: O(1) trung bình (unordered_map).
  - Thêm vào PlaybackQueue: O(1).
  - Đẩy vào PlaybackHistory: O(1).
- **generateSmartPlaylist (BFS)**:
  - Mỗi bài hát được xử lý một lần → O(n) với n là số bài hát trong playlist.
  - Kiểm tra và thêm vào set: O(log n) mỗi lần.
  - Tổng thể: O(n log n) trong trường hợp xấu nhất.
 
 
---