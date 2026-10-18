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

    rc = cmn_window_set_title(win, "cmn Native Buttons (cmn_button_t)");
    rc = cmn_flex_set_direction(root_flex, CMN_FLEX_COLUMN);
    rc = cmn_flex_set_align_items(root_flex, CMN_ALIGN_STRETCH);
    rc = cmn_flex_set_gap(root_flex, 16, 16);
    rc = cmn_flex_set_padding(root_flex, pad);

    rc = cmn_flex_set_direction(row_flex, CMN_FLEX_ROW);
    rc = cmn_flex_set_justify_content(row_flex, CMN_JUSTIFY_FLEX_START);
    rc = cmn_flex_set_align_items(row_flex, CMN_ALIGN_CENTER);
    rc = cmn_flex_set_gap(row_flex, 12, 16);
    rc = cmn_flex_add_breakpoint(row_flex, 320, CMN_FLEX_COLUMN,
                                 CMN_FLEX_NOWRAP);

    rc =
        cmn_label_set_text(lbl_header, "Native OS Push Buttons (cmn_button_t)");
    rc = cmn_button_set_text(btn_primary, "Save Changes");
    rc = cmn_button_set_text(btn_secondary, "Cancel");
    rc = cmn_button_set_text(btn_counter, "Trigger Action (Click Me)");
    rc = cmn_button_set_on_click(btn_counter, on_showcase_btn_click, &clicks);

    rc = cmn_widget_set_bounds(lbl_header, r_hdr);
    rc = cmn_widget_set_bounds(row_flex, r_row);
    rc = cmn_widget_set_bounds(btn_primary, r_btn);
    rc = cmn_widget_set_bounds(btn_secondary, r_btn);
    rc = cmn_widget_set_bounds(btn_counter, r_act);
    rc = cmn_widget_set_bounds(lbl_status, r_stat);

    rc = cmn_flex_item_style_init(&grow_btn);
    grow_btn.flex_grow = 1.0f;
    grow_btn.flex_basis = 160;
    rc = cmn_flex_add_child(row_flex, btn_primary, &grow_btn);
    rc = cmn_flex_add_child(row_flex, btn_secondary, &grow_btn);
    rc = cmn_flex_add_child(root_flex, lbl_header, NULL);
    rc = cmn_flex_add_child(root_flex, row_flex, NULL);
    rc = cmn_flex_add_child(root_flex, btn_counter, NULL);
    rc = cmn_flex_add_child(root_flex, lbl_status, NULL);
    rc = cmn_window_set_layout(win, root_flex);
    rc = cmn_window_set_size(win, 480, 360);

    rc = cmn_automation_click(btn_counter);
    rc = cmn_automation_click(btn_counter);
    rc = cmn_automation_click(btn_counter);
    rc =
        cmn_sprintf_s(status_buf, sizeof(status_buf),
                      "Status: Button clicked " CMN_INT32_FMT " times", clicks);
    rc = cmn_label_set_text(lbl_status, status_buf);
    rc = cmn_window_show(win);
    rc = cmn_automation_capture_screenshot(win, filepath);
  }

  if (lbl_status != NULL) {
    (void)cmn_label_destroy(&lbl_status);
  }
  if (btn_counter != NULL) {
    (void)cmn_button_destroy(&btn_counter);
  }
  if (btn_secondary != NULL) {
    (void)cmn_button_destroy(&btn_secondary);
  }
  if (btn_primary != NULL) {
    (void)cmn_button_destroy(&btn_primary);
  }
  if (lbl_header != NULL) {
    (void)cmn_label_destroy(&lbl_header);
  }
  if (row_flex != NULL) {
    (void)cmn_flex_destroy(&row_flex);
  }
  if (root_flex != NULL) {
    (void)cmn_flex_destroy(&root_flex);
  }
  if (win != NULL) {
    (void)cmn_window_destroy(&win);
  }
  if (app != NULL) {
    (void)cmn_app_destroy(&app);
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

    rc = cmn_grid_set_columns(grid, &col_track, 1);
    rc = cmn_grid_set_rows(grid, row_tracks, 4);
    rc = cmn_grid_set_gap(grid, 10, 10);
    rc = cmn_grid_set_padding(grid, pad);

    rc = cmn_window_set_title(win,
                              "cmn Native Inputs (Label, TextBox, TextView)");
    rc = cmn_label_set_text(lbl_search, "Search Filter (cmn_textbox_t):");
    rc = cmn_automation_type_text(tb_query, "SELECT * FROM tasks WHERE id = 1",
                                  0);
    rc = cmn_label_set_text(lbl_log, "Event Log (Scrollable cmn_textview_t):");
    rc = cmn_textview_set_text(tv_log, "");
    rc = cmn_sprintf_s(log_line, sizeof(log_line),
                       "[01] Signal initialized (val = 0)%c", 10);
    rc = cmn_textview_append_text(tv_log, log_line);
    rc = cmn_sprintf_s(log_line, sizeof(log_line),
                       "[02] Route /login -> /welcome%c", 10);
    rc = cmn_textview_append_text(tv_log, log_line);
    rc = cmn_sprintf_s(log_line, sizeof(log_line),
                       "[03] Room DAO synced 12 SQLite rows%c", 10);
    rc = cmn_textview_append_text(tv_log, log_line);

    rc = cmn_grid_add_child(grid, lbl_search, NULL);
    rc = cmn_grid_add_child(grid, tb_query, NULL);
    rc = cmn_grid_add_child(grid, lbl_log, NULL);
    rc = cmn_grid_add_child(grid, tv_log, NULL);
    rc = cmn_window_set_layout(win, grid);
    rc = cmn_window_set_size(win, 480, 360);

    rc = cmn_window_show(win);
    rc = cmn_automation_capture_screenshot(win, filepath);
  }

  if (tv_log != NULL) {
    (void)cmn_textview_destroy(&tv_log);
  }
  if (lbl_log != NULL) {
    (void)cmn_label_destroy(&lbl_log);
  }
  if (tb_query != NULL) {
    (void)cmn_textbox_destroy(&tb_query);
  }
  if (lbl_search != NULL) {
    (void)cmn_label_destroy(&lbl_search);
  }
  if (grid != NULL) {
    (void)cmn_grid_destroy(&grid);
  }
  if (win != NULL) {
    (void)cmn_window_destroy(&win);
  }
  if (app != NULL) {
    (void)cmn_app_destroy(&app);
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
  ctx->count++;
  (void)cmn_sprintf_s(line, sizeof(line), "Button click (" CMN_INT32_FMT ")%c",
                      ctx->count, 10);
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

    rc = cmn_window_set_title(win, "Hello World");
    rc = cmn_flex_set_direction(flex, CMN_FLEX_COLUMN);
    rc = cmn_flex_set_gap(flex, 12, 12);
    rc = cmn_flex_set_padding(flex, pad);

    rc = cmn_label_set_text(lbl, "Hello!, I'm a label");
    rc = cmn_button_set_text(btn, "Click Me!");
    rc = cmn_button_set_on_click(btn, on_hello_scene_click, &ctx);

    rc = cmn_widget_set_bounds(lbl, r_lbl);
    rc = cmn_widget_set_bounds(btn, r_btn);
    rc = cmn_widget_set_bounds(tv, r_tv);

    rc = cmn_flex_add_child(flex, lbl, NULL);
    rc = cmn_flex_item_style_init(&btn_style);
    btn_style.align_self = CMN_ALIGN_FLEX_START;
    rc = cmn_flex_add_child(flex, btn, &btn_style);
    rc = cmn_flex_item_style_init(&tv_style);
    tv_style.flex_grow = 1.0f;
    tv_style.flex_basis = 160;
    rc = cmn_flex_add_child(flex, tv, &tv_style);
    rc = cmn_window_set_layout(win, flex);
    rc = cmn_window_set_size(win, 480, 360);

    rc = cmn_automation_click(btn);
    rc = cmn_automation_click(btn);
    rc = cmn_automation_click(btn);
    rc = cmn_automation_click(btn);

    rc = cmn_window_show(win);
    rc = cmn_automation_capture_screenshot(win, filepath);
  }

  if (tv != NULL) {
    (void)cmn_textview_destroy(&tv);
  }
  if (btn != NULL) {
    (void)cmn_button_destroy(&btn);
  }
  if (lbl != NULL) {
    (void)cmn_label_destroy(&lbl);
  }
  if (flex != NULL) {
    (void)cmn_flex_destroy(&flex);
  }
  if (win != NULL) {
    (void)cmn_window_destroy(&win);
  }
  if (app != NULL) {
    (void)cmn_app_destroy(&app);
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
  (void)userdata;
  *out_valid = (text != NULL && strlen(text) >= 4) ? 1 : 0;
  *out_err = NULL;
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

    rc = cmn_window_set_title(win, "Simple Password Flow (/login)");
    rc = cmn_grid_set_auto_fit_columns(grid, 260, 1);
    rc = cmn_grid_set_rows(grid, row_tracks, 6);
    rc = cmn_grid_set_gap(grid, 10, 10);
    rc = cmn_grid_set_padding(grid, pad);
    rc = cmn_grid_add_breakpoint(grid, 400, 1);

    rc = cmn_label_set_text(lbl_user, "Username:");
    rc = cmn_automation_type_text(tb_user, "samuel_marks", 0);
    rc = cmn_label_set_text(lbl_pass, "Password (min 4 chars):");
    rc = cmn_textbox_set_password(tb_pass, 1);
    rc = cmn_automation_type_text(tb_pass, "correct_horse_battery", 0);
    rc = cmn_button_set_text(btn_submit, "Sign in / Sign up");

    rc = cmn_form_add_input(form, tb_user, validate_non_empty, NULL);
    rc = cmn_form_add_input(form, tb_pass, validate_non_empty, NULL);
    rc = cmn_form_validate(form, &is_valid);

    rc = cmn_label_set_text(lbl_status, is_valid ? "Form Status: Valid (Ready)"
                                                 : "Form Status: Invalid");

    {
      cmn_rect_t r_btn = {0, 0, 180, 34};
      rc = cmn_widget_set_bounds(btn_submit, r_btn);
    }
    rc = cmn_grid_item_style_init(&btn_gs);
    btn_gs.justify_self = CMN_ALIGN_FLEX_START;

    rc = cmn_grid_add_child(grid, lbl_user, NULL);
    rc = cmn_grid_add_child(grid, tb_user, NULL);
    rc = cmn_grid_add_child(grid, lbl_pass, NULL);
    rc = cmn_grid_add_child(grid, tb_pass, NULL);
    rc = cmn_grid_add_child(grid, lbl_status, NULL);
    rc = cmn_grid_add_child(grid, btn_submit, &btn_gs);
    rc = cmn_window_set_layout(win, grid);
    rc = cmn_window_set_size(win, 480, 360);

    rc = cmn_window_show(win);
    rc = cmn_automation_capture_screenshot(win, filepath);
  }

  if (btn_submit != NULL) {
    (void)cmn_button_destroy(&btn_submit);
  }
  if (lbl_status != NULL) {
    (void)cmn_label_destroy(&lbl_status);
  }
  if (tb_pass != NULL) {
    (void)cmn_textbox_destroy(&tb_pass);
  }
  if (lbl_pass != NULL) {
    (void)cmn_label_destroy(&lbl_pass);
  }
  if (tb_user != NULL) {
    (void)cmn_textbox_destroy(&tb_user);
  }
  if (lbl_user != NULL) {
    (void)cmn_label_destroy(&lbl_user);
  }
  if (form != NULL) {
    (void)cmn_form_destroy(&form);
  }
  if (grid != NULL) {
    (void)cmn_grid_destroy(&grid);
  }
  if (win != NULL) {
    (void)cmn_window_destroy(&win);
  }
  if (app != NULL) {
    (void)cmn_app_destroy(&app);
  }
  return rc;
}

/**
 * @brief Writes the HTML5 DOM document corresponding to a `cmn_web_widget.c`
 * scene (`0`=buttons, `1`=inputs, `2`=hello_world, `3`=password_flow) so a real
 * headless browser can render and screenshot the exact Web DOM widgets.
 * @param scene_id Scene index (`0`..`3`).
 * @param html_filepath Destination `.html` file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_screenshot_write_web_html(int scene_id,
                                          const char *html_filepath) {
  static const char *const prefix_a =
      "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><style>"
      "body{margin:0;padding:20px 24px;box-sizing:border-box;width:480px;"
      "height:360px;font-family:system-ui,-apple-system,sans-serif;"
      "font-size:14px;background:#fff;}"
      ".fcol{display:flex;flex-direction:column;gap:14px;width:100%;}";
  static const char *const prefix_b =
      ".frow{display:flex;flex-direction:row;gap:16px;height:34px;}"
      ".grd{display:grid;grid-template-columns:1fr;gap:10px;width:100%;}"
      "input,textarea{width:100%;box-sizing:border-box;}"
      "button{height:32px;}</style></head><body>";
  static const char *const bodies[4] = {
      "<div class=\"fcol\"><span>Native OS Push Buttons (cmn_button_t)</span>"
      "<div class=\"frow\"><button style=\"flex:1\">Save Changes</button>"
      "<button style=\"flex:1\">Cancel</button></div>"
      "<button>Trigger Action (Click Me)</button>"
      "<span>Status: Button clicked 3 times</span></div>",

      "<div class=\"grd\"><span>Search Filter (cmn_textbox_t):</span>"
      "<input type=\"text\" value=\"SELECT * FROM tasks WHERE id = 1\">"
      "<span>Event Log (Scrollable cmn_textview_t):</span>"
      "<textarea readonly style=\"height:170px;resize:none;overflow-y:scroll\">"
      "[01] Signal initialized (val = 0)&#10;"
      "[02] Route /login -&gt; /welcome&#10;"
      "[03] Room DAO synced 12 SQLite rows&#10;</textarea></div>",

      "<div class=\"fcol\"><span>Hello!, I'm a label</span>"
      "<button style=\"width:160px\">Click Me!</button>"
      "<textarea readonly style=\"height:190px;resize:none;overflow-y:scroll\">"
      "Button click (1)&#10;Button click (2)&#10;"
      "Button click (3)&#10;Button click (4)&#10;</textarea></div>",

      "<div class=\"grd\"><span>Username:</span>"
      "<input type=\"text\" value=\"samuel_marks\">"
      "<span>Password (min 4 chars):</span>"
      "<input type=\"password\" value=\"correct_horse_battery\">"
      "<span>Form Status: Valid (Ready)</span>"
      "<button style=\"width:180px\">Sign in / Sign up</button></div>"};
  static const char *const suffix = "</body></html>";
  FILE *fp = NULL;

  if (scene_id < 0 || scene_id > 3 || html_filepath == NULL ||
      html_filepath[0] == '\0') {
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
  (void)fwrite(prefix_a, 1, strlen(prefix_a), fp);
  (void)fwrite(prefix_b, 1, strlen(prefix_b), fp);
  (void)fwrite(bodies[scene_id], 1, strlen(bodies[scene_id]), fp);
  (void)fwrite(suffix, 1, strlen(suffix), fp);
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
  cmn_error_e rc;

  if (output_dir == NULL || output_dir[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  if (plat == NULL || plat[0] == '\0') {
    (void)cmn_screenshot_get_platform_suffix(&plat);
  }

#if defined(_WIN32)
  (void)_mkdir(output_dir);
#else
  {
    cfs_path dir_path;
    cfs_error_code cfs_rc;
    memset(&cfs_rc, 0, sizeof(cfs_rc));
    if (cfs_path_init_str(&dir_path, output_dir) == 0) {
      (void)cfs_create_directories(&dir_path, &cfs_rc);
      cfs_path_destroy(&dir_path);
    }
  }
#endif

  (void)cmn_sprintf_s(path_buf, sizeof(path_buf), "%s/button_%s.png",
                      output_dir, plat);
  rc = cmn_screenshot_capture_buttons_scene(path_buf);

  if (rc == CMN_SUCCESS) {
    (void)cmn_sprintf_s(path_buf, sizeof(path_buf), "%s/inputs_%s.png",
                        output_dir, plat);
    rc = cmn_screenshot_capture_inputs_scene(path_buf);
  }

  if (rc == CMN_SUCCESS) {
    (void)cmn_sprintf_s(path_buf, sizeof(path_buf), "%s/hello_world_%s.png",
                        output_dir, plat);
    rc = cmn_screenshot_capture_hello_world_scene(path_buf);
  }

  if (rc == CMN_SUCCESS) {
    (void)cmn_sprintf_s(path_buf, sizeof(path_buf), "%s/password_flow_%s.png",
                        output_dir, plat);
    rc = cmn_screenshot_capture_password_flow_scene(path_buf);
  }

  return rc;
}
