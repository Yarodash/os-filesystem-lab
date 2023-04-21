#include "filesystem_folders.hpp"


bool check_name(const uint8 name[MEMBER_NAME_CHARS]) 
{
    int32 i = MEMBER_NAME_CHARS - 1;

    for (; i >= 0 && name[i] == '\x0'; i--) {}

    if (i < 0) return false;

    for (; i >= 0; i--) {
        if (name[i] == '\x0' || (!islower(name[i]) && !isdigit(name[i]) && name[i] != '.')) {
            return false;
        }
    }

    return true;
}

member_pos_index get_folder_member(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS])
{
    file* folder = get_file(fs, folder_index);
    assert(folder->type == FILE_FOLDER);

    descriptor d{};
    init_descriptor(&d, fs, folder_index);

    uint32 pos = 0;
    uint8 member_info[MEMBER_SIZE] = {};
    while (read_descriptor(fs, &d, member_info, MEMBER_SIZE) == MEMBER_SIZE)
    {
        if (memcmp(member_info, name, MEMBER_NAME_CHARS) == 0)
        {
            return member_pos_index{ pos, *(uint32*)(member_info + MEMBER_NAME_CHARS) };
        }
        pos++;
    }

    return member_pos_index{ INDEX_EMPTY, INDEX_EMPTY };
}

std::string MEMBER_NAME_to_string(uint8 member_name[MEMBER_NAME_CHARS])
{
    uint8 zero_ending[MEMBER_NAME_CHARS + 1] = {};
    memcpy(zero_ending, member_name, MEMBER_NAME_CHARS);
    return std::string((const char*)zero_ending);
}

void MEMBER_NAME_to_chars(std::string name, uint8 member_name[MEMBER_NAME_CHARS]) 
{
    const char* zero_ending = name.c_str();
    assert(strlen(zero_ending) <= MEMBER_NAME_CHARS);
    memset(member_name, 0, MEMBER_NAME_CHARS);
    memcpy(member_name, zero_ending, MEMBER_NAME_CHARS);
}

std::string get_folder_member_name(filesystem* fs, uint32 folder_index, uint32 member_index)
{
    file* folder = get_file(fs, folder_index);

    descriptor d{};
    init_descriptor(&d, fs, folder_index);

    uint8 member_info[MEMBER_SIZE] = {};
    while (read_descriptor(fs, &d, member_info, MEMBER_SIZE) == MEMBER_SIZE)
    {
        uint32 current_member_index = *(uint32*)(member_info + MEMBER_NAME_CHARS);
        if (current_member_index == member_index) 
        {
            uint8 member_name[MEMBER_NAME_CHARS + 1] = {};
            memcpy(member_name, member_info, MEMBER_NAME_CHARS);
            return std::string((const char*)member_name);
        }
    }

    return "";
}

uint32 folder_add_member(filesystem* fs, uint32 folder_index, uint32 member_index, const uint8 name[MEMBER_NAME_CHARS])
{
    if (!check_name(name)) {
        return MEMBER_BAD_NAME;
    }

    if (get_folder_member(fs, folder_index, name).index != INDEX_EMPTY) {
        return MEMBER_ALREADY_EXISTS;
    }

    uint8 payload[MEMBER_SIZE] = {};

    for (uint32 i = 0; i < MEMBER_NAME_CHARS && name[i] != '\x0'; i++) {
        payload[i] = name[i];
    }

    *(uint32*)(payload + MEMBER_NAME_CHARS) = member_index;

    descriptor d{};
    init_descriptor(&d, fs, folder_index);
    seek_descriptor(fs, &d, get_file(fs, folder_index)->size);

    if (write_descriptor(fs, &d, payload, MEMBER_SIZE) == NO_FREE_MEMORY) {
        return MEMBER_NO_MEMORY;
    }

    get_file(fs, member_index)->nlink++;

    return MEMBER_ADDED_SUCCESSFUL;
}

void folder_remove_member(filesystem* fs, uint32 folder_index, uint32 member_pos) 
{
    file* folder = get_file(fs, folder_index);

    assert(folder->type == FILE_FOLDER);
    assert(folder->size / MEMBER_SIZE > member_pos);

    uint32 last_member_index = folder->size / MEMBER_SIZE - 1;

    if (last_member_index > member_pos) 
    {
        descriptor write_d{}, read_d{};

        init_descriptor(&write_d, fs, folder_index);
        init_descriptor(&read_d, fs, folder_index);

        seek_descriptor(fs, &write_d, member_pos * MEMBER_SIZE);
        seek_descriptor(fs, &read_d, last_member_index * MEMBER_SIZE);

        uint8 copy_buf[MEMBER_SIZE] = {};
        assert(read_descriptor(fs, &read_d, copy_buf, MEMBER_SIZE) == MEMBER_SIZE);
        assert(write_descriptor(fs, &write_d, copy_buf, MEMBER_SIZE) == MEMBER_SIZE);
    }

    assert(change_file_size(fs, folder, folder->size - MEMBER_SIZE));
}

filesystem* init_filesystem(void* ptr, uint32 size, uint32 files, uint32 block_size_bytes)
{
    filesystem* fs = (filesystem*)ptr;

    memset(ptr, 0, size);

    fs->total_size = size;
    fs->files = files;
    fs->block_size_bytes = block_size_bytes;

    uint32 free_size = size - sizeof(filesystem) - files * sizeof(file);
    uint32 blocks = get_blocks_in_size(free_size, block_size_bytes);
    fs->blocks = blocks;
    fs->free_blocks = blocks;

    fs->start_of_files = sizeof(filesystem);
    fs->start_of_bitmap = fs->start_of_files + files * sizeof(file);
    fs->start_of_maps = fs->start_of_bitmap + get_size_of_bitmap(blocks);
    fs->start_of_blocks = fs->start_of_maps + blocks * sizeof(map);

    uint32 root_folder_index = create_file(fs, FILE_FOLDER);
    assert(root_folder_index == 0);

    assert(folder_add_member(fs, 0, 0, current_folder_name) == MEMBER_ADDED_SUCCESSFUL);
    assert(folder_add_member(fs, 0, 0, parent_folder_name) == MEMBER_ADDED_SUCCESSFUL);

    return fs;
}

uint32 create_link_in_folder(filesystem* fs, uint32 folder_index, uint32 member_index, const uint8 name[MEMBER_NAME_CHARS])
{
    assert(get_file(fs, folder_index)->type == FILE_FOLDER);
    assert(get_file(fs, member_index)->type == FILE_REGULAR);

    if (folder_add_member(fs, folder_index, member_index, name) != MEMBER_ADDED_SUCCESSFUL) {
        return LINK_CREATE_UNSUCCESSFUL;
    }

    return LINK_CREATE_SUCCESSFUL;
}

uint32 create_file_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS])
{
    assert(get_file(fs, folder_index)->type == FILE_FOLDER);

    uint32 file_index = create_file(fs, FILE_REGULAR);
    if (file_index == INDEX_EMPTY) {
        return FILE_CREATE_UNSUCCESSFUL;
    }

    if (create_link_in_folder(fs, folder_index, file_index, name) != LINK_CREATE_SUCCESSFUL) {
        delete_file(fs, file_index);
        return FILE_CREATE_UNSUCCESSFUL;
    }

    return file_index;
}

uint32 create_folder_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS])
{
    assert(get_file(fs, folder_index)->type == FILE_FOLDER);

    uint32 inner_folder_index = create_file(fs, FILE_FOLDER);
    if (inner_folder_index == INDEX_EMPTY) {
        return FILE_CREATE_UNSUCCESSFUL;
    }

    if (folder_add_member(fs, inner_folder_index, inner_folder_index, current_folder_name) != MEMBER_ADDED_SUCCESSFUL) 
    {
        delete_file(fs, inner_folder_index);
        return FILE_CREATE_UNSUCCESSFUL;
    }

    if (folder_add_member(fs, inner_folder_index, folder_index, parent_folder_name) != MEMBER_ADDED_SUCCESSFUL)
    {
        delete_file(fs, inner_folder_index);
        return FILE_CREATE_UNSUCCESSFUL;
    }

    if (folder_add_member(fs, folder_index, inner_folder_index, name) != MEMBER_ADDED_SUCCESSFUL) 
    {
        delete_file(fs, inner_folder_index);
        return FILE_CREATE_UNSUCCESSFUL;
    }

    return inner_folder_index;
}

bool is_folder_empty(filesystem* fs, uint32 folder_index) 
{
    file* f = get_file(fs, folder_index);
    assert(f->type == FILE_FOLDER);
    return get_file(fs, folder_index)->size == 2 * MEMBER_SIZE;
}

uint32 delete_link_file_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS])
{
    file* folder = get_file(fs, folder_index);
    assert(folder->type == FILE_FOLDER);

    if (memcmp(name, parent_folder_name, MEMBER_NAME_CHARS) == 0 ||
        memcmp(name, current_folder_name, MEMBER_NAME_CHARS) == 0)
    {
        return UNLINK_UNSUCCESSFUL;
    }

    member_pos_index member = get_folder_member(fs, folder_index, name);

    if (member.pos == INDEX_EMPTY) {
        return UNLINK_UNSUCCESSFUL;
    }

    file* member_file = get_file(fs, member.index);

    if (member_file->type == FILE_REGULAR) 
    {
        folder_remove_member(fs, folder_index, member.pos);

        if (--member_file->nlink == 0) {
            delete_file(fs, member.index);
        }

        return UNLINK_SUCCESSFUL;
    }

    return UNLINK_UNSUCCESSFUL;
}

uint32 delete_link_folder_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS])
{
    file* folder = get_file(fs, folder_index);
    assert(folder->type == FILE_FOLDER);

    if (memcmp(name, parent_folder_name, MEMBER_NAME_CHARS) == 0 ||
        memcmp(name, current_folder_name, MEMBER_NAME_CHARS) == 0)
    {
        return UNLINK_UNSUCCESSFUL;
    }

    member_pos_index member = get_folder_member(fs, folder_index, name);

    if (member.pos == INDEX_EMPTY) {
        return UNLINK_UNSUCCESSFUL;
    }

    file* member_file = get_file(fs, member.index);

    if (member_file->type == FILE_FOLDER)
    {
        if (!is_folder_empty(fs, member.index)) {
            return UNLINK_UNSUCCESSFUL;
        }

        assert(member_file->nlink == 2);

        folder_remove_member(fs, folder_index, member.pos);
        delete_file(fs, member.index);
        folder->nlink--;

        return UNLINK_SUCCESSFUL;
    }

    return UNLINK_UNSUCCESSFUL;
}

uint32 create_symlink_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS], const uint8* symlink_path)
{
    assert(get_file(fs, folder_index)->type == FILE_FOLDER);

    uint32 symlink_index = create_file(fs, FILE_SYMLINK);
    if (symlink_index == INDEX_EMPTY) {
        return SYMLINK_CREATE_UNSUCCESSFUL;
    }

    if (folder_add_member(fs, folder_index, symlink_index, name) != MEMBER_ADDED_SUCCESSFUL) 
    {
        delete_file(fs, symlink_index);
        return SYMLINK_CREATE_UNSUCCESSFUL;
    }

    descriptor d;
    init_descriptor(&d, fs, symlink_index);
    uint32 symlink_path_size = strlen((const char*)symlink_path);
    if (write_descriptor(fs, &d, symlink_path, symlink_path_size) != symlink_path_size) 
    {
        delete_file(fs, symlink_index);
        return SYMLINK_CREATE_UNSUCCESSFUL;
    }

    return symlink_index;
}

uint32 delete_symlink_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS])
{
    file* folder = get_file(fs, folder_index);
    assert(folder->type == FILE_FOLDER);

    if (memcmp(name, parent_folder_name, MEMBER_NAME_CHARS) == 0 ||
        memcmp(name, current_folder_name, MEMBER_NAME_CHARS) == 0)
    {
        return UNLINK_UNSUCCESSFUL;
    }

    member_pos_index member = get_folder_member(fs, folder_index, name);

    if (member.pos == INDEX_EMPTY) {
        return UNLINK_UNSUCCESSFUL;
    }

    file* member_file = get_file(fs, member.index);

    if (member_file->type == FILE_SYMLINK) 
    {
        folder_remove_member(fs, folder_index, member.pos);

        if (--member_file->nlink == 0) {
            delete_file(fs, member.index);
        }

        return UNLINK_SUCCESSFUL;
    }

    return UNLINK_UNSUCCESSFUL;
}
