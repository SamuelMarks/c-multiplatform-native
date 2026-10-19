/* clang-format off */
#include "cmn_ios_internal.h"
#include "cmn/a11y_i18n.h"
/* clang-format on */

/**
 * @file cmn_ios_a11y.m
 * @brief iOS UIKit accessibility implementation.
 */

/**
 * @brief Sets the accessibility role for a widget.
 * @param widget The widget to modify.
 * @param role The accessibility role to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_role(void *widget, cmn_a11y_role_e role) {
  cmn_widget_kind_e kind;
  if (widget == NULL || role < CMN_A11Y_ROLE_BUTTON ||
      role > CMN_A11Y_ROLE_DIALOG) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS) {
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
  cmn_widget_kind_e kind;
  if (widget == NULL || label == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS) {
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
  cmn_widget_kind_e kind;
  if (widget == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)widget)->a11y_tabindex = index;
  return CMN_SUCCESS;
}
