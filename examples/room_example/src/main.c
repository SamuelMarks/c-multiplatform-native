/* clang-format off */
#include "room_app.h"
#include <stdio.h>
/* clang-format on */

/**
 * @file main.c
 * @brief Entry point for the Room ORM example executable.
 */

/**
 * @brief Runs the Room ORM example and returns 0 on success.
 * @return Process exit code.
 */
int main(void) {
  int32_t found_id = 0;
  cmn_error_e rc = room_example_run(&found_id);
  if (rc != CMN_SUCCESS) {
    printf("Room example failed: " CMN_INT32_FMT "\n", (int)rc);
    return (int)rc;
  }
  printf("Successfully queried task id=" CMN_INT32_FMT "\n", (int)found_id);
  return 0;
}
