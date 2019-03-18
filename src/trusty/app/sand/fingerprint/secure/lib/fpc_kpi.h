/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 * KPI measurements of the individual parts of the fpc_lib
 * such as Fingerprint Sensor Debounce, CAC, capture, etc
 */

#ifndef _FPC_KPI_H_DEFINED_
#define _FPC_KPI_H_DEFINED_

#include <stdlib.h>

/*
 * Enable kpi logging, allocs and initializes internal data.
 *
 * @return 0 on success, -1 on failure to allocate buffer for kpi logging.
 */
int32_t fpc_kpi_enable(void);

/*
 * Disable kpi logging, frees any allocated buffers.
 *
 * @return 0. Can not fail.
 */
int32_t fpc_kpi_disable(void);

/*
 * Start kpi logging.
 *
 * @return 0. Can not fail.
 */
int32_t fpc_kpi_start(void);

/*
 * Stop kpi logging and copy accumlated data to supplied buffer.
 *
 * @param[out] data, buffer to fill with kpi info.
 * @param[out] size of data copied in bytes.
 *
 * @return 0 on success, -1 if data or size is NULL.
 */
int32_t fpc_kpi_stop(uint8_t *data, uint32_t *size);
#endif

