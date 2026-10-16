#ifndef CMN_ERROR_H
#define CMN_ERROR_H

/* clang-format off */
#if defined(_MSC_VER) && _MSC_VER >= 1700
#include <sal.h>
#endif
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file error.h
 * @brief Error codes and compiler warning annotations for cmn.
 */

/**
 * @defgroup cmn_error Error Handling
 * @brief Error Handling module.
 * @{
 */

/** @brief Compiler attribute to warn if a function's return value is discarded.
 */
#if defined(__clang__) || defined(__GNUC__)
#define CMN_NO_DISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
#define CMN_NO_DISCARD _Check_return_
#else
#define CMN_NO_DISCARD
#endif

/**
 * @brief Error codes returned by cmn functions.
 */
typedef enum cmn_error_e {
  /** @brief Operation succeeded. */
  CMN_SUCCESS = 0,
  /** @brief Out of memory. */
  CMN_ERR_NOMEM = 1,
  /** @brief Invalid argument. */
  CMN_ERR_INVAL = 2,
  /** @brief Requested resource was not found. */
  CMN_ERR_NOT_FOUND = 3,
  /** @brief I/O or system operation failed. */
  CMN_ERR_IO = 4
} cmn_error_e;

/**
 * @brief Gets a string representation of an error code.
 * @param error The error code.
 * @param out_str Pointer to store the resulting static string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_error_string(cmn_error_e error,
                                            const char **out_str);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_ERROR_H */
