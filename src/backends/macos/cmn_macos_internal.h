#ifndef CMN_MACOS_INTERNAL_H
#define CMN_MACOS_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn_internal.h"
#include "cmn/app.h"
#include "cmn/widget.h"
#include <Cocoa/Cocoa.h>
/* clang-format on */

/**
 * @file cmn_macos_internal.h
 * @brief Internal macOS AppKit widget and window structure definitions.
 */

/**
 * @brief Internal representation of a macOS window.
 */
struct cmn_window_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  NSWindow *ns_window;      /**< The native macOS window. */
  cmn_app_t *app;           /**< Pointer to the parent application. */
};

/**
 * @brief Internal representation of a macOS button widget.
 */
struct cmn_button_s {
  cmn_widget_header_t base;     /**< Tagged polymorphic widget header. */
  NSButton *ns_button;          /**< The native macOS button. */
  cmn_button_click_fn on_click; /**< Callback for click events. */
  void *userdata;               /**< User data passed to the callback. */
  id target;                    /**< The Objective-C target object. */
  int tabindex;                 /**< Keyboard navigation tab index. */
};

/**
 * @brief Internal representation of a macOS label widget.
 */
struct cmn_label_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  NSTextField *ns_label;    /**< The native macOS text field used as a label. */
  int tabindex;             /**< Keyboard navigation tab index. */
};

/**
 * @brief Internal representation of a macOS text box widget.
 */
struct cmn_textbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  NSTextField *ns_textbox;  /**< The native macOS text field. */
  char text_cache[1024];    /**< Cache used for returning C strings. */
  int tabindex;             /**< Keyboard navigation tab index. */
};

/**
 * @brief Internal representation of a macOS text view widget.
 */
struct cmn_textview_s {
  cmn_widget_header_t base;    /**< Tagged polymorphic widget header. */
  NSScrollView *ns_scrollview; /**< The native macOS scroll view. */
  NSTextView *ns_textview;     /**< The native macOS text view. */
  char text_cache[4096];       /**< Cache used for returning multi-line text. */
  int tabindex;                /**< Keyboard navigation tab index. */
};

/**
 * @brief Internal representation of a macOS vertical box layout.
 */
struct cmn_vbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  NSStackView *ns_stack;    /**< The native macOS vertical stack view. */
  int tabindex;             /**< Keyboard navigation tab index. */
};

/**
 * @brief Internal representation of a macOS horizontal box layout.
 */
struct cmn_hbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  NSStackView *ns_stack;    /**< The native macOS horizontal stack view. */
  int tabindex;             /**< Keyboard navigation tab index. */
};

/**
 * @brief Internal representation of a macOS menu.
 */
struct cmn_menu_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  NSMenu *ns_menu;          /**< The native macOS menu. */
  NSMutableArray *targets;  /**< Array of CmnMenuItemTarget objects. */
};

/**
 * @brief Internal representation of a macOS file picker dialog.
 */
struct cmn_filepicker_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  NSString *ns_path;        /**< Selected file path as an NSString. */
  char path_cache[1024];    /**< Selected file path cached as UTF-8 C string. */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_MACOS_INTERNAL_H */
