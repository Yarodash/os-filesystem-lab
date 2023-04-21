#pragma once

#include "filesystem_folders.hpp"
#include "filesystem_path.hpp"

#define MAX_DESCRIPTORS 16

struct filesystem_proxy {
	filesystem* fs;
	descriptor descriptors[MAX_DESCRIPTORS] = {};
	bool slots[MAX_DESCRIPTORS] = {};
	uint32 cwd = 0;

	filesystem_proxy(filesystem* fs) : fs(fs) {}
};

bool create_file_proxy(filesystem_proxy* fs_proxy, std::string file_path);

bool create_folder_proxy(filesystem_proxy* fs_proxy, std::string folder_path);

bool go_to_folder_proxy(filesystem_proxy* fs_proxy, std::string folder_path);

uint32 open_file_proxy(filesystem_proxy* fs_proxy, std::string file_path);

uint32 read_descriptor_proxy(filesystem_proxy* fs_proxy, uint32 descriptor_index, uint8* buf, uint32 size);

uint32 write_descriptor_proxy(filesystem_proxy* fs_proxy, uint32 descriptor_index, uint8* buf, uint32 size);

int32 seek_descriptor_proxy(filesystem_proxy* fs_proxy, uint32 descriptor_index, int32 shift);

bool close_descriptor_proxy(filesystem_proxy* fs_proxy, uint32 descriptor_index);

bool change_file_size_proxy(filesystem_proxy* fs_proxy, std::string file_path, uint32 new_size);

bool create_link_proxy(filesystem_proxy* fs_proxy, std::string file_path, std::string link_path);

bool unlink_file_proxy(filesystem_proxy* fs_proxy, std::string file_path);

bool remove_folder_proxy(filesystem_proxy* fs_proxy, std::string folder_path);

bool create_symlink_proxy(filesystem_proxy* fs_proxy, std::string symlink_path, std::string file_path);
