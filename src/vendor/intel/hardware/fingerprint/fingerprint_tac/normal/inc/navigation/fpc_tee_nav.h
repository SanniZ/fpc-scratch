/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_TEE_NAV_H
#define FPC_TEE_NAV_H

#include "fpc_tee.h"
#include "fpc_nav_types.h"


int fpc_tee_nav_poll_data(fpc_tee_t* tee, fpc_nav_data_t* data);
int fpc_tee_nav_set_config(fpc_tee_t* tee, const fpc_nav_config_t* config);
int fpc_tee_nav_get_config(fpc_tee_t* tee, fpc_nav_config_t* config);
int fpc_tee_nav_init(fpc_tee_t* tee);
int fpc_tee_nav_exit(fpc_tee_t* tee);

#endif // FPC_TEE_NAV_H

