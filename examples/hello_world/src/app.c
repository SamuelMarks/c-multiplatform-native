/* clang-format off */
#include "app.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include <stdio.h>
/* clang-format on */

/**
 * @file app.c
 * @brief Implementation of the Hello World 3-row native GUI counter app.
 */

/**
 * @brief Callback invoked when the "Click Me!" button is clicked.
 * @param userdata Pointer to `hello_world_app_t`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e handle_button_click(void *userdata) {
  hello_world_app_t *ctx = (hello_world_app_t *)userdata;
  char buf[256];
  cmn_error_e rc;

  ctx->click_count++;
  rc = cmn_sprintf_s(buf, sizeof(buf), "Button click (" CMN_UINT32_FMT ")\n",
                     ctx->click_count);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  return cmn_textview_append_text(ctx->textview, buf);
}

/**
 * @brief Initializes the hello world application.
 * @param out_app Pointer to receive the initialized app context.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e hello_world_app_init(hello_world_app_t **out_app) {
  hello_world_app_t *ctx = NULL;
  cmn_edges_t pad = {16, 20, 16, 20};
  cmn_rect_t lbl_bounds = {0, 0, 400, 26};
  cmn_rect_t btn_bounds = {0, 0, 160, 32};
  cmn_rect_t tv_bounds = {0, 0, 400, 140};
  cmn_flex_item_style_t tv_style;
  cmn_error_e rc;
  cmn_error_e clean_rc;

  if (out_app == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(hello_world_app_t), (void **)&ctx);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_app_create(&ctx->app);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }

  rc = cmn_window_create(ctx->app, &ctx->window);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_window_set_title(ctx->window, "Hello World");

  rc = cmn_flex_create(&ctx->flex);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_flex_set_direction(ctx->flex, CMN_FLEX_COLUMN);
  rc = cmn_flex_set_gap(ctx->flex, 12, 12);
  rc = cmn_flex_set_padding(ctx->flex, pad);
  rc =
      cmn_flex_add_breakpoint(ctx->flex, 360, CMN_FLEX_COLUMN, CMN_FLEX_NOWRAP);

  rc = cmn_label_create(&ctx->label);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_label_set_text(ctx->label, "Hello!, I'm a label");
  rc = cmn_widget_set_bounds(ctx->label, lbl_bounds);

  rc = cmn_button_create(&ctx->button);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_button_set_text(ctx->button, "Click Me!");
  rc = cmn_button_set_on_click(ctx->button, handle_button_click, ctx);
  rc = cmn_widget_set_bounds(ctx->button, btn_bounds);

  rc = cmn_textview_create(&ctx->textview);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_widget_set_bounds(ctx->textview, tv_bounds);

  rc = cmn_flex_add_child(ctx->flex, ctx->label, NULL);
  rc = cmn_flex_add_child(ctx->flex, ctx->button, NULL);
  rc = cmn_flex_item_style_init(&tv_style);
  tv_style.flex_grow = 1.0f;
  tv_style.flex_basis = 140;
  tv_style.min_height = 80;
  rc = cmn_flex_add_child(ctx->flex, ctx->textview, &tv_style);
  rc = cmn_window_set_layout(ctx->window, ctx->flex);
  rc = cmn_window_set_size(ctx->window, 480, 360);
  rc = cmn_window_show(ctx->window);

  *out_app = ctx;
  return rc;

cleanup:
  if (ctx->button != NULL) {
    clean_rc = cmn_button_destroy(&ctx->button);
    (void)clean_rc;
  }
  if (ctx->label != NULL) {
    clean_rc = cmn_label_destroy(&ctx->label);
    (void)clean_rc;
  }
  if (ctx->flex != NULL) {
    clean_rc = cmn_flex_destroy(&ctx->flex);
    (void)clean_rc;
  }
  if (ctx->window != NULL) {
    clean_rc = cmn_window_destroy(&ctx->window);
    (void)clean_rc;
  }
  if (ctx->app != NULL) {
    clean_rc = cmn_app_destroy(&ctx->app);
    (void)clean_rc;
  }
  clean_rc = cmn_free((void **)&ctx);
  (void)clean_rc;
  *out_app = NULL;
  return rc;
}

/**
 * @brief Runs the hello world application.
 * @param app The initialized app context.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e hello_world_app_run(hello_world_app_t *app) {
  if (app == NULL || app->app == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_app_run(app->app);
}

/**
 * @brief Tears down the hello world application.
 * @param app The app context pointer to free.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e hello_world_app_destroy(hello_world_app_t **app) {
  hello_world_app_t *ctx;
  cmn_error_e rc = CMN_SUCCESS;

  if (app == NULL || *app == NULL) {
    return CMN_ERR_INVAL;
  }

  ctx = *app;

  if (ctx->textview != NULL) {
    rc = cmn_textview_destroy(&ctx->textview);
  }
  if (ctx->button != NULL) {
    rc = (cmn_error_e)((int)rc | (int)cmn_button_destroy(&ctx->button));
  }
  if (ctx->label != NULL) {
    rc = (cmn_error_e)((int)rc | (int)cmn_label_destroy(&ctx->label));
  }
  if (ctx->flex != NULL) {
    rc = (cmn_error_e)((int)rc | (int)cmn_flex_destroy(&ctx->flex));
  }
  if (ctx->window != NULL) {
    rc = (cmn_error_e)((int)rc | (int)cmn_window_destroy(&ctx->window));
  }
  if (ctx->app != NULL) {
    rc = (cmn_error_e)((int)rc | (int)cmn_app_destroy(&ctx->app));
  }

  rc = (cmn_error_e)((int)rc | (int)cmn_free((void **)&ctx));
  *app = NULL;
  return rc;
}
