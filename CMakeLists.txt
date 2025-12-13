cmake_minimum_required(VERSION 3.16)
project(Mini_Project_Qt LANGUAGES CXX)

# 1. C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

# 2. Enable Qt Auto Tools (Quan trọng cho MOC, UIC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 3. Find Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Multimedia Svg)

# 4. Include directories
# Thêm CMAKE_BINARY_DIR để VSCode tìm thấy file ui_*.h tự sinh (Sửa lỗi đỏ header UI)
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR} 
)

# 5. TỰ ĐỘNG TÌM FILE
# Tìm tất cả file .cpp trong thư mục src/ (Bao gồm cả controller mới)
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Tìm tất cả file .hpp trong thư mục include/ (Bao gồm cả controller mới)
file(GLOB_RECURSE HEADERS
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
)

# Tìm tất cả file .ui trong src/view/ (Bao gồm PersonalView.ui, PlaylistView.ui...)
file(GLOB_RECURSE UI_FILES
    "${CMAKE_SOURCE_DIR}/src/view/*.ui"
)

# File Resources (.qrc)
set(RESOURCE_FILES
	${CMAKE_SOURCE_DIR}/resources/resources.qrc)

# 6. Tạo Executable
add_executable(${PROJECT_NAME}
    ${SOURCES}          # Đã chứa StatisticsManager.cpp
    ${HEADERS}          # Đã chứa StatisticsManager.hpp
    ${UI_FILES}         # Đã chứa 3 file UI
    ${CMAKE_SOURCE_DIR}/main.cpp # File main nằm ngoài src nên phải thêm thủ công
	${RESOURCE_FILES}
    # ĐÃ XÓA 2 DÒNG THỪA Ở ĐÂY để tránh lỗi duplicate
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# 7. Link thư viện Qt
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
	Qt5::Svg
)

# 8. Copy tài nguyên
configure_file(${CMAKE_SOURCE_DIR}/songs.txt ${CMAKE_BINARY_DIR}/songs.txt COPYONLY)

if(EXISTS "${CMAKE_SOURCE_DIR}/resources/media")
    file(COPY ${CMAKE_SOURCE_DIR}/resources/media DESTINATION ${CMAKE_BINARY_DIR})
endif()

# 9. Thiết lập thư mục output
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Kích hoạt tính năng Test của CMake
enable_testing()

# Gọi vào thư mục tests để chạy file CMake con bên trong
add_subdirectory(tests)