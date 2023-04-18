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
