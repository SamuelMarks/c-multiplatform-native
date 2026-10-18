/* clang-format off */
#include "room_app.h"
#include "cmn/string_safe.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @file room_app.c
 * @brief Implementation of the Room ORM example workflow with responsive
 * Flexbox and Grid layout.
 */

/**
 * @brief Executes the Room ORM example workflow (create table, insert task,
 * query task by ID, and lay out the task row inside a responsive CSS Grid and
 * Flexbox container).
 * @param out_found_id Optional output pointer receiving the queried task ID.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e room_example_run(int32_t *out_found_id) {
  cmn_room_db_t *db = NULL;
  cmn_room_dao_t *dao = NULL;
  cmn_grid_t *grid = NULL;
  cmn_flex_t *row_flex = NULL;
  cmn_label_t *lbl_id = NULL;
  cmn_label_t *lbl_title = NULL;
  c_orm_table_meta_t meta;
  c_orm_column_meta_t cols[2];
  cmn_grid_track_t grid_cols[2];
  cmn_edges_t pad = {12, 16, 12, 16};
  cmn_flex_item_style_t grow_style;
  struct task_s task_in;
  struct task_s task_out;
  char id_buf[32];
  cmn_error_e rc;

  if (out_found_id == NULL) {
    return CMN_ERR_INVAL;
  }

  memset(&meta, 0, sizeof(meta));
  memset(cols, 0, sizeof(cols));
  memset(grid_cols, 0, sizeof(grid_cols));
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

  rc = cmn_room_dao_create(db, &meta, &dao);
  if (rc != CMN_SUCCESS) {
    cmn_error_e db_rc = cmn_room_db_destroy(&db);
    (void)db_rc;
    return rc;
  }

  task_in.id = 1;
  task_in.title = "Implement Room in C89";
  rc = cmn_room_dao_insert(dao, &task_in);
  rc = cmn_room_dao_find_by_id_int32(dao, 1, &task_out);

  rc = cmn_grid_create(&grid);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  grid_cols[0].unit = CMN_GRID_UNIT_FR;
  grid_cols[0].value = 1.0f;
  rc = cmn_grid_set_columns(grid, grid_cols, 1);
  rc = cmn_grid_set_gap(grid, 8, 8);
  rc = cmn_grid_set_padding(grid, pad);
  rc = cmn_grid_add_breakpoint(grid, 360, 1);

  rc = cmn_flex_create(&row_flex);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_flex_set_direction(row_flex, CMN_FLEX_ROW);
  rc = cmn_flex_set_gap(row_flex, 8, 12);
  rc = cmn_flex_add_breakpoint(row_flex, 360, CMN_FLEX_COLUMN, CMN_FLEX_NOWRAP);

  rc = cmn_label_create(&lbl_id);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_sprintf_s(id_buf, sizeof(id_buf), "#" CMN_INT32_FMT,
                     (int)task_out.id);
  rc = cmn_label_set_text(lbl_id, id_buf);

  rc = cmn_label_create(&lbl_title);
  if (rc != CMN_SUCCESS) {
    goto cleanup;
  }
  rc = cmn_label_set_text(lbl_title,
                          task_out.title != NULL ? task_out.title : "");

  rc = cmn_flex_add_child(row_flex, lbl_id, NULL);
  rc = cmn_flex_item_style_init(&grow_style);
  grow_style.flex_grow = 1.0f;
  rc = cmn_flex_add_child(row_flex, lbl_title, &grow_style);
  rc = cmn_grid_add_child(grid, row_flex, NULL);
  rc = cmn_grid_compute_layout(grid, 480, 120);

  *out_found_id = task_out.id;

cleanup:
  if (lbl_title != NULL) {
    cmn_error_e clean_rc = cmn_label_destroy(&lbl_title);
    (void)clean_rc;
  }
  if (lbl_id != NULL) {
    cmn_error_e clean_rc = cmn_label_destroy(&lbl_id);
    (void)clean_rc;
  }
  if (row_flex != NULL) {
    cmn_error_e clean_rc = cmn_flex_destroy(&row_flex);
    (void)clean_rc;
  }
  if (grid != NULL) {
    cmn_error_e clean_rc = cmn_grid_destroy(&grid);
    (void)clean_rc;
  }
  if (task_out.title != NULL) {
    free(task_out.title);
  }
  {
    cmn_error_e dao_rc = cmn_room_dao_destroy(&dao);
    cmn_error_e db_rc = cmn_room_db_destroy(&db);
    (void)dao_rc;
    (void)db_rc;
  }
  return rc;
}
