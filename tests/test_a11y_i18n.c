/* clang-format off */
#include "greatest.h"
#include "cmn/a11y_i18n.h"
#include "cmn/widget.h"
#include "cmn/app.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file test_a11y_i18n.c
 * @brief Unit tests for internationalization (i18n) and accessibility (a11y).
 */

/**
 * @brief Tests `cmn_i18n_init` and `cmn_i18n_set` with valid and invalid
 * inputs.
 * @return Greatest test result code.
 */
TEST test_i18n_init(void) {
  int i;
  char key_buf[32];

  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_init(NULL, "{}"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_init("en-US", NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_set(NULL, "val"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_set("key", NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_set("", "val"));

  ASSERT_EQ(
      CMN_SUCCESS,
      cmn_i18n_init("en-US", "{\"greeting\":\"Hello\",\"farewell\":\"Bye\"}"));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_set("greeting", "Hello Updated"));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("ar-SA", "{}"));

  /* Test malformed JSON fragments in catalog_path */
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("en-US", "{\"unclosed_key"));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("en-US", "{\"key_no_colon\""));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("en-US", "{\"key\":123}"));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("en-US", "{\"key\":\"unclosed_val"));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("en-US", "{\"\":\"empty_key\"}"));

  /* Fill catalog to capacity (32 entries) */
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("en-US", "{}"));
  for (i = 0; i < 31; ++i) {
#if defined(_MSC_VER)
    sprintf_s(key_buf, sizeof(key_buf), "k_%d", i);
#else
    sprintf(key_buf, "k_%d", i);
#endif
    ASSERT_EQ(CMN_SUCCESS, cmn_i18n_set(key_buf, "v"));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_i18n_set("overflow_key", "v"));

  /* Test catalog capacity overflow directly inside cmn_i18n_init */
  {
    char big_catalog[1024];
    size_t pos = 0;
    big_catalog[pos++] = '{';
    for (i = 0; i < 32; ++i) {
      int n;
#if defined(_MSC_VER)
      n = sprintf_s(big_catalog + pos, sizeof(big_catalog) - pos,
                    "%s\"k%d\":\"v\"", i > 0 ? "," : "", i);
#else
      n = sprintf(big_catalog + pos, "%s\"k%d\":\"v\"", i > 0 ? "," : "", i);
#endif
      pos += (size_t)n;
    }
    big_catalog[pos++] = '}';
    big_catalog[pos] = '\0';
    ASSERT_EQ(CMN_ERR_NOMEM, cmn_i18n_init("en-US", big_catalog));
  }

  PASS();
}

/**
 * @brief Tests `cmn_i18n_init` and `cmn_i18n_set` string truncation error
 * paths.
 * @return Greatest test result code.
 */
TEST test_i18n_init_too_long(void) {
#ifndef _MSC_VER
  char long_str[200];
  memset(long_str, 'a', sizeof(long_str) - 1);
  long_str[sizeof(long_str) - 1] = '\0';

  ASSERT_EQ(CMN_ERR_IO, cmn_i18n_init("this-is-a-very-long-locale-string-that-"
                                      "will-cause-strcpy_s-to-fail",
                                      "{}"));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("en-US", "{}"));
  ASSERT_EQ(CMN_ERR_IO, cmn_i18n_set(long_str, "val"));
  ASSERT_EQ(CMN_ERR_IO, cmn_i18n_set("short_key", long_str));
  ASSERT_EQ(CMN_ERR_IO, cmn_i18n_set("hello_world", long_str));
#endif
  PASS();
}

/**
 * @brief Tests `cmn_i18n_is_rtl` across LTR and RTL locales.
 * @return Greatest test result code.
 */
TEST test_i18n_is_rtl(void) {
  int is_rtl = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_is_rtl(NULL, &is_rtl));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_is_rtl("en-US", NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_is_rtl("en-US", &is_rtl));
  ASSERT_EQ(0, is_rtl);
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_is_rtl("artist", &is_rtl));
  ASSERT_EQ(0, is_rtl);
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_is_rtl("ar", &is_rtl));
  ASSERT_EQ(1, is_rtl);
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_is_rtl("ar-SA", &is_rtl));
  ASSERT_EQ(1, is_rtl);
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_is_rtl("ar_EG", &is_rtl));
  ASSERT_EQ(1, is_rtl);
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_is_rtl("he-IL", &is_rtl));
  ASSERT_EQ(1, is_rtl);
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_is_rtl("fa-IR", &is_rtl));
  ASSERT_EQ(1, is_rtl);
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_is_rtl("ur-PK", &is_rtl));
  ASSERT_EQ(1, is_rtl);
  PASS();
}

/**
 * @brief Tests `cmn_i18n_get` translation retrieval and fallback behavior.
 * @return Greatest test result code.
 */
TEST test_i18n_get(void) {
  const char *str = NULL;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_get(NULL, &str));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_get("hello_world", NULL));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_i18n_init("en-US", "{\"custom_key\":\"Custom Value\"}"));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_get("hello_world", &str));
  ASSERT_STR_EQ("Hello, World!", str);
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_get("custom_key", &str));
  ASSERT_STR_EQ("Custom Value", str);

  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_init("ar-SA", "{}"));
  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_get("hello_world", &str));
  ASSERT_STR_EQ("مرحبا بالعالم", str);

  ASSERT_EQ(CMN_SUCCESS, cmn_i18n_get("unknown_key", &str));
  ASSERT_STR_EQ("unknown_key", str);
  PASS();
}

/**
 * @brief Tests `cmn_a11y_set_role`, `cmn_a11y_set_label`, and
 * `cmn_a11y_set_tabindex` across all widget kinds and invalid handles.
 * @return Greatest test result code.
 */
TEST test_a11y_set(void) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_button_t *btn = NULL;
  cmn_label_t *lbl = NULL;
  cmn_textbox_t *tb = NULL;
  cmn_textview_t *tv = NULL;
  cmn_vbox_t *vbox = NULL;
  cmn_hbox_t *hbox = NULL;
  cmn_menu_t *menu = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_create(&lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_create(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&vbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_hbox_create(&hbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_create(&menu));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_role(NULL, CMN_A11Y_ROLE_BUTTON));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_role(btn, CMN_A11Y_ROLE_NONE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_role(btn, (cmn_a11y_role_e)99));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_role(app, CMN_A11Y_ROLE_BUTTON));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_role(menu, CMN_A11Y_ROLE_BUTTON));

  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_role(btn, CMN_A11Y_ROLE_BUTTON));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_role(lbl, CMN_A11Y_ROLE_LABEL));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_role(win, CMN_A11Y_ROLE_WINDOW));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_role(win, CMN_A11Y_ROLE_DIALOG));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_label(NULL, "label"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_label(btn, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_label(app, "label"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_label(menu, "label"));

  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_label(win, "Main Window"));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_label(btn, "Submit Button"));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_label(lbl, "Username Label"));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_label(tb, "Username Input"));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_label(tv, "Log Output"));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_label(vbox, "Vertical Container"));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_label(hbox, "Horizontal Container"));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_tabindex(NULL, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_tabindex(app, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_tabindex(win, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_tabindex(btn, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_tabindex(lbl, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_tabindex(tb, 3));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_tabindex(tv, 4));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_tabindex(vbox, 5));
  ASSERT_EQ(CMN_SUCCESS, cmn_a11y_set_tabindex(hbox, 6));

  ASSERT_EQ(CMN_SUCCESS, cmn_menu_destroy(&menu));
  ASSERT_EQ(CMN_SUCCESS, cmn_hbox_destroy(&hbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&vbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_destroy(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_destroy(&lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  PASS();
}

/**
 * @brief Test suite for a11y and i18n functionality.
 */
SUITE(a11y_i18n_suite) {
  RUN_TEST(test_i18n_init);
  RUN_TEST(test_i18n_init_too_long);
  RUN_TEST(test_i18n_is_rtl);
  RUN_TEST(test_i18n_get);
  RUN_TEST(test_a11y_set);
}
