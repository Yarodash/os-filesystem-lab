#pragma once

#include "filesystem_io.hpp"
#include <string>

#define MEMBER_NAME_CHARS 12
#define MEMBER_INDEX_SIZE sizeof(uint32)
#define MEMBER_SIZE (MEMBER_NAME_CHARS + MEMBER_INDEX_SIZE)

#define MEMBER_ADDED_SUCCESSFUL 0x00'00'00'00
#define MEMBER_ALREADY_EXISTS 0x00'00'00'01
#define MEMBER_NO_MEMORY 0x00'00'00'02
#define MEMBER_BAD_NAME 0'00'00'00'04

#define LINK_CREATE_SUCCESSFUL 0x00'00'00'00
#define LINK_CREATE_UNSUCCESSFUL 0xff'ff'ff'ff

#define FILE_CREATE_UNSUCCESSFUL 0xff'ff'ff'ff

#define UNLINK_SUCCESSFUL 0x00'00'00'00
#define UNLINK_UNSUCCESSFUL 0x00'00'00'01

struct member_pos_index {
    uint32 pos;
    uint32 index;
};

const uint8 current_folder_name[MEMBER_NAME_CHARS] = ".";
const uint8 parent_folder_name[MEMBER_NAME_CHARS] = "..";
const uint8 empty_folder_name[MEMBER_NAME_CHARS] = "";

bool check_name(const uint8 name[MEMBER_NAME_CHARS]);

member_pos_index get_folder_member(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS]);

std::string MEMBER_NAME_to_string(uint8 member_name[MEMBER_NAME_CHARS]);

void MEMBER_NAME_to_chars(std::string name, uint8 member_name[MEMBER_NAME_CHARS]);

std::string get_folder_member_name(filesystem* fs, uint32 folder_index, uint32 member_index);

uint32 folder_add_member(filesystem* fs, uint32 folder_index, uint32 member_index, const uint8 name[MEMBER_NAME_CHARS]);

void folder_remove_member(filesystem* fs, uint32 folder_index, uint32 member_pos);

filesystem* init_filesystem(void* ptr, uint32 size, uint32 files, uint32 block_size_bytes);

uint32 create_link_in_folder(filesystem* fs, uint32 folder_index, uint32 member_index, const uint8 name[MEMBER_NAME_CHARS]);

uint32 create_file_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS]);

uint32 create_folder_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS]);

bool is_folder_empty(filesystem* fs, uint32 folder_index);

uint32 delete_link_file_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS]);

uint32 delete_link_folder_in_folder(filesystem* fs, uint32 folder_index, const uint8 name[MEMBER_NAME_CHARS]);
