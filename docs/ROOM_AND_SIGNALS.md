# Room and Signals Documentation

This framework provides modern app architecture primitives for C89, specifically bringing concepts from Android (Room) and web frameworks (Signals) to native C.

## 1. Room-like Consistency & ORM

The `cmn/room.h` module provides a high-level wrapper over `c-orm` to enable easy persistence.

### Database Creation

```c
#include <cmn/error.h>
#include <cmn/room.h>
#include <stddef.h>

static cmn_error_e init_database(cmn_room_db_t **out_db) {
  cmn_room_db_t *db = NULL;
  cmn_error_e err = CMN_SUCCESS;
  cmn_error_e cleanup_err = CMN_SUCCESS;

  if (out_db == NULL) {
    return CMN_ERR_INVAL;
  }

  err = cmn_room_db_create("app_data.db", &db);
  if (err != CMN_SUCCESS) {
    return err;
  }

  /* Run raw SQL (e.g. for creating tables) */
  err = cmn_room_db_execute(
      db, "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)");
  if (err != CMN_SUCCESS) {
    cleanup_err = cmn_room_db_destroy(&db);
    if (cleanup_err != CMN_SUCCESS) {
      return cleanup_err;
    }
    return err;
  }

  *out_db = db;
  return CMN_SUCCESS;
}
```

### DAO and Entity Mapping

Define a standard C struct and metadata:

```c
#include <cmn/error.h>
#include <cmn/room.h>
#include <cmn/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct user_s {
  int32_t id;
  char *name;
};

static cmn_error_e run_user_dao_example(cmn_room_db_t *db) {
  c_orm_table_meta_t meta;
  c_orm_column_meta_t cols[2];
  cmn_room_dao_t *dao = NULL;
  struct user_s new_user;
  struct user_s found_user;
  cmn_error_e err = CMN_SUCCESS;
  cmn_error_e cleanup_err = CMN_SUCCESS;

  memset(&meta, 0, sizeof(meta));
  memset(cols, 0, sizeof(cols));
  memset(&new_user, 0, sizeof(new_user));
  memset(&found_user, 0, sizeof(found_user));

  /* Define columns and table metadata (see c_orm_meta.h) */
  cols[0].name = "id";
  cols[0].type = C_ORM_TYPE_INT32;
  cols[0].offset = offsetof(struct user_s, id);
  cols[0].is_pk = 1;

  cols[1].name = "name";
  cols[1].type = C_ORM_TYPE_STRING;
  cols[1].offset = offsetof(struct user_s, name);

  meta.name = "users";
  meta.columns = cols;
  meta.num_columns = 2;
  meta.struct_size = sizeof(struct user_s);
  meta.query_insert = "INSERT INTO users (id, name) VALUES (?, ?)";
  meta.query_select_by_pk = "SELECT * FROM users WHERE id=?";

  err = cmn_room_dao_create(db, &meta, &dao);
  if (err != CMN_SUCCESS) {
    return err;
  }

  /* Insert */
  new_user.id = 1;
  new_user.name = "Alice";
  err = cmn_room_dao_insert(dao, &new_user);
  if (err != CMN_SUCCESS) {
    goto cleanup;
  }

  /* Query */
  err = cmn_room_dao_find_by_id_int32(dao, 1, &found_user);
  if (err != CMN_SUCCESS) {
    goto cleanup;
  }

  if (found_user.name != NULL) {
    free(found_user.name);
    found_user.name = NULL;
  }

cleanup:
  if (dao != NULL) {
    cleanup_err = cmn_room_dao_destroy(&dao);
    if (err == CMN_SUCCESS && cleanup_err != CMN_SUCCESS) {
      err = cleanup_err;
    }
  }
  return err;
}
```

### Sync with REST API

You can sync your local DAO state with a remote server using the built-in HTTP sync via `c-abstract-http`:

```c
#include <cmn/error.h>
#include <cmn/room.h>

static cmn_error_e sync_users(cmn_room_dao_t *dao,
                              struct HttpClient *http_client) {
  cmn_error_e err;

  err = cmn_room_dao_sync(dao, http_client,
                          "https://api.example.com/sync/users");
  if (err != CMN_SUCCESS) {
    return err;
  }
  return CMN_SUCCESS;
}
```

## 2. Signals API

The `cmn/signals.h` module brings Angular-inspired reactive state to C.

```c
#include <cmn/error.h>
#include <cmn/signals.h>
#include <stddef.h>

static cmn_error_e my_compute_fn(void *userdata, int *out_val) {
  cmn_signal_t *counter = (cmn_signal_t *)userdata;
  int val = 0;
  cmn_error_e err;

  if (out_val == NULL) {
    return CMN_ERR_INVAL;
  }
  err = cmn_signal_int_get(counter, &val);
  if (err != CMN_SUCCESS) {
    return err;
  }
  *out_val = val * 2;
  return CMN_SUCCESS;
}

static cmn_error_e my_effect_fn(void *userdata) {
  cmn_computed_t *doubled = (cmn_computed_t *)userdata;
  int val = 0;
  cmn_error_e err;

  err = cmn_computed_int_get(doubled, &val);
  if (err != CMN_SUCCESS) {
    return err;
  }
  return CMN_SUCCESS;
}

static cmn_error_e run_signals_example(void) {
  cmn_signal_t *counter = NULL;
  cmn_computed_t *doubled = NULL;
  cmn_effect_t *eff = NULL;
  int val = 0;
  cmn_error_e err = CMN_SUCCESS;
  cmn_error_e cleanup_err = CMN_SUCCESS;

  err = cmn_signal_int_create(0, &counter);
  if (err != CMN_SUCCESS) {
    return err;
  }

  /* Read */
  err = cmn_signal_int_get(counter, &val);
  if (err != CMN_SUCCESS) {
    goto cleanup;
  }

  /* Write */
  err = cmn_signal_int_set(counter, val + 1);
  if (err != CMN_SUCCESS) {
    goto cleanup;
  }

  /* Computed / Derived State */
  err = cmn_computed_int_create(my_compute_fn, counter, &doubled);
  if (err != CMN_SUCCESS) {
    goto cleanup;
  }

  /* Side-Effects (Automatic reactivity) */
  err = cmn_effect_create(my_effect_fn, doubled, &eff);
  if (err != CMN_SUCCESS) {
    goto cleanup;
  }

cleanup:
  if (eff != NULL) {
    cleanup_err = cmn_effect_destroy(&eff);
    if (err == CMN_SUCCESS && cleanup_err != CMN_SUCCESS) {
      err = cleanup_err;
    }
  }
  if (doubled != NULL) {
    cleanup_err = cmn_computed_destroy(&doubled);
    if (err == CMN_SUCCESS && cleanup_err != CMN_SUCCESS) {
      err = cleanup_err;
    }
  }
  if (counter != NULL) {
    cleanup_err = cmn_signal_destroy(&counter);
    if (err == CMN_SUCCESS && cleanup_err != CMN_SUCCESS) {
      err = cleanup_err;
    }
  }
  return err;
}
```

Both APIs emphasize C89 strictness, no memory leaks, and pointer-based output. Every function returns a `cmn_error_e`.

## 3. Native UI Integration Screenshots

### Reactive Signal & Room DAO Event Log (`cmn_textbox_t` & `cmn_textview_t`)

| macOS (Cocoa) | Windows 11 (Win32) |
| :---: | :---: |
| [![Inputs on macOS](https://github.com/SamuelMarks/cc0-assets/raw/master/c-multiplatform-native/screenshots/inputs_macos.png)](https://github.com/SamuelMarks/cc0-assets/blob/master/c-multiplatform-native/screenshots/inputs_macos.png) | [![Inputs on Windows](https://github.com/SamuelMarks/cc0-assets/raw/master/c-multiplatform-native/screenshots/inputs_windows.png)](https://github.com/SamuelMarks/cc0-assets/blob/master/c-multiplatform-native/screenshots/inputs_windows.png) |
| **Linux (GTK3)** | **Web (WASM)** |
| [![Inputs on Linux](https://github.com/SamuelMarks/cc0-assets/raw/master/c-multiplatform-native/screenshots/inputs_linux.png)](https://github.com/SamuelMarks/cc0-assets/blob/master/c-multiplatform-native/screenshots/inputs_linux.png) | [![Inputs on Web](https://github.com/SamuelMarks/cc0-assets/raw/master/c-multiplatform-native/screenshots/inputs_web.png)](https://github.com/SamuelMarks/cc0-assets/blob/master/c-multiplatform-native/screenshots/inputs_web.png) |

### Reactive Form Validation & SPA Routing (`examples/simple_password_flow`)

| macOS (Cocoa) | Windows 11 (Win32) |
| :---: | :---: |
| [![Password Flow on macOS](https://github.com/SamuelMarks/cc0-assets/raw/master/c-multiplatform-native/screenshots/password_flow_macos.png)](https://github.com/SamuelMarks/cc0-assets/blob/master/c-multiplatform-native/screenshots/password_flow_macos.png) | [![Password Flow on Windows](https://github.com/SamuelMarks/cc0-assets/raw/master/c-multiplatform-native/screenshots/password_flow_windows.png)](https://github.com/SamuelMarks/cc0-assets/blob/master/c-multiplatform-native/screenshots/password_flow_windows.png) |
| **Linux (GTK3)** | **Web (WASM)** |
| [![Password Flow on Linux](https://github.com/SamuelMarks/cc0-assets/raw/master/c-multiplatform-native/screenshots/password_flow_linux.png)](https://github.com/SamuelMarks/cc0-assets/blob/master/c-multiplatform-native/screenshots/password_flow_linux.png) | [![Password Flow on Web](https://github.com/SamuelMarks/cc0-assets/raw/master/c-multiplatform-native/screenshots/password_flow_web.png)](https://github.com/SamuelMarks/cc0-assets/blob/master/c-multiplatform-native/screenshots/password_flow_web.png) |
