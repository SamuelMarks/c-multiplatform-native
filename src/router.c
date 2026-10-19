/* clang-format off */
#include "cmn_internal.h"
#include "cmn/router.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

/**
 * @file router.c
 * @brief Implementation of routing, parameter extraction, view mounting, and
 * deep linking.
 */

/**
 * @def MAX_ROUTES
 * @brief Maximum number of routes that can be registered in a router.
 */
#define MAX_ROUTES 32

/**
 * @def MAX_PATH_LEN
 * @brief Maximum length of a route path or URL segment.
 */
#define MAX_PATH_LEN 256

/**
 * @brief Internal record representing a registered route.
 */
typedef struct route_record_s {
  char pattern[MAX_PATH_LEN];   /**< The route pattern (e.g., "/users/:id"). */
  cmn_route_factory_fn factory; /**< Factory function to create the view. */
  void *userdata;               /**< User data passed to the factory. */
} route_record_t;

/**
 * @brief Internal representation of a router instance.
 */
struct cmn_router_s {
  void *container;                   /**< The parent container widget. */
  void *current_view;                /**< The currently active view widget. */
  char current_path[MAX_PATH_LEN];   /**< The current active path. */
  route_record_t routes[MAX_ROUTES]; /**< Array of registered routes. */
  size_t route_count;                /**< Number of registered routes. */
};

/**
 * @brief Creates a new router instance bound to a parent container or window.
 * @param container The parent container widget (`cmn_container_t *`,
 * `cmn_window_t *`, `cmn_vbox_t *`, or `cmn_hbox_t *`) where routed views will
 * be mounted.
 * @param out_router Pointer to store the created router.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_router_create(void *container, cmn_router_t **out_router) {
  cmn_error_e rc;
  cmn_router_t *r = NULL;

  if (container == NULL || out_router == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_router_t), (void **)&r);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  r->container = container;
  r->current_view = NULL;
  r->current_path[0] = '\0';
  r->route_count = 0;

  *out_router = r;
  return CMN_SUCCESS;
}

/**
 * @brief Registers a route pattern and its view factory with the router.
 * @param router The router instance.
 * @param route_pattern The route pattern (e.g., "/users/:id").
 * @param factory The factory function to create the view.
 * @param userdata User data passed to the factory function.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_router_register(cmn_router_t *router, const char *route_pattern,
                                cmn_route_factory_fn factory, void *userdata) {
  cmn_error_e rc;

  if (router == NULL || route_pattern == NULL || factory == NULL) {
    return CMN_ERR_INVAL;
  }

  if (router->route_count >= MAX_ROUTES) {
    return CMN_ERR_NOMEM;
  }

  rc = cmn_strcpy_s(router->routes[router->route_count].pattern, MAX_PATH_LEN,
                    route_pattern);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  router->routes[router->route_count].factory = factory;
  router->routes[router->route_count].userdata = userdata;
  router->route_count++;

  return CMN_SUCCESS;
}

/**
 * @brief Matches a path against a route pattern and extracts null-terminated
 * parameters.
 * @param pattern The route pattern (e.g., "/user/:id/profile").
 * @param path The path to match (e.g., "/user/42/profile").
 * @param param_buffers 2D character buffer storing null-terminated parameters.
 * @param params Output array of pointers into `param_buffers`.
 * @param param_count Output pointer to store the number of matched parameters.
 * @param out_matched Output pointer to store 1 if matched, 0 otherwise.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e
match_route(const char *pattern, const char *path,
            char param_buffers[CMN_MAX_ROUTE_PARAMS][MAX_PATH_LEN],
            const char **params, size_t *param_count, int *out_matched) {
  const char *p_pat = pattern;
  const char *p_path = path;
  *param_count = 0;
  *out_matched = 0;

  while (*p_pat && *p_path) {
    if (*p_pat == ':') {
      const char *start;
      size_t seg_len;
      size_t i;

      while (*p_pat && *p_pat != '/') {
        p_pat++;
      }

      start = p_path;
      while (*p_path && *p_path != '/') {
        p_path++;
      }

      seg_len = (size_t)(p_path - start);
      if (seg_len == 0) {
        return CMN_SUCCESS;
      }

      if (*param_count < CMN_MAX_ROUTE_PARAMS) {
        for (i = 0; i < seg_len; ++i) {
          param_buffers[*param_count][i] = start[i];
        }
        param_buffers[*param_count][seg_len] = '\0';
        params[*param_count] = param_buffers[*param_count];
        (*param_count)++;
      }
    } else {
      if (*p_pat != *p_path) {
        return CMN_SUCCESS;
      }
      p_pat++;
      p_path++;
    }
  }

  *out_matched = (*p_pat == '\0' && *p_path == '\0') ? 1 : 0;
  return CMN_SUCCESS;
}

/**
 * @brief Navigates to a specific path, instantiating and mounting the matched
 * view.
 * @param router The router instance.
 * @param path The path to navigate to.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_router_navigate(cmn_router_t *router, const char *path) {
  size_t i;
  char param_buffers[CMN_MAX_ROUTE_PARAMS][MAX_PATH_LEN];
  const char *params[CMN_MAX_ROUTE_PARAMS];
  size_t param_count = 0;
  void *new_view = NULL;
  cmn_error_e rc;

  if (router == NULL || path == NULL || strlen(path) >= MAX_PATH_LEN) {
    return CMN_ERR_INVAL;
  }

  for (i = 0; i < router->route_count; ++i) {
    int is_matched = 0;
    rc = match_route(router->routes[i].pattern, path, param_buffers, params,
                     &param_count, &is_matched);
    if (is_matched && rc == CMN_SUCCESS) {
      rc = router->routes[i].factory(router->routes[i].userdata, params,
                                     param_count, &new_view);
      if (rc != CMN_SUCCESS) {
        return rc;
      }

      rc = cmn_strcpy_s(router->current_path, MAX_PATH_LEN, path);

      if (router->current_view != NULL) {
        cmn_error_e destroy_rc =
            cmn_widget_destroy((cmn_widget_t **)&router->current_view);
        rc = (rc != CMN_SUCCESS) ? rc : destroy_rc;
      }

      if (new_view != NULL) {
        cmn_error_e parent_rc =
            cmn_widget_set_parent(router->container, new_view);
        rc = (rc != CMN_SUCCESS) ? rc : parent_rc;
      }

      router->current_view = new_view;
      return rc;
    }
  }

  return CMN_ERR_NOT_FOUND;
}

/**
 * @brief Handles a deep link URL by extracting the path and navigating to it.
 * @param router The router instance.
 * @param url The deep link URL (e.g., "myapp://host/users/123" or
 * "myapp://login").
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_router_handle_deep_link(cmn_router_t *router, const char *url) {
  const char *scheme_end;
  const char *src_path = url;
  char normalized_path[MAX_PATH_LEN];
  size_t i = 0;

  if (router == NULL || url == NULL) {
    return CMN_ERR_INVAL;
  }

  scheme_end = strstr(url, "://");
  if (scheme_end != NULL) {
    const char *after_scheme = scheme_end + 3;
    const char *slash = strchr(after_scheme, '/');

    if (slash != NULL) {
      src_path = slash;
    } else {
      normalized_path[i++] = '/';
      src_path = after_scheme;
    }
  }

  while (*src_path != '\0' && *src_path != '?' && *src_path != '#') {
    normalized_path[i++] = *src_path++;
  }
  normalized_path[i] = '\0';
  return cmn_router_navigate(router, normalized_path);
}

/**
 * @brief Gets the current active path of the router.
 * @param router The router instance.
 * @param out_path Pointer to store the current path string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_router_get_current_path(cmn_router_t *router,
                                        const char **out_path) {
  if (router == NULL || out_path == NULL) {
    return CMN_ERR_INVAL;
  }

  *out_path = router->current_path;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a router instance, unmounting any active view and freeing its
 * memory.
 * @param router Pointer to the router instance to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_router_destroy(cmn_router_t **router) {
  cmn_error_e rc = CMN_SUCCESS;
  cmn_error_e free_rc;
  if (router == NULL || *router == NULL) {
    return CMN_ERR_INVAL;
  }

  if ((*router)->current_view != NULL) {
    rc = cmn_widget_destroy((cmn_widget_t **)&(*router)->current_view);
  }

  free_rc = cmn_free((void **)router);
  return rc != CMN_SUCCESS ? rc : free_rc;
}
