/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <stdio.h>
#include <string.h>
#include "fpc_log.h"
#include "fpc_debug.h"

#define DBG_LOG_STR_MAX 1024

void fpc_tee_debug(const char* tag, const char* format, ...) {
  va_list args;
  char log_str[DBG_LOG_STR_MAX];

  /* Temporary workaround for not trying to print floats
   * that will crash the MSM9884 platform */
  if (strstr(format, "%f") || strstr(format, "%.") || strstr(format, "%10f")) {
    LOGD("Trying to print floats%s", format);
  } else {
    va_start(args, format);

    // Overflow check
    if (vsnprintf(log_str, DBG_LOG_STR_MAX, format, args) >= DBG_LOG_STR_MAX) {
      LOGE("%s - Trying to print a too long line", __func__);
      log_str[DBG_LOG_STR_MAX - 1] = '\0';
    }

    LOGD("%s%s", tag, log_str);
    va_end(args);
  }
}

void fpc_tee_error(const char* tag, const char* format, ...) {
  va_list args;
  char log_str[DBG_LOG_STR_MAX];

  /* Temporary workaround for not trying to print floats
   * that will crash the MSM9884 platform */
  if (strstr(format, "%f") || strstr(format, "%.") || strstr(format, "%10f")) {
    LOGD("Trying to print floats%s", format);
  } else {
    va_start(args, format);

    // Overflow check
    if (vsnprintf(log_str, DBG_LOG_STR_MAX, format, args) >= DBG_LOG_STR_MAX) {
      LOGE("%s - Trying to print a too long line", __func__);
      log_str[DBG_LOG_STR_MAX - 1] = '\0';
    }

    LOGE("%s%s", tag, log_str);
    va_end(args);
  }
}

