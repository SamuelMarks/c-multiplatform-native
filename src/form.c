/* clang-format off */
#include "cmn/form.h"
#include "cmn/memory.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file form.c
 * @brief Implementation of form management, validation, debounce, and wizards.
 */

/**
 * @def MAX_FORM_INPUTS
 * @brief Maximum number of inputs allowed in a single form.
 */
#define MAX_FORM_INPUTS 32

/**
 * @brief Internal record representing a single input field in a form.
 */
typedef struct form_input_record_s {
  cmn_textbox_t *textbox;     /**< The text box widget. */
  cmn_validator_fn validator; /**< The validation function. */
  void *userdata;             /**< User data passed to the validator. */
  int debounce_delay_ms;      /**< Debounce delay in milliseconds. */
  int is_valid;               /**< Current validity state. */
  const char *last_err_msg;   /**< Last validation error message (or NULL). */
} form_input_record_t;

/**
 * @brief Internal representation of a form.
 */
struct cmn_form_s {
  form_input_record_t inputs[MAX_FORM_INPUTS]; /**< Array of form inputs. */
  size_t input_count;                          /**< Number of inputs in form. */
};

/**
 * @brief Creates a new form instance.
 * @param out_form Pointer to store the created form.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_form_create(cmn_form_t **out_form) {
  if (out_form == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_calloc(1, sizeof(cmn_form_t), (void **)out_form);
}

/**
 * @brief Adds a text box input and its validator to a form.
 * @param form The form to modify.
 * @param textbox The text box widget to add.
 * @param validator The validation callback (can be NULL).
 * @param userdata User data passed to the validator.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_form_add_input(cmn_form_t *form, cmn_textbox_t *textbox,
                               cmn_validator_fn validator, void *userdata) {
  size_t i;
  if (form == NULL || textbox == NULL) {
    return CMN_ERR_INVAL;
  }

  for (i = 0; i < form->input_count; ++i) {
    if (form->inputs[i].textbox == textbox) {
      form->inputs[i].validator = validator;
      form->inputs[i].userdata = userdata;
      form->inputs[i].is_valid = 1;
      form->inputs[i].last_err_msg = NULL;
      return CMN_SUCCESS;
    }
  }

  if (form->input_count >= MAX_FORM_INPUTS) {
    return CMN_ERR_NOMEM;
  }

  form->inputs[form->input_count].textbox = textbox;
  form->inputs[form->input_count].validator = validator;
  form->inputs[form->input_count].userdata = userdata;
  form->inputs[form->input_count].debounce_delay_ms = 0;
  form->inputs[form->input_count].is_valid = 1;
  form->inputs[form->input_count].last_err_msg = NULL;
  form->input_count++;

  return CMN_SUCCESS;
}

/**
 * @brief Sets the debounce delay for validating a specific input in a form.
 * @param form The form containing the input.
 * @param textbox The text box input to configure.
 * @param delay_ms The debounce delay in milliseconds.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_form_set_debounce(cmn_form_t *form, cmn_textbox_t *textbox,
                                  int delay_ms) {
  size_t i;
  if (form == NULL || textbox == NULL || delay_ms < 0) {
    return CMN_ERR_INVAL;
  }

  for (i = 0; i < form->input_count; ++i) {
    if (form->inputs[i].textbox == textbox) {
      form->inputs[i].debounce_delay_ms = delay_ms;
      return CMN_SUCCESS;
    }
  }
  return CMN_ERR_NOT_FOUND;
}

/**
 * @brief Gets the configured debounce delay for a specific input in a form.
 * @param form The form containing the input.
 * @param textbox The text box input to query.
 * @param out_delay_ms Pointer to store the debounce delay in milliseconds.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if absent, error code
 * otherwise.
 */
cmn_error_e cmn_form_get_input_debounce(cmn_form_t *form,
                                        cmn_textbox_t *textbox,
                                        int *out_delay_ms) {
  size_t i;
  if (form == NULL || textbox == NULL || out_delay_ms == NULL) {
    return CMN_ERR_INVAL;
  }

  for (i = 0; i < form->input_count; ++i) {
    if (form->inputs[i].textbox == textbox) {
      *out_delay_ms = form->inputs[i].debounce_delay_ms;
      return CMN_SUCCESS;
    }
  }
  return CMN_ERR_NOT_FOUND;
}

/**
 * @brief Gets the last validation error message for a specific input in a form.
 * @param form The form containing the input.
 * @param textbox The text box input to query.
 * @param out_err_msg Pointer to store the last validation error message (NULL
 * if valid).
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if absent, error code
 * otherwise.
 */
cmn_error_e cmn_form_get_input_error(cmn_form_t *form, cmn_textbox_t *textbox,
                                     const char **out_err_msg) {
  size_t i;
  if (form == NULL || textbox == NULL || out_err_msg == NULL) {
    return CMN_ERR_INVAL;
  }

  for (i = 0; i < form->input_count; ++i) {
    if (form->inputs[i].textbox == textbox) {
      *out_err_msg = form->inputs[i].last_err_msg;
      return CMN_SUCCESS;
    }
  }
  return CMN_ERR_NOT_FOUND;
}

/**
 * @brief Validates all inputs in the form.
 * @param form The form to validate.
 * @param out_is_valid Pointer to store the overall validity (1 if all valid, 0
 * otherwise).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_form_validate(cmn_form_t *form, int *out_is_valid) {
  size_t i;
  int all_valid = 1;
  cmn_error_e rc;

  if (form == NULL || out_is_valid == NULL) {
    return CMN_ERR_INVAL;
  }

  for (i = 0; i < form->input_count; ++i) {
    if (form->inputs[i].validator != NULL) {
      const char *text = NULL;
      int is_valid = 0;
      char *err_msg = NULL;

      rc = cmn_textbox_get_text(form->inputs[i].textbox, &text);
      if (rc != CMN_SUCCESS) {
        return rc;
      }

      rc = form->inputs[i].validator(text, form->inputs[i].userdata, &is_valid,
                                     &err_msg);
      if (rc != CMN_SUCCESS) {
        return rc;
      }

      form->inputs[i].is_valid = is_valid;
      form->inputs[i].last_err_msg = is_valid ? NULL : err_msg;
      if (!is_valid) {
        all_valid = 0;
      }
    }
  }

  *out_is_valid = all_valid;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a form and frees its memory.
 * @param form Pointer to the form pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_form_destroy(cmn_form_t **form) {
  if (form == NULL || *form == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)form);
}

/* --- Multi-step Forms (Wizards) --- */

/**
 * @def MAX_WIZARD_PAGES
 * @brief Maximum number of pages allowed in a wizard.
 */
#define MAX_WIZARD_PAGES 16

/**
 * @brief Internal representation of a wizard.
 */
struct cmn_wizard_s {
  cmn_form_t *pages[MAX_WIZARD_PAGES]; /**< Array of form pages. */
  size_t page_count;                   /**< Total number of pages. */
  size_t current_index;                /**< Current active page index. */
};

/**
 * @brief Creates a new multi-step form wizard.
 * @param out_wizard Pointer to store the created wizard.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_wizard_create(cmn_wizard_t **out_wizard) {
  if (out_wizard == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_calloc(1, sizeof(cmn_wizard_t), (void **)out_wizard);
}

/**
 * @brief Adds a form as a page in the wizard.
 * @param wizard The wizard to modify.
 * @param page_form The form representing the page.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_wizard_add_page(cmn_wizard_t *wizard, cmn_form_t *page_form) {
  if (wizard == NULL || page_form == NULL) {
    return CMN_ERR_INVAL;
  }

  if (wizard->page_count >= MAX_WIZARD_PAGES) {
    return CMN_ERR_NOMEM;
  }

  wizard->pages[wizard->page_count++] = page_form;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the active form page of the wizard.
 * @param wizard The wizard to query.
 * @param out_form Pointer to store the current page form.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if wizard has no pages,
 * error code otherwise.
 */
cmn_error_e cmn_wizard_get_current_page(cmn_wizard_t *wizard,
                                        cmn_form_t **out_form) {
  if (wizard == NULL || out_form == NULL) {
    return CMN_ERR_INVAL;
  }
  if (wizard->page_count == 0) {
    return CMN_ERR_NOT_FOUND;
  }
  *out_form = wizard->pages[wizard->current_index];
  return CMN_SUCCESS;
}

/**
 * @brief Advances the wizard to the next page if the current page is valid.
 * @param wizard The wizard to advance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_wizard_next(cmn_wizard_t *wizard) {
  int is_valid = 0;
  cmn_error_e rc;

  if (wizard == NULL) {
    return CMN_ERR_INVAL;
  }

  if (wizard->page_count == 0 ||
      wizard->current_index + 1 >= wizard->page_count) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_form_validate(wizard->pages[wizard->current_index], &is_valid);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  if (!is_valid) {
    return CMN_ERR_INVAL;
  }

  wizard->current_index++;
  return CMN_SUCCESS;
}

/**
 * @brief Moves the wizard back to the previous page.
 * @param wizard The wizard to move back.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_wizard_prev(cmn_wizard_t *wizard) {
  if (wizard == NULL) {
    return CMN_ERR_INVAL;
  }

  if (wizard->current_index == 0) {
    return CMN_ERR_INVAL;
  }

  wizard->current_index--;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a wizard and frees its memory.
 * @param wizard Pointer to the wizard pointer to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_wizard_destroy(cmn_wizard_t **wizard) {
  if (wizard == NULL || *wizard == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)wizard);
}
