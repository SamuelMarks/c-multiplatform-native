/* clang-format off */
#include "app.h"
#include <stdio.h>
/* clang-format on */

/**
 * @file main.c
 * @brief Entry point for the Hello World GUI counter application.
 */

/**
 * @brief Initializes, runs, and destroys the Hello World GUI counter
 * application.
 * @return Process exit code (`0` on `CMN_SUCCESS`).
 */
int main(void) {
  hello_world_app_t *app = NULL;
  cmn_error_e rc;
  cmn_error_e destroy_rc;

  rc = hello_world_app_init(&app);
  if (rc != CMN_SUCCESS) {
    printf("Failed to initialize app: " CMN_INT32_FMT "\n", (int)rc);
    return (int)rc;
  }

  rc = hello_world_app_run(app);
  if (rc != CMN_SUCCESS) {
    printf("App run failed: " CMN_INT32_FMT "\n", (int)rc);
  }

  destroy_rc = hello_world_app_destroy(&app);
  if (destroy_rc != CMN_SUCCESS) {
    printf("App destruction failed: " CMN_INT32_FMT "\n", (int)destroy_rc);
    if (rc == CMN_SUCCESS) {
      rc = destroy_rc;
    }
  }

  return (int)rc;
}
