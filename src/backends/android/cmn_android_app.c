/* clang-format off */
#include "cmn/app.h"
#include "cmn/memory.h"
#include <jni.h>
/* clang-format on */

/**
 * @file cmn_android_app.c
 * @brief Android SDK implementation of the application lifecycle.
 */

/**
 * @brief Internal representation of an Android application instance.
 */
struct cmn_app_s {
  int is_running; /**< Non-zero while Android activity loop is active. */
};

/**
 * @brief Creates a new application instance.
 * @param out_app Pointer to store the created application handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_create(cmn_app_t **out_app) {
  cmn_error_e rc;
  cmn_app_t *app = NULL;

  if (out_app == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_app_t), (void **)&app);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  app->is_running = 0;
  *out_app = app;
  return CMN_SUCCESS;
}

/**
 * @brief Starts the main event loop of the application.
 * @param app The application instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_run(cmn_app_t *app) {
  if (app == NULL) {
    return CMN_ERR_INVAL;
  }
  app->is_running = 1;
  return CMN_SUCCESS;
}

/**
 * @brief Signals the application's main event loop to quit.
 * @param app The application instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_quit(cmn_app_t *app) {
  if (app == NULL) {
    return CMN_ERR_INVAL;
  }
  app->is_running = 0;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys an application instance and frees its resources.
 * @param app Pointer to the application handle to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_destroy(cmn_app_t **app) {
  if (app == NULL || *app == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)app);
}

/**
 * @brief Posts a task to be executed on the main UI thread.
 * @param app The application instance.
 * @param task The function pointer to execute.
 * @param userdata User data passed to the task function.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_post_task(cmn_app_t *app, cmn_error_e (*task)(void *),
                              void *userdata) {
  if (app == NULL || task == NULL) {
    return CMN_ERR_INVAL;
  }
  return task(userdata);
}
