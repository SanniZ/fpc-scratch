/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_DEBUG_H_INCLUDE_GUARD
#define FPC_DEBUG_H_INCLUDE_GUARD

#include <stdint.h>
#include <stddef.h>

/* Used to store information about a debug contex */
typedef struct {
    /* The current address of the top of the heap */
    uint64_t max_heap_address;
    /* The first adress allocated, considered the bottom of the heap */
    uint64_t first_heap_address;
    /* The diff between top and bottom of the heap*/
    uint32_t max_heap_size;
} fpc_debug_context_t;

typedef struct {
    char* log;
    char* end;
    int full;
} fpc_debug_log_context_t;

void fpc_debug_circular_log(char* format, ...);
void fpc_debug_get_circular_log(uint8_t* buffer, uint32_t* size);

/**
 * Calculates the current max heap given the newly allocated pointer and size.
 *
 * param[in] ctx     - the debug contex to add this allocation.
 * param[in] pointer - pointer to the newly allocated memory
 * param[in] size    - size of newly allocated memory
 *
*/
void fpc_debug_heap_allocation(fpc_debug_context_t* ctx,
        void* pointer, size_t size);

#endif /* FPC_DEBUG_H_INCLUDE_GUARD */
