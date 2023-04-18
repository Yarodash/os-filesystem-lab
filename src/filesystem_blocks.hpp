#pragma once

#include "utils.hpp"

struct filesystem {
    uint32 total_size;
    uint32 block_size_bytes;
    uint32 files;
    uint32 blocks;
    uint32 free_blocks;

    uint32 start_of_files;
    uint32 start_of_bitmap;
    uint32 start_of_maps;
    uint32 start_of_blocks;

    uint32 __a;
    uint32 __b;
    uint32 __c;
};

struct file {
    uint32 type;
    uint32 nlink;
    uint32 size;
    uint32 map_index;
};

#define FILE_FREE 0x00'00'00'00
#define FILE_FOLDER 0x00'00'00'01
#define FILE_REGULAR 0x00'00'00'02

struct map {
    uint32 next_map_index;
};

#define INDEX_EMPTY 0xff'ff'ff'ff

uint32 get_size_of_bitmap(uint32 length);

uint32 get_blocks_in_size(uint32 size, uint32 block_size_bytes);

file* get_file(filesystem* fs, uint32 index);

map* get_map(filesystem* fs, uint32 index);

uint8* get_block(filesystem* fs, uint32 index);

uint32 get_first_free_file(filesystem* fs);

bool is_block_free(filesystem* fs, uint32 index);

void set_block_busy(filesystem* fs, uint32 index);

void set_block_free(filesystem* fs, uint32 index);

uint32 get_first_free_block_index(filesystem* fs, uint32 start = 0);

void fill_block_with_zero(filesystem* fs, uint32 index);

void fill_block_with_zero_part(filesystem* fs, uint32 index, uint32 old_size, uint32 new_size);

bool change_file_size(filesystem* fs, file* file, uint32 new_size);

uint32 create_file(filesystem* fs, uint32 FILE_TYPE);

void delete_file(filesystem* fs, uint32 index);
