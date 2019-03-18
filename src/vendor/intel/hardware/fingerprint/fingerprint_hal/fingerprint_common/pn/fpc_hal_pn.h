/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_PN_H
#define FPC_HAL_PN_H

#include "fpc_tee_hal.h"

int fpc_save_pn(void *image, uint32_t image_size);

#ifdef FPC_CONFIG_ENGINEERING
int fpc_save_pn_debug(fpc_hal_common_t* hal);
#endif

int fpc_load_pn(fpc_hal_common_t* dev);

#endif // FPC_HAL_PN_H
