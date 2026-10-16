/* clang-format off */
#include "greatest.h"
#include "../src/cmn_internal.h"
#include "cmn/app.h"
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
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_window_create(app, &win));
  (void)cmn_mock_alloc_set(0, 0);
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
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_filepicker_create(&picker));
  (void)cmn_mock_alloc_set(0, 0);
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
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_menu_create(&menu));
  (void)cmn_mock_alloc_set(0, 0);
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
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_app_create(&app));
  (void)cmn_mock_alloc_set(0, 0);
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
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_button_create(&btn));
  (void)cmn_mock_alloc_set(0, 0);
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));
  ASSERT_NEQ((void *)NULL, btn);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_text(NULL, "Click Me"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_text(btn, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_set_text(btn, "Click Me"));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_create(NULL));
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_vbox_create(&vbox));
  (void)cmn_mock_alloc_set(0, 0);
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&vbox));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_create(NULL));
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_hbox_create(&hbox));
  (void)cmn_mock_alloc_set(0, 0);
  ASSERT_EQ(CMN_SUCCESS, cmn_hbox_create(&hbox));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_create(NULL));
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_label_create(&lbl));
  (void)cmn_mock_alloc_set(0, 0);
  ASSERT_EQ(CMN_SUCCESS, cmn_label_create(&lbl));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_text(NULL, "txt"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_text(lbl, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_label_set_text(lbl, "txt"));

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb));

  /* Setting a non-container (label) as parent must fail */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(lbl, btn));
  /* Setting a window or non-widget as child must fail */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(vbox, win));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(app, btn));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(win, app));

  /* Valid parenting across window, vbox, hbox, label, textbox, button */
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(hbox, lbl));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_set_parent(hbox, tb));
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
  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_textview_create(&tv));
  (void)cmn_mock_alloc_set(0, 0);

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
 * @brief Test suite for application and widget operations.
 */
SUITE(app_suite) {
  RUN_TEST(test_app_lifecycle);
  RUN_TEST(test_window_properties);
  RUN_TEST(test_filepicker);
  RUN_TEST(test_menus);
  RUN_TEST(test_textview);
  RUN_TEST(test_widget_upcasts_and_polymorphic_destroy);
}
