/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_BIO_H
#define FPC_TEE_BIO_H
#include "stdint.h"
#include "fpc_tee.h"
#include "fpc_ta_bio_interface.h"

typedef struct fpc_tee_bio fpc_tee_bio_t;

fpc_tee_bio_t* fpc_tee_bio_init(fpc_tee_t* tee);
void fpc_tee_bio_release(fpc_tee_bio_t* tee);

int fpc_tee_set_gid(fpc_tee_bio_t* tee, int32_t gid);
int fpc_tee_begin_enrol(fpc_tee_bio_t* tee);
int fpc_tee_enrol(fpc_tee_bio_t* tee, uint32_t* remaining);
int fpc_tee_end_enrol(fpc_tee_bio_t* tee, uint32_t* id);
int fpc_tee_identify(fpc_tee_bio_t* tee, uint32_t* id);
int fpc_tee_qualify_image(fpc_tee_bio_t* tee);

int fpc_tee_update_template(fpc_tee_bio_t* tee, uint32_t* update);
int fpc_tee_get_finger_ids(fpc_tee_bio_t* tee, uint32_t* size, uint32_t* ids);
int fpc_tee_delete_template(fpc_tee_bio_t* tee, uint32_t id);
int fpc_tee_get_template_db_id(fpc_tee_bio_t* tee, uint64_t* id);

int fpc_tee_load_empty_db(fpc_tee_bio_t* tee);
int fpc_tee_store_template_db(fpc_tee_bio_t* tee, const char* path);
int fpc_tee_load_template_db(fpc_tee_bio_t* tee, const char* path);

#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_get_identify_statistics(fpc_tee_bio_t* tee, fpc_ta_bio_identify_statistics_t* stat);
#endif

#endif /* FPC_TEE_BIO_H */
