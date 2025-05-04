# BetaTest - A Lightweight Unit Testing Framework for C

BetaTest is a simple, macro-based unit testing framework for C that provides a clean API for writing and running tests with colorful output.

## Features

- Simple macro-based API
- Colorful terminal output
- Comprehensive assertion macros
- Detailed failure messages with file and line numbers
- Test statistics (pass/fail counts)
- Continues running all tests after failures
- Single-header library

## Quick Start

1. Include the header in your test file:

```c
#include "betatest.h"
```

1. Define your tests using the `TEST` macro:

```c
TEST(my_test_name) {
    ASSERT_INT_EQ(2 + 2, 4);
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

- `ASSERT_STR_EQ(s1, s2)` - Assert strings are equal
- `ASSERT_STR_NEQ(s1, s2)` - Assert strings are not equal

### Float/Double Assertions

- `ASSERT_FLOAT_EQ(a, b, epsilon)` - Assert floats are equal within epsilon

### Custom Assertions

- `ASSERT_MSG(condition, message, ...)` - Assert with custom printf-style message

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

    BETATEST_SUMMARY();
    return BETATEST_RETURN_CODE();
}
```

## Macros Reference

### Test Definition

- `TEST(name)` - Define a test case
- `RUN_TEST(name)` - Execute a test case

### Test Control

- `BETATEST_SUMMARY()` - Print test summary with statistics
- `BETATEST_RETURN_CODE()` - Return 0 if all tests passed, 1 otherwise
- `BETATEST_RESET()` - Reset all test statistics

### Configuration

- Define `BETATEST_NO_COLOR` before including the header to disable colored output:

```c
#define BETATEST_NO_COLOR
#include "betatest.h"
```

## Output Example

```
[TEST] test_addition
[PASS] Test 'test_addition' passed

[TEST] test_with_failure
[FAIL] Assertion failed: integers not equal
       Expected: 2 + 2 = 4
       Got:      result = 5
       at example_test.c:42

========================================
           TEST SUMMARY
========================================
Tests:      2 run, 1 passed, 1 failed
Assertions: 3 run, 2 passed, 1 failed
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
