/* clang-format off */
#include "greatest.h"
#include "cmn/automation.h"
#include "cmn/widget.h"
#include "cmn/app.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file test_automation.c
 * @brief Behavioral unit tests for UI automation (click, type_text, send_key,
 * screenshot).
 */

/**
 * @brief Callback invoked when the test button is clicked.
 * @param userdata Pointer to `int` click flag.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e mock_click(void *userdata) {
  int *clicked = (int *)userdata;
  *clicked = 1;
  return CMN_SUCCESS;
}

/**
 * @brief Verifies `cmn_automation_click` on buttons and rejects non-button
 * widgets.
 * @return Greatest test result code.
 */
TEST test_automation_click(void) {
  cmn_button_t *btn = NULL;
  cmn_label_t *lbl = NULL;
  int dummy_raw = 0;
  int clicked = 0;

  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_click(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_click(&dummy_raw));

  ASSERT_EQ(CMN_SUCCESS, cmn_label_create(&lbl));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_click(lbl));

  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_set_on_click(btn, mock_click, &clicked));

  ASSERT_EQ(CMN_SUCCESS, cmn_automation_click(btn));
  ASSERT_EQ(1, clicked);

  ASSERT_EQ(CMN_SUCCESS, cmn_label_destroy(&lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btn));
  PASS();
}

/**
 * @brief Verifies `cmn_automation_type_text` updates textbox and textview
 * state and rejects invalid targets.
 * @return Greatest test result code.
 */
TEST test_automation_type_text(void) {
  cmn_textbox_t *tb = NULL;
  cmn_textview_t *tv = NULL;
  cmn_button_t *btn = NULL;
  const char *text = NULL;
  int dummy_raw = 0;

  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_type_text(NULL, "text", 10));

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_create(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_type_text(tb, NULL, 10));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_type_text(tb, "text", -1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_type_text(&dummy_raw, "text", 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_type_text(btn, "text", 0));

  ASSERT_EQ(CMN_SUCCESS, cmn_automation_type_text(tb, "typed_hello", 5));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_get_text(tb, &text));
  ASSERT_STR_EQ("typed_hello", text);

  ASSERT_EQ(CMN_SUCCESS, cmn_automation_type_text(tv, "line1\n", 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_automation_type_text(tv, "line2", 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_get_text(tv, &text));
  ASSERT_STR_EQ("line1\nline2", text);

  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_destroy(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb));
  PASS();
}

/**
 * @brief Verifies `cmn_automation_send_key` validates window handle and keycode
 * enum bounds.
 * @return Greatest test result code.
 */
TEST test_automation_send_key(void) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_button_t *btn = NULL;

  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_send_key(NULL, CMN_KEY_TAB));

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));

  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_automation_send_key((cmn_window_t *)(void *)btn, CMN_KEY_TAB));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_send_key(win, (cmn_keycode_e)0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_send_key(win, (cmn_keycode_e)99));
  ASSERT_EQ(CMN_SUCCESS, cmn_automation_send_key(win, CMN_KEY_TAB));
  ASSERT_EQ(CMN_SUCCESS, cmn_automation_send_key(win, CMN_KEY_ENTER));
  ASSERT_EQ(CMN_SUCCESS, cmn_automation_send_key(win, CMN_KEY_RIGHT));

  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  PASS();
}

/**
 * @brief Verifies `cmn_automation_capture_screenshot` writes a valid image file
 * and rejects invalid paths.
 * @return Greatest test result code.
 */
TEST test_automation_capture_screenshot(void) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  FILE *fp = NULL;
  const char *out_path = "cmn_test_screenshot.png";

  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_capture_screenshot(NULL, out_path));

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_show(win));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_capture_screenshot(win, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_capture_screenshot(win, ""));
  ASSERT_EQ(CMN_ERR_IO, cmn_automation_capture_screenshot(
                            win, "/nonexistent_dir_xyz/a.png"));

  ASSERT_EQ(CMN_SUCCESS, cmn_automation_capture_screenshot(win, out_path));
#if defined(_MSC_VER)
  ASSERT_EQ(0, fopen_s(&fp, out_path, "rb"));
#else
  fp = fopen(out_path, "rb");
#endif
  ASSERT_NEQ((void *)NULL, fp);
  if (fp != NULL) {
    fclose(fp);
    ASSERT_EQ(0, remove(out_path));
  }

  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  PASS();
}

/**
 * @brief Test suite for UI automation functions.
 */
SUITE(automation_suite) {
  RUN_TEST(test_automation_click);
  RUN_TEST(test_automation_type_text);
  RUN_TEST(test_automation_send_key);
  RUN_TEST(test_automation_capture_screenshot);
}
