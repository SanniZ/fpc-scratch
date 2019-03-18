/**
 * @copyright
 * Copyright (c) 2016-2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <errno.h>

#include "test_main.h"

static void RunAllTests(void)
{
    RUN_TEST_GROUP(GROUP_SNPRINTF);
}

FILE *g_tests_log;

void tests_log_char(int c)
{
    /* Emit test names, PASS or FAIL etc both to stdout and to the log file */
    (void)putchar(c);
    putc(c, g_tests_log);
}

int main(int argc, const char *argv[])
{
    int ret = 0;

    g_tests_log = fopen("test_fingerprint_pal.txt", "w");
    if (g_tests_log) {
        ret = UnityMain(argc, argv, RunAllTests);
        fclose(g_tests_log);
    } else {
        ret = -errno;
    }

    return ret;
}
