/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TA_ROUTER_H
#define FPC_TA_ROUTER_H

#include <stdint.h>

int fpc_ta_router_init(void);
void fpc_ta_router_exit(void);
int fpc_ta_route_command(void* shared_buffer, uint32_t size_buffer);



#endif // FPC_TA_ROUTER_H

