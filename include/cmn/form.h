#ifndef CMN_FORM_H
#define CMN_FORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
#include "cmn/widget.h"
/* clang-format on */

/**
 * @file form.h
 * @brief Form management, validation, debounce, and multi-step wizard
 * workflows.
 */

/**
 * @defgroup cmn_form Forms and Validation
 * @brief Forms and Validation module.
 * @{
 */

/**
 * @brief Callback function type for validating a form input's text.
 * @param text The current text value of the input.
 * @param userdata User data passed during validator registration.
 * @param out_is_valid Pointer to store whether the input is valid (1) or not
 * (0).
 * @param out_err_msg Pointer to store an error message if invalid (can be
 * NULL).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_validator_fn)(const char *text, void *userdata,
                                        int *out_is_valid, char **out_err_msg);

/**
 * @brief Creates a new form instance.
 * @param out_form Pointer to store the created form.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_form_create(cmn_form_t **out_form);

/**
 * @brief Adds a text box input and its validator to a form.
 * @param form The form to modify.
 * @param textbox The text box widget to add.
 * @param validator The validation callback (can be NULL).
 * @param userdata User data passed to the validator.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_form_add_input(cmn_form_t *form,
                                              cmn_textbox_t *textbox,
                                              cmn_validator_fn validator,
                                              void *userdata);

/**
 * @brief Sets the debounce delay for validating a specific input in a form.
 * @param form The form containing the input.
 * @param textbox The text box input to configure.
 * @param delay_ms The debounce delay in milliseconds.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_form_set_debounce(cmn_form_t *form,
                                                 cmn_textbox_t *textbox,
                                                 int delay_ms);

/**
 * @brief Gets the configured debounce delay for a specific input in a form.
 * @param form The form containing the input.
 * @param textbox The text box input to query.
 * @param out_delay_ms Pointer to store the debounce delay in milliseconds.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if absent, error code
 * otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_form_get_input_debounce(cmn_form_t *form,
                                                       cmn_textbox_t *textbox,
                                                       int *out_delay_ms);

/**
 * @brief Gets the last validation error message for a specific input in a form.
 * @param form The form containing the input.
 * @param textbox The text box input to query.
 * @param out_err_msg Pointer to store the last validation error message (NULL
 * if valid).
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if absent, error code
 * otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_form_get_input_error(cmn_form_t *form,
                                                    cmn_textbox_t *textbox,
                                                    const char **out_err_msg);

/**
 * @brief Validates all inputs in the form.
 * @param form The form to validate.
 * @param out_is_valid Pointer to store the overall validity (1 if all valid, 0
 * otherwise).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_form_validate(cmn_form_t *form,
                                             int *out_is_valid);

/**
 * @brief Destroys a form and frees its memory.
 * @param form Pointer to the form pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_form_destroy(cmn_form_t **form);

/* --- Multi-step Forms (Wizards) --- */

/**
 * @brief Creates a new multi-step form wizard.
 * @param out_wizard Pointer to store the created wizard.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_wizard_create(cmn_wizard_t **out_wizard);

/**
 * @brief Adds a form as a page in the wizard.
 * @param wizard The wizard to modify.
 * @param page_form The form representing the page.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_wizard_add_page(cmn_wizard_t *wizard,
                                               cmn_form_t *page_form);

/**
 * @brief Gets the active form page of the wizard.
 * @param wizard The wizard to query.
 * @param out_form Pointer to store the current page form.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if wizard has no pages,
 * error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_wizard_get_current_page(cmn_wizard_t *wizard,
                                                       cmn_form_t **out_form);

/**
 * @brief Advances the wizard to the next page if the current page is valid.
 * @param wizard The wizard to advance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_wizard_next(cmn_wizard_t *wizard);

/**
 * @brief Moves the wizard back to the previous page.
 * @param wizard The wizard to move back.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_wizard_prev(cmn_wizard_t *wizard);

/**
 * @brief Destroys a wizard and frees its memory.
 * @param wizard Pointer to the wizard pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_wizard_destroy(cmn_wizard_t **wizard);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_FORM_H */
