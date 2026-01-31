# 🎵 Modern Music Player (C++ / Qt)

![Project Status](https://img.shields.io/badge/Status-Completed-success)
![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue)
![Framework](https://img.shields.io/badge/Framework-Qt5-green)
![Build](https://img.shields.io/badge/Build-CMake-orange)

Ứng dụng nghe nhạc **Modern Music Player** được xây dựng bằng **C++17** và **Qt 5**, hướng tới giao diện hiện đại (Dark Mode), logic phát nhạc mạnh mẽ và kiến trúc rõ ràng, dễ mở rộng. Dự án áp dụng mô hình **MVC (Model – View – Controller)**, tích hợp Smart Playlist, thống kê thói quen nghe nhạc và hệ thống hàng đợi nâng cao.

---

## 📌 Thông Tin Dự Án

| Hạng Mục           | Thông Tin                                   |
| ------------------ | ------------------------------------------- |
| **Tên đồ án**      | Modern Music Player                         |
| **Người thực hiện** | Ngô Trần Quốc Bảo                          |
| **Ngôn ngữ**       | C++17                                       |
| **Framework**      | Qt 5 (Widgets, Multimedia, Svg)             |
| **Build tool**     | CMake                                       |
| **Testing**        | Google Test (GTest)                         |

---

## 🏗 Kiến Trúc Tổng Thể (MVC)

Dự án được tổ chức chặt chẽ theo mô hình **MVC**, đảm bảo phân tách trách nhiệm rõ ràng:

### 🔹 Model (`include/model`, `src/model`)

* Quản lý dữ liệu và thuật toán nền tảng.
* Các lớp chính: `Song`, `PlaybackQueue`, `PlayNextQueue`, `History`, `SmartPlaylist`, `PlaylistLibrary`.
* Hỗ trợ **JSON Serialization** để lưu/khôi phục trạng thái phiên làm việc.

### 🔹 View (`include/view`, `src/view`)

* Xây dựng giao diện người dùng bằng **Qt Widgets** và **Qt Designer (.ui)**.
* Các màn hình chính:

  * `MainWindow`: trình phát nhạc và điều hướng chính.
  * `PlaylistView`: quản lý playlist tùy chỉnh.
  * `PersonalView`: thống kê và dashboard cá nhân.
* Giao diện **Dark Mode**, responsive, sử dụng Qt StyleSheets (CSS).

### 🔹 Controller (`include/controller`, `src/controller`)

* Trung gian xử lý logic giữa View và Model.
* Các lớp chính:

  * `MusicPlayer`: điều phối toàn bộ logic phát nhạc, hàng đợi, tìm kiếm.
  * `StatsManager`: thu thập và xử lý dữ liệu thống kê.

---

## 📂 Cấu Trúc Thư Mục

```text
Mini_Project_Qt/
├── build/                  # Thư mục build (CMake generate)
├── docs/                   # Tài liệu dự án
├── include/
│   ├── controller/         # Header cho Controller
│   ├── model/              # Header cho Model
│   └── view/               # Header cho View
├── resources/
│   ├── icons/              # Icon, SVG
│   ├── media/              # File nhạc (mp3, wav, ...)
│   └── resources.qrc       # Qt Resource file
├── src/
│   ├── controller/         # Implementation Controller
│   ├── model/              # Implementation Model
│   └── view/               # Implementation View + .ui
├── tests/                  # Unit Test (Google Test)
├── CMakeLists.txt          # CMake cấu hình chính
└── songs.txt               # Mock database bài hát
```

---

## ✨ Tính Năng Chính

* 🎧 **Playback nâng cao**: Play / Pause / Stop / Next / Prev / Loop / Shuffle.
* 📋 **Dual Queue System**: Main Queue + Up Next (Priority Queue).
* 🧠 **Smart Playlist**: Tạo playlist thông minh dựa trên Artist/Album (thuật toán BFS).
* 🔍 **Search nâng cao**: Tìm theo ID, Title, Artist, Album (partial & case-insensitive).
* 📊 **Personal Statistics**:

  * Thời gian nghe nhạc (Today / Week / Month).
  * Top Songs, Top Artists.
  * Lịch sử Recently Played.
* 💾 **Session Restore**: Lưu và khôi phục đầy đủ trạng thái khi mở lại ứng dụng.
* 🎨 **Modern UI**: Dark Mode, layout responsive, custom dialogs.

---

## 🛠 Yêu Cầu Hệ Thống

* **Compiler**: GCC / Clang (C++17) hoặc MSVC.
* **Qt 5**: Core, Widgets, Multimedia, Svg.
* **CMake**: >= 3.16.
* **Google Test** (tùy chọn): để chạy unit test.

---

## 🚀 Hướng Dẫn Build & Chạy

### 1️⃣ Clone hoặc tải source

```bash
cd Music-Player-App
```

### 2️⃣ Build bằng CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 3️⃣ Chạy chương trình

* **Windows**:

  ```bash
  Mini_Project_Qt.exe
  ```
* **Linux / macOS**:

  ```bash
  ./Mini_Project_Qt
  ```

---

## ⚠️ Lưu Ý Quan Trọng

* Thư mục **`resources/media/`** hiện **không chứa sẵn file nhạc**.
* Người dùng cần:

  1. Tải các file nhạc (`.mp3`, `.wav`, ...).
  2. Đặt **tên file khớp với danh sách trong `songs.txt`**.
  3. Copy các file nhạc vào đúng đường dẫn:

     ```text
     resources/media/
     ```
* Đảm bảo `songs.txt` và thư mục `media/` nằm đúng vị trí để ứng dụng load nhạc thành công.

---

## 🧪 Testing

Dự án sử dụng **Google Test** để kiểm thử logic backend:

* PlaybackQueue / PlayNextQueue.
* Shuffle & Loop.
* Search & Smart Playlist.
* Save / Restore Session.

Chạy test:

```bash
cd build
ctest --verbose
```

---

## 📄 License

Dự án được phát hành theo **MIT License**. Xem chi tiết tại file [LICENSE](LICENSE).
