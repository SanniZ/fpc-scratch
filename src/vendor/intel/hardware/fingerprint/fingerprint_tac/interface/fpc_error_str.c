/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include "fpc_types.h"

const char* fpc_error_str(int err)
{
  switch(err)
  {
    case -FPC_ERROR_INPUT:
      return "FPC_ERROR_INPUT";
    case -FPC_ERROR_TIMEDOUT:
      return "FPC_ERROR_TIMEDOUT";
    case -FPC_ERROR_ALLOC:
      return "FPC_ERROR_ALLOC";
    case -FPC_ERROR_COMM:
      return "FPC_ERROR_COMM";
    case -FPC_ERROR_NOSPACE:
      return "FPC_ERROR_NOSPACE";
    case -FPC_ERROR_IO:
      return "FPC_ERROR_IO";
    case -FPC_ERROR_CANCELLED:
      return "FPC_ERROR_CANCELLED";
    case -FPC_ERROR_NOENTITY:
      return "FPC_ERROR_NOENTITY";
    case -FPC_ERROR_HARDWARE:
      return "FPC_ERROR_HARDWARE";
    case -FPC_ERROR_CONFIG:
      return "FPC_ERROR_CONFIG";
    case -FPC_ERROR_NOT_INITIALIZED:
      return "FPC_ERROR_NOT_INITIALIZED";
    case -FPC_ERROR_RESET_HARDWARE:
      return "FPC_ERROR_RESET_HARDWARE";
    case -FPC_ERROR_PN:
      return "FPC_ERROR_PN";
    case -FPC_ERROR_CRYPTO:
      return "FPC_ERROR_CRYPTO";
    case -FPC_ERROR_DB:
      return "FPC_ERROR_DB";
    case -FPC_ERROR_TEMPLATE_CORRUPTED:
      return "FPC_ERROR_TEMPLATE_CORRUPTED";
    default:
      return "UNKNOWN ERROR";
  }
}
