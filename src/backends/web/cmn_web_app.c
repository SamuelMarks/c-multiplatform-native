/* clang-format off */
#include "cmn/app.h"
#include "cmn/memory.h"
#include <emscripten.h>
/* clang-format on */

/**
 * @file cmn_web_app.c
 * @brief Web (Emscripten) implementation of the application lifecycle.
 */

/**
 * @brief Internal representation of an Emscripten application instance.
 */
struct cmn_app_s {
  int is_running; /**< Non-zero while main loop is active. */
};

/**
 * @brief Emscripten main loop iteration callback.
 * @param arg Pointer to `cmn_app_t`.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e cmn_emscripten_loop_iteration(void *arg) {
  const cmn_app_t *app = (const cmn_app_t *)arg;
  if (!app->is_running) {
    emscripten_cancel_main_loop();
  }
  return CMN_SUCCESS;
}

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
#ifndef CMN_TESTING
  emscripten_set_main_loop_arg(
      (em_arg_callback_func)cmn_emscripten_loop_iteration, app, 0, 1);
#else
  (void)cmn_emscripten_loop_iteration;
#endif

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
#ifndef CMN_TESTING
  emscripten_cancel_main_loop();
#endif

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
 * @brief Heap-allocated task payload for `emscripten_async_call`.
 */
struct web_task_payload_s {
  cmn_error_e (*task)(void *); /**< Task callback function. */
  void *userdata;              /**< User data pointer. */
};
/**
 * @brief Emscripten async callback adapter.
 * @param arg Pointer to `struct web_task_payload_s`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e cmn_web_async_task_adapter(void *arg) {
  struct web_task_payload_s *payload = (struct web_task_payload_s *)arg;
  cmn_error_e rc = CMN_SUCCESS;
  if (payload != NULL) {
    if (payload->task != NULL) {
      rc = payload->task(payload->userdata);
    }
    (void)cmn_free((void **)&payload);
  }
  return rc;
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

#ifdef CMN_TESTING
  (void)cmn_web_async_task_adapter;
  return task(userdata);
#else
  {
    struct web_task_payload_s *payload = NULL;
    cmn_error_e rc =
        cmn_malloc(sizeof(struct web_task_payload_s), (void **)&payload);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
    payload->task = task;
    payload->userdata = userdata;
    emscripten_async_call((em_arg_callback_func)cmn_web_async_task_adapter,
                          payload, 0);
  }
  return CMN_SUCCESS;
#endif
}
