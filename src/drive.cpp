#include "drive.hpp"

FileSystem::FileSystem(std::shared_ptr<filesystem_proxy> fs_proxy)
{
	this->fs_proxy = fs_proxy;
}

FileSystem::FileSystem(const FileSystem& other)
{
	this->fs_proxy = other.fs_proxy;
}

FileSystem FileSystem::mkfs(void* ptr, uint32 size, uint32 files, uint32 block_size_bytes)
{
	filesystem* fs = init_filesystem(ptr, size, files, block_size_bytes);
	std::shared_ptr<filesystem_proxy> fs_proxy = std::make_shared<filesystem_proxy>(fs);

	return FileSystem(fs_proxy);
}

bool FileSystem::create(std::string file_path)
{
	return create_file_proxy(this->fs_proxy.get(), file_path);
}

bool FileSystem::link(std::string file_path, std::string link_path)
{
	return create_link_proxy(this->fs_proxy.get(), file_path, link_path);
}

bool FileSystem::unlink(std::string file_path)
{
	return unlink_file_proxy(this->fs_proxy.get(), file_path);
}

bool FileSystem::truncate(std::string file_path, uint32 new_size)
{
	return change_file_size_proxy(this->fs_proxy.get(), file_path, new_size);
}

bool FileSystem::stat(std::string path)
{
	filesystem_proxy* fs_proxy = this->fs_proxy.get();
	std::pair<std::string, std::string> file_path_splitted;
	uint32 folder_index;

	if (path[0] == '/') {
		file_path_splitted = split_by_last_slash("/." + path);
		folder_index = get_file_by_path(fs_proxy->fs, file_path_splitted.first, fs_proxy->cwd);
	}
	else {
		file_path_splitted = split_by_last_slash("./" + path);
		folder_index = get_file_by_path(fs_proxy->fs, file_path_splitted.first, fs_proxy->cwd);
	}

	if (folder_index == INDEX_EMPTY) {
		printf("Stat: %s not found.\n", path.c_str());
		return false;
	}

	if (get_file(fs_proxy->fs, folder_index)->type != FILE_FOLDER) {
		printf("Stat: %s not found.\n", path.c_str());
		return false;
	}

	if (file_path_splitted.second.length() > MEMBER_NAME_CHARS) {
		printf("Stat: %s not found.\n", path.c_str());
		return false;
	}

	if (file_path_splitted.second == "") {
		file_path_splitted.second = ".";
	}

	uint8 name[MEMBER_NAME_CHARS] = {};
	MEMBER_NAME_to_chars(file_path_splitted.second, name);

	uint32 index = get_folder_member(fs_proxy->fs, folder_index, name).index;

	if (index == INDEX_EMPTY) {
		printf("Stat: %s not found.\n", path.c_str());
		return false;
	}

	file* f = get_file(fs_proxy->fs, index);

	printf("-------------------------------------------------------\n");
	printf("Stat:   %s\n", path.c_str());
	printf("Actual: %s\n", get_full_path_file(fs_proxy->fs, folder_index, index).c_str());
	printf("Index:  %4d\n", index);
	printf("Size:   %4d\n", f->size);
	printf("nLinks: %4d\n", f->nlink);
	printf("Type:   %s\n", (f->type == FILE_REGULAR ? "REGULAR FILE"
						: f->type == FILE_FOLDER ? "FOLDER"
						: f->type == FILE_SYMLINK ? "SYMLINK"
						: "UNKNOWN"));
	if (f->size > 0) {
		printf("Blocks: ");
		uint32 block = f->map_index;
		while (block != INDEX_EMPTY) {
			printf("#%d ", block);
			block = get_map(fs_proxy->fs, block)->next_map_index;
		}
		printf("\n");
	}
	printf("-------------------------------------------------------\n");

	return true;
}

uint32 FileSystem::open(std::string file_path)
{
	return open_file_proxy(this->fs_proxy.get(), file_path);
}

uint32 FileSystem::read(uint32 fd, uint8* buf, uint32 size)
{
	return read_descriptor_proxy(this->fs_proxy.get(), fd, buf, size);
}

uint32 FileSystem::write(uint32 fd, uint8* buf, uint32 size)
{
	return write_descriptor_proxy(this->fs_proxy.get(), fd, buf, size);
}

int32 FileSystem::seek(uint32 fd, int32 shift)
{
	return seek_descriptor_proxy(this->fs_proxy.get(), fd, shift);
}

bool FileSystem::close(uint32 fd)
{
	return close_descriptor_proxy(this->fs_proxy.get(), fd);
}

bool FileSystem::cd(std::string folder_path)
{
	return go_to_folder_proxy(this->fs_proxy.get(), folder_path);
}

bool FileSystem::mkdir(std::string folder_path)
{
	return create_folder_proxy(this->fs_proxy.get(), folder_path);
}

bool FileSystem::rmdir(std::string folder_path)
{
	return remove_folder_proxy(this->fs_proxy.get(), folder_path);
}

bool FileSystem::symlink(std::string symlink_path, std::string file_path) 
{
	return create_symlink_proxy(this->fs_proxy.get(), symlink_path, file_path);
}

std::vector<std::string> FileSystem::listdir(std::string folder_path)
{
	auto v = std::vector<std::string>();
	filesystem_proxy* fs_proxy = this->fs_proxy.get();

	uint32 folder_index = get_file_by_path(fs_proxy->fs, folder_path, fs_proxy->cwd);
	if (folder_index == INDEX_EMPTY) {
		return v;
	}

	auto it = create_folder_members_iterator(fs_proxy->fs, folder_index);

	while (!is_empty_iterator(&it)) {
		v.push_back(next_iterator(&it).folder_name);
	}

	return v;
}

bool FileSystem::ls(std::string folder_path)
{
	filesystem_proxy* fs_proxy = this->fs_proxy.get();
	uint32 folder_index = get_file_by_path(fs_proxy->fs, folder_path, fs_proxy->cwd);

	if (folder_index == INDEX_EMPTY) {
		printf("ls: %s not found.\n", folder_path.c_str());
		return false;
	}

	printf("-----------------------------------------------\n");
	printf("ls:     %s\n", folder_path.c_str());
	printf("Actual: %s\n", get_full_path(fs_proxy->fs, folder_index).c_str());
	printf("-------------------------------------------------------\n");

	for (std::string& member : this->listdir(folder_path)) {
		assert(member.length() <= MEMBER_NAME_CHARS);

		uint8 name[MEMBER_NAME_CHARS] = {};
		MEMBER_NAME_to_chars(member, name);

		uint32 index = get_folder_member(fs_proxy->fs, folder_index, name).index;
		assert(index != INDEX_EMPTY);
		file* f = get_file(fs_proxy->fs, index);
		printf("%-24s | %-7s | %5d bytes | #%3d\n",
			member.c_str(),
			(f->type == FILE_REGULAR ? "REGULAR"
				: f->type == FILE_FOLDER ? "FOLDER"
				: f->type == FILE_SYMLINK ? "SYMLINK"
				: "UNKNOWN"),
			f->size,
			index
		);
	}
	printf("-------------------------------------------------------\n");

	return true;
}
