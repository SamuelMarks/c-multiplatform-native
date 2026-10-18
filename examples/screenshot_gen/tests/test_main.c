/* clang-format off */
#include "greatest.h"
#include "screenshot_gen.h"
#include "cmn/memory.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file test_main.c
 * @brief Unit tests for the real native OS screenshot generator.
 */

/**
 * @brief Tests invalid/NULL arguments across all screenshot capture functions.
 * @return Greatest test result code.
 */
TEST test_screenshot_invalid_args(void) {
  const char *suffix = NULL;

  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_get_platform_suffix(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_screenshot_get_platform_suffix(&suffix));
  ASSERT_NEQ((const char *)NULL, suffix);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_capture_buttons_scene(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_capture_buttons_scene(""));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_capture_inputs_scene(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_capture_inputs_scene(""));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_capture_hello_world_scene(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_capture_hello_world_scene(""));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_capture_password_flow_scene(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_capture_password_flow_scene(""));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_generate_all(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_generate_all("", NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_write_web_html(-1, "out.html"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_write_web_html(4, "out.html"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_write_web_html(0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_screenshot_write_web_html(0, ""));
  ASSERT_EQ(CMN_ERR_IO,
            cmn_screenshot_write_web_html(0, "/nonexistent_dir_xyz/out.html"));
  ASSERT_EQ(CMN_SUCCESS, cmn_screenshot_write_web_html(0, "test_web_0.html"));
  (void)remove("test_web_0.html");

  PASS();
}

/**
 * @brief Tests full native scene capture and OOM resilience.
 * @return Greatest test result code.
 */
TEST test_screenshot_scenes_and_oom(void) {
  int i;

  ASSERT_EQ(CMN_SUCCESS,
            cmn_screenshot_generate_all("test_screenshots_tmp", NULL));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_screenshot_generate_all("test_screenshots_tmp", "custom"));

  for (i = 0; i < 16; ++i) {
    cmn_error_e rc;
    (void)cmn_mock_alloc_set(1, i);
    rc = cmn_screenshot_capture_buttons_scene(
        "test_screenshots_tmp/button_oom.png");
    (void)cmn_mock_alloc_set(0, 0);
    if (rc == CMN_SUCCESS) {
      break;
    }
  }

  for (i = 0; i < 16; ++i) {
    cmn_error_e rc;
    (void)cmn_mock_alloc_set(1, i);
    rc = cmn_screenshot_capture_inputs_scene(
        "test_screenshots_tmp/inputs_oom.png");
    (void)cmn_mock_alloc_set(0, 0);
    if (rc == CMN_SUCCESS) {
      break;
    }
  }

  for (i = 0; i < 16; ++i) {
    cmn_error_e rc;
    (void)cmn_mock_alloc_set(1, i);
    rc = cmn_screenshot_capture_hello_world_scene(
        "test_screenshots_tmp/hello_oom.png");
    (void)cmn_mock_alloc_set(0, 0);
    if (rc == CMN_SUCCESS) {
      break;
    }
  }

  for (i = 0; i < 20; ++i) {
    cmn_error_e rc;
    (void)cmn_mock_alloc_set(1, i);
    rc = cmn_screenshot_capture_password_flow_scene(
        "test_screenshots_tmp/pw_oom.png");
    (void)cmn_mock_alloc_set(0, 0);
    if (rc == CMN_SUCCESS) {
      break;
    }
  }

  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM,
            cmn_screenshot_generate_all("test_screenshots_tmp", "oom"));
  (void)cmn_mock_alloc_set(0, 0);

  (void)remove("test_screenshots_tmp/button_macos.png");
  (void)remove("test_screenshots_tmp/inputs_macos.png");
  (void)remove("test_screenshots_tmp/hello_world_macos.png");
  (void)remove("test_screenshots_tmp/password_flow_macos.png");
  (void)remove("test_screenshots_tmp/button_windows.png");
  (void)remove("test_screenshots_tmp/inputs_windows.png");
  (void)remove("test_screenshots_tmp/hello_world_windows.png");
  (void)remove("test_screenshots_tmp/password_flow_windows.png");
  (void)remove("test_screenshots_tmp/button_linux.png");
  (void)remove("test_screenshots_tmp/inputs_linux.png");
  (void)remove("test_screenshots_tmp/hello_world_linux.png");
  (void)remove("test_screenshots_tmp/password_flow_linux.png");
  (void)remove("test_screenshots_tmp/button_custom.png");
  (void)remove("test_screenshots_tmp/inputs_custom.png");
  (void)remove("test_screenshots_tmp/hello_world_custom.png");
  (void)remove("test_screenshots_tmp/password_flow_custom.png");
  (void)remove("test_screenshots_tmp/button_oom.png");
  (void)remove("test_screenshots_tmp/inputs_oom.png");
  (void)remove("test_screenshots_tmp/hello_world_oom.png");
  (void)remove("test_screenshots_tmp/password_flow_oom.png");
  (void)remove("test_screenshots_tmp/hello_oom.png");
  (void)remove("test_screenshots_tmp/pw_oom.png");

  PASS();
}

/**
 * @brief Test suite for the real native OS screenshot generator.
 */
SUITE(screenshot_gen_suite) {
  RUN_TEST(test_screenshot_invalid_args);
  RUN_TEST(test_screenshot_scenes_and_oom);
}

/** @brief Greatest test framework main definitions. */
GREATEST_MAIN_DEFS();

/**
 * @brief Test runner entry point for screenshot generator tests.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Process exit code.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(screenshot_gen_suite);
  GREATEST_MAIN_END();
}
