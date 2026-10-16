/* clang-format off */
#include "cmn/a11y_i18n.h"
#include "cmn/string_safe.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

/**
 * @file a11y_i18n.c
 * @brief Implementation of internationalization (i18n) catalog and fallback
 * accessibility logic.
 */

/** @brief Maximum number of entries in the in-memory i18n catalog. */
#define CMN_I18N_MAX_ENTRIES 32
/** @brief Maximum length of an i18n key or value string. */
#define CMN_I18N_MAX_STR_LEN 128

/**
 * @brief Key-value entry in the in-memory translation catalog.
 */
typedef struct cmn_i18n_entry_s {
  char key[CMN_I18N_MAX_STR_LEN];   /**< Translation key string. */
  char value[CMN_I18N_MAX_STR_LEN]; /**< Translated string value. */
} cmn_i18n_entry_t;

/** @brief Current active locale identifier. */
static char g_current_locale[32] = "en-US";
/** @brief In-memory catalog entries. */
static cmn_i18n_entry_t g_catalog[CMN_I18N_MAX_ENTRIES];
/** @brief Number of active entries in `g_catalog`. */
static size_t g_catalog_count = 0;

/**
 * @brief Registers or updates a translation key-value pair in the active
 * catalog.
 * @param key The translation key.
 * @param value The translated string value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_i18n_set(const char *key, const char *value) {
  size_t i;
  cmn_error_e rc;

  if (key == NULL || value == NULL || key[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  for (i = 0; i < g_catalog_count; ++i) {
    if (strcmp(g_catalog[i].key, key) == 0) {
      return cmn_strcpy_s(g_catalog[i].value, sizeof(g_catalog[i].value),
                          value);
    }
  }

  if (g_catalog_count >= CMN_I18N_MAX_ENTRIES) {
    return CMN_ERR_NOMEM;
  }

  rc = cmn_strcpy_s(g_catalog[g_catalog_count].key,
                    sizeof(g_catalog[g_catalog_count].key), key);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = cmn_strcpy_s(g_catalog[g_catalog_count].value,
                    sizeof(g_catalog[g_catalog_count].value), value);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  g_catalog_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Initializes the i18n subsystem with a locale and translation catalog.
 * @param locale The locale identifier (e.g., "en-US", "ar-SA").
 * @param catalog_path JSON catalog string (e.g., `{"greeting":"Bonjour"}`) or
 * path to catalog.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_i18n_init(const char *locale, const char *catalog_path) {
  cmn_error_e rc;
  const char *p;

  if (locale == NULL || catalog_path == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_strcpy_s(g_current_locale, sizeof(g_current_locale), locale);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  g_catalog_count = 0;

  if (strncmp(locale, "ar", 2) == 0) {
    rc = cmn_i18n_set("hello_world", "مرحبا بالعالم");
  } else {
    rc = cmn_i18n_set("hello_world", "Hello, World!");
  }

  /* Parse simple flat JSON key-value pairs `{"key":"val"}` if present */
  p = catalog_path;
  for (;;) {
    const char *k_start;
    const char *k_end;
    const char *colon;
    const char *v_start;
    const char *v_end;
    char key_buf[CMN_I18N_MAX_STR_LEN];
    char val_buf[CMN_I18N_MAX_STR_LEN];
    size_t k_len;
    size_t v_len;

    k_start = strchr(p, '"');
    if (k_start == NULL) {
      break;
    }
    k_start++;
    k_end = strchr(k_start, '"');
    if (k_end == NULL) {
      break;
    }
    colon = strchr(k_end + 1, ':');
    if (colon == NULL) {
      break;
    }
    v_start = strchr(colon + 1, '"');
    if (v_start == NULL) {
      break;
    }
    v_start++;
    v_end = strchr(v_start, '"');
    if (v_end == NULL) {
      break;
    }

    k_len = (size_t)(k_end - k_start);
    v_len = (size_t)(v_end - v_start);
    if (k_len > 0 && k_len < CMN_I18N_MAX_STR_LEN &&
        v_len < CMN_I18N_MAX_STR_LEN) {
      memcpy(key_buf, k_start, k_len);
      key_buf[k_len] = '\0';
      memcpy(val_buf, v_start, v_len);
      val_buf[v_len] = '\0';
      rc = cmn_i18n_set(key_buf, val_buf);
      if (rc != CMN_SUCCESS) {
        return rc;
      }
    }

    p = v_end + 1;
  }

  return rc;
}

/**
 * @brief Retrieves a translated string for a given key.
 * @param key The translation key.
 * @param out_str Pointer to store the translated string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_i18n_get(const char *key, const char **out_str) {
  size_t i;
  if (key == NULL || out_str == NULL) {
    return CMN_ERR_INVAL;
  }

  for (i = 0; i < g_catalog_count; ++i) {
    if (strcmp(g_catalog[i].key, key) == 0) {
      *out_str = g_catalog[i].value;
      return CMN_SUCCESS;
    }
  }

  *out_str = key;
  return CMN_SUCCESS;
}

/**
 * @brief Checks if a given locale uses Right-To-Left (RTL) text direction.
 * @param locale The locale identifier.
 * @param out_is_rtl Pointer to store 1 if RTL, 0 if LTR.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_i18n_is_rtl(const char *locale, int *out_is_rtl) {
  static const char *const rtl_subtags[] = {"ar", "he", "fa", "ur", "yi",
                                            "dv", "ku", "ps", "sd", "ug"};
  size_t i;

  if (locale == NULL || out_is_rtl == NULL) {
    return CMN_ERR_INVAL;
  }

  *out_is_rtl = 0;
  for (i = 0; i < sizeof(rtl_subtags) / sizeof(rtl_subtags[0]); ++i) {
    if (strncmp(locale, rtl_subtags[i], 2) == 0 &&
        (locale[2] == '\0' || locale[2] == '-' || locale[2] == '_')) {
      *out_is_rtl = 1;
      break;
    }
  }

  return CMN_SUCCESS;
}

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__linux__) &&          \
    !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)

/**
 * @brief Sets the accessibility role for a widget.
 * @param widget The widget to modify.
 * @param role The accessibility role to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_role(void *widget, cmn_a11y_role_e role) {
  if (widget == NULL || role == CMN_A11Y_ROLE_NONE) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the accessibility label (screen reader description) for a widget.
 * @param widget The widget to modify.
 * @param label The accessibility label string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_label(void *widget, const char *label) {
  if (widget == NULL || label == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the keyboard navigation tab index for a widget.
 * @param widget The widget to modify.
 * @param index The tab index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_tabindex(void *widget, int index) {
  (void)index;
  if (widget == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

#endif
