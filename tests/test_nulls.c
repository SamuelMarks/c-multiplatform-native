/* clang-format off */
#include "greatest.h"
#include "test_nulls.h"
#include "cmn/a11y_i18n.h"
#include "cmn/app.h"
#include "cmn/automation.h"
#include "cmn/form.h"
#include "cmn/layout.h"
#include "cmn/memory.h"
#include "cmn/router.h"
#include "cmn/signals.h"
#include "cmn/widget.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file test_nulls.c
 * @brief Behavioral verification of NULL pointer and invalid argument checks
 * across all public APIs.
 */

/**
 * @brief Verifies that passing NULL or uninitialized handles to public cmn
 * functions returns `CMN_ERR_INVAL`.
 * @return Greatest test result code.
 */
TEST run_test_nulls(void) {
  /* app */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_run(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_quit(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_destroy(NULL));
  {
    cmn_app_t *app = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_app_destroy(&app));
  }
  ASSERT_EQ(CMN_ERR_INVAL, cmn_app_post_task(NULL, NULL, NULL));

  /* window */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_create(NULL, NULL));
  {
    cmn_app_t *app = NULL;
    ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_window_create(app, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  }
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_title(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_window_set_style(NULL, CMN_WINDOW_STYLE_DEFAULT));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_fullscreen(NULL, CMN_FALSE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_maximize(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_minimize(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_center_on_display(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_get_dpi_scale(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_show(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_destroy(NULL));
  {
    cmn_window_t *w = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_window_destroy(&w));
  }

  /* display */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_display_get_count(NULL));

  /* upcasts & polymorphic widget destroy */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_as_container(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_as_container(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_as_container(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_as_widget(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_as_widget(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_as_widget(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_as_widget(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_as_widget(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_as_widget(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_destroy(NULL));
  {
    cmn_widget_t *w = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_destroy(&w));
  }

  /* widgets */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_text(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_set_on_click(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_trigger_click(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_button_destroy(NULL));
  {
    cmn_button_t *b = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_button_destroy(&b));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_set_text(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_label_destroy(NULL));
  {
    cmn_label_t *l = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_label_destroy(&l));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_get_text(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_set_text(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_set_password(NULL, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_destroy(NULL));
  {
    cmn_textbox_t *t = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_textbox_destroy(&t));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_get_text(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_set_text(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_append_text(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_destroy(NULL));
  {
    cmn_textview_t *tv = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_textview_destroy(&tv));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_destroy(NULL));
  {
    cmn_vbox_t *v = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_vbox_destroy(&v));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_destroy(NULL));
  {
    cmn_hbox_t *h = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_hbox_destroy(&h));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(NULL, NULL));
  {
    cmn_vbox_t *v = NULL;
    ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&v));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_parent(v, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&v));
  }

  /* file picker */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_get_path(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_destroy(NULL));
  {
    cmn_filepicker_t *f = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_filepicker_destroy(&f));
  }

  /* form */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_add_input(NULL, NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_set_debounce(NULL, NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_get_input_debounce(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_get_input_error(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_validate(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_destroy(NULL));
  {
    cmn_form_t *f = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_form_destroy(&f));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_add_page(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_get_current_page(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_next(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_prev(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_destroy(NULL));
  {
    cmn_wizard_t *w = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_destroy(&w));
  }

  /* router */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_create(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_register(NULL, NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_navigate(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_handle_deep_link(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_get_current_path(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_destroy(NULL));
  {
    cmn_router_t *r = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_router_destroy(&r));
  }

  /* signals */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_signal_int_create(0, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_signal_int_get(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_signal_int_set(NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_signal_destroy(NULL));
  {
    cmn_signal_t *s = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_signal_destroy(&s));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_computed_int_create(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_computed_int_get(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_computed_destroy(NULL));
  {
    cmn_computed_t *c = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_computed_destroy(&c));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_effect_create(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_effect_destroy(NULL));
  {
    cmn_effect_t *e = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_effect_destroy(&e));
  }

  /* automation */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_click(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_type_text(NULL, NULL, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_send_key(NULL, CMN_KEY_TAB));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_automation_capture_screenshot(NULL, NULL));

  /* a11y & i18n */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_init(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_set(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_get(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_i18n_is_rtl(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_role(NULL, CMN_A11Y_ROLE_NONE));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_label(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_a11y_set_tabindex(NULL, 0));

  /* dialogs */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_dialog_show_modal(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_message_box_show(NULL, NULL));

  /* layout (flex, grid, bounds, window resize) */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_item_style_init(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_destroy(NULL));
  {
    cmn_flex_t *fl = NULL;
    cmn_edges_t pad = {0, 0, 0, 0};
    cmn_rect_t rect = {0, 0, 10, 10};
    int gap = 0;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_destroy(&fl));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_direction(NULL, CMN_FLEX_ROW));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_direction(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_wrap(NULL, CMN_FLEX_NOWRAP));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_wrap(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL,
              cmn_flex_set_justify_content(NULL, CMN_JUSTIFY_FLEX_START));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_justify_content(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_align_items(NULL, CMN_ALIGN_STRETCH));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_align_items(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_gap(NULL, 0, 0));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_gap(NULL, NULL, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_flex_create(&fl));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_gap(fl, &gap, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_gap(fl, NULL, &gap));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_padding(NULL, pad));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_padding(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(NULL, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(fl, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_child_style(NULL, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_child_style(fl, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_child_style(fl, fl, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_child_rect(NULL, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_child_rect(fl, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_child_rect(fl, fl, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_breakpoint(NULL, 400, CMN_FLEX_COLUMN,
                                                     CMN_FLEX_NOWRAP));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_compute_layout(NULL, 100, 100));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_compute_layout(fl, -1, 100));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_compute_layout(fl, 100, -1));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_as_container(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_as_widget(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_bounds(NULL, rect));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_bounds(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_bounds(fl, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_flex_destroy(&fl));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_item_style_init(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_create(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_destroy(NULL));
  {
    cmn_grid_t *gr = NULL;
    cmn_edges_t pad = {0, 0, 0, 0};
    int gap = 0;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_destroy(&gr));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(NULL, NULL, 0));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_rows(NULL, NULL, 0));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_auto_fit_columns(NULL, 100, 2));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_gap(NULL, 0, 0));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_gap(NULL, NULL, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_grid_create(&gr));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_gap(gr, &gap, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_gap(gr, NULL, &gap));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_padding(NULL, pad));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_padding(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(NULL, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(gr, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_child_style(NULL, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_child_style(gr, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_child_style(gr, gr, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_child_rect(NULL, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_child_rect(gr, NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_child_rect(gr, gr, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_breakpoint(NULL, 400, 1));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_effective_columns(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_effective_columns(gr, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_compute_layout(NULL, 100, 100));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_compute_layout(gr, -1, 100));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_compute_layout(gr, 100, -1));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_as_container(NULL, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_as_widget(NULL, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_grid_destroy(&gr));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_layout(NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_size(NULL, 100, 100));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_get_size(NULL, NULL, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_on_resize(NULL, NULL, NULL));

  /* error.c */
  {
    const char *s = NULL;
    ASSERT_EQ(CMN_SUCCESS, cmn_error_string(CMN_ERR_NOMEM, &s));
    ASSERT_EQ(CMN_SUCCESS, cmn_error_string(CMN_ERR_NOT_FOUND, &s));
    ASSERT_EQ(CMN_SUCCESS, cmn_error_string(CMN_ERR_IO, &s));
    ASSERT_EQ(CMN_SUCCESS, cmn_error_string((cmn_error_e)999, &s));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_error_string(CMN_SUCCESS, NULL));
  }
  /* memory.c */
  {
    void *p = NULL;
    ASSERT_EQ(CMN_SUCCESS, cmn_malloc(0, &p));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_malloc(100, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_calloc(0, 10, &p));
    ASSERT_EQ(CMN_SUCCESS, cmn_calloc(10, 0, &p));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_calloc(10, 10, NULL));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_realloc(NULL, 10, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_realloc(p, 0, &p));
    ASSERT_EQ(CMN_ERR_INVAL, cmn_free(NULL));
  }
  PASS();
}

/**
 * @brief Test suite running all NULL argument validation tests.
 */
SUITE(nulls_suite) { RUN_TEST(run_test_nulls); }
