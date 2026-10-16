#ifndef CMN_MEMORY_H
#define CMN_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file memory.h
 * @brief Memory allocation and deallocation functions.
 */

/**
 * @defgroup cmn_memory Memory Management
 * @brief Memory Management module.
 * @{
 */

/**
 * @brief Allocates uninitialized memory.
 * @param size Number of bytes to allocate.
 * @param out_ptr Pointer to store the allocated memory address.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_malloc(size_t size, void **out_ptr);

/**
 * @brief Allocates zero-initialized memory for an array.
 * @param count Number of elements.
 * @param size Size of each element.
 * @param out_ptr Pointer to store the allocated memory address.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_calloc(size_t count, size_t size,
                                      void **out_ptr);

/**
 * @brief Reallocates a memory block.
 * @param ptr Existing memory block (can be NULL).
 * @param size New size in bytes.
 * @param out_ptr Pointer to store the reallocated memory address.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_realloc(void *ptr, size_t size, void **out_ptr);

/**
 * @brief Frees allocated memory and sets the pointer to NULL.
 * @param ptr Pointer to the memory pointer to be freed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_free(void **ptr);

#ifdef CMN_TESTING
/**
 * @brief Configures mock allocation failure for testing OOM conditions.
 * @param fail Whether to fail (1) or not (0).
 * @param count How many allocations to wait before failing.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_mock_alloc_set(int fail, int count);
#endif

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_MEMORY_H */
