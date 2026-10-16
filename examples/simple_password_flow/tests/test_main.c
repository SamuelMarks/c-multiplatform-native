/* clang-format off */
#include "greatest.h"
#include "app_state.h"
#include "login_view.h"
#include "welcome_view.h"
#include <cmn/app.h>
#include <cmn/automation.h>
#include <cmn/error.h>
#include <cmn/router.h>
#include <cmn/widget.h>
#include <cmn/memory.h>
#include <cmn/form.h>
#include <cmn/string_safe.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file test_main.c
 * @brief Behavioral unit tests for the Simple Password Flow example.
 */

/** @brief Greatest test framework main definitions. */
GREATEST_MAIN_DEFS();

/**
 * @brief External test helper `cmn_mock_alloc_set`.
 * @param fail Whether to fail (1) or not (0).
 * @param count Number of allocations before failing.
 * @return CMN_SUCCESS on success.
 */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);
/**
 * @brief External test helper `app_main`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
extern cmn_error_e app_main(void);

/**
 * @brief Tests `app_state_init` with valid and NULL arguments.
 * @return Greatest test result code.
 */
TEST test_app_state_init(void) {
  app_state_t state;
  cmn_router_t *router = (cmn_router_t *)0x1;

  ASSERT_EQ(CMN_ERR_INVAL, app_state_init(NULL, router));
  ASSERT_EQ(CMN_ERR_INVAL, app_state_init(&state, NULL));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));
  ASSERT_STR_EQ("", state.username);

  PASS();
}

/**
 * @brief Tests `validate_username` and `validate_password` rules.
 * @return Greatest test result code.
 */
TEST test_login_validations(void) {
  int is_valid = 0;
  char *err_msg = NULL;

  ASSERT_EQ(CMN_SUCCESS, validate_username("test", NULL, &is_valid, &err_msg));
  ASSERT_EQ(1, is_valid);
  ASSERT_EQ(CMN_SUCCESS, validate_username("", NULL, &is_valid, &err_msg));
  ASSERT_EQ(0, is_valid);
  ASSERT_EQ(CMN_SUCCESS, validate_username(NULL, NULL, &is_valid, &err_msg));
  ASSERT_EQ(0, is_valid);

  ASSERT_EQ(CMN_SUCCESS, validate_password("pass", NULL, &is_valid, &err_msg));
  ASSERT_EQ(1, is_valid);
  ASSERT_EQ(CMN_SUCCESS, validate_password("pwd", NULL, &is_valid, &err_msg));
  ASSERT_EQ(0, is_valid);
  ASSERT_EQ(CMN_SUCCESS, validate_password(NULL, NULL, &is_valid, &err_msg));
  ASSERT_EQ(0, is_valid);

  PASS();
}

/**
 * @brief Tests `on_login_submit` when form inputs are empty/invalid.
 * @return Greatest test result code.
 */
TEST test_login_submit_invalid(void) {
  app_state_t state;
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  void *widget = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));

  ASSERT_EQ(CMN_SUCCESS, login_view_factory(&state, NULL, 0, &widget));
  ASSERT_EQ(CMN_SUCCESS, on_login_submit(g_login_data));
  ASSERT_STR_EQ("", state.username);

  ASSERT_EQ(CMN_SUCCESS, login_view_cleanup(1));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests `on_login_submit` when form inputs are valid and verifies
 * navigation to `/welcome`.
 * @return Greatest test result code.
 */
TEST test_login_submit_valid(void) {
  app_state_t state;
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  const char *cur_path = NULL;
  void *widget = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));

  ASSERT_EQ(CMN_SUCCESS, cmn_router_register(router, "/welcome",
                                             welcome_view_factory, &state));
  ASSERT_EQ(CMN_SUCCESS, login_view_factory(&state, NULL, 0, &widget));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_textbox_set_text(g_login_data->tb_username, "samuel"));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_textbox_set_text(g_login_data->tb_password, "password123"));

  ASSERT_EQ(CMN_SUCCESS, on_login_submit(g_login_data));
  ASSERT_STR_EQ("samuel", state.username);
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &cur_path));
  ASSERT_STR_EQ("/welcome", cur_path);

  ASSERT_EQ(CMN_SUCCESS, login_view_cleanup(1));
  ASSERT_EQ(CMN_SUCCESS, welcome_view_cleanup(0));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests `on_login_submit` when navigation to `/welcome` fails due to
 * OOM.
 * @return Greatest test result code.
 */
TEST test_login_submit_oom(void) {
  app_state_t state;
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  void *widget = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));

  ASSERT_EQ(CMN_SUCCESS, cmn_router_register(router, "/welcome",
                                             welcome_view_factory, &state));
  ASSERT_EQ(CMN_SUCCESS, login_view_factory(&state, NULL, 0, &widget));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_textbox_set_text(g_login_data->tb_username, "samuel"));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_textbox_set_text(g_login_data->tb_password, "password123"));

  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, on_login_submit(g_login_data));
  (void)cmn_mock_alloc_set(0, 0);

  ASSERT_EQ(CMN_SUCCESS, login_view_cleanup(1));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests `on_login_submit` when username exceeds `state.username` buffer
 * capacity.
 * @return Greatest test result code.
 */
TEST test_login_submit_oom2(void) {
  app_state_t state;
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  void *widget = NULL;
  char long_username[300];

  (void)cmn_mock_alloc_set(0, 0);

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));

  ASSERT_EQ(CMN_SUCCESS, login_view_factory(&state, NULL, 0, &widget));

  memset(long_username, 'a', sizeof(long_username) - 1);
  long_username[sizeof(long_username) - 1] = '\0';

  ASSERT_EQ(CMN_SUCCESS,
            cmn_textbox_set_text(g_login_data->tb_username, long_username));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_textbox_set_text(g_login_data->tb_password, "password123"));

#if !defined(_MSC_VER)
  ASSERT_EQ(CMN_ERR_IO, on_login_submit(g_login_data));
#endif

  ASSERT_EQ(CMN_SUCCESS, login_view_cleanup(1));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests `on_login_submit` when `form` is NULL.
 * @return Greatest test result code.
 */
TEST test_login_submit_null_form(void) {
  app_state_t state;
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  void *widget = NULL;
  cmn_form_t *temp_form = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));

  ASSERT_EQ(CMN_SUCCESS, login_view_factory(&state, NULL, 0, &widget));

  temp_form = g_login_data->form;
  g_login_data->form = NULL;
  ASSERT_EQ(CMN_ERR_INVAL, on_login_submit(g_login_data));
  g_login_data->form = temp_form;

  ASSERT_EQ(CMN_SUCCESS, login_view_cleanup(1));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests `on_login_submit` when `tb_username` is NULL after validation.
 * @return Greatest test result code.
 */
TEST test_login_submit_null_tb(void) {
  app_state_t state;
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  void *widget = NULL;
  cmn_textbox_t *temp_tb = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));

  ASSERT_EQ(CMN_SUCCESS, login_view_factory(&state, NULL, 0, &widget));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_textbox_set_text(g_login_data->tb_username, "samuel"));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_textbox_set_text(g_login_data->tb_password, "password123"));

  temp_tb = g_login_data->tb_username;
  g_login_data->tb_username = NULL;
  ASSERT_EQ(CMN_ERR_INVAL, on_login_submit(g_login_data));
  g_login_data->tb_username = temp_tb;

  ASSERT_EQ(CMN_SUCCESS, login_view_cleanup(1));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Exhaustively tests all OOM rollback paths in `login_view_factory`.
 * @return Greatest test result code.
 */
TEST test_login_factory_oom(void) {
  app_state_t state;
  void *widget = NULL;
  int i;

  memset(&state, 0, sizeof(state));

  for (i = 0; i < 10; i++) {
    cmn_error_e rc;
    (void)cmn_mock_alloc_set(1, i);
    rc = login_view_factory(&state, NULL, 0, &widget);
    (void)cmn_mock_alloc_set(0, 0);

    if (rc == CMN_SUCCESS) {
      ASSERT_EQ(CMN_SUCCESS, login_view_cleanup(1));
      break;
    }
    ASSERT_EQ(CMN_ERR_NOMEM, rc);
  }
  PASS();
}

/**
 * @brief Tests `on_logout_click` clearing username and navigating back to
 * `/login`.
 * @return Greatest test result code.
 */
TEST test_welcome_logout(void) {
  app_state_t state;
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  const char *cur_path = NULL;
  void *widget = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_strcpy_s(state.username, sizeof(state.username), "samuel"));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_register(router, "/login", login_view_factory, &state));
  ASSERT_EQ(CMN_SUCCESS, welcome_view_factory(&state, NULL, 0, &widget));

  ASSERT_EQ(CMN_SUCCESS, on_logout_click(g_welcome_data));
  ASSERT_STR_EQ("", state.username);
  ASSERT_EQ(CMN_SUCCESS, cmn_router_get_current_path(router, &cur_path));
  ASSERT_STR_EQ("/login", cur_path);

  ASSERT_EQ(CMN_SUCCESS, welcome_view_cleanup(1));
  ASSERT_EQ(CMN_SUCCESS, login_view_cleanup(0));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Tests `on_logout_click` when navigation to `/login` fails due to OOM.
 * @return Greatest test result code.
 */
TEST test_welcome_logout_oom(void) {
  app_state_t state;
  cmn_router_t *router = NULL;
  cmn_vbox_t *container = NULL;
  void *widget = NULL;

  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_create(&container));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_create(container, &router));
  ASSERT_EQ(CMN_SUCCESS, app_state_init(&state, router));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_strcpy_s(state.username, sizeof(state.username), "samuel"));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_router_register(router, "/login", login_view_factory, &state));
  ASSERT_EQ(CMN_SUCCESS, welcome_view_factory(&state, NULL, 0, &widget));

  (void)cmn_mock_alloc_set(1, 0);
  ASSERT_EQ(CMN_ERR_NOMEM, on_logout_click(g_welcome_data));
  (void)cmn_mock_alloc_set(0, 0);

  ASSERT_EQ(CMN_SUCCESS, welcome_view_cleanup(1));
  ASSERT_EQ(CMN_SUCCESS, cmn_router_destroy(&router));
  ASSERT_EQ(CMN_SUCCESS, cmn_vbox_destroy(&container));
  PASS();
}

/**
 * @brief Exhaustively tests all OOM rollback paths in `welcome_view_factory`.
 * @return Greatest test result code.
 */
TEST test_welcome_factory_oom(void) {
  app_state_t state;
  void *widget = NULL;
  int i;

  memset(&state, 0, sizeof(state));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_strcpy_s(state.username, sizeof(state.username), "samuel"));

  for (i = 0; i < 10; i++) {
    cmn_error_e rc;
    (void)cmn_mock_alloc_set(1, i);
    rc = welcome_view_factory(&state, NULL, 0, &widget);
    (void)cmn_mock_alloc_set(0, 0);

    if (rc == CMN_SUCCESS) {
      ASSERT_EQ(CMN_SUCCESS, welcome_view_cleanup(1));
      break;
    }
    ASSERT_EQ(CMN_ERR_NOMEM, rc);
  }
  PASS();
}

/**
 * @brief Test suite for login view.
 */
SUITE(login_view_suite) {
  RUN_TEST(test_login_validations);
  RUN_TEST(test_login_submit_invalid);
  RUN_TEST(test_login_submit_valid);
  RUN_TEST(test_login_submit_oom);
  RUN_TEST(test_login_submit_oom2);
  RUN_TEST(test_login_submit_null_form);
  RUN_TEST(test_login_submit_null_tb);
  RUN_TEST(test_login_factory_oom);
}

/**
 * @brief Test suite for welcome view.
 */
SUITE(welcome_view_suite) {
  RUN_TEST(test_welcome_logout);
  RUN_TEST(test_welcome_logout_oom);
  RUN_TEST(test_welcome_factory_oom);
}

/**
 * @brief Tests `app_main` under OOM and normal execution.
 * @return Greatest test result code.
 */
TEST test_app_main(void) {
  int i;
  int succeeded = 0;

  for (i = 0; i < 15; i++) {
    cmn_error_e rc;
    (void)cmn_mock_alloc_set(1, i);
    rc = app_main();
    (void)cmn_mock_alloc_set(0, 0);
    if (rc == CMN_SUCCESS) {
      succeeded = 1;
      break;
    }
    ASSERT_EQ(CMN_ERR_NOMEM, rc);
  }
  ASSERT_EQ(1, succeeded);

  PASS();
}

/**
 * @brief Test suite for app state.
 */
SUITE(app_state_suite) { RUN_TEST(test_app_state_init); }

/**
 * @brief Test suite for main application flow.
 */
SUITE(main_suite) { RUN_TEST(test_app_main); }

/**
 * @brief Test runner entry point for simple_password_flow tests.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Process exit code.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(app_state_suite);
  RUN_SUITE(login_view_suite);
  RUN_SUITE(welcome_view_suite);
  RUN_SUITE(main_suite);
  GREATEST_MAIN_END();
}
