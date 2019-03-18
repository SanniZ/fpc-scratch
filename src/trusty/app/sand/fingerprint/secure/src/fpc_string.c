/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include "fpc_string.h"
#include <stddef.h>

size_t __wrap_strnlen(const char *s, size_t maxlen)
{
    const char *p = s;

    while (maxlen-- && *s) {
        ++s;
    }

    return (size_t)(s - p);
}

char *__wrap_strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;

    for (; n; n--) {
        *dest = *src;

        if (*src == 0) {
            break;
        }

        dest++;
        src++;
    }

    while (n--) {
        *dest++ = 0;
    }

    return ret;
}

