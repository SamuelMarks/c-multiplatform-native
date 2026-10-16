/* clang-format off */
#include "cmn/error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file error.c
 * @brief Implementation of error handling functions.
 */

/**
 * @brief Gets a string representation of an error code.
 * @param error The error code.
 * @param out_str Pointer to store the resulting static string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_error_string(cmn_error_e error, const char **out_str) {
  if (out_str == NULL) {
    return CMN_ERR_INVAL;
  }

  switch (error) {
  case CMN_SUCCESS:
    *out_str = "Success";
    break;
  case CMN_ERR_NOMEM:
    *out_str = "Out of memory";
    break;
  case CMN_ERR_INVAL:
    *out_str = "Invalid argument";
    break;
  case CMN_ERR_NOT_FOUND:
    *out_str = "Not found";
    break;
  case CMN_ERR_IO:
    *out_str = "I/O error";
    break;
  default:
    *out_str = "Unknown error";
    break;
  }

  return CMN_SUCCESS;
}
