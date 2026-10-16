# Room and Signals Documentation

This framework provides modern app architecture primitives for C89, specifically bringing concepts from Android (Room) and web frameworks (Signals) to native C.

## 1. Room-like Consistency & ORM

The `cmn/room.h` module provides a high-level wrapper over `c-orm` to enable easy persistence.

### Database Creation

```c
cmn_room_db_t *db = NULL;
cmn_error_e err = cmn_room_db_create("app_data.db", &db);
if (err != CMN_SUCCESS) {
  /* Handle error */
}

/* Run raw SQL (e.g. for creating tables) */
err = cmn_room_db_execute(db, "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)");
```

### DAO and Entity Mapping

Define a standard C struct and metadata:

```c
struct user_s {
  int32_t id;
  char *name;
};

/* Define columns and table metadata (see c_orm_meta.h) */
c_orm_table_meta_t meta;
/* ... initialize meta ... */

cmn_room_dao_t *dao = NULL;
err = cmn_room_dao_create(db, &meta, &dao);

/* Insert */
struct user_s new_user;
new_user.id = 1;
new_user.name = "Alice";
err = cmn_room_dao_insert(dao, &new_user);

/* Query */
struct user_s found_user;
err = cmn_room_dao_find_by_id_int32(dao, 1, &found_user);
```

### Sync with REST API

You can sync your local DAO state with a remote server using the built-in HTTP sync via `c-abstract-http`:

```c
void *http_client; /* Obtain an http client */
cmn_room_dao_sync(dao, http_client, "https://api.example.com/sync/users");
```

## 2. Signals API

The `cmn/signals.h` module brings Angular-inspired reactive state to C.

```c
cmn_signal_t *counter = NULL;
cmn_signal_int_create(0, &counter);

/* Read */
int val;
cmn_signal_int_get(counter, &val);

/* Write */
cmn_signal_int_set(counter, val + 1);

/* Computed / Derived State */
cmn_computed_t *doubled = NULL;
cmn_computed_int_create(my_compute_fn, counter, &doubled);

/* Side-Effects (Automatic reactivity) */
cmn_effect_t *eff = NULL;
cmn_effect_create(my_effect_fn, doubled, &eff);
```

Both APIs emphasize C89 strictness, no memory leaks, and pointer-based output. Every function returns a `cmn_error_e`.
