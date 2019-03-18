/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdint.h>
#include <stddef.h>

#include "fpc_crypto.h"

//If we dont have HW_AUTH enabled, we need to stub this function,
//otherwise it is implemented in the hw_auth module.
#ifndef FPC_CONFIG_HW_AUTH
#include <stdbool.h>

typedef struct fpc_bio fpc_bio_t;
int fpc_enrollment_allowed(fpc_bio_t* bio, uint64_t* user)
{
    (void) bio;
    *user = 0;
    return true;
}
#endif

int fpc_crypto_memcmp(const uint8_t *mem_1, const uint8_t *mem_2, size_t nbr_bytes) {
  size_t i;
  uint8_t match = 0;

  for (i = 0; i < nbr_bytes; ++i) {
      match = match | (mem_1[i] ^ mem_2[i]);
  }

  return match;
}

