/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
#ifndef FPC_STRING_H
#define FPC_STRING_H
#include <stddef.h>

size_t __wrap_strnlen(const char *s, size_t maxlen);

char *__wrap_strncpy(char *dest, const char *src, size_t n);

#endif
