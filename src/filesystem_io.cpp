#include "filesystem_io.hpp"


void init_descriptor(descriptor* d, filesystem* fs, uint32 file_index) 
{
    assert(0 <= file_index && file_index < fs->files);

    d->file_index = file_index;
    d->map_index = get_file(fs, file_index)->map_index;
    d->seek = 0;
}

int32 seek_descriptor(filesystem* fs, descriptor* d, int32 shift) 
{
    file* f = get_file(fs, d->file_index);

    uint32 new_seek = d->seek + shift;
    if ((int32)new_seek > (int32)f->size) new_seek = f->size;
    if ((int32)new_seek < 0) new_seek = 0;
    int32 difference = (int32)new_seek - (int32)d->seek;

    uint32 block_number = SECTOR(d->seek, fs->block_size_bytes);
    uint32 new_block_number = SECTOR(new_seek, fs->block_size_bytes);

    if (new_block_number == block_number)
    {
        d->seek = new_seek;
        return difference;
    }

    else if (new_block_number > block_number) 
    {
        uint32 blocks_skip = new_block_number - block_number;

        for (uint32 i = 0; i < blocks_skip; i++) {
            d->map_index = get_map(fs, d->map_index)->next_map_index;
        }

        d->seek = new_seek;
        return difference;
    }

    else 
    {
        d->map_index = f->map_index;
        d->seek = 0;
        seek_descriptor(fs, d, new_seek);
        return difference;
    }
}

uint32 read_descriptor_local(filesystem* fs, descriptor* d, file* f, uint8* buf, uint32 size) 
{
    uint32 local_seek = d->seek & (fs->block_size_bytes-1);
    uint32 local_left = fs->block_size_bytes - local_seek;
    uint32 data_left = f->size - d->seek;
    uint32 local_read_size = MIN(MIN(size, local_left), data_left);

    if (d->map_index == INDEX_EMPTY) {
        uint32 current_seek = d->seek;
        d->seek = 0;
        d->map_index = f->map_index;
        seek_descriptor(fs, d, current_seek);
    }

    memcpy(buf, get_block(fs, d->map_index) + local_seek, local_read_size);
    d->seek = d->seek + local_read_size;

    if (local_read_size == local_left) {
        d->map_index = get_map(fs, d->map_index)->next_map_index;
    }
        
    return local_read_size;
}

uint32 read_descriptor(filesystem* fs, descriptor* d, uint8* buf, uint32 size) 
{
    file* f = get_file(fs, d->file_index);
    uint32 data_left = f->size - d->seek;
    uint32 read_size = MIN(size, data_left);
    size = read_size;

    while (read_size > 0)
    {
        uint32 read_size_local = read_descriptor_local(fs, d, f, buf, read_size);
        read_size = read_size - read_size_local;
        buf = buf + read_size_local;
    }

    return size;
}

uint32 write_descriptor_local(filesystem* fs, descriptor* d, file* f, const uint8* buf, uint32 size)
{
    uint32 local_seek = d->seek & (fs->block_size_bytes - 1);
    uint32 local_left = fs->block_size_bytes - local_seek;
    uint32 data_left = f->size - d->seek;
    uint32 local_write_size = MIN(MIN(size, local_left), data_left);

    if (d->map_index == INDEX_EMPTY) {
        uint32 current_seek = d->seek;
        d->seek = 0;
        d->map_index = f->map_index;
        seek_descriptor(fs, d, current_seek);
    }

    memcpy(get_block(fs, d->map_index) + local_seek, buf, local_write_size);
    d->seek = d->seek + local_write_size;

    if (local_write_size == local_left) {
        d->map_index = get_map(fs, d->map_index)->next_map_index;
    }

    return local_write_size;
}

uint32 write_descriptor(filesystem* fs, descriptor* d, uint8* buf, uint32 size)
{
    file* f = get_file(fs, d->file_index);
    uint32 data_left = f->size - d->seek;

    if (size > data_left) {
        if (!change_file_size(fs, f, d->seek + size)) {
            return NO_FREE_MEMORY;
        }
        assert(f->size == d->seek + size);
    }

    uint32 write_size = size;

    while (write_size > 0)
    {
        uint32 write_size_local = write_descriptor_local(fs, d, f, buf, write_size);
        write_size = write_size - write_size_local;
        buf = buf + write_size_local;
    }

    return size;
}

