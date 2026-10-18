/* clang-format off */
#include "welcome_view.h"
#include "app_state.h"
#include <cmn/memory.h>
#include <cmn/string_safe.h>
#include <cmn/widget.h>
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

/**
 * @file welcome_view.c
 * @brief Implementation of welcome view and logout action.
 */

welcome_view_data_t *g_welcome_data = NULL;

/**
 * @brief Tears down and frees the active welcome view data and its child
 * widgets.
 * @param destroy_root Non-zero to also destroy `root` (0 when router owns
 * `root`).
 * @return CMN_SUCCESS on success.
 */
cmn_error_e welcome_view_cleanup(int destroy_root) {
  cmn_error_e rc = CMN_SUCCESS;
  if (g_welcome_data != NULL) {
    if (g_welcome_data->btn_logout != NULL) {
      rc = cmn_button_destroy(&g_welcome_data->btn_logout);
    }
    if (g_welcome_data->lbl_welcome != NULL) {
      rc = (cmn_error_e)((int)rc |
                         (int)cmn_label_destroy(&g_welcome_data->lbl_welcome));
    }
    if (destroy_root && g_welcome_data->root != NULL) {
      rc =
          (cmn_error_e)((int)rc | (int)cmn_flex_destroy(&g_welcome_data->root));
    }
    rc = (cmn_error_e)((int)rc | (int)cmn_free((void **)&g_welcome_data));
  }
  return rc;
}

/**
 * @brief Handles logout button click.
 * @param userdata Pointer to `welcome_view_data_t`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e on_logout_click(void *userdata) {
  welcome_view_data_t *data = (welcome_view_data_t *)userdata;
  cmn_error_e rc;

  data->app_state->username[0] = '\0';

  rc = cmn_router_navigate(data->app_state->router, "/login");
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  return CMN_SUCCESS;
}

/**
 * @brief Factory function for the welcome view matching `cmn_route_factory_fn`.
 * @param userdata Pointer to `app_state_t`.
 * @param params Route parameters.
 * @param param_count Number of route parameters.
 * @param out_widget Output widget pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e welcome_view_factory(void *userdata, const char **params,
                                 size_t param_count, void **out_widget) {
  cmn_error_e rc;
  cmn_error_e cleanup_rc;
  app_state_t *state = (app_state_t *)userdata;
  welcome_view_data_t *data = NULL;
  cmn_edges_t pad = {20, 24, 20, 24};
  char welcome_msg[512];

  (void)params;
  (void)param_count;

  cleanup_rc = welcome_view_cleanup(0);
  (void)cleanup_rc;

  rc = cmn_calloc(1, sizeof(welcome_view_data_t), (void **)&data);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_welcome_data = data;
  data->app_state = state;

  rc = cmn_flex_create(&data->root);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = welcome_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }
  rc = cmn_flex_set_direction(data->root, CMN_FLEX_COLUMN);
  rc = cmn_flex_set_justify_content(data->root, CMN_JUSTIFY_CENTER);
  rc = cmn_flex_set_align_items(data->root, CMN_ALIGN_CENTER);
  rc = cmn_flex_set_gap(data->root, 16, 16);
  rc = cmn_flex_set_padding(data->root, pad);

  rc = cmn_label_create(&data->lbl_welcome);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = welcome_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_sprintf_s(welcome_msg, sizeof(welcome_msg), "Welcome %s",
                     state->username);
  rc = cmn_label_set_text(data->lbl_welcome, welcome_msg);

  rc = cmn_button_create(&data->btn_logout);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = welcome_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_button_set_text(data->btn_logout, "Logout");
  rc = cmn_button_set_on_click(data->btn_logout, on_logout_click, data);

  rc = cmn_flex_add_child(data->root, data->lbl_welcome, NULL);
  rc = cmn_flex_add_child(data->root, data->btn_logout, NULL);
  rc = cmn_flex_compute_layout(data->root, 480, 360);

  *out_widget = data->root;
  return rc;
}
