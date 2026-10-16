/* clang-format off */
#include "app_state.h"
#include "login_view.h"
#include "welcome_view.h"
#include <cmn/app.h>
#include <cmn/error.h>
#include <cmn/router.h>
#include <cmn/widget.h>
#include <stdio.h>
/* clang-format on */

/**
 * @file main.c
 * @brief Entry point for simple password flow example.
 */

/**
 * @brief Main execution function for the simple password flow application.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e app_main(void);

cmn_error_e app_main(void) {
  cmn_error_e rc;
  cmn_error_e c1_rc;
  cmn_error_e c2_rc;
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_router_t *router = NULL;
  app_state_t state;

  rc = cmn_app_create(&app);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_window_create(app, &win);
  if (rc != CMN_SUCCESS) {
    (void)cmn_app_destroy(&app);
    return rc;
  }

  rc = cmn_window_set_title(win, "Simple Password Flow");
  if (rc != CMN_SUCCESS) {
    (void)cmn_window_destroy(&win);
    (void)cmn_app_destroy(&app);
    return rc;
  }

  rc = cmn_router_create(win, &router);
  if (rc != CMN_SUCCESS) {
    (void)cmn_window_destroy(&win);
    (void)cmn_app_destroy(&app);
    return rc;
  }

  rc = app_state_init(&state, router);
  if (rc != CMN_SUCCESS) {
    (void)cmn_router_destroy(&router);
    (void)cmn_window_destroy(&win);
    (void)cmn_app_destroy(&app);
    return rc;
  }
  rc = cmn_router_register(router, "/login", login_view_factory, &state);
  if (rc != CMN_SUCCESS) {
    (void)cmn_router_destroy(&router);
    (void)cmn_window_destroy(&win);
    (void)cmn_app_destroy(&app);
    return rc;
  }
  rc = cmn_router_register(router, "/welcome", welcome_view_factory, &state);
  if (rc != CMN_SUCCESS) {
    (void)cmn_router_destroy(&router);
    (void)cmn_window_destroy(&win);
    (void)cmn_app_destroy(&app);
    return rc;
  }

  rc = cmn_router_navigate(router, "/login");
  if (rc != CMN_SUCCESS) {
    c1_rc = login_view_cleanup(0);
    c2_rc = welcome_view_cleanup(0);
    (void)cmn_router_destroy(&router);
    (void)cmn_window_destroy(&win);
    (void)cmn_app_destroy(&app);
    return rc != CMN_SUCCESS ? rc : (c1_rc != CMN_SUCCESS ? c1_rc : c2_rc);
  }

  rc = cmn_window_show(win);
  if (rc == CMN_SUCCESS) {
    rc = cmn_app_run(app);
  }

  c1_rc = login_view_cleanup(0);
  c2_rc = welcome_view_cleanup(0);
  (void)cmn_router_destroy(&router);
  (void)cmn_window_destroy(&win);
  (void)cmn_app_destroy(&app);

  return rc != CMN_SUCCESS ? rc : (c1_rc != CMN_SUCCESS ? c1_rc : c2_rc);
}

#ifndef CMN_TESTING
/**
 * @brief Standard C main entry point.
 * @return Process exit code.
 */
int main(void) {
  cmn_error_e rc = app_main();
  if (rc != CMN_SUCCESS) {
    printf("Password flow app failed: " CMN_INT32_FMT "\n", (int)rc);
    return (int)rc;
  }
  return 0;
}
#endif
