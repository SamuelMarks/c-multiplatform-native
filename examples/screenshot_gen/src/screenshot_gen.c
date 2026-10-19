/* clang-format off */
#include "screenshot_gen.h"
#include "cmn/app.h"
#include "cmn/automation.h"
#include "cmn/form.h"
#include "cmn/layout.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include "cmn/widget.h"
#if defined(_WIN32)
#include <direct.h>
#else
#include <cfs/cfs.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file screenshot_gen.c
 * @brief Implementation of the real native OS window and widget screenshot
 * generator in strict C89.
 */

/**
 * @brief Preserves the first non-zero error code between `prev` and `next`.
 * @param prev Previous error status.
 * @param next New operation error status.
 * @return `prev` if non-zero, otherwise `next`.
 */
static cmn_error_e accum_err(cmn_error_e prev, cmn_error_e next) {
  return (prev != CMN_SUCCESS) ? prev : next;
}

/**
 * @brief Returns the canonical platform suffix string (`"macos"`, `"windows"`,
 * `"linux"`, or `"web"`) for the current target OS.
 * @param out_suffix Pointer to receive the static platform suffix string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_get_platform_suffix(const char **out_suffix) {
  if (out_suffix == NULL) {
    return CMN_ERR_INVAL;
  }
#if defined(_WIN32)
  *out_suffix = "windows";
#elif defined(__APPLE__)
  *out_suffix = "macos";
#elif defined(__EMSCRIPTEN__)
  *out_suffix = "web";
#else
  *out_suffix = "linux";
#endif
  return CMN_SUCCESS;
}

/**
 * @brief Button click callback used in the native Buttons showcase window.
 * @param userdata Pointer to an integer click counter.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e on_showcase_btn_click(void *userdata) {
  int *count = (int *)userdata;
  *count = *count + 1;
  return CMN_SUCCESS;
}

/**
 * @brief Builds a real native Buttons showcase window (`cmn_button_t` inside
 * responsive `cmn_flex_t` containers), triggers programmatic button clicks, and
 * captures the live OS window to `filepath`.
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_capture_buttons_scene(const char *filepath) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_flex_t *root_flex = NULL;
  cmn_flex_t *row_flex = NULL;
  cmn_label_t *lbl_header = NULL;
  cmn_button_t *btn_primary = NULL;
  cmn_button_t *btn_secondary = NULL;
  cmn_button_t *btn_counter = NULL;
  cmn_label_t *lbl_status = NULL;
  cmn_edges_t pad = {20, 24, 20, 24};
  int clicks = 0;
  char status_buf[128];
  cmn_error_e rc;

  if (filepath == NULL || filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  rc = cmn_app_create(&app);
  if (rc == CMN_SUCCESS) {
    rc = cmn_window_create(app, &win);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_flex_create(&root_flex);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_flex_create(&row_flex);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_header);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_button_create(&btn_primary);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_button_create(&btn_secondary);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_button_create(&btn_counter);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_status);
  }

  if (rc == CMN_SUCCESS) {
    cmn_rect_t r_hdr = {0, 0, 432, 26};
    cmn_rect_t r_row = {0, 0, 432, 36};
    cmn_rect_t r_btn = {0, 0, 160, 32};
    cmn_rect_t r_act = {0, 0, 432, 32};
    cmn_rect_t r_stat = {0, 0, 432, 26};
    cmn_flex_item_style_t grow_btn;

    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Native Buttons (cmn_button_t)"));
    rc = accum_err(rc, cmn_flex_set_direction(root_flex, CMN_FLEX_COLUMN));
    rc = accum_err(rc, cmn_flex_set_align_items(root_flex, CMN_ALIGN_STRETCH));
    rc = accum_err(rc, cmn_flex_set_gap(root_flex, 16, 16));
    rc = accum_err(rc, cmn_flex_set_padding(root_flex, pad));

    rc = accum_err(rc, cmn_flex_set_direction(row_flex, CMN_FLEX_ROW));
    rc = accum_err(
        rc, cmn_flex_set_justify_content(row_flex, CMN_JUSTIFY_FLEX_START));
    rc = accum_err(rc, cmn_flex_set_align_items(row_flex, CMN_ALIGN_CENTER));
    rc = accum_err(rc, cmn_flex_set_gap(row_flex, 12, 16));
    rc = accum_err(rc, cmn_flex_add_breakpoint(row_flex, 320, CMN_FLEX_COLUMN,
                                               CMN_FLEX_NOWRAP));

    rc =
        accum_err(rc, cmn_label_set_text(
                          lbl_header, "Native OS Push Buttons (cmn_button_t)"));
    rc = accum_err(rc, cmn_button_set_text(btn_primary, "Save Changes"));
    rc = accum_err(rc, cmn_button_set_text(btn_secondary, "Cancel"));
    rc = accum_err(
        rc, cmn_button_set_text(btn_counter, "Trigger Action (Click Me)"));
    rc = accum_err(rc, cmn_button_set_on_click(btn_counter,
                                               on_showcase_btn_click, &clicks));

    rc = accum_err(rc, cmn_widget_set_bounds(lbl_header, r_hdr));
    rc = accum_err(rc, cmn_widget_set_bounds(row_flex, r_row));
    rc = accum_err(rc, cmn_widget_set_bounds(btn_primary, r_btn));
    rc = accum_err(rc, cmn_widget_set_bounds(btn_secondary, r_btn));
    rc = accum_err(rc, cmn_widget_set_bounds(btn_counter, r_act));
    rc = accum_err(rc, cmn_widget_set_bounds(lbl_status, r_stat));

    rc = accum_err(rc, cmn_flex_item_style_init(&grow_btn));
    grow_btn.flex_grow = 1.0f;
    grow_btn.flex_basis = 160;
    rc = accum_err(rc, cmn_flex_add_child(row_flex, btn_primary, &grow_btn));
    rc = accum_err(rc, cmn_flex_add_child(row_flex, btn_secondary, &grow_btn));
    rc = accum_err(rc, cmn_flex_add_child(root_flex, lbl_header, NULL));
    rc = accum_err(rc, cmn_flex_add_child(root_flex, row_flex, NULL));
    rc = accum_err(rc, cmn_flex_add_child(root_flex, btn_counter, NULL));
    rc = accum_err(rc, cmn_flex_add_child(root_flex, lbl_status, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, root_flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));

    rc = accum_err(rc, cmn_automation_click(btn_counter));
    rc = accum_err(rc, cmn_automation_click(btn_counter));
    rc = accum_err(rc, cmn_automation_click(btn_counter));
    rc = accum_err(rc, cmn_sprintf_s(status_buf, sizeof(status_buf),
                                     "Status: Button clicked " CMN_INT32_FMT
                                     " times",
                                     clicks));
    rc = accum_err(rc, cmn_label_set_text(lbl_status, status_buf));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
  }

  if (lbl_status != NULL) {
    rc = accum_err(rc, cmn_label_destroy(&lbl_status));
  }
  if (btn_counter != NULL) {
    rc = accum_err(rc, cmn_button_destroy(&btn_counter));
  }
  if (btn_secondary != NULL) {
    rc = accum_err(rc, cmn_button_destroy(&btn_secondary));
  }
  if (btn_primary != NULL) {
    rc = accum_err(rc, cmn_button_destroy(&btn_primary));
  }
  if (lbl_header != NULL) {
    rc = accum_err(rc, cmn_label_destroy(&lbl_header));
  }
  if (row_flex != NULL) {
    rc = accum_err(rc, cmn_flex_destroy(&row_flex));
  }
  if (root_flex != NULL) {
    rc = accum_err(rc, cmn_flex_destroy(&root_flex));
  }
  if (win != NULL) {
    rc = accum_err(rc, cmn_window_destroy(&win));
  }
  if (app != NULL) {
    rc = accum_err(rc, cmn_app_destroy(&app));
  }
  return rc;
}

/**
 * @brief Builds a real native Inputs showcase window (`cmn_label_t`,
 * `cmn_textbox_t`, and scrollable `cmn_textview_t`), populates them via UI
 * automation, and captures the live OS window to `filepath`.
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_capture_inputs_scene(const char *filepath) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_grid_t *grid = NULL;
  cmn_label_t *lbl_search = NULL;
  cmn_textbox_t *tb_query = NULL;
  cmn_label_t *lbl_log = NULL;
  cmn_textview_t *tv_log = NULL;
  cmn_grid_track_t col_track;
  cmn_edges_t pad = {20, 24, 20, 24};
  cmn_error_e rc;

  if (filepath == NULL || filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  rc = cmn_app_create(&app);
  if (rc == CMN_SUCCESS) {
    rc = cmn_window_create(app, &win);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_grid_create(&grid);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_search);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_textbox_create(&tb_query);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_log);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_textview_create(&tv_log);
  }

  if (rc == CMN_SUCCESS) {
    cmn_grid_track_t row_tracks[4];
    char log_line[128];

    memset(&col_track, 0, sizeof(col_track));
    memset(row_tracks, 0, sizeof(row_tracks));
    col_track.unit = CMN_GRID_UNIT_FR;
    col_track.value = 1.0f;
    row_tracks[0].unit = CMN_GRID_UNIT_PX;
    row_tracks[0].value = 24.0f;
    row_tracks[1].unit = CMN_GRID_UNIT_PX;
    row_tracks[1].value = 30.0f;
    row_tracks[2].unit = CMN_GRID_UNIT_PX;
    row_tracks[2].value = 24.0f;
    row_tracks[3].unit = CMN_GRID_UNIT_FR;
    row_tracks[3].value = 1.0f;

    rc = accum_err(rc, cmn_grid_set_columns(grid, &col_track, 1));
    rc = accum_err(rc, cmn_grid_set_rows(grid, row_tracks, 4));
    rc = accum_err(rc, cmn_grid_set_gap(grid, 10, 10));
    rc = accum_err(rc, cmn_grid_set_padding(grid, pad));

    rc =
        accum_err(rc, cmn_window_set_title(
                          win, "cmn Native Inputs (Label, TextBox, TextView)"));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_search, "Search Filter (cmn_textbox_t):"));
    rc = accum_err(rc, cmn_automation_type_text(
                           tb_query, "SELECT * FROM tasks WHERE id = 1", 0));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_log, "Event Log (Scrollable cmn_textview_t):"));
    rc = accum_err(rc, cmn_textview_set_text(tv_log, ""));
    rc =
        accum_err(rc, cmn_sprintf_s(log_line, sizeof(log_line),
                                    "[01] Signal initialized (val = 0)%c", 10));
    rc = accum_err(rc, cmn_textview_append_text(tv_log, log_line));
    rc = accum_err(rc, cmn_sprintf_s(log_line, sizeof(log_line),
                                     "[02] Route /login -> /welcome%c", 10));
    rc = accum_err(rc, cmn_textview_append_text(tv_log, log_line));
    rc = accum_err(rc,
                   cmn_sprintf_s(log_line, sizeof(log_line),
                                 "[03] Room DAO synced 12 SQLite rows%c", 10));
    rc = accum_err(rc, cmn_textview_append_text(tv_log, log_line));

    rc = accum_err(rc, cmn_grid_add_child(grid, lbl_search, NULL));
    rc = accum_err(rc, cmn_grid_add_child(grid, tb_query, NULL));
    rc = accum_err(rc, cmn_grid_add_child(grid, lbl_log, NULL));
    rc = accum_err(rc, cmn_grid_add_child(grid, tv_log, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, grid));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));

    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
  }

  if (tv_log != NULL) {
    rc = accum_err(rc, cmn_textview_destroy(&tv_log));
  }
  if (lbl_log != NULL) {
    rc = accum_err(rc, cmn_label_destroy(&lbl_log));
  }
  if (tb_query != NULL) {
    rc = accum_err(rc, cmn_textbox_destroy(&tb_query));
  }
  if (lbl_search != NULL) {
    rc = accum_err(rc, cmn_label_destroy(&lbl_search));
  }
  if (grid != NULL) {
    rc = accum_err(rc, cmn_grid_destroy(&grid));
  }
  if (win != NULL) {
    rc = accum_err(rc, cmn_window_destroy(&win));
  }
  if (app != NULL) {
    rc = accum_err(rc, cmn_app_destroy(&app));
  }
  return rc;
}

/**
 * @brief Context for the Hello World counter scene.
 */
struct hello_scene_ctx_s {
  cmn_textview_t *tv; /**< Target scrollable textview. */
  int count;          /**< Number of clicks recorded. */
};

/**
 * @brief Button click handler for the Hello World counter scene.
 * @param userdata Pointer to `struct hello_scene_ctx_s`.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e on_hello_scene_click(void *userdata) {
  struct hello_scene_ctx_s *ctx = (struct hello_scene_ctx_s *)userdata;
  char line[128];
  cmn_error_e rc;
  ctx->count++;
  rc = cmn_sprintf_s(line, sizeof(line), "Button click (" CMN_INT32_FMT ")%c",
                     ctx->count, 10);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  return cmn_textview_append_text(ctx->tv, line);
}

/**
 * @brief Builds the real Hello World 3-row counter application window
 * (`cmn_label_t`, `cmn_button_t`, `cmn_textview_t`), simulates 4 native button
 * clicks, and captures the live OS window to `filepath`.
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_capture_hello_world_scene(const char *filepath) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_flex_t *flex = NULL;
  cmn_label_t *lbl = NULL;
  cmn_button_t *btn = NULL;
  cmn_textview_t *tv = NULL;
  cmn_edges_t pad = {16, 20, 16, 20};
  cmn_flex_item_style_t tv_style;
  struct hello_scene_ctx_s ctx;
  cmn_error_e rc;

  if (filepath == NULL || filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  rc = cmn_app_create(&app);
  if (rc == CMN_SUCCESS) {
    rc = cmn_window_create(app, &win);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_flex_create(&flex);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_button_create(&btn);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_textview_create(&tv);
  }

  if (rc == CMN_SUCCESS) {
    cmn_rect_t r_lbl = {0, 0, 400, 26};
    cmn_rect_t r_btn = {0, 0, 160, 32};
    cmn_rect_t r_tv = {0, 0, 400, 160};
    cmn_flex_item_style_t btn_style;

    ctx.tv = tv;
    ctx.count = 0;

    rc = accum_err(rc, cmn_window_set_title(win, "Hello World"));
    rc = accum_err(rc, cmn_flex_set_direction(flex, CMN_FLEX_COLUMN));
    rc = accum_err(rc, cmn_flex_set_gap(flex, 12, 12));
    rc = accum_err(rc, cmn_flex_set_padding(flex, pad));

    rc = accum_err(rc, cmn_label_set_text(lbl, "Hello!, I'm a label"));
    rc = accum_err(rc, cmn_button_set_text(btn, "Click Me!"));
    rc =
        accum_err(rc, cmn_button_set_on_click(btn, on_hello_scene_click, &ctx));

    rc = accum_err(rc, cmn_widget_set_bounds(lbl, r_lbl));
    rc = accum_err(rc, cmn_widget_set_bounds(btn, r_btn));
    rc = accum_err(rc, cmn_widget_set_bounds(tv, r_tv));

    rc = accum_err(rc, cmn_flex_add_child(flex, lbl, NULL));
    rc = accum_err(rc, cmn_flex_item_style_init(&btn_style));
    btn_style.align_self = CMN_ALIGN_FLEX_START;
    rc = accum_err(rc, cmn_flex_add_child(flex, btn, &btn_style));
    rc = accum_err(rc, cmn_flex_item_style_init(&tv_style));
    tv_style.flex_grow = 1.0f;
    tv_style.flex_basis = 160;
    rc = accum_err(rc, cmn_flex_add_child(flex, tv, &tv_style));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));

    rc = accum_err(rc, cmn_automation_click(btn));
    rc = accum_err(rc, cmn_automation_click(btn));
    rc = accum_err(rc, cmn_automation_click(btn));
    rc = accum_err(rc, cmn_automation_click(btn));

    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
  }

  if (tv != NULL) {
    rc = accum_err(rc, cmn_textview_destroy(&tv));
  }
  if (btn != NULL) {
    rc = accum_err(rc, cmn_button_destroy(&btn));
  }
  if (lbl != NULL) {
    rc = accum_err(rc, cmn_label_destroy(&lbl));
  }
  if (flex != NULL) {
    rc = accum_err(rc, cmn_flex_destroy(&flex));
  }
  if (win != NULL) {
    rc = accum_err(rc, cmn_window_destroy(&win));
  }
  if (app != NULL) {
    rc = accum_err(rc, cmn_app_destroy(&app));
  }
  return rc;
}

/**
 * @brief Non-empty validator for the Password Flow scene.
 * @param text Input text.
 * @param userdata Unused pointer.
 * @param out_valid Pointer to store validity flag.
 * @param out_err Pointer to store error message string.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e validate_non_empty(const char *text, void *userdata,
                                      int *out_valid, char **out_err) {
  *out_valid = (text != NULL && strlen(text) >= 4) ? 1 : 0;
  *out_err = (char *)userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Builds the real Simple Password Flow application window (`cmn_form_t`,
 * `cmn_textbox_t`, `cmn_button_t`, `cmn_label_t`), enters credentials,
 * validates the form, and captures the live OS window to `filepath`.
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_capture_password_flow_scene(const char *filepath) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_grid_t *grid = NULL;
  cmn_form_t *form = NULL;
  cmn_label_t *lbl_user = NULL;
  cmn_textbox_t *tb_user = NULL;
  cmn_label_t *lbl_pass = NULL;
  cmn_textbox_t *tb_pass = NULL;
  cmn_label_t *lbl_status = NULL;
  cmn_button_t *btn_submit = NULL;
  cmn_edges_t pad = {20, 24, 20, 24};
  int is_valid = 0;
  cmn_error_e rc;

  if (filepath == NULL || filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  rc = cmn_app_create(&app);
  if (rc == CMN_SUCCESS) {
    rc = cmn_window_create(app, &win);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_grid_create(&grid);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_form_create(&form);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_user);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_textbox_create(&tb_user);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_pass);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_textbox_create(&tb_pass);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_status);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_button_create(&btn_submit);
  }

  if (rc == CMN_SUCCESS) {
    cmn_grid_track_t row_tracks[6];
    cmn_grid_item_style_t btn_gs;
    cmn_rect_t r_btn = {0, 0, 180, 34};

    memset(row_tracks, 0, sizeof(row_tracks));
    row_tracks[0].unit = CMN_GRID_UNIT_PX;
    row_tracks[0].value = 24.0f;
    row_tracks[1].unit = CMN_GRID_UNIT_PX;
    row_tracks[1].value = 30.0f;
    row_tracks[2].unit = CMN_GRID_UNIT_PX;
    row_tracks[2].value = 24.0f;
    row_tracks[3].unit = CMN_GRID_UNIT_PX;
    row_tracks[3].value = 30.0f;
    row_tracks[4].unit = CMN_GRID_UNIT_PX;
    row_tracks[4].value = 26.0f;
    row_tracks[5].unit = CMN_GRID_UNIT_PX;
    row_tracks[5].value = 34.0f;

    rc = accum_err(rc,
                   cmn_window_set_title(win, "Simple Password Flow (/login)"));
    rc = accum_err(rc, cmn_grid_set_auto_fit_columns(grid, 260, 1));
    rc = accum_err(rc, cmn_grid_set_rows(grid, row_tracks, 6));
    rc = accum_err(rc, cmn_grid_set_gap(grid, 10, 10));
    rc = accum_err(rc, cmn_grid_set_padding(grid, pad));
    rc = accum_err(rc, cmn_grid_add_breakpoint(grid, 400, 1));

    rc = accum_err(rc, cmn_label_set_text(lbl_user, "Username:"));
    rc = accum_err(rc, cmn_automation_type_text(tb_user, "samuel_marks", 0));
    rc = accum_err(rc, cmn_label_set_text(lbl_pass, "Password (min 4 chars):"));
    rc = accum_err(rc, cmn_textbox_set_password(tb_pass, 1));
    rc = accum_err(
        rc, cmn_automation_type_text(tb_pass, "correct_horse_battery", 0));
    rc = accum_err(rc, cmn_button_set_text(btn_submit, "Sign in / Sign up"));

    rc = accum_err(rc,
                   cmn_form_add_input(form, tb_user, validate_non_empty, NULL));
    rc = accum_err(rc,
                   cmn_form_add_input(form, tb_pass, validate_non_empty, NULL));
    rc = accum_err(rc, cmn_form_validate(form, &is_valid));

    rc = accum_err(rc, cmn_label_set_text(
                           lbl_status, is_valid ? "Form Status: Valid (Ready)"
                                                : "Form Status: Invalid"));

    rc = accum_err(rc, cmn_widget_set_bounds(btn_submit, r_btn));
    rc = accum_err(rc, cmn_grid_item_style_init(&btn_gs));
    btn_gs.justify_self = CMN_ALIGN_FLEX_START;

    rc = accum_err(rc, cmn_grid_add_child(grid, lbl_user, NULL));
    rc = accum_err(rc, cmn_grid_add_child(grid, tb_user, NULL));
    rc = accum_err(rc, cmn_grid_add_child(grid, lbl_pass, NULL));
    rc = accum_err(rc, cmn_grid_add_child(grid, tb_pass, NULL));
    rc = accum_err(rc, cmn_grid_add_child(grid, lbl_status, NULL));
    rc = accum_err(rc, cmn_grid_add_child(grid, btn_submit, &btn_gs));
    rc = accum_err(rc, cmn_window_set_layout(win, grid));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));

    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
  }

  if (btn_submit != NULL) {
    rc = accum_err(rc, cmn_button_destroy(&btn_submit));
  }
  if (lbl_status != NULL) {
    rc = accum_err(rc, cmn_label_destroy(&lbl_status));
  }
  if (tb_pass != NULL) {
    rc = accum_err(rc, cmn_textbox_destroy(&tb_pass));
  }
  if (lbl_pass != NULL) {
    rc = accum_err(rc, cmn_label_destroy(&lbl_pass));
  }
  if (tb_user != NULL) {
    rc = accum_err(rc, cmn_textbox_destroy(&tb_user));
  }
  if (lbl_user != NULL) {
    rc = accum_err(rc, cmn_label_destroy(&lbl_user));
  }
  if (form != NULL) {
    rc = accum_err(rc, cmn_form_destroy(&form));
  }
  if (grid != NULL) {
    rc = accum_err(rc, cmn_grid_destroy(&grid));
  }
  if (win != NULL) {
    rc = accum_err(rc, cmn_window_destroy(&win));
  }
  if (app != NULL) {
    rc = accum_err(rc, cmn_app_destroy(&app));
  }
  return rc;
}

/** @brief Canonical base names for all 24 visual widget and app scenes. */
static const char *const g_scene_names[CMN_SCREENSHOT_SCENE_COUNT] = {
    "button",      "label",        "textbox",     "textview",     "checkbox",
    "radiobutton", "togglebutton", "line",        "slider",       "progress",
    "updown",      "popup",        "combo",       "listbox",      "imageview",
    "tableview",   "view",         "webview",     "panel",        "tabs",
    "splitview",   "inputs",       "hello_world", "password_flow"};

/**
 * @brief Returns the canonical base name for a given scene index (`0`..`23`).
 * @param scene_id Scene index (`0`..`CMN_SCREENSHOT_SCENE_COUNT - 1`).
 * @param out_name Pointer to receive the static scene base name string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_get_scene_name(int scene_id, const char **out_name) {
  if (scene_id < 0 || scene_id >= CMN_SCREENSHOT_SCENE_COUNT ||
      out_name == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_name = g_scene_names[scene_id];
  return CMN_SUCCESS;
}

/**
 * @brief Captures a single visual widget or application scene by index
 * (`0`..`CMN_SCREENSHOT_SCENE_COUNT - 1`) to `filepath`.
 * @param scene_id Scene index (`0`..`CMN_SCREENSHOT_SCENE_COUNT - 1`).
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_capture_scene(int scene_id, const char *filepath) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_flex_t *flex = NULL;
  cmn_label_t *lbl_hdr = NULL;
  cmn_label_t *lbl_sub = NULL;
  cmn_edges_t pad = {20, 24, 20, 24};
  cmn_rect_t r_hdr = {0, 0, 432, 26};
  cmn_rect_t r_sub = {0, 0, 432, 24};
  cmn_error_e rc;

  if (scene_id < 0 || scene_id >= CMN_SCREENSHOT_SCENE_COUNT ||
      filepath == NULL || filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  if (scene_id == 0) {
    return cmn_screenshot_capture_buttons_scene(filepath);
  }
  if (scene_id == 21) {
    return cmn_screenshot_capture_inputs_scene(filepath);
  }
  if (scene_id == 22) {
    return cmn_screenshot_capture_hello_world_scene(filepath);
  }
  if (scene_id == 23) {
    return cmn_screenshot_capture_password_flow_scene(filepath);
  }

  rc = cmn_app_create(&app);
  if (rc == CMN_SUCCESS) {
    rc = cmn_window_create(app, &win);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_flex_create(&flex);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_hdr);
  }
  if (rc == CMN_SUCCESS) {
    rc = cmn_label_create(&lbl_sub);
  }
  if (rc != CMN_SUCCESS) {
    if (lbl_sub != NULL)
      rc = accum_err(rc, cmn_label_destroy(&lbl_sub));
    if (lbl_hdr != NULL)
      rc = accum_err(rc, cmn_label_destroy(&lbl_hdr));
    if (flex != NULL)
      rc = accum_err(rc, cmn_flex_destroy(&flex));
    if (win != NULL)
      rc = accum_err(rc, cmn_window_destroy(&win));
    if (app != NULL)
      rc = accum_err(rc, cmn_app_destroy(&app));
    return rc;
  }

  rc = accum_err(rc, cmn_flex_set_direction(flex, CMN_FLEX_COLUMN));
  rc = accum_err(rc, cmn_flex_set_align_items(flex, CMN_ALIGN_STRETCH));
  rc = accum_err(rc, cmn_flex_set_gap(flex, 14, 14));
  rc = accum_err(rc, cmn_flex_set_padding(flex, pad));
  rc = accum_err(rc, cmn_widget_set_bounds(lbl_hdr, r_hdr));
  rc = accum_err(rc, cmn_widget_set_bounds(lbl_sub, r_sub));
  rc = accum_err(rc, cmn_flex_add_child(flex, lbl_hdr, NULL));

  switch (scene_id) {
  case 1: { /* label */
    cmn_label_t *l1 = NULL, *l2 = NULL;
    cmn_rect_t rl = {0, 0, 432, 28};
    rc = accum_err(rc,
                   cmn_window_set_title(win, "cmn Native Label (cmn_label_t)"));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_hdr, "Native OS Text Labels (cmn_label_t)"));
    rc = accum_err(rc, cmn_label_create(&l1));
    rc = accum_err(rc, cmn_label_create(&l2));
    rc =
        accum_err(rc, cmn_label_set_text(
                          l1, "Primary Heading: Cross-Platform C89 Native UI"));
    rc =
        accum_err(rc, cmn_label_set_text(
                          l2, "Multiline & Ellipsis-Aware Typography Control"));
    rc = accum_err(rc,
                   cmn_label_set_text(lbl_sub, "Status: UTF-8 & i18n Ready"));
    rc = accum_err(rc, cmn_widget_set_bounds(l1, rl));
    rc = accum_err(rc, cmn_widget_set_bounds(l2, rl));
    rc = accum_err(rc, cmn_flex_add_child(flex, l1, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, l2, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_label_destroy(&l2));
    rc = accum_err(rc, cmn_label_destroy(&l1));
    break;
  }
  case 2: { /* textbox */
    cmn_textbox_t *tb1 = NULL, *tb2 = NULL;
    cmn_rect_t rt = {0, 0, 432, 30};
    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Native TextBox (cmn_textbox_t)"));
    rc = accum_err(rc, cmn_label_set_text(lbl_hdr,
                                          "Single-Line Text & Password Inputs "
                                          "(cmn_textbox_t)"));
    rc = accum_err(rc, cmn_textbox_create(&tb1));
    rc = accum_err(rc, cmn_textbox_create(&tb2));
    rc = accum_err(
        rc, cmn_automation_type_text(tb1, "https://api.cmn.dev/v1/sync", 0));
    rc = accum_err(rc, cmn_textbox_set_password(tb2, 1));
    rc = accum_err(rc, cmn_automation_type_text(tb2, "super_secret_token", 0));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_sub, "Supports plain text, password masking & "
                                    "selection"));
    rc = accum_err(rc, cmn_widget_set_bounds(tb1, rt));
    rc = accum_err(rc, cmn_widget_set_bounds(tb2, rt));
    rc = accum_err(rc, cmn_flex_add_child(flex, tb1, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, tb2, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_textbox_destroy(&tb2));
    rc = accum_err(rc, cmn_textbox_destroy(&tb1));
    break;
  }
  case 3: { /* textview */
    cmn_textview_t *tv = NULL;
    cmn_rect_t rtv = {0, 0, 432, 210};
    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Native TextView (cmn_textview_t)"));
    rc = accum_err(rc, cmn_label_set_text(lbl_hdr,
                                          "Multi-Line Scrollable Text Editor "
                                          "(cmn_textview_t)"));
    rc = accum_err(rc, cmn_textview_create(&tv));
    rc =
        accum_err(rc, cmn_textview_set_text(
                          tv, "[INFO] Initialized cmn v0.1.0 runtime\n"
                              "[INFO] Loaded SQLite Room database schema\n"
                              "[SYNC] Fetched 24 records from remote endpoint\n"
                              "[A11Y] Attached native accessibility tree "
                              "roles\n"));
    rc = accum_err(rc, cmn_widget_set_bounds(tv, rtv));
    rc = accum_err(rc, cmn_flex_add_child(flex, tv, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_textview_destroy(&tv));
    break;
  }
  case 4: { /* checkbox */
    cmn_checkbox_t *cb1 = NULL, *cb2 = NULL, *cb3 = NULL;
    cmn_rect_t rcb = {0, 0, 432, 28};
    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Native CheckBox (cmn_checkbox_t)"));
    rc = accum_err(rc, cmn_label_set_text(lbl_hdr,
                                          "Two-State & Tri-State Checkboxes "
                                          "(cmn_checkbox_t)"));
    rc = accum_err(rc, cmn_checkbox_create(&cb1));
    rc = accum_err(rc, cmn_checkbox_create(&cb2));
    rc = accum_err(rc, cmn_checkbox_create_tristate(&cb3));
    rc = accum_err(rc, cmn_checkbox_set_text(
                           cb1, "Enable automatic background database sync"));
    rc = accum_err(rc, cmn_checkbox_set_state(cb1, CMN_CHECK_ON));
    rc = accum_err(
        rc, cmn_checkbox_set_text(cb2, "Send anonymous crash diagnostics"));
    rc = accum_err(rc, cmn_checkbox_set_state(cb2, CMN_CHECK_OFF));
    rc = accum_err(rc,
                   cmn_checkbox_set_text(
                       cb3, "Select all workspace modules (Tri-State Mixed)"));
    rc = accum_err(rc, cmn_checkbox_set_state(cb3, CMN_CHECK_MIXED));
    rc = accum_err(rc, cmn_widget_set_bounds(cb1, rcb));
    rc = accum_err(rc, cmn_widget_set_bounds(cb2, rcb));
    rc = accum_err(rc, cmn_widget_set_bounds(cb3, rcb));
    rc = accum_err(rc, cmn_flex_add_child(flex, cb1, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, cb2, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, cb3, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_checkbox_destroy(&cb3));
    rc = accum_err(rc, cmn_checkbox_destroy(&cb2));
    rc = accum_err(rc, cmn_checkbox_destroy(&cb1));
    break;
  }
  case 5: { /* radiobutton */
    cmn_radiobutton_t *rb1 = NULL, *rb2 = NULL, *rb3 = NULL;
    cmn_rect_t rrb = {0, 0, 432, 28};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native RadioButton (cmn_radiobutton_t)"));
    rc = accum_err(rc, cmn_label_set_text(lbl_hdr,
                                          "Mutually Exclusive Radio Buttons "
                                          "(cmn_radiobutton_t)"));
    rc = accum_err(rc, cmn_radiobutton_create(&rb1));
    rc = accum_err(rc, cmn_radiobutton_create(&rb2));
    rc = accum_err(rc, cmn_radiobutton_create(&rb3));
    rc = accum_err(
        rc, cmn_radiobutton_set_text(rb1, "Release Mode (-O3, LTO Enabled)"));
    rc = accum_err(rc, cmn_radiobutton_set_selected(rb1, CMN_TRUE));
    rc = accum_err(
        rc, cmn_radiobutton_set_text(rb2, "Debug Mode (-g, AddressSanitizer)"));
    rc = accum_err(rc, cmn_radiobutton_set_selected(rb2, CMN_FALSE));
    rc = accum_err(
        rc, cmn_radiobutton_set_text(rb3, "Minimum Size (-Os, Strip Symbols)"));
    rc = accum_err(rc, cmn_radiobutton_set_selected(rb3, CMN_FALSE));
    rc = accum_err(rc, cmn_widget_set_bounds(rb1, rrb));
    rc = accum_err(rc, cmn_widget_set_bounds(rb2, rrb));
    rc = accum_err(rc, cmn_widget_set_bounds(rb3, rrb));
    rc = accum_err(rc, cmn_flex_add_child(flex, rb1, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, rb2, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, rb3, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_radiobutton_destroy(&rb3));
    rc = accum_err(rc, cmn_radiobutton_destroy(&rb2));
    rc = accum_err(rc, cmn_radiobutton_destroy(&rb1));
    break;
  }
  case 6: { /* togglebutton */
    cmn_togglebutton_t *tb1 = NULL, *tb2 = NULL;
    cmn_rect_t rtb = {0, 0, 260, 34};
    rc =
        accum_err(rc, cmn_window_set_title(
                          win, "cmn Native ToggleButton (cmn_togglebutton_t)"));
    rc = accum_err(rc,
                   cmn_label_set_text(lbl_hdr, "Latching Push Toggle Buttons "
                                               "(cmn_togglebutton_t)"));
    rc = accum_err(rc, cmn_togglebutton_create(&tb1));
    rc = accum_err(rc, cmn_togglebutton_create(&tb2));
    rc = accum_err(rc,
                   cmn_togglebutton_set_text(tb1, "Live Telemetry Stream: ON"));
    rc = accum_err(rc, cmn_togglebutton_set_toggled(tb1, CMN_TRUE));
    rc = accum_err(rc,
                   cmn_togglebutton_set_text(tb2, "Offline Cache Mode: OFF"));
    rc = accum_err(rc, cmn_togglebutton_set_toggled(tb2, CMN_FALSE));
    rc = accum_err(rc, cmn_widget_set_bounds(tb1, rtb));
    rc = accum_err(rc, cmn_widget_set_bounds(tb2, rtb));
    rc = accum_err(rc, cmn_flex_add_child(flex, tb1, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, tb2, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_togglebutton_destroy(&tb2));
    rc = accum_err(rc, cmn_togglebutton_destroy(&tb1));
    break;
  }
  case 7: { /* line */
    cmn_line_t *ln1 = NULL, *ln2 = NULL;
    cmn_rect_t rln = {0, 0, 432, 16};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native Separator Line (cmn_line_t)"));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_hdr, "Section 1: Network Configuration"));
    rc = accum_err(rc, cmn_line_create_horizontal(&ln1));
    rc =
        accum_err(rc, cmn_label_set_text(
                          lbl_sub, "Section 2: Storage & Encryption Settings"));
    rc = accum_err(rc, cmn_line_create_horizontal(&ln2));
    rc = accum_err(rc, cmn_widget_set_bounds(ln1, rln));
    rc = accum_err(rc, cmn_widget_set_bounds(ln2, rln));
    rc = accum_err(rc, cmn_flex_add_child(flex, ln1, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, ln2, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_line_destroy(&ln2));
    rc = accum_err(rc, cmn_line_destroy(&ln1));
    break;
  }
  case 8: { /* slider */
    cmn_slider_t *sl1 = NULL, *sl2 = NULL;
    cmn_rect_t rsl = {0, 0, 432, 32};
    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Native Slider (cmn_slider_t)"));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_hdr, "Continuous & Discrete Trackbar Sliders "
                                    "(cmn_slider_t)"));
    rc = accum_err(rc, cmn_slider_create(&sl1));
    rc = accum_err(rc, cmn_slider_create(&sl2));
    rc = accum_err(rc, cmn_slider_set_value(sl1, 0.72));
    rc = accum_err(rc, cmn_slider_set_steps(sl2, 10));
    rc = accum_err(rc, cmn_slider_set_value(sl2, 0.40));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_sub,
                               "Audio Gain: 72%   |   Thread Pool Scale: 40%"));
    rc = accum_err(rc, cmn_widget_set_bounds(sl1, rsl));
    rc = accum_err(rc, cmn_widget_set_bounds(sl2, rsl));
    rc = accum_err(rc, cmn_flex_add_child(flex, sl1, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, sl2, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_slider_destroy(&sl2));
    rc = accum_err(rc, cmn_slider_destroy(&sl1));
    break;
  }
  case 9: { /* progress */
    cmn_progress_t *pr1 = NULL, *pr2 = NULL;
    cmn_rect_t rpr = {0, 0, 432, 28};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native Progress Bar (cmn_progress_t)"));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_hdr, "Progress Indicators (cmn_progress_t)"));
    rc = accum_err(rc, cmn_progress_create(&pr1));
    rc = accum_err(rc, cmn_progress_create(&pr2));
    rc = accum_err(rc, cmn_progress_set_value(pr1, 0.68));
    rc = accum_err(rc, cmn_progress_set_value(pr2, 0.90));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_sub,
                               "Compiling C89 Targets: 68%  |  Linking Assets: "
                               "90%"));
    rc = accum_err(rc, cmn_widget_set_bounds(pr1, rpr));
    rc = accum_err(rc, cmn_widget_set_bounds(pr2, rpr));
    rc = accum_err(rc, cmn_flex_add_child(flex, pr1, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, pr2, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_progress_destroy(&pr2));
    rc = accum_err(rc, cmn_progress_destroy(&pr1));
    break;
  }
  case 10: { /* updown */
    cmn_flex_t *row = NULL;
    cmn_textbox_t *tb_val = NULL;
    cmn_updown_t *ud = NULL;
    cmn_rect_t rrow = {0, 0, 260, 32};
    cmn_rect_t rtb = {0, 0, 140, 28};
    cmn_rect_t rud = {0, 0, 80, 28};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native UpDown Stepper (cmn_updown_t)"));
    rc = accum_err(
        rc, cmn_label_set_text(
                lbl_hdr, "Numeric Up/Down Stepper Control (cmn_updown_t)"));
    rc = accum_err(rc, cmn_flex_create(&row));
    rc = accum_err(rc, cmn_flex_set_direction(row, CMN_FLEX_ROW));
    rc = accum_err(rc, cmn_flex_set_align_items(row, CMN_ALIGN_CENTER));
    rc = accum_err(rc, cmn_flex_set_gap(row, 8, 8));
    rc = accum_err(rc, cmn_textbox_create(&tb_val));
    rc = accum_err(rc, cmn_textbox_set_text(tb_val, "45.0"));
    rc = accum_err(rc, cmn_updown_create(&ud));
    rc = accum_err(rc, cmn_updown_set_step(ud, 5.0));
    rc = accum_err(rc, cmn_updown_set_value(ud, 45.0));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_sub,
                               "Current Stepper Value: 45.0 (Step Increment: "
                               "5.0)"));
    rc = accum_err(rc, cmn_widget_set_bounds(row, rrow));
    rc = accum_err(rc, cmn_widget_set_bounds(tb_val, rtb));
    rc = accum_err(rc, cmn_widget_set_bounds(ud, rud));
    rc = accum_err(rc, cmn_flex_add_child(row, tb_val, NULL));
    rc = accum_err(rc, cmn_flex_add_child(row, ud, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, row, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_updown_destroy(&ud));
    rc = accum_err(rc, cmn_textbox_destroy(&tb_val));
    rc = accum_err(rc, cmn_flex_destroy(&row));
    break;
  }
  case 11: { /* popup */
    cmn_popup_t *pu = NULL;
    cmn_rect_t rpu = {0, 0, 340, 32};
    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Native PopUp Menu (cmn_popup_t)"));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_hdr, "Drop-Down Selection PopUp (cmn_popup_t)"));
    rc = accum_err(rc, cmn_popup_create(&pu));
    rc = accum_err(rc, cmn_popup_add_item(pu, "macOS AppKit (Cocoa)"));
    rc = accum_err(rc, cmn_popup_add_item(pu, "Linux GTK+ 3"));
    rc = accum_err(rc, cmn_popup_add_item(pu, "Windows Win32 (ComCtl32)"));
    rc = accum_err(rc, cmn_popup_add_item(pu, "WebAssembly (HTML5 DOM)"));
    rc = accum_err(rc, cmn_popup_set_selected(pu, 0));
    rc = accum_err(
        rc, cmn_label_set_text(
                lbl_sub, "Selected Target Backend: macOS AppKit (Cocoa)"));
    rc = accum_err(rc, cmn_widget_set_bounds(pu, rpu));
    rc = accum_err(rc, cmn_flex_add_child(flex, pu, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_popup_destroy(&pu));
    break;
  }
  case 12: { /* combo */
    cmn_combo_t *co = NULL;
    cmn_rect_t rco = {0, 0, 360, 32};
    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Native ComboBox (cmn_combo_t)"));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_hdr, "Editable ComboBox Control (cmn_combo_t)"));
    rc = accum_err(rc, cmn_combo_create(&co));
    rc = accum_err(rc, cmn_combo_add_item(co, "sqlite://local_cache.db"));
    rc = accum_err(rc, cmn_combo_add_item(co, "postgres://localhost:5432/app"));
    rc = accum_err(rc, cmn_combo_set_text(co, "sqlite://local_cache.db"));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_sub, "Allows custom text entry or preset item "
                                    "selection"));
    rc = accum_err(rc, cmn_widget_set_bounds(co, rco));
    rc = accum_err(rc, cmn_flex_add_child(flex, co, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_combo_destroy(&co));
    break;
  }
  case 13: { /* listbox */
    cmn_listbox_t *lb = NULL;
    cmn_rect_t rlb = {0, 0, 432, 180};
    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Native ListBox (cmn_listbox_t)"));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_hdr,
                               "Scrollable Checkable ListBox (cmn_listbox_t)"));
    rc = accum_err(rc, cmn_listbox_create(&lb));
    rc = accum_err(rc, cmn_listbox_set_checkable(lb, CMN_TRUE));
    rc =
        accum_err(rc, cmn_listbox_add_item(
                          lb, "Core Signals & Reactive State (cmn/signals.h)"));
    rc = accum_err(rc,
                   cmn_listbox_add_item(
                       lb, "CSS Flexbox & Grid Layout Engine (cmn/layout.h)"));
    rc = accum_err(rc, cmn_listbox_add_item(
                           lb, "Room SQLite ORM Persistence (cmn/room.h)"));
    rc = accum_err(
        rc, cmn_listbox_add_item(lb, "UI Automation & Screenshot Testing "
                                     "(cmn/automation.h)"));
    rc = accum_err(rc, cmn_listbox_set_checked(lb, 0, CMN_TRUE));
    rc = accum_err(rc, cmn_listbox_set_checked(lb, 1, CMN_TRUE));
    rc = accum_err(rc, cmn_listbox_set_checked(lb, 3, CMN_TRUE));
    rc = accum_err(rc, cmn_listbox_set_selected(lb, 1));
    rc = accum_err(rc, cmn_widget_set_bounds(lb, rlb));
    rc = accum_err(rc, cmn_flex_add_child(flex, lb, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_listbox_destroy(&lb));
    break;
  }
  case 14: { /* imageview */
    cmn_imageview_t *iv = NULL;
    cmn_rect_t riv = {0, 0, 260, 160};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native ImageView (cmn_imageview_t)"));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_hdr, "Native Raster & Vector Image Display "
                                        "(cmn_imageview_t)"));
    rc = accum_err(rc, cmn_imageview_create(&iv));
    rc = accum_err(rc, cmn_imageview_set_scale(iv, CMN_SCALE_ASPECT));
    rc = accum_err(
        rc, cmn_label_set_text(
                lbl_sub, "Scale Mode: CMN_SCALE_ASPECT (160x100 Preview)"));
    rc = accum_err(rc, cmn_widget_set_bounds(iv, riv));
    rc = accum_err(rc, cmn_flex_add_child(flex, iv, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_imageview_destroy(&iv));
    break;
  }
  case 15: { /* tableview */
    cmn_tableview_t *tbl = NULL;
    cmn_rect_t rtbl = {0, 0, 432, 190};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native TableView (cmn_tableview_t)"));
    rc = accum_err(
        rc, cmn_label_set_text(
                lbl_hdr, "Multi-Column Data TableView (cmn_tableview_t)"));
    rc = accum_err(rc, cmn_tableview_create(&tbl));
    rc = accum_err(rc, cmn_tableview_add_column(tbl, "ID", 60));
    rc = accum_err(rc, cmn_tableview_add_column(tbl, "Module", 180));
    rc = accum_err(rc, cmn_tableview_add_column(tbl, "Status", 120));
    rc = accum_err(rc, cmn_tableview_set_row_count(tbl, 3));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 0, 0, "101"));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 0, 1, "auth_service"));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 0, 2, "Running"));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 1, 0, "102"));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 1, 1, "sqlite_sync"));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 1, 2, "Synced"));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 2, 0, "103"));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 2, 1, "ui_renderer"));
    rc = accum_err(rc, cmn_tableview_set_cell_text(tbl, 2, 2, "Active"));
    rc = accum_err(rc, cmn_tableview_set_selected_row(tbl, 1));
    rc = accum_err(rc, cmn_widget_set_bounds(tbl, rtbl));
    rc = accum_err(rc, cmn_flex_add_child(flex, tbl, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_tableview_destroy(&tbl));
    break;
  }
  case 16: { /* view */
    cmn_view_t *cv = NULL;
    cmn_rect_t rcv = {0, 0, 432, 180};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Custom 2D Drawing View (cmn_view_t)"));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_hdr, "Custom 2D Drawing Canvas (cmn_view_t)"));
    rc = accum_err(rc, cmn_view_create(&cv));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_sub,
                               "Supports custom paint, mouse, wheel & keyboard "
                               "callbacks"));
    rc = accum_err(rc, cmn_widget_set_bounds(cv, rcv));
    rc = accum_err(rc, cmn_flex_add_child(flex, cv, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_view_destroy(&cv));
    break;
  }
  case 17: { /* webview */
    cmn_webview_t *wv = NULL;
    cmn_rect_t rwv = {0, 0, 432, 200};
    rc = accum_err(
        rc, cmn_window_set_title(win, "cmn Embedded WebView (cmn_webview_t)"));
    rc =
        accum_err(rc, cmn_label_set_text(
                          lbl_hdr, "Embedded Browser WebView (cmn_webview_t)"));
    rc = accum_err(rc, cmn_webview_create(&wv));
    rc =
        accum_err(rc, cmn_webview_navigate(wv, "https://github.com/SamuelMarks/"
                                               "c-multiplatform-native"));
    rc = accum_err(rc, cmn_webview_load_html(
                           wv, "Interactive HTML5/JS bridge inside native C89 "
                               "window."));
    rc = accum_err(rc, cmn_widget_set_bounds(wv, rwv));
    rc = accum_err(rc, cmn_flex_add_child(flex, wv, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_webview_destroy(&wv));
    break;
  }
  case 18: { /* panel */
    cmn_panel_t *pnl = NULL;
    cmn_rect_t rpnl = {0, 0, 432, 180};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native Panel Container (cmn_panel_t)"));
    rc = accum_err(
        rc, cmn_label_set_text(lbl_hdr,
                               "Grouped Panel Sub-Container (cmn_panel_t)"));
    rc = accum_err(rc, cmn_panel_create_scroll(&pnl));
    rc = accum_err(rc, cmn_label_set_text(
                           lbl_sub, "Supports bordered grouping and scrollable "
                                    "viewports"));
    rc = accum_err(rc, cmn_widget_set_bounds(pnl, rpnl));
    rc = accum_err(rc, cmn_flex_add_child(flex, pnl, NULL));
    rc = accum_err(rc, cmn_flex_add_child(flex, lbl_sub, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_panel_destroy(&pnl));
    break;
  }
  case 19: { /* tabs */
    cmn_tabs_t *tabs = NULL;
    cmn_label_t *page0 = NULL, *page1 = NULL;
    cmn_rect_t rtabs = {0, 0, 432, 200};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native Tabbed Container (cmn_tabs_t)"));
    rc =
        accum_err(rc, cmn_label_set_text(
                          lbl_hdr, "Multi-Page Tabbed Container (cmn_tabs_t)"));
    rc = accum_err(rc, cmn_tabs_create(&tabs));
    rc = accum_err(rc, cmn_label_create(&page0));
    rc = accum_err(rc, cmn_label_create(&page1));
    rc = accum_err(rc, cmn_label_set_text(
                           page0, "General Workspace Settings & Preferences"));
    rc = accum_err(
        rc, cmn_label_set_text(page1, "Database Connection Pool & Sync Rules"));
    rc = accum_err(rc, cmn_tabs_add_tab(tabs, "General", page0));
    rc = accum_err(rc, cmn_tabs_add_tab(tabs, "Database", page1));
    rc = accum_err(rc, cmn_tabs_add_tab(tabs, "Advanced", NULL));
    rc = accum_err(rc, cmn_tabs_set_selected(tabs, 0));
    rc = accum_err(rc, cmn_widget_set_bounds(tabs, rtabs));
    rc = accum_err(rc, cmn_flex_add_child(flex, tabs, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_tabs_destroy(&tabs));
    rc = accum_err(rc, cmn_label_destroy(&page1));
    rc = accum_err(rc, cmn_label_destroy(&page0));
    break;
  }
  default: { /* 20: splitview */
    cmn_splitview_t *sv = NULL;
    cmn_textbox_t *left_pane = NULL;
    cmn_textview_t *right_pane = NULL;
    cmn_rect_t rsv = {0, 0, 432, 200};
    rc = accum_err(rc, cmn_window_set_title(
                           win, "cmn Native SplitView (cmn_splitview_t)"));
    rc = accum_err(
        rc, cmn_label_set_text(
                lbl_hdr, "Resizable SplitView Divider (cmn_splitview_t)"));
    rc = accum_err(rc, cmn_splitview_create_vertical(&sv));
    rc = accum_err(rc, cmn_textbox_create(&left_pane));
    rc = accum_err(rc, cmn_textview_create(&right_pane));
    rc = accum_err(rc, cmn_textbox_set_text(left_pane, "Navigation Tree"));
    rc = accum_err(
        rc, cmn_textview_set_text(
                right_pane, "Detail Inspector Pane\nSplit Ratio: 45% / 55%"));
    rc = accum_err(rc, cmn_splitview_set_children(sv, left_pane, right_pane));
    rc = accum_err(rc, cmn_splitview_set_pos(sv, 0.45));
    rc = accum_err(rc, cmn_widget_set_bounds(sv, rsv));
    rc = accum_err(rc, cmn_flex_add_child(flex, sv, NULL));
    rc = accum_err(rc, cmn_window_set_layout(win, flex));
    rc = accum_err(rc, cmn_window_set_size(win, 480, 360));
    rc = accum_err(rc, cmn_window_show(win));
    rc = accum_err(rc, cmn_automation_capture_screenshot(win, filepath));
    rc = accum_err(rc, cmn_splitview_destroy(&sv));
    rc = accum_err(rc, cmn_textview_destroy(&right_pane));
    rc = accum_err(rc, cmn_textbox_destroy(&left_pane));
    break;
  }
  }

  rc = accum_err(rc, cmn_label_destroy(&lbl_sub));
  rc = accum_err(rc, cmn_label_destroy(&lbl_hdr));
  rc = accum_err(rc, cmn_flex_destroy(&flex));
  rc = accum_err(rc, cmn_window_destroy(&win));
  rc = accum_err(rc, cmn_app_destroy(&app));
  return rc;
}

/**
 * @brief Writes the HTML5 DOM document corresponding to a `cmn_web_widget.c`
 * scene (`0`..`CMN_SCREENSHOT_SCENE_COUNT - 1`) so a real headless browser can
 * render and screenshot the exact Web DOM widgets.
 * @param scene_id Scene index (`0`..`CMN_SCREENSHOT_SCENE_COUNT - 1`).
 * @param html_filepath Destination `.html` file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_write_web_html(int scene_id,
                                          const char *html_filepath) {
  static const char *const prefix_a =
      "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><style>"
      "body{margin:0;padding:20px 24px;box-sizing:border-box;width:480px;"
      "height:360px;font-family:system-ui,-apple-system,sans-serif;"
      "font-size:14px;background:#fff;color:#111827;}"
      ".fcol{display:flex;flex-direction:column;gap:14px;width:100%;}"
      ".chk{display:flex;align-items:center;gap:8px;}";
  static const char *const prefix_b =
      ".frow{display:flex;flex-direction:row;gap:16px;height:34px;}"
      ".grd{display:grid;grid-template-columns:1fr;gap:10px;width:100%;}"
      ".card{border:1px solid #9ca3af;box-sizing:border-box;}"
      ".tbar{display:flex;background:#f3f4f6;border-bottom:1px solid #d1d5db;}"
      ".tab{padding:7px 14px;font-size:12px;border-right:1px solid #d1d5db;}"
      "input[type=text],input[type=password],textarea,select{width:100%;box-"
      "sizing:border-box;padding:5px 8px;}"
      "button{height:32px;}</style></head><body>";
  static const char *const bodies[CMN_SCREENSHOT_SCENE_COUNT] = {
      /* 0: button */
      "<div class=\"fcol\"><span>Native OS Push Buttons (cmn_button_t)</span>"
      "<div class=\"frow\"><button style=\"flex:1\">Save Changes</button>"
      "<button style=\"flex:1\">Cancel</button></div>"
      "<button>Trigger Action (Click Me)</button>"
      "<span>Status: Button clicked 3 times</span></div>",
      /* 1: label */
      "<div class=\"fcol\"><span>Native OS Text Labels (cmn_label_t)</span>"
      "<span>Primary Heading: Cross-Platform C89 Native UI</span>"
      "<span>Multiline &amp; Ellipsis-Aware Typography Control</span>"
      "<span>Status: UTF-8 &amp; i18n Ready</span></div>",
      /* 2: textbox */
      "<div class=\"fcol\"><span>Single-Line Text &amp; Password Inputs "
      "(cmn_textbox_t)</span>"
      "<input type=\"text\" value=\"https://api.cmn.dev/v1/sync\">"
      "<input type=\"password\" value=\"super_secret_token\">"
      "<span>Supports plain text, password masking &amp; "
      "selection</span></div>",
      /* 3: textview */
      "<div class=\"fcol\"><span>Multi-Line Scrollable Text Editor "
      "(cmn_textview_t)</span>"
      "<textarea readonly "
      "style=\"height:210px;resize:none;overflow-y:scroll;font-family:"
      "monospace\">"
      "[INFO] Initialized cmn v0.1.0 runtime&#10;"
      "[INFO] Loaded SQLite Room database schema&#10;"
      "[SYNC] Fetched 24 records from remote endpoint&#10;"
      "[A11Y] Attached native accessibility tree roles</textarea></div>",
      /* 4: checkbox */
      "<div class=\"fcol\"><span>Two-State &amp; Tri-State Checkboxes "
      "(cmn_checkbox_t)</span>"
      "<label class=\"chk\"><input type=\"checkbox\" checked> Enable automatic "
      "background database sync</label>"
      "<label class=\"chk\"><input type=\"checkbox\"> Send anonymous crash "
      "diagnostics</label>"
      "<label class=\"chk\"><input type=\"checkbox\" id=\"cbm\"> Select all "
      "workspace modules (Tri-State Mixed)</label>"
      "<script>document.getElementById('cbm').indeterminate=true;</script></"
      "div>",
      /* 5: radiobutton */
      "<div class=\"fcol\"><span>Mutually Exclusive Radio Buttons "
      "(cmn_radiobutton_t)</span>"
      "<label class=\"chk\"><input type=\"radio\" name=\"r\" checked> Release "
      "Mode (-O3, LTO Enabled)</label>"
      "<label class=\"chk\"><input type=\"radio\" name=\"r\"> Debug Mode (-g, "
      "AddressSanitizer)</label>"
      "<label class=\"chk\"><input type=\"radio\" name=\"r\"> Minimum Size "
      "(-Os, Strip Symbols)</label></div>",
      /* 6: togglebutton */
      "<div class=\"fcol\"><span>Latching Push Toggle Buttons "
      "(cmn_togglebutton_t)</span>"
      "<button style=\"width:260px;background:#2563eb;color:#fff;border:1px "
      "solid #1d4ed8;border-radius:4px;font-weight:600\">Live Telemetry "
      "Stream: ON</button>"
      "<button style=\"width:260px;background:#f3f4f6;color:#111827;border:1px "
      "solid #9ca3af;border-radius:4px\">Offline Cache Mode: "
      "OFF</button></div>",
      /* 7: line */
      "<div class=\"fcol\"><span>Section 1: Network Configuration</span>"
      "<hr style=\"width:100%;border:none;border-top:1px solid #9ca3af\">"
      "<span>Section 2: Storage &amp; Encryption Settings</span>"
      "<hr style=\"width:100%;border:none;border-top:1px solid "
      "#9ca3af\"></div>",
      /* 8: slider */
      "<div class=\"fcol\"><span>Continuous &amp; Discrete Trackbar Sliders "
      "(cmn_slider_t)</span>"
      "<input type=\"range\" min=\"0\" max=\"100\" value=\"72\">"
      "<input type=\"range\" min=\"0\" max=\"100\" step=\"10\" value=\"40\">"
      "<span>Audio Gain: 72%   |   Thread Pool Scale: 40%</span></div>",
      /* 9: progress */
      "<div class=\"fcol\"><span>Progress Indicators (cmn_progress_t)</span>"
      "<progress max=\"100\" value=\"68\" "
      "style=\"width:100%;height:24px\"></progress>"
      "<progress max=\"100\" value=\"90\" "
      "style=\"width:100%;height:24px\"></progress>"
      "<span>Compiling C89 Targets: 68%  |  Linking Assets: 90%</span></div>",
      /* 10: updown */
      "<div class=\"fcol\"><span>Numeric Up/Down Stepper Control "
      "(cmn_updown_t)</span>"
      "<input type=\"number\" value=\"45\" step=\"5\" "
      "style=\"width:220px;padding:6px\">"
      "<span>Current Stepper Value: 45.0 (Step Increment: 5.0)</span></div>",
      /* 11: popup */
      "<div class=\"fcol\"><span>Drop-Down Selection PopUp (cmn_popup_t)</span>"
      "<select style=\"width:340px\"><option selected>macOS AppKit "
      "(Cocoa)</option>"
      "<option>Linux GTK+ 3</option><option>Windows Win32 (ComCtl32)</option>"
      "<option>WebAssembly (HTML5 DOM)</option></select>"
      "<span>Selected Target Backend: macOS AppKit (Cocoa)</span></div>",
      /* 12: combo */
      "<div class=\"fcol\"><span>Editable ComboBox Control (cmn_combo_t)</span>"
      "<input type=\"text\" value=\"sqlite://local_cache.db\" "
      "style=\"width:360px\">"
      "<span>Allows custom text entry or preset item selection</span></div>",
      /* 13: listbox */
      "<div class=\"fcol\"><span>Scrollable Checkable ListBox "
      "(cmn_listbox_t)</span>"
      "<select multiple size=\"6\" "
      "style=\"height:180px;font-family:monospace\">"
      "<option>[x] Core Signals &amp; Reactive State (cmn/signals.h)</option>"
      "<option selected>[x] CSS Flexbox &amp; Grid Layout Engine "
      "(cmn/layout.h)</option>"
      "<option>[ ] Room SQLite ORM Persistence (cmn/room.h)</option>"
      "<option>[x] UI Automation &amp; Screenshot Testing "
      "(cmn/automation.h)</option></select></div>",
      /* 14: imageview */
      "<div class=\"fcol\"><span>Native Raster &amp; Vector Image Display "
      "(cmn_imageview_t)</span>"
      "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"260\" height=\"160\" "
      "class=\"card\">"
      "<rect width=\"260\" height=\"160\" fill=\"#2e70c7\"/>"
      "<circle cx=\"200\" cy=\"44\" r=\"22\" fill=\"#f2c238\"/>"
      "<rect y=\"110\" width=\"260\" height=\"50\" fill=\"#38a873\"/></svg>"
      "<span>Scale Mode: CMN_SCALE_ASPECT (160x100 Preview)</span></div>",
      /* 15: tableview */
      "<div class=\"fcol\"><span>Multi-Column Data TableView "
      "(cmn_tableview_t)</span>"
      "<div class=\"card\" "
      "style=\"padding:8px;font-family:monospace;white-space:pre;height:"
      "190px\">"
      "ID              Module          Status          \n"
      "------------------------------------------------\n"
      "  101             auth_service    Running         \n"
      "&gt; 102             sqlite_sync     Synced          \n"
      "  103             ui_renderer     Active          </div></div>",
      /* 16: view */
      "<div class=\"fcol\"><span>Custom 2D Drawing Canvas (cmn_view_t)</span>"
      "<svg width=\"432\" height=\"180\" class=\"card\" "
      "style=\"background:#f5f8fd\">"
      "<rect x=\"20\" y=\"20\" width=\"120\" height=\"44\" fill=\"#337adb\"/>"
      "<circle cx=\"186\" cy=\"42\" r=\"26\" fill=\"#29a66b\"/>"
      "<line x1=\"230\" y1=\"64\" x2=\"360\" y2=\"20\" stroke=\"#e05240\" "
      "stroke-width=\"3\"/>"
      "<text x=\"20\" y=\"156\" font-weight=\"bold\" fill=\"#232a34\">2D "
      "Vector Canvas (cmn_view_t)</text></svg>"
      "<span>Supports custom paint &amp; input callbacks</span></div>",
      /* 17: webview */
      "<div class=\"fcol\"><span>Embedded Browser WebView "
      "(cmn_webview_t)</span>"
      "<div class=\"card\" style=\"height:200px\">"
      "<div style=\"background:#edf0f5;padding:6px "
      "10px;font-size:11px;border-bottom:1px solid #d1d5db\">"
      "URL: https://github.com/SamuelMarks/c-multiplatform-native</div>"
      "<div style=\"padding:12px 14px\">"
      "<div style=\"font-weight:bold;color:#1c529e;margin-bottom:8px\">cmn "
      "Embedded WebView Document</div>"
      "<div>Interactive HTML5/JS bridge inside native C89 "
      "window.</div></div></div></div>",
      /* 18: panel */
      "<div class=\"fcol\"><span>Grouped Panel Sub-Container "
      "(cmn_panel_t)</span>"
      "<div class=\"card\" "
      "style=\"border-radius:6px;background:#f9fafb;padding:12px;height:"
      "180px\">"
      "<div style=\"font-weight:600;font-size:12px;color:#374151\">Panel "
      "Container (cmn_panel_t)</div></div>"
      "<span>Supports bordered grouping and scrollable viewports</span></div>",
      /* 19: tabs */
      "<div class=\"fcol\"><span>Multi-Page Tabbed Container "
      "(cmn_tabs_t)</span>"
      "<div class=\"card\" style=\"border-radius:6px;height:200px\">"
      "<div class=\"tbar\"><div class=\"tab\" "
      "style=\"font-weight:600;background:#fff\">General</div>"
      "<div class=\"tab\">Database</div><div class=\"tab\">Advanced</div></div>"
      "<div style=\"padding:16px\">General Workspace Settings &amp; "
      "Preferences</div></div></div>",
      /* 20: splitview */
      "<div class=\"fcol\"><span>Resizable SplitView Divider "
      "(cmn_splitview_t)</span>"
      "<div class=\"card\" style=\"display:flex;height:200px\">"
      "<div style=\"width:45%;padding:12px;background:#f8fafc;border-right:4px "
      "solid #cbd5e1\">Navigation Tree</div>"
      "<div "
      "style=\"flex:1;padding:12px;background:#f1f5f9;white-space:pre-line\">"
      "Detail Inspector Pane&#10;Split Ratio: 45% / 55%</div></div></div>",
      /* 21: inputs */
      "<div class=\"grd\"><span>Search Filter (cmn_textbox_t):</span>"
      "<input type=\"text\" value=\"SELECT * FROM tasks WHERE id = 1\">"
      "<span>Event Log (Scrollable cmn_textview_t):</span>"
      "<textarea readonly style=\"height:170px;resize:none;overflow-y:scroll\">"
      "[01] Signal initialized (val = 0)&#10;"
      "[02] Route /login -&gt; /welcome&#10;"
      "[03] Room DAO synced 12 SQLite rows&#10;</textarea></div>",
      /* 22: hello_world */
      "<div class=\"fcol\"><span>Hello!, I'm a label</span>"
      "<button style=\"width:160px\">Click Me!</button>"
      "<textarea readonly style=\"height:190px;resize:none;overflow-y:scroll\">"
      "Button click (1)&#10;Button click (2)&#10;"
      "Button click (3)&#10;Button click (4)&#10;</textarea></div>",
      /* 23: password_flow */
      "<div class=\"grd\"><span>Username:</span>"
      "<input type=\"text\" value=\"samuel_marks\">"
      "<span>Password (min 4 chars):</span>"
      "<input type=\"password\" value=\"correct_horse_battery\">"
      "<span>Form Status: Valid (Ready)</span>"
      "<button style=\"width:180px\">Sign in / Sign up</button></div>"};
  static const char *const suffix = "</body></html>";
  FILE *fp = NULL;

  if (scene_id < 0 || scene_id >= CMN_SCREENSHOT_SCENE_COUNT ||
      html_filepath == NULL || html_filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, html_filepath, "wb") != 0 || fp == NULL) {
    return CMN_ERR_IO;
  }
#else
  fp = fopen(html_filepath, "wb");
  if (fp == NULL) {
    return CMN_ERR_IO;
  }
#endif
  if (fwrite(prefix_a, 1, strlen(prefix_a), fp) == 0 ||
      fwrite(prefix_b, 1, strlen(prefix_b), fp) == 0 ||
      fwrite(bodies[scene_id], 1, strlen(bodies[scene_id]), fp) == 0 ||
      fwrite(suffix, 1, strlen(suffix), fp) == 0) {
    fclose(fp);
    return CMN_ERR_IO;
  }
  fclose(fp);
  return CMN_SUCCESS;
}

/**
 * @brief Captures all real native widget and application screenshots for the
 * current host OS into `output_dir` using `_<platform>.png` filenames (or
 * `platform_override` when non-NULL).
 * @param output_dir Directory path where PNG files will be written.
 * @param platform_override Optional platform suffix override (e.g., `"macos"`,
 * `"windows"`, `"linux"`, `"web"`), or `NULL` to auto-detect the host OS.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_generate_all(const char *output_dir,
                                        const char *platform_override) {
  const char *plat = platform_override;
  char path_buf[1024];
  cmn_error_e rc = CMN_SUCCESS;
  int i;

  if (output_dir == NULL || output_dir[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  if (plat == NULL || plat[0] == '\0') {
    rc = cmn_screenshot_get_platform_suffix(&plat);
  }

#if defined(_WIN32)
  if (_mkdir(output_dir) != 0) {
    rc = CMN_SUCCESS;
  }
#else
  {
    cfs_path dir_path;
    cfs_error_code cfs_rc;
    memset(&cfs_rc, 0, sizeof(cfs_rc));
    if (cfs_path_init_str(&dir_path, output_dir) == 0) {
      if (cfs_create_directories(&dir_path, &cfs_rc) != 0) {
        memset(&cfs_rc, 0, sizeof(cfs_rc));
      }
      cfs_path_destroy(&dir_path);
    }
  }
#endif

  for (i = 0; i < CMN_SCREENSHOT_SCENE_COUNT; ++i) {
    const char *name = NULL;
    rc = accum_err(rc, cmn_screenshot_get_scene_name(i, &name));
    rc = accum_err(rc, cmn_sprintf_s(path_buf, sizeof(path_buf), "%s/%s_%s.png",
                                     output_dir, name, plat));
    rc = accum_err(rc, cmn_screenshot_capture_scene(i, path_buf));
    if (rc != CMN_SUCCESS) {
      break;
    }
  }

  return rc;
}
