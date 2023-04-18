#pragma once

#include "filesystem_proxy.hpp"
#include <string>
#include <memory>

class FileSystem
{
private:
	std::shared_ptr<filesystem_proxy> fs_proxy;

	FileSystem(std::shared_ptr<filesystem_proxy> fs_proxy);

public:
	FileSystem(const FileSystem& other);

	static FileSystem mkfs(void* ptr, uint32 size, uint32 files, uint32 block_size_bytes);

	bool create(std::string file_path);
	bool link(std::string file_path, std::string link_path);
	bool unlink(std::string file_path);
	bool truncate(std::string file_path, uint32 new_size);

	uint32 open(std::string file_path);
	uint32 read(uint32 fd, uint8* buf, uint32 size);
	uint32 write(uint32 fd, uint8* buf, uint32 size);
	int32 seek(uint32 fd, int32 shift);
	bool close(uint32 fd);

	bool cd(std::string folder_path);
	bool mkdir(std::string folder_path);
	bool rmdir(std::string folder_path);
};
