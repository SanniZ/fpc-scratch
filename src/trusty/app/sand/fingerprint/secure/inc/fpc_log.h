/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 * LOGD(ARG) : Debug warning print, where ARG is the same syntax of printf(ARG)
 * LOGE(ARG) : Error print, where ARG is the same syntax of printf(ARG). The program will exit when using this with error code 1
 */
#ifndef _FPC_LOG_H_
#define _FPC_LOG_H_

void fpc_tee_debug(const char* tag, const char* format, ...);
void fpc_tee_error(const char* tag, const char* format, ...);
void fpc_debug_circular_log(char* format, ...);

#if defined(_TEE_DEBUG_)
  #ifndef FPC_LOG_TAG
     #define FPC_LOG_TAG "FPC LIB"
  #endif
  #define LOGD(f, ...)    fpc_tee_debug("["FPC_LOG_TAG"] ", f, ## __VA_ARGS__)
  #define LOGD_STR(__str) fpc_tee_debug("", "%s", __str)
  #define LOGE(f, ...)    fpc_tee_debug("["FPC_LOG_TAG"] ", f, ## __VA_ARGS__)
  #define LOGE_STR(__str) fpc_tee_debug("", "%s", __str)
  #define LOGI(f, ...)    fpc_tee_debug("["FPC_LOG_TAG"] ", f, ## __VA_ARGS__)
  #define LOGS(f, ...)    fpc_tee_debug("["FPC_LOG_TAG"] ", f, ## __VA_ARGS__)
  #define LOG_ENTER() LOGD("--> %s", __func__)
  #define LOG_LEAVE() LOGD("<-- %s", __func__)
  #define LOG_LEAVE_TRACE(ret) LOGD("<-- %s, return: %d", __func__, ret)
#elif defined(FPC_CONFIG_LOGGING_IN_RELEASE)
  #if defined(FPC_CONFIG_LOGGING_IN_RELEASE_FILE)
    #define LOGD(f, ...)
    #define LOGD_STR(...)
    #define LOGE(f, ...)    fpc_debug_circular_log(f, ## __VA_ARGS__)
    #define LOGE_STR(__str)
    #define LOGI(f, ...)
    #define LOGS(f, ...)
    #define LOG_ENTER()
    #define LOG_LEAVE()
    #define LOG_LEAVE_TRACE(ret)
  #else
    #define LOGD(f, ...)
    #define LOGD_STR(...)
    #define LOGE(f, ...)    fpc_tee_error("", f, ## __VA_ARGS__)
    #define LOGE_STR(__str) fpc_tee_error("", "%s", __str)
    #define LOGI(...)
    #define LOGS(...)
    #define LOG_ENTER()
    #define LOG_LEAVE()
    #define LOG_LEAVE_TRACE(ret)
  #endif
#else
  #define LOGD(...)
  #define LOGD_STR(...)
  #define LOGE(...)
  #define LOGE_STR(__str)
  #define LOGI(...)
  #define LOGS(...)
  #define LOG_ENTER()
  #define LOG_LEAVE()
  #define LOG_LEAVE_TRACE(ret)
#endif

#endif /* _FPC_LOG_H_ */
