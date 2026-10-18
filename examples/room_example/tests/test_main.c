/* clang-format off */
#include "greatest.h"
#include "room_app.h"
/* clang-format on */

/**
 * @file test_main.c
 * @brief Unit tests for the Room ORM example workflow.
 */

/**
 * @brief External test helper `cmn_mock_alloc_set`.
 * @param fail Whether to fail (1) or not (0).
 * @param count Number of allocations before failing.
 * @return CMN_SUCCESS on success.
 */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);

/**
 * @brief Verifies `room_example_run` NULL handling, OOM rollback, and normal
 * execution.
 * @return Greatest test result code.
 */
TEST test_room_example_workflow(void) {
  int32_t found_id = 0;
  int i;

  ASSERT_EQ(CMN_ERR_INVAL, room_example_run(NULL));

  for (i = 0; i < 12; ++i) {
    cmn_error_e rc;
    (void)cmn_mock_alloc_set(1, i);
    rc = room_example_run(&found_id);
    (void)cmn_mock_alloc_set(0, 0);
    if (rc == CMN_SUCCESS) {
      break;
    }
    ASSERT_EQ(CMN_ERR_NOMEM, rc);
  }

  ASSERT_EQ(CMN_SUCCESS, room_example_run(&found_id));
  ASSERT_EQ(1, found_id);
  PASS();
}

/**
 * @brief Test suite for Room ORM example.
 */
SUITE(room_example_suite) { RUN_TEST(test_room_example_workflow); }

/** @brief Greatest test framework main definitions. */
GREATEST_MAIN_DEFS();

/**
 * @brief Test runner entry point for Room ORM example tests.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Process exit code.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(room_example_suite);
  GREATEST_MAIN_END();
}
