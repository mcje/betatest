#ifndef BETATEST_H
#define BETATEST_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Configuration */
#ifndef BETATEST_NO_COLOR
#define BETATEST_USE_COLOR 1
#else
#define BETATEST_USE_COLOR 0
#endif

#ifndef BETATEST_PRINT_ON_TEST
#define BETATEST_DO_PRINT_TEST 0
#else
#define BETATEST_DO_PRINT_TEST 1
#endif

#ifndef BETATEST_PRINT_ON_PASS
#define BETATEST_DO_PRINT_PASS 0
#else
#define BETATEST_DO_PRINT_PASS 1
#endif

#ifndef BETATEST_PRINT_NOT_ON_FAIL
#define BETATEST_DO_PRINT_FAIL 1
#else
#define BETATEST_DO_PRINT_FAIL 0
#endif

/* Color codes */
#if BETATEST_USE_COLOR
#define BETATEST_COLOR_GREEN "\033[32m"
#define BETATEST_COLOR_RED "\033[31m"
#define BETATEST_COLOR_YELLOW "\033[33m"
#define BETATEST_COLOR_CYAN "\033[36m"
#define BETATEST_COLOR_RESET "\033[0m"
#define BETATEST_COLOR_BOLD "\033[1m"
#else
#define BETATEST_COLOR_GREEN ""
#define BETATEST_COLOR_RED ""
#define BETATEST_COLOR_YELLOW ""
#define BETATEST_COLOR_CYAN ""
#define BETATEST_COLOR_RESET ""
#define BETATEST_COLOR_BOLD ""
#endif

/* Test statistics */
static struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
    int assertions_run;
    int assertions_passed;
    int assertions_failed;
    int current_test_failed;
    char *current_test_name;
} betatest_stats = {0, 0, 0, 0, 0, 0, 0, 0};

/* Print helpers */
#define BETATEST_PRINT_PASS()                                                  \
    printf("%s[PASS]%s ", BETATEST_COLOR_GREEN, BETATEST_COLOR_RESET)

#define BETATEST_PRINT_FAIL()                                                  \
    printf("%s[FAIL]%s ", BETATEST_COLOR_RED, BETATEST_COLOR_RESET)

#define BETATEST_PRINT_INFO()                                                  \
    printf("%s[INFO]%s ", BETATEST_COLOR_CYAN, BETATEST_COLOR_RESET)

/* Test definition macros */
#define TEST(name)                                                             \
    static void test_##name(void);                                             \
    static void run_test_##name(void) {                                        \
        betatest_stats.current_test_name = #name;                              \
        betatest_stats.tests_run++;                                            \
        betatest_stats.current_test_failed = 0;                                \
        int print_nl = 0;                                                      \
        if (BETATEST_DO_PRINT_TEST) {                                          \
            printf("%s%s[TEST]%s %s%s\n", BETATEST_COLOR_BOLD,                 \
                   BETATEST_COLOR_CYAN, BETATEST_COLOR_RESET, #name,           \
                   BETATEST_COLOR_RESET);                                      \
            print_nl = 1;                                                      \
        }                                                                      \
        test_##name();                                                         \
        if (betatest_stats.current_test_failed) {                              \
            betatest_stats.tests_failed++;                                     \
        } else {                                                               \
            betatest_stats.tests_passed++;                                     \
            if (BETATEST_DO_PRINT_PASS) {                                      \
                BETATEST_PRINT_PASS();                                         \
                printf("%s\n", #name);                                         \
                print_nl = 1;                                                  \
            }                                                                  \
        }                                                                      \
        if (print_nl) {                                                        \
            printf("\n");                                                      \
        }                                                                      \
    }                                                                          \
    static void test_##name(void)

#define RUN_TEST(name) run_test_##name()

/* Assertion helpers */
#define BETATEST_RECORD_PASS()                                                 \
    do {                                                                       \
        betatest_stats.assertions_run++;                                       \
        betatest_stats.assertions_passed++;                                    \
    } while (0)

#define BETATEST_RECORD_FAIL(msg, ...)                                         \
    do {                                                                       \
        betatest_stats.assertions_run++;                                       \
        betatest_stats.assertions_failed++;                                    \
        betatest_stats.current_test_failed = 1;                                \
        if (BETATEST_DO_PRINT_FAIL) {                                          \
            BETATEST_PRINT_FAIL();                                             \
            printf("%s\n       ", betatest_stats.current_test_name);           \
            printf(msg, ##__VA_ARGS__);                                        \
            printf("\n       at %s:%d\n", __FILE__, __LINE__);                 \
        }                                                                      \
    } while (0)

/* Core assertion macros */
#define ASSERT_TRUE(cond)                                                      \
    do {                                                                       \
        if (cond) {                                                            \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: expected true, got false\n"                 \
                "       Expression: %s",                                       \
                #cond);                                                        \
        }                                                                      \
    } while (0)

#define ASSERT_FALSE(cond)                                                     \
    do {                                                                       \
        if (!(cond)) {                                                         \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: expected false, got true\n"                 \
                "       Expression: %s",                                       \
                #cond);                                                        \
        }                                                                      \
    } while (0)

#define ASSERT_EQ(a, b)                                                        \
    do {                                                                       \
        if ((a) == (b)) {                                                      \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: expected equal\n"          \
                                 "       Expression: %s == %s",                \
                                 #a, #b);                                      \
        }                                                                      \
    } while (0)

#define ASSERT_NEQ(a, b)                                                       \
    do {                                                                       \
        if ((a) != (b)) {                                                      \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: expected not equal\n"      \
                                 "       Expression: %s != %s",                \
                                 #a, #b);                                      \
        }                                                                      \
    } while (0)

#define ASSERT_NULL(ptr)                                                       \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: expected NULL\n"           \
                                 "       Expression: %s",                      \
                                 #ptr);                                        \
        }                                                                      \
    } while (0)

#define ASSERT_NOT_NULL(ptr)                                                   \
    do {                                                                       \
        if ((ptr) != NULL) {                                                   \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: expected not NULL\n"       \
                                 "       Expression: %s",                      \
                                 #ptr);                                        \
        }                                                                      \
    } while (0)

/* Integer comparison with value display */
#define ASSERT_INT_EQ(a, b)                                                    \
    do {                                                                       \
        long long _a = (long long)(a);                                         \
        long long _b = (long long)(b);                                         \
        if (_a == _b) {                                                        \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: integers not equal\n"      \
                                 "       1:  %s = %lld\n"                      \
                                 "       2:  %s = %lld",                       \
                                 #a, _a, #b, _b);                              \
        }                                                                      \
    } while (0)

#define ASSERT_INT_NEQ(a, b)                                                   \
    do {                                                                       \
        long long _a = (long long)(a);                                         \
        long long _b = (long long)(b);                                         \
        if (_a != _b) {                                                        \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: integers should not be equal\n"             \
                "       Both: %lld",                                           \
                _a);                                                           \
        }                                                                      \
    } while (0)

/* String comparison */
#define ASSERT_STR_EQ(s1, s2)                                                  \
    do {                                                                       \
        const char *_s1 = (s1);                                                \
        const char *_s2 = (s2);                                                \
        if (_s1 == NULL && _s2 == NULL) {                                      \
            BETATEST_RECORD_PASS();                                            \
        } else if (_s1 == NULL || _s2 == NULL) {                               \
            BETATEST_RECORD_FAIL("Assertion failed: one string is NULL\n"      \
                                 "       %s = %s\n"                            \
                                 "       %s = %s",                             \
                                 #s1, _s1 ? _s1 : "NULL", #s2,                 \
                                 _s2 ? _s2 : "NULL");                          \
        } else if (strcmp(_s1, _s2) == 0) {                                    \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: strings not equal\n"       \
                                 "       1:  %s = \"%s\"\n"                    \
                                 "       2:  %s = \"%s\"\n",                   \
                                 #s1, _s1, #s2, _s2);                          \
        }                                                                      \
    } while (0)

#define ASSERT_STR_NEQ(s1, s2)                                                 \
    do {                                                                       \
        const char *_s1 = (s1);                                                \
        const char *_s2 = (s2);                                                \
        if (_s1 == NULL || _s2 == NULL || strcmp(_s1, _s2) != 0) {             \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: strings should not be equal\n"              \
                "       Both: \"%s\"",                                         \
                _s1);                                                          \
        }                                                                      \
    } while (0)

/* Float/double comparison with epsilon */
#define ASSERT_FLOAT_EQ(a, b, epsilon)                                         \
    do {                                                                       \
        double _a = (double)(a);                                               \
        double _b = (double)(b);                                               \
        double _epsilon = (double)(epsilon);                                   \
        if (fabs(_a - _b) <= _epsilon) {                                       \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: floats not equal within epsilon\n"          \
                "       Got:      %s = %.10g\n"                                \
                "       Expected: %s = %.10g\n"                                \
                "       Epsilon:  %.10g\n"                                     \
                "       Diff:     %.10g",                                      \
                #a, _a, #b, _b, _epsilon, fabs(_a - _b));                      \
        }                                                                      \
    } while (0)

/* Comparison operators */
#define ASSERT_LT(a, b)                                                        \
    do {                                                                       \
        if ((a) < (b)) {                                                       \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: expected %s < %s", #a,     \
                                 #b);                                          \
        }                                                                      \
    } while (0)

#define ASSERT_LE(a, b)                                                        \
    do {                                                                       \
        if ((a) <= (b)) {                                                      \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: expected %s <= %s", #a,    \
                                 #b);                                          \
        }                                                                      \
    } while (0)

#define ASSERT_GT(a, b)                                                        \
    do {                                                                       \
        if ((a) > (b)) {                                                       \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: expected %s > %s", #a,     \
                                 #b);                                          \
        }                                                                      \
    } while (0)

#define ASSERT_GE(a, b)                                                        \
    do {                                                                       \
        if ((a) >= (b)) {                                                      \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: expected %s >= %s", #a,    \
                                 #b);                                          \
        }                                                                      \
    } while (0)

/* Custom assertion with message */
#define ASSERT_MSG(cond, msg, ...)                                             \
    do {                                                                       \
        if (cond) {                                                            \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: " msg, ##__VA_ARGS__);     \
        }                                                                      \
    } while (0)

/* Summary and reset */
#define TEST_SUMMARY()                                                         \
    do {                                                                       \
        printf("%s%s", BETATEST_COLOR_BOLD, BETATEST_COLOR_CYAN);              \
        printf("========================================\n");                  \
        printf("           TEST SUMMARY\n");                                   \
        printf("========================================%s\n",                 \
               BETATEST_COLOR_RESET);                                          \
        printf("Tests:      %d run, ", betatest_stats.tests_run);              \
        printf("%s%d passed%s, ", BETATEST_COLOR_GREEN,                        \
               betatest_stats.tests_passed, BETATEST_COLOR_RESET);             \
        printf("%s%d failed%s\n", BETATEST_COLOR_RED,                          \
               betatest_stats.tests_failed, BETATEST_COLOR_RESET);             \
        printf("Assertions: %d run, ", betatest_stats.assertions_run);         \
        printf("%s%d passed%s, ", BETATEST_COLOR_GREEN,                        \
               betatest_stats.assertions_passed, BETATEST_COLOR_RESET);        \
        printf("%s%d failed%s\n", BETATEST_COLOR_RED,                          \
               betatest_stats.assertions_failed, BETATEST_COLOR_RESET);        \
        printf("%s========================================%s\n",               \
               BETATEST_COLOR_CYAN, BETATEST_COLOR_RESET);                     \
        if (betatest_stats.tests_failed == 0) {                                \
            printf("%s%sALL TESTS PASSED!%s\n", BETATEST_COLOR_BOLD,           \
                   BETATEST_COLOR_GREEN, BETATEST_COLOR_RESET);                \
        } else {                                                               \
            printf("%s%sSOME TESTS FAILED%s\n", BETATEST_COLOR_BOLD,           \
                   BETATEST_COLOR_RED, BETATEST_COLOR_RESET);                  \
        }                                                                      \
        printf("\n");                                                          \
    } while (0)

#define TEST_RESET()                                                           \
    do {                                                                       \
        memset(&betatest_stats, 0, sizeof(betatest_stats));                    \
    } while (0)

/* Return success/failure code */
#define TEST_RETURN_CODE() (betatest_stats.tests_failed == 0 ? 0 : 1)

#endif /* BETATEST_H */
