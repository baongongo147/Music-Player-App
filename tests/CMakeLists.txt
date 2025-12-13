# 1. Tải GoogleTest về bằng FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Ép GTest dùng chung thư viện Runtime với Qt trên Windows để tránh lỗi Linker
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# 2. Tìm các thư viện Qt cần thiết
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Multimedia)

# 3. Trỏ đường dẫn Header (quan trọng vì bạn tách include và src)
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# 4. Danh sách các file nguồn cần test (Model + Controller)
# LƯU Ý: KHÔNG đưa main.cpp và các file View (MainWindow, PlaylistView...) vào đây để tránh lỗi liên quan đến giao diện khi chạy Unit Test.
set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/model/MusicLibrary.cpp
    ${CMAKE_SOURCE_DIR}/src/model/PlaybackQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/model/PlayNextQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/model/PlaybackHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/model/ShuffleManager.cpp
    ${CMAKE_SOURCE_DIR}/src/model/PlaylistLibrary.cpp
    ${CMAKE_SOURCE_DIR}/src/model/SmartPlaylist.cpp
    ${CMAKE_SOURCE_DIR}/src/model/Utils.cpp
    ${CMAKE_SOURCE_DIR}/src/controller/MusicPlayer.cpp
    ${CMAKE_SOURCE_DIR}/src/controller/StatisticsManager.cpp
)

# 5. Tạo file thực thi Test
add_executable(MusicPlayerTests
    tst_MusicPlayer.cpp
    ${TEST_SOURCES}
)

# 6. Link thư viện
target_link_libraries(MusicPlayerTests
    PRIVATE
    GTest::gtest_main  # Thư viện GTest chính
    Qt5::Core 
    Qt5::Widgets 
    Qt5::Multimedia
)

# 7. Đăng ký với CTest
include(GoogleTest)
gtest_discover_tests(MusicPlayerTests)