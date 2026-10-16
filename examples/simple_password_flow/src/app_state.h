#ifndef APP_STATE_H
#define APP_STATE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <cmn/error.h>
#include <cmn/router.h>
/* clang-format on */

/**
 * @file app_state.h
 * @brief Application state management for simple password flow.
 */

/**
 * @defgroup example_password_flow Simple Password Flow Example
 * @brief Simple Password Flow Example module.
 * @{
 */

/**
 * @brief Application state container.
 */
typedef struct {
  cmn_router_t *router; /**< The router instance. */
  char username[256];   /**< The logged in username. */
} app_state_t;

/**
 * @brief Initializes the application state.
 * @param state The state to initialize.
 * @param router The router instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e app_state_init(app_state_t *state,
                                          cmn_router_t *router);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APP_STATE_H */
