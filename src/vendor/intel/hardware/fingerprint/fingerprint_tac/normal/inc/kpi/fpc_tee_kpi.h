/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_KPI_H
#define FPC_TEE_KPI_H
#include "fpc_tee.h"
void fpc_tee_kpi_start(fpc_tee_t* tee);
void fpc_tee_kpi_stop(fpc_tee_t* tee);
int fpc_tee_print_build_info(fpc_tee_t* tee);
#endif //FPC_TEE_KPI_H
