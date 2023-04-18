#pragma once 

#include "filesystem_blocks.hpp"

struct descriptor {
    uint32 file_index;
    uint32 map_index;
    uint32 seek;
};

#define NO_FREE_MEMORY 0xff'ff'ff'ff

void init_descriptor(descriptor* d, filesystem* fs, uint32 file_index);

int32 seek_descriptor(filesystem* fs, descriptor* d, int32 shift);

uint32 read_descriptor_local(filesystem* fs, descriptor* d, file* f, uint8* buf, uint32 size);

uint32 read_descriptor(filesystem* fs, descriptor* d, uint8* buf, uint32 size);

uint32 write_descriptor_local(filesystem* fs, descriptor* d, file* f, const uint8* buf, uint32 size);

uint32 write_descriptor(filesystem* fs, descriptor* d, uint8* buf, uint32 size);