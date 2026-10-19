/* clang-format off */
#include "cmn_internal.h"
#include "cmn/widget.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file widget.c
 * @brief Shared polymorphic widget tagged-header, type-safe upcasting,
 * extended widget implementations, and generic destructor dispatch.
 */

/**
 * @brief Initializes a `cmn_widget_header_t` for a newly allocated object.
 * @param header Pointer to the header to initialize.
 * @param kind Concrete widget kind.
 * @param destroy_fn Destructor callback for this widget kind.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if header is NULL or kind is
 * invalid.
 */
cmn_error_e cmn_widget_header_init(cmn_widget_header_t *header,
                                   cmn_widget_kind_e kind,
                                   cmn_widget_destroy_fn destroy_fn) {
  if (header == NULL || kind == CMN_WIDGET_KIND_NONE || destroy_fn == NULL) {
    return CMN_ERR_INVAL;
  }
  header->magic = CMN_WIDGET_MAGIC;
  header->kind = kind;
  header->destroy_fn = destroy_fn;
  header->bounds.x = 0;
  header->bounds.y = 0;
  header->layout_root = NULL;
  header->on_resize = NULL;
  header->resize_userdata = NULL;
  header->native_handle = NULL;
  header->dom_id = 0;
  header->enabled = CMN_TRUE;
  header->visible = CMN_TRUE;
  header->padding.top = 0;
  header->padding.right = 0;
  header->padding.bottom = 0;
  header->padding.left = 0;
  header->default_button = NULL;
  header->cursor = CMN_CURSOR_ARROW;
  header->on_close = NULL;
  header->close_userdata = NULL;
  header->on_moved = NULL;
  header->moved_userdata = NULL;
  header->on_activate = NULL;
  header->activate_userdata = NULL;
  header->menu_ext_count = 0;
  header->fp_mode = CMN_FILEPICKER_OPEN_FILE;
  header->fp_filetype_count = 0;
  header->tooltip[0] = '\0';
  header->icon_path[0] = '\0';
  header->multiline = CMN_FALSE;
  header->text_align = CMN_TEXT_ALIGN_LEFT;
  header->fg_rgba = 0x000000FFu;
  header->bg_rgba = 0xFFFFFFFFu;
  header->editable = CMN_TRUE;
  header->autoselect = CMN_FALSE;
  header->sel_start = 0;
  header->sel_end = 0;
  header->font_family[0] = '\0';
  header->font_size = 12.0;
  header->font_style_mask = CMN_FONT_STYLE_NORMAL;
  header->text_wrap = CMN_TRUE;
  switch (kind) {
  case CMN_WIDGET_KIND_WINDOW:
  case CMN_WIDGET_KIND_VBOX:
  case CMN_WIDGET_KIND_HBOX:
  case CMN_WIDGET_KIND_FLEX:
  case CMN_WIDGET_KIND_GRID:
  case CMN_WIDGET_KIND_PANEL:
  case CMN_WIDGET_KIND_TABS:
  case CMN_WIDGET_KIND_SPLITVIEW:
    header->bounds.width = 480;
    header->bounds.height = 360;
    break;
  case CMN_WIDGET_KIND_TEXTVIEW:
  case CMN_WIDGET_KIND_LISTBOX:
  case CMN_WIDGET_KIND_TABLEVIEW:
  case CMN_WIDGET_KIND_VIEW:
  case CMN_WIDGET_KIND_WEBVIEW:
  case CMN_WIDGET_KIND_IMAGEVIEW:
    header->bounds.width = 360;
    header->bounds.height = 140;
    break;
  case CMN_WIDGET_KIND_TEXTBOX:
  case CMN_WIDGET_KIND_SLIDER:
  case CMN_WIDGET_KIND_PROGRESS:
  case CMN_WIDGET_KIND_POPUP:
  case CMN_WIDGET_KIND_COMBO:
    header->bounds.width = 240;
    header->bounds.height = 32;
    break;
  default:
    header->bounds.width = 120;
    header->bounds.height = 32;
    break;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Validates a raw pointer as an initialized `cmn_widget_header_t` and
 * retrieves its kind.
 * @param ptr Pointer to inspect.
 * @param out_kind Output pointer receiving the widget kind.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if ptr is NULL or
 * uninitialized.
 */
cmn_error_e cmn_widget_get_kind(const void *ptr, cmn_widget_kind_e *out_kind) {
  const cmn_widget_header_t *hdr;
  if (ptr == NULL || out_kind == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (const cmn_widget_header_t *)ptr;
  if (hdr->magic != CMN_WIDGET_MAGIC || hdr->kind == CMN_WIDGET_KIND_NONE) {
    return CMN_ERR_INVAL;
  }
  *out_kind = hdr->kind;
  return CMN_SUCCESS;
}

/**
 * @brief Internal helper validating that `ptr` has `expected_kind`.
 * @param ptr Pointer to inspect.
 * @param expected_kind Expected `cmn_widget_kind_e`.
 * @return CMN_SUCCESS on match, CMN_ERR_INVAL otherwise.
 */
static cmn_error_e validate_kind(const void *ptr,
                                 cmn_widget_kind_e expected_kind) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_error_e rc;
  rc = cmn_widget_get_kind(ptr, &kind);
  if (rc != CMN_SUCCESS || kind != expected_kind) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Internal helper validating that `ptr` has `expected_kind` and storing
 * `ptr` into `*out_target`.
 * @param ptr The source widget or container handle.
 * @param expected_kind The expected `cmn_widget_kind_e` discriminant.
 * @param out_target Output pointer receiving `ptr`.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL otherwise.
 */
static cmn_error_e upcast_checked(void *ptr, cmn_widget_kind_e expected_kind,
                                  void **out_target) {
  if (out_target == NULL || validate_kind(ptr, expected_kind) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  *out_target = ptr;
  return CMN_SUCCESS;
}

/**
 * @brief Allocates, initializes header, and initializes the backend native
 * handle for a widget.
 * @param size Size in bytes of the concrete widget struct.
 * @param kind Concrete `cmn_widget_kind_e` discriminant.
 * @param destroy_fn Polymorphic destructor adapter.
 * @param out_ptr Output pointer receiving the allocated widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e alloc_and_init_widget(size_t size, cmn_widget_kind_e kind,
                                         cmn_widget_destroy_fn destroy_fn,
                                         void **out_ptr) {
  cmn_error_e rc;
  cmn_error_e init_rc;
  cmn_widget_header_t *hdr = NULL;
  if (out_ptr == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, size, (void **)&hdr);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = cmn_widget_header_init(hdr, kind, destroy_fn);
  init_rc = cmn_backend_widget_init(hdr);
  *out_ptr = hdr;
  return (rc != CMN_SUCCESS) ? rc : init_rc;
}

/**
 * @brief Cleans up the backend native handle and frees an extended widget.
 * @param ptr Address of the widget pointer to destroy.
 * @param expected_kind Expected `cmn_widget_kind_e` discriminant.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_extended_widget(void **ptr,
                                           cmn_widget_kind_e expected_kind) {
  cmn_error_e rc;
  cmn_error_e free_rc;
  if (ptr == NULL || validate_kind(*ptr, expected_kind) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_backend_widget_cleanup((cmn_widget_header_t *)*ptr);
  free_rc = cmn_free(ptr);
  return (rc != CMN_SUCCESS) ? rc : free_rc;
}

/**
 * @brief Upcasts a `cmn_window_t` to a `cmn_container_t`.
 * @param window The window instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_window_as_container(cmn_window_t *window,
                                    cmn_container_t **out_container) {
  return upcast_checked(window, CMN_WIDGET_KIND_WINDOW, (void **)out_container);
}

/**
 * @brief Upcasts a `cmn_vbox_t` to a `cmn_container_t`.
 * @param vbox The vertical box instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_vbox_as_container(cmn_vbox_t *vbox,
                                  cmn_container_t **out_container) {
  return upcast_checked(vbox, CMN_WIDGET_KIND_VBOX, (void **)out_container);
}

/**
 * @brief Upcasts a `cmn_hbox_t` to a `cmn_container_t`.
 * @param hbox The horizontal box instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_hbox_as_container(cmn_hbox_t *hbox,
                                  cmn_container_t **out_container) {
  return upcast_checked(hbox, CMN_WIDGET_KIND_HBOX, (void **)out_container);
}

/**
 * @brief Upcasts a `cmn_vbox_t` to a `cmn_widget_t`.
 * @param vbox The vertical box instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_vbox_as_widget(cmn_vbox_t *vbox, cmn_widget_t **out_widget) {
  return upcast_checked(vbox, CMN_WIDGET_KIND_VBOX, (void **)out_widget);
}

/**
 * @brief Upcasts a `cmn_hbox_t` to a `cmn_widget_t`.
 * @param hbox The horizontal box instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_hbox_as_widget(cmn_hbox_t *hbox, cmn_widget_t **out_widget) {
  return upcast_checked(hbox, CMN_WIDGET_KIND_HBOX, (void **)out_widget);
}

/**
 * @brief Upcasts a `cmn_button_t` to a `cmn_widget_t`.
 * @param button The button instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_button_as_widget(cmn_button_t *button,
                                 cmn_widget_t **out_widget) {
  return upcast_checked(button, CMN_WIDGET_KIND_BUTTON, (void **)out_widget);
}

/**
 * @brief Upcasts a `cmn_label_t` to a `cmn_widget_t`.
 * @param label The label instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_label_as_widget(cmn_label_t *label, cmn_widget_t **out_widget) {
  return upcast_checked(label, CMN_WIDGET_KIND_LABEL, (void **)out_widget);
}

/**
 * @brief Upcasts a `cmn_textbox_t` to a `cmn_widget_t`.
 * @param textbox The textbox instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_textbox_as_widget(cmn_textbox_t *textbox,
                                  cmn_widget_t **out_widget) {
  return upcast_checked(textbox, CMN_WIDGET_KIND_TEXTBOX, (void **)out_widget);
}

/**
 * @brief Upcasts a `cmn_textview_t` to a `cmn_widget_t`.
 * @param textview The textview instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_textview_as_widget(cmn_textview_t *textview,
                                   cmn_widget_t **out_widget) {
  return upcast_checked(textview, CMN_WIDGET_KIND_TEXTVIEW,
                        (void **)out_widget);
}

/**
 * @brief Upcasts a `cmn_flex_t` to a `cmn_container_t`.
 * @param flex The flex container instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_flex_as_container(cmn_flex_t *flex,
                                  cmn_container_t **out_container) {
  return upcast_checked(flex, CMN_WIDGET_KIND_FLEX, (void **)out_container);
}

/**
 * @brief Upcasts a `cmn_flex_t` to a `cmn_widget_t`.
 * @param flex The flex container instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_flex_as_widget(cmn_flex_t *flex, cmn_widget_t **out_widget) {
  return upcast_checked(flex, CMN_WIDGET_KIND_FLEX, (void **)out_widget);
}

/**
 * @brief Upcasts a `cmn_grid_t` to a `cmn_container_t`.
 * @param grid The grid container instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_grid_as_container(cmn_grid_t *grid,
                                  cmn_container_t **out_container) {
  return upcast_checked(grid, CMN_WIDGET_KIND_GRID, (void **)out_container);
}

/**
 * @brief Upcasts a `cmn_grid_t` to a `cmn_widget_t`.
 * @param grid The grid container instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
cmn_error_e cmn_grid_as_widget(cmn_grid_t *grid, cmn_widget_t **out_widget) {
  return upcast_checked(grid, CMN_WIDGET_KIND_GRID, (void **)out_widget);
}

/**
 * @brief Sets the explicit or intrinsic bounding rectangle of any widget.
 * @param widget The widget to update.
 * @param bounds The bounding rectangle (`width >= 0`, `height >= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_set_bounds(void *widget, cmn_rect_t bounds) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_error_e rc;
  if (bounds.width < 0 || bounds.height < 0) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_widget_get_kind(widget, &kind);
  if (rc != CMN_SUCCESS || kind == CMN_WIDGET_KIND_MENU ||
      kind == CMN_WIDGET_KIND_FILEPICKER ||
      kind == CMN_WIDGET_KIND_COLORPICKER) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)widget)->bounds = bounds;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the current computed or configured bounding rectangle of any
 * widget.
 * @param widget The widget to query.
 * @param out_bounds Output pointer receiving the `cmn_rect_t`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_get_bounds(const void *widget, cmn_rect_t *out_bounds) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_error_e rc;
  if (out_bounds == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_widget_get_kind(widget, &kind);
  if (rc != CMN_SUCCESS || kind == CMN_WIDGET_KIND_MENU ||
      kind == CMN_WIDGET_KIND_FILEPICKER ||
      kind == CMN_WIDGET_KIND_COLORPICKER) {
    return CMN_ERR_INVAL;
  }
  *out_bounds = ((const cmn_widget_header_t *)widget)->bounds;
  return CMN_SUCCESS;
}

/**
 * @brief Polymorphically destroys any widget handle created by cmn.
 * @param widget Pointer to the widget pointer to destroy and set to NULL.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_destroy(cmn_widget_t **widget) {
  cmn_widget_header_t *hdr;
  if (widget == NULL || *widget == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)*widget;
  if (hdr->magic != CMN_WIDGET_MAGIC || hdr->destroy_fn == NULL) {
    return CMN_ERR_INVAL;
  }
  return hdr->destroy_fn((void **)widget);
}

/* --- CheckBox --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_checkbox_t`.
 * @param ptr Address of the checkbox pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_checkbox_adapter(void **ptr) {
  return cmn_checkbox_destroy((cmn_checkbox_t **)ptr);
}

/**
 * @brief Creates a new 2-state checkbox widget.
 * @param out_checkbox Pointer to store the created checkbox.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_create(cmn_checkbox_t **out_checkbox) {
  return alloc_and_init_widget(sizeof(cmn_checkbox_t), CMN_WIDGET_KIND_CHECKBOX,
                               destroy_checkbox_adapter, (void **)out_checkbox);
}

/**
 * @brief Creates a new 3-state checkbox widget.
 * @param out_checkbox Pointer to store the created tri-state checkbox.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_create_tristate(cmn_checkbox_t **out_checkbox) {
  cmn_error_e rc;
  rc = cmn_checkbox_create(out_checkbox);
  if (rc == CMN_SUCCESS) {
    (*out_checkbox)->is_tristate = CMN_TRUE;
    rc = cmn_backend_widget_sync(&(*out_checkbox)->base);
  }
  return rc;
}

/**
 * @brief Sets the label text of a checkbox.
 * @param checkbox The checkbox to modify.
 * @param text The new UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_set_text(cmn_checkbox_t *checkbox, const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(checkbox, CMN_WIDGET_KIND_CHECKBOX) != CMN_SUCCESS ||
      text == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(checkbox->text, sizeof(checkbox->text), text);
  sync_rc = cmn_backend_widget_sync(&checkbox->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the label text of a checkbox.
 * @param checkbox The checkbox to query.
 * @param out_text Pointer to store the UTF-8 text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_get_text(const cmn_checkbox_t *checkbox,
                                  const char **out_text) {
  if (validate_kind(checkbox, CMN_WIDGET_KIND_CHECKBOX) != CMN_SUCCESS ||
      out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_text = checkbox->text;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the check state of a checkbox.
 * @param checkbox The checkbox to modify.
 * @param state Desired check state (`CMN_CHECK_OFF`, `CMN_CHECK_ON`, or
 * `CMN_CHECK_MIXED` if tri-state).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_set_state(cmn_checkbox_t *checkbox,
                                   cmn_check_state_e state) {
  if (validate_kind(checkbox, CMN_WIDGET_KIND_CHECKBOX) != CMN_SUCCESS ||
      state < CMN_CHECK_OFF || state > CMN_CHECK_MIXED ||
      (state == CMN_CHECK_MIXED && !checkbox->is_tristate)) {
    return CMN_ERR_INVAL;
  }
  checkbox->state = state;
  return cmn_backend_widget_sync(&checkbox->base);
}

/**
 * @brief Gets the current check state of a checkbox.
 * @param checkbox The checkbox to query.
 * @param out_state Pointer to store the current check state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_get_state(const cmn_checkbox_t *checkbox,
                                   cmn_check_state_e *out_state) {
  if (validate_kind(checkbox, CMN_WIDGET_KIND_CHECKBOX) != CMN_SUCCESS ||
      out_state == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_state = checkbox->state;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the toggle callback for a checkbox.
 * @param checkbox The checkbox to modify.
 * @param cb Callback function invoked when toggled.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_set_on_toggle(cmn_checkbox_t *checkbox,
                                       cmn_checkbox_toggle_fn cb,
                                       void *userdata) {
  if (validate_kind(checkbox, CMN_WIDGET_KIND_CHECKBOX) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  checkbox->on_toggle = cb;
  checkbox->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically toggles a checkbox and invokes its callback.
 * @param checkbox The checkbox to toggle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_trigger_toggle(cmn_checkbox_t *checkbox) {
  cmn_error_e rc;
  if (validate_kind(checkbox, CMN_WIDGET_KIND_CHECKBOX) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  checkbox->state =
      (checkbox->state == CMN_CHECK_ON) ? CMN_CHECK_OFF : CMN_CHECK_ON;
  rc = cmn_backend_widget_sync(&checkbox->base);
  if (checkbox->on_toggle != NULL) {
    rc = checkbox->on_toggle(checkbox->userdata);
  }
  return rc;
}

/**
 * @brief Upcasts a `cmn_checkbox_t` to a `cmn_widget_t`.
 * @param checkbox The checkbox instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_as_widget(cmn_checkbox_t *checkbox,
                                   cmn_widget_t **out_widget) {
  return upcast_checked(checkbox, CMN_WIDGET_KIND_CHECKBOX,
                        (void **)out_widget);
}

/**
 * @brief Destroys a checkbox and frees its memory.
 * @param checkbox Pointer to the checkbox pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_checkbox_destroy(cmn_checkbox_t **checkbox) {
  return destroy_extended_widget((void **)checkbox, CMN_WIDGET_KIND_CHECKBOX);
}

/* --- RadioButton --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_radiobutton_t`.
 * @param ptr Address of the radio button pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_radiobutton_adapter(void **ptr) {
  return cmn_radiobutton_destroy((cmn_radiobutton_t **)ptr);
}

/**
 * @brief Creates a new radio button widget.
 * @param out_radiobutton Pointer to store the created radio button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_create(cmn_radiobutton_t **out_radiobutton) {
  return alloc_and_init_widget(
      sizeof(cmn_radiobutton_t), CMN_WIDGET_KIND_RADIOBUTTON,
      destroy_radiobutton_adapter, (void **)out_radiobutton);
}

/**
 * @brief Sets the label text of a radio button.
 * @param radiobutton The radio button to modify.
 * @param text The new UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_set_text(cmn_radiobutton_t *radiobutton,
                                     const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(radiobutton, CMN_WIDGET_KIND_RADIOBUTTON) != CMN_SUCCESS ||
      text == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(radiobutton->text, sizeof(radiobutton->text), text);
  sync_rc = cmn_backend_widget_sync(&radiobutton->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the label text of a radio button.
 * @param radiobutton The radio button to query.
 * @param out_text Pointer to store the UTF-8 text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_get_text(const cmn_radiobutton_t *radiobutton,
                                     const char **out_text) {
  if (validate_kind(radiobutton, CMN_WIDGET_KIND_RADIOBUTTON) != CMN_SUCCESS ||
      out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_text = radiobutton->text;
  return CMN_SUCCESS;
}

/**
 * @brief Sets whether a radio button is selected.
 * @param radiobutton The radio button to modify.
 * @param selected `CMN_TRUE` if selected, `CMN_FALSE` otherwise.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_set_selected(cmn_radiobutton_t *radiobutton,
                                         cmn_bool_t selected) {
  if (validate_kind(radiobutton, CMN_WIDGET_KIND_RADIOBUTTON) != CMN_SUCCESS ||
      (selected != CMN_FALSE && selected != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  radiobutton->selected = selected;
  return cmn_backend_widget_sync(&radiobutton->base);
}

/**
 * @brief Gets whether a radio button is selected.
 * @param radiobutton The radio button to query.
 * @param out_selected Pointer to store the selection state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_get_selected(const cmn_radiobutton_t *radiobutton,
                                         cmn_bool_t *out_selected) {
  if (validate_kind(radiobutton, CMN_WIDGET_KIND_RADIOBUTTON) != CMN_SUCCESS ||
      out_selected == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_selected = radiobutton->selected;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the selection callback for a radio button.
 * @param radiobutton The radio button to modify.
 * @param cb Callback function invoked when selected.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_set_on_select(cmn_radiobutton_t *radiobutton,
                                          cmn_radiobutton_select_fn cb,
                                          void *userdata) {
  if (validate_kind(radiobutton, CMN_WIDGET_KIND_RADIOBUTTON) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  radiobutton->on_select = cb;
  radiobutton->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically selects a radio button and invokes its callback.
 * @param radiobutton The radio button to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_trigger_select(cmn_radiobutton_t *radiobutton) {
  cmn_error_e rc;
  if (validate_kind(radiobutton, CMN_WIDGET_KIND_RADIOBUTTON) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  radiobutton->selected = CMN_TRUE;
  rc = cmn_backend_widget_sync(&radiobutton->base);
  if (radiobutton->on_select != NULL) {
    rc = radiobutton->on_select(radiobutton->userdata);
  }
  return rc;
}

/**
 * @brief Upcasts a `cmn_radiobutton_t` to a `cmn_widget_t`.
 * @param radiobutton The radio button instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_as_widget(cmn_radiobutton_t *radiobutton,
                                      cmn_widget_t **out_widget) {
  return upcast_checked(radiobutton, CMN_WIDGET_KIND_RADIOBUTTON,
                        (void **)out_widget);
}

/**
 * @brief Destroys a radio button and frees its memory.
 * @param radiobutton Pointer to the radio button pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_radiobutton_destroy(cmn_radiobutton_t **radiobutton) {
  return destroy_extended_widget((void **)radiobutton,
                                 CMN_WIDGET_KIND_RADIOBUTTON);
}

/* --- ToggleButton --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_togglebutton_t`.
 * @param ptr Address of the toggle button pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_togglebutton_adapter(void **ptr) {
  return cmn_togglebutton_destroy((cmn_togglebutton_t **)ptr);
}

/**
 * @brief Creates a new flat/toggle button widget.
 * @param out_togglebutton Pointer to store the created toggle button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_create(cmn_togglebutton_t **out_togglebutton) {
  return alloc_and_init_widget(
      sizeof(cmn_togglebutton_t), CMN_WIDGET_KIND_TOGGLEBUTTON,
      destroy_togglebutton_adapter, (void **)out_togglebutton);
}

/**
 * @brief Sets the text of a toggle button.
 * @param togglebutton The toggle button to modify.
 * @param text The new UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_set_text(cmn_togglebutton_t *togglebutton,
                                      const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(togglebutton, CMN_WIDGET_KIND_TOGGLEBUTTON) !=
          CMN_SUCCESS ||
      text == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(togglebutton->text, sizeof(togglebutton->text), text);
  sync_rc = cmn_backend_widget_sync(&togglebutton->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the text of a toggle button.
 * @param togglebutton The toggle button to query.
 * @param out_text Pointer to store the UTF-8 text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_get_text(const cmn_togglebutton_t *togglebutton,
                                      const char **out_text) {
  if (validate_kind(togglebutton, CMN_WIDGET_KIND_TOGGLEBUTTON) !=
          CMN_SUCCESS ||
      out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_text = togglebutton->text;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the toggled state of a toggle button.
 * @param togglebutton The toggle button to modify.
 * @param toggled `CMN_TRUE` if pressed/active, `CMN_FALSE` otherwise.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_set_toggled(cmn_togglebutton_t *togglebutton,
                                         cmn_bool_t toggled) {
  if (validate_kind(togglebutton, CMN_WIDGET_KIND_TOGGLEBUTTON) !=
          CMN_SUCCESS ||
      (toggled != CMN_FALSE && toggled != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  togglebutton->toggled = toggled;
  return cmn_backend_widget_sync(&togglebutton->base);
}

/**
 * @brief Gets the toggled state of a toggle button.
 * @param togglebutton The toggle button to query.
 * @param out_toggled Pointer to store the toggled state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_get_toggled(const cmn_togglebutton_t *togglebutton,
                                         cmn_bool_t *out_toggled) {
  if (validate_kind(togglebutton, CMN_WIDGET_KIND_TOGGLEBUTTON) !=
          CMN_SUCCESS ||
      out_toggled == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_toggled = togglebutton->toggled;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the toggle callback for a toggle button.
 * @param togglebutton The toggle button to modify.
 * @param cb Callback function invoked when toggled.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_set_on_toggle(cmn_togglebutton_t *togglebutton,
                                           cmn_togglebutton_toggle_fn cb,
                                           void *userdata) {
  if (validate_kind(togglebutton, CMN_WIDGET_KIND_TOGGLEBUTTON) !=
      CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  togglebutton->on_toggle = cb;
  togglebutton->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically toggles a toggle button and invokes its callback.
 * @param togglebutton The toggle button to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_trigger_toggle(cmn_togglebutton_t *togglebutton) {
  cmn_error_e rc;
  if (validate_kind(togglebutton, CMN_WIDGET_KIND_TOGGLEBUTTON) !=
      CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  togglebutton->toggled =
      (togglebutton->toggled == CMN_TRUE) ? CMN_FALSE : CMN_TRUE;
  rc = cmn_backend_widget_sync(&togglebutton->base);
  if (togglebutton->on_toggle != NULL) {
    rc = togglebutton->on_toggle(togglebutton->userdata);
  }
  return rc;
}

/**
 * @brief Upcasts a `cmn_togglebutton_t` to a `cmn_widget_t`.
 * @param togglebutton The toggle button instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_as_widget(cmn_togglebutton_t *togglebutton,
                                       cmn_widget_t **out_widget) {
  return upcast_checked(togglebutton, CMN_WIDGET_KIND_TOGGLEBUTTON,
                        (void **)out_widget);
}

/**
 * @brief Destroys a toggle button and frees its memory.
 * @param togglebutton Pointer to the toggle button pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_togglebutton_destroy(cmn_togglebutton_t **togglebutton) {
  return destroy_extended_widget((void **)togglebutton,
                                 CMN_WIDGET_KIND_TOGGLEBUTTON);
}

/* --- Line Separator --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_line_t`.
 * @param ptr Address of the line pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_line_adapter(void **ptr) {
  return cmn_line_destroy((cmn_line_t **)ptr);
}

/**
 * @brief Creates a horizontal separator line widget.
 * @param out_line Pointer to store the created line widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_line_create_horizontal(cmn_line_t **out_line) {
  return alloc_and_init_widget(sizeof(cmn_line_t), CMN_WIDGET_KIND_LINE,
                               destroy_line_adapter, (void **)out_line);
}

/**
 * @brief Creates a vertical separator line widget.
 * @param out_line Pointer to store the created line widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_line_create_vertical(cmn_line_t **out_line) {
  cmn_error_e rc;
  rc = cmn_line_create_horizontal(out_line);
  if (rc == CMN_SUCCESS) {
    (*out_line)->vertical = CMN_TRUE;
    rc = cmn_backend_widget_sync(&(*out_line)->base);
  }
  return rc;
}

/**
 * @brief Queries whether a separator line is vertical.
 * @param line The line widget to query.
 * @param out_vertical Pointer receiving `CMN_TRUE` if vertical, `CMN_FALSE` if
 * horizontal.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_line_is_vertical(const cmn_line_t *line,
                                 cmn_bool_t *out_vertical) {
  if (validate_kind(line, CMN_WIDGET_KIND_LINE) != CMN_SUCCESS ||
      out_vertical == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_vertical = line->vertical;
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_line_t` to a `cmn_widget_t`.
 * @param line The line widget instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_line_as_widget(cmn_line_t *line, cmn_widget_t **out_widget) {
  return upcast_checked(line, CMN_WIDGET_KIND_LINE, (void **)out_widget);
}

/**
 * @brief Destroys a separator line widget and frees its memory.
 * @param line Pointer to the line widget pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_line_destroy(cmn_line_t **line) {
  return destroy_extended_widget((void **)line, CMN_WIDGET_KIND_LINE);
}

/* --- Slider --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_slider_t`.
 * @param ptr Address of the slider pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_slider_adapter(void **ptr) {
  return cmn_slider_destroy((cmn_slider_t **)ptr);
}

/**
 * @brief Creates a horizontal slider widget.
 * @param out_slider Pointer to store the created slider.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_create(cmn_slider_t **out_slider) {
  return alloc_and_init_widget(sizeof(cmn_slider_t), CMN_WIDGET_KIND_SLIDER,
                               destroy_slider_adapter, (void **)out_slider);
}

/**
 * @brief Creates a vertical slider widget.
 * @param out_slider Pointer to store the created vertical slider.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_create_vertical(cmn_slider_t **out_slider) {
  cmn_error_e rc;
  rc = cmn_slider_create(out_slider);
  if (rc == CMN_SUCCESS) {
    (*out_slider)->vertical = CMN_TRUE;
    rc = cmn_backend_widget_sync(&(*out_slider)->base);
  }
  return rc;
}

/**
 * @brief Queries whether a slider is oriented vertically.
 * @param slider The slider to query.
 * @param out_vertical Pointer receiving `CMN_TRUE` if vertical, `CMN_FALSE` if
 * horizontal.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_is_vertical(const cmn_slider_t *slider,
                                   cmn_bool_t *out_vertical) {
  if (validate_kind(slider, CMN_WIDGET_KIND_SLIDER) != CMN_SUCCESS ||
      out_vertical == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_vertical = slider->vertical;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the normalized value (`[0.0, 1.0]`) of a slider.
 * @param slider The slider to modify.
 * @param value Normalized position in `[0.0, 1.0]`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_set_value(cmn_slider_t *slider, double value) {
  if (validate_kind(slider, CMN_WIDGET_KIND_SLIDER) != CMN_SUCCESS ||
      value < 0.0 || value > 1.0) {
    return CMN_ERR_INVAL;
  }
  slider->value = value;
  return cmn_backend_widget_sync(&slider->base);
}

/**
 * @brief Gets the normalized value (`[0.0, 1.0]`) of a slider.
 * @param slider The slider to query.
 * @param out_value Pointer to store the current value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_get_value(const cmn_slider_t *slider,
                                 double *out_value) {
  if (validate_kind(slider, CMN_WIDGET_KIND_SLIDER) != CMN_SUCCESS ||
      out_value == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_value = slider->value;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the discrete tick step count of a slider.
 * @param slider The slider to modify.
 * @param steps Number of discrete steps (`0` for continuous).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_set_steps(cmn_slider_t *slider, uint32_t steps) {
  if (validate_kind(slider, CMN_WIDGET_KIND_SLIDER) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  slider->steps = steps;
  return cmn_backend_widget_sync(&slider->base);
}

/**
 * @brief Gets the discrete tick step count of a slider.
 * @param slider The slider to query.
 * @param out_steps Pointer to store the discrete step count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_get_steps(const cmn_slider_t *slider,
                                 uint32_t *out_steps) {
  if (validate_kind(slider, CMN_WIDGET_KIND_SLIDER) != CMN_SUCCESS ||
      out_steps == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_steps = slider->steps;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the movement callback for a slider.
 * @param slider The slider to modify.
 * @param cb Callback function invoked when slider value changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_set_on_moved(cmn_slider_t *slider,
                                    cmn_slider_moved_fn cb, void *userdata) {
  if (validate_kind(slider, CMN_WIDGET_KIND_SLIDER) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  slider->on_moved = cb;
  slider->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the movement callback of a slider.
 * @param slider The slider to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_trigger_moved(cmn_slider_t *slider) {
  if (validate_kind(slider, CMN_WIDGET_KIND_SLIDER) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (slider->on_moved != NULL) {
    return slider->on_moved(slider->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_slider_t` to a `cmn_widget_t`.
 * @param slider The slider instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_as_widget(cmn_slider_t *slider,
                                 cmn_widget_t **out_widget) {
  return upcast_checked(slider, CMN_WIDGET_KIND_SLIDER, (void **)out_widget);
}

/**
 * @brief Destroys a slider and frees its memory.
 * @param slider Pointer to the slider pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_slider_destroy(cmn_slider_t **slider) {
  return destroy_extended_widget((void **)slider, CMN_WIDGET_KIND_SLIDER);
}

/* --- Progress Bar --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_progress_t`.
 * @param ptr Address of the progress bar pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_progress_adapter(void **ptr) {
  return cmn_progress_destroy((cmn_progress_t **)ptr);
}

/**
 * @brief Creates a new progress bar widget.
 * @param out_progress Pointer to store the created progress bar.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_progress_create(cmn_progress_t **out_progress) {
  return alloc_and_init_widget(sizeof(cmn_progress_t), CMN_WIDGET_KIND_PROGRESS,
                               destroy_progress_adapter, (void **)out_progress);
}

/**
 * @brief Sets the normalized value (`[0.0, 1.0]`) of a progress bar.
 * @param progress The progress bar to modify.
 * @param value Normalized completion ratio in `[0.0, 1.0]`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_progress_set_value(cmn_progress_t *progress, double value) {
  if (validate_kind(progress, CMN_WIDGET_KIND_PROGRESS) != CMN_SUCCESS ||
      value < 0.0 || value > 1.0) {
    return CMN_ERR_INVAL;
  }
  progress->value = value;
  progress->indeterminate = CMN_FALSE;
  return cmn_backend_widget_sync(&progress->base);
}

/**
 * @brief Gets the normalized value (`[0.0, 1.0]`) of a progress bar.
 * @param progress The progress bar to query.
 * @param out_value Pointer to store the current value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_progress_get_value(const cmn_progress_t *progress,
                                   double *out_value) {
  if (validate_kind(progress, CMN_WIDGET_KIND_PROGRESS) != CMN_SUCCESS ||
      out_value == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_value = progress->value;
  return CMN_SUCCESS;
}

/**
 * @brief Configures whether a progress bar is in indeterminate animation mode.
 * @param progress The progress bar to modify.
 * @param indeterminate `CMN_TRUE` for indeterminate mode, `CMN_FALSE` for
 * determinate mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_progress_set_indeterminate(cmn_progress_t *progress,
                                           cmn_bool_t indeterminate) {
  if (validate_kind(progress, CMN_WIDGET_KIND_PROGRESS) != CMN_SUCCESS ||
      (indeterminate != CMN_FALSE && indeterminate != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  progress->indeterminate = indeterminate;
  return cmn_backend_widget_sync(&progress->base);
}

/**
 * @brief Queries whether a progress bar is in indeterminate mode.
 * @param progress The progress bar to query.
 * @param out_indeterminate Pointer receiving `CMN_TRUE` if indeterminate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_progress_get_indeterminate(const cmn_progress_t *progress,
                                           cmn_bool_t *out_indeterminate) {
  if (validate_kind(progress, CMN_WIDGET_KIND_PROGRESS) != CMN_SUCCESS ||
      out_indeterminate == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_indeterminate = progress->indeterminate;
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_progress_t` to a `cmn_widget_t`.
 * @param progress The progress bar instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_progress_as_widget(cmn_progress_t *progress,
                                   cmn_widget_t **out_widget) {
  return upcast_checked(progress, CMN_WIDGET_KIND_PROGRESS,
                        (void **)out_widget);
}

/**
 * @brief Destroys a progress bar and frees its memory.
 * @param progress Pointer to the progress bar pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_progress_destroy(cmn_progress_t **progress) {
  return destroy_extended_widget((void **)progress, CMN_WIDGET_KIND_PROGRESS);
}

/* --- UpDown / Stepper --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_updown_t`.
 * @param ptr Address of the up-down pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_updown_adapter(void **ptr) {
  return cmn_updown_destroy((cmn_updown_t **)ptr);
}

/**
 * @brief Creates a new stepper / up-down widget.
 * @param out_updown Pointer to store the created up-down widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_create(cmn_updown_t **out_updown) {
  cmn_error_e rc;
  rc = alloc_and_init_widget(sizeof(cmn_updown_t), CMN_WIDGET_KIND_UPDOWN,
                             destroy_updown_adapter, (void **)out_updown);
  if (rc == CMN_SUCCESS) {
    (*out_updown)->step = 1.0;
  }
  return rc;
}

/**
 * @brief Sets the current numeric value of an up-down stepper.
 * @param updown The up-down widget to modify.
 * @param value The numeric value to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_set_value(cmn_updown_t *updown, double value) {
  if (validate_kind(updown, CMN_WIDGET_KIND_UPDOWN) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  updown->value = value;
  return cmn_backend_widget_sync(&updown->base);
}

/**
 * @brief Gets the current numeric value of an up-down stepper.
 * @param updown The up-down widget to query.
 * @param out_value Pointer to store the current numeric value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_get_value(const cmn_updown_t *updown,
                                 double *out_value) {
  if (validate_kind(updown, CMN_WIDGET_KIND_UPDOWN) != CMN_SUCCESS ||
      out_value == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_value = updown->value;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the step delta of an up-down stepper.
 * @param updown The up-down widget to modify.
 * @param step Positive step increment (`> 0.0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_set_step(cmn_updown_t *updown, double step) {
  if (validate_kind(updown, CMN_WIDGET_KIND_UPDOWN) != CMN_SUCCESS ||
      step <= 0.0) {
    return CMN_ERR_INVAL;
  }
  updown->step = step;
  return cmn_backend_widget_sync(&updown->base);
}

/**
 * @brief Gets the step delta of an up-down stepper.
 * @param updown The up-down widget to query.
 * @param out_step Pointer to store the step increment.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_get_step(const cmn_updown_t *updown, double *out_step) {
  if (validate_kind(updown, CMN_WIDGET_KIND_UPDOWN) != CMN_SUCCESS ||
      out_step == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_step = updown->step;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the step callback for an up-down stepper.
 * @param updown The up-down widget to modify.
 * @param cb Callback function invoked when stepped up or down.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_set_on_step(cmn_updown_t *updown, cmn_updown_step_fn cb,
                                   void *userdata) {
  if (validate_kind(updown, CMN_WIDGET_KIND_UPDOWN) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  updown->on_step = cb;
  updown->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically steps an up-down widget (`direction > 0` for up,
 * `< 0` for down) and invokes its callback.
 * @param updown The up-down widget to step.
 * @param direction `+1` to increment, `-1` to decrement.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_trigger_step(cmn_updown_t *updown, int direction) {
  cmn_error_e rc;
  if (validate_kind(updown, CMN_WIDGET_KIND_UPDOWN) != CMN_SUCCESS ||
      direction == 0) {
    return CMN_ERR_INVAL;
  }
  updown->value += (direction > 0) ? updown->step : -updown->step;
  rc = cmn_backend_widget_sync(&updown->base);
  if (updown->on_step != NULL) {
    rc = updown->on_step(updown->userdata);
  }
  return rc;
}

/**
 * @brief Upcasts a `cmn_updown_t` to a `cmn_widget_t`.
 * @param updown The up-down instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_as_widget(cmn_updown_t *updown,
                                 cmn_widget_t **out_widget) {
  return upcast_checked(updown, CMN_WIDGET_KIND_UPDOWN, (void **)out_widget);
}

/**
 * @brief Destroys an up-down widget and frees its memory.
 * @param updown Pointer to the up-down pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_updown_destroy(cmn_updown_t **updown) {
  return destroy_extended_widget((void **)updown, CMN_WIDGET_KIND_UPDOWN);
}

/* --- PopUp --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_popup_t`.
 * @param ptr Address of the pop-up pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_popup_adapter(void **ptr) {
  return cmn_popup_destroy((cmn_popup_t **)ptr);
}

/**
 * @brief Creates a new drop-down pop-up selector widget.
 * @param out_popup Pointer to store the created pop-up widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_create(cmn_popup_t **out_popup) {
  return alloc_and_init_widget(sizeof(cmn_popup_t), CMN_WIDGET_KIND_POPUP,
                               destroy_popup_adapter, (void **)out_popup);
}

/**
 * @brief Appends an option item to a pop-up selector.
 * @param popup The pop-up widget to modify.
 * @param text The item text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_add_item(cmn_popup_t *popup, const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS ||
      text == NULL) {
    return CMN_ERR_INVAL;
  }
  if (popup->count >= CMN_WIDGET_MAX_ITEMS) {
    return CMN_ERR_NOMEM;
  }
  rc = cmn_strcpy_s(popup->items[popup->count],
                    sizeof(popup->items[popup->count]), text);
  popup->count++;
  sync_rc = cmn_backend_widget_sync(&popup->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Replaces the text of an existing item in a pop-up selector.
 * @param popup The pop-up widget to modify.
 * @param index Zero-based item index.
 * @param text Replacement UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_set_item(cmn_popup_t *popup, uint32_t index,
                               const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS ||
      text == NULL || index >= popup->count) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(popup->items[index], sizeof(popup->items[index]), text);
  sync_rc = cmn_backend_widget_sync(&popup->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Removes all items from a pop-up selector.
 * @param popup The pop-up widget to clear.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_clear(cmn_popup_t *popup) {
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  popup->count = 0;
  popup->selected = 0;
  return cmn_backend_widget_sync(&popup->base);
}

/**
 * @brief Gets the number of items in a pop-up selector.
 * @param popup The pop-up widget to query.
 * @param out_count Pointer to store the item count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_get_count(const cmn_popup_t *popup, uint32_t *out_count) {
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS ||
      out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_count = popup->count;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the selected item index of a pop-up selector.
 * @param popup The pop-up widget to modify.
 * @param index Zero-based index of the item to select.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_set_selected(cmn_popup_t *popup, uint32_t index) {
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS ||
      index >= popup->count) {
    return CMN_ERR_INVAL;
  }
  popup->selected = index;
  return cmn_backend_widget_sync(&popup->base);
}

/**
 * @brief Gets the selected item index of a pop-up selector.
 * @param popup The pop-up widget to query.
 * @param out_index Pointer to store the selected item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_get_selected(const cmn_popup_t *popup,
                                   uint32_t *out_index) {
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS ||
      out_index == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_index = popup->selected;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the text of an item in a pop-up selector.
 * @param popup The pop-up widget to query.
 * @param index Zero-based item index.
 * @param out_text Pointer to store the item text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_get_item_text(const cmn_popup_t *popup, uint32_t index,
                                    const char **out_text) {
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS ||
      out_text == NULL || index >= popup->count) {
    return CMN_ERR_INVAL;
  }
  *out_text = popup->items[index];
  return CMN_SUCCESS;
}

/**
 * @brief Sets the selection callback for a pop-up selector.
 * @param popup The pop-up widget to modify.
 * @param cb Callback function invoked on item selection.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_set_on_select(cmn_popup_t *popup, cmn_popup_select_fn cb,
                                    void *userdata) {
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  popup->on_select = cb;
  popup->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the selection callback of a pop-up selector.
 * @param popup The pop-up widget to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_trigger_select(cmn_popup_t *popup) {
  if (validate_kind(popup, CMN_WIDGET_KIND_POPUP) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (popup->on_select != NULL) {
    return popup->on_select(popup->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_popup_t` to a `cmn_widget_t`.
 * @param popup The pop-up instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_as_widget(cmn_popup_t *popup, cmn_widget_t **out_widget) {
  return upcast_checked(popup, CMN_WIDGET_KIND_POPUP, (void **)out_widget);
}

/**
 * @brief Destroys a pop-up selector and frees its memory.
 * @param popup Pointer to the pop-up pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_popup_destroy(cmn_popup_t **popup) {
  return destroy_extended_widget((void **)popup, CMN_WIDGET_KIND_POPUP);
}

/* --- Combo --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_combo_t`.
 * @param ptr Address of the combo box pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_combo_adapter(void **ptr) {
  return cmn_combo_destroy((cmn_combo_t **)ptr);
}

/**
 * @brief Creates a new editable combo box widget.
 * @param out_combo Pointer to store the created combo box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_create(cmn_combo_t **out_combo) {
  return alloc_and_init_widget(sizeof(cmn_combo_t), CMN_WIDGET_KIND_COMBO,
                               destroy_combo_adapter, (void **)out_combo);
}

/**
 * @brief Sets the editable text of a combo box.
 * @param combo The combo box to modify.
 * @param text The new UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_set_text(cmn_combo_t *combo, const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS ||
      text == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(combo->text, sizeof(combo->text), text);
  sync_rc = cmn_backend_widget_sync(&combo->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the current editable text of a combo box.
 * @param combo The combo box to query.
 * @param out_text Pointer to store the current UTF-8 text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_get_text(const cmn_combo_t *combo,
                               const char **out_text) {
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS ||
      out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_text = combo->text;
  return CMN_SUCCESS;
}

/**
 * @brief Appends a suggestion item to a combo box.
 * @param combo The combo box to modify.
 * @param text The item text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_add_item(cmn_combo_t *combo, const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS ||
      text == NULL) {
    return CMN_ERR_INVAL;
  }
  if (combo->count >= CMN_WIDGET_MAX_ITEMS) {
    return CMN_ERR_NOMEM;
  }
  rc = cmn_strcpy_s(combo->items[combo->count],
                    sizeof(combo->items[combo->count]), text);
  combo->count++;
  sync_rc = cmn_backend_widget_sync(&combo->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Removes all suggestion items from a combo box.
 * @param combo The combo box to clear.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_clear(cmn_combo_t *combo) {
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  combo->count = 0;
  combo->selected = 0;
  return cmn_backend_widget_sync(&combo->base);
}

/**
 * @brief Gets the number of suggestion items in a combo box.
 * @param combo The combo box to query.
 * @param out_count Pointer to store the item count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_get_count(const cmn_combo_t *combo, uint32_t *out_count) {
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS ||
      out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_count = combo->count;
  return CMN_SUCCESS;
}

/**
 * @brief Selects a suggestion item by index and updates the combo box text.
 * @param combo The combo box to modify.
 * @param index Zero-based item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_set_selected(cmn_combo_t *combo, uint32_t index) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS ||
      index >= combo->count) {
    return CMN_ERR_INVAL;
  }
  combo->selected = index;
  rc = cmn_strcpy_s(combo->text, sizeof(combo->text), combo->items[index]);
  sync_rc = cmn_backend_widget_sync(&combo->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the currently selected item index of a combo box.
 * @param combo The combo box to query.
 * @param out_index Pointer to store the selected item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_get_selected(const cmn_combo_t *combo,
                                   uint32_t *out_index) {
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS ||
      out_index == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_index = combo->selected;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the change callback for a combo box.
 * @param combo The combo box to modify.
 * @param cb Callback function invoked when text or selection changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_set_on_change(cmn_combo_t *combo, cmn_combo_change_fn cb,
                                    void *userdata) {
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  combo->on_change = cb;
  combo->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the change callback of a combo box.
 * @param combo The combo box to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_trigger_change(cmn_combo_t *combo) {
  if (validate_kind(combo, CMN_WIDGET_KIND_COMBO) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (combo->on_change != NULL) {
    return combo->on_change(combo->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_combo_t` to a `cmn_widget_t`.
 * @param combo The combo box instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_as_widget(cmn_combo_t *combo, cmn_widget_t **out_widget) {
  return upcast_checked(combo, CMN_WIDGET_KIND_COMBO, (void **)out_widget);
}

/**
 * @brief Destroys a combo box and frees its memory.
 * @param combo Pointer to the combo box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_combo_destroy(cmn_combo_t **combo) {
  return destroy_extended_widget((void **)combo, CMN_WIDGET_KIND_COMBO);
}

/* --- ListBox --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_listbox_t`.
 * @param ptr Address of the list box pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_listbox_adapter(void **ptr) {
  return cmn_listbox_destroy((cmn_listbox_t **)ptr);
}

/**
 * @brief Creates a new scrollable list box widget.
 * @param out_listbox Pointer to store the created list box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_create(cmn_listbox_t **out_listbox) {
  return alloc_and_init_widget(sizeof(cmn_listbox_t), CMN_WIDGET_KIND_LISTBOX,
                               destroy_listbox_adapter, (void **)out_listbox);
}

/**
 * @brief Appends an item to a list box.
 * @param listbox The list box to modify.
 * @param text The item text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_add_item(cmn_listbox_t *listbox, const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      text == NULL) {
    return CMN_ERR_INVAL;
  }
  if (listbox->count >= CMN_WIDGET_MAX_ITEMS) {
    return CMN_ERR_NOMEM;
  }
  rc = cmn_strcpy_s(listbox->items[listbox->count],
                    sizeof(listbox->items[listbox->count]), text);
  listbox->checked[listbox->count] = CMN_FALSE;
  listbox->count++;
  sync_rc = cmn_backend_widget_sync(&listbox->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Removes an item at `index` from a list box.
 * @param listbox The list box to modify.
 * @param index Zero-based index of the item to remove.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_remove_item(cmn_listbox_t *listbox, uint32_t index) {
  uint32_t i;
  cmn_error_e rc = CMN_SUCCESS;
  cmn_error_e sync_rc;
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      index >= listbox->count) {
    return CMN_ERR_INVAL;
  }
  for (i = index; i + 1 < listbox->count; ++i) {
    cmn_error_e cp_rc = cmn_strcpy_s(
        listbox->items[i], sizeof(listbox->items[i]), listbox->items[i + 1]);
    rc = (rc != CMN_SUCCESS) ? rc : cp_rc;
    listbox->checked[i] = listbox->checked[i + 1];
  }
  listbox->count--;
  if (listbox->selected >= listbox->count && listbox->count > 0) {
    listbox->selected = listbox->count - 1;
  }
  sync_rc = cmn_backend_widget_sync(&listbox->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Clears all items from a list box.
 * @param listbox The list box to clear.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_clear(cmn_listbox_t *listbox) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  listbox->count = 0;
  listbox->selected = 0;
  return cmn_backend_widget_sync(&listbox->base);
}

/**
 * @brief Gets the number of items in a list box.
 * @param listbox The list box to query.
 * @param out_count Pointer to store the item count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_get_count(const cmn_listbox_t *listbox,
                                  uint32_t *out_count) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_count = listbox->count;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the text of an item in a list box.
 * @param listbox The list box to query.
 * @param index Zero-based item index.
 * @param out_text Pointer to store the item text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_get_item_text(const cmn_listbox_t *listbox,
                                      uint32_t index, const char **out_text) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      out_text == NULL || index >= listbox->count) {
    return CMN_ERR_INVAL;
  }
  *out_text = listbox->items[index];
  return CMN_SUCCESS;
}

/**
 * @brief Selects an item in a list box.
 * @param listbox The list box to modify.
 * @param index Zero-based item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_set_selected(cmn_listbox_t *listbox, uint32_t index) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      index >= listbox->count) {
    return CMN_ERR_INVAL;
  }
  listbox->selected = index;
  return cmn_backend_widget_sync(&listbox->base);
}

/**
 * @brief Gets the selected item index of a list box.
 * @param listbox The list box to query.
 * @param out_index Pointer to store the selected item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_get_selected(const cmn_listbox_t *listbox,
                                     uint32_t *out_index) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      out_index == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_index = listbox->selected;
  return CMN_SUCCESS;
}

/**
 * @brief Configures whether a list box displays per-row checkboxes.
 * @param listbox The list box to modify.
 * @param checkable `CMN_TRUE` to show checkboxes, `CMN_FALSE` otherwise.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_set_checkable(cmn_listbox_t *listbox,
                                      cmn_bool_t checkable) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      (checkable != CMN_FALSE && checkable != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  listbox->checkable = checkable;
  return cmn_backend_widget_sync(&listbox->base);
}

/**
 * @brief Queries whether a list box displays per-row checkboxes.
 * @param listbox The list box to query.
 * @param out_checkable Pointer receiving `CMN_TRUE` if checkable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_get_checkable(const cmn_listbox_t *listbox,
                                      cmn_bool_t *out_checkable) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      out_checkable == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_checkable = listbox->checkable;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the checked state of a specific item in a list box.
 * @param listbox The list box to modify.
 * @param index Zero-based item index.
 * @param checked `CMN_TRUE` if checked, `CMN_FALSE` otherwise.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_set_checked(cmn_listbox_t *listbox, uint32_t index,
                                    cmn_bool_t checked) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      index >= listbox->count ||
      (checked != CMN_FALSE && checked != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  listbox->checked[index] = checked;
  return cmn_backend_widget_sync(&listbox->base);
}

/**
 * @brief Gets the checked state of a specific item in a list box.
 * @param listbox The list box to query.
 * @param index Zero-based item index.
 * @param out_checked Pointer receiving the item's checked state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_get_checked(const cmn_listbox_t *listbox,
                                    uint32_t index, cmn_bool_t *out_checked) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS ||
      out_checked == NULL || index >= listbox->count) {
    return CMN_ERR_INVAL;
  }
  *out_checked = listbox->checked[index];
  return CMN_SUCCESS;
}

/**
 * @brief Sets the selection callback for a list box.
 * @param listbox The list box to modify.
 * @param cb Callback function invoked when selection changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_set_on_select(cmn_listbox_t *listbox,
                                      cmn_listbox_select_fn cb,
                                      void *userdata) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  listbox->on_select = cb;
  listbox->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the selection callback of a list box.
 * @param listbox The list box to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_trigger_select(cmn_listbox_t *listbox) {
  if (validate_kind(listbox, CMN_WIDGET_KIND_LISTBOX) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (listbox->on_select != NULL) {
    return listbox->on_select(listbox->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_listbox_t` to a `cmn_widget_t`.
 * @param listbox The list box instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_as_widget(cmn_listbox_t *listbox,
                                  cmn_widget_t **out_widget) {
  return upcast_checked(listbox, CMN_WIDGET_KIND_LISTBOX, (void **)out_widget);
}

/**
 * @brief Destroys a list box and frees its memory.
 * @param listbox Pointer to the list box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_listbox_destroy(cmn_listbox_t **listbox) {
  return destroy_extended_widget((void **)listbox, CMN_WIDGET_KIND_LISTBOX);
}

/* --- ImageView --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_imageview_t`.
 * @param ptr Address of the image view pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_imageview_adapter(void **ptr) {
  return cmn_imageview_destroy((cmn_imageview_t **)ptr);
}

/**
 * @brief Creates a new image view widget.
 * @param out_imageview Pointer to store the created image view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_create(cmn_imageview_t **out_imageview) {
  return alloc_and_init_widget(
      sizeof(cmn_imageview_t), CMN_WIDGET_KIND_IMAGEVIEW,
      destroy_imageview_adapter, (void **)out_imageview);
}

/**
 * @brief Sets the image path or URI displayed by an image view.
 * @param imageview The image view to modify.
 * @param path File path or URI string of the image.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_set_image_path(cmn_imageview_t *imageview,
                                         const char *path) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(imageview, CMN_WIDGET_KIND_IMAGEVIEW) != CMN_SUCCESS ||
      path == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(imageview->image_path, sizeof(imageview->image_path), path);
  sync_rc = cmn_backend_widget_sync(&imageview->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the image path or URI displayed by an image view.
 * @param imageview The image view to query.
 * @param out_path Pointer to store the image path string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_get_image_path(const cmn_imageview_t *imageview,
                                         const char **out_path) {
  if (validate_kind(imageview, CMN_WIDGET_KIND_IMAGEVIEW) != CMN_SUCCESS ||
      out_path == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_path = imageview->image_path;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the scaling mode of an image view.
 * @param imageview The image view to modify.
 * @param scale Scaling mode (`CMN_SCALE_AUTO`, `CMN_SCALE_NONE`,
 * `CMN_SCALE_ASPECT`, `CMN_SCALE_FIT`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_set_scale(cmn_imageview_t *imageview,
                                    cmn_scale_mode_e scale) {
  if (validate_kind(imageview, CMN_WIDGET_KIND_IMAGEVIEW) != CMN_SUCCESS ||
      scale < CMN_SCALE_AUTO || scale > CMN_SCALE_FIT) {
    return CMN_ERR_INVAL;
  }
  imageview->scale = scale;
  return cmn_backend_widget_sync(&imageview->base);
}

/**
 * @brief Gets the scaling mode of an image view.
 * @param imageview The image view to query.
 * @param out_scale Pointer to store the scaling mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_get_scale(const cmn_imageview_t *imageview,
                                    cmn_scale_mode_e *out_scale) {
  if (validate_kind(imageview, CMN_WIDGET_KIND_IMAGEVIEW) != CMN_SUCCESS ||
      out_scale == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_scale = imageview->scale;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the click callback for an image view.
 * @param imageview The image view to modify.
 * @param cb Callback function invoked when clicked.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_set_on_click(cmn_imageview_t *imageview,
                                       cmn_imageview_click_fn cb,
                                       void *userdata) {
  if (validate_kind(imageview, CMN_WIDGET_KIND_IMAGEVIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  imageview->on_click = cb;
  imageview->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the click callback of an image view.
 * @param imageview The image view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_trigger_click(cmn_imageview_t *imageview) {
  if (validate_kind(imageview, CMN_WIDGET_KIND_IMAGEVIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (imageview->on_click != NULL) {
    return imageview->on_click(imageview->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_imageview_t` to a `cmn_widget_t`.
 * @param imageview The image view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_as_widget(cmn_imageview_t *imageview,
                                    cmn_widget_t **out_widget) {
  return upcast_checked(imageview, CMN_WIDGET_KIND_IMAGEVIEW,
                        (void **)out_widget);
}

/**
 * @brief Destroys an image view and frees its memory.
 * @param imageview Pointer to the image view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_imageview_destroy(cmn_imageview_t **imageview) {
  return destroy_extended_widget((void **)imageview, CMN_WIDGET_KIND_IMAGEVIEW);
}

/* --- TableView --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_tableview_t`.
 * @param ptr Address of the table view pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_tableview_adapter(void **ptr) {
  return cmn_tableview_destroy((cmn_tableview_t **)ptr);
}

/**
 * @brief Creates a new multi-column table view widget.
 * @param out_tableview Pointer to store the created table view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_create(cmn_tableview_t **out_tableview) {
  return alloc_and_init_widget(
      sizeof(cmn_tableview_t), CMN_WIDGET_KIND_TABLEVIEW,
      destroy_tableview_adapter, (void **)out_tableview);
}

/**
 * @brief Appends a text column to a table view.
 * @param tableview The table view to modify.
 * @param title Header title for the column.
 * @param width Initial column width in pixels (`> 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_add_column(cmn_tableview_t *tableview,
                                     const char *title, int width) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      title == NULL || width <= 0) {
    return CMN_ERR_INVAL;
  }
  if (tableview->col_count >= CMN_TABLEVIEW_MAX_COLS) {
    return CMN_ERR_NOMEM;
  }
  rc = cmn_strcpy_s(tableview->col_titles[tableview->col_count],
                    sizeof(tableview->col_titles[tableview->col_count]), title);
  tableview->col_widths[tableview->col_count] = width;
  tableview->col_min_widths[tableview->col_count] = 0;
  tableview->col_max_widths[tableview->col_count] = width * 4;
  tableview->col_count++;
  sync_rc = cmn_backend_widget_sync(&tableview->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the number of columns in a table view.
 * @param tableview The table view to query.
 * @param out_cols Pointer to store the column count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_get_column_count(const cmn_tableview_t *tableview,
                                           uint32_t *out_cols) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      out_cols == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_cols = tableview->col_count;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the header title of a column in a table view.
 * @param tableview The table view to query.
 * @param col Zero-based column index.
 * @param out_title Pointer to store the column title string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_get_column_title(const cmn_tableview_t *tableview,
                                           uint32_t col,
                                           const char **out_title) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      out_title == NULL || col >= tableview->col_count) {
    return CMN_ERR_INVAL;
  }
  *out_title = tableview->col_titles[col];
  return CMN_SUCCESS;
}

/**
 * @brief Sets the total row count of a table view.
 * @param tableview The table view to modify.
 * @param rows Number of rows.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_set_row_count(cmn_tableview_t *tableview,
                                        uint32_t rows) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      rows > CMN_TABLEVIEW_MAX_ROWS) {
    return CMN_ERR_INVAL;
  }
  tableview->row_count = rows;
  return cmn_backend_widget_sync(&tableview->base);
}

/**
 * @brief Gets the total row count of a table view.
 * @param tableview The table view to query.
 * @param out_rows Pointer to store the row count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_get_row_count(const cmn_tableview_t *tableview,
                                        uint32_t *out_rows) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      out_rows == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_rows = tableview->row_count;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the text of a cell at `(row, col)` in a table view.
 * @param tableview The table view to modify.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @param text Cell text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_set_cell_text(cmn_tableview_t *tableview,
                                        uint32_t row, uint32_t col,
                                        const char *text) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      text == NULL || row >= tableview->row_count ||
      col >= tableview->col_count) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(tableview->cells[row][col],
                    sizeof(tableview->cells[row][col]), text);
  sync_rc = cmn_backend_widget_sync(&tableview->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the text of a cell at `(row, col)` in a table view.
 * @param tableview The table view to query.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @param out_text Pointer to store the cell text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_get_cell_text(const cmn_tableview_t *tableview,
                                        uint32_t row, uint32_t col,
                                        const char **out_text) {
  cmn_error_e rc;
  cmn_tableview_t *mut_tv;
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      out_text == NULL || row >= tableview->row_count ||
      col >= tableview->col_count) {
    return CMN_ERR_INVAL;
  }
  mut_tv = (cmn_tableview_t *)tableview;
  if (mut_tv->on_data != NULL) {
    rc =
        mut_tv->on_data(row, col, mut_tv->cells[row][col],
                        sizeof(mut_tv->cells[row][col]), mut_tv->data_userdata);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
  }
  *out_text = tableview->cells[row][col];
  return CMN_SUCCESS;
}

/**
 * @brief Selects a row in a table view.
 * @param tableview The table view to modify.
 * @param row Zero-based row index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_set_selected_row(cmn_tableview_t *tableview,
                                           uint32_t row) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      row >= tableview->row_count) {
    return CMN_ERR_INVAL;
  }
  tableview->selected_row = row;
  return cmn_backend_widget_sync(&tableview->base);
}

/**
 * @brief Gets the currently selected row in a table view.
 * @param tableview The table view to query.
 * @param out_row Pointer to store the selected row index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_get_selected_row(const cmn_tableview_t *tableview,
                                           uint32_t *out_row) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      out_row == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_row = tableview->selected_row;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the row selection callback for a table view.
 * @param tableview The table view to modify.
 * @param cb Callback function invoked when row selection changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_set_on_select(cmn_tableview_t *tableview,
                                        cmn_tableview_select_fn cb,
                                        void *userdata) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  tableview->on_select = cb;
  tableview->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the row selection callback of a table view.
 * @param tableview The table view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_trigger_select(cmn_tableview_t *tableview) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (tableview->on_select != NULL) {
    return tableview->on_select(tableview->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_tableview_t` to a `cmn_widget_t`.
 * @param tableview The table view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_as_widget(cmn_tableview_t *tableview,
                                    cmn_widget_t **out_widget) {
  return upcast_checked(tableview, CMN_WIDGET_KIND_TABLEVIEW,
                        (void **)out_widget);
}

/**
 * @brief Destroys a table view and frees its memory.
 * @param tableview Pointer to the table view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_destroy(cmn_tableview_t **tableview) {
  return destroy_extended_widget((void **)tableview, CMN_WIDGET_KIND_TABLEVIEW);
}

/* --- Custom View --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_view_t`.
 * @param ptr Address of the custom view pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_view_adapter(void **ptr) {
  return cmn_view_destroy((cmn_view_t **)ptr);
}

/**
 * @brief Creates a new custom 2D drawing view widget.
 * @param out_view Pointer to store the created custom view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_create(cmn_view_t **out_view) {
  cmn_error_e rc;
  rc = alloc_and_init_widget(sizeof(cmn_view_t), CMN_WIDGET_KIND_VIEW,
                             destroy_view_adapter, (void **)out_view);
  if (rc == CMN_SUCCESS) {
    (*out_view)->content_width = 360;
    (*out_view)->content_height = 140;
  }
  return rc;
}

/**
 * @brief Creates a new scrollable custom 2D drawing view widget.
 * @param out_view Pointer to store the created scrollable custom view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_create_scroll(cmn_view_t **out_view) {
  cmn_error_e rc;
  rc = cmn_view_create(out_view);
  if (rc == CMN_SUCCESS) {
    (*out_view)->scrollable = CMN_TRUE;
    rc = cmn_backend_widget_sync(&(*out_view)->base);
  }
  return rc;
}

/**
 * @brief Queries whether a custom view is scrollable.
 * @param view The custom view to query.
 * @param out_scrollable Pointer receiving `CMN_TRUE` if scrollable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_is_scrollable(const cmn_view_t *view,
                                   cmn_bool_t *out_scrollable) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS ||
      out_scrollable == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_scrollable = view->scrollable;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the virtual content size of a custom view.
 * @param view The custom view to modify.
 * @param width Content width in pixels (`> 0`).
 * @param height Content height in pixels (`> 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_set_content_size(cmn_view_t *view, int width, int height) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS || width <= 0 ||
      height <= 0) {
    return CMN_ERR_INVAL;
  }
  view->content_width = width;
  view->content_height = height;
  return cmn_backend_widget_sync(&view->base);
}

/**
 * @brief Gets the virtual content size of a custom view.
 * @param view The custom view to query.
 * @param out_width Pointer to store the content width.
 * @param out_height Pointer to store the content height.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_get_content_size(const cmn_view_t *view, int *out_width,
                                      int *out_height) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS ||
      out_width == NULL || out_height == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_width = view->content_width;
  *out_height = view->content_height;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the draw callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback function invoked on redraw.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_set_on_draw(cmn_view_t *view, cmn_view_event_fn cb,
                                 void *userdata) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  view->on_draw = cb;
  view->draw_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Invalidates a custom view and triggers its draw callback.
 * @param view The custom view to invalidate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_invalidate(cmn_view_t *view) {
  cmn_error_e rc;
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_backend_widget_sync(&view->base);
  if (view->on_draw != NULL) {
    rc = view->on_draw(view->draw_userdata);
  }
  return rc;
}

/**
 * @brief Sets the click callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback function invoked when clicked.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_set_on_click(cmn_view_t *view, cmn_view_event_fn cb,
                                  void *userdata) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  view->on_click = cb;
  view->click_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the click callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_trigger_click(cmn_view_t *view) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (view->on_click != NULL) {
    return view->on_click(view->click_userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_view_t` to a `cmn_widget_t`.
 * @param view The custom view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_as_widget(cmn_view_t *view, cmn_widget_t **out_widget) {
  return upcast_checked(view, CMN_WIDGET_KIND_VIEW, (void **)out_widget);
}

/**
 * @brief Destroys a custom view and frees its memory.
 * @param view Pointer to the custom view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_destroy(cmn_view_t **view) {
  return destroy_extended_widget((void **)view, CMN_WIDGET_KIND_VIEW);
}

/* --- WebView --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_webview_t`.
 * @param ptr Address of the web view pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_webview_adapter(void **ptr) {
  return cmn_webview_destroy((cmn_webview_t **)ptr);
}

/**
 * @brief Creates a new embedded web browser view widget.
 * @param out_webview Pointer to store the created web view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_create(cmn_webview_t **out_webview) {
  return alloc_and_init_widget(sizeof(cmn_webview_t), CMN_WIDGET_KIND_WEBVIEW,
                               destroy_webview_adapter, (void **)out_webview);
}

/**
 * @brief Navigates a web view to the given URL.
 * @param webview The web view to navigate.
 * @param url Target URL string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_navigate(cmn_webview_t *webview, const char *url) {
  cmn_error_e rc;
  cmn_error_e url_rc;
  cmn_error_e sync_rc;
  if (validate_kind(webview, CMN_WIDGET_KIND_WEBVIEW) != CMN_SUCCESS ||
      url == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(webview->prev_url, sizeof(webview->prev_url), webview->url);
  url_rc = cmn_strcpy_s(webview->url, sizeof(webview->url), url);
  rc = (rc != CMN_SUCCESS) ? rc : url_rc;
  sync_rc = cmn_backend_widget_sync(&webview->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the current URL of a web view.
 * @param webview The web view to query.
 * @param out_url Pointer to store the URL string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_get_url(const cmn_webview_t *webview,
                                const char **out_url) {
  if (validate_kind(webview, CMN_WIDGET_KIND_WEBVIEW) != CMN_SUCCESS ||
      out_url == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_url = webview->url;
  return CMN_SUCCESS;
}

/**
 * @brief Loads raw HTML source code into a web view.
 * @param webview The web view to modify.
 * @param html UTF-8 HTML document string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_load_html(cmn_webview_t *webview, const char *html) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(webview, CMN_WIDGET_KIND_WEBVIEW) != CMN_SUCCESS ||
      html == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_strcpy_s(webview->html, sizeof(webview->html), html);
  sync_rc = cmn_backend_widget_sync(&webview->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the last loaded HTML source code of a web view.
 * @param webview The web view to query.
 * @param out_html Pointer to store the HTML string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_get_html(const cmn_webview_t *webview,
                                 const char **out_html) {
  if (validate_kind(webview, CMN_WIDGET_KIND_WEBVIEW) != CMN_SUCCESS ||
      out_html == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_html = webview->html;
  return CMN_SUCCESS;
}

/**
 * @brief Navigates back in the web view history.
 * @param webview The web view to navigate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_go_back(cmn_webview_t *webview) {
  cmn_error_e rc = CMN_SUCCESS;
  cmn_error_e sync_rc;
  if (validate_kind(webview, CMN_WIDGET_KIND_WEBVIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (webview->prev_url[0] != '\0') {
    cmn_error_e url_rc;
    rc = cmn_strcpy_s(webview->next_url, sizeof(webview->next_url),
                      webview->url);
    url_rc =
        cmn_strcpy_s(webview->url, sizeof(webview->url), webview->prev_url);
    rc = (rc != CMN_SUCCESS) ? rc : url_rc;
  }
  sync_rc = cmn_backend_widget_sync(&webview->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Navigates forward in the web view history.
 * @param webview The web view to navigate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_go_forward(cmn_webview_t *webview) {
  cmn_error_e rc = CMN_SUCCESS;
  cmn_error_e sync_rc;
  if (validate_kind(webview, CMN_WIDGET_KIND_WEBVIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (webview->next_url[0] != '\0') {
    cmn_error_e url_rc;
    rc = cmn_strcpy_s(webview->prev_url, sizeof(webview->prev_url),
                      webview->url);
    url_rc =
        cmn_strcpy_s(webview->url, sizeof(webview->url), webview->next_url);
    rc = (rc != CMN_SUCCESS) ? rc : url_rc;
  }
  sync_rc = cmn_backend_widget_sync(&webview->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Upcasts a `cmn_webview_t` to a `cmn_widget_t`.
 * @param webview The web view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_as_widget(cmn_webview_t *webview,
                                  cmn_widget_t **out_widget) {
  return upcast_checked(webview, CMN_WIDGET_KIND_WEBVIEW, (void **)out_widget);
}

/**
 * @brief Destroys a web view and frees its memory.
 * @param webview Pointer to the web view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_webview_destroy(cmn_webview_t **webview) {
  return destroy_extended_widget((void **)webview, CMN_WIDGET_KIND_WEBVIEW);
}

/* --- Panel Container --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_panel_t`.
 * @param ptr Address of the panel pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_panel_adapter(void **ptr) {
  return cmn_panel_destroy((cmn_panel_t **)ptr);
}

/**
 * @brief Creates a new grouping panel container.
 * @param out_panel Pointer to store the created panel.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_panel_create(cmn_panel_t **out_panel) {
  return alloc_and_init_widget(sizeof(cmn_panel_t), CMN_WIDGET_KIND_PANEL,
                               destroy_panel_adapter, (void **)out_panel);
}

/**
 * @brief Creates a new scrollable panel container.
 * @param out_panel Pointer to store the created scrollable panel.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_panel_create_scroll(cmn_panel_t **out_panel) {
  cmn_error_e rc;
  rc = cmn_panel_create(out_panel);
  if (rc == CMN_SUCCESS) {
    (*out_panel)->scrollable = CMN_TRUE;
    rc = cmn_backend_widget_sync(&(*out_panel)->base);
  }
  return rc;
}

/**
 * @brief Queries whether a panel container has scrollbars enabled.
 * @param panel The panel to query.
 * @param out_scrollable Pointer receiving `CMN_TRUE` if scrollable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_panel_is_scrollable(const cmn_panel_t *panel,
                                    cmn_bool_t *out_scrollable) {
  if (validate_kind(panel, CMN_WIDGET_KIND_PANEL) != CMN_SUCCESS ||
      out_scrollable == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_scrollable = panel->scrollable;
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_panel_t` to a `cmn_container_t`.
 * @param panel The panel instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_panel_as_container(cmn_panel_t *panel,
                                   cmn_container_t **out_container) {
  return upcast_checked(panel, CMN_WIDGET_KIND_PANEL, (void **)out_container);
}

/**
 * @brief Upcasts a `cmn_panel_t` to a `cmn_widget_t`.
 * @param panel The panel instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_panel_as_widget(cmn_panel_t *panel, cmn_widget_t **out_widget) {
  return upcast_checked(panel, CMN_WIDGET_KIND_PANEL, (void **)out_widget);
}

/**
 * @brief Destroys a panel container and frees its memory.
 * @param panel Pointer to the panel pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_panel_destroy(cmn_panel_t **panel) {
  return destroy_extended_widget((void **)panel, CMN_WIDGET_KIND_PANEL);
}

/* --- Tabs Container --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_tabs_t`.
 * @param ptr Address of the tabs pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_tabs_adapter(void **ptr) {
  return cmn_tabs_destroy((cmn_tabs_t **)ptr);
}

/**
 * @brief Creates a new tabbed container widget.
 * @param out_tabs Pointer to store the created tabs widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_create(cmn_tabs_t **out_tabs) {
  return alloc_and_init_widget(sizeof(cmn_tabs_t), CMN_WIDGET_KIND_TABS,
                               destroy_tabs_adapter, (void **)out_tabs);
}

/**
 * @brief Appends a tab page with a title and child content widget.
 * @param tabs The tabs container to modify.
 * @param title Title label of the tab page.
 * @param content Child widget or container for the tab page (may be NULL).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_add_tab(cmn_tabs_t *tabs, const char *title,
                             void *content) {
  cmn_error_e rc;
  cmn_error_e sync_rc;
  if (validate_kind(tabs, CMN_WIDGET_KIND_TABS) != CMN_SUCCESS ||
      title == NULL) {
    return CMN_ERR_INVAL;
  }
  if (tabs->count >= CMN_WIDGET_MAX_ITEMS) {
    return CMN_ERR_NOMEM;
  }
  rc = cmn_strcpy_s(tabs->titles[tabs->count],
                    sizeof(tabs->titles[tabs->count]), title);
  tabs->contents[tabs->count] = content;
  tabs->count++;
  sync_rc = cmn_backend_widget_sync(&tabs->base);
  return (rc != CMN_SUCCESS) ? rc : sync_rc;
}

/**
 * @brief Gets the number of tabs in a tabbed container.
 * @param tabs The tabs container to query.
 * @param out_count Pointer to store the tab count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_get_count(const cmn_tabs_t *tabs, uint32_t *out_count) {
  if (validate_kind(tabs, CMN_WIDGET_KIND_TABS) != CMN_SUCCESS ||
      out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_count = tabs->count;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the title of a tab page by index.
 * @param tabs The tabs container to query.
 * @param index Zero-based tab index.
 * @param out_title Pointer to store the tab title string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_get_tab_title(const cmn_tabs_t *tabs, uint32_t index,
                                   const char **out_title) {
  if (validate_kind(tabs, CMN_WIDGET_KIND_TABS) != CMN_SUCCESS ||
      out_title == NULL || index >= tabs->count) {
    return CMN_ERR_INVAL;
  }
  *out_title = tabs->titles[index];
  return CMN_SUCCESS;
}

/**
 * @brief Gets the content widget associated with a tab page.
 * @param tabs The tabs container to query.
 * @param index Zero-based tab index.
 * @param out_content Pointer to store the content widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_get_tab_content(const cmn_tabs_t *tabs, uint32_t index,
                                     void **out_content) {
  if (validate_kind(tabs, CMN_WIDGET_KIND_TABS) != CMN_SUCCESS ||
      out_content == NULL || index >= tabs->count) {
    return CMN_ERR_INVAL;
  }
  *out_content = tabs->contents[index];
  return CMN_SUCCESS;
}

/**
 * @brief Selects the active tab page by index.
 * @param tabs The tabs container to modify.
 * @param index Zero-based tab index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_set_selected(cmn_tabs_t *tabs, uint32_t index) {
  if (validate_kind(tabs, CMN_WIDGET_KIND_TABS) != CMN_SUCCESS ||
      index >= tabs->count) {
    return CMN_ERR_INVAL;
  }
  tabs->selected = index;
  return cmn_backend_widget_sync(&tabs->base);
}

/**
 * @brief Gets the active tab page index.
 * @param tabs The tabs container to query.
 * @param out_index Pointer to store the active tab index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_get_selected(const cmn_tabs_t *tabs, uint32_t *out_index) {
  if (validate_kind(tabs, CMN_WIDGET_KIND_TABS) != CMN_SUCCESS ||
      out_index == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_index = tabs->selected;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the tab selection callback.
 * @param tabs The tabs container to modify.
 * @param cb Callback function invoked when active tab changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_set_on_select(cmn_tabs_t *tabs, cmn_tabs_select_fn cb,
                                   void *userdata) {
  if (validate_kind(tabs, CMN_WIDGET_KIND_TABS) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  tabs->on_select = cb;
  tabs->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the tab selection callback.
 * @param tabs The tabs container to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_trigger_select(cmn_tabs_t *tabs) {
  if (validate_kind(tabs, CMN_WIDGET_KIND_TABS) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (tabs->on_select != NULL) {
    return tabs->on_select(tabs->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_tabs_t` to a `cmn_widget_t`.
 * @param tabs The tabs container instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_as_widget(cmn_tabs_t *tabs, cmn_widget_t **out_widget) {
  return upcast_checked(tabs, CMN_WIDGET_KIND_TABS, (void **)out_widget);
}

/**
 * @brief Destroys a tabs container and frees its memory.
 * @param tabs Pointer to the tabs pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tabs_destroy(cmn_tabs_t **tabs) {
  return destroy_extended_widget((void **)tabs, CMN_WIDGET_KIND_TABS);
}

/* --- SplitView Container --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_splitview_t`.
 * @param ptr Address of the split view pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_splitview_adapter(void **ptr) {
  return cmn_splitview_destroy((cmn_splitview_t **)ptr);
}

/**
 * @brief Creates a horizontal split view with a horizontal divider.
 * @param out_splitview Pointer to store the created split view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_create_horizontal(cmn_splitview_t **out_splitview) {
  cmn_error_e rc;
  rc = alloc_and_init_widget(sizeof(cmn_splitview_t), CMN_WIDGET_KIND_SPLITVIEW,
                             destroy_splitview_adapter, (void **)out_splitview);
  if (rc == CMN_SUCCESS) {
    (*out_splitview)->pos = 0.5;
    (*out_splitview)->pane_visible[0] = CMN_TRUE;
    (*out_splitview)->pane_visible[1] = CMN_TRUE;
  }
  return rc;
}

/**
 * @brief Creates a vertical split view with a vertical divider.
 * @param out_splitview Pointer to store the created split view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_create_vertical(cmn_splitview_t **out_splitview) {
  cmn_error_e rc;
  rc = cmn_splitview_create_horizontal(out_splitview);
  if (rc == CMN_SUCCESS) {
    (*out_splitview)->vertical = CMN_TRUE;
    rc = cmn_backend_widget_sync(&(*out_splitview)->base);
  }
  return rc;
}

/**
 * @brief Queries whether a split view uses a vertical divider.
 * @param splitview The split view to query.
 * @param out_vertical Pointer receiving `CMN_TRUE` if vertical.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_is_vertical(const cmn_splitview_t *splitview,
                                      cmn_bool_t *out_vertical) {
  if (validate_kind(splitview, CMN_WIDGET_KIND_SPLITVIEW) != CMN_SUCCESS ||
      out_vertical == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_vertical = splitview->vertical;
  return CMN_SUCCESS;
}

/**
 * @brief Attaches the two child panes of a split view.
 * @param splitview The split view to modify.
 * @param child0 First pane child widget.
 * @param child1 Second pane child widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_set_children(cmn_splitview_t *splitview, void *child0,
                                       void *child1) {
  cmn_widget_kind_e k0 = CMN_WIDGET_KIND_NONE;
  cmn_widget_kind_e k1 = CMN_WIDGET_KIND_NONE;
  if (validate_kind(splitview, CMN_WIDGET_KIND_SPLITVIEW) != CMN_SUCCESS ||
      cmn_widget_get_kind(child0, &k0) != CMN_SUCCESS ||
      cmn_widget_get_kind(child1, &k1) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  splitview->child0 = child0;
  splitview->child1 = child1;
  return cmn_backend_widget_sync(&splitview->base);
}

/**
 * @brief Gets the two child panes of a split view.
 * @param splitview The split view to query.
 * @param out_child0 Pointer to store the first pane widget.
 * @param out_child1 Pointer to store the second pane widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_get_children(const cmn_splitview_t *splitview,
                                       void **out_child0, void **out_child1) {
  if (validate_kind(splitview, CMN_WIDGET_KIND_SPLITVIEW) != CMN_SUCCESS ||
      out_child0 == NULL || out_child1 == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_child0 = splitview->child0;
  *out_child1 = splitview->child1;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the normalized divider position (`[0.0, 1.0]`) of a split view.
 * @param splitview The split view to modify.
 * @param pos Normalized divider ratio in `[0.0, 1.0]`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_set_pos(cmn_splitview_t *splitview, double pos) {
  if (validate_kind(splitview, CMN_WIDGET_KIND_SPLITVIEW) != CMN_SUCCESS ||
      pos < 0.0 || pos > 1.0) {
    return CMN_ERR_INVAL;
  }
  splitview->pos = pos;
  return cmn_backend_widget_sync(&splitview->base);
}

/**
 * @brief Gets the normalized divider position (`[0.0, 1.0]`) of a split view.
 * @param splitview The split view to query.
 * @param out_pos Pointer to store the divider ratio.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_get_pos(const cmn_splitview_t *splitview,
                                  double *out_pos) {
  if (validate_kind(splitview, CMN_WIDGET_KIND_SPLITVIEW) != CMN_SUCCESS ||
      out_pos == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_pos = splitview->pos;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the visibility of pane `0` or pane `1`.
 * @param splitview The split view to modify.
 * @param pane_index Pane index (`0` or `1`).
 * @param visible `CMN_TRUE` if visible, `CMN_FALSE` if collapsed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_set_pane_visible(cmn_splitview_t *splitview,
                                           int pane_index, cmn_bool_t visible) {
  if (validate_kind(splitview, CMN_WIDGET_KIND_SPLITVIEW) != CMN_SUCCESS ||
      (pane_index != 0 && pane_index != 1) ||
      (visible != CMN_FALSE && visible != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  splitview->pane_visible[pane_index] = visible;
  return cmn_backend_widget_sync(&splitview->base);
}

/**
 * @brief Gets the visibility of pane `0` or pane `1`.
 * @param splitview The split view to query.
 * @param pane_index Pane index (`0` or `1`).
 * @param out_visible Pointer receiving `CMN_TRUE` if visible.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_get_pane_visible(const cmn_splitview_t *splitview,
                                           int pane_index,
                                           cmn_bool_t *out_visible) {
  if (validate_kind(splitview, CMN_WIDGET_KIND_SPLITVIEW) != CMN_SUCCESS ||
      (pane_index != 0 && pane_index != 1) || out_visible == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_visible = splitview->pane_visible[pane_index];
  return CMN_SUCCESS;
}

/**
 * @brief Upcasts a `cmn_splitview_t` to a `cmn_widget_t`.
 * @param splitview The split view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_as_widget(cmn_splitview_t *splitview,
                                    cmn_widget_t **out_widget) {
  return upcast_checked(splitview, CMN_WIDGET_KIND_SPLITVIEW,
                        (void **)out_widget);
}

/**
 * @brief Destroys a split view and frees its memory.
 * @param splitview Pointer to the split view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_splitview_destroy(cmn_splitview_t **splitview) {
  return destroy_extended_widget((void **)splitview, CMN_WIDGET_KIND_SPLITVIEW);
}

/* --- ColorPicker Dialog --- */

/**
 * @brief Polymorphic destructor adapter for `cmn_colorpicker_t`.
 * @param ptr Address of the color picker pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_colorpicker_adapter(void **ptr) {
  return cmn_colorpicker_destroy((cmn_colorpicker_t **)ptr);
}

/**
 * @brief Creates a new native color picker dialog.
 * @param out_colorpicker Pointer to store the created color picker.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_colorpicker_create(cmn_colorpicker_t **out_colorpicker) {
  cmn_error_e rc;
  rc = alloc_and_init_widget(
      sizeof(cmn_colorpicker_t), CMN_WIDGET_KIND_COLORPICKER,
      destroy_colorpicker_adapter, (void **)out_colorpicker);
  if (rc == CMN_SUCCESS) {
    (*out_colorpicker)->rgba = 0x000000FFu;
  }
  return rc;
}

/**
 * @brief Sets the packed RGBA (`0xRRGGBBAA`) color of a color picker.
 * @param colorpicker The color picker to modify.
 * @param rgba Packed 32-bit RGBA color value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_colorpicker_set_color(cmn_colorpicker_t *colorpicker,
                                      uint32_t rgba) {
  if (validate_kind(colorpicker, CMN_WIDGET_KIND_COLORPICKER) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  colorpicker->rgba = rgba;
  return cmn_backend_widget_sync(&colorpicker->base);
}

/**
 * @brief Gets the packed RGBA (`0xRRGGBBAA`) color of a color picker.
 * @param colorpicker The color picker to query.
 * @param out_rgba Pointer to store the packed 32-bit RGBA color value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_colorpicker_get_color(const cmn_colorpicker_t *colorpicker,
                                      uint32_t *out_rgba) {
  if (validate_kind(colorpicker, CMN_WIDGET_KIND_COLORPICKER) != CMN_SUCCESS ||
      out_rgba == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_rgba = colorpicker->rgba;
  return CMN_SUCCESS;
}

/**
 * @brief Shows the native color picker dialog.
 * @param colorpicker The color picker dialog to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_colorpicker_show_dialog(cmn_colorpicker_t *colorpicker) {
  if (validate_kind(colorpicker, CMN_WIDGET_KIND_COLORPICKER) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  return cmn_backend_widget_sync(&colorpicker->base);
}

/**
 * @brief Sets the color change callback for a color picker.
 * @param colorpicker The color picker to modify.
 * @param cb Callback function invoked when a color is chosen.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_colorpicker_set_on_change(cmn_colorpicker_t *colorpicker,
                                          cmn_colorpicker_change_fn cb,
                                          void *userdata) {
  if (validate_kind(colorpicker, CMN_WIDGET_KIND_COLORPICKER) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  colorpicker->on_change = cb;
  colorpicker->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the color change callback.
 * @param colorpicker The color picker to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_colorpicker_trigger_change(cmn_colorpicker_t *colorpicker) {
  if (validate_kind(colorpicker, CMN_WIDGET_KIND_COLORPICKER) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (colorpicker->on_change != NULL) {
    return colorpicker->on_change(colorpicker->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a color picker and frees its memory.
 * @param colorpicker Pointer to the color picker pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_colorpicker_destroy(cmn_colorpicker_t **colorpicker) {
  return destroy_extended_widget((void **)colorpicker,
                                 CMN_WIDGET_KIND_COLORPICKER);
}

/* --- Extended Window Controls --- */

/**
 * @brief Sets the default push button activated on Enter key in a window.
 * @param window The window to modify.
 * @param button The default push button (may be NULL to clear).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_default_button(cmn_window_t *window,
                                          cmn_button_t *button) {
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS ||
      (button != NULL &&
       validate_kind(button, CMN_WIDGET_KIND_BUTTON) != CMN_SUCCESS)) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)window)->default_button = button;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the default push button configured for a window.
 * @param window The window to query.
 * @param out_button Pointer to store the default push button handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_get_default_button(const cmn_window_t *window,
                                          cmn_button_t **out_button) {
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS ||
      out_button == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_button = ((const cmn_widget_header_t *)window)->default_button;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the active mouse cursor shape for a window.
 * @param window The window to modify.
 * @param cursor Cursor shape (`CMN_CURSOR_ARROW`, `CMN_CURSOR_HAND`, etc.).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_cursor(cmn_window_t *window, cmn_cursor_e cursor) {
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS ||
      cursor < CMN_CURSOR_ARROW || cursor > CMN_CURSOR_SIZENS) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)window)->cursor = cursor;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the active mouse cursor shape for a window.
 * @param window The window to query.
 * @param out_cursor Pointer to store the cursor shape.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_get_cursor(const cmn_window_t *window,
                                  cmn_cursor_e *out_cursor) {
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS ||
      out_cursor == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_cursor = ((const cmn_widget_header_t *)window)->cursor;
  return CMN_SUCCESS;
}

/**
 * @brief Converts window client-area coordinates to screen coordinates.
 * @param window The reference window.
 * @param client_x X coordinate in window client space.
 * @param client_y Y coordinate in window client space.
 * @param out_screen_x Pointer receiving screen X coordinate.
 * @param out_screen_y Pointer receiving screen Y coordinate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_client_to_screen(const cmn_window_t *window,
                                        int client_x, int client_y,
                                        int *out_screen_x, int *out_screen_y) {
  const cmn_widget_header_t *hdr;
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS ||
      out_screen_x == NULL || out_screen_y == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (const cmn_widget_header_t *)window;
  *out_screen_x = hdr->bounds.x + client_x;
  *out_screen_y = hdr->bounds.y + client_y;
  return CMN_SUCCESS;
}

/**
 * @brief Registers the close lifecycle callback for a window.
 * @param window The window to modify.
 * @param cb Callback invoked when the window is closing.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_on_close(cmn_window_t *window,
                                    cmn_window_event_fn cb, void *userdata) {
  cmn_widget_header_t *hdr;
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)window;
  hdr->on_close = cb;
  hdr->close_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the close callback of a window.
 * @param window The window to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_trigger_close(cmn_window_t *window) {
  cmn_widget_header_t *hdr;
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)window;
  if (hdr->on_close != NULL) {
    return hdr->on_close(hdr->close_userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Registers the moved lifecycle callback for a window.
 * @param window The window to modify.
 * @param cb Callback invoked when the window moves.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_on_moved(cmn_window_t *window,
                                    cmn_window_event_fn cb, void *userdata) {
  cmn_widget_header_t *hdr;
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)window;
  hdr->on_moved = cb;
  hdr->moved_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the moved callback of a window.
 * @param window The window to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_trigger_moved(cmn_window_t *window) {
  cmn_widget_header_t *hdr;
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)window;
  if (hdr->on_moved != NULL) {
    return hdr->on_moved(hdr->moved_userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Registers the activation lifecycle callback for a window.
 * @param window The window to modify.
 * @param cb Callback invoked when the window becomes active.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_on_activate(cmn_window_t *window,
                                       cmn_window_event_fn cb, void *userdata) {
  cmn_widget_header_t *hdr;
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)window;
  hdr->on_activate = cb;
  hdr->activate_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the activate callback of a window.
 * @param window The window to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_trigger_activate(cmn_window_t *window) {
  cmn_widget_header_t *hdr;
  if (validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)window;
  if (hdr->on_activate != NULL) {
    return hdr->on_activate(hdr->activate_userdata);
  }
  return CMN_SUCCESS;
}

/* --- Extended Menu & MenuItem Controls --- */

/**
 * @brief Appends a horizontal separator item to a menu.
 * @param menu The menu to modify.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_add_separator(cmn_menu_t *menu) {
  cmn_widget_header_t *hdr;
  if (validate_kind(menu, CMN_WIDGET_KIND_MENU) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)menu;
  if (hdr->menu_ext_count >= 32) {
    return CMN_ERR_NOMEM;
  }
  hdr->menu_item_enabled[hdr->menu_ext_count] = CMN_FALSE;
  hdr->menu_item_state[hdr->menu_ext_count] = CMN_CHECK_OFF;
  hdr->menu_ext_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Attaches a hierarchical submenu item to a menu.
 * @param menu The parent menu to modify.
 * @param title Label title for the submenu item.
 * @param submenu The child submenu to attach.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_add_submenu(cmn_menu_t *menu, const char *title,
                                 cmn_menu_t *submenu) {
  cmn_widget_header_t *hdr;
  if (validate_kind(menu, CMN_WIDGET_KIND_MENU) != CMN_SUCCESS ||
      title == NULL ||
      validate_kind(submenu, CMN_WIDGET_KIND_MENU) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)menu;
  if (hdr->menu_ext_count >= 32) {
    return CMN_ERR_NOMEM;
  }
  hdr->menu_item_enabled[hdr->menu_ext_count] = CMN_TRUE;
  hdr->menu_item_state[hdr->menu_ext_count] = CMN_CHECK_OFF;
  hdr->menu_ext_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the enabled state of a menu item by index.
 * @param menu The menu to modify.
 * @param index Zero-based menu item index (`>= 0`).
 * @param enabled `CMN_TRUE` to enable, `CMN_FALSE` to disable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_set_item_enabled(cmn_menu_t *menu, int index,
                                      cmn_bool_t enabled) {
  cmn_widget_header_t *hdr;
  if (validate_kind(menu, CMN_WIDGET_KIND_MENU) != CMN_SUCCESS || index < 0 ||
      index >= 32 || (enabled != CMN_FALSE && enabled != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)menu;
  hdr->menu_item_enabled[index] = enabled;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the enabled state of a menu item by index.
 * @param menu The menu to query.
 * @param index Zero-based menu item index (`>= 0`).
 * @param out_enabled Pointer receiving `CMN_TRUE` if enabled.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_get_item_enabled(const cmn_menu_t *menu, int index,
                                      cmn_bool_t *out_enabled) {
  const cmn_widget_header_t *hdr;
  if (validate_kind(menu, CMN_WIDGET_KIND_MENU) != CMN_SUCCESS || index < 0 ||
      index >= 32 || out_enabled == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (const cmn_widget_header_t *)menu;
  *out_enabled = hdr->menu_item_enabled[index];
  return CMN_SUCCESS;
}

/**
 * @brief Sets the check state of a menu item by index.
 * @param menu The menu to modify.
 * @param index Zero-based menu item index (`>= 0`).
 * @param state Check state (`CMN_CHECK_OFF`, `CMN_CHECK_ON`,
 * `CMN_CHECK_MIXED`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_set_item_state(cmn_menu_t *menu, int index,
                                    cmn_check_state_e state) {
  cmn_widget_header_t *hdr;
  if (validate_kind(menu, CMN_WIDGET_KIND_MENU) != CMN_SUCCESS || index < 0 ||
      index >= 32 || state < CMN_CHECK_OFF || state > CMN_CHECK_MIXED) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)menu;
  hdr->menu_item_state[index] = state;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the check state of a menu item by index.
 * @param menu The menu to query.
 * @param index Zero-based menu item index (`>= 0`).
 * @param out_state Pointer receiving the item's check state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_get_item_state(const cmn_menu_t *menu, int index,
                                    cmn_check_state_e *out_state) {
  const cmn_widget_header_t *hdr;
  if (validate_kind(menu, CMN_WIDGET_KIND_MENU) != CMN_SUCCESS || index < 0 ||
      index >= 32 || out_state == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (const cmn_widget_header_t *)menu;
  *out_state = hdr->menu_item_state[index];
  return CMN_SUCCESS;
}

/**
 * @brief Displays a menu as a context popup at `(x, y)` in a window.
 * @param menu The popup menu to display.
 * @param window The parent window.
 * @param x Popup X coordinate.
 * @param y Popup Y coordinate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_popup(cmn_menu_t *menu, cmn_window_t *window, int x,
                           int y) {
  cmn_widget_header_t *hdr;
  if (validate_kind(menu, CMN_WIDGET_KIND_MENU) != CMN_SUCCESS ||
      validate_kind(window, CMN_WIDGET_KIND_WINDOW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)menu;
  hdr->popup_x = x;
  hdr->popup_y = y;
  return CMN_SUCCESS;
}

/* --- Extended FilePicker Controls --- */

/**
 * @brief Sets the dialog mode of a file picker.
 * @param filepicker The file picker to modify.
 * @param mode Dialog mode (`CMN_FILEPICKER_OPEN_FILE`,
 * `CMN_FILEPICKER_SAVE_FILE`, `CMN_FILEPICKER_SELECT_DIR`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_set_mode(cmn_filepicker_t *filepicker,
                                    cmn_filepicker_mode_e mode) {
  if (validate_kind(filepicker, CMN_WIDGET_KIND_FILEPICKER) != CMN_SUCCESS ||
      mode < CMN_FILEPICKER_OPEN_FILE || mode > CMN_FILEPICKER_SELECT_DIR) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)filepicker)->fp_mode = mode;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the dialog mode of a file picker.
 * @param filepicker The file picker to query.
 * @param out_mode Pointer receiving the dialog mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_get_mode(const cmn_filepicker_t *filepicker,
                                    cmn_filepicker_mode_e *out_mode) {
  if (validate_kind(filepicker, CMN_WIDGET_KIND_FILEPICKER) != CMN_SUCCESS ||
      out_mode == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_mode = ((const cmn_widget_header_t *)filepicker)->fp_mode;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the allowed file extension filters of a file picker.
 * @param filepicker The file picker to modify.
 * @param extensions Array of file extension strings (e.g., `{"png", "jpg"}`).
 * @param count Number of extension strings in `extensions`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_set_filetypes(cmn_filepicker_t *filepicker,
                                         const char *const *extensions,
                                         size_t count) {
  size_t i;
  if (validate_kind(filepicker, CMN_WIDGET_KIND_FILEPICKER) != CMN_SUCCESS ||
      (count > 0 && extensions == NULL)) {
    return CMN_ERR_INVAL;
  }
  for (i = 0; i < count; ++i) {
    if (extensions[i] == NULL) {
      return CMN_ERR_INVAL;
    }
  }
  ((cmn_widget_header_t *)filepicker)->fp_filetype_count = count;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the number of configured file extension filters on a file picker.
 * @param filepicker The file picker to query.
 * @param out_count Pointer receiving the filter count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e
cmn_filepicker_get_filetype_count(const cmn_filepicker_t *filepicker,
                                  size_t *out_count) {
  if (validate_kind(filepicker, CMN_WIDGET_KIND_FILEPICKER) != CMN_SUCCESS ||
      out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_count = ((const cmn_widget_header_t *)filepicker)->fp_filetype_count;
  return CMN_SUCCESS;
}

/* --- Extended Push Button Controls --- */

/**
 * @brief Sets the hover tooltip text of a push button.
 * @param button The button to modify.
 * @param tooltip UTF-8 tooltip text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_set_tooltip(cmn_button_t *button, const char *tooltip) {
  cmn_widget_header_t *hdr;
  if (validate_kind(button, CMN_WIDGET_KIND_BUTTON) != CMN_SUCCESS ||
      tooltip == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)button;
  return cmn_strcpy_s(hdr->tooltip, sizeof(hdr->tooltip), tooltip);
}

/**
 * @brief Gets the hover tooltip text of a push button.
 * @param button The button to query.
 * @param out_tooltip Pointer receiving the tooltip string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_get_tooltip(const cmn_button_t *button,
                                   const char **out_tooltip) {
  if (validate_kind(button, CMN_WIDGET_KIND_BUTTON) != CMN_SUCCESS ||
      out_tooltip == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_tooltip = ((const cmn_widget_header_t *)button)->tooltip;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the icon image path of a push button.
 * @param button The button to modify.
 * @param image_path File path or resource URI of the icon image.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_set_image(cmn_button_t *button, const char *image_path) {
  cmn_widget_header_t *hdr;
  if (validate_kind(button, CMN_WIDGET_KIND_BUTTON) != CMN_SUCCESS ||
      image_path == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)button;
  return cmn_strcpy_s(hdr->icon_path, sizeof(hdr->icon_path), image_path);
}

/**
 * @brief Gets the icon image path of a push button.
 * @param button The button to query.
 * @param out_image_path Pointer receiving the image path string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_get_image(const cmn_button_t *button,
                                 const char **out_image_path) {
  if (validate_kind(button, CMN_WIDGET_KIND_BUTTON) != CMN_SUCCESS ||
      out_image_path == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_image_path = ((const cmn_widget_header_t *)button)->icon_path;
  return CMN_SUCCESS;
}

/* --- Extended Label Controls --- */

/**
 * @brief Configures whether a label wraps text across multiple lines.
 * @param label The label to modify.
 * @param multiline `CMN_TRUE` for multi-line wrapping, `CMN_FALSE` for single
 * line.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_set_multiline(cmn_label_t *label, cmn_bool_t multiline) {
  if (validate_kind(label, CMN_WIDGET_KIND_LABEL) != CMN_SUCCESS ||
      (multiline != CMN_FALSE && multiline != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)label)->multiline = multiline;
  return CMN_SUCCESS;
}

/**
 * @brief Queries whether a label wraps text across multiple lines.
 * @param label The label to query.
 * @param out_multiline Pointer receiving `CMN_TRUE` if multi-line.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_get_multiline(const cmn_label_t *label,
                                    cmn_bool_t *out_multiline) {
  if (validate_kind(label, CMN_WIDGET_KIND_LABEL) != CMN_SUCCESS ||
      out_multiline == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_multiline = ((const cmn_widget_header_t *)label)->multiline;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the horizontal text alignment of a label.
 * @param label The label to modify.
 * @param align Text alignment (`CMN_TEXT_ALIGN_LEFT`, `CMN_TEXT_ALIGN_CENTER`,
 * `CMN_TEXT_ALIGN_RIGHT`, `CMN_TEXT_ALIGN_JUSTIFY`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_set_align(cmn_label_t *label, cmn_text_align_e align) {
  if (validate_kind(label, CMN_WIDGET_KIND_LABEL) != CMN_SUCCESS ||
      align < CMN_TEXT_ALIGN_LEFT || align > CMN_TEXT_ALIGN_JUSTIFY) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)label)->text_align = align;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the horizontal text alignment of a label.
 * @param label The label to query.
 * @param out_align Pointer receiving the text alignment.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_get_align(const cmn_label_t *label,
                                cmn_text_align_e *out_align) {
  if (validate_kind(label, CMN_WIDGET_KIND_LABEL) != CMN_SUCCESS ||
      out_align == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_align = ((const cmn_widget_header_t *)label)->text_align;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the foreground text RGBA color of a label.
 * @param label The label to modify.
 * @param rgba Packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_set_color(cmn_label_t *label, uint32_t rgba) {
  if (validate_kind(label, CMN_WIDGET_KIND_LABEL) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)label)->fg_rgba = rgba;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the foreground text RGBA color of a label.
 * @param label The label to query.
 * @param out_rgba Pointer receiving the packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_get_color(const cmn_label_t *label, uint32_t *out_rgba) {
  if (validate_kind(label, CMN_WIDGET_KIND_LABEL) != CMN_SUCCESS ||
      out_rgba == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_rgba = ((const cmn_widget_header_t *)label)->fg_rgba;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the background RGBA color of a label.
 * @param label The label to modify.
 * @param rgba Packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_set_bgcolor(cmn_label_t *label, uint32_t rgba) {
  if (validate_kind(label, CMN_WIDGET_KIND_LABEL) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)label)->bg_rgba = rgba;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the background RGBA color of a label.
 * @param label The label to query.
 * @param out_rgba Pointer receiving the packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_get_bgcolor(const cmn_label_t *label,
                                  uint32_t *out_rgba) {
  if (validate_kind(label, CMN_WIDGET_KIND_LABEL) != CMN_SUCCESS ||
      out_rgba == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_rgba = ((const cmn_widget_header_t *)label)->bg_rgba;
  return CMN_SUCCESS;
}

/* --- Extended TextBox Controls --- */

/**
 * @brief Sets whether a text box is user-editable.
 * @param textbox The text box to modify.
 * @param editable `CMN_TRUE` if editable, `CMN_FALSE` if read-only.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_editable(cmn_textbox_t *textbox,
                                     cmn_bool_t editable) {
  if (validate_kind(textbox, CMN_WIDGET_KIND_TEXTBOX) != CMN_SUCCESS ||
      (editable != CMN_FALSE && editable != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)textbox)->editable = editable;
  return CMN_SUCCESS;
}

/**
 * @brief Gets whether a text box is user-editable.
 * @param textbox The text box to query.
 * @param out_editable Pointer receiving `CMN_TRUE` if editable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_get_editable(const cmn_textbox_t *textbox,
                                     cmn_bool_t *out_editable) {
  if (validate_kind(textbox, CMN_WIDGET_KIND_TEXTBOX) != CMN_SUCCESS ||
      out_editable == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_editable = ((const cmn_widget_header_t *)textbox)->editable;
  return CMN_SUCCESS;
}

/**
 * @brief Sets whether a text box automatically selects all text on focus.
 * @param textbox The text box to modify.
 * @param autoselect `CMN_TRUE` to auto-select on focus.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_autoselect(cmn_textbox_t *textbox,
                                       cmn_bool_t autoselect) {
  if (validate_kind(textbox, CMN_WIDGET_KIND_TEXTBOX) != CMN_SUCCESS ||
      (autoselect != CMN_FALSE && autoselect != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)textbox)->autoselect = autoselect;
  return CMN_SUCCESS;
}

/**
 * @brief Gets whether a text box automatically selects all text on focus.
 * @param textbox The text box to query.
 * @param out_autoselect Pointer receiving `CMN_TRUE` if auto-select is active.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_get_autoselect(const cmn_textbox_t *textbox,
                                       cmn_bool_t *out_autoselect) {
  if (validate_kind(textbox, CMN_WIDGET_KIND_TEXTBOX) != CMN_SUCCESS ||
      out_autoselect == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_autoselect = ((const cmn_widget_header_t *)textbox)->autoselect;
  return CMN_SUCCESS;
}

/**
 * @brief Selects a character range `[start, end]` in a text box.
 * @param textbox The text box to modify.
 * @param start Selection start character offset (`>= 0`).
 * @param end Selection end character offset (`>= start` or `-1` for end).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_select_range(cmn_textbox_t *textbox, int start,
                                     int end) {
  cmn_widget_header_t *hdr;
  if (validate_kind(textbox, CMN_WIDGET_KIND_TEXTBOX) != CMN_SUCCESS ||
      start < 0 || (end != -1 && end < start)) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)textbox;
  hdr->sel_start = start;
  hdr->sel_end = end;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the selected character range `[start, end]` of a text box.
 * @param textbox The text box to query.
 * @param out_start Pointer receiving selection start offset.
 * @param out_end Pointer receiving selection end offset.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_get_selection_range(const cmn_textbox_t *textbox,
                                            int *out_start, int *out_end) {
  const cmn_widget_header_t *hdr;
  if (validate_kind(textbox, CMN_WIDGET_KIND_TEXTBOX) != CMN_SUCCESS ||
      out_start == NULL || out_end == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (const cmn_widget_header_t *)textbox;
  *out_start = hdr->sel_start;
  *out_end = hdr->sel_end;
  return CMN_SUCCESS;
}

/**
 * @brief Performs a clipboard operation (`copy`, `cut`, `paste`) on a text box.
 * @param textbox The text box to operate on.
 * @param op Clipboard operation (`CMN_CLIPBOARD_COPY`, `CMN_CLIPBOARD_CUT`,
 * `CMN_CLIPBOARD_PASTE`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_clipboard_ops(cmn_textbox_t *textbox,
                                      cmn_clipboard_op_e op) {
  if (validate_kind(textbox, CMN_WIDGET_KIND_TEXTBOX) != CMN_SUCCESS ||
      op < CMN_CLIPBOARD_COPY || op > CMN_CLIPBOARD_PASTE) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/* --- Extended TextView Rich Formatting Controls --- */

/**
 * @brief Sets the font family, point size, and style mask for a text view.
 * @param textview The text view to modify.
 * @param family Font family name (e.g., `"Menlo"`).
 * @param size Font size in points (`> 0.0`).
 * @param style_mask Bitmask of `cmn_font_style_e` flags.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_set_font(cmn_textview_t *textview, const char *family,
                                  double size, uint32_t style_mask) {
  cmn_widget_header_t *hdr;
  if (validate_kind(textview, CMN_WIDGET_KIND_TEXTVIEW) != CMN_SUCCESS ||
      family == NULL || size <= 0.0) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)textview;
  hdr->font_size = size;
  hdr->font_style_mask = style_mask;
  return cmn_strcpy_s(hdr->font_family, sizeof(hdr->font_family), family);
}

/**
 * @brief Gets the font family, point size, and style mask of a text view.
 * @param textview The text view to query.
 * @param out_family Pointer receiving the font family string pointer.
 * @param out_size Pointer receiving the font size in points.
 * @param out_style_mask Pointer receiving the style bitmask.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_get_font(const cmn_textview_t *textview,
                                  const char **out_family, double *out_size,
                                  uint32_t *out_style_mask) {
  const cmn_widget_header_t *hdr;
  if (validate_kind(textview, CMN_WIDGET_KIND_TEXTVIEW) != CMN_SUCCESS ||
      out_family == NULL || out_size == NULL || out_style_mask == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (const cmn_widget_header_t *)textview;
  *out_family = hdr->font_family;
  *out_size = hdr->font_size;
  *out_style_mask = hdr->font_style_mask;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the foreground and background RGBA colors of a text view.
 * @param textview The text view to modify.
 * @param text_rgba Foreground text RGBA color.
 * @param bg_rgba Background RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_set_colors(cmn_textview_t *textview,
                                    uint32_t text_rgba, uint32_t bg_rgba) {
  cmn_widget_header_t *hdr;
  if (validate_kind(textview, CMN_WIDGET_KIND_TEXTVIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  hdr = (cmn_widget_header_t *)textview;
  hdr->fg_rgba = text_rgba;
  hdr->bg_rgba = bg_rgba;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the foreground and background RGBA colors of a text view.
 * @param textview The text view to query.
 * @param out_text_rgba Pointer receiving the foreground RGBA color.
 * @param out_bg_rgba Pointer receiving the background RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_get_colors(const cmn_textview_t *textview,
                                    uint32_t *out_text_rgba,
                                    uint32_t *out_bg_rgba) {
  const cmn_widget_header_t *hdr;
  if (validate_kind(textview, CMN_WIDGET_KIND_TEXTVIEW) != CMN_SUCCESS ||
      out_text_rgba == NULL || out_bg_rgba == NULL) {
    return CMN_ERR_INVAL;
  }
  hdr = (const cmn_widget_header_t *)textview;
  *out_text_rgba = hdr->fg_rgba;
  *out_bg_rgba = hdr->bg_rgba;
  return CMN_SUCCESS;
}

/**
 * @brief Configures line wrapping on a text view.
 * @param textview The text view to modify.
 * @param wrap `CMN_TRUE` to wrap lines, `CMN_FALSE` for horizontal scrolling.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_set_wrap(cmn_textview_t *textview, cmn_bool_t wrap) {
  if (validate_kind(textview, CMN_WIDGET_KIND_TEXTVIEW) != CMN_SUCCESS ||
      (wrap != CMN_FALSE && wrap != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)textview)->text_wrap = wrap;
  return CMN_SUCCESS;
}

/**
 * @brief Queries whether line wrapping is enabled on a text view.
 * @param textview The text view to query.
 * @param out_wrap Pointer receiving `CMN_TRUE` if line wrapping is enabled.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_get_wrap(const cmn_textview_t *textview,
                                  cmn_bool_t *out_wrap) {
  if (validate_kind(textview, CMN_WIDGET_KIND_TEXTVIEW) != CMN_SUCCESS ||
      out_wrap == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_wrap = ((const cmn_widget_header_t *)textview)->text_wrap;
  return CMN_SUCCESS;
}

/* --- Extended TableView Controls --- */

/**
 * @brief Registers a virtualized cell data callback on a table view.
 * @param tableview The table view to modify.
 * @param cb Data provider callback invoked when querying virtual cells.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_set_on_data(cmn_tableview_t *tableview,
                                      cmn_tableview_data_fn cb,
                                      void *userdata) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  tableview->on_data = cb;
  tableview->data_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Sets minimum and maximum width limits for a table column.
 * @param tableview The table view to modify.
 * @param col Zero-based column index.
 * @param min_width Minimum column width in pixels (`>= 0`).
 * @param max_width Maximum column width in pixels (`>= min_width`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_set_column_limits(cmn_tableview_t *tableview,
                                            uint32_t col, int min_width,
                                            int max_width) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      col >= tableview->col_count || min_width < 0 || max_width < min_width) {
    return CMN_ERR_INVAL;
  }
  tableview->col_min_widths[col] = min_width;
  tableview->col_max_widths[col] = max_width;
  return cmn_backend_widget_sync(&tableview->base);
}

/**
 * @brief Gets the minimum and maximum width limits for a table column.
 * @param tableview The table view to query.
 * @param col Zero-based column index.
 * @param out_min_width Pointer receiving minimum column width.
 * @param out_max_width Pointer receiving maximum column width.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_get_column_limits(const cmn_tableview_t *tableview,
                                            uint32_t col, int *out_min_width,
                                            int *out_max_width) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      col >= tableview->col_count || out_min_width == NULL ||
      out_max_width == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_min_width = tableview->col_min_widths[col];
  *out_max_width = tableview->col_max_widths[col];
  return CMN_SUCCESS;
}

/**
 * @brief Sets the number of frozen leading columns in a table view.
 * @param tableview The table view to modify.
 * @param freeze_cols Number of leading columns to freeze (`<= col_count`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_set_column_freeze(cmn_tableview_t *tableview,
                                            uint32_t freeze_cols) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      freeze_cols > tableview->col_count) {
    return CMN_ERR_INVAL;
  }
  tableview->freeze_cols = freeze_cols;
  return cmn_backend_widget_sync(&tableview->base);
}

/**
 * @brief Gets the number of frozen leading columns in a table view.
 * @param tableview The table view to query.
 * @param out_freeze_cols Pointer receiving the frozen column count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_tableview_get_column_freeze(const cmn_tableview_t *tableview,
                                            uint32_t *out_freeze_cols) {
  if (validate_kind(tableview, CMN_WIDGET_KIND_TABLEVIEW) != CMN_SUCCESS ||
      out_freeze_cols == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_freeze_cols = tableview->freeze_cols;
  return CMN_SUCCESS;
}

/* --- Extended Custom View 2D Drawing & Input Events --- */

/**
 * @brief Records a 2D filled/stroked rectangle command on a custom view.
 * @param view The custom view to draw into.
 * @param rect Rectangle geometry (`width >= 0`, `height >= 0`).
 * @param rgba Packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_draw_rect(cmn_view_t *view, cmn_rect_t rect,
                               uint32_t rgba) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS ||
      rect.width < 0 || rect.height < 0) {
    return CMN_ERR_INVAL;
  }
  view->last_rect = rect;
  view->last_rgba = rgba;
  view->draw_cmd_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Records a 2D line segment command on a custom view.
 * @param view The custom view to draw into.
 * @param x0 Start X coordinate.
 * @param y0 Start Y coordinate.
 * @param x1 End X coordinate.
 * @param y1 End Y coordinate.
 * @param rgba Packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_draw_line(cmn_view_t *view, int x0, int y0, int x1, int y1,
                               uint32_t rgba) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  view->last_x0 = x0;
  view->last_y0 = y0;
  view->last_x1 = x1;
  view->last_y1 = y1;
  view->last_rgba = rgba;
  view->draw_cmd_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Records a 2D text draw command on a custom view.
 * @param view The custom view to draw into.
 * @param text UTF-8 text string to render.
 * @param x Anchor X coordinate.
 * @param y Anchor Y coordinate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_draw_text(cmn_view_t *view, const char *text, int x,
                               int y) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS ||
      text == NULL) {
    return CMN_ERR_INVAL;
  }
  view->last_x0 = x;
  view->last_y0 = y;
  view->draw_cmd_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the total number of 2D draw commands recorded on a custom view.
 * @param view The custom view to query.
 * @param out_count Pointer receiving the draw command count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_get_draw_command_count(const cmn_view_t *view,
                                            uint32_t *out_count) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS ||
      out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_count = view->draw_cmd_count;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the mouse wheel callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback invoked on wheel scroll.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_set_on_wheel(cmn_view_t *view, cmn_view_event_fn cb,
                                  void *userdata) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  view->on_wheel = cb;
  view->wheel_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the wheel callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_trigger_wheel(cmn_view_t *view) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (view->on_wheel != NULL) {
    return view->on_wheel(view->wheel_userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the mouse drag callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback invoked on pointer drag.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_set_on_drag(cmn_view_t *view, cmn_view_event_fn cb,
                                 void *userdata) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  view->on_drag = cb;
  view->drag_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the drag callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_trigger_drag(cmn_view_t *view) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (view->on_drag != NULL) {
    return view->on_drag(view->drag_userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the key-down callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback invoked on key press.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_set_on_key_down(cmn_view_t *view, cmn_view_event_fn cb,
                                     void *userdata) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  view->on_key_down = cb;
  view->key_down_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the key-down callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_trigger_key_down(cmn_view_t *view) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (view->on_key_down != NULL) {
    return view->on_key_down(view->key_down_userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the key-up callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback invoked on key release.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_set_on_key_up(cmn_view_t *view, cmn_view_event_fn cb,
                                   void *userdata) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  view->on_key_up = cb;
  view->key_up_userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers the key-up callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_view_trigger_key_up(cmn_view_t *view) {
  if (validate_kind(view, CMN_WIDGET_KIND_VIEW) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (view->on_key_up != NULL) {
    return view->on_key_up(view->key_up_userdata);
  }
  return CMN_SUCCESS;
}

/* --- Cell-Level Widget Controls --- */

/**
 * @brief Sets whether any widget is interactive/enabled.
 * @param widget The widget to modify.
 * @param enabled `CMN_TRUE` to enable, `CMN_FALSE` to disable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_set_enabled(void *widget, cmn_bool_t enabled) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER ||
      kind == CMN_WIDGET_KIND_COLORPICKER ||
      (enabled != CMN_FALSE && enabled != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)widget)->enabled = enabled;
  return CMN_SUCCESS;
}

/**
 * @brief Queries whether any widget is interactive/enabled.
 * @param widget The widget to query.
 * @param out_enabled Pointer receiving `CMN_TRUE` if enabled.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_get_enabled(const void *widget,
                                   cmn_bool_t *out_enabled) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER ||
      kind == CMN_WIDGET_KIND_COLORPICKER || out_enabled == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_enabled = ((const cmn_widget_header_t *)widget)->enabled;
  return CMN_SUCCESS;
}

/**
 * @brief Sets whether any widget is visible in its layout cell.
 * @param widget The widget to modify.
 * @param visible `CMN_TRUE` to show, `CMN_FALSE` to hide.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_set_visible(void *widget, cmn_bool_t visible) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER ||
      kind == CMN_WIDGET_KIND_COLORPICKER ||
      (visible != CMN_FALSE && visible != CMN_TRUE)) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)widget)->visible = visible;
  return CMN_SUCCESS;
}

/**
 * @brief Queries whether any widget is visible in its layout cell.
 * @param widget The widget to query.
 * @param out_visible Pointer receiving `CMN_TRUE` if visible.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_get_visible(const void *widget,
                                   cmn_bool_t *out_visible) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER ||
      kind == CMN_WIDGET_KIND_COLORPICKER || out_visible == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_visible = ((const cmn_widget_header_t *)widget)->visible;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the cell padding edges around any widget.
 * @param widget The widget to modify.
 * @param padding Padding edges (`top >= 0`, `right >= 0`, `bottom >= 0`,
 * `left >= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_set_padding(void *widget, cmn_edges_t padding) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER ||
      kind == CMN_WIDGET_KIND_COLORPICKER || padding.top < 0 ||
      padding.right < 0 || padding.bottom < 0 || padding.left < 0) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)widget)->padding = padding;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the cell padding edges around any widget.
 * @param widget The widget to query.
 * @param out_padding Pointer receiving the `cmn_edges_t` padding.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_get_padding(const void *widget,
                                   cmn_edges_t *out_padding) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER ||
      kind == CMN_WIDGET_KIND_COLORPICKER || out_padding == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_padding = ((const cmn_widget_header_t *)widget)->padding;
  return CMN_SUCCESS;
}
