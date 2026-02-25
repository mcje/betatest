#ifndef BETATEST_H
#define BETATEST_H

#include <math.h>
#include <regex.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/* Configuration */
#ifndef BETATEST_NO_COLOR
#define BETATEST_USE_COLOR 1
#else
#define BETATEST_USE_COLOR 0
#endif

#ifndef BETATEST_HEX_MAX_BYTES
#define BETATEST_HEX_MAX_BYTES 64 /* 0 = print all */
#endif

#ifndef BETATEST_HEX_BYTES_PER_LINE
#define BETATEST_HEX_BYTES_PER_LINE 16
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

#ifndef BETATEST_TRACK_CRASHES
#define BETATEST_DO_TRACK_CRASHES 0
#else
#define BETATEST_DO_TRACK_CRASHES 1
#endif

#ifndef BETATEST_MAX_ONCE_LOCATIONS
#define BETATEST_MAX_ONCE_LOCATIONS 256
#endif

/* Token concatenation helpers */
#define BETATEST_CAT_(a, b) a##b
#define BETATEST_CAT(a, b) BETATEST_CAT_(a, b)

/* Color codes */
#if BETATEST_USE_COLOR
#define BETATEST_COLOR_GREEN "\033[32m"
#define BETATEST_COLOR_RED "\033[31m"
#define BETATEST_COLOR_YELLOW "\033[33m"
#define BETATEST_COLOR_CYAN "\033[36m"
#define BETATEST_COLOR_MAGENTA "\033[35m"
#define BETATEST_COLOR_DIM "\033[90m"
#define BETATEST_COLOR_RESET "\033[0m"
#define BETATEST_COLOR_BOLD "\033[1m"
#define BETATEST_HEX_PADDING "??"
#else
#define BETATEST_COLOR_GREEN ""
#define BETATEST_COLOR_RED ""
#define BETATEST_COLOR_YELLOW ""
#define BETATEST_COLOR_CYAN ""
#define BETATEST_COLOR_MAGENTA ""
#define BETATEST_COLOR_DIM ""
#define BETATEST_HEX_PADDING "??"
#define BETATEST_COLOR_RESET ""
#define BETATEST_COLOR_BOLD ""
#endif

/* Test statistics */
static struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
    int tests_skipped;
    int tests_xfail;
    int tests_xpass;
    int assertions_run;
    int assertions_passed;
    int assertions_failed;
    int assertions_crashed;
    int current_test_failed;
    int current_test_skipped;
    char *current_test_name;
    int suppress_failure_output;
    int suppress_recording;
    int last_assertion_failed;
    int expecting_failure;
    int print_once_mode;
} betatest_stats = {0};

/* Print-once tracking for ASSERT_PRINT_ONCE
 * WARNING: Do not nest ASSERT_PRINT_ONCE blocks - behavior is undefined */
static struct {
    const char *file;
    int line;
} betatest_once_locations[BETATEST_MAX_ONCE_LOCATIONS];
static int betatest_once_count = 0;

static inline int betatest_once_should_print(const char *file, int line) {
    if (!betatest_stats.print_once_mode) return 1;

    for (int i = 0; i < betatest_once_count; i++) {
        if (betatest_once_locations[i].line == line &&
            betatest_once_locations[i].file == file) {
            return 0;
        }
    }

    if (betatest_once_count < BETATEST_MAX_ONCE_LOCATIONS) {
        betatest_once_locations[betatest_once_count].file = file;
        betatest_once_locations[betatest_once_count].line = line;
        betatest_once_count++;
    }
    return 1;
}

/* Timeout support */
static jmp_buf betatest_timeout_jmp;
static volatile sig_atomic_t betatest_timed_out = 0;

static void betatest_timeout_handler(int sig) {
    (void)sig;
    betatest_timed_out = 1;
    longjmp(betatest_timeout_jmp, 1);
}

static inline void betatest_timeout_start(int ms) {
    betatest_timed_out = 0;
    signal(SIGALRM, betatest_timeout_handler);
    struct itimerval timer = {0};
    timer.it_value.tv_sec = ms / 1000;
    timer.it_value.tv_usec = (ms % 1000) * 1000;
    setitimer(ITIMER_REAL, &timer, NULL);
}

static inline void betatest_timeout_stop(void) {
    struct itimerval timer = {0};
    setitimer(ITIMER_REAL, &timer, NULL);
    signal(SIGALRM, SIG_DFL);
}

/* Crash/signal handling support */
static jmp_buf betatest_crash_jmp;
static volatile sig_atomic_t betatest_crash_sig = 0;

static const char *betatest_signal_name(int sig) {
    switch (sig) {
        case SIGSEGV: return "SIGSEGV (segmentation fault)";
        case SIGBUS:  return "SIGBUS (bus error)";
        case SIGFPE:  return "SIGFPE (floating point exception)";
        case SIGABRT: return "SIGABRT (abort)";
        case SIGILL:  return "SIGILL (illegal instruction)";
        default:      return "unknown signal";
    }
}

static void betatest_crash_handler(int sig) {
    betatest_crash_sig = sig;
    longjmp(betatest_crash_jmp, 1);
}

static inline void betatest_crash_protect_start(void) {
    betatest_crash_sig = 0;
    signal(SIGSEGV, betatest_crash_handler);
    signal(SIGBUS, betatest_crash_handler);
    signal(SIGFPE, betatest_crash_handler);
    signal(SIGABRT, betatest_crash_handler);
    signal(SIGILL, betatest_crash_handler);
}

static inline void betatest_crash_protect_stop(void) {
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGILL, SIG_DFL);
}

/* Print helpers */
#define BETATEST_PRINT_PASS()                                                  \
    printf("%s[PASS]%s ", BETATEST_COLOR_GREEN, BETATEST_COLOR_RESET)

#define BETATEST_PRINT_FAIL()                                                  \
    printf("%s[FAIL]%s ", BETATEST_COLOR_RED, BETATEST_COLOR_RESET)

#define BETATEST_PRINT_INFO()                                                  \
    printf("%s[INFO]%s ", BETATEST_COLOR_CYAN, BETATEST_COLOR_RESET)

#define BETATEST_PRINT_SKIP()                                                  \
    printf("%s[SKIP]%s ", BETATEST_COLOR_YELLOW, BETATEST_COLOR_RESET)

#define BETATEST_PRINT_XFAIL()                                                 \
    printf("%s[XFAIL]%s ", BETATEST_COLOR_DIM, BETATEST_COLOR_RESET)

#define BETATEST_PRINT_XPASS()                                                 \
    printf("%s%s[XPASS]%s ", BETATEST_COLOR_BOLD, BETATEST_COLOR_MAGENTA,      \
           BETATEST_COLOR_RESET)

/* Helper functions to print values by pointer (uniform signature for _Generic) */
static inline void betatest_print_hex(const void *p, size_t size) {
    const unsigned char *bytes = (const unsigned char *)p;
    if (size <= 8) {
        /* Small values: inline format */
        printf("0x");
        for (size_t i = 0; i < size; ++i) {
            printf("%02x", bytes[i]);
        }
    } else {
        /* Larger values: multi-line format */
        printf("\n");
        for (size_t i = 0; i < size; i += BETATEST_HEX_BYTES_PER_LINE) {
            printf("              0x%04zx: ", i);
            for (size_t j = 0; j < BETATEST_HEX_BYTES_PER_LINE && i + j < size; ++j) {
                printf("%02x ", bytes[i + j]);
            }
            printf("\n");
        }
        printf("             ");
    }
}

/* Helper to print side-by-side memory diff centered on diff_offset */
static inline void betatest_print_hex_diff(const void *pa, const void *pb,
                                           size_t size, size_t diff_offset,
                                           const char *name_a, const char *name_b) {
    const unsigned char *a = (const unsigned char *)pa;
    const unsigned char *b = (const unsigned char *)pb;
    size_t start = 0;
    size_t end = size;
    int truncated_start = 0;
    int truncated_end = 0;

    /* Use half the bytes per line for side-by-side display */
    size_t bpl = BETATEST_HEX_BYTES_PER_LINE / 2;
    if (bpl < 4) bpl = 4;

    /* Calculate column width: each byte = "XX " (3 chars) */
    size_t col_width = bpl * 3;

    if (BETATEST_HEX_MAX_BYTES > 0 && size > (size_t)BETATEST_HEX_MAX_BYTES) {
        size_t half = BETATEST_HEX_MAX_BYTES / 2;
        if (diff_offset > half) {
            start = diff_offset - half;
            truncated_start = 1;
        }
        end = start + BETATEST_HEX_MAX_BYTES;
        if (end > size) {
            end = size;
            if (start > 0 && end - start < (size_t)BETATEST_HEX_MAX_BYTES) {
                start = (end > (size_t)BETATEST_HEX_MAX_BYTES) ?
                        end - BETATEST_HEX_MAX_BYTES : 0;
            }
        }
        if (end < size) {
            truncated_end = 1;
        }
    }

    /* Align start to line boundary */
    start = (start / bpl) * bpl;

    /* Print header with proper alignment */
    printf("       Offset: %-*s | %s\n", (int)col_width, name_a, name_b);

    if (truncated_start) {
        printf("       ... (showing bytes %zu-%zu of %zu)\n", start, end - 1, size);
    }

    for (size_t i = start; i < end; i += bpl) {
        printf("       0x%04zx: ", i);

        /* Left side (a) */
        for (size_t j = 0; j < bpl && i + j < end; ++j) {
            if (i + j == diff_offset) {
                printf("%s%02x%s ", BETATEST_COLOR_RED, a[i + j],
                       BETATEST_COLOR_RESET);
            } else if (a[i + j] != b[i + j]) {
                printf("%s%02x%s ", BETATEST_COLOR_YELLOW, a[i + j],
                       BETATEST_COLOR_RESET);
            } else {
                printf("%02x ", a[i + j]);
            }
        }
        for (size_t j = end - i; j < bpl; ++j) {
            printf("%s%s%s ", BETATEST_COLOR_DIM, BETATEST_HEX_PADDING,
                   BETATEST_COLOR_RESET);
        }

        printf(" | ");

        /* Right side (b) */
        for (size_t j = 0; j < bpl && i + j < end; ++j) {
            if (i + j == diff_offset) {
                printf("%s%02x%s ", BETATEST_COLOR_RED, b[i + j],
                       BETATEST_COLOR_RESET);
            } else if (a[i + j] != b[i + j]) {
                printf("%s%02x%s ", BETATEST_COLOR_YELLOW, b[i + j],
                       BETATEST_COLOR_RESET);
            } else {
                printf("%02x ", b[i + j]);
            }
        }
        for (size_t j = end - i; j < bpl; ++j) {
            printf("%s%s%s ", BETATEST_COLOR_DIM, BETATEST_HEX_PADDING,
                   BETATEST_COLOR_RESET);
        }
        printf("\n");
    }

    if (truncated_end) {
        printf("       ...\n");
    }
}
static inline void betatest_print_char(const void *p, size_t s) {
    (void)s; printf("'%c'", *(const char *)p);
}
static inline void betatest_print_schar(const void *p, size_t s) {
    (void)s; printf("%hhd", *(const signed char *)p);
}
static inline void betatest_print_uchar(const void *p, size_t s) {
    (void)s; printf("%hhu", *(const unsigned char *)p);
}
static inline void betatest_print_short(const void *p, size_t s) {
    (void)s; printf("%hd", *(const short *)p);
}
static inline void betatest_print_ushort(const void *p, size_t s) {
    (void)s; printf("%hu", *(const unsigned short *)p);
}
static inline void betatest_print_int(const void *p, size_t s) {
    (void)s; printf("%d", *(const int *)p);
}
static inline void betatest_print_uint(const void *p, size_t s) {
    (void)s; printf("%u", *(const unsigned int *)p);
}
static inline void betatest_print_long(const void *p, size_t s) {
    (void)s; printf("%ld", *(const long *)p);
}
static inline void betatest_print_ulong(const void *p, size_t s) {
    (void)s; printf("%lu", *(const unsigned long *)p);
}
static inline void betatest_print_llong(const void *p, size_t s) {
    (void)s; printf("%lld", *(const long long *)p);
}
static inline void betatest_print_ullong(const void *p, size_t s) {
    (void)s; printf("%llu", *(const unsigned long long *)p);
}
static inline void betatest_print_float(const void *p, size_t s) {
    (void)s; printf("%g", (double)*(const float *)p);
}
static inline void betatest_print_double(const void *p, size_t s) {
    (void)s; printf("%g", *(const double *)p);
}
static inline void betatest_print_str(const void *p, size_t s) {
    (void)s; printf("\"%s\"", *(const char *const *)p);
}

/* Helper macro to print a value with appropriate format (C11 _Generic) */
#define BETATEST_PRINT_VAL(x) (_Generic((x),                                   \
    char: betatest_print_char,                                                 \
    signed char: betatest_print_schar,                                         \
    unsigned char: betatest_print_uchar,                                       \
    short: betatest_print_short,                                               \
    unsigned short: betatest_print_ushort,                                     \
    int: betatest_print_int,                                                   \
    unsigned int: betatest_print_uint,                                         \
    long: betatest_print_long,                                                 \
    unsigned long: betatest_print_ulong,                                       \
    long long: betatest_print_llong,                                           \
    unsigned long long: betatest_print_ullong,                                 \
    float: betatest_print_float,                                               \
    double: betatest_print_double,                                             \
    char *: betatest_print_str,                                                \
    const char *: betatest_print_str,                                          \
    default: betatest_print_hex))(&(x), sizeof(x))

/* Helper macro to check if type is a known primitive (1) or unknown/struct (0) */
#define BETATEST_IS_PRIMITIVE(x) _Generic((x),                                 \
    char: 1, signed char: 1, unsigned char: 1,                                 \
    short: 1, unsigned short: 1,                                               \
    int: 1, unsigned int: 1,                                                   \
    long: 1, unsigned long: 1,                                                 \
    long long: 1, unsigned long long: 1,                                       \
    float: 1, double: 1,                                                       \
    char *: 1, const char *: 1,                                                \
    default: 0)

/* Test definition macros */
#define TEST(name)                                                             \
    static void test_##name(void);                                             \
    static void run_test_##name(void) {                                        \
        betatest_stats.current_test_name = #name;                              \
        betatest_stats.tests_run++;                                            \
        betatest_stats.current_test_failed = 0;                                \
        betatest_stats.current_test_skipped = 0;                               \
        int _betatest_xfail = betatest_stats.expecting_failure;                \
        betatest_stats.expecting_failure = 0;                                  \
        int print_nl = 0;                                                      \
        if (BETATEST_DO_PRINT_TEST) {                                          \
            printf("%s%s[TEST]%s %s%s\n", BETATEST_COLOR_BOLD,                 \
                   BETATEST_COLOR_CYAN, BETATEST_COLOR_RESET, #name,           \
                   BETATEST_COLOR_RESET);                                      \
            print_nl = 1;                                                      \
        }                                                                      \
        test_##name();                                                         \
        if (betatest_stats.current_test_skipped) {                             \
            betatest_stats.tests_skipped++;                                    \
        } else if (_betatest_xfail) {                                          \
            if (betatest_stats.current_test_failed) {                          \
                betatest_stats.tests_xfail++;                                  \
                BETATEST_PRINT_XFAIL();                                        \
                printf("%s (expected)\n", #name);                              \
                print_nl = 1;                                                  \
            } else {                                                           \
                betatest_stats.tests_xpass++;                                  \
                BETATEST_PRINT_XPASS();                                        \
                printf("%s (unexpected pass!)\n", #name);                      \
                print_nl = 1;                                                  \
            }                                                                  \
        } else if (betatest_stats.current_test_failed) {                       \
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

/* Run a test that is expected to fail */
#define RUN_TEST_XFAIL(name)                                                   \
    do {                                                                       \
        betatest_stats.expecting_failure = 1;                                  \
        run_test_##name();                                                     \
    } while (0)

/* Skip current test with a reason */
#define SKIP_TEST(reason)                                                      \
    do {                                                                       \
        betatest_stats.current_test_skipped = 1;                               \
        BETATEST_PRINT_SKIP();                                                 \
        printf("%s: %s\n", betatest_stats.current_test_name, reason);          \
        return;                                                                \
    } while (0)

/* Assertion helpers */
#define BETATEST_RECORD_PASS()                                                 \
    do {                                                                       \
        betatest_stats.last_assertion_failed = 0;                              \
        if (!betatest_stats.suppress_recording) {                              \
            betatest_stats.assertions_run++;                                   \
            betatest_stats.assertions_passed++;                                \
        }                                                                      \
    } while (0)

#define BETATEST_RECORD_FAIL(msg, ...)                                         \
    do {                                                                       \
        betatest_stats.last_assertion_failed = 1;                              \
        if (!betatest_stats.suppress_recording) {                              \
            betatest_stats.assertions_run++;                                   \
            betatest_stats.assertions_failed++;                                \
            betatest_stats.current_test_failed = 1;                            \
        }                                                                      \
        if (BETATEST_DO_PRINT_FAIL && !betatest_stats.suppress_failure_output &&\
            betatest_once_should_print(__FILE__, __LINE__)) {                  \
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

/* Timeout assertion - fails if code block doesn't complete within ms */
#define ASSERT_TIMEOUT(ms)                                                     \
    if (setjmp(betatest_timeout_jmp) != 0) {                                   \
        BETATEST_RECORD_FAIL("Timeout: code did not complete within %d ms",    \
                             ms);                                              \
    } else                                                                     \
        for (int _betatest_timeout_done = (betatest_timeout_start(ms), 0);     \
             !_betatest_timeout_done;                                          \
             _betatest_timeout_done = (betatest_timeout_stop(),                \
                                       betatest_timeout_record_pass(), 1))

static inline int betatest_timeout_record_pass(void) {
    betatest_stats.last_assertion_failed = 0;
    if (!betatest_stats.suppress_recording) {
        betatest_stats.assertions_run++;
        betatest_stats.assertions_passed++;
    }
    return 0;
}

/* Crash protection assertion - fails if code block crashes */
#define ASSERT_NO_CRASH                                                        \
    if (setjmp(betatest_crash_jmp) != 0) {                                     \
        betatest_crash_protect_stop();                                         \
        betatest_crash_record_fail(betatest_crash_sig, __FILE__, __LINE__);    \
    } else                                                                     \
        for (int _betatest_crash_done = (betatest_crash_protect_start(), 0);   \
             !_betatest_crash_done;                                            \
             _betatest_crash_done = (betatest_crash_protect_stop(),            \
                                     betatest_crash_record_pass(), 1))

static inline void betatest_crash_record_fail(int sig, const char *file,
                                              int line) {
    betatest_stats.last_assertion_failed = 1;
    if (!betatest_stats.suppress_recording) {
        betatest_stats.assertions_run++;
        betatest_stats.assertions_failed++;
        betatest_stats.assertions_crashed++;
        betatest_stats.current_test_failed = 1;
    }
    if (BETATEST_DO_PRINT_FAIL && !betatest_stats.suppress_failure_output) {
        BETATEST_PRINT_FAIL();
        printf("%s\n", betatest_stats.current_test_name);
        printf("       Crash: caught %s\n", betatest_signal_name(sig));
        printf("       at %s:%d\n", file, line);
    }
}

static inline int betatest_crash_record_pass(void) {
    betatest_stats.last_assertion_failed = 0;
    if (!betatest_stats.suppress_recording) {
        betatest_stats.assertions_run++;
        betatest_stats.assertions_passed++;
    }
    return 0;
}

/* Array equality - compares element by element */
#define ASSERT_ARRAY_EQ(a, b, len)                                             \
    do {                                                                       \
        size_t _betatest_len = (len);                                          \
        int _betatest_arr_eq = 1;                                              \
        size_t _betatest_fail_idx = 0;                                         \
        for (size_t _i = 0; _i < _betatest_len; ++_i) {                        \
            if (memcmp(&(a)[_i], &(b)[_i], sizeof((a)[0])) != 0) {             \
                _betatest_arr_eq = 0;                                          \
                _betatest_fail_idx = _i;                                       \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        if (_betatest_arr_eq) {                                                \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            betatest_stats.last_assertion_failed = 1;                          \
            if (!betatest_stats.suppress_recording) {                          \
                betatest_stats.assertions_run++;                               \
                betatest_stats.assertions_failed++;                            \
                betatest_stats.current_test_failed = 1;                        \
            }                                                                  \
            if (BETATEST_DO_PRINT_FAIL &&                                      \
                !betatest_stats.suppress_failure_output) {                     \
                BETATEST_PRINT_FAIL();                                         \
                printf("%s\n", betatest_stats.current_test_name);              \
                printf("       Arrays not equal at index %zu\n",               \
                       _betatest_fail_idx);                                    \
                if (BETATEST_IS_PRIMITIVE((a)[0])) {                           \
                    printf("       %s[%zu] = ", #a, _betatest_fail_idx);       \
                    BETATEST_PRINT_VAL((a)[_betatest_fail_idx]);               \
                    printf("\n       %s[%zu] = ", #b, _betatest_fail_idx);     \
                    BETATEST_PRINT_VAL((b)[_betatest_fail_idx]);               \
                    printf("\n");                                              \
                } else {                                                       \
                    char _betatest_name_a[64], _betatest_name_b[64];           \
                    snprintf(_betatest_name_a, sizeof(_betatest_name_a),       \
                             "%s[%zu]", #a, _betatest_fail_idx);               \
                    snprintf(_betatest_name_b, sizeof(_betatest_name_b),       \
                             "%s[%zu]", #b, _betatest_fail_idx);               \
                    betatest_print_hex_diff(&(a)[_betatest_fail_idx],          \
                                            &(b)[_betatest_fail_idx],          \
                                            sizeof((a)[0]), 0,                 \
                                            _betatest_name_a, _betatest_name_b);\
                }                                                              \
                printf("       at %s:%d\n", __FILE__, __LINE__);               \
            }                                                                  \
        }                                                                      \
    } while (0)

/* Memory equality - compares raw bytes */
#define ASSERT_MEM_EQ(a, b, size)                                              \
    do {                                                                       \
        size_t _betatest_size = (size);                                        \
        const unsigned char *_betatest_pa = (const unsigned char *)(a);        \
        const unsigned char *_betatest_pb = (const unsigned char *)(b);        \
        int _betatest_mem_eq = 1;                                              \
        size_t _betatest_diff_idx = 0;                                         \
        for (size_t _i = 0; _i < _betatest_size; ++_i) {                       \
            if (_betatest_pa[_i] != _betatest_pb[_i]) {                        \
                _betatest_mem_eq = 0;                                          \
                _betatest_diff_idx = _i;                                       \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        if (_betatest_mem_eq) {                                                \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            betatest_stats.last_assertion_failed = 1;                          \
            if (!betatest_stats.suppress_recording) {                          \
                betatest_stats.assertions_run++;                               \
                betatest_stats.assertions_failed++;                            \
                betatest_stats.current_test_failed = 1;                        \
            }                                                                  \
            if (BETATEST_DO_PRINT_FAIL &&                                      \
                !betatest_stats.suppress_failure_output) {                     \
                BETATEST_PRINT_FAIL();                                         \
                printf("%s\n", betatest_stats.current_test_name);              \
                printf("       Memory not equal (first diff at byte %zu)\n",   \
                       _betatest_diff_idx);                                    \
                betatest_print_hex_diff(_betatest_pa, _betatest_pb,            \
                                        _betatest_size, _betatest_diff_idx,    \
                                        #a, #b);                               \
                printf("       at %s:%d\n", __FILE__, __LINE__);               \
            }                                                                  \
        }                                                                      \
    } while (0)

/* Array contains value */
#define ASSERT_ARRAY_CONTAINS(arr, len, val)                                   \
    do {                                                                       \
        size_t _betatest_len = (len);                                          \
        typeof(val) _betatest_val = (val);                                     \
        int _betatest_found = 0;                                               \
        for (size_t _i = 0; _i < _betatest_len; ++_i) {                        \
            if (memcmp(&(arr)[_i], &_betatest_val, sizeof(_betatest_val)) == 0) { \
                _betatest_found = 1;                                           \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        if (_betatest_found) {                                                 \
            BETATEST_RECORD_PASS();                                            \
        } else {                                                               \
            betatest_stats.last_assertion_failed = 1;                          \
            if (!betatest_stats.suppress_recording) {                          \
                betatest_stats.assertions_run++;                               \
                betatest_stats.assertions_failed++;                            \
                betatest_stats.current_test_failed = 1;                        \
            }                                                                  \
            if (BETATEST_DO_PRINT_FAIL &&                                      \
                !betatest_stats.suppress_failure_output) {                     \
                BETATEST_PRINT_FAIL();                                         \
                printf("%s\n", betatest_stats.current_test_name);              \
                printf("       Array %s (len %zu) does not contain: ", #arr,   \
                       _betatest_len);                                         \
                BETATEST_PRINT_VAL(_betatest_val);                             \
                printf("\n       at %s:%d\n", __FILE__, __LINE__);             \
            }                                                                  \
        }                                                                      \
    } while (0)

/* Loop-friendly assertion wrappers (block-based)
 * WARNING: Do not nest these blocks - behavior is undefined */

/* ASSERT_PRINT_ONCE: Each assertion prints at most once (still counts all) */
#define ASSERT_PRINT_ONCE                                                      \
    for (int _betatest_po = (betatest_stats.print_once_mode = 1, 1);           \
         _betatest_po;                                                         \
         _betatest_po = (betatest_stats.print_once_mode = 0, 0))

/* ASSERT_COUNT_ONCE: All assertions count as 1 total (prints normally) */
#define ASSERT_COUNT_ONCE                                                      \
    static int BETATEST_CAT(_betatest_co_done_, __LINE__) = 0;                 \
    static int BETATEST_CAT(_betatest_co_failed_, __LINE__) = 0;               \
    for (int _betatest_co = (betatest_stats.suppress_recording = 1, 1);        \
         _betatest_co;                                                         \
         _betatest_co = (betatest_count_once_cleanup(                          \
             &BETATEST_CAT(_betatest_co_done_, __LINE__),                       \
             &BETATEST_CAT(_betatest_co_failed_, __LINE__)), 0))

static inline int betatest_count_once_cleanup(int *done, int *failed) {
    betatest_stats.suppress_recording = 0;
    if (!*done) {
        *done = 1;
        betatest_stats.assertions_run++;
        if (betatest_stats.last_assertion_failed) {
            *failed = 1;
            betatest_stats.assertions_failed++;
            betatest_stats.current_test_failed = 1;
        } else {
            betatest_stats.assertions_passed++;
        }
    } else if (!*failed && betatest_stats.last_assertion_failed) {
        *failed = 1;
        betatest_stats.assertions_passed--;
        betatest_stats.assertions_failed++;
        betatest_stats.current_test_failed = 1;
    }
    return 0;
}

/* ASSERT_ONCE: Shorthand for ASSERT_PRINT_ONCE + ASSERT_COUNT_ONCE */
#define ASSERT_ONCE                                                            \
    static int BETATEST_CAT(_betatest_once_done_, __LINE__) = 0;               \
    static int BETATEST_CAT(_betatest_once_failed_, __LINE__) = 0;             \
    for (int _betatest_once = (betatest_stats.print_once_mode = 1,             \
                               betatest_stats.suppress_recording = 1, 1);      \
         _betatest_once;                                                       \
         _betatest_once = (betatest_stats.print_once_mode = 0,                 \
                           betatest_count_once_cleanup(                        \
                               &BETATEST_CAT(_betatest_once_done_, __LINE__),  \
                               &BETATEST_CAT(_betatest_once_failed_, __LINE__)), 0))

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
        printf("%s%d failed%s", BETATEST_COLOR_RED,                            \
               betatest_stats.tests_failed, BETATEST_COLOR_RESET);             \
        if (betatest_stats.tests_skipped > 0) {                                \
            printf(", %s%d skipped%s", BETATEST_COLOR_YELLOW,                  \
                   betatest_stats.tests_skipped, BETATEST_COLOR_RESET);        \
        }                                                                      \
        if (betatest_stats.tests_xfail > 0) {                                  \
            printf(", %s%d xfail%s", BETATEST_COLOR_DIM,                       \
                   betatest_stats.tests_xfail, BETATEST_COLOR_RESET);          \
        }                                                                      \
        if (betatest_stats.tests_xpass > 0) {                                  \
            printf(", %s%d xpass%s", BETATEST_COLOR_MAGENTA,                   \
                   betatest_stats.tests_xpass, BETATEST_COLOR_RESET);          \
        }                                                                      \
        printf("\n");                                                          \
        printf("Assertions: %d run, ", betatest_stats.assertions_run);         \
        printf("%s%d passed%s, ", BETATEST_COLOR_GREEN,                        \
               betatest_stats.assertions_passed, BETATEST_COLOR_RESET);        \
        printf("%s%d failed%s", BETATEST_COLOR_RED,                            \
               betatest_stats.assertions_failed, BETATEST_COLOR_RESET);        \
        if (BETATEST_DO_TRACK_CRASHES && betatest_stats.assertions_crashed > 0) { \
            printf(" (%s%d crashed%s)", BETATEST_COLOR_BOLD,                   \
                   betatest_stats.assertions_crashed, BETATEST_COLOR_RESET);   \
        }                                                                      \
        printf("\n");                                                          \
        printf("%s========================================%s\n",               \
               BETATEST_COLOR_CYAN, BETATEST_COLOR_RESET);                     \
        if (betatest_stats.tests_failed == 0 &&                                \
            betatest_stats.tests_xpass == 0) {                                 \
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

/* Return success/failure code (xpass counts as failure - test should be updated) */
#define TEST_RETURN_CODE()                                                     \
    ((betatest_stats.tests_failed == 0 && betatest_stats.tests_xpass == 0) ? 0 : 1)

#endif /* BETATEST_H */
