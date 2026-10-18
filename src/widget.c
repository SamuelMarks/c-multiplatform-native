/* clang-format off */
#include "cmn_internal.h"
#include "cmn/widget.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file widget.c
 * @brief Shared polymorphic widget tagged-header, type-safe upcasting, and
 * generic destructor dispatch.
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
  switch (kind) {
  case CMN_WIDGET_KIND_WINDOW:
  case CMN_WIDGET_KIND_VBOX:
  case CMN_WIDGET_KIND_HBOX:
  case CMN_WIDGET_KIND_FLEX:
  case CMN_WIDGET_KIND_GRID:
    header->bounds.width = 480;
    header->bounds.height = 360;
    break;
  case CMN_WIDGET_KIND_TEXTVIEW:
    header->bounds.width = 360;
    header->bounds.height = 140;
    break;
  case CMN_WIDGET_KIND_TEXTBOX:
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
 * @brief Internal helper validating that `ptr` has `expected_kind` and storing
 * `ptr` into `*out_target`.
 * @param ptr The source widget or container handle.
 * @param expected_kind The expected `cmn_widget_kind_e` discriminant.
 * @param out_target Output pointer receiving `ptr`.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL otherwise.
 */
static cmn_error_e upcast_checked(void *ptr, cmn_widget_kind_e expected_kind,
                                  void **out_target) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_error_e rc;
  if (out_target == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_widget_get_kind(ptr, &kind);
  if (rc != CMN_SUCCESS || kind != expected_kind) {
    return CMN_ERR_INVAL;
  }
  *out_target = ptr;
  return CMN_SUCCESS;
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
      kind == CMN_WIDGET_KIND_FILEPICKER) {
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
      kind == CMN_WIDGET_KIND_FILEPICKER) {
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
