/* clang-format off */
#include "greatest.h"
#include "cmn/memory.h"
#include "cmn/types.h"
/* clang-format on */

/**
 * @file test_memory.c
 * @brief Unit tests for `cmn_malloc`, `cmn_calloc`, `cmn_realloc`, and
 * `cmn_free`.
 */

/** @brief External test helper `cmn_mock_alloc_set`. */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);

/**
 * @brief Tests `cmn_malloc` normal allocation, zero-size allocation, countdown
 * mock allocation, and NULL argument checks.
 * @return Greatest test result code.
 */
TEST test_memory_malloc(void) {
  void *ptr = NULL;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_malloc(10, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_malloc(0, &ptr));
  ASSERT(ptr == NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_malloc(10, &ptr));
  ASSERT(ptr == NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_malloc(10, &ptr));
  ASSERT(ptr != NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_free(&ptr));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_malloc(10, &ptr));
  ASSERT(ptr != NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_free(&ptr));
  PASS();
}

/**
 * @brief Tests `cmn_calloc` zero-initialized allocation, countdown mock
 * allocation, and NULL checks.
 * @return Greatest test result code.
 */
TEST test_memory_calloc(void) {
  void *ptr = NULL;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_calloc(10, 10, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_calloc(0, 10, &ptr));
  ASSERT(ptr == NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_calloc(10, 0, &ptr));
  ASSERT(ptr == NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_calloc(10, 10, &ptr));
  ASSERT(ptr == NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_calloc(10, 10, &ptr));
  ASSERT(ptr != NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_free(&ptr));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_calloc(10, 10, &ptr));
  ASSERT(ptr != NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_free(&ptr));
  PASS();
}

/**
 * @brief Tests `cmn_realloc` resizing, countdown mock allocation, shrinking to
 * zero, and NULL checks.
 * @return Greatest test result code.
 */
TEST test_memory_realloc(void) {
  void *ptr = NULL;
  void *ptr2 = NULL;
  ASSERT_EQ(CMN_SUCCESS, cmn_malloc(10, &ptr));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_realloc(ptr, 20, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_realloc(ptr, 20, &ptr2));
  ASSERT(ptr2 == NULL);
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_realloc(ptr, 20, &ptr2));
  ASSERT(ptr2 != NULL);
  ptr = ptr2;
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_realloc(ptr, 30, &ptr2));
  ASSERT(ptr2 != NULL);
  ptr = ptr2;
  ASSERT_EQ(CMN_SUCCESS, cmn_realloc(ptr, 0, &ptr2));
  ASSERT(ptr2 == NULL);
  PASS();
}

/**
 * @brief Tests `cmn_free` with NULL and already-freed pointers.
 * @return Greatest test result code.
 */
TEST test_memory_free(void) {
  void *ptr = NULL;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_free(NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_free(&ptr));
  ASSERT_EQ(CMN_SUCCESS, cmn_malloc(10, &ptr));
  ASSERT_EQ(CMN_SUCCESS, cmn_free(&ptr));
  ASSERT(ptr == NULL);
  PASS();
}

/**
 * @brief Tests `cmn_malloc` system OOM with `SIZE_MAX`.
 * @return Greatest test result code.
 */
TEST test_memory_malloc_oom(void) {
  void *ptr = (void *)0x1;
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_malloc(SIZE_MAX, &ptr));
  PASS();
}

/**
 * @brief Tests `cmn_calloc` system OOM with `SIZE_MAX`.
 * @return Greatest test result code.
 */
TEST test_memory_calloc_oom(void) {
  void *ptr = (void *)0x1;
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_calloc(SIZE_MAX, SIZE_MAX, &ptr));
  PASS();
}

/**
 * @brief Tests `cmn_realloc` system OOM with `SIZE_MAX`.
 * @return Greatest test result code.
 */
TEST test_memory_realloc_oom(void) {
  void *ptr = NULL;
  void *ptr2 = (void *)0x1;
  ASSERT_EQ(CMN_SUCCESS, cmn_malloc(10, &ptr));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_realloc(ptr, SIZE_MAX, &ptr2));
  ASSERT_EQ(CMN_SUCCESS, cmn_free(&ptr));
  PASS();
}

/**
 * @brief Test suite for memory management functions.
 */
SUITE(memory_suite) {
  RUN_TEST(test_memory_malloc);
  RUN_TEST(test_memory_calloc);
  RUN_TEST(test_memory_realloc);
  RUN_TEST(test_memory_free);
  RUN_TEST(test_memory_malloc_oom);
  RUN_TEST(test_memory_calloc_oom);
  RUN_TEST(test_memory_realloc_oom);
}
