/* clang-format off */
#include "handle_map.h"
#include "cmn/memory.h"
#include <c_abstract_http/thread_pool.h>
/* clang-format on */

/**
 * @file handle_map.c
 * @brief Implementation of the handle map for mapping native handles to
 * widgets.
 */

/**
 * @def CMN_HANDLE_MAP_CAPACITY
 * @brief The maximum number of entries in the handle map.
 */
#define CMN_HANDLE_MAP_CAPACITY 1024

/**
 * @struct cmn_handle_entry_s
 * @brief An entry in the handle map.
 */
struct cmn_handle_entry_s {
  /** @brief The native handle. */
  void *native_handle;
  /** @brief The cmn widget. */
  void *cmn_widget;
};

/**
 * @brief The global handle map array.
 */
static struct cmn_handle_entry_s g_handle_map[CMN_HANDLE_MAP_CAPACITY];
/**
 * @brief The global mutex for thread-safe access to the handle map.
 */
static struct AbstractHttpMutex *g_handle_map_mutex = NULL;
/**
 * @brief Initialization flag for the handle map.
 */
static int g_handle_map_initialized = 0;

#ifdef CMN_TESTING
/**
 * @brief Flag to mock mutex initialization failure.
 */
static int g_cmn_mock_mutex_init_fail = 0;
/**
 * @brief Mock function to force mutex initialization failure.
 * @param fail 1 to fail, 0 to succeed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_mock_mutex_init_set(int fail) {
  g_cmn_mock_mutex_init_fail = fail;
  return CMN_SUCCESS;
}
/**
 * @brief Flag to mock mutex lock failure.
 */
static int g_cmn_mock_mutex_lock_fail = 0;
/**
 * @brief Mock function to force mutex lock failure.
 * @param fail 1 to fail, 0 to succeed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_mock_mutex_lock_set(int fail) {
  g_cmn_mock_mutex_lock_fail = fail;
  return CMN_SUCCESS;
}
/**
 * @brief Flag to mock mutex unlock failure.
 */
static int g_cmn_mock_mutex_unlock_fail = 0;
/**
 * @brief Mock function to force mutex unlock failure.
 * @param fail 1 to fail, 0 to succeed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_mock_mutex_unlock_set(int fail) {
  g_cmn_mock_mutex_unlock_fail = fail;
  return CMN_SUCCESS;
}
/**
 * @brief Flag to mock mutex free failure.
 */
static int g_cmn_mock_mutex_free_fail = 0;
/**
 * @brief Mock function to force mutex free failure.
 * @param fail 1 to fail, 0 to succeed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_mock_mutex_free_set(int fail) {
  g_cmn_mock_mutex_free_fail = fail;
  return CMN_SUCCESS;
}
/** @brief Test macro wrapping `abstract_http_mutex_init`. */
#define abstract_http_mutex_init(x)                                            \
  (g_cmn_mock_mutex_init_fail ? -1 : abstract_http_mutex_init(x))
/** @brief Test macro wrapping `abstract_http_mutex_lock`. */
#define abstract_http_mutex_lock(x)                                            \
  (g_cmn_mock_mutex_lock_fail ? -1 : abstract_http_mutex_lock(x))
/** @brief Test macro wrapping `abstract_http_mutex_unlock`. */
#define abstract_http_mutex_unlock(x)                                          \
  ((abstract_http_mutex_unlock(x) != 0 || g_cmn_mock_mutex_unlock_fail) ? -1   \
                                                                        : 0)
/** @brief Test macro wrapping `abstract_http_mutex_free`. */
#define abstract_http_mutex_free(x)                                            \
  (g_cmn_mock_mutex_free_fail ? -1 : abstract_http_mutex_free(x))
#endif

/**
 * @brief Initializes the handle map.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_init(void) {
  if (g_handle_map_initialized) {
    return CMN_SUCCESS;
  }

  if (abstract_http_mutex_init(&g_handle_map_mutex) != 0) {
    return CMN_ERR_NOMEM;
  }

  g_handle_map_initialized = 1;
  return CMN_SUCCESS;
}

/**
 * @brief Adds an entry to the handle map.
 * @param native_handle The native handle.
 * @param cmn_widget The cmn widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_add(void *native_handle, void *cmn_widget) {
  int i;

  if (!g_handle_map_initialized) {
    return CMN_ERR_INVAL;
  }
  if (native_handle == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget == NULL) {
    return CMN_ERR_INVAL;
  }

  if (abstract_http_mutex_lock(g_handle_map_mutex) != 0) {
    return CMN_ERR_IO;
  }
  for (i = 0; i < CMN_HANDLE_MAP_CAPACITY; ++i) {
    if (g_handle_map[i].native_handle == NULL) {
      g_handle_map[i].native_handle = native_handle;
      g_handle_map[i].cmn_widget = cmn_widget;
      if (abstract_http_mutex_unlock(g_handle_map_mutex) != 0) {
        return CMN_ERR_IO;
      }
      return CMN_SUCCESS;
    }
  }
  if (abstract_http_mutex_unlock(g_handle_map_mutex) != 0) {
    return CMN_ERR_IO;
  }
  return CMN_ERR_NOMEM;
}

/**
 * @brief Gets an entry from the handle map.
 * @param native_handle The native handle.
 * @param out_cmn_widget Pointer to store the output cmn widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_get(void *native_handle, void **out_cmn_widget) {
  int i;

  if (!g_handle_map_initialized) {
    return CMN_ERR_INVAL;
  }
  if (native_handle == NULL) {
    return CMN_ERR_INVAL;
  }
  if (out_cmn_widget == NULL) {
    return CMN_ERR_INVAL;
  }

  if (abstract_http_mutex_lock(g_handle_map_mutex) != 0) {
    return CMN_ERR_IO;
  }
  for (i = 0; i < CMN_HANDLE_MAP_CAPACITY; ++i) {
    if (g_handle_map[i].native_handle == native_handle) {
      *out_cmn_widget = g_handle_map[i].cmn_widget;
      if (abstract_http_mutex_unlock(g_handle_map_mutex) != 0) {
        return CMN_ERR_IO;
      }
      return CMN_SUCCESS;
    }
  }
  if (abstract_http_mutex_unlock(g_handle_map_mutex) != 0) {
    return CMN_ERR_IO;
  }
  return CMN_ERR_NOT_FOUND;
}

/**
 * @brief Removes an entry from the handle map.
 * @param native_handle The native handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_remove(void *native_handle) {
  int i;

  if (!g_handle_map_initialized) {
    return CMN_ERR_INVAL;
  }
  if (native_handle == NULL) {
    return CMN_ERR_INVAL;
  }

  if (abstract_http_mutex_lock(g_handle_map_mutex) != 0) {
    return CMN_ERR_IO;
  }
  for (i = 0; i < CMN_HANDLE_MAP_CAPACITY; ++i) {
    if (g_handle_map[i].native_handle == native_handle) {
      g_handle_map[i].native_handle = NULL;
      g_handle_map[i].cmn_widget = NULL;
      if (abstract_http_mutex_unlock(g_handle_map_mutex) != 0) {
        return CMN_ERR_IO;
      }
      return CMN_SUCCESS;
    }
  }
  if (abstract_http_mutex_unlock(g_handle_map_mutex) != 0) {
    return CMN_ERR_IO;
  }
  return CMN_ERR_NOT_FOUND;
}

/**
 * @brief Cleans up the handle map.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_handle_map_cleanup(void) {
  int i;

  if (!g_handle_map_initialized) {
    return CMN_SUCCESS;
  }

  if (abstract_http_mutex_free(g_handle_map_mutex) != 0) {
    return CMN_ERR_IO;
  }
  g_handle_map_mutex = NULL;
  g_handle_map_initialized = 0;

  for (i = 0; i < CMN_HANDLE_MAP_CAPACITY; ++i) {
    g_handle_map[i].native_handle = NULL;
    g_handle_map[i].cmn_widget = NULL;
  }

  return CMN_SUCCESS;
}
