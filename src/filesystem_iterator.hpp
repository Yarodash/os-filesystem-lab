#pragma once

#include "filesystem_folders.hpp"

struct folder_members_iterator {
	uint32 folder_index;
	uint32 pos;
	descriptor d = {};
};

struct folder_members_iterator_value {
	uint32 pos;
	std::string folder_name;
	uint32 index;
};

folder_members_iterator create_folder_members_iterator(filesystem* fs, uint32 folder_index);

bool is_empty_iterator(filesystem* fs, folder_members_iterator* it);

folder_members_iterator_value next_iterator(filesystem* fs, folder_members_iterator* it);
