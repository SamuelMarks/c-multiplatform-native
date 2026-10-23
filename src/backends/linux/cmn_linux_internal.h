#ifndef CMN_LINUX_INTERNAL_H
#define CMN_LINUX_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn_internal.h"
#include "cmn/app.h"
#include "cmn/widget.h"
#ifdef CMN_HAS_GTK3
#include <gtk/gtk.h>
#else
/** @brief Fallback opaque GTK widget type when compiled without GTK3. */
typedef void GtkWidget;
/** @brief Fallback GLib boolean type when compiled without GTK3. */
typedef int gboolean;
/** @brief Fallback GLib generic pointer type when compiled without GTK3. */
typedef void *gpointer;
#endif
/* clang-format on */

/**
 * @file cmn_linux_internal.h
 * @brief Internal Linux GTK3 widget and window structure definitions.
 */

/**
 * @brief Internal representation of a GTK3 window.
 */
struct cmn_window_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  GtkWidget *window;        /**< Native GTK3 window handle. */
};

/**
 * @brief Internal representation of a GTK3 push button widget.
 */
struct cmn_button_s {
  cmn_widget_header_t base;     /**< Tagged polymorphic widget header. */
  GtkWidget *button;            /**< Native GTK3 button handle. */
  cmn_button_click_fn on_click; /**< Click callback function. */
  void *userdata;               /**< User data passed to `on_click`. */
};

/**
 * @brief Internal representation of a GTK3 static label widget.
 */
struct cmn_label_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  GtkWidget *label;         /**< Native GTK3 label handle. */
};

/**
 * @brief Internal representation of a GTK3 single-line entry widget.
 */
struct cmn_textbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  GtkWidget *entry;         /**< Native GTK3 entry handle. */
  char text_cache[1024];    /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of a GTK3 scrollable text view widget.
 */
struct cmn_textview_s {
  cmn_widget_header_t base;   /**< Tagged polymorphic widget header. */
  GtkWidget *scrolled_window; /**< Native GTK3 scrolled container. */
  GtkWidget *textview;        /**< Native GTK3 text view control. */
  char text_cache[4096];      /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of a GTK3 vertical box layout container.
 */
struct cmn_vbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  GtkWidget *box;           /**< Native GTK3 vertical box handle. */
};

/**
 * @brief Internal representation of a GTK3 horizontal box layout container.
 */
struct cmn_hbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  GtkWidget *box;           /**< Native GTK3 horizontal box handle. */
};

/**
 * @brief Callback entry for a GTK3 menu item.
 */
struct linux_menu_item_cb {
  cmn_error_e (*on_click)(void *); /**< Menu item click callback. */
  void *userdata;                  /**< User data passed to callback. */
};

/** @brief Maximum number of items per GTK3 menu. */
#define CMN_LINUX_MAX_MENU_ITEMS 64

/**
 * @brief Internal representation of a GTK3 menu.
 */
struct cmn_menu_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  GtkWidget *gtk_menu;      /**< Native GTK3 menu widget. */
  GtkWidget *gtk_menu_bar;  /**< Native GTK3 menu bar widget. */
  struct linux_menu_item_cb
      items[CMN_LINUX_MAX_MENU_ITEMS]; /**< Registered item callbacks. */
  int item_count;                      /**< Number of items in this menu. */
};

/**
 * @brief Internal representation of a GTK3 file picker dialog.
 */
struct cmn_filepicker_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  char *path;               /**< Dynamically allocated UTF-8 file path. */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_LINUX_INTERNAL_H */
