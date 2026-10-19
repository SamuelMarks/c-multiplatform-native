/* clang-format off */
#include "greatest.h"
#include "cmn/form.h"
#include "cmn/widget.h"
#include "cmn/memory.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file test_form.c
 * @brief Behavioral unit tests for form validation, debounce, error inspection,
 * and multi-step wizards.
 */

/**
 * @brief Validator requiring non-empty text and setting an error message on
 * failure.
 * @param val Input text string.
 * @param userdata Unused user data.
 * @param out_is_valid Output validity flag.
 * @param out_err_msg Output error message pointer.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e validator_not_empty(const char *val, void *userdata,
                                       int *out_is_valid, char **out_err_msg) {
  if (val != NULL && strlen(val) > 0) {
    *out_is_valid = 1;
    if (out_err_msg != NULL) {
      *out_err_msg = (char *)userdata;
    }
  } else {
    *out_is_valid = 0;
    if (out_err_msg != NULL) {
      *out_err_msg =
          (userdata != NULL) ? (char *)userdata : "Field cannot be empty";
    }
  }
  return CMN_SUCCESS;
}

/** @brief External test helper `cmn_mock_alloc_set`. */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);
/** @brief External test helper `cmn_mock_textbox_get_text_set`. */
extern cmn_error_e cmn_mock_textbox_get_text_set(int fail);

/**
 * @brief Validator that simulates an internal I/O failure.
 * @param val Unused input text.
 * @param userdata Unused user data.
 * @param out_is_valid Unused output flag.
 * @param out_err_msg Unused error message.
 * @return CMN_ERR_IO.
 */
static cmn_error_e validator_error(const char *val, void *userdata,
                                   int *out_is_valid, char **out_err_msg) {
  if (out_is_valid != NULL) {
    *out_is_valid = (val == NULL && userdata != NULL) ? -1 : 0;
  }
  if (out_err_msg != NULL) {
    *out_err_msg = NULL;
  }
  return CMN_ERR_IO;
}

/**
 * @brief Tests `cmn_form_set_debounce` and `cmn_form_get_input_debounce`.
 * @return Greatest test result code.
 */
TEST test_form_debounce(void) {
  cmn_form_t *form1 = NULL;
  cmn_textbox_t *tb1 = NULL;
  cmn_textbox_t *tb2 = NULL;
  int delay_ms = -1;

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb2));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form1));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_form_add_input(form1, tb1, validator_not_empty, NULL));
  /* Duplicate registration updates in-place */
  ASSERT_EQ(CMN_SUCCESS,
            cmn_form_add_input(form1, tb1, validator_not_empty, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_form_set_debounce(form1, tb1, 500));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_get_input_debounce(form1, tb1, &delay_ms));
  ASSERT_EQ(500, delay_ms);

  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_form_set_debounce(form1, tb2, 500));
  ASSERT_EQ(CMN_ERR_NOT_FOUND,
            cmn_form_get_input_debounce(form1, tb2, &delay_ms));

  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb2));
  PASS();
}

/**
 * @brief Tests form and wizard OOM and capacity limits (`MAX_FORM_INPUTS = 32`,
 * `MAX_WIZARD_PAGES = 16`).
 * @return Greatest test result code.
 */
TEST test_form_oom_and_limits(void) {
  cmn_form_t *form1 = NULL;
  cmn_wizard_t *wizard = NULL;
  cmn_textbox_t *tbs[33];
  int i;

  for (i = 0; i < 33; i++) {
    ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tbs[i]));
  }

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_form_create(&form1));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_wizard_create(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));

  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_create(&wizard));

  for (i = 0; i < 32; i++) {
    ASSERT_EQ(CMN_SUCCESS,
              cmn_form_add_input(form1, tbs[i], validator_not_empty, NULL));
  }
  ASSERT_EQ(CMN_ERR_NOMEM,
            cmn_form_add_input(form1, tbs[32], validator_not_empty, NULL));

  for (i = 0; i < 16; i++) {
    ASSERT_EQ(CMN_SUCCESS, cmn_wizard_add_page(wizard, form1));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_wizard_add_page(wizard, form1));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_destroy(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form1));
  for (i = 0; i < 33; i++) {
    ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tbs[i]));
  }
  PASS();
}

/**
 * @brief Tests validator error percolation in `cmn_form_validate` and
 * `cmn_wizard_next`.
 * @return Greatest test result code.
 */
TEST test_form_validate_errors(void) {
  cmn_form_t *form1 = NULL;
  cmn_wizard_t *wizard = NULL;
  cmn_textbox_t *tb1 = NULL;
  int is_valid = 0;

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb1));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_add_input(form1, tb1, validator_error, NULL));

  ASSERT_EQ(CMN_ERR_IO, cmn_form_validate(form1, &is_valid));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_create(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_add_page(wizard, form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_add_page(wizard, form1));
  ASSERT_EQ(CMN_ERR_IO, cmn_wizard_next(wizard));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_destroy(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb1));
  PASS();
}

/**
 * @brief Tests form validation, error message storage
 * (`cmn_form_get_input_error`), and wizard page validation gating.
 * @return Greatest test result code.
 */
TEST test_form_validation(void) {
  cmn_form_t *form1 = NULL;
  cmn_form_t *form2 = NULL;
  cmn_form_t *cur_page = NULL;
  cmn_wizard_t *wizard = NULL;
  cmn_textbox_t *tb1 = NULL;
  cmn_textbox_t *tb2 = NULL;
  cmn_textbox_t *tb_unregistered = NULL;
  const char *err_msg = NULL;
  int is_valid = 0;

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb2));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb_unregistered));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form2));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_form_add_input(form1, tb1, validator_not_empty, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_add_input(form1, tb2, NULL, NULL));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_form_add_input(form2, tb2, validator_not_empty, NULL));

  /* form1 tb1 is empty, should be invalid and populate error message */
  ASSERT_EQ(CMN_SUCCESS, cmn_form_validate(form1, &is_valid));
  ASSERT_EQ(0, is_valid);
  ASSERT_EQ(CMN_SUCCESS, cmn_form_get_input_error(form1, tb1, &err_msg));
  ASSERT_STR_EQ("Field cannot be empty", err_msg);
  ASSERT_EQ(CMN_ERR_NOT_FOUND,
            cmn_form_get_input_error(form1, tb_unregistered, &err_msg));

  /* Populate tb1 and re-validate */
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_set_text(tb1, "valid_user"));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_validate(form1, &is_valid));
  ASSERT_EQ(1, is_valid);
  ASSERT_EQ(CMN_SUCCESS, cmn_form_get_input_error(form1, tb1, &err_msg));
  ASSERT_EQ((const char *)NULL, err_msg);

  /* Reset tb1 to empty to test wizard gating */
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_set_text(tb1, ""));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_create(&wizard));
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_wizard_get_current_page(wizard, &cur_page));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_next(wizard));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_add_page(wizard, form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_add_page(wizard, form2));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_get_current_page(wizard, &cur_page));
  ASSERT_EQ(form1, cur_page);

  /* Without text, form1 is invalid, so next fails */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_next(wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_get_current_page(wizard, &cur_page));
  ASSERT_EQ(form1, cur_page);

  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_prev(wizard));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_destroy(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form2));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb_unregistered));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb2));

  PASS();
}

/**
 * @brief Tests `cmn_form_validate` when `cmn_textbox_get_text` fails.
 * @return Greatest test result code.
 */
TEST test_form_validate_get_text_error(void) {
  cmn_form_t *form1 = NULL;
  cmn_textbox_t *tb1 = NULL;
  int is_valid = 0;

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb1));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form1));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_form_add_input(form1, tb1, validator_not_empty, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_textbox_get_text_set(1));
  ASSERT_EQ(CMN_ERR_IO, cmn_form_validate(form1, &is_valid));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_textbox_get_text_set(0));

  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb1));
  PASS();
}

/**
 * @brief Validator that always succeeds.
 * @param val Unused input text.
 * @param userdata Unused user data.
 * @param out_is_valid Output validity flag set to 1.
 * @param out_err_msg Unused error message.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e validator_always_valid(const char *val, void *userdata,
                                          int *out_is_valid,
                                          char **out_err_msg) {
  if (out_err_msg != NULL) {
    *out_err_msg = (char *)userdata;
  }
  *out_is_valid = (val != NULL) ? 1 : 2;
  return CMN_SUCCESS;
}

/**
 * @brief Tests `cmn_wizard_next`, `cmn_wizard_prev`, and
 * `cmn_wizard_get_current_page`.
 * @return Greatest test result code.
 */
TEST test_form_wizard_navigation(void) {
  cmn_form_t *form1 = NULL;
  cmn_form_t *form2 = NULL;
  cmn_form_t *cur_page = NULL;
  cmn_wizard_t *wizard = NULL;
  cmn_textbox_t *tb1 = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb1));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form2));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_form_add_input(form1, tb1, validator_always_valid, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_create(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_add_page(wizard, form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_add_page(wizard, form2));

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_next(wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_get_current_page(wizard, &cur_page));
  ASSERT_EQ(form2, cur_page);

  /* Already on last page */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_next(wizard));

  /* Prev returns to form1 */
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_prev(wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_get_current_page(wizard, &cur_page));
  ASSERT_EQ(form1, cur_page);

  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_destroy(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form1));
  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form2));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb1));
  PASS();
}

/**
 * @brief Tests NULL argument checks across all form and wizard APIs.
 * @return Greatest test result code.
 */
TEST test_form_nulls(void) {
  cmn_form_t *form = NULL;
  cmn_form_t *cur_page = NULL;
  cmn_wizard_t *wizard = NULL;
  cmn_textbox_t *tb = NULL;
  const char *err_msg = NULL;
  int is_valid = 0;
  int delay_ms = 0;

  ASSERT_EQ(CMN_SUCCESS, cmn_form_create(&form));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_create(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_create(&tb));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_create(NULL));

  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_form_add_input(NULL, tb, validator_not_empty, NULL));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_form_add_input(form, NULL, validator_not_empty, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_set_debounce(NULL, tb, 500));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_set_debounce(form, NULL, 500));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_set_debounce(form, tb, -1));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_get_input_debounce(NULL, tb, &delay_ms));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_get_input_debounce(form, NULL, &delay_ms));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_get_input_debounce(form, tb, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_get_input_error(NULL, tb, &err_msg));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_get_input_error(form, NULL, &err_msg));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_get_input_error(form, tb, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_validate(NULL, &is_valid));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_validate(form, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_form_destroy(NULL));
  {
    cmn_form_t *null_form = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_form_destroy(&null_form));
  }

  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_create(NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_add_page(NULL, form));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_add_page(wizard, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_get_current_page(NULL, &cur_page));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_get_current_page(wizard, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_next(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_prev(NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_destroy(NULL));
  {
    cmn_wizard_t *null_wiz = NULL;
    ASSERT_EQ(CMN_ERR_INVAL, cmn_wizard_destroy(&null_wiz));
  }

  ASSERT_EQ(CMN_SUCCESS, cmn_form_destroy(&form));
  ASSERT_EQ(CMN_SUCCESS, cmn_wizard_destroy(&wizard));
  ASSERT_EQ(CMN_SUCCESS, cmn_textbox_destroy(&tb));
  PASS();
}

/**
 * @brief Test suite for form and wizard validation.
 */
SUITE(form_suite) {
  RUN_TEST(test_form_nulls);
  RUN_TEST(test_form_validation);
  RUN_TEST(test_form_debounce);
  RUN_TEST(test_form_oom_and_limits);
  RUN_TEST(test_form_validate_errors);
  RUN_TEST(test_form_validate_get_text_error);
  RUN_TEST(test_form_wizard_navigation);
}
