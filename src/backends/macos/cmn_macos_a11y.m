/* clang-format off */
#include "cmn_macos_internal.h"
#include "cmn/a11y_i18n.h"
/* clang-format on */

/**
 * @file cmn_macos_a11y.m
 * @brief macOS AppKit implementation for accessibility operations.
 */

/**
 * @brief Resolves a polymorphic cmn widget handle to its underlying NSObject
 * supporting NSAccessibility.
 * @param widget The cmn widget handle.
 * @param out_obj Output pointer receiving the Objective-C accessibility object.
 * @param out_tabindex_ptr Output pointer receiving the address of the tabindex
 * field (can be NULL for windows).
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if widget is invalid.
 */
static cmn_error_e resolve_a11y_object(void *widget, id *out_obj,
                                       int **out_tabindex_ptr) {
  cmn_widget_kind_e kind;
  cmn_error_e rc;

  rc = cmn_widget_get_kind(widget, &kind);
  if (rc != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }

  *out_tabindex_ptr = NULL;
  switch (kind) {
  case CMN_WIDGET_KIND_WINDOW:
    *out_obj = ((cmn_window_t *)widget)->ns_window;
    break;
  case CMN_WIDGET_KIND_BUTTON:
    *out_obj = ((cmn_button_t *)widget)->ns_button;
    *out_tabindex_ptr = &((cmn_button_t *)widget)->tabindex;
    break;
  case CMN_WIDGET_KIND_LABEL:
    *out_obj = ((cmn_label_t *)widget)->ns_label;
    *out_tabindex_ptr = &((cmn_label_t *)widget)->tabindex;
    break;
  case CMN_WIDGET_KIND_TEXTBOX:
    *out_obj = ((cmn_textbox_t *)widget)->ns_textbox;
    *out_tabindex_ptr = &((cmn_textbox_t *)widget)->tabindex;
    break;
  case CMN_WIDGET_KIND_TEXTVIEW:
    *out_obj = ((cmn_textview_t *)widget)->ns_textview;
    *out_tabindex_ptr = &((cmn_textview_t *)widget)->tabindex;
    break;
  case CMN_WIDGET_KIND_VBOX:
    *out_obj = ((cmn_vbox_t *)widget)->ns_stack;
    *out_tabindex_ptr = &((cmn_vbox_t *)widget)->tabindex;
    break;
  case CMN_WIDGET_KIND_HBOX:
    *out_obj = ((cmn_hbox_t *)widget)->ns_stack;
    *out_tabindex_ptr = &((cmn_hbox_t *)widget)->tabindex;
    break;
  default:
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
  id obj = nil;
  int *tab_ptr = NULL;
  cmn_error_e rc;

  if (widget == NULL || label == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = resolve_a11y_object(widget, &obj, &tab_ptr);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  @autoreleasepool {
    NSString *ns_label = [NSString stringWithUTF8String:label];
    [obj setAccessibilityLabel:ns_label];
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the accessibility role for a widget.
 * @param widget The widget to modify.
 * @param role The accessibility role to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_role(void *widget, cmn_a11y_role_e role) {
  id obj = nil;
  int *tab_ptr = NULL;
  cmn_error_e rc;

  if (widget == NULL || role == CMN_A11Y_ROLE_NONE) {
    return CMN_ERR_INVAL;
  }

  rc = resolve_a11y_object(widget, &obj, &tab_ptr);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  @autoreleasepool {
    switch (role) {
    case CMN_A11Y_ROLE_BUTTON:
      [obj setAccessibilityRole:NSAccessibilityButtonRole];
      break;
    case CMN_A11Y_ROLE_LABEL:
      [obj setAccessibilityRole:NSAccessibilityStaticTextRole];
      break;
    case CMN_A11Y_ROLE_WINDOW:
      [obj setAccessibilityRole:NSAccessibilityWindowRole];
      break;
    case CMN_A11Y_ROLE_DIALOG:
      [obj setAccessibilityRole:NSAccessibilityWindowRole];
      [obj setAccessibilitySubrole:NSAccessibilityDialogSubrole];
      break;
    default:
      return CMN_ERR_INVAL;
    }
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
  id obj = nil;
  int *tab_ptr = NULL;
  cmn_error_e rc;

  if (widget == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = resolve_a11y_object(widget, &obj, &tab_ptr);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  if (tab_ptr != NULL) {
    *tab_ptr = index;
  }
  return CMN_SUCCESS;
}
