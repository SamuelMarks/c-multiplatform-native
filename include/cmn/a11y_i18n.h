#ifndef CMN_A11Y_I18N_H
#define CMN_A11Y_I18N_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
/* clang-format on */

/**
 * @file a11y_i18n.h
 * @brief Accessibility (a11y) and Internationalization (i18n) support.
 */

/**
 * @defgroup cmn_a11y_i18n Accessibility and Internationalization
 * @brief Accessibility and Internationalization module.
 * @{
 */

/* --- Internationalization (i18n) --- */

/**
 * @brief Initializes the i18n subsystem with a locale and translation catalog.
 * @param locale The locale identifier (e.g., "en-US", "ar-SA").
 * @param catalog_path JSON catalog string (e.g., `{"greeting":"Bonjour"}`) or
 * path to catalog.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_i18n_init(const char *locale,
                                         const char *catalog_path);

/**
 * @brief Registers or updates a translation key-value pair in the active
 * catalog.
 * @param key The translation key.
 * @param value The translated string value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_i18n_set(const char *key, const char *value);

/**
 * @brief Retrieves a translated string for a given key.
 * @param key The translation key.
 * @param out_str Pointer to store the translated string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_i18n_get(const char *key, const char **out_str);

/**
 * @brief Checks if a given locale uses Right-To-Left (RTL) text direction.
 * @param locale The locale identifier.
 * @param out_is_rtl Pointer to store 1 if RTL, 0 if LTR.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_i18n_is_rtl(const char *locale, int *out_is_rtl);

/* --- Accessibility (a11y) --- */

/**
 * @brief Accessibility roles for UI widgets.
 */
typedef enum cmn_a11y_role_e {
  /** @brief No specific accessibility role. */
  CMN_A11Y_ROLE_NONE = 0,
  /** @brief Button role. */
  CMN_A11Y_ROLE_BUTTON = 1,
  /** @brief Static label/text role. */
  CMN_A11Y_ROLE_LABEL = 2,
  /** @brief Main window role. */
  CMN_A11Y_ROLE_WINDOW = 3,
  /** @brief Modal or non-modal dialog role. */
  CMN_A11Y_ROLE_DIALOG = 4
} cmn_a11y_role_e;

/**
 * @brief Sets the accessibility role for a widget.
 * @param widget The widget to modify.
 * @param role The accessibility role to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_a11y_set_role(void *widget,
                                             cmn_a11y_role_e role);

/**
 * @brief Sets the accessibility label (screen reader description) for a widget.
 * @param widget The widget to modify.
 * @param label The accessibility label string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_a11y_set_label(void *widget, const char *label);

/**
 * @brief Sets the keyboard navigation tab index for a widget.
 * @param widget The widget to modify.
 * @param index The tab index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_a11y_set_tabindex(void *widget, int index);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_A11Y_I18N_H */
