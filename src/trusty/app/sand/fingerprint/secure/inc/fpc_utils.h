/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef _FPC_UTILS_H_
#define _FPC_UTILS_H_

#include <stdint.h>

typedef enum {
    FPC_KPI_DISABLE,
    FPC_KPI_ENABLE,
    FPC_KPI_STOP,
    FPC_KPI_START
} fpc_kpi_sub_command_t;

/*
 * KPI control
 *
 * @param[in] ctrl, 0 to disable, 1 for enabling kpi, 2 to stop measuring, 3 to start measuing
 * @param[in/out] size, on stop: size of area to store result in/ resulting size
 * @param[in] data, on stop: area to store result in, otherwise not used
 *
 * @return -1 if parameter is a null pointer otherwise 0
 */
void fpc_kpi_enable(void);
void fpc_kpi_disable(void);
void fpc_kpi_start(void);
int32_t fpc_kpi_stop(uint8_t *data, uint32_t *size);

#endif /* _FPC_KPI_H_ */
