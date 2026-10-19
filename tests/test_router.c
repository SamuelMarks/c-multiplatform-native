/* clang-format off */
#include "greatest.h"
#include "cmn/router.h"
#include "cmn/string_safe.h"
#include "cmn/widget.h"
#include "cmn/memory.h"
#include <string.h>
#include <stdio.h>
/* clang-format on */

/**
 * @file test_router.c
 * @brief Behavioral unit tests for routing, parameter extraction, view
 * mounting, and deep linking.
 */

/** @brief External test helper `cmn_mock_alloc_set`. */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);

/**
 * @brief Captures route parameters passed to `factory_capture_params`.
 */
struct captured_route_params_s {
  size_t count;      /**< Number of parameters received. */
  char p0[64];       /**< First parameter copy. */
  char p1[64];       /**< Second parameter copy. */
  int create_widget; /**< 1 to allocate a real `cmn_label_t` view. */
};

/**
 * @brief Route factory that records extracted route parameters and optionally
 * creates a real widget.
 * @param userdata Pointer to `struct captured_route_params_s` (can be NULL).
 * @param params Array of extracted parameter strings.
 * @param param_count Number of extracted parameters.
 * @param out_widget Output pointer receiving the created view widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e factory_capture_params(void *userdata, const char **params,
                                          size_t param_count,
                                          void **out_widget) {
  struct captured_route_params_s *cap =
      (struct captured_route_params_s *)userdata;
  cmn_error_e rc = CMN_SUCCESS;
  *out_widget = NULL;

  if (cap != NULL) {
    cap->count = param_count;
    if (param_count > 0 && params[0] != NULL) {
      rc = cmn_strcpy_s(cap->p0, sizeof(cap->p0), params[0]);
    }
    if (param_count > 1 && params[1] != NULL && rc == CMN_SUCCESS) {
      rc = cmn_strcpy_s(cap->p1, sizeof(cap->p1), params[1]);
    }
    if (cap->create_widget && rc == CMN_SUCCESS) {
      cmn_label_t *lbl = NULL;
      rc = cmn_label_create(&lbl);
      if (rc != CMN_SUCCESS) {
        return rc;
      }
      *out_widget = lbl;
    }
  }
  return rc;
}

/**
 * @brief Route factory that always fails with `CMN_ERR_IO`.
 * @param userdata Unused user data.
 * @param params Unused parameter array.
 * @param param_count Unused parameter count.
 * @param out_widget Output widget pointer set to NULL.
 * @return CMN_ERR_IO.
 */
static cmn_error_e factory_fail(void *userdata, const char **params,
                                size_t param_count, void **out_widget) {
  if (out_widget != NULL) {
    *out_widget = (param_count > 0 && params != NULL) ? userdata : NULL;
  }
  return CMN_ERR_IO;
}

/**
 * @brief Tests route registration, navigation, view mounting, and view cleanup
 * across transitions.
 * @return Greatest test result code.
 */
TEST test_router_navigation(void) {
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  const char *path = NULL;
  struct captured_route_params_s cap;

  memset(&cap, 0, sizeof(cap));
  cap.create_widget = 1;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_register(router, "/home", factory_capture_params, &cap));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_register(router, "/about",
                                             factory_capture_params, &cap));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_register(router, "/user/:id",
                                             factory_capture_params, &cap));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_register(router, "/fail", factory_fail, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_router_navigate(router, "/home"));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &path));
  ASSERT_STR_EQ("/home", path);
  ASSERT_EQ(0u, cap.count);

  ASSERT_EQ(CMN_SUCCESS, cmn_router_navigate(router, "/user/123"));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &path));
  ASSERT_STR_EQ("/user/123", path);
  ASSERT_EQ(1u, cap.count);
  ASSERT_STR_EQ("123", cap.p0);

  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_router_navigate(router, "/unknown"));
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_router_navigate(router, "/home/extra"));
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_router_navigate(router, "/user/"));
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_router_navigate(router, "/user//extra"));
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_router_navigate(router, ""));

  ASSERT_EQ(CMN_ERR_IO, cmn_router_navigate(router, "/fail"));

  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));

  PASS();
}

/**
 * @brief Tests NULL argument handling across all router functions.
 * @return Greatest test result code.
 */
TEST test_router_nulls(void) {
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  const char *path = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_create(NULL, &router));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_create(container, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));

  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_router_register(NULL, "/home", factory_capture_params, NULL));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_router_register(router, NULL, factory_capture_params, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_register(router, "/home", NULL, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_navigate(NULL, "/home"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_navigate(router, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_get_current_path(NULL, &path));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_get_current_path(router, NULL));

  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_router_handle_deep_link(NULL, "app://host/path"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_handle_deep_link(router, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_destroy(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  router = NULL;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_destroy(&router));

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests router OOM and route capacity limits (`MAX_ROUTES = 32`).
 * @return Greatest test result code.
 */
TEST test_router_oom(void) {
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  int i;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));

  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));

  for (i = 0; i < 32; i++) {
    char path[32];
    ASSERT_EQ(CMN_SUCCESS, cmn_sprintf_s(path, sizeof(path), "/path%d", i));
    ASSERT_EQ(CMN_SUCCESS,
              cmn_router_register(router, path, factory_capture_params, NULL));
  }

  ASSERT_EQ(CMN_ERR_NOMEM, cmn_router_register(router, "/path32",
                                               factory_capture_params, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests `cmn_router_get_current_path` before any navigation has
 * occurred.
 * @return Greatest test result code.
 */
TEST test_router_get_empty_path(void) {
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  const char *path = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &path));
  ASSERT_STR_EQ("", path);

  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests `cmn_router_handle_deep_link` with host+path, host-only, root
 * scheme, and query/fragment stripping.
 * @return Greatest test result code.
 */
TEST test_router_deep_link(void) {
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  const char *path = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_register(router, "/", factory_capture_params, NULL));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_register(router, "/home", factory_capture_params, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_register(router, "/login",
                                             factory_capture_params, NULL));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_handle_deep_link(router, "app://host/home?foo=bar#sec"));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &path));
  ASSERT_STR_EQ("/home", path);

  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_handle_deep_link(router, "app://login?redirect=1"));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &path));
  ASSERT_STR_EQ("/login", path);

  ASSERT_EQ(CMN_SUCCESS, cmn_router_handle_deep_link(router, "app://"));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &path));
  ASSERT_STR_EQ("/", path);

  ASSERT_EQ(CMN_SUCCESS, cmn_router_handle_deep_link(router, "/home#top"));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &path));
  ASSERT_STR_EQ("/home", path);

  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests multi-segment route parameter extraction and null-termination.
 * @return Greatest test result code.
 */
TEST test_router_param_matching(void) {
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  struct captured_route_params_s cap;

  memset(&cap, 0, sizeof(cap));

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_register(router, "/org/:org_id/user/:user_id/settings",
                                factory_capture_params, &cap));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_register(router, "/:p1/:p2/:p3/:p4/:p5/:p6/:p7/:p8/:p9",
                                factory_capture_params, &cap));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_navigate(router, "/org/acme/user/42/settings"));
  ASSERT_EQ(2u, cap.count);
  ASSERT_STR_EQ("acme", cap.p0);
  ASSERT_STR_EQ("42", cap.p1);

  ASSERT_EQ(CMN_SUCCESS, cmn_router_navigate(router, "/1/2/3/4/5/6/7/8/9"));
  ASSERT_EQ((size_t)CMN_MAX_ROUTE_PARAMS, cap.count);

  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests router path length overflow handling during registration and
 * navigation.
 * @return Greatest test result code.
 */
TEST test_router_navigate_oom(void) {
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  char long_path[300];
  struct captured_route_params_s cap;

  memset(&cap, 0, sizeof(cap));
  cap.create_widget = 1;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));

  memset(long_path, 'a', sizeof(long_path) - 1);
  long_path[sizeof(long_path) - 1] = '\0';
  long_path[0] = '/';

#if !defined(_MSC_VER)
  ASSERT_EQ(CMN_ERR_IO, cmn_router_register(router, long_path,
                                            factory_capture_params, &cap));
#endif
  ASSERT_EQ(CMN_ERR_INVAL, cmn_router_navigate(router, long_path));

  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Test suite for router operations.
 */
SUITE(router_suite) {
  RUN_TEST(test_router_navigation);
  RUN_TEST(test_router_nulls);
  RUN_TEST(test_router_oom);
  RUN_TEST(test_router_get_empty_path);
  RUN_TEST(test_router_deep_link);
  RUN_TEST(test_router_param_matching);
  RUN_TEST(test_router_navigate_oom);
}
