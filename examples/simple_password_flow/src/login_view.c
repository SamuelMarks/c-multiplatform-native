/* clang-format off */
#include "login_view.h"
#include "app_state.h"
#include <cmn/form.h>
#include <cmn/memory.h>
#include <cmn/string_safe.h>
#include <cmn/widget.h>
#include <stddef.h>
#include <string.h>
/* clang-format on */

/**
 * @file login_view.c
 * @brief Implementation of login view and form validation.
 */

login_view_data_t *g_login_data = NULL;

/**
 * @brief Tears down and frees the active login view data and its child widgets.
 * @param destroy_root Non-zero to also destroy `root` (0 when router owns
 * `root`).
 * @return CMN_SUCCESS on success.
 */
cmn_error_e login_view_cleanup(int destroy_root) {
  cmn_error_e rc = CMN_SUCCESS;
  if (g_login_data != NULL) {
    if (g_login_data->btn_submit != NULL) {
      rc = cmn_button_destroy(&g_login_data->btn_submit);
    }
    if (g_login_data->tb_password != NULL) {
      rc = (cmn_error_e)((int)rc |
                         (int)cmn_textbox_destroy(&g_login_data->tb_password));
    }
    if (g_login_data->tb_username != NULL) {
      rc = (cmn_error_e)((int)rc |
                         (int)cmn_textbox_destroy(&g_login_data->tb_username));
    }
    if (g_login_data->form != NULL) {
      rc = (cmn_error_e)((int)rc | (int)cmn_form_destroy(&g_login_data->form));
    }
    if (destroy_root && g_login_data->root != NULL) {
      rc = (cmn_error_e)((int)rc | (int)cmn_vbox_destroy(&g_login_data->root));
    }
    rc = (cmn_error_e)((int)rc | (int)cmn_free((void **)&g_login_data));
  }
  return rc;
}

/**
 * @brief Validates username input (non-empty).
 * @param val Input username string.
 * @param userdata Unused user data.
 * @param out_is_valid Output validity flag.
 * @param out_err_msg Output error message.
 * @return CMN_SUCCESS on success.
 */
cmn_error_e validate_username(const char *val, void *userdata,
                              int *out_is_valid, char **out_err_msg) {
  (void)userdata;
  (void)out_err_msg;

  if (val != NULL && strlen(val) > 0) {
    *out_is_valid = 1;
  } else {
    *out_is_valid = 0;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Validates password input (> 3 characters).
 * @param val Input password string.
 * @param userdata Unused user data.
 * @param out_is_valid Output validity flag.
 * @param out_err_msg Output error message.
 * @return CMN_SUCCESS on success.
 */
cmn_error_e validate_password(const char *val, void *userdata,
                              int *out_is_valid, char **out_err_msg) {
  (void)userdata;
  (void)out_err_msg;

  if (val != NULL && strlen(val) > 3) {
    *out_is_valid = 1;
  } else {
    *out_is_valid = 0;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Handles login form submission.
 * @param userdata Pointer to `login_view_data_t`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e on_login_submit(void *userdata) {
  login_view_data_t *data = (login_view_data_t *)userdata;
  cmn_error_e rc;
  int is_valid = 0;
  const char *username = NULL;

  rc = cmn_form_validate(data->form, &is_valid);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  if (is_valid) {
    rc = cmn_textbox_get_text(data->tb_username, &username);
    if (rc != CMN_SUCCESS) {
      return rc;
    }

    rc = cmn_strcpy_s(data->app_state->username,
                      sizeof(data->app_state->username), username);
    if (rc != CMN_SUCCESS) {
      return rc;
    }

    rc = cmn_router_navigate(data->app_state->router, "/welcome");
    if (rc != CMN_SUCCESS) {
      return rc;
    }
  }

  return CMN_SUCCESS;
}

/**
 * @brief Factory function for the login view matching `cmn_route_factory_fn`.
 * @param userdata Pointer to `app_state_t`.
 * @param params Route parameters.
 * @param param_count Number of route parameters.
 * @param out_widget Output widget pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e login_view_factory(void *userdata, const char **params,
                               size_t param_count, void **out_widget) {
  cmn_error_e rc;
  cmn_error_e cleanup_rc;
  app_state_t *state = (app_state_t *)userdata;
  login_view_data_t *data = NULL;

  (void)params;
  (void)param_count;

  rc = login_view_cleanup(0);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_calloc(1, sizeof(login_view_data_t), (void **)&data);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_login_data = data;
  data->app_state = state;

  rc = cmn_vbox_create(&data->root);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_form_create(&data->form);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_textbox_create(&data->tb_username);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_textbox_create(&data->tb_password);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_form_add_input(data->form, data->tb_username, validate_username,
                          NULL);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }
  rc = cmn_form_add_input(data->form, data->tb_password, validate_password,
                          NULL);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_button_create(&data->btn_submit);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_button_set_text(data->btn_submit, "Sign in/up");
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }
  rc = cmn_button_set_on_click(data->btn_submit, on_login_submit, data);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  rc = cmn_widget_set_parent(data->root, data->tb_username);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }
  rc = cmn_widget_set_parent(data->root, data->tb_password);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }
  rc = cmn_widget_set_parent(data->root, data->btn_submit);
  if (rc != CMN_SUCCESS) {
    cleanup_rc = login_view_cleanup(1);
    return rc != CMN_SUCCESS ? rc : cleanup_rc;
  }

  *out_widget = data->root;
  return CMN_SUCCESS;
}
