#ifndef CMN_INTERNAL_H
#define CMN_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
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
