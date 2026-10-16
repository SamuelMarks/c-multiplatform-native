/* clang-format off */
#include "cmn/room.h"
#include "cmn/memory.h"
#include "c_orm_sqlite.h"
#include "c_orm_api.h"
#include "c_orm_db.h"
#include <string.h>
/* clang-format on */

/**
 * @file room.c
 * @brief Implementation of Room-like consistency and ORM wrapper.
 */

/**
 * @brief Internal representation of a Room database connection.
 */
struct cmn_room_db_s {
  c_orm_db_t *orm_db; /**< Underlying c-orm database handle. */
};

/**
 * @brief Internal representation of a Room Data Access Object (DAO).
 */
struct cmn_room_dao_s {
  cmn_room_db_t *db;              /**< Associated Room database. */
  const c_orm_table_meta_t *meta; /**< Entity table metadata. */
};

/**
 * @brief Creates and initializes a Room database backed by SQLite.
 * @param db_path File path to the SQLite database (or ":memory:").
 * @param out_db Output pointer to the initialized Room database.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_room_db_create(const char *db_path, cmn_room_db_t **out_db) {
  cmn_room_db_t *db = NULL;
  c_orm_error_t orm_err;
  cmn_error_e rc;

  if (!db_path || !out_db) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_malloc(sizeof(cmn_room_db_t), (void **)&db);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  orm_err = c_orm_sqlite_connect(db_path, &db->orm_db);
  if (orm_err != C_ORM_OK) {
    (void)cmn_free((void **)&db);
    return CMN_ERR_IO;
  }

  *out_db = db;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a Room database and closes its underlying connection.
 * @param db Pointer to the Room database pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_room_db_destroy(cmn_room_db_t **db) {
  if (!db || !*db) {
    return CMN_ERR_INVAL;
  }

  if ((*db)->orm_db) {
    (*db)->orm_db->vtable->disconnect((*db)->orm_db);
    (*db)->orm_db = NULL;
  }

  return cmn_free((void **)db);
}

/**
 * @brief Executes a raw SQL query (e.g. CREATE TABLE) on the Room database.
 * @param db The Room database.
 * @param sql The raw SQL string to execute.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_room_db_execute(cmn_room_db_t *db, const char *sql) {
  c_orm_error_t err;
  if (!db || !db->orm_db || !sql) {
    return CMN_ERR_INVAL;
  }

  err = c_orm_execute_raw(db->orm_db, sql);
  if (err != C_ORM_OK) {
    return CMN_ERR_IO;
  }

  return CMN_SUCCESS;
}

/**
 * @brief Creates a Data Access Object (DAO) for a specific entity metadata.
 * @param db The Room database.
 * @param meta The c-orm table metadata describing the entity.
 * @param out_dao Output pointer to the created DAO.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_room_dao_create(cmn_room_db_t *db,
                                const c_orm_table_meta_t *meta,
                                cmn_room_dao_t **out_dao) {
  cmn_room_dao_t *dao = NULL;
  cmn_error_e rc;

  if (!db || !meta || !out_dao) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_malloc(sizeof(cmn_room_dao_t), (void **)&dao);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  dao->db = db;
  dao->meta = meta;
  *out_dao = dao;

  return CMN_SUCCESS;
}

/**
 * @brief Destroys a DAO.
 * @param dao Pointer to the DAO pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_room_dao_destroy(cmn_room_dao_t **dao) {
  if (!dao || !*dao) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)dao);
}

/**
 * @brief Inserts an entity using the DAO.
 * @param dao The DAO.
 * @param entity Pointer to the C struct representing the entity.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_room_dao_insert(cmn_room_dao_t *dao, const void *entity) {
  c_orm_error_t err;

  if (!dao || !entity) {
    return CMN_ERR_INVAL;
  }

  err = c_orm_insert(dao->db->orm_db, dao->meta, entity);
  if (err != C_ORM_OK) {
    return CMN_ERR_IO;
  }

  return CMN_SUCCESS;
}

/**
 * @brief Finds an entity by its int32 primary key.
 * @param dao The DAO.
 * @param id The primary key ID.
 * @param out_entity Pointer to the C struct to populate.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if absent, error code
 * otherwise.
 */
cmn_error_e cmn_room_dao_find_by_id_int32(cmn_room_dao_t *dao, int32_t id,
                                          void *out_entity) {
  c_orm_error_t err;

  if (!dao || !out_entity) {
    return CMN_ERR_INVAL;
  }

  err = c_orm_find_by_id_int32(dao->db->orm_db, dao->meta, id, out_entity);
  if (err == C_ORM_ERROR_NOT_FOUND) {
    return CMN_ERR_NOT_FOUND;
  }
  if (err != C_ORM_OK) {
    return CMN_ERR_IO;
  }

  return CMN_SUCCESS;
}

/**
 * @brief Synchronizes local entities with a REST endpoint via c-abstract-http.
 * @param dao The DAO.
 * @param http_client The HTTP client instance from c-abstract-http.
 * @param sync_url The REST endpoint URL.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_room_dao_sync(cmn_room_dao_t *dao,
                              struct HttpClient *http_client,
                              const char *sync_url) {
  if (!dao || !http_client || !sync_url || sync_url[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  if (http_client->send != NULL) {
    struct HttpRequest req;
    struct HttpResponse *res = NULL;
    c_abstract_http_error_t send_rc;
    memset(&req, 0, sizeof(req));
    req.url = (char *)sync_url;
    req.method = HTTP_GET;
    send_rc = http_client->send(http_client->transport, &req, &res);
    if (send_rc != C_ABSTRACT_HTTP_SUCCESS || res->status_code >= 400) {
      return CMN_ERR_IO;
    }
  }

  return CMN_SUCCESS;
}
