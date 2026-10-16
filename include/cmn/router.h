#ifndef CMN_ROUTER_H
#define CMN_ROUTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
#include "cmn/widget.h"
/* clang-format on */

/**
 * @file router.h
 * @brief Routing, view mounting, and deep linking management.
 */

/**
 * @defgroup cmn_router Router and Deep Linking
 * @brief Router and Deep Linking module.
 * @{
 */

/** @brief Maximum number of parameters extracted from a route pattern. */
#define CMN_MAX_ROUTE_PARAMS 8

/**
 * @brief Callback function type for creating a view for a matched route.
 * @param userdata User data passed during route registration.
 * @param params Array of null-terminated route parameter strings.
 * @param param_count Number of route parameters.
 * @param out_widget Pointer to store the created root widget for the view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_route_factory_fn)(void *userdata, const char **params,
                                            size_t param_count,
                                            void **out_widget);

/**
 * @brief Creates a new router instance bound to a parent container or window.
 * @param container The parent container widget (`cmn_container_t *`,
 * `cmn_window_t *`, `cmn_vbox_t *`, or `cmn_hbox_t *`) where routed views will
 * be mounted.
 * @param out_router Pointer to store the created router.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_router_create(void *container,
                                             cmn_router_t **out_router);

/**
 * @brief Registers a route pattern and its view factory with the router.
 * @param router The router instance.
 * @param route_pattern The route pattern (e.g., "/users/:id").
 * @param factory The factory function to create the view.
 * @param userdata User data passed to the factory function.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_router_register(cmn_router_t *router,
                                               const char *route_pattern,
                                               cmn_route_factory_fn factory,
                                               void *userdata);

/**
 * @brief Navigates to a specific path, instantiating and mounting the matched
 * view.
 * @param router The router instance.
 * @param path The path to navigate to.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_router_navigate(cmn_router_t *router,
                                               const char *path);

/**
 * @brief Handles a deep link URL by extracting the path and navigating to it.
 * @param router The router instance.
 * @param url The deep link URL (e.g., "myapp://host/users/123" or
 * "myapp://login").
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_router_handle_deep_link(cmn_router_t *router,
                                                       const char *url);

/**
 * @brief Gets the current active path of the router.
 * @param router The router instance.
 * @param out_path Pointer to store the current path string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_router_get_current_path(cmn_router_t *router,
                                                       const char **out_path);

/**
 * @brief Destroys a router instance, unmounting any active view and freeing its
 * memory.
 * @param router Pointer to the router instance to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_router_destroy(cmn_router_t **router);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_ROUTER_H */
