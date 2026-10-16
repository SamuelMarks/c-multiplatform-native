/* clang-format off */
#include "greatest.h"
#include "cmn/error.h"
/* clang-format on */

/**
 * @file test_error.c
 * @brief Unit tests for `cmn_error_string`.
 */

/**
 * @brief Verifies `cmn_error_string` returns `CMN_ERR_INVAL` when `out_str` is
 * NULL.
 * @return Greatest test result code.
 */
TEST test_error_string_null(void) {
  ASSERT_EQ(CMN_ERR_INVAL, cmn_error_string(CMN_SUCCESS, NULL));
  PASS();
}

/**
 * @brief Verifies `cmn_error_string` descriptions for all `cmn_error_e` values.
 * @return Greatest test result code.
 */
TEST test_error_string_values(void) {
  const char *str = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_error_string(CMN_SUCCESS, &str));
  ASSERT_STR_EQ("Success", str);

  ASSERT_EQ(CMN_SUCCESS, cmn_error_string(CMN_ERR_NOMEM, &str));
  ASSERT_STR_EQ("Out of memory", str);

  ASSERT_EQ(CMN_SUCCESS, cmn_error_string(CMN_ERR_INVAL, &str));
  ASSERT_STR_EQ("Invalid argument", str);

  ASSERT_EQ(CMN_SUCCESS, cmn_error_string(CMN_ERR_NOT_FOUND, &str));
  ASSERT_STR_EQ("Not found", str);

  ASSERT_EQ(CMN_SUCCESS, cmn_error_string(CMN_ERR_IO, &str));
  ASSERT_STR_EQ("I/O error", str);

  ASSERT_EQ(CMN_SUCCESS, cmn_error_string((cmn_error_e)9999, &str));
  ASSERT_STR_EQ("Unknown error", str);

  PASS();
}

/**
 * @brief Test suite for error string mapping.
 */
SUITE(error_suite) {
  RUN_TEST(test_error_string_null);
  RUN_TEST(test_error_string_values);
}
