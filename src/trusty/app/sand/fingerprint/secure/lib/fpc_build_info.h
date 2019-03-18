/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef __FPC_BUILD_INFO_H__
#define __FPC_BUILD_INFO_H__

#include "stdint.h"

typedef struct {
    const uint8_t* id;
    uint32_t id_size;
    const uint8_t* info;
    uint32_t info_size;
    const uint8_t* type;
    uint32_t type_size;

} fpc_build_info_t;

/*
 * Get build information from fpc lib.
 *
 * Build information data includes
 * 1) Sha1 of the gits included in the fpclib project
 * 2) A build id unique for the current revision of the gits
 * 3) Hostname of the build node and
 * 4) Timestamp indicating the time of fpclib production.
 * 5) Build type (debug/release).
 * 6) Sensor type.
 * 7) Enabled features (only debug builds).
 *
 * @param[out] build_info, struct including info data and id data.
 * @return -1 if build_info is NULL
 */
int32_t fpc_get_build_info (fpc_build_info_t* build_info);

#endif /* __FPC_BUILD_INFO_H__ */
