/* clang-format off */
#include "cmn_linux_internal.h"
#include "handle_map.h"
#include "cmn/automation.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include "cmn/a11y_i18n.h"
#include <string.h>
/* clang-format on */

/**
 * @file cmn_linux_widget.c
 * @brief Linux GTK3 implementation of the widget ontology.
 */

/** @brief Mock flag to force `cmn_textbox_get_text` failure. */
static int g_cmn_mock_textbox_get_text_fail = 0;
/** @brief Last handle map operation status code. */
static int g_linux_map_rc = 0;

/** @brief Flag tracking whether GTK has been initialized. */
static int g_linux_gtk_initialized = 0;

/**
 * @brief Ensures GTK3 is initialized before creating any GTK widget.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e ensure_gtk_init(void) {
  if (!g_linux_gtk_initialized) {
    int argc = 0;
    char **argv = NULL;
    if (gtk_init_check(&argc, &argv)) {
      g_linux_gtk_initialized = 1;
    }
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets mock failure state for cmn_textbox_get_text.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_textbox_get_text_set(int fail) {
  g_cmn_mock_textbox_get_text_fail = fail;
  return CMN_SUCCESS;
}

/**
 * @brief Polymorphic destructor adapter for `cmn_window_t`.
 * @param ptr Address of the window pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_window_adapter(void **ptr) {
  return cmn_window_destroy((cmn_window_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_vbox_t`.
 * @param ptr Address of the vbox pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_vbox_adapter(void **ptr) {
  return cmn_vbox_destroy((cmn_vbox_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_hbox_t`.
 * @param ptr Address of the hbox pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_hbox_adapter(void **ptr) {
  return cmn_hbox_destroy((cmn_hbox_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_button_t`.
 * @param ptr Address of the button pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_button_adapter(void **ptr) {
  return cmn_button_destroy((cmn_button_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_label_t`.
 * @param ptr Address of the label pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_label_adapter(void **ptr) {
  return cmn_label_destroy((cmn_label_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_textbox_t`.
 * @param ptr Address of the textbox pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_textbox_adapter(void **ptr) {
  return cmn_textbox_destroy((cmn_textbox_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_textview_t`.
 * @param ptr Address of the textview pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_textview_adapter(void **ptr) {
  return cmn_textview_destroy((cmn_textview_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_menu_t`.
 * @param ptr Address of the menu pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_menu_adapter(void **ptr) {
  return cmn_menu_destroy((cmn_menu_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_filepicker_t`.
 * @param ptr Address of the filepicker pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_filepicker_adapter(void **ptr) {
  return cmn_filepicker_destroy((cmn_filepicker_t **)ptr);
}

/**
 * @brief GTK3 signal callback for window destruction.
 * @param widget The GTK widget being destroyed.
 * @param data Unused user data pointer.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e on_window_destroy(GtkWidget *widget, gpointer data) {
  (void)widget;
  (void)data;
#ifndef CMN_TESTING
  gtk_main_quit();
#endif
  return CMN_SUCCESS;
}

/**
 * @brief GTK3 signal callback for button click events.
 * @param widget The GTK button widget.
 * @param data Pointer to `cmn_button_t`.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e on_button_clicked(GtkWidget *widget, gpointer data) {
  cmn_button_t *btn = (cmn_button_t *)data;
  (void)widget;
  if (btn && btn->on_click) {
    return btn->on_click(btn->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Creates a new window for the given application.
 * @param app The application instance.
 * @param out_window Pointer to store the created window.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_create(cmn_app_t *app, cmn_window_t **out_window) {
  cmn_error_e rc;
  cmn_window_t *win = NULL;

  if (app == NULL || out_window == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_window_t), (void **)&win);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&win->base, CMN_WIDGET_KIND_WINDOW,
                              destroy_window_adapter);

  win->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(win->window), 480, 360);
  gtk_container_set_border_width(GTK_CONTAINER(win->window), 0);
  {
    GtkWidget *hbar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(hbar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(hbar), "");
    gtk_window_set_titlebar(GTK_WINDOW(win->window), hbar);
  }
  g_signal_connect(win->window, "destroy", G_CALLBACK(on_window_destroy), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_linux_map_rc = (int)cmn_handle_map_add(win->window, win);

  *out_window = win;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the title of a window.
 * @param window The window to modify.
 * @param title The new title string (UTF-8).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_title(cmn_window_t *window, const char *title) {
  if (window == NULL || title == NULL) {
    return CMN_ERR_INVAL;
  }
  gtk_window_set_title(GTK_WINDOW(window->window), title);
  return CMN_SUCCESS;
}

/**
 * @brief Sets the style of a window (e.g., frameless, transparent).
 * @param window The window to modify.
 * @param style Bitmask of `cmn_window_style_e` flags.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_style(cmn_window_t *window,
                                 cmn_window_style_e style) {
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  if (style & CMN_WINDOW_STYLE_FRAMELESS) {
    gtk_window_set_decorated(GTK_WINDOW(window->window), FALSE);
  } else {
    gtk_window_set_decorated(GTK_WINDOW(window->window), TRUE);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the fullscreen state of a window.
 * @param window The window to modify.
 * @param is_fullscreen `true` (non-zero) for fullscreen, `false` (0) for
 * windowed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_fullscreen(cmn_window_t *window,
                                      cmn_bool_t is_fullscreen) {
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  if (is_fullscreen) {
    gtk_window_fullscreen(GTK_WINDOW(window->window));
  } else {
    gtk_window_unfullscreen(GTK_WINDOW(window->window));
  }
  return CMN_SUCCESS;
}

/**
 * @brief Maximizes a window.
 * @param window The window to maximize.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_maximize(cmn_window_t *window) {
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  gtk_window_maximize(GTK_WINDOW(window->window));
  return CMN_SUCCESS;
}

/**
 * @brief Minimizes a window.
 * @param window The window to minimize.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_minimize(cmn_window_t *window) {
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  gtk_window_iconify(GTK_WINDOW(window->window));
  return CMN_SUCCESS;
}

/**
 * @brief Gets the number of available displays/monitors.
 * @param out_count Pointer to store the display count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_display_get_count(int *out_count) {
  GdkDisplay *display;
  if (out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  display = gdk_display_get_default();
  if (display) {
    *out_count = gdk_display_get_n_monitors(display);
  } else {
    *out_count = 1;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Centers a window on a specific display.
 * @param window The window to center.
 * @param display_index The 0-based index of the display.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_center_on_display(cmn_window_t *window,
                                         int display_index) {
  int count = 1;
  cmn_error_e rc;
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_display_get_count(&count);
  if (rc != CMN_SUCCESS || display_index < 0 || display_index >= count) {
    return CMN_ERR_INVAL;
  }
  gtk_window_set_position(GTK_WINDOW(window->window), GTK_WIN_POS_CENTER);
  return CMN_SUCCESS;
}

/**
 * @brief Gets the DPI scale factor for a window.
 * @param window The window to query.
 * @param out_scale Pointer to store the DPI scale factor (e.g., 1.0f, 2.0f).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_get_dpi_scale(cmn_window_t *window, float *out_scale) {
  if (window == NULL || out_scale == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_scale = (float)gtk_widget_get_scale_factor(window->window);
  return CMN_SUCCESS;
}

/**
 * @brief Recursively attaches and positions child `GtkWidget` instances inside
 * a `GtkFixed` container according to the computed Flexbox (`cmn_flex_t`) or
 * Grid (`cmn_grid_t`) layout hierarchy.
 * @param fixed_container The target `GtkFixed` widget inside the window.
 * @param node Current layout container or leaf widget pointer.
 * @param offset_x Cumulative horizontal offset from window origin.
 * @param offset_y Cumulative vertical offset from window origin.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e linux_apply_layout_tree(GtkWidget *fixed_container,
                                           void *node, int offset_x,
                                           int offset_y) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  cmn_error_e rc;

  if (fixed_container == NULL || node == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_widget_get_kind(node, &kind);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  if (kind == CMN_WIDGET_KIND_FLEX) {
    cmn_flex_t *flex = (cmn_flex_t *)node;
    for (i = 0; i < flex->child_count; ++i) {
      cmn_rect_t r = flex->children[i].computed_rect;
      rc = linux_apply_layout_tree(fixed_container, flex->children[i].widget,
                                   offset_x + r.x, offset_y + r.y);
    }
    return rc;
  } else if (kind == CMN_WIDGET_KIND_GRID) {
    cmn_grid_t *grid = (cmn_grid_t *)node;
    for (i = 0; i < grid->child_count; ++i) {
      cmn_rect_t r = grid->children[i].computed_rect;
      rc = linux_apply_layout_tree(fixed_container, grid->children[i].widget,
                                   offset_x + r.x, offset_y + r.y);
    }
    return rc;
  } else {
    GtkWidget *c_widget = NULL;
    GtkWidget *old_parent = NULL;
    cmn_rect_t r = ((cmn_widget_header_t *)node)->bounds;
    int w = (r.width > 0) ? r.width : 200;
    int h = (r.height > 0) ? r.height
                           : (kind == CMN_WIDGET_KIND_TEXTVIEW ? 140 : 28);

    switch (kind) {
    case CMN_WIDGET_KIND_BUTTON:
      c_widget = ((cmn_button_t *)node)->button;
      break;
    case CMN_WIDGET_KIND_LABEL:
      c_widget = ((cmn_label_t *)node)->label;
      break;
    case CMN_WIDGET_KIND_TEXTBOX:
      c_widget = ((cmn_textbox_t *)node)->entry;
      gtk_editable_select_region(GTK_EDITABLE(c_widget), 0, 0);
      break;
    case CMN_WIDGET_KIND_TEXTVIEW:
      c_widget = ((cmn_textview_t *)node)->scrolled_window;
      break;
    default:
      break;
    }

    if (c_widget != NULL) {
      old_parent = gtk_widget_get_parent(c_widget);
      gtk_widget_set_size_request(c_widget, w, h);
      if (old_parent == fixed_container) {
        gtk_fixed_move(GTK_FIXED(fixed_container), c_widget, offset_x,
                       offset_y);
      } else {
        if (old_parent != NULL) {
          g_object_ref(c_widget);
          gtk_container_remove(GTK_CONTAINER(old_parent), c_widget);
          gtk_fixed_put(GTK_FIXED(fixed_container), c_widget, offset_x,
                        offset_y);
          g_object_unref(c_widget);
        } else {
          gtk_fixed_put(GTK_FIXED(fixed_container), c_widget, offset_x,
                        offset_y);
        }
      }
    }
  }
  return CMN_SUCCESS;
}

/**
 * @brief Shows a window on the screen.
 * @param window The window to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_show(cmn_window_t *window) {
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  if (window->base.layout_root != NULL) {
    int win_w =
        (window->base.bounds.width > 0) ? window->base.bounds.width : 480;
    int win_h =
        (window->base.bounds.height > 0) ? window->base.bounds.height : 360;
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(window->window));
    gtk_window_set_default_size(GTK_WINDOW(window->window), win_w, win_h);
    if (child == NULL || !GTK_IS_FIXED(child)) {
      if (child != NULL) {
        gtk_container_remove(GTK_CONTAINER(window->window), child);
      }
      child = gtk_fixed_new();
      gtk_container_add(GTK_CONTAINER(window->window), child);
    }
    gtk_widget_set_size_request(child, win_w, win_h);
    (void)linux_apply_layout_tree(child, window->base.layout_root, 0, 0);
  }
  gtk_widget_show_all(window->window);
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a window and frees its memory.
 * @param window Pointer to the window pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_destroy(cmn_window_t **window) {
  if (window == NULL || *window == NULL) {
    return CMN_ERR_INVAL;
  }
  g_linux_map_rc = (int)cmn_handle_map_remove((*window)->window);
  gtk_widget_destroy((*window)->window);
  return cmn_free((void **)window);
}

/**
 * @brief Shows a modal dialog attached to a parent window.
 * @param parent The parent window.
 * @param modal The modal window to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_dialog_show_modal(cmn_window_t *parent, cmn_window_t *modal) {
  if (parent == NULL || modal == NULL) {
    return CMN_ERR_INVAL;
  }
  gtk_window_set_transient_for(GTK_WINDOW(modal->window),
                               GTK_WINDOW(parent->window));
  gtk_window_set_modal(GTK_WINDOW(modal->window), TRUE);
  gtk_widget_show_all(modal->window);
  return CMN_SUCCESS;
}

/**
 * @brief Shows a native message box alert.
 * @param title The title of the message box.
 * @param msg The message body of the message box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_message_box_show(const char *title, const char *msg) {
  GtkWidget *dialog;
  if (title == NULL || msg == NULL) {
    return CMN_ERR_INVAL;
  }
  dialog = gtk_message_dialog_new(
      NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
      GTK_BUTTONS_OK, "%s", msg);
  gtk_window_set_title(GTK_WINDOW(dialog), title);
#ifndef CMN_TESTING
  gtk_dialog_run(GTK_DIALOG(dialog));
#endif
  gtk_widget_destroy(dialog);
  return CMN_SUCCESS;
}

/**
 * @brief Sets the parent container of a child widget using validated handles.
 * @param parent The parent container or window (`cmn_container_t *`,
 * `cmn_window_t *`, `cmn_vbox_t *`, or `cmn_hbox_t *`).
 * @param child The child widget (`cmn_widget_t *` or any concrete widget).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_set_parent(void *parent, void *child) {
  GtkWidget *p_widget = NULL;
  GtkWidget *c_widget = NULL;
  cmn_widget_kind_e parent_kind;
  cmn_widget_kind_e child_kind;
  cmn_error_e rc;

  if (parent == NULL || child == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_widget_get_kind(parent, &parent_kind);
  if (rc != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_widget_get_kind(child, &child_kind);
  if (rc != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }

  switch (parent_kind) {
  case CMN_WIDGET_KIND_WINDOW:
    p_widget = ((cmn_window_t *)parent)->window;
    break;
  case CMN_WIDGET_KIND_VBOX:
    p_widget = ((cmn_vbox_t *)parent)->box;
    break;
  case CMN_WIDGET_KIND_HBOX:
    p_widget = ((cmn_hbox_t *)parent)->box;
    break;
  case CMN_WIDGET_KIND_FLEX:
  case CMN_WIDGET_KIND_GRID:
    break;
  default:
    return CMN_ERR_INVAL;
  }

  switch (child_kind) {
  case CMN_WIDGET_KIND_BUTTON:
    c_widget = ((cmn_button_t *)child)->button;
    break;
  case CMN_WIDGET_KIND_LABEL:
    c_widget = ((cmn_label_t *)child)->label;
    break;
  case CMN_WIDGET_KIND_TEXTBOX:
    c_widget = ((cmn_textbox_t *)child)->entry;
    break;
  case CMN_WIDGET_KIND_TEXTVIEW:
    c_widget = ((cmn_textview_t *)child)->scrolled_window;
    break;
  case CMN_WIDGET_KIND_VBOX:
    c_widget = ((cmn_vbox_t *)child)->box;
    break;
  case CMN_WIDGET_KIND_HBOX:
    c_widget = ((cmn_hbox_t *)child)->box;
    break;
  case CMN_WIDGET_KIND_FLEX:
  case CMN_WIDGET_KIND_GRID:
    break;
  default:
    return CMN_ERR_INVAL;
  }

  if (p_widget == NULL || c_widget == NULL) {
    return CMN_SUCCESS;
  }
  if (gtk_widget_get_parent(c_widget) != NULL) {
    g_object_ref(c_widget);
    gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(c_widget)),
                         c_widget);
    if (GTK_IS_BOX(p_widget)) {
      gtk_box_pack_start(GTK_BOX(p_widget), c_widget, FALSE, FALSE, 0);
    } else if (GTK_IS_CONTAINER(p_widget)) {
      gtk_container_add(GTK_CONTAINER(p_widget), c_widget);
    }
    g_object_unref(c_widget);
  } else if (GTK_IS_BOX(p_widget)) {
    gtk_box_pack_start(GTK_BOX(p_widget), c_widget, FALSE, FALSE, 0);
  } else if (GTK_IS_CONTAINER(p_widget)) {
    gtk_container_add(GTK_CONTAINER(p_widget), c_widget);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Creates a new button widget.
 * @param out_button Pointer to store the created button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_create(cmn_button_t **out_button) {
  cmn_error_e rc;
  cmn_button_t *btn = NULL;

  if (out_button == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_button_t), (void **)&btn);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&btn->base, CMN_WIDGET_KIND_BUTTON,
                              destroy_button_adapter);

  (void)ensure_gtk_init();
  btn->button = gtk_button_new();
  g_signal_connect(btn->button, "clicked", G_CALLBACK(on_button_clicked), btn);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_linux_map_rc = (int)cmn_handle_map_add(btn->button, btn);

  *out_button = btn;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the text of a button.
 * @param button The button to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_set_text(cmn_button_t *button, const char *text) {
  if (button == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  gtk_button_set_label(GTK_BUTTON(button->button), text);
  return CMN_SUCCESS;
}

/**
 * @brief Sets the click callback for a button.
 * @param button The button to modify.
 * @param on_click The callback function.
 * @param userdata User data to pass to the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_set_on_click(cmn_button_t *button,
                                    cmn_button_click_fn on_click,
                                    void *userdata) {
  if (button == NULL || on_click == NULL) {
    return CMN_ERR_INVAL;
  }
  button->on_click = on_click;
  button->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers a button's click event.
 * @param button The button to click.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_trigger_click(cmn_button_t *button) {
  if (button == NULL) {
    return CMN_ERR_INVAL;
  }
  return on_button_clicked(button->button, button);
}

/**
 * @brief Destroys a button and frees its memory.
 * @param button Pointer to the button pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_destroy(cmn_button_t **button) {
  if (button == NULL || *button == NULL) {
    return CMN_ERR_INVAL;
  }
  g_linux_map_rc = (int)cmn_handle_map_remove((*button)->button);
  gtk_widget_destroy((*button)->button);
  return cmn_free((void **)button);
}

/**
 * @brief Creates a new label widget.
 * @param out_label Pointer to store the created label.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_create(cmn_label_t **out_label) {
  cmn_error_e rc;
  cmn_label_t *lbl = NULL;

  if (out_label == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_label_t), (void **)&lbl);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&lbl->base, CMN_WIDGET_KIND_LABEL,
                              destroy_label_adapter);

  (void)ensure_gtk_init();
  lbl->label = gtk_label_new("");
  gtk_widget_set_halign(lbl->label, GTK_ALIGN_START);
  gtk_label_set_xalign(GTK_LABEL(lbl->label), 0.0f);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_linux_map_rc = (int)cmn_handle_map_add(lbl->label, lbl);
  *out_label = lbl;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the text of a label.
 * @param label The label to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_set_text(cmn_label_t *label, const char *text) {
  if (label == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  gtk_label_set_text(GTK_LABEL(label->label), text);
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a label and frees its memory.
 * @param label Pointer to the label pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_destroy(cmn_label_t **label) {
  if (label == NULL || *label == NULL) {
    return CMN_ERR_INVAL;
  }
  g_linux_map_rc = (int)cmn_handle_map_remove((*label)->label);
  gtk_widget_destroy((*label)->label);
  return cmn_free((void **)label);
}

/**
 * @brief Creates a new text box widget.
 * @param out_textbox Pointer to store the created text box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_create(cmn_textbox_t **out_textbox) {
  cmn_error_e rc;
  cmn_textbox_t *tb = NULL;

  if (out_textbox == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_textbox_t), (void **)&tb);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&tb->base, CMN_WIDGET_KIND_TEXTBOX,
                              destroy_textbox_adapter);

  (void)ensure_gtk_init();
  tb->entry = gtk_entry_new();
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_linux_map_rc = (int)cmn_handle_map_add(tb->entry, tb);
  *out_textbox = tb;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the text of a text box.
 * @param textbox The text box to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_text(cmn_textbox_t *textbox, const char *text) {
  if (textbox == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  gtk_entry_set_text(GTK_ENTRY(textbox->entry), text);
  return CMN_SUCCESS;
}

/**
 * @brief Configures whether a text box masks its characters using
 * `gtk_entry_set_visibility`.
 * @param textbox The text box to modify.
 * @param is_password Non-zero (`cmn_true`) to enable password masking, `0`
 * (`cmn_false`) for plain text.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_password(cmn_textbox_t *textbox, int is_password) {
  if (textbox == NULL) {
    return CMN_ERR_INVAL;
  }
  gtk_entry_set_visibility(GTK_ENTRY(textbox->entry),
                           is_password ? FALSE : TRUE);
  return CMN_SUCCESS;
}

/**
 * @brief Gets the current text of a text box.
 * @param textbox The text box to query.
 * @param out_text Pointer to store the text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_get_text(cmn_textbox_t *textbox,
                                 const char **out_text) {
  const char *val;

  if (g_cmn_mock_textbox_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textbox == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }

  val = gtk_entry_get_text(GTK_ENTRY(textbox->entry));
  if (val) {
    cmn_error_e rc =
        cmn_strcpy_s(textbox->text_cache, sizeof(textbox->text_cache), val);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
  } else {
    textbox->text_cache[0] = '\0';
  }

  *out_text = textbox->text_cache;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a text box and frees its memory.
 * @param textbox Pointer to the text box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_destroy(cmn_textbox_t **textbox) {
  if (textbox == NULL || *textbox == NULL) {
    return CMN_ERR_INVAL;
  }
  g_linux_map_rc = (int)cmn_handle_map_remove((*textbox)->entry);
  gtk_widget_destroy((*textbox)->entry);
  return cmn_free((void **)textbox);
}

/* --- TextView --- */

/** @brief Mock flag to force `cmn_textview_get_text` failure. */
static int g_cmn_mock_textview_get_text_fail = 0;

/**
 * @brief Sets mock failure state for cmn_textview_get_text.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_textview_get_text_set(int fail) {
  g_cmn_mock_textview_get_text_fail = fail;
  return CMN_SUCCESS;
}

/**
 * @brief Creates a new multi-line scrollable text view widget.
 * @param out_textview Pointer to store the created text view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_create(cmn_textview_t **out_textview) {
  cmn_error_e rc;
  cmn_textview_t *tv = NULL;

  if (out_textview == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_textview_t), (void **)&tv);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&tv->base, CMN_WIDGET_KIND_TEXTVIEW,
                              destroy_textview_adapter);

  (void)ensure_gtk_init();
  tv->scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tv->scrolled_window),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(tv->scrolled_window),
                                      GTK_SHADOW_ETCHED_IN);
  gtk_widget_set_size_request(tv->scrolled_window, 400, 140);

  tv->textview = gtk_text_view_new();
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(tv->textview), 6);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(tv->textview), 6);
  gtk_text_view_set_top_margin(GTK_TEXT_VIEW(tv->textview), 4);
  gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(tv->textview), 4);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(tv->textview), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(tv->textview), FALSE);

  gtk_container_add(GTK_CONTAINER(tv->scrolled_window), tv->textview);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_linux_map_rc = (int)cmn_handle_map_add(tv->scrolled_window, tv);

  *out_textview = tv;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the entire text of a text view.
 * @param textview The text view to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_set_text(cmn_textview_t *textview, const char *text) {
  GtkTextBuffer *buffer;
  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview->textview));
  gtk_text_buffer_set_text(buffer, text, -1);
  return CMN_SUCCESS;
}

/**
 * @brief Appends text to the end of a text view and scrolls to the bottom.
 * @param textview The text view to modify.
 * @param text The text string to append.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_append_text(cmn_textview_t *textview,
                                     const char *text) {
  GtkTextBuffer *buffer;
  GtkTextIter end_iter;
  GtkTextMark *mark;

  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview->textview));
  gtk_text_buffer_get_end_iter(buffer, &end_iter);
  gtk_text_buffer_insert(buffer, &end_iter, text, -1);

  mark = gtk_text_buffer_get_insert(buffer);
  gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(textview->textview), mark, 0.0,
                               TRUE, 0.0, 1.0);

  return CMN_SUCCESS;
}

/**
 * @brief Gets the current text of a text view.
 * @param textview The text view to query.
 * @param out_text Pointer to store the text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_get_text(cmn_textview_t *textview,
                                  const char **out_text) {
  GtkTextBuffer *buffer;
  GtkTextIter start, end;
  char *val;
  cmn_error_e rc;

  if (g_cmn_mock_textview_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textview == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview->textview));
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter(buffer, &end);
  val = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

  if (val) {
    rc = cmn_strcpy_s(textview->text_cache, sizeof(textview->text_cache), val);
    g_free(val);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
  } else {
    textview->text_cache[0] = '\0';
  }

  *out_text = textview->text_cache;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a text view and frees its memory.
 * @param textview Pointer to the text view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_destroy(cmn_textview_t **textview) {
  if (textview == NULL || *textview == NULL) {
    return CMN_ERR_INVAL;
  }
  g_linux_map_rc = (int)cmn_handle_map_remove((*textview)->scrolled_window);
  gtk_widget_destroy((*textview)->scrolled_window);
  return cmn_free((void **)textview);
}

/**
 * @brief Creates a vertical box layout container.
 * @param out_vbox Pointer to store the created vertical box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_vbox_create(cmn_vbox_t **out_vbox) {
  cmn_error_e rc;
  cmn_vbox_t *vbox = NULL;

  if (out_vbox == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_vbox_t), (void **)&vbox);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&vbox->base, CMN_WIDGET_KIND_VBOX,
                              destroy_vbox_adapter);

  (void)ensure_gtk_init();
  vbox->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_linux_map_rc = (int)cmn_handle_map_add(vbox->box, vbox);
  *out_vbox = vbox;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a vertical box layout container.
 * @param vbox Pointer to the vertical box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_vbox_destroy(cmn_vbox_t **vbox) {
  if (vbox == NULL || *vbox == NULL) {
    return CMN_ERR_INVAL;
  }
  g_linux_map_rc = (int)cmn_handle_map_remove((*vbox)->box);
  gtk_widget_destroy((*vbox)->box);
  return cmn_free((void **)vbox);
}

/**
 * @brief Creates a horizontal box layout container.
 * @param out_hbox Pointer to store the created horizontal box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_hbox_create(cmn_hbox_t **out_hbox) {
  cmn_error_e rc;
  cmn_hbox_t *hbox = NULL;

  if (out_hbox == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_hbox_t), (void **)&hbox);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&hbox->base, CMN_WIDGET_KIND_HBOX,
                              destroy_hbox_adapter);

  (void)ensure_gtk_init();
  hbox->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_linux_map_rc = (int)cmn_handle_map_add(hbox->box, hbox);
  *out_hbox = hbox;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a horizontal box layout container.
 * @param hbox Pointer to the horizontal box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_hbox_destroy(cmn_hbox_t **hbox) {
  if (hbox == NULL || *hbox == NULL) {
    return CMN_ERR_INVAL;
  }
  g_linux_map_rc = (int)cmn_handle_map_remove((*hbox)->box);
  gtk_widget_destroy((*hbox)->box);
  return cmn_free((void **)hbox);
}

/**
 * @brief Captures a screenshot of a window and saves it to a file.
 * @param win The window to capture.
 * @param out_filepath The destination file path for the screenshot.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_capture_screenshot(cmn_window_t *win,
                                              const char *out_filepath) {
  GdkPixbuf *pixbuf = NULL;
  gboolean res;
  GtkWidget *offscreen;
  GtkWidget *vbox_frame;
  GtkWidget *hbar;
  GtkWidget *child;
  const gchar *title_str;
  int iter;

  if (win == NULL || out_filepath == NULL || out_filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  title_str = gtk_window_get_title(GTK_WINDOW(win->window));
  child = gtk_bin_get_child(GTK_BIN(win->window));

  offscreen = gtk_offscreen_window_new();
  gtk_window_set_default_size(
      GTK_WINDOW(offscreen),
      (win->base.bounds.width > 0) ? win->base.bounds.width : 480,
      (win->base.bounds.height > 0) ? win->base.bounds.height : 360);

  vbox_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbar = gtk_header_bar_new();
  gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(hbar), TRUE);
  gtk_header_bar_set_title(GTK_HEADER_BAR(hbar), title_str ? title_str : "");
  gtk_box_pack_start(GTK_BOX(vbox_frame), hbar, FALSE, FALSE, 0);

  if (child != NULL) {
    g_object_ref(child);
    gtk_container_remove(GTK_CONTAINER(win->window), child);
    if (!GTK_IS_FIXED(child)) {
      gtk_widget_set_margin_start(child, 16);
      gtk_widget_set_margin_end(child, 16);
      gtk_widget_set_margin_top(child, 16);
      gtk_widget_set_margin_bottom(child, 16);
    }
    gtk_box_pack_start(GTK_BOX(vbox_frame), child, TRUE, TRUE, 0);
  }

  gtk_container_add(GTK_CONTAINER(offscreen), vbox_frame);
  gtk_widget_show_all(offscreen);
  gtk_window_set_focus(GTK_WINDOW(offscreen), NULL);
  if (child != NULL && GTK_IS_CONTAINER(child)) {
    GList *kids = gtk_container_get_children(GTK_CONTAINER(child));
    GList *it;
    for (it = kids; it != NULL; it = it->next) {
      if (GTK_IS_EDITABLE(it->data)) {
        gtk_editable_select_region(GTK_EDITABLE(it->data), 0, 0);
      }
    }
    g_list_free(kids);
  }

  for (iter = 0; iter < 200 && gtk_events_pending(); ++iter) {
    gtk_main_iteration_do(FALSE);
  }
  gtk_window_set_focus(GTK_WINDOW(offscreen), NULL);

  pixbuf = gtk_offscreen_window_get_pixbuf(GTK_OFFSCREEN_WINDOW(offscreen));

  if (child != NULL) {
    gtk_container_remove(GTK_CONTAINER(vbox_frame), child);
    gtk_container_add(GTK_CONTAINER(win->window), child);
    g_object_unref(child);
  }
  gtk_widget_destroy(offscreen);

  if (!pixbuf) {
    return CMN_ERR_IO;
  }
  res = gdk_pixbuf_save(pixbuf, out_filepath, "png", NULL, NULL);
  g_object_unref(pixbuf);
  return res ? CMN_SUCCESS : CMN_ERR_IO;
}

/* --- Menus --- */

/**
 * @brief Creates a new menu.
 * @param out_menu Pointer to store the created menu.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_create(cmn_menu_t **out_menu) {
  cmn_error_e rc;
  cmn_menu_t *m = NULL;
  if (out_menu == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_menu_t), (void **)&m);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&m->base, CMN_WIDGET_KIND_MENU,
                              destroy_menu_adapter);
  (void)ensure_gtk_init();
  m->gtk_menu = gtk_menu_new();
  m->gtk_menu_bar = gtk_menu_bar_new();
  *out_menu = m;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a menu and frees its memory.
 * @param menu Pointer to the menu pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_destroy(cmn_menu_t **menu) {
  if (menu == NULL || *menu == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)menu);
}

/**
 * @brief GTK3 menu item activation callback adapter.
 * @param item The activated GTK menu item.
 * @param user_data Pointer to `struct linux_menu_item_cb`.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e linux_menu_item_activate(GtkMenuItem *item,
                                            gpointer user_data) {
  struct linux_menu_item_cb *cb = (struct linux_menu_item_cb *)user_data;
  (void)item;
  if (cb && cb->on_click) {
    return cb->on_click(cb->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Adds an item to a menu.
 * @param menu The menu to add the item to.
 * @param title The text of the menu item.
 * @param on_click The callback function when the item is clicked.
 * @param userdata User data passed to the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_add_item(cmn_menu_t *menu, const char *title,
                              cmn_error_e (*on_click)(void *), void *userdata) {
  GtkWidget *item;
  if (menu == NULL || title == NULL) {
    return CMN_ERR_INVAL;
  }
  if (menu->item_count >= CMN_LINUX_MAX_MENU_ITEMS) {
    return CMN_ERR_NOMEM;
  }

  menu->items[menu->item_count].on_click = on_click;
  menu->items[menu->item_count].userdata = userdata;

  item = gtk_menu_item_new_with_label(title);
  if (on_click) {
    g_signal_connect(item, "activate", G_CALLBACK(linux_menu_item_activate),
                     &menu->items[menu->item_count]);
  }
  menu->item_count++;

  gtk_menu_shell_append(GTK_MENU_SHELL(menu->gtk_menu), item);
  gtk_widget_show(item);
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers a menu item's callback by index.
 * @param menu The menu containing the item.
 * @param index The 0-based index of the menu item to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_trigger_item(cmn_menu_t *menu, int index) {
  if (menu == NULL || index < 0 || index >= menu->item_count) {
    return CMN_ERR_INVAL;
  }
  if (menu->items[index].on_click != NULL) {
    return menu->items[index].on_click(menu->items[index].userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the main menu bar for a window.
 * @param window The window to set the menu on.
 * @param menu The menu to attach.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_menu(cmn_window_t *window, cmn_menu_t *menu) {
  (void)window;
  if (menu == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/* --- File Picker --- */

/**
 * @brief Creates a new file picker dialog.
 * @param out_filepicker Pointer to store the created file picker.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_create(cmn_filepicker_t **out_filepicker) {
  cmn_error_e rc;
  cmn_filepicker_t *fp = NULL;
  if (out_filepicker == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_filepicker_t), (void **)&fp);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = cmn_widget_header_init(&fp->base, CMN_WIDGET_KIND_FILEPICKER,
                              destroy_filepicker_adapter);
  fp->path = NULL;
  *out_filepicker = fp;
  return CMN_SUCCESS;
}

/** @brief Mock flag to bypass `cmn_filepicker_show_dialog`. */
static int g_cmn_mock_filepicker_show_dialog_bypass = 0;

/**
 * @brief Sets mock bypass state for cmn_filepicker_show_dialog.
 * @param bypass 1 to bypass, 0 for normal behavior.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_filepicker_show_dialog_set(int bypass) {
  g_cmn_mock_filepicker_show_dialog_bypass = bypass;
  return CMN_SUCCESS;
}

/**
 * @brief Shows the file picker dialog and waits for user selection.
 * @param filepicker The file picker to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_show_dialog(cmn_filepicker_t *filepicker) {
  GtkWidget *dialog;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;

  if (filepicker == NULL) {
    return CMN_ERR_INVAL;
  }

  if (g_cmn_mock_filepicker_show_dialog_bypass) {
    return CMN_SUCCESS;
  }

  dialog = gtk_file_chooser_dialog_new("Open File", NULL, action, "_Cancel",
                                       GTK_RESPONSE_CANCEL, "_Open",
                                       GTK_RESPONSE_ACCEPT, NULL);

#ifdef CMN_TESTING
  (void)res;
  if (filepicker->path) {
    g_free(filepicker->path);
  }
  filepicker->path = g_strdup("/tmp/test");
#else
  res = gtk_dialog_run(GTK_DIALOG(dialog));
  if (res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    if (filepicker->path) {
      g_free(filepicker->path);
    }
    filepicker->path = gtk_file_chooser_get_filename(chooser);
  }
#endif

  gtk_widget_destroy(dialog);
  return CMN_SUCCESS;
}

/**
 * @brief Gets the selected file path from the file picker.
 * @param filepicker The file picker to query.
 * @param out_path Pointer to store the selected file path string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_get_path(cmn_filepicker_t *filepicker,
                                    const char **out_path) {
  if (filepicker == NULL || out_path == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_path = filepicker->path ? filepicker->path : "";
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a file picker and frees its memory.
 * @param filepicker Pointer to the file picker pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_destroy(cmn_filepicker_t **filepicker) {
  if (filepicker == NULL || *filepicker == NULL) {
    return CMN_ERR_INVAL;
  }
  if ((*filepicker)->path) {
    g_free((*filepicker)->path);
  }
  return cmn_free((void **)filepicker);
}

/* --- Accessibility (a11y) Overrides --- */

/**
 * @brief Sets the accessibility label (screen reader description) for a widget.
 * @param widget The widget to modify.
 * @param label The accessibility label string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_label(void *widget, const char *label) {
  GtkWidget *gtk_widget = NULL;
  cmn_widget_kind_e kind;
  if (widget == NULL || label == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }

  switch (kind) {
  case CMN_WIDGET_KIND_WINDOW:
    gtk_widget = ((cmn_window_t *)widget)->window;
    break;
  case CMN_WIDGET_KIND_BUTTON:
    gtk_widget = ((cmn_button_t *)widget)->button;
    break;
  case CMN_WIDGET_KIND_LABEL:
    gtk_widget = ((cmn_label_t *)widget)->label;
    break;
  case CMN_WIDGET_KIND_TEXTBOX:
    gtk_widget = ((cmn_textbox_t *)widget)->entry;
    break;
  case CMN_WIDGET_KIND_TEXTVIEW:
    gtk_widget = ((cmn_textview_t *)widget)->textview;
    break;
  case CMN_WIDGET_KIND_VBOX:
    gtk_widget = ((cmn_vbox_t *)widget)->box;
    break;
  case CMN_WIDGET_KIND_HBOX:
    gtk_widget = ((cmn_hbox_t *)widget)->box;
    break;
  default:
    return CMN_ERR_INVAL;
  }

  if (GTK_IS_WIDGET(gtk_widget)) {
    atk_object_set_name(gtk_widget_get_accessible(gtk_widget), label);
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
  cmn_widget_kind_e kind;
  if (widget == NULL || role < CMN_A11Y_ROLE_BUTTON ||
      role > CMN_A11Y_ROLE_DIALOG) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER) {
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
  (void)index;
  if (widget == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}
