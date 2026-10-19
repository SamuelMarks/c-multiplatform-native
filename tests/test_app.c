/* clang-format off */
#include "greatest.h"
#include "../src/cmn_internal.h"
#include "cmn/app.h"
#include "cmn/string_safe.h"
#include "cmn/widget.h"
#include <string.h>
#include <stdio.h>
/* clang-format on */

/**
 * @file test_app.c
 * @brief Behavioral unit tests for application lifecycle, windows, menus,
 * file pickers, polymorphic widget handles, and type-safe upcasts.
 */

/** @brief External test helper `cmn_mock_alloc_set`. */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);
/** @brief External test helper `cmn_mock_filepicker_show_dialog_set`. */
extern cmn_error_e cmn_mock_filepicker_show_dialog_set(int bypass);
#ifdef __APPLE__
/** @brief External test helper `cmn_macos_test_exercise_delegate`. */
extern cmn_error_e cmn_macos_test_exercise_delegate(cmn_app_t *app);
#endif

/**
 * @brief Task callback that stops the application event loop.
 * @param userdata Pointer to `cmn_app_t`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e mock_quit_task(void *userdata) {
  cmn_app_t *app = (cmn_app_t *)userdata;
  return cmn_app_quit(app);
}

/**
 * @brief Menu item callback that increments an integer counter.
 * @param userdata Pointer to `int` counter.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e mock_menu_click(void *userdata) {
  int *count = (int *)userdata;
  if (count != NULL) {
    (*count)++;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Tests window creation, styles, fullscreen, display centering, DPI
 * scaling, modal dialogs, and message boxes.
 * @return Greatest test result code.
 */
TEST test_window_properties(void) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_window_t *modal = NULL;
  int count = 0;
  float scale = 0.0f;

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_create(NULL, &win));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_create(app, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &modal));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_title(NULL, "Test"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_title(win, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_title(win, "Main App Window"));

  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_window_set_style(NULL, CMN_WINDOW_STYLE_DEFAULT));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_window_set_style(
                win, (cmn_window_style_e)(CMN_WINDOW_STYLE_FRAMELESS |
                                          CMN_WINDOW_STYLE_TRANSPARENT)));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_style(win, CMN_WINDOW_STYLE_DEFAULT));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_fullscreen(NULL, CMN_TRUE));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_fullscreen(win, CMN_TRUE));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_fullscreen(win, CMN_FALSE));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_maximize(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_maximize(win));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_minimize(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_minimize(win));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_show(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_show(win));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_display_get_count(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_display_get_count(&count));
  ASSERT(count >= 1);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_center_on_display(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_center_on_display(win, -1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_center_on_display(win, 9999));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_center_on_display(win, 0));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_get_dpi_scale(NULL, &scale));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_get_dpi_scale(win, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_get_dpi_scale(win, &scale));
  ASSERT(scale > 0.0f);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_dialog_show_modal(NULL, modal));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_dialog_show_modal(win, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_dialog_show_modal(win, modal));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_message_box_show(NULL, "msg"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_message_box_show("title", NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_app_post_task(app, mock_quit_task, app));
  ASSERT_EQ(CMN_SUCCESS, cmn_message_box_show("title", "msg"));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&modal));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  PASS();
}

/**
 * @brief Tests native file picker creation, dialog bypass, path retrieval, and
 * destruction.
 * @return Greatest test result code.
 */
TEST test_filepicker(void) {
  cmn_filepicker_t *picker = NULL;
  const char *path = NULL;

  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_filepicker_create(&picker));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_create(&picker));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_show_dialog(NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_filepicker_show_dialog_set(1));
  {
    cmn_app_t *app = NULL;
    ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
    ASSERT_EQ(CMN_SUCCESS, cmn_app_post_task(app, mock_quit_task, app));
    ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_show_dialog(picker));
    ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  }

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_filepicker_show_dialog_set(0));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_show_dialog(picker));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_get_path(NULL, &path));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_get_path(picker, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_get_path(picker, &path));
  ASSERT_STR_EQ("/tmp/test", path);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_destroy(&picker));
  PASS();
}

/**
 * @brief Tests menu creation, item registration without side-effects, and
 * explicit `cmn_menu_trigger_item` invocation.
 * @return Greatest test result code.
 */
TEST test_menus(void) {
  cmn_menu_t *menu = NULL;
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  int menu_clicks = 0;

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_menu_create(&menu));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_create(&menu));

  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_menu_add_item(NULL, "Item", mock_menu_click, &menu_clicks));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_menu_add_item(menu, NULL, mock_menu_click, &menu_clicks));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_menu_add_item(menu, "Item 1", mock_menu_click, &menu_clicks));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_add_item(menu, "Item 2 (No CB)", NULL, NULL));

  /* Adding an item must NOT trigger its callback automatically */
  ASSERT_EQ(0, menu_clicks);

  /* Triggering item 0 invokes callback once */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_trigger_item(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_trigger_item(menu, -1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_trigger_item(menu, 99));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_trigger_item(menu, 0));
  ASSERT_EQ(1, menu_clicks);
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_trigger_item(menu, 1));
  ASSERT_EQ(1, menu_clicks);

  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_menu(NULL, menu));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_menu(win, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_menu(win, menu));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_destroy(&menu));

  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  PASS();
}

/**
 * @brief Tests application lifecycle, widgets, and parent-child validation.
 * @return Greatest test result code.
 */
TEST test_app_lifecycle(void) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_button_t *btn = NULL;
  cmn_vbox_t *vbox = NULL;
  cmn_hbox_t *hbox = NULL;
  cmn_label_t *lbl = NULL;
  cmn_textbox_t *tb = NULL;

  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_NEQ((void *)NULL, app);

#ifdef __APPLE__
  ASSERT_EQ(CMN_ERR_INVAL, cmn_macos_test_exercise_delegate(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_macos_test_exercise_delegate(app));
#endif

  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_run(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_quit(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_destroy(NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_post_task(NULL, mock_quit_task, app));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_post_task(app, NULL, app));

  ASSERT_EQ(CMN_SUCCESS, cmn_app_post_task(app, mock_quit_task, app));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_run(app));

  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_NEQ((void *)NULL, win);

  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_title(win, "Test Window"));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_button_create(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));
  ASSERT_NEQ((void *)NULL, btn);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_text(NULL, "Click Me"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_text(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_set_text(btn, "Click Me"));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_vbox_create(&vbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&vbox));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_hbox_create(&hbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_hbox_create(&hbox));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_label_create(&lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_create(&lbl));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_text(NULL, "txt"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_text(lbl, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_set_text(lbl, "txt"));

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_set_password(NULL, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_set_password(tb, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_set_password(tb, 0));

  /* Setting a non-container (label) as parent must fail */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(lbl, btn));
  /* Setting a window or non-widget as child must fail */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(vbox, win));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(app, btn));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(win, app));

  /* Valid parenting across window, vbox, hbox, label, textbox, button */
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(hbox, lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(hbox, tb));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_set_password(tb, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_set_password(tb, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(vbox, hbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(vbox, btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(win, vbox));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_destroy(&lbl));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_hbox_destroy(&hbox));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&vbox));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_on_click(btn, NULL, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_trigger_click(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_trigger_click(btn));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(NULL, btn));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(win, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(win, btn));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));

  PASS();
}

/** @brief External test helper `cmn_mock_textview_get_text_set`. */
extern cmn_error_e cmn_mock_textview_get_text_set(int fail);

/**
 * @brief Tests multi-line `cmn_textview_t` operations and content verification.
 * @return Greatest test result code.
 */
TEST test_textview(void) {
  cmn_textview_t *tv = NULL;
  cmn_window_t *win = NULL;
  cmn_vbox_t *vbox = NULL;
  cmn_app_t *app = NULL;
  const char *text = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&vbox));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_textview_create(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));

  ASSERT_EQ(CMN_SUCCESS, cmn_textview_create(&tv));
  ASSERT_NEQ((void *)NULL, tv);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_set_text(NULL, "Hello"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_set_text(tv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_set_text(tv, "Hello"));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_append_text(NULL, " World"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_append_text(tv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_append_text(tv, " World"));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_append_text(tv, "\nNew Line"));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_text(NULL, &text));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_text(tv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_get_text(tv, &text));
  ASSERT_STR_EQ("Hello World\nNew Line", text);

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_textview_get_text_set(1));
  ASSERT_EQ(CMN_ERR_IO, cmn_textview_get_text(tv, &text));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_textview_get_text_set(0));

  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(vbox, tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(win, vbox));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_destroy(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&vbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));

  PASS();
}

/**
 * @brief Tests type-safe upcast functions (`cmn_*_as_container`,
 * `cmn_*_as_widget`) and polymorphic `cmn_widget_destroy`.
 * @return Greatest test result code.
 */
TEST test_widget_upcasts_and_polymorphic_destroy(void) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_vbox_t *vbox = NULL;
  cmn_hbox_t *hbox = NULL;
  cmn_button_t *btn = NULL;
  cmn_label_t *lbl = NULL;
  cmn_textbox_t *tb = NULL;
  cmn_textview_t *tv = NULL;
  cmn_menu_t *menu = NULL;
  cmn_filepicker_t *fp = NULL;
  cmn_container_t *cont = NULL;
  cmn_widget_t *w = NULL;
  cmn_widget_header_t bad_hdr;
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;

  memset(&bad_hdr, 0, sizeof(bad_hdr));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_header_init(NULL, CMN_WIDGET_KIND_BUTTON,
                                                  (cmn_widget_destroy_fn)0x1));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_widget_header_init(&bad_hdr, CMN_WIDGET_KIND_NONE,
                                   (cmn_widget_destroy_fn)0x1));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_widget_header_init(&bad_hdr, CMN_WIDGET_KIND_BUTTON, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_kind(NULL, &kind));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_kind(&bad_hdr, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_kind(&bad_hdr, &kind));
  bad_hdr.magic = CMN_WIDGET_MAGIC;
  bad_hdr.kind = CMN_WIDGET_KIND_NONE;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_kind(&bad_hdr, &kind));

  {
    cmn_widget_t *bad_w = (cmn_widget_t *)(void *)&bad_hdr;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_destroy(&bad_w));
    bad_hdr.magic = 0;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_destroy(&bad_w));
  }

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&vbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_hbox_create(&hbox));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_create(&lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_create(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_create(&menu));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_create(&fp));

  /* Valid upcasts */
  ASSERT_EQ(CMN_SUCCESS, cmn_window_as_container(win, &cont));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_as_container(vbox, &cont));
  ASSERT_EQ(CMN_SUCCESS, cmn_hbox_as_container(hbox, &cont));

  /* Mismatched upcasts fail with CMN_ERR_INVAL */
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_window_as_container((cmn_window_t *)(void *)btn, &cont));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_vbox_as_container((cmn_vbox_t *)(void *)btn, &cont));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_hbox_as_container((cmn_hbox_t *)(void *)btn, &cont));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_as_widget((cmn_vbox_t *)(void *)btn, &w));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_as_widget((cmn_hbox_t *)(void *)btn, &w));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_button_as_widget((cmn_button_t *)(void *)vbox, &w));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_as_widget((cmn_label_t *)(void *)btn, &w));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_textbox_as_widget((cmn_textbox_t *)(void *)btn, &w));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_textview_as_widget((cmn_textview_t *)(void *)btn, &w));

  /* Polymorphic destruction of each widget kind */
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_as_widget(vbox, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));
  ASSERT_EQ((cmn_widget_t *)NULL, w);

  ASSERT_EQ(CMN_SUCCESS, cmn_hbox_as_widget(hbox, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  ASSERT_EQ(CMN_SUCCESS, cmn_button_as_widget(btn, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  ASSERT_EQ(CMN_SUCCESS, cmn_label_as_widget(lbl, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_as_widget(tb, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  ASSERT_EQ(CMN_SUCCESS, cmn_textview_as_widget(tv, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  w = (cmn_widget_t *)(void *)menu;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  w = (cmn_widget_t *)(void *)fp;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  w = (cmn_widget_t *)(void *)win;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  PASS();
}

/**
 * @brief Generic callback incrementing an integer counter for widget events.
 * @param userdata Pointer to an `int` counter.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e widget_event_counter_cb(void *userdata) {
  int *count = (int *)userdata;
  if (count != NULL) {
    (*count)++;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Tests CheckBox, RadioButton, ToggleButton, Line, Slider, Progress, and
 * UpDown widgets.
 * @return Greatest test result code.
 */
TEST test_extended_basic_widgets(void) {
  cmn_checkbox_t *cb = NULL;
  cmn_checkbox_t *cb3 = NULL;
  cmn_radiobutton_t *rb = NULL;
  cmn_togglebutton_t *tb = NULL;
  cmn_line_t *hline = NULL;
  cmn_line_t *vline = NULL;
  cmn_slider_t *hslider = NULL;
  cmn_slider_t *vslider = NULL;
  cmn_progress_t *prog = NULL;
  cmn_updown_t *ud = NULL;
  cmn_widget_t *w = NULL;
  const char *txt = NULL;
  cmn_check_state_e cstate = CMN_CHECK_OFF;
  cmn_bool_t bval = CMN_FALSE;
  double dval = 0.0;
  uint32_t uval = 0;
  int events = 0;

  /* Allocation & NULL validation */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_create(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_checkbox_create(&cb));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_checkbox_create_tristate(&cb3));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_line_create_vertical(&vline));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_slider_create_vertical(&vslider));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_updown_create(&ud));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));

  /* CheckBox & Tri-state CheckBox */
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_create(&cb));
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_create_tristate(&cb3));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_set_text(NULL, "A"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_set_text(cb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_set_text(cb, "Accept"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_get_text(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_get_text(cb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_get_text(cb, &txt));
  ASSERT_STR_EQ("Accept", txt);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_set_state(NULL, CMN_CHECK_ON));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_set_state(cb, (cmn_check_state_e)99));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_set_state(cb, CMN_CHECK_MIXED));
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_set_state(cb3, CMN_CHECK_MIXED));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_get_state(NULL, &cstate));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_get_state(cb3, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_get_state(cb3, &cstate));
  ASSERT_EQ(CMN_CHECK_MIXED, cstate);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_set_on_toggle(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_trigger_toggle(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_trigger_toggle(cb));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_checkbox_set_on_toggle(cb, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_trigger_toggle(cb));
  ASSERT_EQ(1, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_as_widget(cb, &w));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_checkbox_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_checkbox_destroy(&cb));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy((cmn_widget_t **)(void *)&cb3));

  /* RadioButton */
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_create(&rb));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_set_text(NULL, "R"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_set_text(rb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_set_text(rb, "Option 1"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_get_text(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_get_text(rb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_get_text(rb, &txt));
  ASSERT_STR_EQ("Option 1", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_set_selected(NULL, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_set_selected(rb, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_set_selected(rb, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_get_selected(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_get_selected(rb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_get_selected(rb, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_set_on_select(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_radiobutton_trigger_select(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_trigger_select(rb));
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_set_on_select(
                             rb, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_trigger_select(rb));
  ASSERT_EQ(2, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_radiobutton_as_widget(rb, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  /* ToggleButton */
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_create(&tb));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_set_text(NULL, "T"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_set_text(tb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_set_text(tb, "Bold"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_get_text(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_get_text(tb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_get_text(tb, &txt));
  ASSERT_STR_EQ("Bold", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_set_toggled(NULL, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_set_toggled(tb, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_set_toggled(tb, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_get_toggled(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_get_toggled(tb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_get_toggled(tb, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_set_on_toggle(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_togglebutton_trigger_toggle(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_trigger_toggle(tb));
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_set_on_toggle(
                             tb, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_trigger_toggle(tb));
  ASSERT_EQ(3, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_togglebutton_as_widget(tb, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  /* Line */
  ASSERT_EQ(CMN_SUCCESS, cmn_line_create_horizontal(&hline));
  ASSERT_EQ(CMN_SUCCESS, cmn_line_create_vertical(&vline));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_line_is_vertical(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_line_is_vertical(hline, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_line_is_vertical(hline, &bval));
  ASSERT_EQ(CMN_FALSE, bval);
  ASSERT_EQ(CMN_SUCCESS, cmn_line_is_vertical(vline, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_SUCCESS, cmn_line_as_widget(hline, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));
  ASSERT_EQ(CMN_SUCCESS, cmn_line_destroy(&vline));

  /* Slider */
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_create(&hslider));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_create_vertical(&vslider));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_is_vertical(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_is_vertical(vslider, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_is_vertical(vslider, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_set_value(NULL, 0.5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_set_value(hslider, -0.1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_set_value(hslider, 1.5));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_set_value(hslider, 0.75));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_get_value(NULL, &dval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_get_value(hslider, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_get_value(hslider, &dval));
  ASSERT_EQ(0.75, dval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_set_steps(NULL, 5));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_set_steps(hslider, 10));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_get_steps(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_get_steps(hslider, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_get_steps(hslider, &uval));
  ASSERT_EQ(10u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_set_on_moved(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_slider_trigger_moved(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_trigger_moved(hslider));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_slider_set_on_moved(hslider, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_trigger_moved(hslider));
  ASSERT_EQ(4, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_as_widget(hslider, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));
  ASSERT_EQ(CMN_SUCCESS, cmn_slider_destroy(&vslider));

  /* Progress */
  ASSERT_EQ(CMN_SUCCESS, cmn_progress_create(&prog));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_set_value(NULL, 0.5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_set_value(prog, -0.1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_set_value(prog, 1.1));
  ASSERT_EQ(CMN_SUCCESS, cmn_progress_set_value(prog, 0.5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_get_value(NULL, &dval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_get_value(prog, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_progress_get_value(prog, &dval));
  ASSERT_EQ(0.5, dval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_set_indeterminate(NULL, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_set_indeterminate(prog, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_progress_set_indeterminate(prog, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_get_indeterminate(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_progress_get_indeterminate(prog, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_progress_get_indeterminate(prog, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_SUCCESS, cmn_progress_as_widget(prog, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  /* UpDown */
  ASSERT_EQ(CMN_SUCCESS, cmn_updown_create(&ud));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_set_value(NULL, 5.0));
  ASSERT_EQ(CMN_SUCCESS, cmn_updown_set_value(ud, 10.0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_get_value(NULL, &dval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_get_value(ud, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_updown_get_value(ud, &dval));
  ASSERT_EQ(10.0, dval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_set_step(NULL, 2.0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_set_step(ud, 0.0));
  ASSERT_EQ(CMN_SUCCESS, cmn_updown_set_step(ud, 2.5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_get_step(NULL, &dval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_get_step(ud, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_updown_get_step(ud, &dval));
  ASSERT_EQ(2.5, dval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_set_on_step(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_trigger_step(NULL, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_updown_trigger_step(ud, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_updown_trigger_step(ud, 1));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_updown_set_on_step(ud, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_updown_trigger_step(ud, -1));
  ASSERT_EQ(5, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_updown_as_widget(ud, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  PASS();
}

/**
 * @brief Tests PopUp, Combo, and ListBox widgets.
 * @return Greatest test result code.
 */
TEST test_extended_selection_widgets(void) {
  cmn_popup_t *pu = NULL;
  cmn_combo_t *co = NULL;
  cmn_listbox_t *lb = NULL;
  cmn_widget_t *w = NULL;
  const char *txt = NULL;
  uint32_t uval = 0;
  uint32_t i;
  cmn_bool_t bval = CMN_FALSE;
  int events = 0;

  /* PopUp */
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_create(&pu));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_add_item(NULL, "A"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_add_item(pu, NULL));
  for (i = 0; i < CMN_WIDGET_MAX_ITEMS; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_popup_add_item(pu, "Item"));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_popup_add_item(pu, "Overflow"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_clear(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_clear(pu));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_add_item(pu, "First"));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_add_item(pu, "Second"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_set_item(NULL, 0, "X"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_set_item(pu, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_set_item(pu, 99, "X"));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_set_item(pu, 1, "Updated"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_get_count(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_get_count(pu, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_get_count(pu, &uval));
  ASSERT_EQ(2u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_set_selected(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_set_selected(pu, 99));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_set_selected(pu, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_get_selected(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_get_selected(pu, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_get_selected(pu, &uval));
  ASSERT_EQ(1u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_get_item_text(NULL, 0, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_get_item_text(pu, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_get_item_text(pu, 99, &txt));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_get_item_text(pu, 1, &txt));
  ASSERT_STR_EQ("Updated", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_set_on_select(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_popup_trigger_select(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_trigger_select(pu));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_popup_set_on_select(pu, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_trigger_select(pu));
  ASSERT_EQ(1, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_popup_as_widget(pu, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  /* Combo */
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_create(&co));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_set_text(NULL, "A"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_set_text(co, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_set_text(co, "Typed"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_get_text(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_get_text(co, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_get_text(co, &txt));
  ASSERT_STR_EQ("Typed", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_add_item(NULL, "A"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_add_item(co, NULL));
  for (i = 0; i < CMN_WIDGET_MAX_ITEMS; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_combo_add_item(co, "Opt"));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_combo_add_item(co, "Overflow"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_clear(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_clear(co));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_add_item(co, "Alpha"));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_add_item(co, "Beta"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_get_count(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_get_count(co, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_get_count(co, &uval));
  ASSERT_EQ(2u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_set_selected(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_set_selected(co, 99));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_set_selected(co, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_get_selected(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_get_selected(co, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_get_selected(co, &uval));
  ASSERT_EQ(1u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_set_on_change(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_combo_trigger_change(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_trigger_change(co));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_combo_set_on_change(co, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_trigger_change(co));
  ASSERT_EQ(2, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_combo_as_widget(co, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  /* ListBox */
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_create(&lb));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_add_item(NULL, "A"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_add_item(lb, NULL));
  for (i = 0; i < CMN_WIDGET_MAX_ITEMS; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_listbox_add_item(lb, "Row"));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_listbox_add_item(lb, "Overflow"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_clear(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_clear(lb));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_add_item(lb, "Row0"));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_add_item(lb, "Row1"));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_add_item(lb, "Row2"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_count(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_count(lb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_get_count(lb, &uval));
  ASSERT_EQ(3u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_item_text(NULL, 0, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_item_text(lb, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_item_text(lb, 99, &txt));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_get_item_text(lb, 1, &txt));
  ASSERT_STR_EQ("Row1", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_set_selected(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_set_selected(lb, 99));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_set_selected(lb, 2));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_selected(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_selected(lb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_get_selected(lb, &uval));
  ASSERT_EQ(2u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_remove_item(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_remove_item(lb, 99));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_remove_item(lb, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_set_checkable(NULL, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_set_checkable(lb, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_set_checkable(lb, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_checkable(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_checkable(lb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_get_checkable(lb, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_set_checked(NULL, 0, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_set_checked(lb, 99, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_set_checked(lb, 0, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_set_checked(lb, 0, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_checked(NULL, 0, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_checked(lb, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_get_checked(lb, 99, &bval));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_get_checked(lb, 0, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_set_on_select(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_listbox_trigger_select(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_trigger_select(lb));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_listbox_set_on_select(lb, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_trigger_select(lb));
  ASSERT_EQ(3, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_listbox_as_widget(lb, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  PASS();
}

/**
 * @brief Tests ImageView, TableView, View, WebView, Panel, Tabs, SplitView,
 * and ColorPicker widgets.
 * @return Greatest test result code.
 */
TEST test_extended_rich_and_container_widgets(void) {
  cmn_imageview_t *iv = NULL;
  cmn_tableview_t *tv = NULL;
  cmn_view_t *view = NULL;
  cmn_view_t *sview = NULL;
  cmn_webview_t *wv = NULL;
  cmn_panel_t *panel = NULL;
  cmn_panel_t *spanel = NULL;
  cmn_tabs_t *tabs = NULL;
  cmn_splitview_t *hsplit = NULL;
  cmn_splitview_t *vsplit = NULL;
  cmn_colorpicker_t *cp = NULL;
  cmn_container_t *cont = NULL;
  cmn_widget_t *w = NULL;
  const char *txt = NULL;
  void *ptr0 = NULL;
  void *ptr1 = NULL;
  cmn_scale_mode_e scale = CMN_SCALE_AUTO;
  cmn_bool_t bval = CMN_FALSE;
  uint32_t uval = 0;
  uint32_t i;
  int cw = 0;
  int ch = 0;
  double dval = 0.0;
  int events = 0;
  cmn_rect_t rect = {0, 0, 100, 100};

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_view_create(&view));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_view_create_scroll(&sview));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_panel_create_scroll(&spanel));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_splitview_create_horizontal(&hsplit));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_splitview_create_vertical(&vsplit));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_colorpicker_create(&cp));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));

  /* ImageView */
  ASSERT_EQ(CMN_SUCCESS, cmn_imageview_create(&iv));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_set_image_path(NULL, "a.png"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_set_image_path(iv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_imageview_set_image_path(iv, "logo.png"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_get_image_path(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_get_image_path(iv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_imageview_get_image_path(iv, &txt));
  ASSERT_STR_EQ("logo.png", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_set_scale(NULL, CMN_SCALE_FIT));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_set_scale(iv, (cmn_scale_mode_e)99));
  ASSERT_EQ(CMN_SUCCESS, cmn_imageview_set_scale(iv, CMN_SCALE_FIT));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_get_scale(NULL, &scale));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_get_scale(iv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_imageview_get_scale(iv, &scale));
  ASSERT_EQ(CMN_SCALE_FIT, scale);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_set_on_click(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_imageview_trigger_click(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_imageview_trigger_click(iv));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_imageview_set_on_click(iv, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_imageview_trigger_click(iv));
  ASSERT_EQ(1, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_imageview_as_widget(iv, &w));

  /* TableView */
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_create(&tv));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_add_column(NULL, "C", 50));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_add_column(tv, NULL, 50));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_add_column(tv, "C", 0));
  for (i = 0; i < CMN_TABLEVIEW_MAX_COLS; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_tableview_add_column(tv, "Col", 80));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_tableview_add_column(tv, "Extra", 80));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_count(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_count(tv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_get_column_count(tv, &uval));
  ASSERT_EQ((uint32_t)CMN_TABLEVIEW_MAX_COLS, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_title(NULL, 0, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_title(tv, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_title(tv, 99, &txt));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_get_column_title(tv, 0, &txt));
  ASSERT_STR_EQ("Col", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_row_count(NULL, 2));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_tableview_set_row_count(tv, CMN_TABLEVIEW_MAX_ROWS + 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_set_row_count(tv, 3));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_row_count(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_row_count(tv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_get_row_count(tv, &uval));
  ASSERT_EQ(3u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_cell_text(NULL, 0, 0, "V"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_cell_text(tv, 0, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_cell_text(tv, 99, 0, "V"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_cell_text(tv, 0, 99, "V"));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_set_cell_text(tv, 1, 0, "Cell10"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_cell_text(NULL, 1, 0, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_cell_text(tv, 1, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_cell_text(tv, 99, 0, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_cell_text(tv, 1, 99, &txt));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_get_cell_text(tv, 1, 0, &txt));
  ASSERT_STR_EQ("Cell10", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_selected_row(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_selected_row(tv, 99));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_set_selected_row(tv, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_selected_row(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_selected_row(tv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_get_selected_row(tv, &uval));
  ASSERT_EQ(1u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_on_select(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_trigger_select(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_trigger_select(tv));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_tableview_set_on_select(tv, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_trigger_select(tv));
  ASSERT_EQ(2, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_as_widget(tv, &w));

  /* Custom View */
  ASSERT_EQ(CMN_SUCCESS, cmn_view_create(&view));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_create_scroll(&sview));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_is_scrollable(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_is_scrollable(sview, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_is_scrollable(sview, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_content_size(NULL, 100, 100));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_content_size(view, 0, 100));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_content_size(view, 100, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_set_content_size(view, 800, 600));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_get_content_size(NULL, &cw, &ch));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_get_content_size(view, NULL, &ch));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_get_content_size(view, &cw, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_get_content_size(view, &cw, &ch));
  ASSERT_EQ(800, cw);
  ASSERT_EQ(600, ch);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_on_draw(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_invalidate(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_invalidate(view));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_view_set_on_draw(view, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_invalidate(view));
  ASSERT_EQ(3, events);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_on_click(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_trigger_click(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_click(view));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_view_set_on_click(view, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_click(view));
  ASSERT_EQ(4, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_view_as_widget(view, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_destroy(&sview));

  /* WebView */
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_create(&wv));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_navigate(NULL, "https://example.com"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_navigate(wv, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_go_back(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_go_forward(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_go_back(wv));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_go_forward(wv));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_navigate(wv, "https://a.example"));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_navigate(wv, "https://b.example"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_get_url(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_get_url(wv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_get_url(wv, &txt));
  ASSERT_STR_EQ("https://b.example", txt);
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_go_back(wv));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_get_url(wv, &txt));
  ASSERT_STR_EQ("https://a.example", txt);
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_go_forward(wv));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_get_url(wv, &txt));
  ASSERT_STR_EQ("https://b.example", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_load_html(NULL, "<p>Hi</p>"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_load_html(wv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_load_html(wv, "<h1>Hello</h1>"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_get_html(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_webview_get_html(wv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_get_html(wv, &txt));
  ASSERT_STR_EQ("<h1>Hello</h1>", txt);
  ASSERT_EQ(CMN_SUCCESS, cmn_webview_as_widget(wv, &w));

  /* Panel */
  ASSERT_EQ(CMN_SUCCESS, cmn_panel_create(&panel));
  ASSERT_EQ(CMN_SUCCESS, cmn_panel_create_scroll(&spanel));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_panel_is_scrollable(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_panel_is_scrollable(spanel, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_panel_is_scrollable(spanel, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_SUCCESS, cmn_panel_as_container(panel, &cont));
  ASSERT_EQ(CMN_SUCCESS, cmn_panel_as_widget(panel, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(panel, iv));
  ASSERT_EQ(CMN_SUCCESS, cmn_panel_destroy(&spanel));

  /* Tabs */
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_create(&tabs));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_add_tab(NULL, "T", NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_add_tab(tabs, NULL, NULL));
  for (i = 0; i < CMN_WIDGET_MAX_ITEMS; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_tabs_add_tab(tabs, "Tab", panel));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_tabs_add_tab(tabs, "Overflow", NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_count(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_count(tabs, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_get_count(tabs, &uval));
  ASSERT_EQ((uint32_t)CMN_WIDGET_MAX_ITEMS, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_tab_title(NULL, 0, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_tab_title(tabs, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_tab_title(tabs, 99, &txt));
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_get_tab_title(tabs, 0, &txt));
  ASSERT_STR_EQ("Tab", txt);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_tab_content(NULL, 0, &ptr0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_tab_content(tabs, 0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_tab_content(tabs, 99, &ptr0));
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_get_tab_content(tabs, 0, &ptr0));
  ASSERT_EQ((void *)panel, ptr0);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_set_selected(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_set_selected(tabs, 99));
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_set_selected(tabs, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_selected(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_get_selected(tabs, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_get_selected(tabs, &uval));
  ASSERT_EQ(1u, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_set_on_select(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tabs_trigger_select(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_trigger_select(tabs));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_tabs_set_on_select(tabs, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_trigger_select(tabs));
  ASSERT_EQ(5, events);
  ASSERT_EQ(CMN_SUCCESS, cmn_tabs_as_widget(tabs, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  /* SplitView */
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_create_horizontal(&hsplit));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_create_vertical(&vsplit));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_is_vertical(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_is_vertical(vsplit, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_is_vertical(vsplit, &bval));
  ASSERT_EQ(CMN_TRUE, bval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_set_children(NULL, view, wv));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_set_children(hsplit, NULL, wv));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_set_children(hsplit, view, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_set_children(hsplit, view, wv));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_get_children(NULL, &ptr0, &ptr1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_get_children(hsplit, NULL, &ptr1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_get_children(hsplit, &ptr0, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_get_children(hsplit, &ptr0, &ptr1));
  ASSERT_EQ((void *)view, ptr0);
  ASSERT_EQ((void *)wv, ptr1);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_set_pos(NULL, 0.5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_set_pos(hsplit, -0.1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_set_pos(hsplit, 1.1));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_set_pos(hsplit, 0.3));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_get_pos(NULL, &dval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_get_pos(hsplit, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_get_pos(hsplit, &dval));
  ASSERT_EQ(0.3, dval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_set_pane_visible(NULL, 0, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_splitview_set_pane_visible(hsplit, 2, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_set_pane_visible(hsplit, 0, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_set_pane_visible(hsplit, 0, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_get_pane_visible(NULL, 0, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_get_pane_visible(hsplit, 2, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_splitview_get_pane_visible(hsplit, 0, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_get_pane_visible(hsplit, 0, &bval));
  ASSERT_EQ(CMN_FALSE, bval);
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_as_widget(hsplit, &w));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));
  ASSERT_EQ(CMN_SUCCESS, cmn_splitview_destroy(&vsplit));

  /* ColorPicker */
  ASSERT_EQ(CMN_SUCCESS, cmn_colorpicker_create(&cp));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_bounds(cp, rect));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_bounds(cp, &rect));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_colorpicker_set_color(NULL, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_colorpicker_set_color(cp, 0xFF00AAFFu));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_colorpicker_get_color(NULL, &uval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_colorpicker_get_color(cp, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_colorpicker_get_color(cp, &uval));
  ASSERT_EQ(0xFF00AAFFu, uval);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_colorpicker_show_dialog(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_colorpicker_show_dialog(cp));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_colorpicker_set_on_change(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_colorpicker_trigger_change(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_colorpicker_trigger_change(cp));
  ASSERT_EQ(CMN_SUCCESS, cmn_colorpicker_set_on_change(
                             cp, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_colorpicker_trigger_change(cp));
  ASSERT_EQ(6, events);
  w = (cmn_widget_t *)(void *)cp;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  w = (cmn_widget_t *)(void *)iv;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));
  w = (cmn_widget_t *)(void *)tv;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));
  w = (cmn_widget_t *)(void *)view;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));
  w = (cmn_widget_t *)(void *)wv;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));
  w = (cmn_widget_t *)(void *)panel;
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&w));

  PASS();
}

/**
 * @brief Virtualized TableView cell data provider for testing `OnData`.
 * @param row Requested row index.
 * @param col Requested column index.
 * @param out_buf Output buffer for cell text.
 * @param buf_size Output buffer size.
 * @param userdata Optional error code override pointer.
 * @return CMN_SUCCESS or error code in `userdata`.
 */
static cmn_error_e test_tableview_on_data_cb(uint32_t row, uint32_t col,
                                             char *out_buf, size_t buf_size,
                                             void *userdata) {
  if (userdata != NULL && *(int *)userdata != 0) {
    return CMN_ERR_IO;
  }
  return cmn_strcpy_s(out_buf, buf_size,
                      (row == 0 && col == 0) ? "VirtualCell" : "OtherCell");
}

/**
 * @brief Tests extended Window, Menu, FilePicker, Button, Label, TextBox,
 * TextView, TableView, Custom View, and Cell-level widget controls.
 * @return Greatest test result code.
 */
TEST test_extended_widget_properties(void) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_button_t *btn = NULL;
  cmn_button_t *out_btn = NULL;
  cmn_label_t *lbl = NULL;
  cmn_textbox_t *tb = NULL;
  cmn_textview_t *txv = NULL;
  cmn_menu_t *menu = NULL;
  cmn_menu_t *sub = NULL;
  cmn_filepicker_t *fp = NULL;
  cmn_tableview_t *tv = NULL;
  cmn_view_t *view = NULL;
  cmn_cursor_e cur = CMN_CURSOR_ARROW;
  cmn_bool_t bval = CMN_FALSE;
  cmn_check_state_e cstate = CMN_CHECK_OFF;
  cmn_filepicker_mode_e fpmode = CMN_FILEPICKER_OPEN_FILE;
  cmn_text_align_e align = CMN_TEXT_ALIGN_LEFT;
  const char *exts[2] = {"png", "jpg"};
  const char *bad_exts[2] = {"png", NULL};
  const char *txt = NULL;
  size_t sz = 0;
  uint32_t u0 = 0;
  uint32_t u1 = 0;
  double dval = 0.0;
  int i0 = 0;
  int i1 = 0;
  int events = 0;
  int fail_data = 0;
  int k;
  cmn_rect_t r = {10, 20, 50, 40};
  cmn_rect_t bad_r = {0, 0, -1, 10};
  cmn_edges_t pad = {4, 8, 4, 8};
  cmn_edges_t out_pad = {0, 0, 0, 0};
  cmn_edges_t bad_pad = {-1, 0, 0, 0};

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_create(&lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_create(&txv));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_create(&menu));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_create(&sub));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_create(&fp));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_create(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_create(&view));

  /* Window extended parity */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_default_button(NULL, btn));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_window_set_default_button(win, (cmn_button_t *)(void *)lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_default_button(win, btn));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_get_default_button(NULL, &out_btn));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_get_default_button(win, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_get_default_button(win, &out_btn));
  ASSERT_EQ(btn, out_btn);
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_default_button(win, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_cursor(NULL, CMN_CURSOR_HAND));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_cursor(win, (cmn_cursor_e)99));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_cursor(win, CMN_CURSOR_HAND));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_get_cursor(NULL, &cur));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_get_cursor(win, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_get_cursor(win, &cur));
  ASSERT_EQ(CMN_CURSOR_HAND, cur);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_client_to_screen(NULL, 5, 10, &i0, &i1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_client_to_screen(win, 5, 10, NULL, &i1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_client_to_screen(win, 5, 10, &i0, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_client_to_screen(win, 5, 10, &i0, &i1));
  ASSERT_EQ(5, i0);
  ASSERT_EQ(10, i1);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_on_close(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_trigger_close(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_trigger_close(win));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_window_set_on_close(win, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_trigger_close(win));
  ASSERT_EQ(1, events);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_on_moved(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_trigger_moved(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_trigger_moved(win));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_window_set_on_moved(win, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_trigger_moved(win));
  ASSERT_EQ(2, events);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_on_activate(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_trigger_activate(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_trigger_activate(win));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_window_set_on_activate(win, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_trigger_activate(win));
  ASSERT_EQ(3, events);

  /* Menu extended parity */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_add_separator(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_add_submenu(NULL, "Sub", sub));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_add_submenu(menu, NULL, sub));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_add_submenu(menu, "Sub", NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_add_submenu(menu, "Sub", sub));
  for (k = 0; k < 31; ++k) {
    ASSERT_EQ(CMN_SUCCESS, cmn_menu_add_separator(menu));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_menu_add_separator(menu));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_menu_add_submenu(menu, "Overflow", sub));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_set_item_enabled(NULL, 0, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_set_item_enabled(menu, -1, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_set_item_enabled(menu, 32, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_set_item_enabled(menu, 0, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_set_item_enabled(menu, 0, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_get_item_enabled(NULL, 0, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_get_item_enabled(menu, -1, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_get_item_enabled(menu, 32, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_get_item_enabled(menu, 0, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_get_item_enabled(menu, 0, &bval));
  ASSERT_EQ(CMN_FALSE, bval);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_set_item_state(NULL, 0, CMN_CHECK_ON));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_set_item_state(menu, -1, CMN_CHECK_ON));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_set_item_state(menu, 32, CMN_CHECK_ON));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_menu_set_item_state(menu, 0, (cmn_check_state_e)99));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_set_item_state(menu, 0, CMN_CHECK_ON));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_get_item_state(NULL, 0, &cstate));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_get_item_state(menu, -1, &cstate));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_get_item_state(menu, 32, &cstate));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_get_item_state(menu, 0, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_get_item_state(menu, 0, &cstate));
  ASSERT_EQ(CMN_CHECK_ON, cstate);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_popup(NULL, win, 10, 10));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_menu_popup(menu, NULL, 10, 10));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_popup(menu, win, 10, 10));

  /* FilePicker extended parity */
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_filepicker_set_mode(NULL, CMN_FILEPICKER_SAVE_FILE));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_filepicker_set_mode(fp, (cmn_filepicker_mode_e)99));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_filepicker_set_mode(fp, CMN_FILEPICKER_SELECT_DIR));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_get_mode(NULL, &fpmode));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_get_mode(fp, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_get_mode(fp, &fpmode));
  ASSERT_EQ(CMN_FILEPICKER_SELECT_DIR, fpmode);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_set_filetypes(NULL, exts, 2));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_set_filetypes(fp, NULL, 2));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_set_filetypes(fp, bad_exts, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_set_filetypes(fp, exts, 2));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_get_filetype_count(NULL, &sz));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_get_filetype_count(fp, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_get_filetype_count(fp, &sz));
  ASSERT_EQ((size_t)2, sz);

  /* Button extended parity */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_tooltip(NULL, "Tip"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_tooltip(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_set_tooltip(btn, "Click me"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_get_tooltip(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_get_tooltip(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_get_tooltip(btn, &txt));
  ASSERT_STR_EQ("Click me", txt);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_image(NULL, "icon.png"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_image(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_set_image(btn, "icon.png"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_get_image(NULL, &txt));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_get_image(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_get_image(btn, &txt));
  ASSERT_STR_EQ("icon.png", txt);

  /* Label extended parity */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_multiline(NULL, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_multiline(lbl, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_set_multiline(lbl, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_get_multiline(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_get_multiline(lbl, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_get_multiline(lbl, &bval));
  ASSERT_EQ(CMN_TRUE, bval);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_align(NULL, CMN_TEXT_ALIGN_CENTER));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_align(lbl, (cmn_text_align_e)99));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_set_align(lbl, CMN_TEXT_ALIGN_CENTER));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_get_align(NULL, &align));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_get_align(lbl, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_get_align(lbl, &align));
  ASSERT_EQ(CMN_TEXT_ALIGN_CENTER, align);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_color(NULL, 0x112233FFu));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_set_color(lbl, 0x112233FFu));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_get_color(NULL, &u0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_get_color(lbl, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_get_color(lbl, &u0));
  ASSERT_EQ(0x112233FFu, u0);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_bgcolor(NULL, 0x445566FFu));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_set_bgcolor(lbl, 0x445566FFu));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_get_bgcolor(NULL, &u0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_get_bgcolor(lbl, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_get_bgcolor(lbl, &u0));
  ASSERT_EQ(0x445566FFu, u0);

  /* TextBox extended parity */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_set_editable(NULL, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_set_editable(tb, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_set_editable(tb, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_get_editable(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_get_editable(tb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_get_editable(tb, &bval));
  ASSERT_EQ(CMN_FALSE, bval);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_set_autoselect(NULL, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_set_autoselect(tb, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_set_autoselect(tb, CMN_TRUE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_get_autoselect(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_get_autoselect(tb, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_get_autoselect(tb, &bval));
  ASSERT_EQ(CMN_TRUE, bval);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_select_range(NULL, 0, 5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_select_range(tb, -1, 5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_select_range(tb, 5, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_select_range(tb, 2, 6));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_get_selection_range(NULL, &i0, &i1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_get_selection_range(tb, NULL, &i1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_get_selection_range(tb, &i0, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_get_selection_range(tb, &i0, &i1));
  ASSERT_EQ(2, i0);
  ASSERT_EQ(6, i1);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_clipboard_ops(NULL, CMN_CLIPBOARD_COPY));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_textbox_clipboard_ops(tb, (cmn_clipboard_op_e)99));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_clipboard_ops(tb, CMN_CLIPBOARD_COPY));

  /* TextView extended parity */
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_textview_set_font(NULL, "Menlo", 14.0, CMN_FONT_STYLE_BOLD));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_textview_set_font(txv, NULL, 14.0, CMN_FONT_STYLE_BOLD));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_textview_set_font(txv, "Menlo", 0.0, CMN_FONT_STYLE_BOLD));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_textview_set_font(txv, "Menlo", 14.0, CMN_FONT_STYLE_BOLD));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_font(NULL, &txt, &dval, &u0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_font(txv, NULL, &dval, &u0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_font(txv, &txt, NULL, &u0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_font(txv, &txt, &dval, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_get_font(txv, &txt, &dval, &u0));
  ASSERT_STR_EQ("Menlo", txt);
  ASSERT_EQ(14.0, dval);
  ASSERT_EQ((uint32_t)CMN_FONT_STYLE_BOLD, u0);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_set_colors(NULL, 1, 2));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_textview_set_colors(txv, 0x102030FFu, 0xF0E0D0FFu));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_colors(NULL, &u0, &u1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_colors(txv, NULL, &u1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_colors(txv, &u0, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_get_colors(txv, &u0, &u1));
  ASSERT_EQ(0x102030FFu, u0);
  ASSERT_EQ(0xF0E0D0FFu, u1);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_set_wrap(NULL, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_set_wrap(txv, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_set_wrap(txv, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_wrap(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_wrap(txv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_get_wrap(txv, &bval));
  ASSERT_EQ(CMN_FALSE, bval);

  /* TableView extended parity */
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_add_column(tv, "C0", 80));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_add_column(tv, "C1", 80));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_set_row_count(tv, 2));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_on_data(NULL, NULL, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_set_on_data(
                             tv, test_tableview_on_data_cb, &fail_data));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_get_cell_text(tv, 0, 0, &txt));
  ASSERT_STR_EQ("VirtualCell", txt);
  fail_data = 1;
  ASSERT_EQ(CMN_ERR_IO, cmn_tableview_get_cell_text(tv, 0, 0, &txt));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_column_limits(NULL, 0, 40, 200));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_column_limits(tv, 99, 40, 200));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_column_limits(tv, 0, -1, 200));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_column_limits(tv, 0, 200, 40));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_set_column_limits(tv, 0, 40, 200));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_limits(NULL, 0, &i0, &i1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_limits(tv, 99, &i0, &i1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_limits(tv, 0, NULL, &i1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_limits(tv, 0, &i0, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_get_column_limits(tv, 0, &i0, &i1));
  ASSERT_EQ(40, i0);
  ASSERT_EQ(200, i1);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_column_freeze(NULL, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_set_column_freeze(tv, 99));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_set_column_freeze(tv, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_freeze(NULL, &u0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_tableview_get_column_freeze(tv, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_get_column_freeze(tv, &u0));
  ASSERT_EQ(1u, u0);

  /* Custom View 2D drawing & events */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_draw_rect(NULL, r, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_draw_rect(view, bad_r, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_draw_rect(view, r, 0xFF0000FFu));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_draw_line(NULL, 0, 0, 10, 10, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_draw_line(view, 0, 0, 10, 10, 0x00FF00FFu));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_draw_text(NULL, "Hi", 5, 5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_draw_text(view, NULL, 5, 5));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_draw_text(view, "Hi", 5, 5));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_get_draw_command_count(NULL, &u0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_get_draw_command_count(view, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_get_draw_command_count(view, &u0));
  ASSERT_EQ(3u, u0);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_on_wheel(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_trigger_wheel(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_wheel(view));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_view_set_on_wheel(view, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_wheel(view));
  ASSERT_EQ(4, events);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_on_drag(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_trigger_drag(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_drag(view));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_view_set_on_drag(view, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_drag(view));
  ASSERT_EQ(5, events);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_on_key_down(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_trigger_key_down(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_key_down(view));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_view_set_on_key_down(view, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_key_down(view));
  ASSERT_EQ(6, events);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_set_on_key_up(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_view_trigger_key_up(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_key_up(view));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_view_set_on_key_up(view, widget_event_counter_cb, &events));
  ASSERT_EQ(CMN_SUCCESS, cmn_view_trigger_key_up(view));
  ASSERT_EQ(7, events);

  /* Cell-level widget controls */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_enabled(NULL, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_enabled(menu, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_enabled(btn, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_enabled(btn, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_enabled(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_enabled(menu, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_enabled(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_enabled(btn, &bval));
  ASSERT_EQ(CMN_FALSE, bval);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_visible(NULL, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_visible(menu, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_visible(btn, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_visible(btn, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_visible(NULL, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_visible(menu, &bval));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_visible(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_visible(btn, &bval));
  ASSERT_EQ(CMN_FALSE, bval);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_padding(NULL, pad));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_padding(menu, pad));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_padding(btn, bad_pad));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_padding(btn, pad));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_padding(NULL, &out_pad));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_padding(menu, &out_pad));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_padding(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_padding(btn, &out_pad));
  ASSERT_EQ(4, out_pad.top);
  ASSERT_EQ(8, out_pad.right);

  ASSERT_EQ(CMN_SUCCESS, cmn_view_destroy(&view));
  ASSERT_EQ(CMN_SUCCESS, cmn_tableview_destroy(&tv));
  ASSERT_EQ(CMN_SUCCESS, cmn_filepicker_destroy(&fp));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_destroy(&sub));
  ASSERT_EQ(CMN_SUCCESS, cmn_menu_destroy(&menu));
  ASSERT_EQ(CMN_SUCCESS, cmn_textview_destroy(&txv));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_destroy(&lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  PASS();
}

/**
 * @brief Test suite for application and widget operations.
 */
SUITE(app_suite) {
  RUN_TEST(test_app_lifecycle);
  RUN_TEST(test_window_properties);
  RUN_TEST(test_filepicker);
  RUN_TEST(test_menus);
  RUN_TEST(test_textview);
  RUN_TEST(test_widget_upcasts_and_polymorphic_destroy);
  RUN_TEST(test_extended_basic_widgets);
  RUN_TEST(test_extended_selection_widgets);
  RUN_TEST(test_extended_rich_and_container_widgets);
  RUN_TEST(test_extended_widget_properties);
}
