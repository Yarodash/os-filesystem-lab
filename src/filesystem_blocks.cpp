#include "filesystem_blocks.hpp"
#include <assert.h>


uint32 get_size_of_bitmap(uint32 length) {
    return ROUND(length, 8) / 8;
}

uint32 get_blocks_in_size(uint32 size, uint32 block_size_bytes) {
    return (8 * size) / (8 * block_size_bytes + 8 * sizeof(map) + 1);
}

file* get_file(filesystem* fs, uint32 index) {
    assert(0 <= index && index < fs->files);
    return (file*)((uint8*)fs + fs->start_of_files + index * sizeof(file));
}

map* get_map(filesystem* fs, uint32 index) {
    assert(0 <= index && index < fs->blocks);
    return (map*)((uint8*)fs + fs->start_of_maps + index * sizeof(map));
}

uint8* get_block(filesystem* fs, uint32 index) {
    assert(0 <= index && index < fs->blocks);
    return (uint8*)fs + fs->start_of_blocks + index * fs->block_size_bytes;
}

uint32 get_first_free_file(filesystem* fs) 
{
    for (uint32 index = 0; index < fs->files; index++) 
    {
        file* f = get_file(fs, index);
        if (f->type == FILE_FREE) {
            return index;
        }
    }

    return INDEX_EMPTY;
}

bool is_block_free(filesystem* fs, uint32 index) 
{
    uint8* bitmap = (uint8*)fs + fs->start_of_bitmap;
    uint32 byte = index >> 3;
    uint8 bit = index & 7;
    return (*(bitmap + byte) & (1 << bit)) == 0;
}

void set_block_busy(filesystem* fs, uint32 index) 
{
    uint8* bitmap = (uint8*)fs + fs->start_of_bitmap;
    uint32 byte = index >> 3;
    uint8 bit = index & 7;
    *(bitmap + byte) |= (1 << bit);
}

void set_block_free(filesystem* fs, uint32 index)
{
    uint8* bitmap = (uint8*)fs + fs->start_of_bitmap;
    uint32 byte = index >> 3;
    uint8 bit = index & 7;
    *(bitmap + byte) &= ~(1 << bit);
}

uint32 get_first_free_block_index(filesystem* fs, uint32 start)
{
    for (uint32 index = start; index < fs->blocks; index++) 
    {
        if (is_block_free(fs, index)) {
            return index;
        }
    }

    return INDEX_EMPTY;
}

void fill_block_with_zero(filesystem* fs, uint32 index) {
    memset(get_block(fs, index), 0, fs->block_size_bytes);
}

void fill_block_with_zero_part(filesystem* fs, uint32 index, uint32 old_size, uint32 delta_size) 
{
    assert(old_size < fs->block_size_bytes);
    assert(delta_size <= fs->block_size_bytes);

    if (old_size + delta_size > fs->block_size_bytes) {
        delta_size = fs->block_size_bytes - old_size;
    }

    void* block = get_block(fs, index);
    memset((uint8*)block + old_size, 0, delta_size);
}

bool change_file_size(filesystem* fs, file* file, uint32 new_size) 
{
    uint32 blocks_count = ROUND(file->size, fs->block_size_bytes) / fs->block_size_bytes;
    uint32 new_blocks_count = ROUND(new_size, fs->block_size_bytes) / fs->block_size_bytes;

    if (new_blocks_count == blocks_count) 
    {
        if (new_blocks_count == 0) {
            assert(file->size == new_size);
            return true;
        }

        uint32 block_index = file->map_index;
        while (true) {
            uint32 next_index = get_map(fs, block_index)->next_map_index;
            if (next_index == INDEX_EMPTY) break;
            block_index = next_index;
        }

        if (new_size > file->size) {
            fill_block_with_zero_part(fs, block_index, file->size & (fs->block_size_bytes - 1), new_size - file->size);
        }
        file->size = new_size;
        return true;
    }

    else if (new_blocks_count > blocks_count)
    {
        if (new_blocks_count - blocks_count > fs->free_blocks) {
            return false;
        }

        if (file->map_index == INDEX_EMPTY)
        {
            uint32 free_block_index = get_first_free_block_index(fs);

            file->map_index = free_block_index;
            set_block_busy(fs, free_block_index);
            map* m = get_map(fs, free_block_index);
            fill_block_with_zero(fs, free_block_index);
            fs->free_blocks--;

            for (uint32 i = blocks_count + 1; i < new_blocks_count; i++) 
            {
                free_block_index = get_first_free_block_index(fs, free_block_index);

                m->next_map_index = free_block_index;
                set_block_busy(fs, free_block_index);
                map* next_m = get_map(fs, free_block_index);
                fill_block_with_zero(fs, free_block_index);
                fs->free_blocks--;

                m = next_m;
            }

            m->next_map_index = INDEX_EMPTY;
        }
        else 
        {
            uint32 free_block_index = 0;
            uint32 block_index = file->map_index;
            while (true) {
                uint32 next_index = get_map(fs, block_index)->next_map_index;
                if (next_index == INDEX_EMPTY) break;
                block_index = next_index;
            }

            if ((file->size & (fs->block_size_bytes - 1)) != 0) {
                fill_block_with_zero_part(fs, block_index, file->size & (fs->block_size_bytes - 1), fs->block_size_bytes);
            }

            map* m = get_map(fs, block_index);

            for (uint32 i = blocks_count; i < new_blocks_count; i++)
            {
                free_block_index = get_first_free_block_index(fs, free_block_index);

                m->next_map_index = free_block_index;
                set_block_busy(fs, free_block_index);
                map* next_m = get_map(fs, free_block_index);
                fill_block_with_zero(fs, free_block_index);
                fs->free_blocks--;

                m = next_m;
            }

            m->next_map_index = INDEX_EMPTY;
        }

        file->size = new_size;
        return true;
    }
    else
    {
        if (new_blocks_count == 0) 
        {
            uint32 block_index = file->map_index;

            while (block_index != INDEX_EMPTY) {
                set_block_free(fs, block_index);
                fs->free_blocks++;
                block_index = get_map(fs, block_index)->next_map_index;
            }

            file->size = new_size;
            return true;
        }

        else 
        {
            uint32 block_index = file->map_index;
            map* m = get_map(fs, block_index);

            for (uint32 i = 1; i < new_blocks_count; i++) {
                block_index = m->next_map_index;
                m = get_map(fs, block_index);
            }

            block_index = m->next_map_index;
            m->next_map_index = INDEX_EMPTY;

            while (block_index != INDEX_EMPTY) {
                set_block_free(fs, block_index);
                fs->free_blocks++;
                block_index = get_map(fs, block_index)->next_map_index;
            }

            file->size = new_size;
            return true;
        }
    }
}

uint32 create_file(filesystem* fs, uint32 FILE_TYPE)
{
    uint32 index = get_first_free_file(fs);

    if (index == INDEX_EMPTY) {
        return INDEX_EMPTY;
    }

    file* f = get_file(fs, index);

    f->type = FILE_TYPE;
    f->nlink = 0;
    f->size = 0;
    f->map_index = INDEX_EMPTY;

    return index;
}

void delete_file(filesystem* fs, uint32 index) 
{
    assert(0 <= index && index < fs->files);

    file* f = get_file(fs, index);
    f->type = FILE_FREE;

    uint32 map_index = f->map_index;
    while (map_index != INDEX_EMPTY) 
    {
        set_block_free(fs, map_index);
        fs->free_blocks++;
        map_index = get_map(fs, map_index)->next_map_index;
    }
}
