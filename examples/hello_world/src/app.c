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
  cmn_error_e rc;

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
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }

  rc = cmn_vbox_create(&ctx->vbox);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }

  rc = cmn_label_create(&ctx->label);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_label_set_text(ctx->label, "Hello!, I'm a label");
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }

  rc = cmn_button_create(&ctx->button);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_button_set_text(ctx->button, "Click Me!");
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_button_set_on_click(ctx->button, handle_button_click, ctx);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }

  rc = cmn_textview_create(&ctx->textview);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }

  rc = cmn_widget_set_parent(ctx->vbox, ctx->label);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_widget_set_parent(ctx->vbox, ctx->button);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_widget_set_parent(ctx->vbox, ctx->textview);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_widget_set_parent(ctx->window, ctx->vbox);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_window_show(ctx->window);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }

  *out_app = ctx;
  return CMN_SUCCESS;

cleanup:
  if (ctx->button != NULL) {
    (void)cmn_button_destroy(&ctx->button);
  }
  if (ctx->label != NULL) {
    (void)cmn_label_destroy(&ctx->label);
  }
  if (ctx->vbox != NULL) {
    (void)cmn_vbox_destroy(&ctx->vbox);
  }
  if (ctx->window != NULL) {
    (void)cmn_window_destroy(&ctx->window);
  }
  if (ctx->app != NULL) {
    (void)cmn_app_destroy(&ctx->app);
  }
  (void)cmn_free((void **)&ctx);
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
  if (ctx->vbox != NULL) {
    rc = (cmn_error_e)((int)rc | (int)cmn_vbox_destroy(&ctx->vbox));
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
