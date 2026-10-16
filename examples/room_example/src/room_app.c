/* clang-format off */
#include "room_app.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @file room_app.c
 * @brief Implementation of the Room ORM example workflow.
 */

/**
 * @brief Executes the Room ORM example workflow (create table, insert task,
 * query task by ID, and verify title).
 * @param out_found_id Optional output pointer receiving the queried task ID.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e room_example_run(int32_t *out_found_id) {
  cmn_room_db_t *db = NULL;
  cmn_room_dao_t *dao = NULL;
  c_orm_table_meta_t meta;
  c_orm_column_meta_t cols[2];
  struct task_s task_in;
  struct task_s task_out;
  cmn_error_e rc;

  if (out_found_id == NULL) {
    return CMN_ERR_INVAL;
  }

  memset(&meta, 0, sizeof(meta));
  memset(cols, 0, sizeof(cols));
  memset(&task_in, 0, sizeof(task_in));
  memset(&task_out, 0, sizeof(task_out));

  cols[0].name = "id";
  cols[0].type = C_ORM_TYPE_INT32;
  cols[0].offset = offsetof(struct task_s, id);
  cols[0].is_pk = 1;

  cols[1].name = "title";
  cols[1].type = C_ORM_TYPE_STRING;
  cols[1].offset = offsetof(struct task_s, title);

  meta.name = "tasks";
  meta.columns = cols;
  meta.num_columns = 2;
  meta.struct_size = sizeof(struct task_s);
  meta.query_insert = "INSERT INTO tasks (id, title) VALUES (?, ?)";
  meta.query_select_by_pk = "SELECT * FROM tasks WHERE id=?";

  rc = cmn_room_db_create(":memory:", &db);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_room_db_execute(
      db, "CREATE TABLE tasks (id INTEGER PRIMARY KEY, title TEXT)");
  if (rc != CMN_SUCCESS) {
    (void)cmn_room_db_destroy(&db);
    return rc;
  }

  rc = cmn_room_dao_create(db, &meta, &dao);
  if (rc != CMN_SUCCESS) {
    (void)cmn_room_db_destroy(&db);
    return rc;
  }

  task_in.id = 1;
  task_in.title = "Implement Room in C89";
  rc = cmn_room_dao_insert(dao, &task_in);
  if (rc != CMN_SUCCESS) {
    (void)cmn_room_dao_destroy(&dao);
    (void)cmn_room_db_destroy(&db);
    return rc;
  }
  rc = cmn_room_dao_find_by_id_int32(dao, 1, &task_out);
  if (rc != CMN_SUCCESS) {
    (void)cmn_room_dao_destroy(&dao);
    (void)cmn_room_db_destroy(&db);
    return rc;
  }

  *out_found_id = task_out.id;
  if (task_out.title != NULL) {
    free(task_out.title);
  }

  rc = cmn_room_dao_destroy(&dao);
  if (rc == CMN_SUCCESS) {
    rc = cmn_room_db_destroy(&db);
  }
  return rc;
}
