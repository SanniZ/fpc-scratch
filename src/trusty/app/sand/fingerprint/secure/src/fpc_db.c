/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <string.h>

#include "fpc_db.h"
#include "fpc_crypto.h"
#include "fpc_log.h"
#include "fpc_types.h"

#define FPC_DB_MARKER_START_OF_DB    0xABAD1DEA
#define FPC_DB_MARKER_START_OF_SETS  0xBEEFF00D
#define FPC_DB_MARKER_START_OF_S_IDS 0xBADC0FFE
#define FPC_DB_MARKER_START_OF_F_IDS 0xB01DFACE
#define FPC_DB_MARKER_END_OF_FILE    0xB105F00D
#define FPC_DB_VERSION_1             0x01
#define FPC_DB_VERSION_2             0x02
#define FPC_DB_VERSION_3             0x03
#define FPC_DB_VERSION_4             0x04
#define FPC_DB_ENTRY_VALID           1
#define FPC_DB_ENTRY_INVALID         0

typedef struct
{
    /** FPC_DB_ENTRY_VALID if valid fingerprint entry, FPC_DB_ENTRY_INVALID if no data */
    uint32_t valid;

    /** 256bit / 32 byte identity of finger, for use with Raw Verification Index extension etc */
    id256_t fingerprintId;

    /** 64 bit secure user ID used by the Android-M implementation */
    uint64_t secure_user_id;

    /** Template container */
    fpc_algo_template_t ftemplate;

} Fingerprint_t;

typedef struct
{
    /** 32-bit identifier that is used to filter out fingerprint sets.
        For example Android user profile id can be used here to create
        separate sets for each Android user. */
    uint32_t fingerprintSetKey;

    /** 256bit / 32 byte identity of set, for use with UAF etc */
    id256_t fingerprintSetId;

    /** 256bit / 32 byte identity of state, for use with UAF UVS etc */
    id256_t fingerprintStateId;

    /** List of fingerprints */
    Fingerprint_t aFingerprintList[FPC_DB_FINGERPRINTS_IN_SET];

} FingerprintSet_t;

struct FingerprintDatabase_s {
    /** Random identifier for the database, recreated when a template
     * is added.
     */
    uint64_t databaseId;

    /** List of fingerprint sets */
    FingerprintSet_t* pFingerprintSet;
};

//qsee doesn't offer a calloc style API, that's why we need both old_size and manualy copy
static void *fpc_mem_calloc(size_t nmemb, size_t size)
{
    size_t totalsize = nmemb * size;
    void* ptr = malloc( totalsize );

    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, totalsize);
    return ptr;
}

static int db_update_database_id(FingerprintDatabase_t* pDb)
{
    uint64_t db_id = 0;

    if (fpc_secure_random((uint8_t*) &db_id, sizeof(uint64_t)) < 0) {
        return -FPC_ERROR_IO;
    }

    pDb->databaseId = db_id;
    return 0;
}

static int db_update_fingerprint_state_id(FingerprintDatabase_t* pDb)
{

    uint8_t* state_id = &(pDb->pFingerprintSet->fingerprintStateId[0]);

    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pDb->pFingerprintSet == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (fpc_secure_random(state_id, sizeof(id256_t)) != 0) {
        return -FPC_ERROR_IO;
    }

    return 0;
}

static int db_generate_unique_id(id256_t* id)
{

    if (id == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (fpc_secure_random(&(*id[0]), sizeof(id256_t)) != 0) {
        LOGE("%s failed to generate random value", __func__);
        return -FPC_ERROR_IO;
    }

    return 0;
}

static int db_add_set( FingerprintDatabase_t *db, FingerprintSet_t *set)
{

    if (db->pFingerprintSet != NULL) {
        return -FPC_ERROR_INPUT;
    }

    db->pFingerprintSet = set;
    return 0;
}

static int db_free(FingerprintDatabase_t* db)
{
    if (db == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (db->pFingerprintSet != NULL) {
    FingerprintSet_t* set = db->pFingerprintSet;

        for (int i = 0; i < FPC_DB_FINGERPRINTS_IN_SET; i++) {
            Fingerprint_t* finger = &(set->aFingerprintList[i]);
            if (finger->ftemplate.tpl != NULL) {
                free(finger->ftemplate.tpl);
            }
            finger->ftemplate.tpl = NULL;
            finger->ftemplate.size = 0;
        }

        free(set);
        db->pFingerprintSet = NULL;
    }

    free(db);

    return 0;
}

static int pack_uint8(uint8_t data, uint8_t *packed_db, uint32_t *writecounter,
                      uint32_t size)
{
    if (packed_db == NULL) {
        *writecounter += 1;
        return 0;
    }

    if(*writecounter+1 >= size) {
        return -FPC_ERROR_DB;
    }

    packed_db[(*writecounter)++] = data;
    return 0;
}

static int pack_uint8_array(uint8_t* data,
                            uint32_t datalength,
                            uint8_t *packed_db,
                            uint32_t *writecounter,
                            uint32_t size)
{

    if (packed_db == NULL) {
        *writecounter += datalength;
        return 0;
    }

    if(*writecounter+datalength >= size) {
        return -FPC_ERROR_DB;
    }

    for (uint32_t i = 0; i < datalength; i++) {
        packed_db[(*writecounter)++] = data[i];
    }

    return 0;
}

static int pack_uint32(uint32_t data, uint8_t *packed_db, uint32_t *writecounter,
    uint32_t size)
{
    if (packed_db == NULL) {
        *writecounter += 4;
        return 0;
    }

    if(*writecounter+4 > size) {
        return -FPC_ERROR_DB;
    }

    packed_db[(*writecounter)++] = (uint8_t) ((data>>24) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) ((data>>16) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) ((data>>8) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) (data & 0xFF);

    return 0;
}

static int pack_uint64(uint64_t data, uint8_t *packed_db,
                       uint32_t *writecounter, uint32_t size)
{
    if (packed_db == NULL) {
        *writecounter += 8;
        return 0;
    }

    if(*writecounter+8 > size) {
        return -FPC_ERROR_DB;
    }

    packed_db[(*writecounter)++] = (uint8_t) ((data>>56) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) ((data>>48) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) ((data>>40) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) ((data>>32) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) ((data>>24) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) ((data>>16) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) ((data>>8) & 0xFF);
    packed_db[(*writecounter)++] = (uint8_t) (data & 0xFF);
    return 0;
}

static int db_pack_v4(const FingerprintDatabase_t* db, uint8_t* packed_db,
                      uint32_t size, uint32_t* writecounter)
{

    if (db == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (writecounter == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (packed_db != NULL) {
        memset(packed_db, 1, size);
    }

    *writecounter = 0;

    FingerprintSet_t* set = db->pFingerprintSet;
    int set_count = (set != NULL) ? 1 : 0;

    pack_uint32(FPC_DB_MARKER_START_OF_DB, packed_db, writecounter, size);
    pack_uint8(FPC_DB_VERSION_4, packed_db, writecounter, size); //VERSION
    pack_uint64(db->databaseId, packed_db, writecounter, size);
    pack_uint32(set_count, packed_db, writecounter, size);
    pack_uint32(FPC_DB_MARKER_START_OF_SETS, packed_db, writecounter, size);

    if (set_count != 1) {
        pack_uint32(FPC_DB_MARKER_END_OF_FILE, packed_db, writecounter, size);
        return 0;
    }

    pack_uint32(set->fingerprintSetKey, packed_db, writecounter, size);
    pack_uint32(FPC_DB_MARKER_START_OF_S_IDS, packed_db, writecounter, size);
    id256_t* setid = &(set->fingerprintSetId);
    pack_uint8_array(&(*setid[0]), 32, packed_db, writecounter, size);

    id256_t* stateid = &(set->fingerprintStateId);
    pack_uint8_array(&(*stateid[0]), 32, packed_db, writecounter, size);

    for (int j = 0; j < FPC_DB_FINGERPRINTS_IN_SET; j++) {
        Fingerprint_t* finger = set->aFingerprintList+j;
        uint32_t valid = finger->valid;
        pack_uint32(valid, packed_db, writecounter, size);
        if (valid == FPC_DB_ENTRY_VALID) {
            id256_t* fid = &(finger->fingerprintId);
            pack_uint8_array(&(*fid[0]), 32, packed_db, writecounter, size);
            pack_uint64(finger->secure_user_id, packed_db, writecounter, size);
            pack_uint32(finger->ftemplate.size, packed_db, writecounter, size);
            pack_uint8_array(finger->ftemplate.tpl,
                             finger->ftemplate.size,
                             packed_db, writecounter,
                             size);

        } else if (valid != FPC_DB_ENTRY_INVALID) {
            LOGE("Unexpected entry value: %x", valid);
            return -FPC_ERROR_DB;
        }
    }

    pack_uint32(FPC_DB_MARKER_END_OF_FILE, packed_db, writecounter, size);
    return 0;
}

static int db_pack(const FingerprintDatabase_t* db, uint8_t* packed_db,
                   uint32_t size, uint32_t* writecounter)
{
    return db_pack_v4(db, packed_db, size, writecounter);
}

static int unpack_uint8(uint8_t* data, const uint8_t* packed_db,
                        uint32_t* readcounter, uint32_t size)
{
    if (data == NULL || packed_db == NULL || readcounter == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (*readcounter >= size) {
        return -FPC_ERROR_DB;
    }

    *data = packed_db[(*readcounter)++];
    return 0;
}

static int unpack_uint8_array(uint8_t* data, uint32_t datalength,
    const uint8_t* packed_db, uint32_t *readcounter, uint32_t size)
{
    if (data == NULL || packed_db == NULL || readcounter == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if ((*readcounter)+datalength > size) {
        return -FPC_ERROR_DB;
    }

    for (uint32_t i = 0; i < datalength; i++) {
        data[i] = packed_db[(*readcounter)++];
    }

    return 0;
}

static int unpack_uint32(uint32_t* data, const uint8_t* packed_db,
                         uint32_t *readcounter, uint32_t size)
{
    if (data == NULL || packed_db == NULL || readcounter == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if ((*readcounter)+4 > size) {
        return -FPC_ERROR_DB;
    }

    uint8_t byte_0 = packed_db[(*readcounter)++];
    uint8_t byte_1 = packed_db[(*readcounter)++];
    uint8_t byte_2 = packed_db[(*readcounter)++];
    uint8_t byte_3 = packed_db[(*readcounter)++];
    uint32_t result = (((uint32_t) byte_0) << 24) |
                      (((uint32_t) byte_1) << 16) |
                      (((uint32_t) byte_2) << 8)  |
                      ((uint32_t) byte_3);

    *data = result;
    return 0;
}

static int unpack_uint64(uint64_t* data, const uint8_t* packed_db,
                         uint32_t *readcounter, uint32_t size)
{
    if (data == NULL || packed_db == NULL || readcounter == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if ((*readcounter) + 8 > size) {
        return -FPC_ERROR_DB;
    }

    uint8_t byte_0 = packed_db[(*readcounter)++];
    uint8_t byte_1 = packed_db[(*readcounter)++];
    uint8_t byte_2 = packed_db[(*readcounter)++];
    uint8_t byte_3 = packed_db[(*readcounter)++];
    uint8_t byte_4 = packed_db[(*readcounter)++];
    uint8_t byte_5 = packed_db[(*readcounter)++];
    uint8_t byte_6 = packed_db[(*readcounter)++];
    uint8_t byte_7 = packed_db[(*readcounter)++];

    uint64_t result = (((uint64_t) byte_0) << 56) |
                      (((uint64_t) byte_1) << 48) |
                      (((uint64_t) byte_2) << 40) |
                      (((uint64_t) byte_3) << 32) |
                      (((uint64_t) byte_4) << 24) |
                      (((uint64_t) byte_5) << 16) |
                      (((uint64_t) byte_6) << 8)  |
                      ((uint64_t) byte_7);

    *data = result;
    return 0;
}

static int unpack_verify_uint8(uint8_t expected, const uint8_t* packed_db,
                               uint32_t *readcounter, uint32_t size)
{
    uint8_t actual;
    if (unpack_uint8(&actual, packed_db, readcounter, size) != 0) {
        return -FPC_ERROR_DB;
    }

    if (expected != actual) {
        LOGI("Error: when unpacking DB, expected %x actual: %x", expected, actual);
        return -FPC_ERROR_DB;
    }

    return 0;
}

static int unpack_verify_uint32(uint32_t expected, const uint8_t* packed_db, uint32_t *readcounter,
    uint32_t size)
{
    uint32_t actual;
    if (unpack_uint32(&actual, packed_db, readcounter, size) != 0) {
        return -FPC_ERROR_DB;
    }

    if (expected != actual) {
        LOGI("Error: when unpacking DB, expected %x actual: "
                "%x position: (%x)", expected, actual, *readcounter);

        return -FPC_ERROR_DB;
    }
    return 0;
}

static int db_unpack_v2_v3_v4(const uint8_t* packed_db,
                              uint32_t size,
                              uint8_t db_version,
                              FingerprintDatabase_t** newdb)
{
    FingerprintDatabase_t* db =
        fpc_mem_calloc(1, sizeof(FingerprintDatabase_t));

    if (db == NULL) {
        return -FPC_ERROR_ALLOC;
    }

    *newdb = db;
    uint32_t readcounter = 0;

    if (unpack_verify_uint32(
        FPC_DB_MARKER_START_OF_DB, packed_db, &readcounter, size) != 0) {

        return -FPC_ERROR_DB;
    }

    if (unpack_verify_uint8(
         db_version, packed_db, &readcounter, size) != 0) {

        return -FPC_ERROR_DB;
    }

    if (db_version >= FPC_DB_VERSION_3) {
        unpack_uint64(&(db->databaseId), packed_db, &readcounter, size);
    } else {
        int status = db_update_database_id(db);
        if (status != 0) {
          return status;
        }
    }

    uint32_t set_count = (db->pFingerprintSet == NULL) ? 0 : 1;
    unpack_uint32(&set_count, packed_db, &readcounter, size);
    if (unpack_verify_uint32(
            FPC_DB_MARKER_START_OF_SETS, packed_db, &readcounter, size) != 0) {
        return -FPC_ERROR_DB;
    }

    if (set_count > 1) {
        return -FPC_ERROR_DB;
    }
    if (set_count != 1) {
        if (unpack_verify_uint32(
            FPC_DB_MARKER_END_OF_FILE, packed_db, &readcounter, size) != 0) {

            return -FPC_ERROR_DB;
        }

        return 0;
    }

    FingerprintSet_t* set = fpc_mem_calloc( 1, sizeof(FingerprintSet_t));
    if (set == NULL) {
        return -FPC_ERROR_ALLOC;
    }
    db->pFingerprintSet = set;

    unpack_uint32(&(set->fingerprintSetKey), packed_db, &readcounter, size);

    if (unpack_verify_uint32(
        FPC_DB_MARKER_START_OF_S_IDS, packed_db, &readcounter, size) != 0) {

        return -FPC_ERROR_DB;
    }

    id256_t* setid = &(set->fingerprintSetId);
    unpack_uint8_array(&(*setid[0]), 32, packed_db, &readcounter, size);

    if (db_version >= FPC_DB_VERSION_4) {
        unpack_uint8_array(&(set->fingerprintStateId[0]), 32, packed_db, &readcounter, size);
    } else {
        int status = db_update_fingerprint_state_id(db);
        if (status != 0) {
            return status;
        }
    }

    for (int j = 0; j < FPC_DB_FINGERPRINTS_IN_SET; j++)
    {
        Fingerprint_t* finger = set->aFingerprintList+j;
        uint32_t valid = 0;
        unpack_uint32(&valid, packed_db, &readcounter, size);
        finger->valid = valid;
        if (valid == FPC_DB_ENTRY_VALID) {
            uint32_t fsize;
            uint8_t* fmem;
            id256_t* fid = &(finger->fingerprintId);
            unpack_uint8_array(&(*fid[0]), 32, packed_db, &readcounter, size);

            // The only difference between v2 and v3 is the secure user ID in v3
            if (db_version >= FPC_DB_VERSION_3) {
                unpack_uint64(&finger->secure_user_id, packed_db, &readcounter, size);
            } else {
                finger->secure_user_id = 0;
            }

            if(0 != unpack_uint32(&fsize, packed_db, &readcounter, size)) {
              return -FPC_ERROR_DB;
            }

            fmem = fpc_mem_calloc(fsize, sizeof(uint8_t));
            if (fmem == NULL) {
              return -FPC_ERROR_ALLOC;
            }

            finger->ftemplate.size = fsize;
            finger->ftemplate.tpl = fmem;
            unpack_uint8_array(fmem, fsize, packed_db, &readcounter, size);
        } else if (valid != FPC_DB_ENTRY_INVALID) {
           LOGE("Unexpected entry value: %x", valid);
           return -FPC_ERROR_DB;
        }
    }

    if (unpack_verify_uint32(
        FPC_DB_MARKER_END_OF_FILE, packed_db, &readcounter, size) != 0) {

        return -FPC_ERROR_DB;
    }

    return 0;
}

static int db_unpack(const uint8_t* packed_db, uint32_t size,
                    FingerprintDatabase_t** newdb)
{
    *newdb = NULL;
    uint32_t readcounter = 0;
    if (unpack_verify_uint32(
                FPC_DB_MARKER_START_OF_DB, packed_db, &readcounter, size)) {

        LOGE("Invalid DB start!");
        return -FPC_ERROR_DB;
    }

    uint8_t db_version = 0;
    if (unpack_uint8(&db_version, packed_db, &readcounter, size) < 0) {
        LOGE("Error reading database version");
        return -FPC_ERROR_DB;
    }

    int result = 0;
    switch(db_version)
    {
    case FPC_DB_VERSION_1:
        LOGE("Unsupported old version: %x", db_version);
        return -FPC_ERROR_DB;
    case FPC_DB_VERSION_2:
    case FPC_DB_VERSION_3:
        LOGD("Current version: %x upgrading to %x", db_version, FPC_DB_VERSION_4);
    case FPC_DB_VERSION_4:
        result = db_unpack_v2_v3_v4(packed_db, size, db_version, newdb);
        break;
    default:
        LOGE("Unknown/unsupported version: %x", db_version);
        return -FPC_ERROR_DB;
    }

    if (result != 0) {
        if (*newdb != NULL) {
            db_free(*newdb);
            *newdb = NULL;
        }
    }

    return result;
}

int fpc_db_create(FingerprintDatabase_t **ppDb, const uint8_t* pDbData,
                  uint32_t dbDataSize)
{
    if (ppDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pDbData == NULL && dbDataSize != 0) {
        //illegal. not create empty flow, not intialize from packed db flow.
        return -FPC_ERROR_INPUT;
    }

    FingerprintDatabase_t* db;
    if (pDbData == NULL) {
        db = fpc_mem_calloc(1, sizeof(FingerprintDatabase_t));
        if (db == NULL) {
            return -FPC_ERROR_ALLOC;
        }
        db_update_database_id(db);
    } else {
        int result = db_unpack(pDbData, dbDataSize, &db);
        if (result != 0) {
          return result;
        }
    }

    *ppDb = db;
    return 0;
}

int fpc_db_delete_set(FingerprintDatabase_t *pDb,
                                  uint32_t fingerprintSetKey)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pDb->pFingerprintSet == NULL) {
        return -FPC_ERROR_NOENTITY;
    }

    FingerprintSet_t* set = pDb->pFingerprintSet;
    if (set->fingerprintSetKey != fingerprintSetKey) {
        return -FPC_ERROR_NOENTITY;
    }

    for (uint32_t index = 0; index < FPC_DB_FINGERPRINTS_IN_SET; index++) {
        Fingerprint_t* finger = &(set->aFingerprintList[index]);
        finger->valid = FPC_DB_ENTRY_INVALID;
        if (finger->ftemplate.tpl != NULL) {
            memset(finger->ftemplate.tpl, 0xCC, finger->ftemplate.size);
            free(finger->ftemplate.tpl);
        }
        finger->ftemplate.tpl = NULL;
        finger->ftemplate.size = 0;
        memset(&(finger->fingerprintId[0]), 0xCC, sizeof(id256_t));
    }

    memset(set, 0xCC, sizeof(FingerprintSet_t));
    free(set);
    pDb->pFingerprintSet = NULL;

    return 0;
}

int fpc_db_destroy(FingerprintDatabase_t **ppDb)
{
    if (ppDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintDatabase_t* db = *ppDb;
    if (db == NULL) {
        return -FPC_ERROR_INPUT;
    }

    int result = db_free(db);
    if (result != 0) {
        return result;
    }

    *ppDb = NULL;
    return 0;
}

int fpc_db_get_data_size(const FingerprintDatabase_t *pDb, uint32_t* pDbDataSize)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }
    if (pDbDataSize == NULL) {
        return -FPC_ERROR_INPUT;
    }

    uint32_t writecounter = 0;
    int result = db_pack(pDb, NULL, 1<<30 /* 1GB */, &writecounter );
    if (result != 0) {
        return result;
    }
    *pDbDataSize = writecounter;

    return 0;
}

int fpc_db_get_data(const FingerprintDatabase_t *pDb, uint8_t* pDbData,
                    uint32_t dbDataSize)
{

    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pDbData == NULL) {
        return -FPC_ERROR_INPUT;
    }

    uint32_t writecounter = 0;
    int result = db_pack(pDb, pDbData, dbDataSize, &writecounter );
    if (result != 0) {
        return result;
    }

    if (writecounter != dbDataSize) {
        return -FPC_ERROR_DB;
    }

    return 0;
}

static int fpc_db_get_set(FingerprintDatabase_t *pDb, uint32_t fingerprintSetKey,
    FingerprintSet_t** ppSet)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pDb->pFingerprintSet == NULL) {
        return -FPC_ERROR_NOENTITY;
    }

    FingerprintSet_t* set = pDb->pFingerprintSet;

    if (set->fingerprintSetKey != fingerprintSetKey) {
        return -FPC_ERROR_NOENTITY;
    }

    *ppSet = set;
    return 0;
}

int fpc_db_get_fingerprint_id(FingerprintDatabase_t *pDb,
                              uint32_t fingerprintSetKey,
                              uint32_t fingerprintIndex,
                              id256_t* pId)
{

    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (fingerprintIndex >= FPC_DB_FINGERPRINTS_IN_SET) {
        return -FPC_ERROR_INPUT;
    }

    if (pId == NULL) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t *set = NULL;
    int lookup = fpc_db_get_set( pDb, fingerprintSetKey, &set );
    if (lookup != 0) {
        return lookup;
    }

    Fingerprint_t* finger = &(set->aFingerprintList[fingerprintIndex]);
    if (finger->valid == FPC_DB_ENTRY_INVALID) {
        return -FPC_ERROR_NOENTITY;
    }

    id256_t* id = &(finger->fingerprintId);
    if (id == NULL) {
        return -FPC_ERROR_NOENTITY;
    }

    for (int i = 0; i < 32; i++) {
        (*pId)[i] = (*id)[i];
    }

    return 0;

}

int fpc_db_get_secure_user_id(FingerprintDatabase_t *pDb,
                              uint32_t fingerprintSetKey,
                              uint32_t fingerprintIndex,
                              uint64_t* secure_user_id)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (fingerprintIndex >= FPC_DB_FINGERPRINTS_IN_SET) {
        return -FPC_ERROR_INPUT;
    }

    if (secure_user_id == NULL) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t *set = NULL;
    int lookup = fpc_db_get_set( pDb, fingerprintSetKey, &set );
    if (lookup != 0) {
        return lookup;
    }

    Fingerprint_t* finger = &(set->aFingerprintList[fingerprintIndex]);
    if (finger->valid == FPC_DB_ENTRY_INVALID) {
        return -FPC_ERROR_NOENTITY;
    }

    *secure_user_id = finger->secure_user_id;
    return 0;
}

int fpc_db_set_secure_user_id(FingerprintDatabase_t *pDb,
                              uint32_t fingerprintSetKey,
                              uint32_t fingerprintIndex,
                              uint64_t secure_user_id)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (fingerprintIndex >= FPC_DB_FINGERPRINTS_IN_SET) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t *set = NULL;
    int lookup = fpc_db_get_set( pDb, fingerprintSetKey, &set );
    if (lookup != 0) {
        return lookup;
    }

    Fingerprint_t* finger = &(set->aFingerprintList[fingerprintIndex]);
    if (finger->valid == FPC_DB_ENTRY_INVALID) {
        return -FPC_ERROR_NOENTITY;
    }
    finger->secure_user_id = secure_user_id;
    return 0;
}

int fpc_db_get_fingerprint_set_id(FingerprintDatabase_t *pDb,
                                   uint32_t fingerprintSetKey,
                                   id256_t* pId)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t *set = NULL;
    int lookup = fpc_db_get_set( pDb, fingerprintSetKey, &set );
    if (lookup != 0) {
        return lookup;
    }

    id256_t* id = &(set->fingerprintSetId);
    if (id == NULL) {
        return -FPC_ERROR_NOENTITY;
    }

    memcpy(pId, id, sizeof(*pId));

    return 0;
}

int fpc_db_get_fingerprint_state_id(FingerprintDatabase_t *pDb,
                                    uint32_t fingerprintSetKey,
                                    id256_t* pId)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t *set = NULL;
    int lookup = fpc_db_get_set( pDb, fingerprintSetKey, &set );
    if (lookup != 0) {
        return lookup;
    }

    id256_t* id = &(set->fingerprintStateId);
    if (id == NULL) {
        return -FPC_ERROR_NOENTITY;
    }

    memcpy(pId, id, sizeof(*pId));
    return 0;
}

int fpc_db_trim_id_32(id256_t* id, uint32_t *pId32)
{
    uint32_t result32 = 0;
    if (pId32 == NULL)
    {
        return -FPC_ERROR_INPUT;
    }
    *pId32 = 0;
    for(int i = 0; i < 4; i++)
    {
        result32 = (result32 << 8) | ((uint32_t) (*id)[i]);
    }
    *pId32 = result32;
    return 0;
}

int fpc_db_trim_id_64(id256_t* id, uint64_t *pId64)
{
    uint64_t result64 = 0;
    if (pId64 == NULL)
    {
    return -FPC_ERROR_INPUT;
    }
    *pId64 = 0;
    for(int i = 0; i<8; i++)
    {
    result64 = (result64 << 8) | ((uint64_t) (*id)[i]);
    }
    *pId64 = result64;
    return 0;
}

int fpc_db_add_fingerprint(FingerprintDatabase_t *pDb,
                           uint32_t fingerprintSetKey,
                           uint64_t secure_user_id,
                           fpc_algo_template_t* pTemplate,
                           uint32_t* pIndex)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }
    if (pTemplate == NULL) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t* set = NULL;
    int lookup = fpc_db_get_set( pDb, fingerprintSetKey, &set );
    switch (lookup) {
    case 0:
        break;
    case -FPC_ERROR_NOENTITY:
        break;
    default:
        return lookup;
    }

    if ((lookup == -FPC_ERROR_NOENTITY) && (pDb->pFingerprintSet != NULL)) {
        return -FPC_ERROR_INPUT;
    }

    if (lookup == -FPC_ERROR_NOENTITY) {
        // Create a new set
        set = fpc_mem_calloc( 1, sizeof(FingerprintSet_t) );
        if (set == NULL) {
            return -FPC_ERROR_ALLOC;
        }
        set->fingerprintSetKey = fingerprintSetKey;
        if (db_generate_unique_id(&(set->fingerprintSetId))) {
            free(set);
            return -FPC_ERROR_IO;
        }
        for (int i = 0; i < FPC_DB_FINGERPRINTS_IN_SET; i++) {
          set->aFingerprintList[i].valid = FPC_DB_ENTRY_INVALID;
        }

        int result = db_add_set(pDb, set);
        if (result != 0) {
            free(set);
            return result;
        }
    }

    //Determine which finger index to be inserted at
    int fingerIndex = -1;
    for (int i = 0; i < FPC_DB_FINGERPRINTS_IN_SET; i++) {
        if (set->aFingerprintList[i].valid == FPC_DB_ENTRY_INVALID) {
            fingerIndex = i;
            break;
        }
    }

    if (fingerIndex == -1) {
        return -FPC_ERROR_NOSPACE;
    }

    Fingerprint_t* finger = &(set->aFingerprintList[fingerIndex]);

    if (db_generate_unique_id(&(finger->fingerprintId))) {
        return -FPC_ERROR_IO;
    }

    finger->valid = FPC_DB_ENTRY_VALID;
    finger->ftemplate.tpl = pTemplate->tpl;
    finger->ftemplate.size = pTemplate->size;
    finger->secure_user_id = secure_user_id;

    if (pIndex != NULL) {
        *pIndex = fingerIndex;
    }

    db_update_database_id(pDb);
    db_update_fingerprint_state_id(pDb);
    return 0;
}

static uint32_t db_count_fingers_in_set(FingerprintSet_t* set)
{
    uint32_t count = 0;
    for (int i = 0; i < FPC_DB_FINGERPRINTS_IN_SET; i++) {
        if (set->aFingerprintList[i].valid == FPC_DB_ENTRY_VALID) {
            count++;
        }
    }

    return count;
}

int fpc_db_delete_fingerprint(FingerprintDatabase_t *pDb,
                              uint32_t fingerprintSetKey,
                              uint32_t index)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (index >= FPC_DB_FINGERPRINTS_IN_SET) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t* set = NULL;
    int lookup = fpc_db_get_set(pDb, fingerprintSetKey, &set);
    if (lookup != 0) {
        return lookup;
    }

    Fingerprint_t* finger = &(set->aFingerprintList[index]);
    finger->valid = FPC_DB_ENTRY_INVALID;

    if (finger->ftemplate.tpl != NULL) {
        memset(finger->ftemplate.tpl, 0xCC, finger->ftemplate.size);
        free(finger->ftemplate.tpl);
    }

    finger->ftemplate.tpl = NULL;
    finger->ftemplate.size = 0;
    memset(&(finger->fingerprintId[0]), 0xCC, sizeof(id256_t));

    uint32_t count = db_count_fingers_in_set(set);
    if (count == 0) {
        return fpc_db_delete_set(pDb, fingerprintSetKey);
    }

    return 0;
}

int fpc_db_get_fingerprint_count(FingerprintDatabase_t *pDb,
                                 uint32_t fingerprintSetKey,
                                 uint32_t* pCount)
{
    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pCount == NULL) {
        return -FPC_ERROR_INPUT;
    }

    *pCount = 0;
    FingerprintSet_t* set = NULL;
    int lookup = fpc_db_get_set(pDb, fingerprintSetKey, &set);
    switch(lookup) {
    case 0:
      break;
    case -FPC_ERROR_NOENTITY:
      return 0;
    default:
      return lookup;
    }
    uint32_t count = db_count_fingers_in_set(set);
    *pCount = count;
    return 0;
}

int fpc_db_get_indices(FingerprintDatabase_t *pDb,
                       uint32_t fingerprintSetKey,
                       uint32_t* pIndices,
                       uint32_t indicesSize,
                       uint32_t* pCount)
{
    uint32_t i;

    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pIndices == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pCount == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (indicesSize == 0 || indicesSize > FPC_DB_FINGERPRINTS_IN_SET) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t* set = NULL;
    int lookup = fpc_db_get_set( pDb, fingerprintSetKey, &set );
    if (lookup == -FPC_ERROR_NOENTITY) {
        *pCount = 0;
        return 0;
    } else if (lookup != 0) {
        return lookup;
    }

    for(i = 0; i < indicesSize; i++) {
        pIndices[i] = 0;
    }

    uint32_t count = 0;
    for (i = 0; i < FPC_DB_FINGERPRINTS_IN_SET && count < indicesSize; i++) {
        if (set->aFingerprintList[i].valid == FPC_DB_ENTRY_VALID) {
          pIndices[count++] = i;
        }
    }

    *pCount = count;
    return 0;
}

int fpc_db_get_template(FingerprintDatabase_t *pDb, uint32_t fingerprintSetKey,
                        uint32_t index, fpc_algo_template_t** ppTpl)
{

    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (ppTpl == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (index >= FPC_DB_FINGERPRINTS_IN_SET) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t* set = NULL;
    int lookup = fpc_db_get_set(pDb, fingerprintSetKey, &set);
    if (lookup != 0) {
        return lookup;
    }

    if (set->aFingerprintList[index].valid == FPC_DB_ENTRY_INVALID) {
        return -FPC_ERROR_NOENTITY;
    }

    fpc_algo_template_t* pTpl = &(set->aFingerprintList[index].ftemplate);
    *ppTpl = pTpl;
    return 0;
}

int fpc_db_set_template_first(FingerprintDatabase_t* pDb, uint32_t i)
{
    FingerprintSet_t* set = pDb->pFingerprintSet;
    Fingerprint_t temp_template = set->aFingerprintList[i];

    for (; i > 0; i--) {
        set->aFingerprintList[i] = set->aFingerprintList[i-1];
    }

    set->aFingerprintList[i] = temp_template;
    return 0;
}

int fpc_db_get_all_templates(FingerprintDatabase_t* pDb,
                             fpc_algo_template_t* list,
                             uint32_t* size_list)
{
    FingerprintSet_t* set = pDb->pFingerprintSet;
    uint32_t i;

    if (!set) {
        *size_list = 0;
        return 0;
    }

    uint32_t valid_count = 0;

    for (i = 0; i < FPC_DB_FINGERPRINTS_IN_SET; ++i) {

        if (valid_count >= *size_list) {
            break;
        }

        if (set->aFingerprintList[i].valid == FPC_DB_ENTRY_VALID) {
            list[valid_count] = set->aFingerprintList[i].ftemplate;
            ++valid_count;
        }
    }

    *size_list = valid_count;

    return 0;
}

int fpc_db_get_templates(FingerprintDatabase_t *pDb,
                         uint32_t fingerprintSetKey,
                         uint32_t* pIndices,
                         uint32_t size,
                         fpc_algo_template_t* pList)
{

    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (pIndices == NULL) {
        return -FPC_ERROR_INPUT;
    }

    if (size == 0 || size > FPC_DB_FINGERPRINTS_IN_SET) {
        return -FPC_ERROR_INPUT;
    }

    if (pList == NULL) {
        return -FPC_ERROR_INPUT;
    }

    FingerprintSet_t* set = NULL;
    int lookup = fpc_db_get_set(pDb, fingerprintSetKey, &set);
    if (lookup != 0) {
        return lookup;
    }

    for (uint32_t i = 0; i < size; i++) {
        uint32_t index = pIndices[i];
        fpc_algo_template_t* ptemplate = pList + i;
        if (index >= FPC_DB_FINGERPRINTS_IN_SET) {
            return -FPC_ERROR_INPUT;
        }
        if (set->aFingerprintList[index].valid == FPC_DB_ENTRY_INVALID) {
          return -FPC_ERROR_NOENTITY;
        }
        ptemplate->tpl = set->aFingerprintList[index].ftemplate.tpl;
        ptemplate->size = set->aFingerprintList[index].ftemplate.size;
    }
    return 0;
}

int fpc_db_get_database_id(FingerprintDatabase_t *pDb, uint64_t* id)
{

    if (pDb == NULL) {
        return -FPC_ERROR_INPUT;
    }

    *id = pDb->databaseId;

    return 0;
}

int fpc_db_get_index_of_id32(FingerprintDatabase_t* pDb,
                             uint32_t fingerprint_id32,
                             uint32_t* index)
{
    FingerprintSet_t* set = pDb->pFingerprintSet;

    uint32_t i;

    if (!set) {
        return -FPC_ERROR_NOENTITY;
    }

    for (i = 0; i < FPC_DB_FINGERPRINTS_IN_SET; ++i) {
        Fingerprint_t* finger = set->aFingerprintList + i;

        if (finger->valid == FPC_DB_ENTRY_VALID) {
            uint32_t id32;
            fpc_db_trim_id_32(&finger->fingerprintId, &id32);
            if (id32 == fingerprint_id32) {
                *index = i;
                return 0;
            }
        }
    }

    return -FPC_ERROR_NOENTITY;
}

int fpc_db_get_index_of_template(FingerprintDatabase_t* pDb,
                                 fpc_algo_template_t* tpl,
                                 uint32_t* index)
{
    FingerprintSet_t* set = pDb->pFingerprintSet;
    uint32_t i;

    if (!set) {
        return -FPC_ERROR_NOENTITY;
    }

    for (i = 0; i < FPC_DB_FINGERPRINTS_IN_SET; ++i) {
        Fingerprint_t* finger = set->aFingerprintList + i;

        if (finger->valid == FPC_DB_ENTRY_VALID) {
            if (finger->ftemplate.tpl == tpl->tpl) {
                *index = i;
                return 0;
            }
        }
    }

    return -FPC_ERROR_NOENTITY;
}
