#ifndef CMN_SIGNALS_H
#define CMN_SIGNALS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
/* clang-format on */

/**
 * @file signals.h
 * @brief Reactive Signals and State Management (Angular-style).
 */

/**
 * @defgroup cmn_signals Reactive Signals
 * @brief Reactive Signals module.
 * @{
 */

/**
 * @brief Creates a new integer signal.
 * @param initial_val The initial value of the signal.
 * @param out_sig Pointer to store the created signal.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_signal_int_create(int initial_val,
                                                 cmn_signal_t **out_sig);

/**
 * @brief Gets the current value of an integer signal.
 * @param sig The signal to query.
 * @param out_val Pointer to store the current value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_signal_int_get(cmn_signal_t *sig, int *out_val);

/**
 * @brief Sets a new value for an integer signal, triggering dependents.
 * @param sig The signal to modify.
 * @param new_val The new value to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_signal_int_set(cmn_signal_t *sig, int new_val);

/**
 * @brief Destroys a signal and frees its memory.
 * @param sig Pointer to the signal pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_signal_destroy(cmn_signal_t **sig);

/* --- Computed Signals --- */

/**
 * @brief Callback function type for computing a derived integer signal value.
 * @param userdata User data passed during creation.
 * @param out_val Pointer to store the computed value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_compute_int_fn)(void *userdata, int *out_val);

/**
 * @brief Creates a new computed integer signal.
 * @param compute_fn The function to compute the value.
 * @param userdata User data passed to the compute function.
 * @param out_comp Pointer to store the created computed signal.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_computed_int_create(
    cmn_compute_int_fn compute_fn, void *userdata, cmn_computed_t **out_comp);

/**
 * @brief Gets the current value of a computed integer signal.
 * @param comp The computed signal to query.
 * @param out_val Pointer to store the current value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_computed_int_get(cmn_computed_t *comp,
                                                int *out_val);

/**
 * @brief Destroys a computed signal and frees its memory.
 * @param comp Pointer to the computed signal pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_computed_destroy(cmn_computed_t **comp);

/* --- Effects --- */

/**
 * @brief Callback function type for a reactive effect.
 * @param userdata User data passed during creation.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_effect_fn)(void *userdata);

/**
 * @brief Creates a new reactive effect that runs when its dependencies change.
 * @param effect_fn The function to execute.
 * @param userdata User data passed to the effect function.
 * @param out_effect Pointer to store the created effect.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_effect_create(cmn_effect_fn effect_fn,
                                             void *userdata,
                                             cmn_effect_t **out_effect);

/**
 * @brief Destroys an effect and frees its memory.
 * @param effect Pointer to the effect pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_effect_destroy(cmn_effect_t **effect);

#ifdef CMN_TESTING
/**
 * @brief Sets mock failure state for signal subsystem initialization.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_mock_signals_init_fail_set(int fail);

/**
 * @brief Sets mock failure state for unregistering a node in signals.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_mock_signals_unregister_fail_set(int fail);
#endif

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_SIGNALS_H */
