/* clang-format off */
#include "greatest.h"
#include "cmn/error.h"
/* Note: handle_map.h is internal, so include from src */
#include "../src/handle_map.h"
/* clang-format on */

/**
 * @file test_handle_map.c
 * @brief Unit tests for `cmn_handle_map` thread-safe native handle mapping.
 */

/** @brief External test helper `cmn_handle_map_mock_mutex_init_set`. */
extern cmn_error_e cmn_handle_map_mock_mutex_init_set(int fail);
/** @brief External test helper `cmn_handle_map_mock_mutex_lock_set`. */
extern cmn_error_e cmn_handle_map_mock_mutex_lock_set(int fail);
/** @brief External test helper `cmn_handle_map_mock_mutex_unlock_set`. */
extern cmn_error_e cmn_handle_map_mock_mutex_unlock_set(int fail);
/** @brief External test helper `cmn_handle_map_mock_mutex_free_set`. */
extern cmn_error_e cmn_handle_map_mock_mutex_free_set(int fail);

TEST test_handle_map_init_and_cleanup(void) {
  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  /* Double init should return success */
  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());

  /* Double cleanup should return success */
  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());
  PASS();
}

TEST test_handle_map_init_oom(void) {
  (void)cmn_handle_map_mock_mutex_init_set(1);
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_handle_map_init());
  (void)cmn_handle_map_mock_mutex_init_set(0);
  PASS();
}

TEST test_handle_map_add_null(void) {
  void *native = (void *)0x1;
  void *cmn = (void *)0x2;

  /* Before init */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_add(native, cmn));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_add(NULL, cmn));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_add(native, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_add(native, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());
  PASS();
}

TEST test_handle_map_get_null(void) {
  void *native = (void *)0x1;
  void *cmn;

  /* Before init */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_get(native, &cmn));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_get(NULL, &cmn));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_get(native, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_get(native, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());
  PASS();
}

TEST test_handle_map_remove_null(void) {
  void *native = (void *)0x1;

  /* Before init */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_remove(native));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  ASSERT_EQ(CMN_ERR_INVAL, cmn_handle_map_remove(NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());
  PASS();
}

TEST test_handle_map_flow(void) {
  void *native1 = (void *)0x10;
  void *cmn1 = (void *)0x11;
  void *native2 = (void *)0x20;
  void *cmn2 = (void *)0x21;
  void *out;

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_add(native1, cmn1));
  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_add(native2, cmn2));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_get(native1, &out));
  ASSERT_EQ(cmn1, out);

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_get(native2, &out));
  ASSERT_EQ(cmn2, out);

  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_handle_map_get((void *)0x30, &out));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_remove(native1));
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_handle_map_get(native1, &out));

  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_handle_map_remove(native1));

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());
  PASS();
}

TEST test_handle_map_capacity(void) {
  int i;
  char *base = (char *)0x1000;

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  /* Handle map is globally 1024 capacity. The other tests might have
     added/removed things. Adding exactly 1024 new items might fail if the
     capacity is shared globally and didn't clean up well. Wait,
     cmn_handle_map_cleanup clears the array now. So it's fine. */
  /* Fill to capacity (1024) */
  for (i = 0; i < 1024; i++) {
    ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_add(base + i, base + i));
  }

  /* Next should fail with NOMEM */
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_handle_map_add((void *)0x2000, (void *)0x2000));

  /* Remove one, then add should succeed */
  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_remove(base + 500));
  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_add((void *)0x2000, (void *)0x2000));

  /* Cleanup clears everything... wait, cleanup just frees mutex and unsets init
     flag. Adding again would find the map full if we don't zero it. Let's check
     if cmn_handle_map_add handles it properly across cleanups. Actually,
     cmn_handle_map_cleanup doesn't zero g_handle_map. */

  /* Manually empty it for tests */
  for (i = 0; i < 1024; i++) {
    if (i == 500) {
      (void)cmn_handle_map_remove((void *)0x2000);
    } else {
      (void)cmn_handle_map_remove(base + i);
    }
  }

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());
  PASS();
}

TEST test_handle_map_mutex_failures(void) {
  void *native1 = (void *)0x10;
  void *cmn1 = (void *)0x11;
  void *out;

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  (void)cmn_handle_map_mock_mutex_lock_set(1);
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_add(native1, cmn1));
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_get(native1, &out));
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_remove(native1));
  (void)cmn_handle_map_mock_mutex_lock_set(0);

  (void)cmn_handle_map_mock_mutex_unlock_set(1);
  /* test the unlock fail in the loop (found case) */
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_add(native1, cmn1));
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_get(native1, &out));
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_remove(native1));

  /* Now test the unlock fail after the loop exhausts (not found / full case) */
  /* For add, we need it to be full. Let's make it full. */
  /* For get/remove, just querying something not there works since the loop
   * finishes without finding it. */
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_get((void *)0x999, &out));
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_remove((void *)0x999));

  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_add(native1, cmn1));
  /* now it is added, but unlock failed, so return IO. The state might be dirty.
   */
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_add((void *)0x20, cmn1));
  /* get */
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_get(native1, &out));
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_get((void *)0x30, &out));
  /* remove */
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_remove(native1));
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_remove((void *)0x40));

  (void)cmn_handle_map_mock_mutex_unlock_set(0);

  /* Make sure cleanup mock works too */
  (void)cmn_handle_map_mock_mutex_free_set(1);
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_cleanup());
  (void)cmn_handle_map_mock_mutex_free_set(0);

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());
  PASS();
}

TEST test_handle_map_unlock_fail_after_loop(void) {
  int i;
  char *base = (char *)0x5000;
  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_init());

  for (i = 0; i < 1024; i++) {
    ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_add(base + i, base + i));
  }

  (void)cmn_handle_map_mock_mutex_unlock_set(1);
  /* Full, fails to add, tries to unlock -> returns IO instead of NOMEM */
  ASSERT_EQ(CMN_ERR_IO, cmn_handle_map_add((void *)0x6000, (void *)0x6000));
  (void)cmn_handle_map_mock_mutex_unlock_set(0);

  for (i = 0; i < 1024; i++) {
    ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_remove(base + i));
  }

  ASSERT_EQ(CMN_SUCCESS, cmn_handle_map_cleanup());
  PASS();
}

SUITE(handle_map_suite) {
  RUN_TEST(test_handle_map_init_and_cleanup);
  RUN_TEST(test_handle_map_init_oom);
  RUN_TEST(test_handle_map_add_null);
  RUN_TEST(test_handle_map_get_null);
  RUN_TEST(test_handle_map_remove_null);
  RUN_TEST(test_handle_map_flow);
  RUN_TEST(test_handle_map_capacity);
  RUN_TEST(test_handle_map_unlock_fail_after_loop);
  RUN_TEST(test_handle_map_mutex_failures);
}
