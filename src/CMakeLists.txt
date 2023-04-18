add_library(drive_library filesystem_blocks.cpp filesystem_folders.cpp filesystem_io.cpp filesystem_iterator.cpp filesystem_path.cpp filesystem_proxy.cpp drive.cpp)

target_include_directories(drive_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
