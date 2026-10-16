#ifndef CMN_APP_H
#define CMN_APP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
/* clang-format on */

/**
 * @file app.h
 * @brief Application lifecycle management.
 */

/**
 * @defgroup cmn_app Application Lifecycle
 * @brief Application Lifecycle module.
 * @{
 */

/**
 * @brief Creates a new application instance.
 * @param out_app Pointer to store the created application handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_app_create(cmn_app_t **out_app);

/**
 * @brief Starts the main event loop of the application.
 * @param app The application instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_app_run(cmn_app_t *app);

/**
 * @brief Signals the application's main event loop to quit.
 * @param app The application instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_app_quit(cmn_app_t *app);

/**
 * @brief Destroys an application instance and frees its resources.
 * @param app Pointer to the application handle to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_app_destroy(cmn_app_t **app);

/**
 * @brief Posts a task to be executed on the main UI thread.
 * @param app The application instance.
 * @param task The function pointer to execute.
 * @param userdata User data passed to the task function.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_app_post_task(cmn_app_t *app,
                                             cmn_error_e (*task)(void *),
                                             void *userdata);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_APP_H */
