/* clang-format off */
#include "greatest.h"
#include "app.h"
/* clang-format on */

/**
 * @file test_main.c
 * @brief Behavioral unit tests for the Hello World 3-row GUI counter app.
 */

/**
 * @brief External test helper `cmn_mock_alloc_set`.
 * @param fail Whether to fail (1) or not (0).
 * @param count Number of allocations before failing.
 * @return CMN_SUCCESS on success.
 */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);

/**
 * @brief Tests `hello_world_app_init`, `hello_world_app_run`, and
 * `hello_world_app_destroy` lifecycle and NULL argument checks.
 * @return Greatest test result code.
 */
TEST test_app_creation_and_destruction(void) {
  hello_world_app_t *app = NULL;
  hello_world_app_t empty_app;
  cmn_error_e rc;

  memset(&empty_app, 0, sizeof(empty_app));

  ASSERT_EQ(CMN_ERR_INVAL, hello_world_app_init(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, hello_world_app_run(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, hello_world_app_run(&empty_app));
  ASSERT_EQ(CMN_ERR_INVAL, hello_world_app_destroy(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, hello_world_app_destroy(&app));

  rc = hello_world_app_init(&app);
  ASSERT_EQ(CMN_SUCCESS, rc);
  ASSERT_NEQ((void *)NULL, app);
  ASSERT_NEQ((void *)NULL, app->flex);

  {
    cmn_rect_t tv_rect;
    ASSERT_EQ(CMN_SUCCESS, cmn_window_set_size(app->window, 640, 480));
    ASSERT_EQ(CMN_SUCCESS,
              cmn_flex_get_child_rect(app->flex, app->textview, &tv_rect));
    ASSERT(tv_rect.height > 140);
  }

  ASSERT_EQ(CMN_SUCCESS, hello_world_app_run(app));

  rc = hello_world_app_destroy(&app);
  ASSERT_EQ(CMN_SUCCESS, rc);
  ASSERT_EQ((void *)NULL, app);

  PASS();
}

/**
 * @brief Simulates button clicks and verifies both `click_count` and
 * `textview` log output.
 * @return Greatest test result code.
 */
TEST test_app_click_simulation(void) {
  hello_world_app_t *app = NULL;
  const char *log_text = NULL;
  cmn_error_e rc;

  rc = hello_world_app_init(&app);
  ASSERT_EQ(CMN_SUCCESS, rc);

  rc = cmn_button_trigger_click(app->button);
  ASSERT_EQ(CMN_SUCCESS, rc);
  ASSERT_EQ(1u, app->click_count);

  rc = cmn_button_trigger_click(app->button);
  ASSERT_EQ(CMN_SUCCESS, rc);
  ASSERT_EQ(2u, app->click_count);

  ASSERT_EQ(CMN_SUCCESS, cmn_textview_get_text(app->textview, &log_text));
  ASSERT_STR_EQ("Button click (1)\nButton click (2)\n", log_text);

  rc = hello_world_app_destroy(&app);
  ASSERT_EQ(CMN_SUCCESS, rc);

  PASS();
}

/**
 * @brief Exhaustively tests all OOM rollback branches in
 * `hello_world_app_init`.
 * @return Greatest test result code.
 */
TEST test_app_alloc_failure(void) {
  hello_world_app_t *app = NULL;
  int i;

  for (i = 0; i < 8; i++) {
    cmn_error_e rc;
    ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, i));
    rc = hello_world_app_init(&app);
    ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
    if (rc == CMN_SUCCESS) {
      ASSERT_EQ(CMN_SUCCESS, hello_world_app_destroy(&app));
      break;
    }
    ASSERT_EQ(CMN_ERR_NOMEM, rc);
    ASSERT_EQ((void *)NULL, app);
  }

  PASS();
}

/**
 * @brief Test suite for Hello World application.
 */
SUITE(hello_world_suite) {
  RUN_TEST(test_app_creation_and_destruction);
  RUN_TEST(test_app_click_simulation);
  RUN_TEST(test_app_alloc_failure);
}

/** @brief Greatest test framework main definitions. */
GREATEST_MAIN_DEFS();

/**
 * @brief Test runner entry point for Hello World tests.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Process exit code.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(hello_world_suite);
  GREATEST_MAIN_END();
}
