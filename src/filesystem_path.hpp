#pragma once

#include "filesystem_folders.hpp"
#include <string>

std::string __get_full_path(filesystem* fs, uint32 folder_index);

std::string get_full_path(filesystem* fs, uint32 folder_index);

std::string get_full_path_file(filesystem* fs, uint32 folder_index, uint32 member_index);

uint32 get_file_index_in_folder(filesystem* fs, uint32 folder_index, std::string file_name);

uint32 get_file_by_local_path(filesystem* fs, uint32 folder_index, std::string file_path);

uint32 get_file_by_path(filesystem* fs, std::string file_path, uint32 cwd);

std::pair<std::string, std::string> split_by_last_slash(const std::string& input);

std::pair<uint32, std::string> get_folder_and_file_by_path(filesystem* fs, std::string file_path, uint32 cwd);

uint32 get_symlink(filesystem* fs, uint32 folder_index, uint32 symlink_index);
