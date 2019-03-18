/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_LOG_H
#define FPC_LOG_H

#ifdef NO_ANDROID

#include <stdio.h>

#define LOGE(...) printf(__VA_ARGS__)
#define LOGI(...) printf(__VA_ARGS__)
#define LOGD(...) printf(__VA_ARGS__)

#else

#include <cutils/log.h>

#ifdef FPC_DEBUG_LOGGING
#define LOGD(...) ALOGD(__VA_ARGS__)
#else
#define LOGD(...) {}
#endif

#define LOGI(...) ALOGI(__VA_ARGS__)

#define LOGE(...) ALOGE(__VA_ARGS__)

#endif // NO_ANDROID
#endif // FPC_LOG_H

