#include "filesystem_proxy.hpp"
#include "filesystem_path.hpp"


bool create_file_proxy(filesystem_proxy* fs_proxy, std::string file_path)
{
	if (file_path.length() == 0) {
		return false;
	}

	std::pair<std::string, std::string> file_path_splitted;
	uint32 folder_index;

	if (file_path[0] == '/') {
		file_path_splitted = split_by_last_slash("/." + file_path);
		folder_index = get_file_by_path(fs_proxy->fs, file_path_splitted.first, fs_proxy->cwd);
	}
	else {
		file_path_splitted = split_by_last_slash("./" + file_path);
		folder_index = get_file_by_path(fs_proxy->fs, file_path_splitted.first, fs_proxy->cwd);
	}

	if (folder_index == INDEX_EMPTY) {
		return false;
	}

	if (get_file(fs_proxy->fs, folder_index)->type != FILE_FOLDER) {
		return false;
	}

	if (file_path_splitted.second.length() > MEMBER_NAME_CHARS) {
		return false;
	}

	uint8 name[MEMBER_NAME_CHARS] = {};
	MEMBER_NAME_to_chars(file_path_splitted.second, name);

	uint32 index = create_file_in_folder(fs_proxy->fs, folder_index, name);

	if (index == FILE_CREATE_UNSUCCESSFUL) {
		return false;
	}

	return true;
}

bool create_folder_proxy(filesystem_proxy* fs_proxy, std::string folder_path)
{
	if (folder_path.length() == 0) {
		return false;
	}

	std::pair<std::string, std::string> folder_path_splitted;
	uint32 folder_index;

	if (folder_path[0] == '/') {
		folder_path_splitted = split_by_last_slash("/." + folder_path);
		folder_index = get_file_by_path(fs_proxy->fs, folder_path_splitted.first, fs_proxy->cwd);
	}
	else {
		folder_path_splitted = split_by_last_slash("./" + folder_path);
		folder_index = get_file_by_path(fs_proxy->fs, folder_path_splitted.first, fs_proxy->cwd);
	}

	if (folder_index == INDEX_EMPTY) {
		return false;
	}

	if (get_file(fs_proxy->fs, folder_index)->type != FILE_FOLDER) {
		return false;
	}

	if (folder_path_splitted.second.length() > MEMBER_NAME_CHARS) {
		return false;
	}

	uint8 name[MEMBER_NAME_CHARS] = {};
	MEMBER_NAME_to_chars(folder_path_splitted.second, name);

	uint32 index = create_folder_in_folder(fs_proxy->fs, folder_index, name);

	if (index == FILE_CREATE_UNSUCCESSFUL) {
		return false;
	}

	return true;
}

bool go_to_folder_proxy(filesystem_proxy* fs_proxy, std::string folder_path) 
{
	if (folder_path == "/") {
		fs_proxy->cwd = 0;
		return true;
	}

	uint32 folder_index = get_file_by_path(fs_proxy->fs, folder_path, fs_proxy->cwd);

	if (folder_index == INDEX_EMPTY) {
		return false;
	}

	if (get_file(fs_proxy->fs, folder_index)->type != FILE_FOLDER) {
		return false;
	}

	fs_proxy->cwd = folder_index;
	return true;
}

uint32 open_file_proxy(filesystem_proxy* fs_proxy, std::string file_path)
{
	uint32 file_index = get_file_by_path(fs_proxy->fs, file_path, fs_proxy->cwd);

	if (file_index == INDEX_EMPTY) {
		return INDEX_EMPTY;
	}

	if (get_file(fs_proxy->fs, file_index)->type != FILE_REGULAR) {
		return INDEX_EMPTY;
	}

	descriptor d{};
	init_descriptor(&d, fs_proxy->fs, file_index);

	for (uint32 index = 0; index < MAX_DESCRIPTORS; index++) {
		if (!fs_proxy->slots[index]) 
		{
			fs_proxy->slots[index] = true;
			fs_proxy->descriptors[index] = d;
			return index;
		}
	}

	return INDEX_EMPTY;
}

uint32 read_descriptor_proxy(filesystem_proxy* fs_proxy, uint32 descriptor_index, uint8* buf, uint32 size) 
{
	if (descriptor_index >= MAX_DESCRIPTORS) {
		return INDEX_EMPTY;
	}

	if (!fs_proxy->slots[descriptor_index]) {
		return INDEX_EMPTY;
	}

	descriptor* d = &fs_proxy->descriptors[descriptor_index];
	return read_descriptor(fs_proxy->fs, d, buf, size);
}

uint32 write_descriptor_proxy(filesystem_proxy* fs_proxy, uint32 descriptor_index, uint8* buf, uint32 size)
{
	if (descriptor_index >= MAX_DESCRIPTORS) {
		return INDEX_EMPTY;
	}

	if (!fs_proxy->slots[descriptor_index]) {
		return INDEX_EMPTY;
	}

	descriptor* d = &fs_proxy->descriptors[descriptor_index];
	return write_descriptor(fs_proxy->fs, d, buf, size);
}

int32 seek_descriptor_proxy(filesystem_proxy* fs_proxy, uint32 descriptor_index, int32 shift) 
{
	if (descriptor_index >= MAX_DESCRIPTORS) {
		return INDEX_EMPTY;
	}

	if (!fs_proxy->slots[descriptor_index]) {
		return INDEX_EMPTY;
	}

	descriptor* d = &fs_proxy->descriptors[descriptor_index];
	return seek_descriptor(fs_proxy->fs, d, shift);
}

bool close_descriptor_proxy(filesystem_proxy* fs_proxy, uint32 descriptor_index) 
{
	if (descriptor_index >= MAX_DESCRIPTORS) {
		return false;
	}

	if (!fs_proxy->slots[descriptor_index]) {
		return false;
	}

	fs_proxy->slots[descriptor_index] = false;
	return true;
}

bool change_file_size_proxy(filesystem_proxy* fs_proxy, std::string file_path, uint32 new_size)
{
	uint32 file_index = get_file_by_path(fs_proxy->fs, file_path, fs_proxy->cwd);

	if (file_index == INDEX_EMPTY) {
		return false;
	}

	if (get_file(fs_proxy->fs, file_index)->type != FILE_REGULAR) {
		return false;
	}

	return change_file_size(fs_proxy->fs, get_file(fs_proxy->fs, file_index), new_size);
}

bool create_link_proxy(filesystem_proxy* fs_proxy, std::string file_path, std::string link_path) 
{
	uint32 file_index = get_file_by_path(fs_proxy->fs, file_path, fs_proxy->cwd);

	if (file_index == INDEX_EMPTY) {
		return false;
	}

	if (get_file(fs_proxy->fs, file_index)->type != FILE_REGULAR) {
		return false;
	}

	if (link_path.length() == 0) {
		return false;
	}

	std::pair<std::string, std::string> link_path_splitted;
	uint32 link_folder_index;

	if (link_path[0] == '/') {
		link_path_splitted = split_by_last_slash("/." + link_path);
		link_folder_index = get_file_by_path(fs_proxy->fs, link_path_splitted.first, fs_proxy->cwd);
	}
	else {
		link_path_splitted = split_by_last_slash("./" + link_path);
		link_folder_index = get_file_by_path(fs_proxy->fs, link_path_splitted.first, fs_proxy->cwd);
	}

	if (link_folder_index == INDEX_EMPTY) {
		return false;
	}

	if (get_file(fs_proxy->fs, link_folder_index)->type != FILE_FOLDER) {
		return false;
	}

	if (link_path_splitted.second.length() > MEMBER_NAME_CHARS) {
		return false;
	}

	uint8 name[MEMBER_NAME_CHARS] = {};
	MEMBER_NAME_to_chars(link_path_splitted.second, name);

	if (create_link_in_folder(fs_proxy->fs, link_folder_index, file_index, name) != LINK_CREATE_SUCCESSFUL) {
		return false;
	}

	return true;
}

bool unlink_file_proxy(filesystem_proxy* fs_proxy, std::string file_path) 
{
	auto file_indexes = get_folder_and_file_by_path(fs_proxy->fs, file_path, fs_proxy->cwd);

	if (file_indexes.first == INDEX_EMPTY) {
		return false;
	}

	if (file_indexes.second.length() > MEMBER_NAME_CHARS) {
		return false;
	}

	uint8 name[MEMBER_NAME_CHARS] = {};
	MEMBER_NAME_to_chars(file_indexes.second, name);

	if (delete_link_file_in_folder(fs_proxy->fs, file_indexes.first, name) != UNLINK_SUCCESSFUL) {
		return false;
	}

	return true;
}

bool remove_folder_proxy(filesystem_proxy* fs_proxy, std::string folder_path)
{
	auto file_indexes = get_folder_and_file_by_path(fs_proxy->fs, folder_path, fs_proxy->cwd);

	if (file_indexes.first == INDEX_EMPTY) {
		return false;
	}

	if (file_indexes.second.length() > MEMBER_NAME_CHARS) {
		return false;
	}

	uint8 name[MEMBER_NAME_CHARS] = {};
	MEMBER_NAME_to_chars(file_indexes.second, name);

	if (delete_link_folder_in_folder(fs_proxy->fs, file_indexes.first, name) != UNLINK_SUCCESSFUL) {
		return false;
	}

	return true;
}
