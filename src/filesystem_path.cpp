#include "filesystem_path.hpp"


std::string __get_full_path(filesystem* fs, uint32 folder_index)
{
	assert(get_file(fs, folder_index)->type == FILE_FOLDER);

	if (folder_index == 0) {
		return "";
	}

	uint32 parent_folder_index = get_folder_member(fs, folder_index, parent_folder_name).index;
	assert(parent_folder_index != INDEX_EMPTY);

	std::string current_folder_name = get_folder_member_name(fs, parent_folder_index, folder_index);

	return __get_full_path(fs, parent_folder_index) + "/" + current_folder_name;
}

std::string get_full_path(filesystem* fs, uint32 folder_index)
{
	if (folder_index == 0) {
		return "/";
	}

	return __get_full_path(fs, folder_index);
}

std::string get_full_path_file(filesystem* fs, uint32 folder_index, uint32 member_index) {
	std::string folder_path = get_full_path(fs, folder_index);
	if (folder_path[folder_path.length() - 1] != '/') {
		folder_path += "/";
	}
	return folder_path + get_folder_member_name(fs, folder_index, member_index);
}

uint32 get_file_index_in_folder(filesystem* fs, uint32 folder_index, std::string file_name) 
{
	if (get_file(fs, folder_index)->type != FILE_FOLDER) {
		return INDEX_EMPTY;
	}

	if (file_name.length() > MEMBER_NAME_CHARS) {
		return INDEX_EMPTY;
	}

	uint8 name[MEMBER_NAME_CHARS] = {};
	MEMBER_NAME_to_chars(file_name, name);

	uint32 file_index = get_folder_member(fs, folder_index, name).index;

	if (file_index == INDEX_EMPTY) {
		return INDEX_EMPTY;
	}

	if (get_file(fs, file_index)->type == FILE_SYMLINK) {
		return get_symlink(fs, folder_index, file_index);
	}

	return file_index;
}

uint32 get_file_by_local_path(filesystem* fs, uint32 folder_index, std::string file_path) 
{
	if (get_file(fs, folder_index)->type != FILE_FOLDER) {
		return INDEX_EMPTY;
	}

	uint32 slash_index = file_path.find('/');
	std::string left_part = file_path.substr(0, slash_index);

	uint32 inner_index = get_file_index_in_folder(fs, folder_index, left_part);

	if (inner_index == INDEX_EMPTY) {
		return INDEX_EMPTY;
	}
	
	if (slash_index == std::string::npos) {
		return inner_index;
	}

	std::string right_part = file_path.substr(slash_index + 1);
	return get_file_by_local_path(fs, inner_index, right_part);
}

uint32 get_file_by_path(filesystem* fs, std::string file_path, uint32 cwd)
{
	if (file_path == "/") {
		return 0;
	}

	if (file_path[0] == '/') {
		return get_file_by_local_path(fs, 0, file_path.substr(1));
	}

	return get_file_by_local_path(fs, cwd, file_path);
}

// Thanks ChatGPT-4
std::pair<std::string, std::string> split_by_last_slash(const std::string& input) {
	size_t lastSlashPos = input.find_last_of('/');

	if (lastSlashPos == std::string::npos) {
		return { input, "" };
	}

	std::string firstPart = input.substr(0, lastSlashPos);
	std::string secondPart = input.substr(lastSlashPos + 1);

	return { firstPart, secondPart };
}

std::pair<uint32, std::string> get_folder_and_file_by_path(filesystem* fs, std::string file_path, uint32 cwd)
{
	std::pair<std::string, std::string> file_path_splitted;
	uint32 folder_index;

	if (file_path.length() == 0) {
		return { INDEX_EMPTY, "" };
	}

	if (file_path[0] == '/') {
		file_path_splitted = split_by_last_slash("/." + file_path);
		folder_index = get_file_by_path(fs, file_path_splitted.first, cwd);
	}
	else {
		file_path_splitted = split_by_last_slash("./" + file_path);
		folder_index = get_file_by_path(fs, file_path_splitted.first, cwd);
	}

	if (folder_index == INDEX_EMPTY) {
		return { INDEX_EMPTY, "" };
	}

	if (get_file(fs, folder_index)->type != FILE_FOLDER) {
		return { INDEX_EMPTY, "" };
	}

	return { folder_index, file_path_splitted.second };
}

uint32 get_symlink(filesystem* fs, uint32 folder_index, uint32 symlink_index) 
{
	file* symlink_file = get_file(fs, symlink_index);
	assert(symlink_file->type == FILE_SYMLINK);

	uint32 symlink_size = symlink_file->size;
	uint8* symlink_path = new uint8[symlink_size + 1]();

	descriptor d;
	init_descriptor(&d, fs, symlink_index);
	assert(read_descriptor(fs, &d, symlink_path, symlink_size) == symlink_size);

	std::string symlink_path_str((char*)symlink_path);
	delete[] symlink_path;

	return get_file_by_path(fs, symlink_path_str, folder_index);
}
