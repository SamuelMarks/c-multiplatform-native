/* clang-format off */
#include "cmn/signals.h"
#include "cmn/memory.h"
#include <c_abstract_http/thread_pool.h>
#include <stddef.h>
/* clang-format on */

/**
 * @file signals.c
 * @brief Implementation of reactive signals with dynamic dependency tracking,
 * cycle detection, and topological sorting.
 */

/**
 * @brief Defines the type of a node in the signal dependency graph.
 */
typedef enum {
  NODE_TYPE_SIGNAL,   /**< Writable signal node type. */
  NODE_TYPE_COMPUTED, /**< Computed (derived) node type. */
  NODE_TYPE_EFFECT    /**< Side-effect node type. */
} node_type_e;

/** @brief Forward declaration of a reactive graph node. */
typedef struct cmn_node_s cmn_node_t;

/**
 * @brief Base node structure used for graph traversal and dependency tracking.
 */
struct cmn_node_s {
  node_type_e type;        /**< The specific type of the node. */
  cmn_node_t **dependents; /**< Array of dependent nodes. */
  size_t num_dependents;   /**< Current number of dependents. */
  size_t cap_dependents;   /**< Capacity of the dependents array. */
  int mark; /**< DFS visit mark (0: unvisited, 1: visiting, 2: visited). */
};

/**
 * @brief Represents a writable signal state.
 */
struct cmn_signal_s {
  cmn_node_t base; /**< Base graph node properties. */
  int value;       /**< Current stored value. */
};

/**
 * @brief Represents a computed signal.
 */
struct cmn_computed_s {
  cmn_node_t base;               /**< Base graph node properties. */
  cmn_compute_int_fn compute_fn; /**< Function to compute the value. */
  void *userdata;   /**< Optional userdata for the compute function. */
  int cached_value; /**< Most recently computed and cached value. */
};

/**
 * @brief Represents an effect.
 */
struct cmn_effect_s {
  cmn_node_t base;         /**< Base graph node properties. */
  cmn_effect_fn effect_fn; /**< The callback function to run as an effect. */
  void *userdata;          /**< Optional userdata for the effect function. */
};

/** @brief Global mutex protecting the reactive signal graph. */
static struct AbstractHttpMutex *g_signal_mutex = NULL;
/** @brief Non-zero once `g_signal_mutex` has been initialized. */
static int g_signals_initialized = 0;

/** @brief Currently evaluating computed or effect node tracking dependencies.
 */
static cmn_node_t *g_current_tracker = NULL;

/** @brief Dynamically allocated array of all registered graph nodes. */
static cmn_node_t **g_all_nodes = NULL;
/** @brief Number of active nodes in `g_all_nodes`. */
static size_t g_num_nodes = 0;
/** @brief Allocated capacity of `g_all_nodes`. */
static size_t g_cap_nodes = 0;

#ifdef CMN_TESTING
/** @brief Mock flag to simulate mutex initialization failure in signals. */
static int g_cmn_mock_signals_init_fail = 0;

/**
 * @brief Sets mock failure state for signal subsystem initialization.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_signals_init_fail_set(int fail) {
  g_cmn_mock_signals_init_fail = fail;
  return CMN_SUCCESS;
}

/** @brief Mock flag to simulate unregister failure in signals. */
static int g_cmn_mock_signals_unregister_fail = 0;

/**
 * @brief Sets mock failure state for unregistering a node in signals.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_signals_unregister_fail_set(int fail) {
  g_cmn_mock_signals_unregister_fail = fail;
  return CMN_SUCCESS;
}
#endif

/**
 * @brief Combines two error codes, preserving the first failure encountered.
 * @param first Initial error code.
 * @param second Subsequent error code.
 * @return First non-success code, or CMN_SUCCESS.
 */
static cmn_error_e signals_accum_err(cmn_error_e first, cmn_error_e second) {
  return (first != CMN_SUCCESS) ? first : second;
}

/**
 * @brief Acquires the global signal graph mutex.
 * @return CMN_SUCCESS on success, CMN_ERR_IO otherwise.
 */
static cmn_error_e signals_lock(void) {
  return (abstract_http_mutex_lock(g_signal_mutex) == 0) ? CMN_SUCCESS
                                                         : CMN_ERR_IO;
}

/**
 * @brief Releases the global signal graph mutex.
 * @return CMN_SUCCESS on success, CMN_ERR_IO otherwise.
 */
static cmn_error_e signals_unlock(void) {
  return (abstract_http_mutex_unlock(g_signal_mutex) == 0) ? CMN_SUCCESS
                                                           : CMN_ERR_IO;
}

/**
 * @brief Releases the global signal graph mutex and frees it if no nodes
 * remain.
 * @return CMN_SUCCESS on success, CMN_ERR_IO otherwise.
 */
static cmn_error_e signals_unlock_after_unregister(void) {
  cmn_error_e rc = signals_unlock();
  if (g_num_nodes == 0 && g_signal_mutex != NULL && g_current_tracker == NULL) {
    int free_rc = abstract_http_mutex_free(g_signal_mutex);
    g_signal_mutex = NULL;
    g_signals_initialized = 0;
    rc = (free_rc == 0) ? rc : CMN_ERR_IO;
  }
  return rc;
}

/**
 * @brief Ensures the global signal mutex is initialized.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e ensure_signals_initialized(void) {
#ifdef CMN_TESTING
  if (g_cmn_mock_signals_init_fail) {
    return CMN_ERR_IO;
  }
#endif
  if (!g_signals_initialized) {
    int init_rc = abstract_http_mutex_init(&g_signal_mutex);
    g_signals_initialized = (init_rc == 0) ? 1 : 0;
  }
  return g_signals_initialized ? CMN_SUCCESS : CMN_ERR_IO;
}

/**
 * @brief Registers a node in the global graph.
 * @param node Pointer to the node to register.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e register_node(cmn_node_t *node) {
  if (g_num_nodes == g_cap_nodes) {
    void *new_ptr = NULL;
    cmn_error_e rc;
    size_t new_cap = g_cap_nodes == 0 ? 16 : g_cap_nodes * 2;
    rc = cmn_realloc(g_all_nodes, new_cap * sizeof(cmn_node_t *), &new_ptr);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
    g_cap_nodes = new_cap;
    g_all_nodes = (cmn_node_t **)new_ptr;
  }
  g_all_nodes[g_num_nodes++] = node;
  return CMN_SUCCESS;
}

/**
 * @brief Removes all incoming dependency edges pointing to `node` from other
 * nodes in the graph.
 * @param node The dependent node whose incoming edges should be cleared.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e clear_incoming_dependencies(cmn_node_t *node) {
  size_t i, j, k;
  for (i = 0; i < g_num_nodes; i++) {
    cmn_node_t *other = g_all_nodes[i];
    if (other != node && other->dependents != NULL) {
      for (j = 0; j < other->num_dependents; j++) {
        if (other->dependents[j] == node) {
          for (k = j; k + 1 < other->num_dependents; k++) {
            other->dependents[k] = other->dependents[k + 1];
          }
          other->num_dependents--;
          j--;
        }
      }
    }
  }
  return CMN_SUCCESS;
}

/**
 * @brief Unregisters a node from the global graph, frees its dependents list,
 * and cleans up global state when the last node is removed.
 * @param node Pointer to the node to unregister.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e unregister_node(cmn_node_t *node) {
  size_t i;
  cmn_error_e rc;

  rc = clear_incoming_dependencies(node);

  for (i = 0; i < g_num_nodes; i++) {
    if (g_all_nodes[i] == node) {
      g_all_nodes[i] = g_all_nodes[--g_num_nodes];
      break;
    }
  }
  if (node->dependents) {
    rc = signals_accum_err(rc, cmn_free((void **)&node->dependents));
  }
  if (g_num_nodes == 0 && g_all_nodes != NULL) {
    cmn_error_e free_rc = cmn_free((void **)&g_all_nodes);
    rc = signals_accum_err(rc, free_rc);
    g_cap_nodes = 0;
  }
#ifdef CMN_TESTING
  if (g_cmn_mock_signals_unregister_fail) {
    return CMN_ERR_IO;
  }
#endif
  return rc;
}

/**
 * @brief Adds a child node as a dependent to a given dependency node.
 * @param dep Pointer to the dependency node.
 * @param child Pointer to the child dependent node.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e add_dependent(cmn_node_t *dep, cmn_node_t *child) {
  if (dep->num_dependents == dep->cap_dependents) {
    void *new_ptr = NULL;
    cmn_error_e rc;
    size_t new_cap = dep->cap_dependents == 0 ? 4 : dep->cap_dependents * 2;
    rc = cmn_realloc(dep->dependents, new_cap * sizeof(cmn_node_t *), &new_ptr);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
    dep->cap_dependents = new_cap;
    dep->dependents = (cmn_node_t **)new_ptr;
  }
  {
    size_t i;
    for (i = 0; i < dep->num_dependents; i++) {
      if (dep->dependents[i] == child) {
        return CMN_SUCCESS;
      }
    }
  }
  dep->dependents[dep->num_dependents++] = child;
  return CMN_SUCCESS;
}

/**
 * @brief Resets all DFS visit marks in the global graph to unvisited (0).
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e reset_marks(void) {
  size_t i;
  for (i = 0; i < g_num_nodes; i++) {
    g_all_nodes[i]->mark = 0;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Performs depth-first search for topological sorting and cycle
 * detection.
 * @param node Pointer to the node to visit.
 * @param sorted Array to store the post-order visited nodes.
 * @param sorted_count Pointer to the current count of sorted nodes.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if a cycle is detected.
 */
static cmn_error_e dfs_topo(cmn_node_t *node, cmn_node_t **sorted,
                            size_t *sorted_count) {
  size_t i;
  if (node->mark == 2) {
    return CMN_SUCCESS;
  }
  if (node->mark == 1) {
    return CMN_ERR_INVAL;
  }

  node->mark = 1;
  for (i = 0; i < node->num_dependents; i++) {
    cmn_error_e inner_rc = dfs_topo(node->dependents[i], sorted, sorted_count);
    if (inner_rc != CMN_SUCCESS) {
      return inner_rc;
    }
  }
  node->mark = 2;
  sorted[*sorted_count] = node;
  (*sorted_count)++;
  return CMN_SUCCESS;
}

/**
 * @brief Propagates an update starting from a modified signal node in
 * topological order, re-tracking dynamic dependencies during evaluation.
 * @param start_node Pointer to the signal node that was updated.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e propagate_update(cmn_node_t *start_node) {
  cmn_node_t **sorted = NULL;
  size_t sorted_count = 0;
  cmn_error_e rc;
  size_t i;

  rc = reset_marks();
  rc = signals_accum_err(
      rc, cmn_calloc(g_num_nodes, sizeof(cmn_node_t *), (void **)&sorted));
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = dfs_topo(start_node, sorted, &sorted_count);
  if (rc != CMN_SUCCESS) {
    cmn_error_e free_rc = cmn_free((void **)&sorted);
    return (free_rc != CMN_SUCCESS) ? free_rc : rc;
  }

  for (i = sorted_count; i > 0; i--) {
    cmn_node_t *n = sorted[i - 1];
    if (n->type == NODE_TYPE_COMPUTED) {
      struct cmn_computed_s *comp = (struct cmn_computed_s *)n;
      cmn_node_t *prev_tracker = g_current_tracker;
      rc = signals_accum_err(rc, clear_incoming_dependencies(n));
      g_current_tracker = n;
      rc = signals_accum_err(rc, signals_unlock());
      {
        cmn_error_e eval_rc =
            comp->compute_fn(comp->userdata, &comp->cached_value);
        if (eval_rc != CMN_SUCCESS) {
          cmn_error_e free_rc;
          rc = signals_accum_err(eval_rc, signals_lock());
          g_current_tracker = prev_tracker;
          free_rc = cmn_free((void **)&sorted);
          return (free_rc != CMN_SUCCESS) ? free_rc : rc;
        }
      }
      rc = signals_accum_err(rc, signals_lock());
      g_current_tracker = prev_tracker;
    } else if (n->type == NODE_TYPE_EFFECT) {
      struct cmn_effect_s *eff = (struct cmn_effect_s *)n;
      cmn_node_t *prev_tracker = g_current_tracker;
      rc = signals_accum_err(rc, clear_incoming_dependencies(n));
      g_current_tracker = n;
      rc = signals_accum_err(rc, signals_unlock());
      {
        cmn_error_e eff_rc = eff->effect_fn(eff->userdata);
        if (eff_rc != CMN_SUCCESS) {
          cmn_error_e free_rc;
          rc = signals_accum_err(eff_rc, signals_lock());
          g_current_tracker = prev_tracker;
          free_rc = cmn_free((void **)&sorted);
          return (free_rc != CMN_SUCCESS) ? free_rc : rc;
        }
      }
      rc = signals_accum_err(rc, signals_lock());
      g_current_tracker = prev_tracker;
    }
  }

  return signals_accum_err(rc, cmn_free((void **)&sorted));
}

/* --- Signal (Writable) --- */

/**
 * @brief Creates a new integer signal.
 * @param initial_val The initial value of the signal.
 * @param out_sig Pointer to store the created signal.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_signal_int_create(int initial_val, cmn_signal_t **out_sig) {
  cmn_error_e rc;
  cmn_signal_t *sig = NULL;

  if (out_sig == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_calloc(1, sizeof(cmn_signal_t), (void **)&sig);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  sig->base.type = NODE_TYPE_SIGNAL;
  sig->value = initial_val;

  rc = signals_lock();
  rc = signals_accum_err(rc, register_node((cmn_node_t *)sig));
  if (rc != CMN_SUCCESS) {
    cmn_error_e free_rc = cmn_free((void **)&sig);
    rc = signals_accum_err(rc, signals_unlock());
    return (free_rc != CMN_SUCCESS) ? free_rc : rc;
  }
  rc = signals_unlock();

  *out_sig = sig;
  return rc;
}

/**
 * @brief Sets a new value for an integer signal, triggering dependents.
 * @param sig The signal to modify.
 * @param new_val The new value to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_signal_int_set(cmn_signal_t *sig, int new_val) {
  cmn_error_e rc;
  if (sig == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = signals_lock();

  if (sig->value != new_val) {
    sig->value = new_val;
    rc = signals_accum_err(rc, propagate_update((cmn_node_t *)sig));
    if (rc != CMN_SUCCESS) {
      return signals_accum_err(rc, signals_unlock());
    }
  }

  return signals_accum_err(rc, signals_unlock());
}

/**
 * @brief Gets the current value of an integer signal.
 * @param sig The signal to query.
 * @param out_val Pointer to store the current value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_signal_int_get(cmn_signal_t *sig, int *out_val) {
  cmn_error_e rc;
  if (sig == NULL || out_val == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = signals_lock();

  if (g_current_tracker != NULL) {
    cmn_error_e add_rc = add_dependent((cmn_node_t *)sig, g_current_tracker);
    if (add_rc != CMN_SUCCESS) {
      return signals_accum_err(add_rc, signals_unlock());
    }
  }

  *out_val = sig->value;
  return signals_accum_err(rc, signals_unlock());
}

/**
 * @brief Destroys a signal and frees its memory.
 * @param sig Pointer to the signal pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_signal_destroy(cmn_signal_t **sig) {
  cmn_error_e rc;
  if (sig == NULL || *sig == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = signals_lock();
  rc = signals_accum_err(rc, unregister_node((cmn_node_t *)*sig));
  if (rc != CMN_SUCCESS) {
    return signals_accum_err(rc, signals_unlock());
  }
  rc = signals_unlock_after_unregister();

  return signals_accum_err(rc, cmn_free((void **)sig));
}

/* --- Computed (Derived) --- */

/**
 * @brief Creates a new computed integer signal.
 * @param compute_fn The function to compute the value.
 * @param userdata User data passed to the compute function.
 * @param out_comp Pointer to store the created computed signal.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_computed_int_create(cmn_compute_int_fn compute_fn,
                                    void *userdata, cmn_computed_t **out_comp) {
  cmn_error_e rc;
  cmn_computed_t *comp = NULL;
  cmn_node_t *prev_tracker;

  if (compute_fn == NULL || out_comp == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_calloc(1, sizeof(cmn_computed_t), (void **)&comp);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  comp->base.type = NODE_TYPE_COMPUTED;
  comp->compute_fn = compute_fn;
  comp->userdata = userdata;

  rc = signals_lock();
  rc = signals_accum_err(rc, register_node((cmn_node_t *)comp));
  if (rc != CMN_SUCCESS) {
    cmn_error_e free_rc = cmn_free((void **)&comp);
    rc = signals_accum_err(rc, signals_unlock());
    return (free_rc != CMN_SUCCESS) ? free_rc : rc;
  }

  prev_tracker = g_current_tracker;
  g_current_tracker = (cmn_node_t *)comp;
  rc = signals_accum_err(rc, signals_unlock());
  {
    cmn_error_e eval_rc = comp->compute_fn(comp->userdata, &comp->cached_value);
    if (eval_rc != CMN_SUCCESS) {
      cmn_error_e unreg_rc;
      cmn_error_e free_rc;
      rc = signals_accum_err(rc, signals_lock());
      g_current_tracker = prev_tracker;
      unreg_rc = unregister_node((cmn_node_t *)comp);
      rc = signals_accum_err(rc, signals_unlock());
      free_rc = cmn_free((void **)&comp);
      if (unreg_rc != CMN_SUCCESS) {
        return unreg_rc;
      }
      return (free_rc != CMN_SUCCESS) ? free_rc : eval_rc;
    }
  }
  rc = signals_accum_err(rc, signals_lock());
  g_current_tracker = prev_tracker;

  rc = signals_accum_err(rc, signals_unlock());

  *out_comp = comp;
  return rc;
}

/**
 * @brief Gets the current value of a computed integer signal.
 * @param comp The computed signal to query.
 * @param out_val Pointer to store the current value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_computed_int_get(cmn_computed_t *comp, int *out_val) {
  cmn_error_e rc;
  if (comp == NULL || out_val == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = signals_lock();

  if (g_current_tracker != NULL) {
    cmn_error_e add_rc = add_dependent((cmn_node_t *)comp, g_current_tracker);
    if (add_rc != CMN_SUCCESS) {
      return signals_accum_err(add_rc, signals_unlock());
    }
  }

  *out_val = comp->cached_value;
  return signals_accum_err(rc, signals_unlock());
}

/**
 * @brief Destroys a computed signal and frees its memory.
 * @param comp Pointer to the computed signal pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_computed_destroy(cmn_computed_t **comp) {
  cmn_error_e rc;
  if (comp == NULL || *comp == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = signals_lock();
  rc = signals_accum_err(rc, unregister_node((cmn_node_t *)*comp));
  if (rc != CMN_SUCCESS) {
    return signals_accum_err(rc, signals_unlock());
  }
  rc = signals_unlock_after_unregister();

  return signals_accum_err(rc, cmn_free((void **)comp));
}

/* --- Effect (Side-effects) --- */

/**
 * @brief Creates a new reactive effect that runs when its dependencies change.
 * @param effect_fn The function to execute.
 * @param userdata User data passed to the effect function.
 * @param out_effect Pointer to store the created effect.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_effect_create(cmn_effect_fn effect_fn, void *userdata,
                              cmn_effect_t **out_effect) {
  cmn_error_e rc;
  cmn_effect_t *effect = NULL;
  cmn_node_t *prev_tracker;

  if (effect_fn == NULL || out_effect == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_calloc(1, sizeof(cmn_effect_t), (void **)&effect);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  effect->base.type = NODE_TYPE_EFFECT;
  effect->effect_fn = effect_fn;
  effect->userdata = userdata;

  rc = signals_lock();
  rc = signals_accum_err(rc, register_node((cmn_node_t *)effect));
  if (rc != CMN_SUCCESS) {
    cmn_error_e free_rc = cmn_free((void **)&effect);
    rc = signals_accum_err(rc, signals_unlock());
    return (free_rc != CMN_SUCCESS) ? free_rc : rc;
  }

  prev_tracker = g_current_tracker;
  g_current_tracker = (cmn_node_t *)effect;
  rc = signals_accum_err(rc, signals_unlock());
  {
    cmn_error_e init_rc = effect->effect_fn(effect->userdata);
    if (init_rc != CMN_SUCCESS) {
      cmn_error_e unreg_rc;
      cmn_error_e free_rc;
      rc = signals_accum_err(rc, signals_lock());
      g_current_tracker = prev_tracker;
      unreg_rc = unregister_node((cmn_node_t *)effect);
      rc = signals_accum_err(rc, signals_unlock());
      free_rc = cmn_free((void **)&effect);
      if (unreg_rc != CMN_SUCCESS) {
        return unreg_rc;
      }
      return (free_rc != CMN_SUCCESS) ? free_rc : init_rc;
    }
  }
  rc = signals_accum_err(rc, signals_lock());
  g_current_tracker = prev_tracker;

  rc = signals_accum_err(rc, signals_unlock());

  *out_effect = effect;
  return rc;
}

/**
 * @brief Destroys an effect and frees its memory.
 * @param effect Pointer to the effect pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_effect_destroy(cmn_effect_t **effect) {
  cmn_error_e rc;
  if (effect == NULL || *effect == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = ensure_signals_initialized();
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = signals_lock();
  rc = signals_accum_err(rc, unregister_node((cmn_node_t *)*effect));
  if (rc != CMN_SUCCESS) {
    return signals_accum_err(rc, signals_unlock());
  }
  rc = signals_unlock_after_unregister();

  return signals_accum_err(rc, cmn_free((void **)effect));
}
