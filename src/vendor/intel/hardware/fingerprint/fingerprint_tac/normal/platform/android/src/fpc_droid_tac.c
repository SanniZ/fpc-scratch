/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "fpc_tac.h"
#include "fpc_log.h"

extern void ree_app_init(void);
extern void ree_app_shutdown(void);
extern void ree_app_cmd_handler(void* cmd, uint32_t cmdlen, int32_t* rsp);

struct fpc_tac
{
    uint64_t lifetime_total_mem;
    uint32_t current_total_mem;
    uint32_t alloc_shared_calls;
    uint32_t free_shared_calls;
};


fpc_tac_t* fpc_tac_open()
{
    fpc_tac_t *tac = (fpc_tac_t*) malloc(sizeof(struct fpc_tac));
    memset(tac, 0, sizeof(struct fpc_tac));

    LOGD("Init TA");
    ree_app_init();
    return tac;
}

void fpc_tac_release(fpc_tac_t* tac)
{
    LOGE("Shut down TA");
    ree_app_shutdown();

    LOGD("Shared mem stats for session:");
    LOGD("lifetime total: %" PRIu64 " ", tac->lifetime_total_mem);
    LOGD("current total (should be 0): %d", tac->current_total_mem);
    LOGD("Calls to alloc_shared: %d", tac->alloc_shared_calls);
    LOGD("Calls to free_shared: %d", tac->free_shared_calls);

    free(tac);
}

struct _droid_shared_mem
{
    fpc_tac_shared_mem_t mem_buf;
    size_t               buf_size;
    fpc_tac_t           *tac;
};

fpc_tac_shared_mem_t* fpc_tac_alloc_shared(fpc_tac_t* tac, uint32_t size)
{
    tac->alloc_shared_calls++;

    struct _droid_shared_mem *shared =
        (struct _droid_shared_mem *) malloc(sizeof(struct _droid_shared_mem));

    if(!shared)
    {
        LOGE("Failed to malloc shared buf metadata!");
        return NULL;
    }

    memset(shared, 0, sizeof(struct _droid_shared_mem));

    shared->tac = tac;
    shared->buf_size = size;
    shared->mem_buf.addr = malloc(size);

    if(!shared->mem_buf.addr)
    {
        LOGE("Failed to malloc shared buf!");
        free(shared);
        return NULL;
    }

    tac->current_total_mem += size;
    tac->lifetime_total_mem += size;

    memset(shared->mem_buf.addr, 0, size);

    return &shared->mem_buf;
}

void fpc_tac_free_shared(fpc_tac_shared_mem_t* shared_buffer)
{
    struct _droid_shared_mem *shared =
        (struct _droid_shared_mem *) shared_buffer;

    if(!shared)
    {
        return;
    }

    shared->tac->free_shared_calls++;

    if(shared->mem_buf.addr)
    {
        free(shared->mem_buf.addr);
    }

    shared->tac->current_total_mem -= shared->buf_size;

    free(shared);
}

int fpc_tac_transfer(fpc_tac_t* tac, fpc_tac_shared_mem_t *shared_buffer)
{
    int32_t resp = 0;
    (void)tac; // Unused

    struct _droid_shared_mem *shared =
        (struct _droid_shared_mem*) shared_buffer;

    ree_app_cmd_handler(shared->mem_buf.addr, (uint32_t)shared->buf_size, &resp);

    return resp;
}
