/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TAC_LOG
#define INCLUSION_GUARD_FPC_TAC_LOG

#ifndef NDK_ROOT
//#ifdef NDK_ROOT
  #if defined(__ANDROID_API__)
    #include "utils/Log.h"
    #define LOGD(...) ALOGD(__VA_ARGS__)
    #define LOGI(...) ALOGI(__VA_ARGS__)
    #define LOGE(...) ALOGE(__VA_ARGS__)
  #elif defined(__FPC_SW_TEE__)
    #define LOGD(...) {}
    #define LOGI(...) {}
    #define LOGE(...) {}
  #else // __ANDROID_API__
    #include <stdio.h>
    #include <stdlib.h>
    #define LOGD(...) { \
      printf("WARNING (%s:%i) ",LOG_TAG,__LINE__); \
      printf(__VA_ARGS__); }
    #define LOGI(...) { \
      printf("INFO (%s:%i) ",LOG_TAG,__LINE__); \
      printf(__VA_ARGS__); }
    #define LOGE(...) { \
      printf("ERROR (%s:%i) ",LOG_TAG,__LINE__); \
      printf(__VA_ARGS__); \
      exit(1);}
  #endif // __ANDROID_API__
#else // NDK_ROOT
  #include <android/log.h>
  #define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
  #define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
  #define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif
#endif //INCLUSION_GUARD_FPC_TAC_LOG
