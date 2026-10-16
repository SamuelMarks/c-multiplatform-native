/* clang-format off */
#include "app_state.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file app_state.c
 * @brief Implementation of app state initialization.
 */

/**
 * @brief Initializes the application state.
 * @param state The state to initialize.
 * @param router The router instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e app_state_init(app_state_t *state, cmn_router_t *router) {
  if (state == NULL || router == NULL) {
    return CMN_ERR_INVAL;
  }
  state->router = router;
  state->username[0] = '\0';
  return CMN_SUCCESS;
}
