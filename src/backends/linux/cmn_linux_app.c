/* clang-format off */
#include "cmn/app.h"
#include "cmn/memory.h"
#include <gtk/gtk.h>
/* clang-format on */

/**
 * @file cmn_linux_app.c
 * @brief Linux GTK3 implementation of the application lifecycle.
 */

/**
 * @brief Internal representation of a Linux GTK3 application instance.
 */
struct cmn_app_s {
  int is_running; /**< Non-zero while GTK3 main loop is running. */
};

/**
 * @brief Creates a new application instance.
 * @param out_app Pointer to store the created application handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_create(cmn_app_t **out_app) {
  cmn_error_e rc;
  cmn_app_t *app = NULL;
  int argc = 0;
  char **argv = NULL;

  if (out_app == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_app_t), (void **)&app);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  if (!gtk_init_check(&argc, &argv)) {
    rc = cmn_free((void **)&app);
    return rc != CMN_SUCCESS ? rc : CMN_ERR_IO;
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
  gtk_main();
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

  if (app->is_running) {
    app->is_running = 0;
#ifndef CMN_TESTING
    gtk_main_quit();
#endif
  }

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
 * @brief Heap-allocated task payload for GTK3 idle dispatch.
 */
struct task_data {
  cmn_error_e (*task)(void *); /**< Task callback function. */
  void *userdata;              /**< User data pointer. */
};
/**
 * @brief GTK3 idle callback adapter executing a posted cmn task.
 * @param user_data Pointer to `struct task_data`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e cmn_linux_idle_task(gpointer user_data) {
  struct task_data *td = (struct task_data *)user_data;
  cmn_error_e rc = CMN_SUCCESS;
  if (td != NULL) {
    cmn_error_e free_rc;
    if (td->task != NULL) {
      rc = td->task(td->userdata);
    }
    free_rc = cmn_free((void **)&td);
    if (rc == CMN_SUCCESS) {
      rc = free_rc;
    }
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
  {
    struct task_data *td = NULL;
    cmn_error_e rc = cmn_malloc(sizeof(struct task_data), (void **)&td);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
    td->task = task;
    td->userdata = userdata;
    return cmn_linux_idle_task(td);
  }
#else
  {
    struct task_data *td = NULL;
    cmn_error_e rc = cmn_malloc(sizeof(struct task_data), (void **)&td);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
    td->task = task;
    td->userdata = userdata;
    g_idle_add((GSourceFunc)cmn_linux_idle_task, td);
  }
  return CMN_SUCCESS;
#endif
}
