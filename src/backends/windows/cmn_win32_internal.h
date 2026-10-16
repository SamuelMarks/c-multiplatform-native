#ifndef CMN_WIN32_INTERNAL_H
#define CMN_WIN32_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn_internal.h"
#include "cmn/app.h"
#include "cmn/widget.h"
#include <winsock2.h>
/* clang-format on */

/**
 * @file cmn_win32_internal.h
 * @brief Internal Win32 widget and window structure definitions.
 */

/**
 * @brief Internal representation of a Win32 window.
 */
struct cmn_window_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  HWND hwnd;                /**< Native Win32 window handle. */
  int child_count;          /**< Number of child widgets attached. */
};

/**
 * @brief Internal representation of a Win32 push button widget.
 */
struct cmn_button_s {
  cmn_widget_header_t base;     /**< Tagged polymorphic widget header. */
  HWND hwnd;                    /**< Native Win32 button handle. */
  cmn_button_click_fn on_click; /**< Click callback function. */
  void *userdata;               /**< User data passed to `on_click`. */
  int cmd_id;                   /**< Registered WM_COMMAND identifier. */
};

/**
 * @brief Internal representation of a Win32 static label widget.
 */
struct cmn_label_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  HWND hwnd;                /**< Native Win32 static control handle. */
};

/**
 * @brief Internal representation of a Win32 single-line edit control.
 */
struct cmn_textbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  HWND hwnd;                /**< Native Win32 edit control handle. */
  char text_cache[1024];    /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of a Win32 multi-line edit control.
 */
struct cmn_textview_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  HWND hwnd;                /**< Native Win32 multi-line edit handle. */
  char text_cache[4096];    /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of a Win32 vertical box layout container.
 */
struct cmn_vbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  HWND hwnd;                /**< Native Win32 container window handle. */
  int child_count;          /**< Number of child widgets attached. */
};

/**
 * @brief Internal representation of a Win32 horizontal box layout container.
 */
struct cmn_hbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  HWND hwnd;                /**< Native Win32 container window handle. */
  int child_count;          /**< Number of child widgets attached. */
};

/**
 * @brief Callback entry for a Win32 menu item.
 */
struct win32_menu_item_cb {
  cmn_error_e (*on_click)(void *); /**< Menu item click callback. */
  void *userdata;                  /**< User data passed to callback. */
};

/** @brief Maximum number of items per Win32 menu. */
#define CMN_WIN32_MAX_MENU_ITEMS 64

/**
 * @brief Internal representation of a Win32 menu.
 */
struct cmn_menu_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  HMENU hmenu;              /**< Native Win32 HMENU handle. */
  struct win32_menu_item_cb
      items[CMN_WIN32_MAX_MENU_ITEMS]; /**< Registered item callbacks. */
  int item_count;                      /**< Number of items in this menu. */
};

/**
 * @brief Internal representation of a Win32 file picker dialog.
 */
struct cmn_filepicker_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  char path[MAX_PATH];      /**< Selected UTF-8 file path. */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_WIN32_INTERNAL_H */
