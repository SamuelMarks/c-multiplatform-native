/* clang-format off */
#include "cmn/string_safe.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
    #include <string.h>
#endif
/* clang-format on */

/**
 * @file string_safe.c
 * @brief Implementation of safe string wrappers.
 */

#ifdef CMN_TESTING
/**
 * @brief Flag to mock vsnprintf failure.
 */
static int g_cmn_mock_vsnprintf_fail = 0;

/**
 * @brief Sets mock failure state for vsnprintf in cmn_sprintf_s.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_vsnprintf_set(int fail) {
  g_cmn_mock_vsnprintf_fail = fail;
  return CMN_SUCCESS;
}
#endif

/**
 * @brief Safely copies a string to a destination buffer.
 * @param dest Destination buffer.
 * @param dest_size Size of the destination buffer in bytes.
 * @param src Source string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_strcpy_s(char *dest, size_t dest_size, const char *src) {
  if (dest == NULL || src == NULL || dest_size == 0) {
    return CMN_ERR_INVAL;
  }

#if defined(_MSC_VER)
  if (strcpy_s(dest, dest_size, src) != 0) {
    return CMN_ERR_IO;
  }
#else
  {
    size_t src_len;
    src_len = strlen(src);

    if (src_len >= dest_size) {
      /* Truncate and null-terminate for safety, but return an error */
      strncpy(dest, src, dest_size - 1);
      *(dest + dest_size - 1) = '\0';
      return CMN_ERR_IO;
    }

    strcpy(dest, src);
  }
#endif

  return CMN_SUCCESS;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

/**
 * @brief Safely formats and writes data to a string buffer.
 * @param dest Destination buffer.
 * @param dest_size Size of the destination buffer in bytes.
 * @param format Format string.
 * @param ... Additional arguments for formatting.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_sprintf_s(char *dest, size_t dest_size, const char *format,
                          ...) {
  int ret;
  va_list args;

  if (dest == NULL || format == NULL || dest_size == 0) {
    return CMN_ERR_INVAL;
  }

  va_start(args, format);

#if defined(_MSC_VER)
  ret = vsprintf_s(dest, dest_size, format, args);
#ifdef CMN_TESTING
  if (g_cmn_mock_vsnprintf_fail)
    ret = -1;
#endif
  va_end(args);
  if (ret < 0) {
    return CMN_ERR_IO;
  }
#else
  ret = vsnprintf(dest, dest_size, format, args);
#ifdef CMN_TESTING
  if (g_cmn_mock_vsnprintf_fail)
    ret = -1;
#endif

  va_end(args);

  if (ret < 0 || (size_t)ret >= dest_size) {
    return CMN_ERR_IO;
  }
#endif

  return CMN_SUCCESS;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
