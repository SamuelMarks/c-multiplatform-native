#ifndef CMN_STRING_SAFE_H
#define CMN_STRING_SAFE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include <stdarg.h>
#include <stddef.h>
/* clang-format on */

/**
 * @file string_safe.h
 * @brief Safe string manipulation functions (wrapping Safe CRT where
 * available).
 */

/**
 * @defgroup cmn_string_safe Safe String Primitives
 * @brief Safe String Primitives module.
 * @{
 */

/**
 * @def CMN_NUM_FORMAT
 * @brief Format specifier for 64-bit integers.
 */
#if defined(_MSC_VER)
#define CMN_NUM_FORMAT "%I64d"
#elif defined(__LP64__) || defined(_LP64)
#define CMN_NUM_FORMAT "%ld"
#else
#define CMN_NUM_FORMAT "%lld"
#endif

/**
 * @brief Safely copies a string to a destination buffer.
 * @param dest Destination buffer.
 * @param dest_size Size of the destination buffer in bytes.
 * @param src Source string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_strcpy_s(char *dest, size_t dest_size,
                                        const char *src);

/**
 * @brief Safely formats and writes data to a string buffer.
 * @param dest Destination buffer.
 * @param dest_size Size of the destination buffer in bytes.
 * @param format Format string.
 * @param ... Additional arguments for formatting.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_sprintf_s(char *dest, size_t dest_size,
                                         const char *format, ...);

#ifdef CMN_TESTING
/**
 * @brief Sets mock failure state for vsnprintf in cmn_sprintf_s.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_mock_vsnprintf_set(int fail);
#endif

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_STRING_SAFE_H */
