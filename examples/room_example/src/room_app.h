#ifndef ROOM_EXAMPLE_APP_H
#define ROOM_EXAMPLE_APP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/room.h"
/* clang-format on */

/**
 * @file room_app.h
 * @brief Room ORM task entity and workflow declarations.
 */

/**
 * @defgroup example_room Room ORM Example
 * @brief Room ORM Example module.
 * @{
 */

/**
 * @brief Task entity structure mapped to the `tasks` SQLite table.
 */
struct task_s {
  int32_t id;  /**< Primary key task ID. */
  char *title; /**< Task title string. */
};

/**
 * @brief Executes the Room ORM example workflow (create table, insert task,
 * query task by ID, and verify title).
 * @param out_found_id Optional output pointer receiving the queried task ID.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e room_example_run(int32_t *out_found_id);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ROOM_EXAMPLE_APP_H */
