#ifndef BETATEST_H
#define BETATEST_H

#include <math.h>
#include <regex.h>
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
        long long _betatest_a = (long long)(a);                                \
        long long _betatest_b = (long long)(b);                                \
        if (_betatest_a == _betatest_b) {                                      \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: integers not equal\n"      \
                                 "       1:  %s = %lld\n"                      \
                                 "       2:  %s = %lld",                       \
                                 #a, _betatest_a, #b, _betatest_b);            \
        }                                                                      \
    } while (0)

#define ASSERT_INT_NEQ(a, b)                                                   \
    do {                                                                       \
        long long _betatest_a = (long long)(a);                                \
        long long _betatest_b = (long long)(b);                                \
        if (_betatest_a != _betatest_b) {                                      \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: integers should not be equal\n"             \
                "       Both: %lld",                                           \
                _betatest_a);                                                  \
        }                                                                      \
    } while (0)

/* String comparison */
#define ASSERT_STR_EQ(s1, s2)                                                  \
    do {                                                                       \
        const char *_betatest_s1 = (s1);                                       \
        const char *_betatest_s2 = (s2);                                       \
        if (_betatest_s1 == NULL && _betatest_s2 == NULL) {                    \
            BETATEST_RECORD_PASS();                                            \
        } else if (_betatest_s1 == NULL || _betatest_s2 == NULL) {             \
            BETATEST_RECORD_FAIL("Assertion failed: one string is NULL\n"      \
                                 "       %s = %s\n"                            \
                                 "       %s = %s",                             \
                                 #s1, _betatest_s1 ? _betatest_s1 : "NULL",    \
                                 #s2, _betatest_s2 ? _betatest_s2 : "NULL");   \
        } else if (strcmp(_betatest_s1, _betatest_s2) == 0) {                  \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: strings not equal\n"       \
                                 "       1:  %s = \"%s\"\n"                    \
                                 "       2:  %s = \"%s\"\n",                   \
                                 #s1, _betatest_s1, #s2, _betatest_s2);        \
        }                                                                      \
    } while (0)

#define ASSERT_STR_NEQ(s1, s2)                                                 \
    do {                                                                       \
        const char *_betatest_s1 = (s1);                                       \
        const char *_betatest_s2 = (s2);                                       \
        if (_betatest_s1 == NULL || _betatest_s2 == NULL ||                    \
            strcmp(_betatest_s1, _betatest_s2) != 0) {                         \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: strings should not be equal\n"              \
                "       Both: \"%s\"",                                         \
                _betatest_s1);                                                 \
        }                                                                      \
    } while (0)

/* String contains substring */
#define ASSERT_STR_CONTAINS(str, substr)                                       \
    do {                                                                       \
        const char *_betatest_str = (str);                                     \
        const char *_betatest_substr = (substr);                               \
        if (_betatest_str == NULL || _betatest_substr == NULL) {               \
            BETATEST_RECORD_FAIL("Assertion failed: one string is NULL\n"      \
                                 "       %s = %s\n"                            \
                                 "       %s = %s",                             \
                                 #str, _betatest_str ? _betatest_str : "NULL", \
                                 #substr,                                      \
                                 _betatest_substr ? _betatest_substr : "NULL");\
        } else if (strstr(_betatest_str, _betatest_substr) != NULL) {          \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: string does not contain substring\n"        \
                "       String:    %s = \"%s\"\n"                              \
                "       Substring: %s = \"%s\"",                               \
                #str, _betatest_str, #substr, _betatest_substr);               \
        }                                                                      \
    } while (0)

/* String starts with prefix */
#define ASSERT_STR_STARTS_WITH(str, prefix)                                    \
    do {                                                                       \
        const char *_betatest_str = (str);                                     \
        const char *_betatest_prefix = (prefix);                               \
        if (_betatest_str == NULL || _betatest_prefix == NULL) {               \
            BETATEST_RECORD_FAIL("Assertion failed: one string is NULL\n"      \
                                 "       %s = %s\n"                            \
                                 "       %s = %s",                             \
                                 #str, _betatest_str ? _betatest_str : "NULL", \
                                 #prefix,                                      \
                                 _betatest_prefix ? _betatest_prefix : "NULL");\
        } else {                                                               \
            size_t _betatest_prefix_len = strlen(_betatest_prefix);            \
            if (strncmp(_betatest_str, _betatest_prefix,                       \
                        _betatest_prefix_len) == 0) {                          \
                BETATEST_RECORD_PASS();                                        \
            } else {                                                           \
                BETATEST_RECORD_FAIL(                                          \
                    "Assertion failed: string does not start with prefix\n"    \
                    "       String: %s = \"%s\"\n"                             \
                    "       Prefix: %s = \"%s\"",                              \
                    #str, _betatest_str, #prefix, _betatest_prefix);           \
            }                                                                  \
        }                                                                      \
    } while (0)

/* String ends with suffix */
#define ASSERT_STR_ENDS_WITH(str, suffix)                                      \
    do {                                                                       \
        const char *_betatest_str = (str);                                     \
        const char *_betatest_suffix = (suffix);                               \
        if (_betatest_str == NULL || _betatest_suffix == NULL) {               \
            BETATEST_RECORD_FAIL("Assertion failed: one string is NULL\n"      \
                                 "       %s = %s\n"                            \
                                 "       %s = %s",                             \
                                 #str, _betatest_str ? _betatest_str : "NULL", \
                                 #suffix,                                      \
                                 _betatest_suffix ? _betatest_suffix : "NULL");\
        } else {                                                               \
            size_t _betatest_str_len = strlen(_betatest_str);                  \
            size_t _betatest_suffix_len = strlen(_betatest_suffix);            \
            if (_betatest_suffix_len <= _betatest_str_len &&                   \
                strcmp(_betatest_str + _betatest_str_len -                     \
                       _betatest_suffix_len, _betatest_suffix) == 0) {         \
                BETATEST_RECORD_PASS();                                        \
            } else {                                                           \
                BETATEST_RECORD_FAIL(                                          \
                    "Assertion failed: string does not end with suffix\n"      \
                    "       String: %s = \"%s\"\n"                             \
                    "       Suffix: %s = \"%s\"",                              \
                    #str, _betatest_str, #suffix, _betatest_suffix);           \
            }                                                                  \
        }                                                                      \
    } while (0)

/* String is empty */
#define ASSERT_STR_EMPTY(str)                                                  \
    do {                                                                       \
        const char *_betatest_str = (str);                                     \
        if (_betatest_str == NULL) {                                           \
            BETATEST_RECORD_FAIL("Assertion failed: string is NULL\n"          \
                                 "       %s = NULL",                           \
                                 #str);                                        \
        } else if (_betatest_str[0] == '\0') {                                 \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: string is not empty\n"     \
                                 "       %s = \"%s\"",                         \
                                 #str, _betatest_str);                         \
        }                                                                      \
    } while (0)

/* String is not empty */
#define ASSERT_STR_NOT_EMPTY(str)                                              \
    do {                                                                       \
        const char *_betatest_str = (str);                                     \
        if (_betatest_str == NULL) {                                           \
            BETATEST_RECORD_FAIL("Assertion failed: string is NULL\n"          \
                                 "       %s = NULL",                           \
                                 #str);                                        \
        } else if (_betatest_str[0] != '\0') {                                 \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL("Assertion failed: string is empty\n"         \
                                 "       %s = \"\"",                           \
                                 #str);                                        \
        }                                                                      \
    } while (0)

/* String matches regex pattern */
#define ASSERT_STR_MATCHES(str, pattern)                                       \
    do {                                                                       \
        const char *_betatest_str = (str);                                     \
        const char *_betatest_pattern = (pattern);                             \
        if (_betatest_str == NULL || _betatest_pattern == NULL) {              \
            BETATEST_RECORD_FAIL("Assertion failed: string or pattern is NULL\n" \
                                 "       %s = %s\n"                            \
                                 "       %s = %s",                             \
                                 #str, _betatest_str ? _betatest_str : "NULL", \
                                 #pattern,                                     \
                                 _betatest_pattern ? _betatest_pattern : "NULL");\
        } else {                                                               \
            regex_t _betatest_regex;                                           \
            int _betatest_regex_err = regcomp(&_betatest_regex,                \
                                              _betatest_pattern, REG_EXTENDED);\
            if (_betatest_regex_err != 0) {                                    \
                char _betatest_errbuf[256];                                    \
                regerror(_betatest_regex_err, &_betatest_regex,                \
                         _betatest_errbuf, sizeof(_betatest_errbuf));          \
                BETATEST_RECORD_FAIL(                                          \
                    "Assertion failed: regex compilation error\n"              \
                    "       Pattern: %s = \"%s\"\n"                            \
                    "       Error:   %s",                                      \
                    #pattern, _betatest_pattern, _betatest_errbuf);            \
            } else {                                                           \
                int _betatest_match = regexec(&_betatest_regex, _betatest_str, \
                                              0, NULL, 0);                     \
                regfree(&_betatest_regex);                                     \
                if (_betatest_match == 0) {                                    \
                    BETATEST_RECORD_PASS();                                    \
                } else {                                                       \
                    BETATEST_RECORD_FAIL(                                      \
                        "Assertion failed: string does not match pattern\n"    \
                        "       String:  %s = \"%s\"\n"                        \
                        "       Pattern: %s = \"%s\"",                         \
                        #str, _betatest_str, #pattern, _betatest_pattern);     \
                }                                                              \
            }                                                                  \
        }                                                                      \
    } while (0)

/* Float/double comparison with epsilon */
#define ASSERT_FLOAT_EQ(a, b, epsilon)                                         \
    do {                                                                       \
        double _betatest_a = (double)(a);                                      \
        double _betatest_b = (double)(b);                                      \
        double _betatest_epsilon = (double)(epsilon);                          \
        if (fabs(_betatest_a - _betatest_b) <= _betatest_epsilon) {            \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            BETATEST_RECORD_FAIL(                                              \
                "Assertion failed: floats not equal within epsilon\n"          \
                "       Got:      %s = %.10g\n"                                \
                "       Expected: %s = %.10g\n"                                \
                "       Epsilon:  %.10g\n"                                     \
                "       Diff:     %.10g",                                      \
                #a, _betatest_a, #b, _betatest_b, _betatest_epsilon,           \
                fabs(_betatest_a - _betatest_b));                              \
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
