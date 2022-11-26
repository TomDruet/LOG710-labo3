#ifndef LIBMEM
#define LIBMEM

// IMPORTANT(Alexis Brodeur): NE PAS MODIFIER CE FICHIER !

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    MEM_FIRST_FIT,
    MEM_BEST_FIT,
    MEM_WORST_FIT,
    MEM_NEXT_FIT,
    NUM_MEM_STRATEGIES,
} mem_strategy_t;

void mem_init(size_t size, mem_strategy_t strategy);

void mem_deinit(void);

void* mem_alloc(size_t size);

void mem_free(void* ptr);

size_t mem_get_free_block_count();

size_t mem_get_allocated_block_count();

size_t mem_get_free_bytes();

size_t mem_get_biggest_free_block_size();

size_t mem_count_small_free_blocks(size_t max_bytes);

bool mem_is_allocated(void* ptr);

void mem_print_state(void);

#endif
