#ifndef CMN_HANDLE_MAP_H
#define CMN_HANDLE_MAP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/widget.h"
/* clang-format on */

/**
 * @file handle_map.h
 * @brief Thread-safe bidirectional mapping from native OS handles to cmn
 * widgets.
 */

/**
 * @defgroup cmn_handle_map Native Handle Map
 * @brief Native Handle Map module.
 * @{
 */

/**
 * @brief Initializes the handle map.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_init(void);

/**
 * @brief Adds an entry to the handle map.
 * @param native_handle The native handle.
 * @param cmn_widget The cmn widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_add(void *native_handle,
                                              void *cmn_widget);

/**
 * @brief Gets an entry from the handle map.
 * @param native_handle The native handle.
 * @param out_cmn_widget Pointer to store the output cmn widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_get(void *native_handle,
                                              void **out_cmn_widget);

/**
 * @brief Removes an entry from the handle map.
 * @param native_handle The native handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_remove(void *native_handle);

/**
 * @brief Cleans up the handle map.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_cleanup(void);

#ifdef CMN_TESTING
/**
 * @brief Mock function to force mutex initialization failure.
 * @param fail 1 to fail, 0 to succeed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_mock_mutex_init_set(int fail);

/**
 * @brief Mock function to force mutex lock failure.
 * @param fail 1 to fail, 0 to succeed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_mock_mutex_lock_set(int fail);

/**
 * @brief Mock function to force mutex unlock failure.
 * @param fail 1 to fail, 0 to succeed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_mock_mutex_unlock_set(int fail);

/**
 * @brief Mock function to force mutex free failure.
 * @param fail 1 to fail, 0 to succeed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_handle_map_mock_mutex_free_set(int fail);
#endif

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_HANDLE_MAP_H */
