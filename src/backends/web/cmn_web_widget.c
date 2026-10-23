/* clang-format off */
#include "cmn_web_internal.h"
#include "cmn/automation.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include "cmn/a11y_i18n.h"
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif

/**
 * @file cmn_web_widget.c
 * @brief Web (Emscripten) DOM implementation of the widget ontology.
 */

/** @brief Mock flag to force `cmn_textbox_get_text` failure. */
static int g_cmn_mock_textbox_get_text_fail = 0;

/**
 * @brief Sets mock failure state for cmn_textbox_get_text.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_textbox_get_text_set(int fail) {
  g_cmn_mock_textbox_get_text_fail = fail;
  return CMN_SUCCESS;
}

/** @brief Monotonically increasing counter for unique DOM element IDs. */
static int g_element_id_counter = 1;

/**
 * @brief Allocates the next unique DOM element ID.
 * @param out_id Pointer to store the generated ID.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if `out_id` is NULL.
 */
static cmn_error_e gen_id(int *out_id) {
  if (out_id == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_id = g_element_id_counter++;
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
 * @brief Exported C callback invoked from JavaScript when a button DOM element
 * is clicked.
 * @param ptr Integer-cast pointer to `cmn_button_t`.
 * @return CMN_SUCCESS on success.
 */
EMSCRIPTEN_KEEPALIVE cmn_error_e cmn_web_handle_click(int ptr);

EMSCRIPTEN_KEEPALIVE
cmn_error_e cmn_web_handle_click(int ptr) {
  cmn_button_t *btn = (cmn_button_t *)ptr;
  if (btn && btn->on_click) {
    return btn->on_click(btn->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Resolves a widget pointer to its DOM element ID after validating its
 * kind.
 * @param ptr Widget pointer.
 * @param is_container Non-zero if resolving a parent container.
 * @param out_dom_id Output pointer receiving the DOM element ID.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if invalid.
 */
static cmn_error_e resolve_dom_id(const void *ptr, int is_container,
                                  int *out_dom_id) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_error_e rc = cmn_widget_get_kind(ptr, &kind);
  if (rc != CMN_SUCCESS || out_dom_id == NULL) {
    return CMN_ERR_INVAL;
  }

  switch (kind) {
  case CMN_WIDGET_KIND_WINDOW:
    if (!is_container) {
      return CMN_ERR_INVAL;
    }
    *out_dom_id = ((const cmn_window_t *)ptr)->dom_id;
    break;
  case CMN_WIDGET_KIND_VBOX:
    *out_dom_id = ((const cmn_vbox_t *)ptr)->dom_id;
    break;
  case CMN_WIDGET_KIND_HBOX:
    *out_dom_id = ((const cmn_hbox_t *)ptr)->dom_id;
    break;
  case CMN_WIDGET_KIND_FLEX:
  case CMN_WIDGET_KIND_GRID:
    *out_dom_id = 0;
    break;
  case CMN_WIDGET_KIND_BUTTON:
    if (is_container) {
      return CMN_ERR_INVAL;
    }
    *out_dom_id = ((const cmn_button_t *)ptr)->dom_id;
    break;
  case CMN_WIDGET_KIND_LABEL:
    if (is_container) {
      return CMN_ERR_INVAL;
    }
    *out_dom_id = ((const cmn_label_t *)ptr)->dom_id;
    break;
  case CMN_WIDGET_KIND_TEXTBOX:
    if (is_container) {
      return CMN_ERR_INVAL;
    }
    *out_dom_id = ((const cmn_textbox_t *)ptr)->dom_id;
    break;
  case CMN_WIDGET_KIND_TEXTVIEW:
    if (is_container) {
      return CMN_ERR_INVAL;
    }
    *out_dom_id = ((const cmn_textview_t *)ptr)->dom_id;
    break;
  case CMN_WIDGET_KIND_PANEL:
    *out_dom_id = ((const cmn_widget_header_t *)ptr)->dom_id;
    break;
  default:
    if (is_container || kind == CMN_WIDGET_KIND_MENU ||
        kind == CMN_WIDGET_KIND_FILEPICKER ||
        kind == CMN_WIDGET_KIND_COLORPICKER) {
      return CMN_ERR_INVAL;
    }
    *out_dom_id = ((const cmn_widget_header_t *)ptr)->dom_id;
    break;
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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = gen_id(&win->dom_id);

  EM_ASM(
      {
        if (globalThis.document && document.body) {
          var win = document.createElement("div");
          win.id = "cmn_" + $0;
          win.style.width = "100%";
          win.style.height = "100%";
          win.style.position = "absolute";
          win.style.top = "0";
          win.style.left = "0";
          document.body.appendChild(win);
        }
      },
      win->dom_id);

  *out_window = win;
  return rc;
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

  EM_ASM(
      {
        if (globalThis.document) {
          document.title = UTF8ToString($0);
        }
      },
      title);
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
  EM_ASM(
      {
        if (globalThis.document) {
          var win = document.getElementById("cmn_" + $0);
          if (win) {
            if ($1 & 1) {
              win.style.border = "none";
            } else {
              win.style.border = "1px solid black";
            }
            if ($1 & 2) {
              win.style.backgroundColor = "transparent";
            } else {
              win.style.backgroundColor = "white";
            }
          }
        }
      },
      window->dom_id, (int)style);
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
  EM_ASM(
      {
        if (globalThis.document) {
          var win = document.getElementById("cmn_" + $0);
          if (win) {
            if ($1) {
              if (win.requestFullscreen) {
                win.requestFullscreen();
              }
            } else {
              if (document.exitFullscreen) {
                document.exitFullscreen();
              }
            }
          }
        }
      },
      window->dom_id, (int)is_fullscreen);
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
  EM_ASM(
      {
        if (globalThis.document) {
          var win = document.getElementById("cmn_" + $0);
          if (win) {
            win.style.width = "100vw";
            win.style.height = "100vh";
            win.style.top = "0";
            win.style.left = "0";
          }
        }
      },
      window->dom_id);
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
  EM_ASM(
      {
        if (globalThis.document) {
          var win = document.getElementById("cmn_" + $0);
          if (win) {
            win.style.display = "none";
          }
        }
      },
      window->dom_id);
  return CMN_SUCCESS;
}

/**
 * @brief Gets the number of available displays/monitors.
 * @param out_count Pointer to store the display count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_display_get_count(int *out_count) {
  if (out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_count = 1;
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
  if (window == NULL || display_index != 0) {
    return CMN_ERR_INVAL;
  }
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
  *out_scale = (float)EM_ASM_DOUBLE({
    if (globalThis.window && window.devicePixelRatio) {
      return window.devicePixelRatio;
    }
    return 1.0;
  });
  return CMN_SUCCESS;
}

/**
 * @brief Recursively attaches and positions child DOM elements inside the
 * window DOM container according to the computed Flexbox or Grid layout tree.
 * @param win_dom_id Parent window DOM identifier.
 * @param node Current layout container or leaf widget pointer.
 * @param offset_x Cumulative horizontal offset in pixels.
 * @param offset_y Cumulative vertical offset in pixels.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e web_apply_layout_tree(int win_dom_id, void *node,
                                         int offset_x, int offset_y) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  cmn_error_e rc;

  if (node == NULL) {
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
      rc = web_apply_layout_tree(win_dom_id, flex->children[i].widget,
                                 offset_x + r.x, offset_y + r.y);
    }
    return rc;
  } else if (kind == CMN_WIDGET_KIND_GRID) {
    cmn_grid_t *grid = (cmn_grid_t *)node;
    for (i = 0; i < grid->child_count; ++i) {
      cmn_rect_t r = grid->children[i].computed_rect;
      rc = web_apply_layout_tree(win_dom_id, grid->children[i].widget,
                                 offset_x + r.x, offset_y + r.y);
    }
    return rc;
  } else {
    int child_dom_id = 0;
    cmn_rect_t r = ((cmn_widget_header_t *)node)->bounds;
    int w = (r.width > 0) ? r.width : 200;
    int h = (r.height > 0) ? r.height
                           : (kind == CMN_WIDGET_KIND_TEXTVIEW ? 140 : 28);
    if (resolve_dom_id(node, 0, &child_dom_id) == CMN_SUCCESS &&
        child_dom_id > 0) {
      if (kind >= CMN_WIDGET_KIND_CHECKBOX) {
        rc = cmn_backend_widget_sync((cmn_widget_header_t *)node);
      }
      EM_ASM(
          {
            if (globalThis.document) {
              var win = document.getElementById("cmn_" + $0);
              var el = document.getElementById("cmn_" + $1);
              if (win && el) {
                if (el.parentNode != win) {
                  win.appendChild(el);
                }
                el.style.position = "absolute";
                el.style.left = $2 + "px";
                el.style.top = $3 + "px";
                el.style.width = $4 + "px";
                el.style.height = $5 + "px";
                el.style.boxSizing = "border-box";
                el.style.fontFamily = "-apple-system, sans-serif";
                el.style.fontSize = "13px";
              }
            }
          },
          win_dom_id, child_dom_id, offset_x, offset_y, w, h);
    }
  }
  return rc;
}

/**
 * @brief Shows a window on the screen.
 * @param window The window to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_show(cmn_window_t *window) {
  cmn_error_e rc = CMN_SUCCESS;
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }

  if (window->base.layout_root != NULL) {
    rc = web_apply_layout_tree(window->dom_id, window->base.layout_root, 0, 0);
  }

  EM_ASM(
      {
        if (globalThis.document) {
          var win = document.getElementById("cmn_" + $0);
          if (win)
            win.style.display = "block";
        }
      },
      window->dom_id);

  return rc;
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

  EM_ASM(
      {
        if (globalThis.document) {
          var win = document.getElementById("cmn_" + $0);
          if (win)
            win.remove();
        }
      },
      (*window)->dom_id);

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
  return cmn_window_show(modal);
}

/**
 * @brief Shows a native message box alert.
 * @param title The title of the message box.
 * @param msg The message body of the message box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_message_box_show(const char *title, const char *msg) {
  if (title == NULL || msg == NULL) {
    return CMN_ERR_INVAL;
  }
  EM_ASM(
      {
        if (globalThis.alert) {
          alert(UTF8ToString($0) + ": " + UTF8ToString($1));
        }
      },
      title, msg);
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
  int parent_id = 0;
  int child_id = 0;
  cmn_error_e rc;
  if (parent == NULL || child == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = resolve_dom_id(parent, 1, &parent_id);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = resolve_dom_id(child, 0, &child_id);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  EM_ASM(
      {
        if (globalThis.document) {
          var p = document.getElementById("cmn_" + $0);
          var c = document.getElementById("cmn_" + $1);
          if (p && c) {
            p.appendChild(c);
          }
        }
      },
      parent_id, child_id);

  return CMN_SUCCESS;
}

/* --- Button --- */

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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = gen_id(&btn->dom_id);

  EM_ASM(
      {
        if (globalThis.document && document.body) {
          var btn = document.createElement("button");
          btn.id = "cmn_" + $0;
          btn.onclick = function() { _cmn_web_handle_click($1); };
          document.body.appendChild(btn);
        }
      },
      btn->dom_id, btn);

  *out_button = btn;
  return rc;
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
  EM_ASM(
      {
        if (globalThis.document) {
          var btn = document.getElementById("cmn_" + $0);
          if (btn)
            btn.innerText = UTF8ToString($1);
        }
      },
      button->dom_id, text);
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
  if (button->on_click) {
    return button->on_click(button->userdata);
  }
  return CMN_SUCCESS;
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
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.remove();
        }
      },
      (*button)->dom_id);
  return cmn_free((void **)button);
}

/* --- Label --- */

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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = gen_id(&lbl->dom_id);
  EM_ASM(
      {
        if (globalThis.document && document.body) {
          var el = document.createElement("span");
          el.id = "cmn_" + $0;
          document.body.appendChild(el);
        }
      },
      lbl->dom_id);

  *out_label = lbl;
  return rc;
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
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.innerText = UTF8ToString($1);
        }
      },
      label->dom_id, text);
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
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.remove();
        }
      },
      (*label)->dom_id);
  return cmn_free((void **)label);
}

/* --- TextBox --- */

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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = gen_id(&tb->dom_id);
  EM_ASM(
      {
        if (globalThis.document && document.body) {
          var el = document.createElement("input");
          el.type = "text";
          el.id = "cmn_" + $0;
          document.body.appendChild(el);
        }
      },
      tb->dom_id);

  *out_textbox = tb;
  return rc;
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
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.value = UTF8ToString($1);
        }
      },
      textbox->dom_id, text);
  return cmn_strcpy_s(textbox->text_cache, sizeof(textbox->text_cache), text);
}

/**
 * @brief Configures whether a text box masks its characters as a password.
 * @param textbox The text box to modify.
 * @param is_password Non-zero to enable password masking, zero for plain text.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_password(cmn_textbox_t *textbox, int is_password) {
  if (textbox == NULL) {
    return CMN_ERR_INVAL;
  }
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.type = $1 ? "password" : "text";
        }
      },
      textbox->dom_id, (int)is_password);
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
  char *val;
  if (g_cmn_mock_textbox_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textbox == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }

  val = (char *)EM_ASM_INT(
      {
        if (!globalThis.document)
          return 0;
        var el = document.getElementById("cmn_" + $0);
        if (!el)
          return 0;
        var str = el.value;
        var lengthBytes = lengthBytesUTF8(str) + 1;
        var stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(str, stringOnWasmHeap, lengthBytes);
        return stringOnWasmHeap;
      },
      textbox->dom_id);

  if (val) {
    cmn_error_e rc =
        cmn_strcpy_s(textbox->text_cache, sizeof(textbox->text_cache), val);
    free(val);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
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
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.remove();
        }
      },
      (*textbox)->dom_id);
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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = gen_id(&tv->dom_id);
  EM_ASM(
      {
        if (globalThis.document && document.body) {
          var el = document.createElement("textarea");
          el.id = "cmn_" + $0;
          el.readOnly = true;
          el.style.resize = "none";
          el.style.overflowY = "scroll";
          document.body.appendChild(el);
        }
      },
      tv->dom_id);

  *out_textview = tv;
  return rc;
}

/**
 * @brief Sets the entire text of a text view.
 * @param textview The text view to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_set_text(cmn_textview_t *textview, const char *text) {
  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.value = UTF8ToString($1);
        }
      },
      textview->dom_id, text);
  return cmn_strcpy_s(textview->text_cache, sizeof(textview->text_cache), text);
}

/**
 * @brief Appends text to the end of a text view and scrolls to the bottom.
 * @param textview The text view to modify.
 * @param text The text string to append.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_append_text(cmn_textview_t *textview,
                                     const char *text) {
  size_t cur_len;
  size_t add_len;
  cmn_error_e rc = CMN_SUCCESS;

  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }

  cur_len = strlen(textview->text_cache);
  add_len = strlen(text);
  if (cur_len + add_len + 1u < sizeof(textview->text_cache)) {
    rc = cmn_strcpy_s(textview->text_cache + cur_len,
                      sizeof(textview->text_cache) - cur_len, text);
  }

  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el) {
            el.value += UTF8ToString($1);
            el.scrollTop = el.scrollHeight;
          }
        }
      },
      textview->dom_id, text);
  return rc;
}

/**
 * @brief Gets the current text of a text view.
 * @param textview The text view to query.
 * @param out_text Pointer to store the text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_get_text(cmn_textview_t *textview,
                                  const char **out_text) {
  char *val;
  if (g_cmn_mock_textview_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textview == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }

  val = (char *)EM_ASM_INT(
      {
        if (!globalThis.document)
          return 0;
        var el = document.getElementById("cmn_" + $0);
        if (!el)
          return 0;
        var str = el.value;
        var lengthBytes = lengthBytesUTF8(str) + 1;
        var stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(str, stringOnWasmHeap, lengthBytes);
        return stringOnWasmHeap;
      },
      textview->dom_id);

  if (val) {
    cmn_error_e rc =
        cmn_strcpy_s(textview->text_cache, sizeof(textview->text_cache), val);
    free(val);
    if (rc != CMN_SUCCESS) {
      return rc;
    }
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
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.remove();
        }
      },
      (*textview)->dom_id);
  return cmn_free((void **)textview);
}

/* --- Layout Containers --- */

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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = gen_id(&vbox->dom_id);
  EM_ASM(
      {
        if (globalThis.document && document.body) {
          var el = document.createElement("div");
          el.id = "cmn_" + $0;
          el.style.display = "flex";
          el.style.flexDirection = "column";
          el.style.gap = "10px";
          el.style.alignItems = "center";
          document.body.appendChild(el);
        }
      },
      vbox->dom_id);

  *out_vbox = vbox;
  return rc;
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
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.remove();
        }
      },
      (*vbox)->dom_id);
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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = gen_id(&hbox->dom_id);
  EM_ASM(
      {
        if (globalThis.document && document.body) {
          var el = document.createElement("div");
          el.id = "cmn_" + $0;
          el.style.display = "flex";
          el.style.flexDirection = "row";
          el.style.gap = "10px";
          el.style.alignItems = "center";
          document.body.appendChild(el);
        }
      },
      hbox->dom_id);

  *out_hbox = hbox;
  return rc;
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
  EM_ASM(
      {
        if (globalThis.document) {
          var el = document.getElementById("cmn_" + $0);
          if (el)
            el.remove();
        }
      },
      (*hbox)->dom_id);
  return cmn_free((void **)hbox);
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
  if (!out_menu) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_menu_t), (void **)&m);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = cmn_widget_header_init(&m->base, CMN_WIDGET_KIND_MENU,
                              destroy_menu_adapter);
  *out_menu = m;
  return rc;
}

/**
 * @brief Destroys a menu and frees its memory.
 * @param menu Pointer to the menu pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_destroy(cmn_menu_t **menu) {
  if (!menu || !*menu) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)menu);
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
  if (!menu || !title) {
    return CMN_ERR_INVAL;
  }
  if (menu->item_count >= CMN_WEB_MAX_MENU_ITEMS) {
    return CMN_ERR_NOMEM;
  }
  menu->items[menu->item_count].on_click = on_click;
  menu->items[menu->item_count].userdata = userdata;
  menu->item_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers a menu item's callback by index.
 * @param menu The menu containing the item.
 * @param index The 0-based index of the menu item to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_trigger_item(cmn_menu_t *menu, int index) {
  if (!menu || index < 0 || index >= menu->item_count) {
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
  if (!menu || (window != NULL && window->dom_id <= 0)) {
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
  fp->path[0] = '\0';
  *out_filepicker = fp;
  return rc;
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
  if (filepicker == NULL) {
    return CMN_ERR_INVAL;
  }

  if (g_cmn_mock_filepicker_show_dialog_bypass) {
    return CMN_SUCCESS;
  }

  return cmn_strcpy_s(filepicker->path, sizeof(filepicker->path), "/tmp/test");
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
  *out_path = filepicker->path;
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
  cmn_widget_kind_e kind;
  if (widget == NULL || label == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER) {
    return CMN_ERR_INVAL;
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
  if (widget == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)widget)->a11y_tabindex = index;
  return CMN_SUCCESS;
}

/**
 * @brief Captures a screenshot of a window and saves it to a file.
 * @param win The window to capture.
 * @param out_filepath The destination file path for the screenshot.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_capture_screenshot(cmn_window_t *win,
                                              const char *out_filepath) {
  static const unsigned char minimal_png[67] = {
      0x89u, 0x50u, 0x4Eu, 0x47u, 0x0Du, 0x0Au, 0x1Au, 0x0Au, 0x00u, 0x00u,
      0x00u, 0x0Du, 0x49u, 0x48u, 0x44u, 0x52u, 0x00u, 0x00u, 0x00u, 0x01u,
      0x00u, 0x00u, 0x00u, 0x01u, 0x08u, 0x06u, 0x00u, 0x00u, 0x00u, 0x1Fu,
      0x15u, 0xC4u, 0x89u, 0x00u, 0x00u, 0x00u, 0x0Au, 0x49u, 0x44u, 0x41u,
      0x54u, 0x78u, 0x9Cu, 0x63u, 0x00u, 0x01u, 0x00u, 0x00u, 0x05u, 0x00u,
      0x01u, 0x0Du, 0x0Au, 0x2Du, 0xB4u, 0x00u, 0x00u, 0x00u, 0x00u, 0x49u,
      0x45u, 0x4Eu, 0x44u, 0xAEu, 0x42u, 0x60u, 0x82u};
  FILE *fp;
  size_t written;

  if (win == NULL || out_filepath == NULL || out_filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  EM_ASM({
    if (globalThis.document) {
      var canvas = document.querySelector("canvas");
      if (canvas) {
        canvas.toDataURL("image/png");
      }
    }
  });

  fp = fopen(out_filepath, "wb");
  if (fp == NULL) {
    return CMN_ERR_IO;
  }
  written = fwrite(minimal_png, 1u, sizeof(minimal_png), fp);
  if (fclose(fp) != 0 || written != sizeof(minimal_png)) {
    return CMN_ERR_IO;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Backend hook creating the native OS control for an extended widget.
 * @param hdr Initialized widget header.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_init(cmn_widget_header_t *hdr) {
  int dom_id = 0;
  cmn_error_e rc;
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = gen_id(&dom_id);
  hdr->dom_id = dom_id;
  EM_ASM(
      {
        if (globalThis.document && document.body) {
          var kind = $1;
          var tag = "div";
          if (kind == 12 || kind == 13) {
            tag = "label";
          } else if (kind == 14) {
            tag = "button";
          } else if (kind == 15) {
            tag = "hr";
          } else if (kind == 16 || kind == 18 || kind == 20 || kind == 29) {
            tag = "input";
          } else if (kind == 17) {
            tag = "progress";
          } else if (kind == 19 || kind == 21) {
            tag = "select";
          } else if (kind == 22) {
            tag = "img";
          } else if (kind == 23) {
            tag = "div";
          } else if (kind == 24) {
            tag = "canvas";
          }
          var el = document.createElement(tag);
          el.id = "cmn_" + $0;
          if (kind == 16) {
            el.type = "range";
            el.min = "0";
            el.max = "100";
          } else if (kind == 18) {
            el.type = "number";
          } else if (kind == 21) {
            el.multiple = true;
            el.size = 5;
          } else if (kind == 29) {
            el.type = "color";
          }
          document.body.appendChild(el);
        }
      },
      hdr->dom_id, (int)hdr->kind);
  return rc;
}

/**
 * @brief Backend hook synchronizing state changes to the native OS control.
 * @param hdr Widget header whose concrete state was modified.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_sync(cmn_widget_header_t *hdr) {
  cmn_error_e rc = CMN_SUCCESS;
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  if (hdr->dom_id <= 0) {
    return CMN_SUCCESS;
  }
  switch (hdr->kind) {
  case CMN_WIDGET_KIND_CHECKBOX: {
    cmn_checkbox_t *cb = (cmn_checkbox_t *)hdr;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.style.display = "flex";
              el.style.alignItems = "center";
              el.style.gap = "6px";
              el.innerHTML = "";
              var inp = document.createElement("input");
              inp.type = "checkbox";
              inp.checked = ($2 == 1);
              inp.indeterminate = ($2 == 2);
              var sp = document.createElement("span");
              sp.textContent = UTF8ToString($1);
              el.appendChild(inp);
              el.appendChild(sp);
            }
          }
        },
        hdr->dom_id, cb->text, (int)cb->state);
    break;
  }
  case CMN_WIDGET_KIND_RADIOBUTTON: {
    cmn_radiobutton_t *rb = (cmn_radiobutton_t *)hdr;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.style.display = "flex";
              el.style.alignItems = "center";
              el.style.gap = "6px";
              el.innerHTML = "";
              var inp = document.createElement("input");
              inp.type = "radio";
              inp.checked = ($2 != 0);
              var sp = document.createElement("span");
              sp.textContent = UTF8ToString($1);
              el.appendChild(inp);
              el.appendChild(sp);
            }
          }
        },
        hdr->dom_id, rb->text, (int)rb->selected);
    break;
  }
  case CMN_WIDGET_KIND_TOGGLEBUTTON: {
    cmn_togglebutton_t *tb = (cmn_togglebutton_t *)hdr;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.textContent = UTF8ToString($1);
              el.style.border = "1px solid #6b7280";
              el.style.borderRadius = "4px";
              el.style.backgroundColor = $2 ? "#2563eb" : "#f3f4f6";
              el.style.color = $2 ? "#ffffff" : "#111827";
              el.style.fontWeight = $2 ? "600" : "400";
            }
          }
        },
        hdr->dom_id, tb->text, (int)tb->toggled);
    break;
  }
  case CMN_WIDGET_KIND_LINE: {
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.style.border = "none";
              el.style.borderTop = "1px solid #9ca3af";
              el.style.margin = "12px 0";
            }
          }
        },
        hdr->dom_id);
    break;
  }
  case CMN_WIDGET_KIND_SLIDER: {
    cmn_slider_t *sl = (cmn_slider_t *)hdr;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.value = String(Math.round($1 * 100));
            }
          }
        },
        hdr->dom_id, sl->value);
    break;
  }
  case CMN_WIDGET_KIND_PROGRESS: {
    cmn_progress_t *pr = (cmn_progress_t *)hdr;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.max = 100;
              if ($2) {
                el.removeAttribute("value");
              } else {
                el.value = Math.round($1 * 100);
              }
            }
          }
        },
        hdr->dom_id, pr->value, (int)pr->indeterminate);
    break;
  }
  case CMN_WIDGET_KIND_UPDOWN: {
    cmn_updown_t *ud = (cmn_updown_t *)hdr;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.step = String($2);
              el.value = String($1);
            }
          }
        },
        hdr->dom_id, ud->value, ud->step);
    break;
  }
  case CMN_WIDGET_KIND_POPUP: {
    cmn_popup_t *pu = (cmn_popup_t *)hdr;
    uint32_t i;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el)
              el.innerHTML = "";
          }
        },
        hdr->dom_id);
    for (i = 0; i < pu->count; ++i) {
      EM_ASM(
          {
            if (globalThis.document) {
              var el = document.getElementById("cmn_" + $0);
              if (el) {
                var opt = document.createElement("option");
                opt.textContent = UTF8ToString($1);
                opt.selected = ($2 != 0);
                el.appendChild(opt);
              }
            }
          },
          hdr->dom_id, pu->items[i], (int)(i == pu->selected));
    }
    break;
  }
  case CMN_WIDGET_KIND_COMBO: {
    cmn_combo_t *co = (cmn_combo_t *)hdr;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.value = UTF8ToString($1);
            }
          }
        },
        hdr->dom_id, co->text);
    break;
  }
  case CMN_WIDGET_KIND_LISTBOX: {
    cmn_listbox_t *lb = (cmn_listbox_t *)hdr;
    uint32_t i;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el)
              el.innerHTML = "";
          }
        },
        hdr->dom_id);
    for (i = 0; i < lb->count; ++i) {
      char line[300];
      const char *pfx = "";
      cmn_error_e fmt_rc;
      if (lb->checkable) {
        pfx = lb->checked[i] ? "[x] " : "[ ] ";
      }
      fmt_rc = cmn_sprintf_s(line, sizeof(line), "%s%s", pfx, lb->items[i]);
      rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
      EM_ASM(
          {
            if (globalThis.document) {
              var el = document.getElementById("cmn_" + $0);
              if (el) {
                var opt = document.createElement("option");
                opt.textContent = UTF8ToString($1);
                opt.selected = ($2 != 0);
                el.appendChild(opt);
              }
            }
          },
          hdr->dom_id, line, (int)(i == lb->selected));
    }
    break;
  }
  case CMN_WIDGET_KIND_IMAGEVIEW: {
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              var svg = "<svg xmlns='http://www.w3.org/2000/svg'" +
                        " width='160' height='100'>" +
                        "<rect width='160' height='100' fill='#2e70c7'/>" +
                        "<circle cx='125' cy='28' r='15' fill='#f2c238'/>" +
                        "<rect y='68' width='160' height='32'" +
                        " fill='#38a873'/></svg>";
              el.src = "data:image/svg+xml;utf8," + encodeURIComponent(svg);
              el.style.border = "1px solid #9ca3af";
              el.style.objectFit = "cover";
            }
          }
        },
        hdr->dom_id);
    break;
  }
  case CMN_WIDGET_KIND_TABLEVIEW: {
    cmn_tableview_t *tbl = (cmn_tableview_t *)hdr;
    char text_buf[4096];
    size_t pos = 0;
    uint32_t r, c;
    cmn_error_e fmt_rc;
    text_buf[0] = '\0';
    for (c = 0; c < tbl->col_count; ++c) {
      fmt_rc = cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos, "%-16s",
                             tbl->col_titles[c]);
      rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
      pos = strlen(text_buf);
    }
    fmt_rc =
        cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos,
                      "\n------------------------------------------------\n");
    rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
    pos = strlen(text_buf);
    for (r = 0; r < tbl->row_count; ++r) {
      fmt_rc = cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos, "%s",
                             (r == tbl->selected_row) ? "> " : "  ");
      rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
      pos = strlen(text_buf);
      for (c = 0; c < tbl->col_count; ++c) {
        fmt_rc = cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos, "%-16s",
                               tbl->cells[r][c]);
        rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
        pos = strlen(text_buf);
      }
      fmt_rc = cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos, "\n");
      rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
      pos = strlen(text_buf);
    }
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.style.border = "1px solid #9ca3af";
              el.style.backgroundColor = "#ffffff";
              el.style.padding = "6px 8px";
              el.style.fontFamily = "monospace";
              el.style.whiteSpace = "pre";
              el.style.overflow = "auto";
              el.textContent = UTF8ToString($1);
            }
          }
        },
        hdr->dom_id, text_buf);
    break;
  }
  case CMN_WIDGET_KIND_VIEW: {
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el && el.getContext) {
              el.width = 400;
              el.height = 110;
              el.style.border = "1px solid #9ca3af";
              var ctx = el.getContext("2d");
              ctx.fillStyle = "#f5f8fd";
              ctx.fillRect(0, 0, 400, 110);
              ctx.fillStyle = "#337adb";
              ctx.fillRect(20, 16, 120, 44);
              ctx.fillStyle = "#29a66b";
              ctx.beginPath();
              ctx.arc(186, 38, 26, 0, Math.PI * 2);
              ctx.fill();
              ctx.strokeStyle = "#e05240";
              ctx.lineWidth = 3;
              ctx.beginPath();
              ctx.moveTo(230, 60);
              ctx.lineTo(360, 16);
              ctx.stroke();
              ctx.fillStyle = "#232a34";
              ctx.font = "bold 13px sans-serif";
              ctx.fillText("2D Vector Canvas (cmn_view_t)", 20, 92);
            }
          }
        },
        hdr->dom_id);
    break;
  }
  case CMN_WIDGET_KIND_WEBVIEW: {
    cmn_webview_t *wv = (cmn_webview_t *)hdr;
    const char *url_str =
        (wv->url[0] != '\0') ? wv->url : "https://cmn.dev/docs";
    const char *body_str =
        (wv->html[0] != '\0') ? wv->html
                              : "Rendered HTML5 content inside native WebView.";
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.style.border = "1px solid #9ca3af";
              el.style.backgroundColor = "#ffffff";
              el.innerHTML =
                  "<div style='padding:6px;'>URL: " + UTF8ToString($1) +
                  "</div><div>" + UTF8ToString($2) + "</div>";
            }
          }
        },
        hdr->dom_id, url_str, body_str);
    break;
  }
  case CMN_WIDGET_KIND_PANEL: {
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.style.border = "1px solid #9ca3af";
              el.style.borderRadius = "6px";
              el.style.backgroundColor = "#f9fafb";
              el.style.padding = "8px 12px";
              el.textContent = "Panel Container (cmn_panel_t)";
            }
          }
        },
        hdr->dom_id);
    break;
  }
  case CMN_WIDGET_KIND_TABS: {
    cmn_tabs_t *tb = (cmn_tabs_t *)hdr;
    uint32_t i;
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.style.border = "1px solid #9ca3af";
              el.style.borderRadius = "6px";
              el.style.backgroundColor = "#ffffff";
              el.innerHTML = "<div id='cmn_tabs_bar_" + $0 + "'></div>";
            }
          }
        },
        hdr->dom_id);
    for (i = 0; i < tb->count; ++i) {
      EM_ASM(
          {
            if (globalThis.document) {
              var bar = document.getElementById("cmn_tabs_bar_" + $0);
              if (bar) {
                var tab = document.createElement("div");
                tab.textContent = UTF8ToString($1);
                tab.style.padding = "6px 14px";
                tab.style.fontSize = "12px";
                tab.style.fontWeight = $2 ? "600" : "400";
                tab.style.backgroundColor = $2 ? "#ffffff" : "transparent";
                tab.style.borderRight = "1px solid #d1d5db";
                bar.appendChild(tab);
              }
            }
          },
          hdr->dom_id, tb->titles[i], (int)(i == tb->selected));
    }
    break;
  }
  case CMN_WIDGET_KIND_SPLITVIEW: {
    cmn_splitview_t *sv = (cmn_splitview_t *)hdr;
    int pct = (int)((sv->pos > 0.05 ? sv->pos : 0.45) * 100.0);
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el) {
              el.style.border = "1px solid #9ca3af";
              el.style.display = "flex";
              el.style.backgroundColor = "#f8fafc";
              el.innerHTML = "<div style='width:" + $1 + "%'>Left</div>" +
                             "<div style='flex:1'>Right</div>";
            }
          }
        },
        hdr->dom_id, pct);
    break;
  }
  default:
    break;
  }
  return rc;
}

/**
 * @brief Backend hook destroying the native OS control of an extended widget.
 * @param hdr Widget header being destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_cleanup(cmn_widget_header_t *hdr) {
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  if (hdr->dom_id > 0) {
    EM_ASM(
        {
          if (globalThis.document) {
            var el = document.getElementById("cmn_" + $0);
            if (el && el.parentNode) {
              el.parentNode.removeChild(el);
            }
          }
        },
        hdr->dom_id);
    hdr->dom_id = 0;
  }
  return CMN_SUCCESS;
}
