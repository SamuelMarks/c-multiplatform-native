#ifndef CMN_ROOM_H
#define CMN_ROOM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
#include "c_orm_db.h"
#include "c_orm_meta.h"
#include "c_abstract_http/http_types.h"
/* clang-format on */

/**
 * @file room.h
 * @brief Room-like consistency and ORM wrapper over c-orm and c-abstract-http.
 */

/**
 * @defgroup cmn_room Room ORM and Sync
 * @brief Room ORM and Sync module.
 * @{
 */

/** @brief Opaque handle for a Room database connection. */
typedef struct cmn_room_db_s cmn_room_db_t;

/** @brief Opaque handle for a Room Data Access Object (DAO). */
typedef struct cmn_room_dao_s cmn_room_dao_t;

/**
 * @brief Creates and initializes a Room database backed by SQLite.
 * @param db_path File path to the SQLite database (or ":memory:").
 * @param out_db Output pointer to the initialized Room database.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_room_db_create(const char *db_path,
                                              cmn_room_db_t **out_db);

/**
 * @brief Destroys a Room database and closes its underlying connection.
 * @param db Pointer to the Room database pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_room_db_destroy(cmn_room_db_t **db);

/**
 * @brief Executes a raw SQL query (e.g. CREATE TABLE) on the Room database.
 * @param db The Room database.
 * @param sql The raw SQL string to execute.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_room_db_execute(cmn_room_db_t *db,
                                               const char *sql);

/**
 * @brief Creates a Data Access Object (DAO) for a specific entity metadata.
 * @param db The Room database.
 * @param meta The c-orm table metadata describing the entity.
 * @param out_dao Output pointer to the created DAO.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_room_dao_create(cmn_room_db_t *db,
                                               const c_orm_table_meta_t *meta,
                                               cmn_room_dao_t **out_dao);

/**
 * @brief Destroys a DAO.
 * @param dao Pointer to the DAO pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_room_dao_destroy(cmn_room_dao_t **dao);

/**
 * @brief Inserts an entity using the DAO.
 * @param dao The DAO.
 * @param entity Pointer to the C struct representing the entity.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_room_dao_insert(cmn_room_dao_t *dao,
                                               const void *entity);

/**
 * @brief Finds an entity by its int32 primary key.
 * @param dao The DAO.
 * @param id The primary key ID.
 * @param out_entity Pointer to the C struct to populate.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if absent, error code
 * otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_room_dao_find_by_id_int32(cmn_room_dao_t *dao,
                                                         int32_t id,
                                                         void *out_entity);

/**
 * @brief Synchronizes local entities with a REST endpoint via c-abstract-http.
 * @param dao The DAO.
 * @param http_client The HTTP client instance from c-abstract-http.
 * @param sync_url The REST endpoint URL.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_room_dao_sync(cmn_room_dao_t *dao,
                                             struct HttpClient *http_client,
                                             const char *sync_url);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_ROOM_H */
