# BetaTest - A Lightweight Unit Testing Framework for C

BetaTest is a simple, macro-based unit testing framework for C that provides a clean API for writing and running tests with colorful output.

## Features

- Simple macro-based API
- Colorful terminal output
- Comprehensive assertion macros
- Detailed failure messages with file and line numbers
- Test statistics (pass/fail counts)
- Continues running all tests after failures
- Loop-friendly assertions with `ASSERT_ONCE` and `ASSERT_SINGLE`
- Array and memory comparison with side-by-side hex diff
- Test skipping with `SKIP_TEST`
- Expected failures with `RUN_TEST_XFAIL`
- Timeout protection with `ASSERT_TIMEOUT`
- Single-header library

## Quick Start

1. Set flags and include the header in your test file:

```c
#include "betatest.h"
```

1. Define your tests using the `TEST` macro:

```c
TEST(my_test_name) {
    int a = 2 + 2;
    ASSERT_INT_EQ(a, 4);
}
```

1. Run tests in your main function:

```c
int main(void) {
    RUN_TEST(my_test_name);
    TEST_SUMMARY();
    return TEST_RETURN_CODE();
}
```

1. Compile and run:

```bash
gcc -o test example_test.c -lm
./test
```

## Available Assertions

### Boolean Assertions

- `ASSERT_TRUE(condition)` - Assert that condition is true
- `ASSERT_FALSE(condition)` - Assert that condition is false

### Equality/Inequality

- `ASSERT_EQ(a, b)` - Generic equality check
- `ASSERT_NEQ(a, b)` - Generic inequality check
- `ASSERT_INT_EQ(a, b)` - Integer equality with value display
- `ASSERT_INT_NEQ(a, b)` - Integer inequality

### Comparison Operators

- `ASSERT_LT(a, b)` - Assert a < b
- `ASSERT_LE(a, b)` - Assert a <= b
- `ASSERT_GT(a, b)` - Assert a > b
- `ASSERT_GE(a, b)` - Assert a >= b

### Pointer Assertions

- `ASSERT_NULL(ptr)` - Assert pointer is NULL
- `ASSERT_NOT_NULL(ptr)` - Assert pointer is not NULL

### String Assertions

#### Basic String Comparison

- `ASSERT_STR_EQ(s1, s2)` - Assert strings are equal
- `ASSERT_STR_NEQ(s1, s2)` - Assert strings are not equal

#### Substring and Pattern Matching

- `ASSERT_STR_CONTAINS(str, substr)` - Assert string contains substring
- `ASSERT_STR_STARTS_WITH(str, prefix)` - Assert string starts with prefix
- `ASSERT_STR_ENDS_WITH(str, suffix)` - Assert string ends with suffix

#### Empty String Checks

- `ASSERT_STR_EMPTY(str)` - Assert string is empty
- `ASSERT_STR_NOT_EMPTY(str)` - Assert string is not empty

#### Regular Expression Matching

- `ASSERT_STR_MATCHES(str, pattern)` - Assert string matches regex pattern (POSIX ERE)

### Float/Double Assertions

- `ASSERT_FLOAT_EQ(a, b, epsilon)` - Assert floats are equal within epsilon

### Custom Assertions

- `ASSERT_MSG(condition, message, ...)` - Assert with custom printf-style message

### Array/Memory Assertions

- `ASSERT_ARRAY_EQ(a, b, len)` - Assert arrays are equal element-by-element
- `ASSERT_MEM_EQ(a, b, size)` - Assert memory regions are equal byte-by-byte
- `ASSERT_ARRAY_CONTAINS(arr, len, value)` - Assert array contains a value

These assertions show a side-by-side hex diff on failure:

```c
TEST(test_arrays) {
    int a[] = {1, 2, 3, 4, 5};
    int b[] = {1, 2, 9, 4, 5};
    ASSERT_ARRAY_EQ(a, b, 5);  /* Fails at index 2 */
}
```

Output:
```
[FAIL] test_arrays
       Arrays not equal at index 2
       Offset: a[2]                     | b[2]
       0x0000: 03 00 00 00              | 09 00 00 00
       at test.c:5
```

Features:
- First difference highlighted in red, other differences in yellow
- Works with primitives (shows hex of value) and structs (shows full hex dump)
- Grey `??` padding for alignment when comparing small buffers
- Configurable max bytes to display (see Configuration)

### Loop-Friendly Assertions

- `ASSERT_ONCE(assertion)` - Prints first failure only, counts all failures
- `ASSERT_SINGLE(assertion)` - Prints first failure only, counts as 1 assertion total

When testing in loops, a failing assertion can produce thousands of error messages. These wrappers suppress repeated output:

```c
TEST(test_array_values) {
    /* ASSERT_ONCE: prints once, counts all failures */
    for (int i = 0; i < 1000; ++i) {
        ASSERT_ONCE(ASSERT_INT_EQ(array[i], expected[i]));
    }
    // 1000 assertions, N failures (however many actually failed)

    /* ASSERT_SINGLE: prints once, counts as 1 assertion */
    for (int i = 0; i < 1000; ++i) {
        ASSERT_SINGLE(ASSERT_GT(array[i], 0));
    }
    // 1 assertion, 0 or 1 failure (pass if all pass, fail if any fail)
}
```

### Timeout Assertion

- `ASSERT_TIMEOUT(ms) { code }` - Fail if code block doesn't complete within ms

Useful for operations that might hang:

```c
TEST(test_network) {
    int fd = open_connection();
    char *response = NULL;

    ASSERT_TIMEOUT(5000) {
        response = fetch_data(fd);  /* Might hang */
    }

    /* Cleanup AFTER the block - runs whether timeout occurred or not */
    close_connection(fd);
    free(response);

    /* Continue testing if needed */
    if (response) {
        ASSERT_STR_CONTAINS(response, "OK");
    }
}
```

**Important:** If a timeout occurs, execution jumps out of the block immediately via `longjmp`. Any cleanup code *inside* the block will NOT run. Always place cleanup code *after* the `ASSERT_TIMEOUT` block.

## Test Skipping

Use `SKIP_TEST(reason)` to skip tests that can't run in certain conditions:

```c
TEST(test_network_feature) {
    if (!network_available()) {
        SKIP_TEST("Network not available");
    }
    /* Test code here */
}
```

Skipped tests are tracked separately and shown in the summary.

## Expected Failures

Use `RUN_TEST_XFAIL(name)` for tests that document known bugs:

```c
TEST(test_known_bug) {
    ASSERT_INT_EQ(broken_function(5), 10);  /* Known to fail */
}

int main(void) {
    RUN_TEST_XFAIL(test_known_bug);  /* Expected to fail */
    TEST_SUMMARY();
    return TEST_RETURN_CODE();
}
```

- **XFAIL**: Test failed as expected (not counted as failure)
- **XPASS**: Test unexpectedly passed (counted as failure - update your test!)

## Example

```c
#include "betatest.h"

int add(int a, int b) {
    return a + b;
}

TEST(test_addition) {
    ASSERT_INT_EQ(add(2, 3), 5);
    ASSERT_INT_EQ(add(-1, 1), 0);
}

TEST(test_comparisons) {
    int x = 10;
    ASSERT_GT(x, 5);
    ASSERT_LT(x, 20);
}

int main(void) {
    RUN_TEST(test_addition);
    RUN_TEST(test_comparisons);

    TEST_SUMMARY();
    return TEST_RETURN_CODE();
}
```

## Macros Reference

### Test Definition

- `TEST(name)` - Define a test case
- `RUN_TEST(name)` - Execute a test case
- `RUN_TEST_XFAIL(name)` - Execute a test expected to fail

### Test Control

- `SKIP_TEST(reason)` - Skip the current test with a message
- `TEST_SUMMARY()` - Print test summary with statistics
- `TEST_RETURN_CODE()` - Return 0 if all tests passed, 1 otherwise
- `TEST_RESET()` - Reset all test statistics

### Array/Memory Assertions

- `ASSERT_ARRAY_EQ(a, b, len)` - Compare arrays element-by-element
- `ASSERT_MEM_EQ(a, b, size)` - Compare memory byte-by-byte
- `ASSERT_ARRAY_CONTAINS(arr, len, value)` - Check array contains value

### Assertion Wrappers

- `ASSERT_ONCE(assertion)` - Prints once, counts all failures
- `ASSERT_SINGLE(assertion)` - Prints once, counts as 1 pass/fail total
- `ASSERT_TIMEOUT(ms) { code }` - Fails if code doesn't complete in time

### Configuration

You can define certain FLAGS to change behaviour before including the header file.

#### Output Control

- `BETATEST_NO_COLOR` - Disable colored output
- `BETATEST_PRINT_ON_TEST` - Print the test name when starting (useful if test hangs)
- `BETATEST_PRINT_ON_PASS` - Print the test name on pass
- `BETATEST_PRINT_NOT_ON_FAIL` - Suppress output on failure

#### Hex Dump Settings

- `BETATEST_HEX_MAX_BYTES` - Max bytes to show in hex diff (default: 64, 0 = unlimited)
- `BETATEST_HEX_BYTES_PER_LINE` - Bytes per line in hex output (default: 16)

```c
// #define BETATEST_NO_COLOR
#define BETATEST_PRINT_ON_TEST
#define BETATEST_PRINT_ON_PASS
// #define BETATEST_PRINT_NOT_ON_FAIL
#define BETATEST_HEX_MAX_BYTES 128  /* Show more context in hex diffs */
#include "betatest.h"
```

## Output Example

```
[TEST] test_addition
[PASS] test_addition

[TEST] test_with_failure
[FAIL] test_with_failure
       Assertion failed: integers not equal
       1:  2 + 2 = 4
       2:  result = 5
       at example_test.c:42

[TEST] test_network
[SKIP] test_network: Network not available

[TEST] test_known_bug
[FAIL] test_known_bug
       Assertion failed: integers not equal
       ...
[XFAIL] test_known_bug (expected)

[TEST] test_timeout
[FAIL] test_timeout
       Timeout: code did not complete within 100 ms
       at example_test.c:50

========================================
           TEST SUMMARY
========================================
Tests:      5 run, 1 passed, 2 failed, 1 skipped, 1 xfail
Assertions: 5 run, 2 passed, 3 failed
========================================
SOME TESTS FAILED
```

## Design Philosophy

- **Simple**: Just include one header file
- **Transparent**: Clear error messages with exact locations
- **Comprehensive**: Covers common testing needs
- **Non-intrusive**: Pure C macros, no magic
- **Informative**: Detailed statistics and failure reporting

## Inspirations

BetaTest's API and design follows common conventions found in established C testing frameworks:

- **Unity** - Influenced the assertion naming conventions (`ASSERT_EQ`, `ASSERT_TRUE`, etc.) and single-header approach
- **Google Test (C++)** - Inspired the assertion API design and test result reporting
- **MinUnit** - The minimal macro-based approach and `TEST()` pattern
- **Check** - The test statistics tracking and continue-on-failure behavior
