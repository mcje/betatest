#include "betatest.h"

TEST(test_string_contains) {
    const char *text = "Hello, World!";
    ASSERT_STR_CONTAINS(text, "World");
    ASSERT_STR_CONTAINS(text, "Hello");
    ASSERT_STR_CONTAINS(text, ", ");
}

TEST(test_string_starts_with) {
    const char *text = "Hello, World!";
    ASSERT_STR_STARTS_WITH(text, "Hello");
    ASSERT_STR_STARTS_WITH(text, "Hello,");
}

TEST(test_string_ends_with) {
    const char *text = "Hello, World!";
    ASSERT_STR_ENDS_WITH(text, "World!");
    ASSERT_STR_ENDS_WITH(text, "!");
}

TEST(test_string_empty) {
    const char *empty = "";
    const char *not_empty = "data";
    ASSERT_STR_EMPTY(empty);
    ASSERT_STR_NOT_EMPTY(not_empty);
}

TEST(test_string_regex_matches) {
    // Test email pattern
    const char *email = "user@example.com";
    ASSERT_STR_MATCHES(email, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");

    // Test phone number pattern
    const char *phone = "123-456-7890";
    ASSERT_STR_MATCHES(phone, "^[0-9]{3}-[0-9]{3}-[0-9]{4}$");

    // Test hex color code
    const char *color = "#FF5733";
    ASSERT_STR_MATCHES(color, "^#[0-9A-Fa-f]{6}$");

    // Test simple word matching
    const char *text = "Hello World";
    ASSERT_STR_MATCHES(text, "Hello");
    ASSERT_STR_MATCHES(text, ".*World.*");
}

TEST(test_string_regex_numbers) {
    const char *number = "12345";
    ASSERT_STR_MATCHES(number, "^[0-9]+$");

    const char *decimal = "123.45";
    ASSERT_STR_MATCHES(decimal, "^[0-9]+\\.[0-9]+$");
}

int main(void) {
    RUN_TEST(test_string_contains);
    RUN_TEST(test_string_starts_with);
    RUN_TEST(test_string_ends_with);
    RUN_TEST(test_string_empty);
    RUN_TEST(test_string_regex_matches);
    RUN_TEST(test_string_regex_numbers);

    TEST_SUMMARY();
    return TEST_RETURN_CODE();
}
