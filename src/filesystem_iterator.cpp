#include "filesystem_iterator.hpp"


folder_members_iterator create_folder_members_iterator(filesystem* fs, uint32 folder_index) 
{
	folder_members_iterator it{};
	it.fs = fs;
	it.folder_index = folder_index;
	it.pos = 0;
	init_descriptor(&it.d, fs, folder_index);
	return it;
}

bool is_empty_iterator(folder_members_iterator* it) 
{
	return get_file(it->fs, it->folder_index)->size == it->d.seek;
}

folder_members_iterator_value next_iterator(folder_members_iterator* it) 
{
	folder_members_iterator_value result{};
	result.pos = it->pos;

	uint8 member_info[MEMBER_SIZE] = {};
	assert(read_descriptor(it->fs, &it->d, member_info, MEMBER_SIZE) == MEMBER_SIZE);

	result.folder_name = MEMBER_NAME_to_string(member_info);
	result.index = *(uint32*)(member_info + MEMBER_NAME_CHARS);

	it->pos++;

	return result;
}
