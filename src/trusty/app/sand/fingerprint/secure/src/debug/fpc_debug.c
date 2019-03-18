/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "fpc_log.h"
#include "fpc_debug.h"

#define MB  (1024 * 1024)
#define KB  (1024)

#ifdef  FPC_CONFIG_LOGGING_IN_RELEASE_FILE
#ifndef FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE
#define FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE ((uint32_t) 4096)
#endif

/* There must be room for a newline at the end */
#define MAX_LOG_STRING_LENGTH (FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE - 1)

static char _fpc_debug_log_buffer[FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE];

static fpc_debug_log_context_t _fpc_debug_log_context = {
    _fpc_debug_log_buffer,
    _fpc_debug_log_buffer,
    0
};

void fpc_debug_circular_log(char* format, ...) {
    va_list args;
    int32_t length;
    uint32_t buffer_left;
    uint32_t buffer_used;
    fpc_debug_log_context_t* context = &_fpc_debug_log_context;

    if (context->log == NULL ||
        context->end == NULL ||
        (context->log > context->end)) {

        return;
    }

    /* Check the current size left of the buffer. */
    buffer_used = (context->end - context->log);
    buffer_left = (MAX_LOG_STRING_LENGTH - buffer_used);

    va_start(args, format);
    length = vsnprintf(context->end, buffer_left, format, args);
    va_end(args);
    if (length < 0 || (uint32_t) length >= MAX_LOG_STRING_LENGTH) {
        return;
    }

    if ((uint32_t) length >= buffer_left) {
        /* Start from the beginning, and clear out the oldest messages.*/
        memset(context->end, 0, buffer_left);
        context->full = 1;
        context->end = _fpc_debug_log_buffer;

        va_start(args, format);
        length = vsnprintf(context->end, MAX_LOG_STRING_LENGTH, format, args);
        va_end(args);
        if (length < 0) {
            return;
        }
    }

    context->end = context->end + length;

    *(context->end) = '\n';
    context->end++;

}

void fpc_debug_get_circular_log(uint8_t* buffer, uint32_t* size) {
    uint32_t lower_part_size =
        _fpc_debug_log_context.end - _fpc_debug_log_context.log;

    uint32_t upper_part_size =
        FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE - lower_part_size;

    if (*size < FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE) {
        snprintf((char*)buffer, *size, "Too small buffer");
        return;
    }

    if (_fpc_debug_log_context.full) {
        memcpy(buffer, _fpc_debug_log_context.end, upper_part_size);
        memcpy(buffer + upper_part_size,
                _fpc_debug_log_context.log, lower_part_size);

        *size = FPC_CONFIG_LOGGING_IN_RELEASE_BUFFER_SIZE;
    } else {
        memcpy(buffer, _fpc_debug_log_context.log, lower_part_size);
        *size = lower_part_size;
    }
}
#endif

#ifdef FPC_CONFIG_PRIV_HEAP_DEBUG
void fpc_debug_heap_allocation(fpc_debug_context_t* ctx,
        void* pointer, size_t size) {

    uint64_t first_address;
    uint64_t current_address;
    uint64_t max_address;
    uint32_t m_size;
    uint32_t k_size;
    uint32_t b_size;

    if (ctx == NULL) {
        return;
    }

    if (pointer == NULL) {
        return;
    }

    if (size == 0) {
        return;
    }

    first_address = ctx->first_heap_address;
    current_address = ((uint64_t) pointer) + size;
    max_address = ctx->max_heap_address;

    if (first_address == 0) {
        ctx->first_heap_address = (uint64_t) pointer;
        ctx->max_heap_address = ((uint64_t) pointer) + size;

        LOGD("%s, First heap pointer is at 0x%p with size %zu",
                __func__, pointer, size);

    } else if (max_address < current_address) {

        ctx->max_heap_address = current_address;
        ctx->max_heap_size = (uint32_t) (current_address - first_address);

        /* Get a more readable size */
        m_size = ctx->max_heap_size / MB;
        k_size = (ctx->max_heap_size % MB ) / KB;
        b_size = ctx->max_heap_size % KB;

        LOGD("%s, New max heap address is 0x%08x first_adress 0x%08x ",
            __func__, (uint32_t) current_address, (uint32_t) first_address);

        LOGD("(current heap size is %d %d Mb, %d kb, %d b)",
             ctx->max_heap_size, m_size, k_size, b_size);

    }
}
#endif
