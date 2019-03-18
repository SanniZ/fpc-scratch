/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_DB
#define FPC_DB
#include <inttypes.h>
#include "fpc_algo.h"

#define FPC_DB_FINGERPRINTS_IN_SET   5

typedef uint8_t id256_t[32];

typedef struct FingerprintDatabase_s FingerprintDatabase_t;

int fpc_db_create(
    FingerprintDatabase_t **ppDb,
    const uint8_t* pDbData,
    uint32_t dbDataSize);

int fpc_db_destroy(FingerprintDatabase_t **ppDb);

int fpc_db_add_fingerprint(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint64_t secure_user_id,
    fpc_algo_template_t* tpl,
    uint32_t* pIndex);

int fpc_db_delete_fingerprint(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint32_t index);

int fpc_db_delete_set(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey);

int fpc_db_get_data_size(
    const FingerprintDatabase_t *pDb,
    uint32_t* pDbDataSize);

int fpc_db_get_data(
    const FingerprintDatabase_t *pDb,
    uint8_t* pDbData,
    uint32_t dbDataSize);

int fpc_db_get_fingerprint_id(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint32_t fingerprintIndex,
    id256_t* pId);

int fpc_db_get_secure_user_id(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint32_t fingerprintIndex,
    uint64_t* secure_user_id);

int fpc_db_set_secure_user_id(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint32_t fingerprintIndex,
    uint64_t secure_user_id);

int fpc_db_get_fingerprint_set_id(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    id256_t* pId);

int fpc_db_get_fingerprint_count(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint32_t* pCount);

int fpc_db_get_indices(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint32_t* pIndices,
    uint32_t indicesSize,
    uint32_t* pCount);

int fpc_db_get_template(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint32_t index,
    fpc_algo_template_t** ppTpl);

int fpc_db_get_templates(
    FingerprintDatabase_t *pDb,
    uint32_t fingerprintSetKey,
    uint32_t* pIndices,
    uint32_t size,
    fpc_algo_template_t* pList);

int fpc_db_get_all_templates(
    FingerprintDatabase_t* pDb,
    fpc_algo_template_t* list,
    uint32_t* size_list);

int fpc_db_get_database_id(FingerprintDatabase_t *pDb, uint64_t* id);

int fpc_db_trim_id_32(id256_t* id, uint32_t *pId32);
int fpc_db_trim_id_64(id256_t* id, uint64_t *pId64);

int fpc_db_get_index_of_id32(
    FingerprintDatabase_t* pDb,
    uint32_t fingerprint_id32,
    uint32_t* index);

int fpc_db_get_index_of_template(
    FingerprintDatabase_t* pDb,
    fpc_algo_template_t* tpl,
    uint32_t* index);

int fpc_db_set_template_first(FingerprintDatabase_t* pDb, uint32_t i);

#ifdef __CONSOLE_TEST__
void db_print( FingerprintDatabase_t *db );
#endif

#endif
