/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_H
#define FPC_TEE_H

typedef struct fpc_tee fpc_tee_t;

fpc_tee_t* fpc_tee_init();
void fpc_tee_release(fpc_tee_t* tee);
int fpc_tee_get_error_log(fpc_tee_t *tee);

#endif // FPC_TEE_H
