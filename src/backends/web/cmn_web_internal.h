#ifndef CMN_WEB_INTERNAL_H
#define CMN_WEB_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn_internal.h"
#include "cmn/app.h"
#include "cmn/widget.h"
/* clang-format on */

/**
 * @file cmn_web_internal.h
 * @brief Internal Web (Emscripten) widget and window structure definitions.
 */

/**
 * @brief Internal representation of an Emscripten DOM window.
 */
struct cmn_window_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  int dom_id;               /**< DOM element identifier (`cmn_<id>`). */
};

/**
 * @brief Internal representation of an Emscripten DOM button.
 */
struct cmn_button_s {
  cmn_widget_header_t base;     /**< Tagged polymorphic widget header. */
  int dom_id;                   /**< DOM element identifier. */
  cmn_button_click_fn on_click; /**< Click callback function. */
  void *userdata;               /**< User data passed to `on_click`. */
};

/**
 * @brief Internal representation of an Emscripten DOM label.
 */
struct cmn_label_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  int dom_id;               /**< DOM element identifier. */
};

/**
 * @brief Internal representation of an Emscripten DOM text box.
 */
struct cmn_textbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  int dom_id;               /**< DOM element identifier. */
  char text_cache[1024];    /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of an Emscripten DOM text view.
 */
struct cmn_textview_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  int dom_id;               /**< DOM element identifier. */
  char text_cache[4096];    /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of an Emscripten DOM vertical box container.
 */
struct cmn_vbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  int dom_id;               /**< DOM element identifier. */
};

/**
 * @brief Internal representation of an Emscripten DOM horizontal box container.
 */
struct cmn_hbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  int dom_id;               /**< DOM element identifier. */
};

/**
 * @brief Callback entry for a Web menu item.
 */
struct web_menu_item_cb {
  cmn_error_e (*on_click)(void *); /**< Menu item click callback. */
  void *userdata;                  /**< User data passed to callback. */
};

/** @brief Maximum number of items per Web menu. */
#define CMN_WEB_MAX_MENU_ITEMS 64

/**
 * @brief Internal representation of an Emscripten DOM menu.
 */
struct cmn_menu_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  struct web_menu_item_cb
      items[CMN_WEB_MAX_MENU_ITEMS]; /**< Registered item callbacks. */
  int item_count;                    /**< Number of items in this menu. */
};

/**
 * @brief Internal representation of an Emscripten file picker dialog.
 */
struct cmn_filepicker_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  char path[256];           /**< Selected UTF-8 file path. */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_WEB_INTERNAL_H */
