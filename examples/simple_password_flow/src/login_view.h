#ifndef LOGIN_VIEW_H
#define LOGIN_VIEW_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <cmn/error.h>
#include "app_state.h"
#include <cmn/form.h>
#include <cmn/layout.h>
#include <cmn/widget.h>
/* clang-format on */

/**
 * @file login_view.h
 * @brief Login view definitions and validation callbacks.
 * @addtogroup example_password_flow
 * @{
 */

/**
 * @brief Factory function for the login view matching `cmn_route_factory_fn`.
 * @param userdata Pointer to `app_state_t`.
 * @param params Route parameters.
 * @param param_count Number of route parameters.
 * @param out_widget Output widget pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e login_view_factory(void *userdata,
                                              const char **params,
                                              size_t param_count,
                                              void **out_widget);

/**
 * @brief Login view state container.
 */
typedef struct {
  app_state_t *app_state;     /**< Application state. */
  cmn_form_t *form;           /**< Login form. */
  cmn_textbox_t *tb_username; /**< Username textbox. */
  cmn_textbox_t *tb_password; /**< Password textbox. */
  cmn_button_t *btn_submit;   /**< Submit button. */
  cmn_grid_t *root;           /**< Root responsive CSS Grid layout. */
} login_view_data_t;

/** @brief Active login view data instance. */
extern login_view_data_t *g_login_data;

/**
 * @brief Tears down and frees the active login view data and its child widgets.
 * @param destroy_root Non-zero to also destroy `root` (0 when router owns
 * `root`).
 * @return CMN_SUCCESS on success.
 */
CMN_NO_DISCARD cmn_error_e login_view_cleanup(int destroy_root);

/**
 * @brief Validates username input (non-empty).
 * @param val Input username string.
 * @param userdata Unused user data.
 * @param out_is_valid Output validity flag.
 * @param out_err_msg Output error message.
 * @return CMN_SUCCESS on success.
 */
CMN_NO_DISCARD cmn_error_e validate_username(const char *val, void *userdata,
                                             int *out_is_valid,
                                             char **out_err_msg);

/**
 * @brief Validates password input (> 3 characters).
 * @param val Input password string.
 * @param userdata Unused user data.
 * @param out_is_valid Output validity flag.
 * @param out_err_msg Output error message.
 * @return CMN_SUCCESS on success.
 */
CMN_NO_DISCARD cmn_error_e validate_password(const char *val, void *userdata,
                                             int *out_is_valid,
                                             char **out_err_msg);

/**
 * @brief Handles login form submission.
 * @param userdata Pointer to `login_view_data_t`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e on_login_submit(void *userdata);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOGIN_VIEW_H */
