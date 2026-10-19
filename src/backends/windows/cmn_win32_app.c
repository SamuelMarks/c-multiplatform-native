/* clang-format off */
#include "cmn/app.h"
#include "cmn/memory.h"
#include <winsock2.h>
#include <objbase.h>
/* clang-format on */

/**
 * @file cmn_win32_app.c
 * @brief Windows Win32 implementation of the application lifecycle.
 */

/** @brief Custom Win32 thread message ID for posted UI tasks. */
#define CMN_WM_TASK (WM_USER + 1)

/**
 * @brief Task callback function pointer type.
 */
typedef cmn_error_e (*cmn_task_fn)(void *);

/**
 * @brief Heap-allocated task payload passed via `LPARAM` to avoid ISO C
 * function-to-data pointer casts.
 */
struct win32_task_payload_s {
  cmn_task_fn task; /**< Task callback function. */
  void *userdata;   /**< User data pointer. */
};

/**
 * @brief Internal representation of a Win32 application instance.
 */
struct cmn_app_s {
  int is_running;  /**< Non-zero while message loop is running. */
  DWORD thread_id; /**< Win32 thread ID of the UI thread. */
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

#ifndef CMN_TESTING
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
#endif

  app->is_running = 0;
  app->thread_id = GetCurrentThreadId();
  *out_app = app;
  return CMN_SUCCESS;
}

/**
 * @brief Starts the main event loop of the application.
 * @param app The application instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_run(cmn_app_t *app) {
  cmn_error_e rc = CMN_SUCCESS;
  if (app == NULL) {
    return CMN_ERR_INVAL;
  }

  app->is_running = 1;

#ifndef CMN_TESTING
  {
    MSG msg;
    while (app->is_running && GetMessageW(&msg, NULL, 0, 0) > 0) {
      if (msg.message == CMN_WM_TASK) {
        struct win32_task_payload_s *payload =
            (struct win32_task_payload_s *)msg.lParam;
        if (payload != NULL) {
          cmn_error_e free_rc;
          if (payload->task != NULL) {
            cmn_error_e task_rc = payload->task(payload->userdata);
            rc = (rc != CMN_SUCCESS) ? rc : task_rc;
          }
          free_rc = cmn_free((void **)&payload);
          rc = (rc != CMN_SUCCESS) ? rc : free_rc;
        }
      } else {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
      }
    }
  }
#endif

  return rc;
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
  PostQuitMessage(0);

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

#ifndef CMN_TESTING
  CoUninitialize();
#endif

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

#ifdef CMN_TESTING
  return task(userdata);
#else
  {
    struct win32_task_payload_s *payload = NULL;
    cmn_error_e rc =
        cmn_malloc(sizeof(struct win32_task_payload_s), (void **)&payload);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
    payload->task = task;
    payload->userdata = userdata;
    if (!PostThreadMessageW(app->thread_id, CMN_WM_TASK, 0, (LPARAM)payload)) {
      rc = cmn_free((void **)&payload);
      return rc != CMN_SUCCESS ? rc : CMN_ERR_IO;
    }
  }
  return CMN_SUCCESS;
#endif
}
