/* clang-format off */
#include "cmn/memory.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @file memory.c
 * @brief Implementation of memory allocation wrappers.
 */

#ifdef CMN_TESTING
/**
 * @brief Flag to mock allocation failure.
 */
static int g_cmn_mock_alloc_fail = 0;

/**
 * @brief Counter to mock allocation failure after a number of allocations.
 */
static int g_cmn_mock_alloc_count = 0;

/**
 * @brief Configures mock allocation failure for testing OOM conditions.
 * @param fail Whether to fail (1) or not (0).
 * @param count How many allocations to wait before failing.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_alloc_set(int fail, int count) {
  g_cmn_mock_alloc_fail = fail;
  g_cmn_mock_alloc_count = count;
  return CMN_SUCCESS;
}
#endif

/**
 * @brief Allocates uninitialized memory.
 * @param size Number of bytes to allocate.
 * @param out_ptr Pointer to store the allocated memory address.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_malloc(size_t size, void **out_ptr) {
  if (out_ptr == NULL) {
    return CMN_ERR_INVAL;
  }
#ifdef CMN_TESTING
  if (g_cmn_mock_alloc_fail) {
    if (g_cmn_mock_alloc_count <= 0) {
      *out_ptr = NULL;
      return CMN_ERR_NOMEM;
    } else {
      g_cmn_mock_alloc_count--;
    }
  }
#endif

  if (size == 0) {
    *out_ptr = NULL;
    return CMN_SUCCESS;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return CMN_ERR_NOMEM;
  }

  return CMN_SUCCESS;
}

/**
 * @brief Allocates zero-initialized memory for an array.
 * @param count Number of elements.
 * @param size Size of each element.
 * @param out_ptr Pointer to store the allocated memory address.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_calloc(size_t count, size_t size, void **out_ptr) {
  if (out_ptr == NULL) {
    return CMN_ERR_INVAL;
  }
#ifdef CMN_TESTING
  if (g_cmn_mock_alloc_fail) {
    if (g_cmn_mock_alloc_count <= 0) {
      *out_ptr = NULL;
      return CMN_ERR_NOMEM;
    } else {
      g_cmn_mock_alloc_count--;
    }
  }
#endif

  if (count == 0 || size == 0) {
    *out_ptr = NULL;
    return CMN_SUCCESS;
  }

  *out_ptr = calloc(count, size);
  if (*out_ptr == NULL) {
    return CMN_ERR_NOMEM;
  }

  return CMN_SUCCESS;
}

/**
 * @brief Reallocates a memory block.
 * @param ptr Existing memory block (can be NULL).
 * @param size New size in bytes.
 * @param out_ptr Pointer to store the reallocated memory address.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_realloc(void *ptr, size_t size, void **out_ptr) {
  void *new_ptr;

  if (out_ptr == NULL) {
    return CMN_ERR_INVAL;
  }
#ifdef CMN_TESTING
  if (g_cmn_mock_alloc_fail) {
    if (g_cmn_mock_alloc_count <= 0) {
      *out_ptr = NULL;
      return CMN_ERR_NOMEM;
    } else {
      g_cmn_mock_alloc_count--;
    }
  }
#endif

  if (size == 0) {
    free(ptr);
    *out_ptr = NULL;
    return CMN_SUCCESS;
  }

  new_ptr = realloc(ptr, size);
  if (new_ptr == NULL) {
    return CMN_ERR_NOMEM;
  }

  *out_ptr = new_ptr;
  return CMN_SUCCESS;
}

/**
 * @brief Frees allocated memory and sets the pointer to NULL.
 * @param ptr Pointer to the memory pointer to be freed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_free(void **ptr) {
  if (ptr == NULL) {
    return CMN_ERR_INVAL;
  }

  if (*ptr != NULL) {
    free(*ptr);
    *ptr = NULL;
  }

  return CMN_SUCCESS;
}
