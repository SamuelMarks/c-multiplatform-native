#ifndef CMN_INTERNAL_H
#define CMN_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/a11y_i18n.h"
#include "cmn/error.h"
#include "cmn/types.h"
#include "cmn/widget.h"
#include "cmn/layout.h"
/* clang-format on */

/**
 * @file cmn_internal.h
 * @brief Internal shared header for tagged polymorphic widget structures.
 */

/**
 * @defgroup cmn_internal Internal Polymorphic Header
 * @brief Internal Polymorphic Header module.
 * @{
 */

/** @brief Magic tag value used to validate initialized `cmn_widget_header_t`
 * instances. */
#define CMN_WIDGET_MAGIC 0x434D4E57u

/**
 * @brief Polymorphic destructor callback signature for concrete widgets.
 * @param widget_ptr Address of the widget pointer to destroy and null out.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_widget_destroy_fn)(void **widget_ptr);

/**
 * @brief Common tagged header placed as the first member of every concrete
 * widget and container structure.
 */
typedef struct cmn_widget_header_s {
  uint32_t magic;         /**< Validation magic (`CMN_WIDGET_MAGIC`). */
  cmn_widget_kind_e kind; /**< Concrete widget kind discriminant. */
  cmn_widget_destroy_fn destroy_fn; /**< Concrete destructor callback. */
  cmn_rect_t bounds;                /**< Current or intrinsic bounding box. */
  void *layout_root; /**< Attached root layout container for windows. */
  cmn_window_resize_fn on_resize; /**< Window resize callback function. */
  void *resize_userdata;          /**< User data passed to `on_resize`. */
  void *native_handle; /**< Backend-specific native view/control handle. */
  int dom_id;          /**< Backend-specific DOM or integer identifier. */
  cmn_bool_t enabled;  /**< Cell-level interactivity flag. */
  cmn_bool_t visible;  /**< Cell-level visibility flag. */
  cmn_edges_t padding; /**< Cell-level padding edges. */
  cmn_button_t *default_button;          /**< Window default push button. */
  cmn_cursor_e cursor;                   /**< Window active cursor shape. */
  cmn_window_event_fn on_close;          /**< Window close callback. */
  void *close_userdata;                  /**< User data for `on_close`. */
  cmn_window_event_fn on_moved;          /**< Window moved callback. */
  void *moved_userdata;                  /**< User data for `on_moved`. */
  cmn_window_event_fn on_activate;       /**< Window activate callback. */
  void *activate_userdata;               /**< User data for `on_activate`. */
  cmn_bool_t menu_item_enabled[32];      /**< Menu item enabled states. */
  cmn_check_state_e menu_item_state[32]; /**< Menu item check states. */
  uint32_t menu_ext_count;               /**< Extended menu item count. */
  cmn_filepicker_mode_e fp_mode;         /**< File picker operation mode. */
  size_t fp_filetype_count;              /**< File picker filter count. */
  char tooltip[128];                     /**< Widget tooltip text. */
  char icon_path[256];                   /**< Button icon image path. */
  cmn_bool_t multiline;                  /**< Label multiline wrapping flag. */
  cmn_text_align_e text_align;           /**< Label horizontal alignment. */
  uint32_t fg_rgba;                      /**< Foreground RGBA color. */
  uint32_t bg_rgba;                      /**< Background RGBA color. */
  cmn_bool_t editable;                   /**< TextBox editable flag. */
  cmn_bool_t autoselect;                 /**< TextBox autoselect flag. */
  int sel_start;                         /**< TextBox selection start offset. */
  int sel_end;                           /**< TextBox selection end offset. */
  char font_family[64];                  /**< TextView font family name. */
  double font_size;                      /**< TextView font size in points. */
  uint32_t font_style_mask;              /**< TextView font style bitmask. */
  cmn_bool_t text_wrap;                  /**< TextView line wrapping flag. */
  int popup_x;                           /**< Last menu popup X coordinate. */
  int popup_y;                           /**< Last menu popup Y coordinate. */
  cmn_a11y_role_e a11y_role;             /**< Accessibility role. */
  int a11y_tabindex;                     /**< Accessibility tab index. */
} cmn_widget_header_t;

/**
 * @brief Base widget structure wrapping `cmn_widget_header_t`.
 */
struct cmn_widget_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
};

/**
 * @brief Base container structure wrapping `cmn_widget_header_t`.
 */
struct cmn_container_s {
  cmn_widget_header_t base; /**< Tagged polymorphic container header. */
};

/** @brief Maximum number of children inside a single flex or grid container. */
#define CMN_LAYOUT_MAX_CHILDREN 32

/** @brief Maximum number of responsive breakpoints per layout container. */
#define CMN_LAYOUT_MAX_BREAKPOINTS 8

/** @brief Maximum number of explicit or auto-generated grid tracks. */
#define CMN_GRID_MAX_TRACKS 32

/**
 * @brief Internal entry for a child inside a `cmn_flex_t` container.
 */
struct cmn_flex_child_s {
  void *widget;                /**< Child widget handle. */
  cmn_flex_item_style_t style; /**< Configured flex item style. */
  cmn_rect_t intrinsic_rect;   /**< Initial intrinsic bounds when added. */
  cmn_rect_t computed_rect;    /**< Computed rectangle relative to parent. */
};

/**
 * @brief Internal entry for a responsive breakpoint on a `cmn_flex_t`.
 */
struct cmn_flex_breakpoint_s {
  int max_width;            /**< Maximum container width trigger (`> 0`). */
  cmn_flex_direction_e dir; /**< Flex direction when matched. */
  cmn_flex_wrap_e wrap;     /**< Flex wrap mode when matched. */
};

/**
 * @brief Concrete implementation of the Flexbox layout container
 * (`cmn_flex_t`).
 */
struct cmn_flex_s {
  cmn_widget_header_t base;       /**< Tagged polymorphic widget header. */
  cmn_flex_direction_e direction; /**< Configured main-axis direction. */
  cmn_flex_direction_e
      active_dir;                /**< Effective direction after breakpoints. */
  cmn_flex_wrap_e wrap;          /**< Configured line-wrapping mode. */
  cmn_flex_wrap_e active_wrap;   /**< Effective wrap mode after breakpoints. */
  cmn_justify_content_e justify; /**< Main-axis justification mode. */
  cmn_align_items_e align_items; /**< Default cross-axis alignment. */
  int row_gap;                   /**< Vertical gap in pixels. */
  int column_gap;                /**< Horizontal gap in pixels. */
  cmn_edges_t padding;           /**< Inner container padding. */
  struct cmn_flex_child_s
      children[CMN_LAYOUT_MAX_CHILDREN]; /**< Registered child entries. */
  size_t child_count;                    /**< Number of registered children. */
  struct cmn_flex_breakpoint_s
      breakpoints[CMN_LAYOUT_MAX_BREAKPOINTS]; /**< Responsive rules. */
  size_t breakpoint_count;                     /**< Number of breakpoints. */
};

/**
 * @brief Internal entry for a child inside a `cmn_grid_t` container.
 */
struct cmn_grid_child_s {
  void *widget;                /**< Child widget handle. */
  cmn_grid_item_style_t style; /**< Configured grid placement style. */
  cmn_rect_t intrinsic_rect;   /**< Initial intrinsic bounds when added. */
  cmn_rect_t computed_rect;    /**< Computed rectangle relative to parent. */
};

/**
 * @brief Internal entry for a responsive breakpoint on a `cmn_grid_t`.
 */
struct cmn_grid_breakpoint_s {
  int max_width;    /**< Maximum container width trigger (`> 0`). */
  size_t col_count; /**< Overridden column count when matched. */
};

/**
 * @brief Concrete implementation of the Grid layout container (`cmn_grid_t`).
 */
struct cmn_grid_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  cmn_grid_track_t
      col_tracks[CMN_GRID_MAX_TRACKS]; /**< Configured column tracks. */
  size_t col_count;                    /**< Number of configured columns. */
  size_t effective_col_count;          /**< Active columns after layout. */
  cmn_grid_track_t
      row_tracks[CMN_GRID_MAX_TRACKS]; /**< Configured row tracks. */
  size_t row_count;                    /**< Number of configured rows. */
  int auto_fit_min_width; /**< Minimum column width for `auto-fit` (`0` = off).
                           */
  int auto_fit_max_cols;  /**< Maximum columns for `auto-fit`. */
  int row_gap;            /**< Vertical gap between rows in pixels. */
  int column_gap;         /**< Horizontal gap between columns in pixels. */
  cmn_edges_t padding;    /**< Inner container padding. */
  struct cmn_grid_child_s
      children[CMN_LAYOUT_MAX_CHILDREN]; /**< Registered child entries. */
  size_t child_count;                    /**< Number of registered children. */
  struct cmn_grid_breakpoint_s
      breakpoints[CMN_LAYOUT_MAX_BREAKPOINTS]; /**< Responsive rules. */
  size_t breakpoint_count;                     /**< Number of breakpoints. */
};

/** @brief Maximum items in pop-up, combo, list box, or tabs widgets. */
#define CMN_WIDGET_MAX_ITEMS 32
/** @brief Maximum columns in a `cmn_tableview_t`. */
#define CMN_TABLEVIEW_MAX_COLS 8
/** @brief Maximum rows in a `cmn_tableview_t`. */
#define CMN_TABLEVIEW_MAX_ROWS 32

/**
 * @brief Concrete implementation of the checkbox widget (`cmn_checkbox_t`).
 */
struct cmn_checkbox_s {
  cmn_widget_header_t base;         /**< Tagged polymorphic widget header. */
  cmn_bool_t is_tristate;           /**< Whether mixed state is permitted. */
  cmn_check_state_e state;          /**< Current check state. */
  char text[256];                   /**< UTF-8 label text. */
  cmn_checkbox_toggle_fn on_toggle; /**< Toggle event callback. */
  void *userdata;                   /**< User data passed to `on_toggle`. */
};

/**
 * @brief Concrete implementation of the radio button widget
 * (`cmn_radiobutton_t`).
 */
struct cmn_radiobutton_s {
  cmn_widget_header_t base;            /**< Tagged polymorphic widget header. */
  cmn_bool_t selected;                 /**< Selection state. */
  char text[256];                      /**< UTF-8 label text. */
  cmn_radiobutton_select_fn on_select; /**< Selection event callback. */
  void *userdata;                      /**< User data passed to `on_select`. */
};

/**
 * @brief Concrete implementation of the flat/toggle button widget
 * (`cmn_togglebutton_t`).
 */
struct cmn_togglebutton_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  cmn_bool_t toggled;       /**< Pressed/active state. */
  char text[256];           /**< UTF-8 label text. */
  cmn_togglebutton_toggle_fn on_toggle; /**< Toggle event callback. */
  void *userdata;                       /**< User data passed to `on_toggle`. */
};

/**
 * @brief Concrete implementation of the separator line widget (`cmn_line_t`).
 */
struct cmn_line_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  cmn_bool_t vertical;      /**< Orientation (`CMN_TRUE` if vertical). */
};

/**
 * @brief Concrete implementation of the slider widget (`cmn_slider_t`).
 */
struct cmn_slider_s {
  cmn_widget_header_t base;     /**< Tagged polymorphic widget header. */
  cmn_bool_t vertical;          /**< Orientation (`CMN_TRUE` if vertical). */
  double value;                 /**< Normalized position in `[0.0, 1.0]`. */
  uint32_t steps;               /**< Discrete tick step count. */
  cmn_slider_moved_fn on_moved; /**< Movement event callback. */
  void *userdata;               /**< User data passed to `on_moved`. */
};

/**
 * @brief Concrete implementation of the progress bar widget (`cmn_progress_t`).
 */
struct cmn_progress_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  double value;             /**< Normalized progress in `[0.0, 1.0]`. */
  cmn_bool_t indeterminate; /**< Whether indeterminate mode is active. */
};

/**
 * @brief Concrete implementation of the stepper / up-down widget
 * (`cmn_updown_t`).
 */
struct cmn_updown_s {
  cmn_widget_header_t base;   /**< Tagged polymorphic widget header. */
  double value;               /**< Current numeric value. */
  double step;                /**< Positive step delta. */
  cmn_updown_step_fn on_step; /**< Step event callback. */
  void *userdata;             /**< User data passed to `on_step`. */
};

/**
 * @brief Concrete implementation of the drop-down pop-up widget
 * (`cmn_popup_t`).
 */
struct cmn_popup_s {
  cmn_widget_header_t base;              /**< Tagged polymorphic header. */
  char items[CMN_WIDGET_MAX_ITEMS][128]; /**< Stored item text strings. */
  uint32_t count;                        /**< Number of items. */
  uint32_t selected;                     /**< Selected item index. */
  cmn_popup_select_fn on_select;         /**< Selection event callback. */
  void *userdata;                        /**< User data for `on_select`. */
};

/**
 * @brief Concrete implementation of the editable combo box widget
 * (`cmn_combo_t`).
 */
struct cmn_combo_s {
  cmn_widget_header_t base;              /**< Tagged polymorphic header. */
  char text[256];                        /**< Current editable text. */
  char items[CMN_WIDGET_MAX_ITEMS][128]; /**< Suggestion items. */
  uint32_t count;                        /**< Number of suggestion items. */
  uint32_t selected;                     /**< Selected suggestion index. */
  cmn_combo_change_fn on_change;         /**< Change event callback. */
  void *userdata;                        /**< User data for `on_change`. */
};

/**
 * @brief Concrete implementation of the scrollable list box widget
 * (`cmn_listbox_t`).
 */
struct cmn_listbox_s {
  cmn_widget_header_t base;                 /**< Tagged polymorphic header. */
  char items[CMN_WIDGET_MAX_ITEMS][128];    /**< List item strings. */
  cmn_bool_t checked[CMN_WIDGET_MAX_ITEMS]; /**< Per-item checked state. */
  uint32_t count;                           /**< Number of items. */
  uint32_t selected;                        /**< Selected item index. */
  cmn_bool_t checkable;            /**< Whether checkboxes are shown. */
  cmn_listbox_select_fn on_select; /**< Selection event callback. */
  void *userdata;                  /**< User data for `on_select`. */
};

/**
 * @brief Concrete implementation of the image view widget (`cmn_imageview_t`).
 */
struct cmn_imageview_s {
  cmn_widget_header_t base;        /**< Tagged polymorphic widget header. */
  char image_path[512];            /**< Image file path or URI. */
  cmn_scale_mode_e scale;          /**< Image scaling mode. */
  cmn_imageview_click_fn on_click; /**< Click event callback. */
  void *userdata;                  /**< User data passed to `on_click`. */
};

/**
 * @brief Concrete implementation of the multi-column table view widget
 * (`cmn_tableview_t`).
 */
struct cmn_tableview_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  char col_titles[CMN_TABLEVIEW_MAX_COLS][64]; /**< Column header titles. */
  int col_widths[CMN_TABLEVIEW_MAX_COLS];      /**< Column widths in pixels. */
  int col_min_widths[CMN_TABLEVIEW_MAX_COLS];  /**< Column minimum widths. */
  int col_max_widths[CMN_TABLEVIEW_MAX_COLS];  /**< Column maximum widths. */
  uint32_t col_count;                          /**< Number of columns. */
  uint32_t row_count;                          /**< Number of rows. */
  uint32_t freeze_cols;                        /**< Frozen leading columns. */
  char cells[CMN_TABLEVIEW_MAX_ROWS][CMN_TABLEVIEW_MAX_COLS]
            [64];                    /**< Cell text matrix. */
  uint32_t selected_row;             /**< Selected row index. */
  cmn_tableview_select_fn on_select; /**< Row selection callback. */
  void *userdata;                    /**< User data for `on_select`. */
  cmn_tableview_data_fn on_data;     /**< Virtualized cell data callback. */
  void *data_userdata;               /**< User data for `on_data`. */
};

/**
 * @brief Concrete implementation of the custom 2D drawing view widget
 * (`cmn_view_t`).
 */
struct cmn_view_s {
  cmn_widget_header_t base;      /**< Tagged polymorphic widget header. */
  cmn_bool_t scrollable;         /**< Whether scrollbars are enabled. */
  int content_width;             /**< Virtual content width in pixels. */
  int content_height;            /**< Virtual content height in pixels. */
  uint32_t draw_cmd_count;       /**< Number of recorded 2D draw commands. */
  cmn_rect_t last_rect;          /**< Last drawn rectangle geometry. */
  uint32_t last_rgba;            /**< Last drawn RGBA color. */
  int last_x0;                   /**< Last drawn start X coordinate. */
  int last_y0;                   /**< Last drawn start Y coordinate. */
  int last_x1;                   /**< Last drawn end X coordinate. */
  int last_y1;                   /**< Last drawn end Y coordinate. */
  cmn_view_event_fn on_draw;     /**< Draw callback function. */
  void *draw_userdata;           /**< User data passed to `on_draw`. */
  cmn_view_event_fn on_click;    /**< Click callback function. */
  void *click_userdata;          /**< User data passed to `on_click`. */
  cmn_view_event_fn on_wheel;    /**< Mouse wheel callback function. */
  void *wheel_userdata;          /**< User data passed to `on_wheel`. */
  cmn_view_event_fn on_drag;     /**< Mouse drag callback function. */
  void *drag_userdata;           /**< User data passed to `on_drag`. */
  cmn_view_event_fn on_key_down; /**< Key down callback function. */
  void *key_down_userdata;       /**< User data passed to `on_key_down`. */
  cmn_view_event_fn on_key_up;   /**< Key up callback function. */
  void *key_up_userdata;         /**< User data passed to `on_key_up`. */
};

/**
 * @brief Concrete implementation of the embedded web browser view widget
 * (`cmn_webview_t`).
 */
struct cmn_webview_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  char url[512];            /**< Current navigated URL. */
  char html[2048];          /**< Current loaded HTML document. */
  char prev_url[512];       /**< Back history URL entry. */
  char next_url[512];       /**< Forward history URL entry. */
};

/**
 * @brief Concrete implementation of the grouping or scrollable panel container
 * (`cmn_panel_t`).
 */
struct cmn_panel_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  cmn_bool_t scrollable;    /**< Whether scrollbars are enabled. */
};

/**
 * @brief Concrete implementation of the tabbed container widget (`cmn_tabs_t`).
 */
struct cmn_tabs_s {
  cmn_widget_header_t base;              /**< Tagged polymorphic header. */
  char titles[CMN_WIDGET_MAX_ITEMS][64]; /**< Tab header titles. */
  void *contents[CMN_WIDGET_MAX_ITEMS];  /**< Tab content widgets. */
  uint32_t count;                        /**< Number of tab pages. */
  uint32_t selected;                     /**< Active tab index. */
  cmn_tabs_select_fn on_select;          /**< Tab selection callback. */
  void *userdata;                        /**< User data for `on_select`. */
};

/**
 * @brief Concrete implementation of the split view container
 * (`cmn_splitview_t`).
 */
struct cmn_splitview_s {
  cmn_widget_header_t base;   /**< Tagged polymorphic widget header. */
  cmn_bool_t vertical;        /**< Orientation (`CMN_TRUE` if vertical). */
  void *child0;               /**< First pane child widget. */
  void *child1;               /**< Second pane child widget. */
  double pos;                 /**< Normalized divider position `[0, 1]`. */
  cmn_bool_t pane_visible[2]; /**< Visibility flags for panes 0 and 1. */
};

/**
 * @brief Concrete implementation of the color picker dialog
 * (`cmn_colorpicker_t`).
 */
struct cmn_colorpicker_s {
  cmn_widget_header_t base;            /**< Tagged polymorphic widget header. */
  uint32_t rgba;                       /**< Packed RGBA (`0xRRGGBBAA`) color. */
  cmn_colorpicker_change_fn on_change; /**< Color change callback. */
  void *userdata;                      /**< User data passed to `on_change`. */
};

/**
 * @brief Backend hook creating the native OS control for an extended widget.
 * @param hdr Initialized widget header.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_backend_widget_init(cmn_widget_header_t *hdr);

/**
 * @brief Backend hook synchronizing state changes to the native OS control.
 * @param hdr Widget header whose concrete state was modified.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_backend_widget_sync(cmn_widget_header_t *hdr);

/**
 * @brief Backend hook destroying the native OS control of an extended widget.
 * @param hdr Widget header being destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_backend_widget_cleanup(cmn_widget_header_t *hdr);

/**
 * @brief Initializes a `cmn_widget_header_t` for a newly allocated object.
 * @param header Pointer to the header to initialize.
 * @param kind Concrete widget kind.
 * @param destroy_fn Destructor callback for this widget kind.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if header is NULL or kind is
 * invalid.
 */
CMN_NO_DISCARD cmn_error_e
cmn_widget_header_init(cmn_widget_header_t *header, cmn_widget_kind_e kind,
                       cmn_widget_destroy_fn destroy_fn);

/**
 * @brief Validates a raw pointer as an initialized `cmn_widget_header_t` and
 * retrieves its kind.
 * @param ptr Pointer to inspect.
 * @param out_kind Output pointer receiving the widget kind.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if ptr is NULL or
 * uninitialized.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_get_kind(const void *ptr,
                                               cmn_widget_kind_e *out_kind);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_INTERNAL_H */
