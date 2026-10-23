/* clang-format off */
#include "greatest.h"
#include "cmn/room.h"
#include "cmn/error.h"
#include "cmn/memory.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @file test_room.c
 * @brief Behavioral unit tests for Room ORM CRUD and HTTP synchronization.
 */

/**
 * @brief Test entity structure mapped to `test_table`.
 */
struct test_entity_s {
  int32_t id; /**< Primary key ID. */
  char *name; /**< Entity name string. */
};

/** @brief External test helper `cmn_mock_alloc_set`. */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);

/** @brief Static HTTP response used by mock HTTP transports. */
static struct HttpResponse g_mock_http_res;

/**
 * @brief Mock HTTP transport that succeeds with 200 OK.
 * @param ctx Counter pointer incremented on send.
 * @param req HTTP request structure.
 * @param res Output HTTP response pointer.
 * @return C_ABSTRACT_HTTP_OK on success.
 */
static c_abstract_http_error_t
mock_http_send_ok(struct HttpTransportContext *ctx,
                  const struct HttpRequest *req, struct HttpResponse **res) {
  int *called = (int *)(void *)ctx;
  if (called != NULL && req != NULL) {
    (*called)++;
  }
  memset(&g_mock_http_res, 0, sizeof(g_mock_http_res));
  g_mock_http_res.status_code = 200;
  *res = &g_mock_http_res;
  return C_ABSTRACT_HTTP_SUCCESS;
}

/**
 * @brief Mock HTTP transport that returns a 500 error status.
 * @param ctx Unused context pointer.
 * @param req Unused HTTP request.
 * @param res Output HTTP response pointer populated with status 500.
 * @return C_ABSTRACT_HTTP_SUCCESS.
 */
static c_abstract_http_error_t
mock_http_send_500(struct HttpTransportContext *ctx,
                   const struct HttpRequest *req, struct HttpResponse **res) {
  if (res == NULL || (ctx != NULL && req == NULL)) {
    return (c_abstract_http_error_t)1;
  }
  memset(&g_mock_http_res, 0, sizeof(g_mock_http_res));
  g_mock_http_res.status_code = 500;
  *res = &g_mock_http_res;
  return C_ABSTRACT_HTTP_SUCCESS;
}

/**
 * @brief Mock HTTP transport that fails at the transport layer.
 * @param ctx Unused context pointer.
 * @param req Unused HTTP request.
 * @param res Output HTTP response pointer set to NULL.
 * @return Non-zero transport error.
 */
static c_abstract_http_error_t
mock_http_send_err(struct HttpTransportContext *ctx,
                   const struct HttpRequest *req, struct HttpResponse **res) {
  if (res != NULL) {
    *res = NULL;
  }
  if (ctx != NULL || req == NULL) {
    return (c_abstract_http_error_t)2;
  }
  return (c_abstract_http_error_t)1;
}

/**
 * @brief Tests basic Room database creation, table execution, insert, find by
 * ID, `CMN_ERR_NOT_FOUND` on missing row, and HTTP synchronization.
 * @return Greatest test result code.
 */
TEST test_room_basic_crud(void) {
  cmn_room_db_t *db = NULL;
  cmn_room_dao_t *dao = NULL;
  c_orm_table_meta_t meta;
  c_orm_column_meta_t cols[2];
  struct test_entity_s in_entity;
  struct test_entity_s out_entity;
  struct HttpClient http_client;
  int sync_calls = 0;

  memset(&meta, 0, sizeof(meta));
  memset(cols, 0, sizeof(cols));
  memset(&in_entity, 0, sizeof(in_entity));
  memset(&out_entity, 0, sizeof(out_entity));
  memset(&http_client, 0, sizeof(http_client));

  cols[0].name = "id";
  cols[0].type = C_ORM_TYPE_INT32;
  cols[0].offset = offsetof(struct test_entity_s, id);
  cols[0].is_pk = 1;

  cols[1].name = "name";
  cols[1].type = C_ORM_TYPE_STRING;
  cols[1].offset = offsetof(struct test_entity_s, name);

  meta.name = "test_table";
  meta.columns = cols;
  meta.num_columns = 2;
  meta.struct_size = sizeof(struct test_entity_s);
  meta.query_insert = "INSERT INTO test_table (id, name) VALUES (?, ?)";
  meta.query_select_by_pk = "SELECT * FROM test_table WHERE id=?";

  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_create(":memory:", &db));
  ASSERT_EQ(
      CMN_SUCCESS,
      cmn_room_db_execute(
          db, "CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT)"));
  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_create(db, &meta, &dao));

  in_entity.id = 42;
  in_entity.name = "John Doe";
  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_insert(dao, &in_entity));

  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_find_by_id_int32(dao, 42, &out_entity));
  ASSERT_EQ(42, out_entity.id);
  ASSERT_STR_EQ("John Doe", out_entity.name);
  if (out_entity.name) {
    ASSERT_EQ(C_ORM_OK, c_orm_system_free(out_entity.name));
  }

  /* Finding a non-existent primary key returns CMN_ERR_NOT_FOUND */
  ASSERT_EQ(CMN_ERR_NOT_FOUND,
            cmn_room_dao_find_by_id_int32(dao, 999, &out_entity));

  /* HTTP sync with real HttpClient callback */
  http_client.transport = (struct HttpTransportContext *)(void *)&sync_calls;
  http_client.send = mock_http_send_ok;
  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_sync(dao, &http_client,
                                           "https://api.example.com/tasks"));
  ASSERT_EQ(1, sync_calls);

  http_client.send = mock_http_send_500;
  ASSERT_EQ(CMN_ERR_IO, cmn_room_dao_sync(dao, &http_client,
                                          "https://api.example.com/tasks"));

  http_client.send = mock_http_send_err;
  ASSERT_EQ(CMN_ERR_IO, cmn_room_dao_sync(dao, &http_client,
                                          "https://api.example.com/tasks"));

  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_destroy(&dao));
  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_destroy(&db));
  PASS();
}

/**
 * @brief Tests NULL argument checks across all Room database and DAO functions.
 * @return Greatest test result code.
 */
TEST test_room_nulls(void) {
  cmn_room_db_t *db = NULL;
  cmn_room_dao_t *dao = NULL;
  c_orm_table_meta_t meta;
  struct HttpClient dummy_http;

  memset(&meta, 0, sizeof(meta));
  memset(&dummy_http, 0, sizeof(dummy_http));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_db_create(NULL, &db));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_db_create(":memory:", NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_db_destroy(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_db_destroy(&db));

  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_create(":memory:", &db));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_db_execute(NULL, "SELECT 1"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_db_execute(db, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_create(NULL, &meta, &dao));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_create(db, NULL, &dao));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_create(db, &meta, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_destroy(NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_destroy(&dao));

  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_create(db, &meta, &dao));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_insert(NULL, (void *)0x1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_insert(dao, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_find_by_id_int32(NULL, 1, (void *)0x1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_find_by_id_int32(dao, 1, NULL));

  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_sync(NULL, &dummy_http, "http"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_sync(dao, NULL, "http"));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_sync(dao, &dummy_http, NULL));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_dao_sync(dao, &dummy_http, ""));
  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_sync(dao, &dummy_http, "http"));

  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_destroy(&dao));
  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_destroy(&db));
  PASS();
}

/**
 * @brief Tests OOM handling in `cmn_room_db_create` and `cmn_room_dao_create`.
 * @return Greatest test result code.
 */
TEST test_room_oom(void) {
  cmn_room_db_t *db = NULL;
  cmn_room_dao_t *dao = NULL;
  c_orm_table_meta_t meta;
  memset(&meta, 0, sizeof(meta));

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_room_db_create(":memory:", &db));

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_create(":memory:", &db));

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_room_dao_create(db, &meta, &dao));

  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));
  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_destroy(&db));

  PASS();
}

/**
 * @brief Tests `cmn_room_db_execute` with invalid SQL syntax.
 * @return Greatest test result code.
 */
TEST test_room_db_execute_io_errors(void) {
  cmn_room_db_t *db = NULL;
  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_create(":memory:", &db));

  ASSERT_EQ(CMN_ERR_IO, cmn_room_db_execute(db, "INVALID SQL JIBBERISH"));

  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_destroy(&db));
  PASS();
}

/**
 * @brief Tests `cmn_room_db_create` with an inaccessible database file path.
 * @return Greatest test result code.
 */
TEST test_room_db_connect_error(void) {
  cmn_room_db_t *db = NULL;
  ASSERT_EQ(CMN_ERR_IO,
            cmn_room_db_create("/root/invalid/path/db.sqlite", &db));
  PASS();
}

/**
 * @brief Tests `cmn_room_dao_insert` and `cmn_room_dao_find_by_id_int32` when
 * the underlying table does not exist.
 * @return Greatest test result code.
 */
TEST test_room_dao_io_errors(void) {
  cmn_room_db_t *db = NULL;
  cmn_room_dao_t *dao = NULL;
  c_orm_table_meta_t meta;
  c_orm_column_meta_t cols[1];
  struct test_entity_s in_entity;
  struct test_entity_s out_entity;

  memset(&meta, 0, sizeof(meta));
  memset(cols, 0, sizeof(cols));
  memset(&in_entity, 0, sizeof(in_entity));

  cols[0].name = "id";
  cols[0].type = C_ORM_TYPE_INT32;
  cols[0].offset = offsetof(struct test_entity_s, id);
  cols[0].is_pk = 1;

  meta.name = "non_existent_table";
  meta.columns = cols;
  meta.num_columns = 1;
  meta.struct_size = sizeof(struct test_entity_s);
  meta.query_insert = "INSERT INTO non_existent_table (id) VALUES (?)";
  meta.query_select_by_pk = "SELECT * FROM non_existent_table WHERE id=?";

  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_create(":memory:", &db));
  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_create(db, &meta, &dao));

  ASSERT_EQ(CMN_ERR_IO, cmn_room_dao_insert(dao, &in_entity));
  ASSERT_EQ(CMN_ERR_IO, cmn_room_dao_find_by_id_int32(dao, 1, &out_entity));

  ASSERT_EQ(CMN_SUCCESS, cmn_room_dao_destroy(&dao));
  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_destroy(&db));

  PASS();
}

/**
 * @brief Mirror of internal `cmn_room_db_s` for driver vtable fault injection.
 */
struct cmn_room_db_s {
  c_orm_db_t *orm_db; /**< Underlying c-orm database handle. */
};

/**
 * @brief Mock c-orm step function that returns `C_ORM_ERROR_STEP`.
 * @param query Unused query handle.
 * @param out_has_row Unused row indicator.
 * @return C_ORM_ERROR_STEP.
 */
static c_orm_error_t mock_step_fail(c_orm_query_t *query, int *out_has_row) {
  if (out_has_row != NULL) {
    *out_has_row = (query == NULL) ? -1 : 0;
  }
  return C_ORM_ERROR_STEP;
}

/**
 * @brief Mock c-orm prepare function that succeeds.
 * @param db Unused database handle.
 * @param sql Unused SQL string.
 * @param out_query Output query pointer.
 * @return C_ORM_OK.
 */
static c_orm_error_t mock_prepare_ok(c_orm_db_t *db, const char *sql,
                                     c_orm_query_t **out_query) {
  if (db == NULL || sql == NULL || out_query == NULL) {
    return C_ORM_ERROR_MEMORY;
  }
  *out_query = (c_orm_query_t *)0x1234;
  return C_ORM_OK;
}

/**
 * @brief Mock c-orm finalize function that succeeds.
 * @param query Unused query handle.
 * @return C_ORM_OK.
 */
static c_orm_error_t mock_finalize_ok(c_orm_query_t *query) {
  return (query != NULL) ? C_ORM_OK : C_ORM_ERROR_MEMORY;
}

/**
 * @brief Tests driver vtable edge cases and NULL `orm_db` branches.
 * @return Greatest test result code.
 */
TEST test_room_mocked_errors(void) {
  cmn_room_db_t *db = NULL;
  c_orm_db_t fake_orm_db;
  c_orm_driver_vtable_t fake_vtable;
  struct cmn_room_db_s *fake_db;
  c_orm_db_t *orig_orm;

  memset(&fake_orm_db, 0, sizeof(fake_orm_db));
  memset(&fake_vtable, 0, sizeof(fake_vtable));

  fake_vtable.prepare = mock_prepare_ok;
  fake_vtable.step = mock_step_fail;
  fake_vtable.finalize = mock_finalize_ok;
  fake_orm_db.vtable = &fake_vtable;

  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_create(":memory:", &db));
  fake_db = (struct cmn_room_db_s *)db;
  orig_orm = fake_db->orm_db;

  fake_db->orm_db = &fake_orm_db;
  ASSERT_EQ(CMN_ERR_IO, cmn_room_db_execute(db, "SELECT 1"));

  fake_db->orm_db = NULL;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_room_db_execute(db, "SELECT 1"));

  if (orig_orm && orig_orm->vtable && orig_orm->vtable->disconnect) {
    orig_orm->vtable->disconnect(orig_orm);
  }
  ASSERT_EQ(CMN_SUCCESS, cmn_room_db_destroy(&db));

  PASS();
}

/**
 * @brief Test suite for Room ORM and synchronization.
 */
SUITE(room_suite) {
  RUN_TEST(test_room_basic_crud);
  RUN_TEST(test_room_nulls);
  RUN_TEST(test_room_oom);
  RUN_TEST(test_room_db_execute_io_errors);
  RUN_TEST(test_room_db_connect_error);
  RUN_TEST(test_room_dao_io_errors);
  RUN_TEST(test_room_mocked_errors);
}
