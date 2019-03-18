/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
#ifndef FPC_SNPRINTF_H
#define FPC_SNPRINTF_H
#include <stdint.h>
#include <stdarg.h>

/**
 * This function implements a limited version of vsnprintf. See test_fpc_snprintf.c
 * for supported formatting parameters.
 *
 * @param[out] buffer   Pointer to a character string to write to
 * @param[in]  size     Maximum number of characters to be written to buffer
 * @param[in]  format   Pointer to a null-terminated format string
 * @param[out] args     Pointer va_list arguments
 *
 * @returns Return the number of characters printed (excluding the null byte used
 *          to end output to strings) if success. This means that a value of size
 *          or more indicates that the buffer was truncated and not null-terminated.

 *          In case of failure a negative value is returned.
 */
int fpc_vsnprintf(char *buffer, size_t size, const char *format, va_list args);

/**
 * This function implements a limited version of snprintf. See test_fpc_snprintf.c
 * for supported formatting parameters.
 *
 * @param[out] buffer   Pointer to a character string to write to
 * @param[in]  size     Maximum number of characters to be written to buffer
 * @param[in]  format   Pointer to a null-terminated format string
 *
 * @returns Return the number of characters printed (excluding the null byte used
 *          to end output to strings) if success. This means that a value of size
 *          or more indicates that the buffer was truncated and not null-terminated.

 *          In case of failure a negative value is returned.
 */
int fpc_snprintf(char *buffer, size_t size, const char *format, ...);

#endif
