/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TA_MODULE_H
#define FPC_TA_MODULE_H

#include <stdint.h>

typedef struct fpc_ta_module {
    int (*init)(void);
    void (*exit)(void);
    int (*handle_message)(void* buffer, uint32_t size_buffer);
    int32_t key;
} fpc_ta_module_t;

#endif // FPC_TA_MODULE_H

