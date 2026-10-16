/* clang-format off */
#include "greatest.h"
/* clang-format on */

/**
 * @file test_main.c
 * @brief Main test runner entry point executing all cmn test suites.
 */

#ifndef __has_feature
/** @brief Fallback macro when compiler does not support `__has_feature`. */
#define __has_feature(x) 0
#endif

#if defined(__SANITIZE_ADDRESS__) || __has_feature(address_sanitizer)
/** @brief Configures AddressSanitizer options for OOM testing. */
const char *__asan_default_options(void);
/**
 * @brief Returns default AddressSanitizer options allowing NULL returns from
 * malloc.
 * @return Static options string.
 */
const char *__asan_default_options(void) {
  return "allocator_may_return_null=1";
}
#endif

SUITE_EXTERN(error_suite);
SUITE_EXTERN(memory_suite);
SUITE_EXTERN(string_safe_suite);
SUITE_EXTERN(a11y_i18n_suite);
SUITE_EXTERN(automation_suite);
SUITE_EXTERN(nulls_suite);
SUITE_EXTERN(room_suite);
SUITE_EXTERN(app_suite);
SUITE_EXTERN(form_suite);
SUITE_EXTERN(handle_map_suite);
SUITE_EXTERN(router_suite);
SUITE_EXTERN(signals_suite);

/** @brief Greatest test framework main definitions. */
GREATEST_MAIN_DEFS();

/**
 * @brief Main entry point for the `cmn_tests` executable.
 * @param argc Command-line argument count.
 * @param argv Command-line argument vector.
 * @return 0 if all tests pass, non-zero otherwise.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  RUN_SUITE(error_suite);
  RUN_SUITE(memory_suite);
  RUN_SUITE(string_safe_suite);
  RUN_SUITE(a11y_i18n_suite);
  RUN_SUITE(automation_suite);
  RUN_SUITE(nulls_suite);
  RUN_SUITE(room_suite);
  RUN_SUITE(app_suite);
  RUN_SUITE(form_suite);
  RUN_SUITE(handle_map_suite);
  RUN_SUITE(router_suite);
  RUN_SUITE(signals_suite);

  GREATEST_MAIN_END();
}
