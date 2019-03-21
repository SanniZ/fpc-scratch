/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

/* LOGD(ARG) : Debug warning print, where ARG is the same syntax of printf(ARG)
 * LOGE(ARG) : Error print, where ARG is the same syntax of printf(ARG). The program will exit when using this with error code 1
 */

#ifndef _FPC_LOG_H_
#define _FPC_LOG_H_

#ifdef __CONSOLE_TEST__
  #define LOGD(...)   printf(__VA_ARGS__)
  #define LOGE(...)   printf(__VA_ARGS__)
  #define LOGI(...)   printf(__VA_ARGS__)
  #define LOGS(...)   printf(__VA_ARGS__)
#else
  #include "fpc_ta_debug.h"

  #define __FPC_SW_TEE__
#endif

#if defined(__FPC_SW_TEE__)
   void* malloc(size_t size);
   void free(void *ptr);

  #if defined(_TEE_DEBUG_)
    void fpc_tee_debug(const char* tag, const char* fmt, ...);
#ifdef __BLACKBOX_TEST__
    #define LOGD(...)   fpc_tee_debug("[FPC LIB] ", __VA_ARGS__)
    #define LOGE(...)   fpc_tee_debug("[FPC LIB] ", __VA_ARGS__)
    #define LOGI(...)   fpc_tee_debug("[FPC LIB] ", __VA_ARGS__)
    #define LOGS(...)   fpc_tee_debug("[FPC LIB] ", __VA_ARGS__)
#else
    #include <trusty_std.h>
    #include <stdio.h>
    #define LOGD(...)   fprintf(stderr,__VA_ARGS__); \
                        fprintf(stderr, "\n")
    #define LOGE(...)   fprintf(stderr, __VA_ARGS__); \
                        fprintf(stderr, "\n")
    #define LOGI(...)   fprintf(stderr,  __VA_ARGS__); \
                        fprintf(stderr, "\n")
    #define LOGS(...)   fprintf(stderr,  __VA_ARGS__); \
                        fprintf(stderr, "\n")
#endif /* __BLACKBOX__ */

  #elif defined(FPC_CONFIG_LOGGING_IN_RELEASE) /* __TEE_DEBUG__ */
    #include <trusty_std.h>
    #include <stdio.h>
    #define LOGD(...)
    #define LOGE(...)   fprintf(stderr,  __VA_ARGS__); \
                        fprintf(stderr, "\n")
    #define LOGI(...)
    #define LOGS(...)

  #else /* __TEE_DEBUG__ */
    #define LOGD(...)
    #define LOGE(...)
    #define LOGI(...)
    #define LOGS(...)
  #endif /* __TEE_DEBUG__ */
#endif /* __FPC_SW_TEE__ */

#endif /* _FPC_LOG_H_ */
