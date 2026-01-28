#define BETATEST_PRINT_ON_TEST
#define BETATEST_PRINT_ON_PASS
// #define BETATEST_PRINT_NOT_ON_FAIL
// #define BETATEST_NO_COLOR
#include "betatest.h"

/* Example function to test */
int add(int a, int b) { return a + b; }

int multiply(int a, int b) { return a * b; }

char *greet(const char *name) {
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "Hello, %s!", name);
    return buffer;
}

/* Test cases */
TEST(test_addition) {
    ASSERT_INT_EQ(add(2, 3), 6);
    ASSERT_INT_EQ(add(-1, 1), 0);
    ASSERT_INT_EQ(add(0, 0), 0);
}

TEST(test_multiplication) {
    ASSERT_INT_EQ(multiply(2, 3), 6);
    ASSERT_INT_EQ(multiply(-2, 3), -6);
    ASSERT_INT_EQ(multiply(0, 100), 0);
}

TEST(test_boolean_assertions) {
    ASSERT_TRUE(1 == 1);
    ASSERT_FALSE(1 == 2);
    ASSERT_TRUE(add(2, 2) == 4);
}

TEST(test_comparisons) {
    int x = 10;
    ASSERT_GT(x, 5);
    ASSERT_GE(x, 10);
    ASSERT_LT(x, 20);
    ASSERT_LE(x, 10);
}

TEST(test_string_operations) {
    const char *result = greet("World");
    ASSERT_STR_EQ(result, "Hello, World!");
    ASSERT_STR_NEQ(result, "Goodbye");
}

TEST(test_null_pointers) {
    char *ptr = NULL;
    ASSERT_NULL(ptr);

    ptr = "not null";
    ASSERT_NOT_NULL(ptr);
}

TEST(test_float_equality) {
    double pi = 3.14159265359;
    ASSERT_FLOAT_EQ(pi, 3.14159, 0.001);
    ASSERT_FLOAT_EQ(0.1 + 0.2, 0.3, 0.0001);
}

TEST(test_with_intentional_failure) {
    ASSERT_INT_EQ(2 + 2, 4); /* This passes */
    ASSERT_INT_EQ(2 + 2, 5); /* This fails */
    ASSERT_FLOAT_EQ(0.1 + 0.2, 0.3, 0);
    ASSERT_TRUE(0); /* This also fails */
}

int main(void) {
    printf("Running BetaTest Example Tests\n\n");

    RUN_TEST(test_addition);
    RUN_TEST(test_multiplication);
    RUN_TEST(test_boolean_assertions);
    RUN_TEST(test_comparisons);
    RUN_TEST(test_string_operations);
    RUN_TEST(test_null_pointers);
    RUN_TEST(test_float_equality);
    RUN_TEST(test_with_intentional_failure);

    TEST_SUMMARY();

    return TEST_RETURN_CODE();
}
