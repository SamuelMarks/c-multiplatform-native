/* clang-format off */
#include "greatest.h"
#include "cmn/string_safe.h"
#include <string.h>
/* clang-format on */

/**
 * @file test_string_safe.c
 * @brief Unit tests for `cmn_strcpy_s` and `cmn_sprintf_s`.
 */

/**
 * @brief Tests `cmn_strcpy_s` with valid destination and source strings.
 * @return Greatest test result code.
 */
TEST test_strcpy_s_valid(void) {
  char buf[20];
  ASSERT_EQ(CMN_SUCCESS, cmn_strcpy_s(buf, sizeof(buf), "Hello, World!"));
  ASSERT_STR_EQ("Hello, World!", buf);
  PASS();
}

/**
 * @brief Tests `cmn_strcpy_s` with NULL or zero-size parameters.
 * @return Greatest test result code.
 */
TEST test_strcpy_s_invalid(void) {
  char buf[20];
  ASSERT_EQ(CMN_ERR_INVAL, cmn_strcpy_s(NULL, sizeof(buf), "Hello"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_strcpy_s(buf, 0, "Hello"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_strcpy_s(buf, sizeof(buf), NULL));
  PASS();
}

/**
 * @brief Tests `cmn_strcpy_s` when the source string exceeds destination
 * buffer size.
 * @return Greatest test result code.
 */
TEST test_strcpy_s_truncate(void) {
#if !defined(_MSC_VER)
  char buf[5];
  ASSERT_EQ(CMN_ERR_IO, cmn_strcpy_s(buf, sizeof(buf), "Hello, World!"));
#endif
  PASS();
}

/**
 * @brief Tests `cmn_sprintf_s` formatting with valid inputs.
 * @return Greatest test result code.
 */
TEST test_sprintf_s_valid(void) {
  char buf[50];
  ASSERT_EQ(CMN_SUCCESS,
            cmn_sprintf_s(buf, sizeof(buf), "Num: %d, Str: %s", 42, "test"));
  ASSERT_STR_EQ("Num: 42, Str: test", buf);
  PASS();
}

/**
 * @brief Tests `cmn_sprintf_s` with NULL or zero-size arguments.
 * @return Greatest test result code.
 */
TEST test_sprintf_s_invalid(void) {
  char buf[20];
  ASSERT_EQ(CMN_ERR_INVAL, cmn_sprintf_s(NULL, sizeof(buf), "Hello"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_sprintf_s(buf, 0, "Hello"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_sprintf_s(buf, sizeof(buf), NULL));
  PASS();
}

/**
 * @brief Tests `cmn_sprintf_s` buffer overflow detection.
 * @return Greatest test result code.
 */
TEST test_sprintf_s_truncate(void) {
#if !defined(_MSC_VER)
  char buf[5];
  ASSERT_EQ(CMN_ERR_IO, cmn_sprintf_s(buf, sizeof(buf), "Hello, %s!", "World"));
#endif
  PASS();
}

/** @brief External test helper `cmn_mock_vsnprintf_set`. */
extern cmn_error_e cmn_mock_vsnprintf_set(int fail);

/**
 * @brief Tests `cmn_sprintf_s` when `vsnprintf` returns a negative encoding
 * error.
 * @return Greatest test result code.
 */
TEST test_sprintf_s_encoding_error(void) {
#if !defined(_MSC_VER)
  char buf[64];
  (void)cmn_mock_vsnprintf_set(1);
  ASSERT_EQ(CMN_ERR_IO, cmn_sprintf_s(buf, sizeof(buf), "Hello"));
  (void)cmn_mock_vsnprintf_set(0);
#endif
  PASS();
}

/**
 * @brief Test suite for safe string operations.
 */
SUITE(string_safe_suite) {
  RUN_TEST(test_strcpy_s_valid);
  RUN_TEST(test_strcpy_s_invalid);
  RUN_TEST(test_strcpy_s_truncate);
  RUN_TEST(test_sprintf_s_valid);
  RUN_TEST(test_sprintf_s_invalid);
  RUN_TEST(test_sprintf_s_truncate);
  RUN_TEST(test_sprintf_s_encoding_error);
}
