/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
#include "test_main.h"
#include "../secure/src/fpc_snprintf.c"

#include "time.h"
#include <string.h>

#define TEST_BUF_SZ 50

TEST_GROUP(GROUP_SNPRINTF);


TEST_SETUP(GROUP_SNPRINTF)
{
}

TEST_TEAR_DOWN(GROUP_SNPRINTF)
{
}

static void run_supported_test(const char* format, ...)
{
    char actual[TEST_BUF_SZ];
    char expected[TEST_BUF_SZ];
    va_list args_copy;
    actual[sizeof(actual) - 1] = '\0';
    expected[sizeof(expected) - 1] = '\0';

    va_list args;
    va_start(args, format);
    for (size_t i = 0; i < TEST_BUF_SZ; i++) {
        memset(actual, 'z', sizeof(actual) - 1);
        memset(expected, 'z', sizeof(expected) - 1);
        va_copy(args_copy, args);
        int len = fpc_vsnprintf(actual, i, format, args_copy);
        va_copy(args_copy, args);
        int expected_len = vsnprintf(expected, i, format, args_copy);
        TEST_ASSERT_EQUAL_STRING(expected, actual);
        TEST_ASSERT_EQUAL_INT(expected_len, len);
        TEST_ASSERT_EQUAL_MEMORY(expected, actual, TEST_BUF_SZ);
    }
    va_end(args);
}

TEST(GROUP_SNPRINTF, SNPRINTF_SUPPORTED)
{
    run_supported_test("%d", 1234567890);
    run_supported_test("%X", 0x1234ABCD);
    run_supported_test("%d", -1);
    run_supported_test("%X", 0x1234ABCD);
    run_supported_test("%ld", 2147483647);
    run_supported_test("%lld", 9223372036854775807);
    run_supported_test("%llu", 9223372036854775807);
    run_supported_test("%llu", 0xffffffffffffffff);
    run_supported_test("%llx", 0x7fffffffffffffff);
    run_supported_test("%llx", 0xffffffffffffffff);
    run_supported_test("%%");

    run_supported_test("%10d", 123);
    run_supported_test("%*d", 10, 123);

    run_supported_test("%.*d", 10, 123);
    run_supported_test("%.0d", 0);

    run_supported_test("I am a %s that tests", "test program");

    run_supported_test("%p", &run_supported_test);

    run_supported_test("Pointer to this function is %p", &run_supported_test);
    run_supported_test("Pointer to this function is %p %s", &run_supported_test, "string");

    run_supported_test("This code is written by %c%c%c", 'F', 'P', 'C');

    run_supported_test("This %d %c %p %x", -32768, 'P', &run_supported_test, 0xFF);

    run_supported_test("%02x", 0x1);
    run_supported_test("%02x", 0x20);

    /* Care should be taken when testing rounding as the double
     * representation of some numbers are not exact. E.g.
     * 0.35 = 0.34999999999999997780
     * 3.5 = 3.50000000000000000000
     */
    run_supported_test("%f", 1.5);
    run_supported_test("%f", 15000.5);

    run_supported_test("%.2f", 1.55);
    run_supported_test("%.2f", 1.0055);
    run_supported_test("%4.4f", 3.14);
    run_supported_test("%04.4f", 3.14);
    run_supported_test("%f", -3.14);

    run_supported_test("%.2f", -1.55);
    run_supported_test("%.2f", -1.0055);
    run_supported_test("%.0f", 0.555555);
    run_supported_test("%.20f", 0.0000000000008);

    run_supported_test("%.0f", 3.5);
    run_supported_test("%.1f", .35);

    run_supported_test("%f", (double)(0x7ffffffffffff000));
    run_supported_test("%f", -(double)(0x7ffffffffffff000));

    run_supported_test("%10s\n", "Test");
    run_supported_test("%*s%d\n", 0, "", 55);
    run_supported_test("%*s%d\n", 10, "", 55);
    run_supported_test("%*s\n", 5, "Test");
    run_supported_test("%*s\n", 0, "Test");
}

static void run_non_supported_test(const char* expected, int expected_len, const char* format, ...)
{
    char actual[TEST_BUF_SZ];
    actual[sizeof(actual) - 1] = '\0';

    va_list args;
    va_start(args, format);
    memset(actual, 'z', sizeof(actual));
    int len = fpc_vsnprintf(actual, TEST_BUF_SZ, format, args);

    TEST_ASSERT_EQUAL_STRING(expected, actual);
    TEST_ASSERT_EQUAL_INT(expected_len, len);

    va_end(args);
}

TEST(GROUP_SNPRINTF, SNPRINTF_NOT_SUPPORTED)
{
    // Fails due to different rounding method used in Linux
    run_non_supported_test("3", 1, "%.0f", 2.5);
    run_non_supported_test("0.3", 3, "%.1f", .25);

    double big_number = (double)(0x7ffffffffffff000);
    big_number += 10000.0;
    run_non_supported_test("Err", 3, "%f", big_number);
    run_non_supported_test("Err", 3, "%f", -big_number);
}

TEST(GROUP_SNPRINTF, SNPRINTF_SPECIAL)
{
    int written_so_far = 0;
    char actual[TEST_BUF_SZ];
    char * expected = "A special test";
    int expected_len = 14;
    int expected_so_far = 10;

    int len = fpc_snprintf(actual, TEST_BUF_SZ, "A special %ntest", &written_so_far);

    TEST_ASSERT_EQUAL_STRING(expected, actual);
    TEST_ASSERT_EQUAL_INT(expected_len, len);
    TEST_ASSERT_EQUAL_INT(expected_so_far, written_so_far);
}

static double get_duration(struct timespec* start, struct timespec* stop)
{
    double ns = (double) (stop->tv_sec - start->tv_sec) * 1.0e9
            + (double) (stop->tv_nsec - start->tv_nsec);
    return ns;
}
#define TEST_ROUNDS 1000000

static void run_performance_test(char* buffer, size_t len, char* format, ...)
{
    va_list args;
    va_list args_copy;
    va_start(args, format);
    struct timespec start, stop;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    for (size_t i = 0; i < TEST_ROUNDS; i++) {
        va_copy(args_copy, args);
        vsnprintf(buffer, len, format, args_copy);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
    double builtin_dur = get_duration(&start, &stop);
    printf("\nBuiltin version took %.0f ns (=%f ns each)\n", builtin_dur, builtin_dur / TEST_ROUNDS);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int i = 0; i < TEST_ROUNDS; i++) {
        va_copy(args_copy, args);
        fpc_vsnprintf(buffer, len, format, args_copy);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
    double fpc_dur = get_duration(&start, &stop);
    printf("\nFPC version took %.0f ns (=%f ns each)\n", fpc_dur, fpc_dur / TEST_ROUNDS);

    printf("\nBuilt in version is %f times faster\n", fpc_dur / builtin_dur);
    va_end(args);
}

TEST(GROUP_SNPRINTF, SNPRINTF_PERFORMANCE)
{
    char actual[TEST_BUF_SZ];
    run_performance_test(actual, TEST_BUF_SZ, "A %s %d %X %f", "string is", 9876543, 0xFFFFF123,
            1.4545);
}

TEST_GROUP_RUNNER(GROUP_SNPRINTF)
{
    RUN_TEST_CASE(GROUP_SNPRINTF, SNPRINTF_SUPPORTED);
    RUN_TEST_CASE(GROUP_SNPRINTF, SNPRINTF_NOT_SUPPORTED);
    RUN_TEST_CASE(GROUP_SNPRINTF, SNPRINTF_SPECIAL);
    RUN_TEST_CASE(GROUP_SNPRINTF, SNPRINTF_PERFORMANCE);
}

