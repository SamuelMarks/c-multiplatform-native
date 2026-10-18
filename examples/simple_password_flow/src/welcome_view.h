#ifndef WELCOME_VIEW_H
#define WELCOME_VIEW_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <cmn/error.h>
#include "app_state.h"
#include <cmn/layout.h>
#include <cmn/widget.h>
/* clang-format on */

/**
 * @file welcome_view.h
 * @brief Welcome view definitions and logout handler.
 * @addtogroup example_password_flow
 * @{
 */

/**
 * @brief Factory function for the welcome view matching `cmn_route_factory_fn`.
 * @param userdata Pointer to `app_state_t`.
 * @param params Route parameters.
 * @param param_count Number of route parameters.
 * @param out_widget Output widget pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e welcome_view_factory(void *userdata,
                                                const char **params,
                                                size_t param_count,
                                                void **out_widget);

/**
 * @brief Welcome view state container.
 */
typedef struct {
  app_state_t *app_state;   /**< Application state. */
  cmn_label_t *lbl_welcome; /**< Welcome label. */
  cmn_button_t *btn_logout; /**< Logout button. */
  cmn_flex_t *root;         /**< Root responsive Flexbox layout. */
} welcome_view_data_t;

/** @brief Active welcome view data instance. */
extern welcome_view_data_t *g_welcome_data;

/**
 * @brief Tears down and frees the active welcome view data and its child
 * widgets.
 * @param destroy_root Non-zero to also destroy `root` (0 when router owns
 * `root`).
 * @return CMN_SUCCESS on success.
 */
CMN_NO_DISCARD cmn_error_e welcome_view_cleanup(int destroy_root);

/**
 * @brief Handles logout button click.
 * @param userdata Pointer to `welcome_view_data_t`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e on_logout_click(void *userdata);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WELCOME_VIEW_H */
