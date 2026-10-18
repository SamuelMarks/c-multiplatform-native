#ifndef CMN_LAYOUT_H
#define CMN_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
/* clang-format on */

/**
 * @file layout.h
 * @brief Responsive layout system inspired by CSS Flexbox and CSS Grid.
 */

/**
 * @defgroup cmn_layout Responsive Layout System (Flexbox & Grid)
 * @brief Responsive layout containers, item placement, breakpoints, and window
 * resize adaptation.
 * @{
 */

/**
 * @brief Computed 2D bounding rectangle for a widget or layout cell.
 */
typedef struct cmn_rect_s {
  int x;      /**< Horizontal offset in pixels relative to container. */
  int y;      /**< Vertical offset in pixels relative to container. */
  int width;  /**< Width in pixels. */
  int height; /**< Height in pixels. */
} cmn_rect_t;

/**
 * @brief Four-sided edge insets used for container padding and item margins.
 */
typedef struct cmn_edges_s {
  int top;    /**< Top edge inset in pixels. */
  int right;  /**< Right edge inset in pixels. */
  int bottom; /**< Bottom edge inset in pixels. */
  int left;   /**< Left edge inset in pixels. */
} cmn_edges_t;

/* --- CSS Flexbox-Inspired Types --- */

/**
 * @brief Main-axis direction for a `cmn_flex_t` container (`flex-direction`).
 */
typedef enum cmn_flex_direction_e {
  /** @brief Left-to-right horizontal main axis (`row`). */
  CMN_FLEX_ROW = 0,
  /** @brief Right-to-left horizontal main axis (`row-reverse`). */
  CMN_FLEX_ROW_REVERSE = 1,
  /** @brief Top-to-bottom vertical main axis (`column`). */
  CMN_FLEX_COLUMN = 2,
  /** @brief Bottom-to-top vertical main axis (`column-reverse`). */
  CMN_FLEX_COLUMN_REVERSE = 3
} cmn_flex_direction_e;

/**
 * @brief Line wrapping behavior for a `cmn_flex_t` container (`flex-wrap`).
 */
typedef enum cmn_flex_wrap_e {
  /** @brief All items stay on a single line (`nowrap`). */
  CMN_FLEX_NOWRAP = 0,
  /** @brief Items wrap onto additional lines along the cross axis (`wrap`). */
  CMN_FLEX_WRAP = 1,
  /** @brief Items wrap onto additional lines in reverse cross-axis order
   * (`wrap-reverse`). */
  CMN_FLEX_WRAP_REVERSE = 2
} cmn_flex_wrap_e;

/**
 * @brief Main-axis distribution of items and free space (`justify-content`).
 */
typedef enum cmn_justify_content_e {
  /** @brief Pack items toward the start of the main axis (`flex-start`). */
  CMN_JUSTIFY_FLEX_START = 0,
  /** @brief Pack items toward the end of the main axis (`flex-end`). */
  CMN_JUSTIFY_FLEX_END = 1,
  /** @brief Pack items around the center of the main axis (`center`). */
  CMN_JUSTIFY_CENTER = 2,
  /** @brief Distribute items evenly; first at start, last at end
   * (`space-between`). */
  CMN_JUSTIFY_SPACE_BETWEEN = 3,
  /** @brief Distribute items with equal space around each item
   * (`space-around`). */
  CMN_JUSTIFY_SPACE_AROUND = 4,
  /** @brief Distribute items with equal space between and around all items
   * (`space-evenly`). */
  CMN_JUSTIFY_SPACE_EVENLY = 5
} cmn_justify_content_e;

/**
 * @brief Cross-axis alignment of items within a flex line or grid cell
 * (`align-items` / `align-self`).
 */
typedef enum cmn_align_items_e {
  /** @brief Inherit alignment from parent container (for `align_self`). */
  CMN_ALIGN_AUTO = 0,
  /** @brief Stretch item to fill the cross-axis size (`stretch`). */
  CMN_ALIGN_STRETCH = 1,
  /** @brief Align item at the start of the cross axis (`flex-start`). */
  CMN_ALIGN_FLEX_START = 2,
  /** @brief Align item at the end of the cross axis (`flex-end`). */
  CMN_ALIGN_FLEX_END = 3,
  /** @brief Center item along the cross axis (`center`). */
  CMN_ALIGN_CENTER = 4
} cmn_align_items_e;

/**
 * @brief Per-item layout properties for a child inside a `cmn_flex_t`
 * container.
 */
typedef struct cmn_flex_item_style_s {
  float flex_grow;   /**< Relative growth factor (`flex-grow`, >= 0.0f). */
  float flex_shrink; /**< Relative shrink factor (`flex-shrink`, >= 0.0f). */
  int flex_basis; /**< Initial main-axis size in pixels (`<= 0` uses intrinsic).
                   */
  int min_width;  /**< Minimum width constraint in pixels (`0` for none). */
  int min_height; /**< Minimum height constraint in pixels (`0` for none). */
  int max_width;  /**< Maximum width constraint in pixels (`<= 0` for none). */
  int max_height; /**< Maximum height constraint in pixels (`<= 0` for none). */
  cmn_align_items_e align_self; /**< Per-item cross-axis alignment override. */
  cmn_edges_t margin;           /**< Outer margin insets in pixels. */
} cmn_flex_item_style_t;

/* --- CSS Grid-Inspired Types --- */

/**
 * @brief Sizing unit for a `cmn_grid_t` column or row track.
 */
typedef enum cmn_grid_track_unit_e {
  /** @brief Fixed pixel track size (`px`). */
  CMN_GRID_UNIT_PX = 0,
  /** @brief Fractional share of remaining free space (`fr`). */
  CMN_GRID_UNIT_FR = 1,
  /** @brief Size to largest item's intrinsic dimension in the track (`auto`).
   */
  CMN_GRID_UNIT_AUTO = 2,
  /** @brief Clamped flexible track between `min_px` and `max_px` (`minmax`). */
  CMN_GRID_UNIT_MINMAX = 3
} cmn_grid_track_unit_e;

/**
 * @brief Definition of a single column or row track in a `cmn_grid_t`
 * container.
 */
typedef struct cmn_grid_track_s {
  cmn_grid_track_unit_e
      unit;    /**< Sizing mode (`px`, `fr`, `auto`, `minmax`). */
  float value; /**< Pixel size for `PX` or fractional weight for `FR`. */
  int min_px;  /**< Minimum pixel bound for `CMN_GRID_UNIT_MINMAX`. */
  int max_px;  /**< Maximum pixel bound for `CMN_GRID_UNIT_MINMAX`. */
} cmn_grid_track_t;

/**
 * @brief Per-item placement and alignment properties for a child inside a
 * `cmn_grid_t` container.
 */
typedef struct cmn_grid_item_style_s {
  int row_start; /**< 0-based starting row index (`-1` for auto-placement). */
  int col_start; /**< 0-based starting column index (`-1` for auto-placement).
                  */
  int row_span;  /**< Number of rows spanned (`<= 0` defaults to `1`). */
  int col_span;  /**< Number of columns spanned (`<= 0` defaults to `1`). */
  cmn_align_items_e
      justify_self; /**< Horizontal alignment inside cell (`stretch`, etc.). */
  cmn_align_items_e
      align_self;     /**< Vertical alignment inside cell (`stretch`, etc.). */
  cmn_edges_t margin; /**< Outer margin insets within the grid cell. */
} cmn_grid_item_style_t;

/* --- Flexbox Container API --- */

/**
 * @brief Initializes a `cmn_flex_item_style_t` with standard CSS-like defaults
 * (`flex_grow = 0`, `flex_shrink = 1`, `flex_basis = 0`, `align_self = AUTO`).
 * @param out_style Pointer to the style struct to initialize.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if `out_style` is NULL.
 */
CMN_NO_DISCARD cmn_error_e
cmn_flex_item_style_init(cmn_flex_item_style_t *out_style);

/**
 * @brief Creates a responsive Flexbox layout container (`cmn_flex_t`).
 * @param out_flex Pointer to store the created flex container.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_create(cmn_flex_t **out_flex);

/**
 * @brief Destroys a Flexbox layout container and frees its memory.
 * @param flex Pointer to the flex container pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_destroy(cmn_flex_t **flex);

/**
 * @brief Sets the main-axis direction of a flex container.
 * @param flex The flex container to modify.
 * @param direction The desired `cmn_flex_direction_e`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_flex_set_direction(cmn_flex_t *flex, cmn_flex_direction_e direction);

/**
 * @brief Gets the configured (or active responsive) main-axis direction of a
 * flex container.
 * @param flex The flex container to query.
 * @param out_direction Output pointer receiving the direction.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_get_direction(
    const cmn_flex_t *flex, cmn_flex_direction_e *out_direction);

/**
 * @brief Sets the line-wrapping mode of a flex container.
 * @param flex The flex container to modify.
 * @param wrap The desired `cmn_flex_wrap_e`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_set_wrap(cmn_flex_t *flex,
                                             cmn_flex_wrap_e wrap);

/**
 * @brief Gets the configured (or active responsive) line-wrapping mode of a
 * flex container.
 * @param flex The flex container to query.
 * @param out_wrap Output pointer receiving the wrap mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_get_wrap(const cmn_flex_t *flex,
                                             cmn_flex_wrap_e *out_wrap);

/**
 * @brief Sets the main-axis justification mode of a flex container.
 * @param flex The flex container to modify.
 * @param justify The desired `cmn_justify_content_e`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_flex_set_justify_content(cmn_flex_t *flex, cmn_justify_content_e justify);

/**
 * @brief Gets the main-axis justification mode of a flex container.
 * @param flex The flex container to query.
 * @param out_justify Output pointer receiving the justification mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_get_justify_content(
    const cmn_flex_t *flex, cmn_justify_content_e *out_justify);

/**
 * @brief Sets the default cross-axis alignment for items in a flex container.
 * @param flex The flex container to modify.
 * @param align The desired `cmn_align_items_e`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_set_align_items(cmn_flex_t *flex,
                                                    cmn_align_items_e align);

/**
 * @brief Gets the default cross-axis alignment for items in a flex container.
 * @param flex The flex container to query.
 * @param out_align Output pointer receiving the alignment mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_flex_get_align_items(const cmn_flex_t *flex, cmn_align_items_e *out_align);

/**
 * @brief Sets the row and column gaps (in pixels) between items in a flex
 * container.
 * @param flex The flex container to modify.
 * @param row_gap Vertical gap between lines/items (`>= 0`).
 * @param column_gap Horizontal gap between lines/items (`>= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_set_gap(cmn_flex_t *flex, int row_gap,
                                            int column_gap);

/**
 * @brief Gets the row and column gaps of a flex container.
 * @param flex The flex container to query.
 * @param out_row_gap Output pointer receiving the row gap.
 * @param out_column_gap Output pointer receiving the column gap.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_get_gap(const cmn_flex_t *flex,
                                            int *out_row_gap,
                                            int *out_column_gap);

/**
 * @brief Sets the inner padding insets of a flex container.
 * @param flex The flex container to modify.
 * @param padding Padding insets (`top`, `right`, `bottom`, `left` >= 0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_set_padding(cmn_flex_t *flex,
                                                cmn_edges_t padding);

/**
 * @brief Gets the inner padding insets of a flex container.
 * @param flex The flex container to query.
 * @param out_padding Output pointer receiving the padding insets.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_get_padding(const cmn_flex_t *flex,
                                                cmn_edges_t *out_padding);

/**
 * @brief Adds a child widget to a flex container with optional flex item
 * styling and binds its native parent.
 * @param flex The parent flex container.
 * @param child The child widget (`cmn_button_t *`, `cmn_flex_t *`, etc.).
 * @param style Optional pointer to item style (if NULL, default style is used).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_add_child(
    cmn_flex_t *flex, void *child, const cmn_flex_item_style_t *style);

/**
 * @brief Updates the flex item style of an existing child widget in a flex
 * container.
 * @param flex The flex container.
 * @param child The child widget already added to `flex`.
 * @param style New flex item style to apply.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if `child` is not in
 * `flex`, or CMN_ERR_INVAL on invalid arguments.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_set_child_style(
    cmn_flex_t *flex, const void *child, const cmn_flex_item_style_t *style);

/**
 * @brief Retrieves the computed bounding rectangle of a child widget inside a
 * flex container.
 * @param flex The flex container.
 * @param child The child widget to query.
 * @param out_rect Output pointer receiving the computed `cmn_rect_t`.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if `child` is not in
 * `flex`, or CMN_ERR_INVAL on NULL arguments.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_get_child_rect(const cmn_flex_t *flex,
                                                   const void *child,
                                                   cmn_rect_t *out_rect);

/**
 * @brief Registers a responsive breakpoint rule on a flex container so that
 * when `available_width <= max_width`, the container automatically switches to
 * the specified `direction` and `wrap` mode (e.g., row -> column on narrow
 * windows).
 * @param flex The flex container to configure.
 * @param max_width Maximum container width in pixels for this breakpoint (`>
 * 0`).
 * @param direction Flex direction to activate when width `<= max_width`.
 * @param wrap Flex wrap mode to activate when width `<= max_width`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_flex_add_breakpoint(cmn_flex_t *flex, int max_width,
                        cmn_flex_direction_e direction, cmn_flex_wrap_e wrap);

/**
 * @brief Computes the responsive Flexbox layout for the given available width
 * and height, updating all child rectangles, nested layout containers, and
 * native widget bounds.
 * @param flex The flex container to layout.
 * @param available_width Available width in pixels (`>= 0`).
 * @param available_height Available height in pixels (`>= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_compute_layout(cmn_flex_t *flex,
                                                   int available_width,
                                                   int available_height);

/* --- Grid Container API --- */

/**
 * @brief Initializes a `cmn_grid_item_style_t` with default auto-placement
 * (`row_start = -1`, `col_start = -1`, `row_span = 1`, `col_span = 1`,
 * `justify_self = STRETCH`, `align_self = STRETCH`).
 * @param out_style Pointer to the grid item style struct to initialize.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if `out_style` is NULL.
 */
CMN_NO_DISCARD cmn_error_e
cmn_grid_item_style_init(cmn_grid_item_style_t *out_style);

/**
 * @brief Creates a responsive CSS Grid-inspired layout container
 * (`cmn_grid_t`).
 * @param out_grid Pointer to store the created grid container.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_create(cmn_grid_t **out_grid);

/**
 * @brief Destroys a Grid layout container and frees its memory.
 * @param grid Pointer to the grid container pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_destroy(cmn_grid_t **grid);

/**
 * @brief Defines the explicit column track template (`grid-template-columns`)
 * for a grid container.
 * @param grid The grid container to modify.
 * @param tracks Array of `count` column track definitions.
 * @param count Number of column tracks (`1 .. 32`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_set_columns(cmn_grid_t *grid,
                                                const cmn_grid_track_t *tracks,
                                                size_t count);

/**
 * @brief Defines the explicit row track template (`grid-template-rows`) for a
 * grid container.
 * @param grid The grid container to modify.
 * @param tracks Array of `count` row track definitions.
 * @param count Number of row tracks (`1 .. 32`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_set_rows(cmn_grid_t *grid,
                                             const cmn_grid_track_t *tracks,
                                             size_t count);

/**
 * @brief Configures responsive `repeat(auto-fit, minmax(min_col_width, 1fr))`
 * columns that automatically adjust the number of columns as the container or
 * window is resized.
 * @param grid The grid container to modify.
 * @param min_col_width Minimum column width in pixels (`> 0`).
 * @param max_cols Maximum number of columns (`> 0`, `<= 32`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_set_auto_fit_columns(cmn_grid_t *grid,
                                                         int min_col_width,
                                                         int max_cols);

/**
 * @brief Sets the row and column gaps (in pixels) between tracks in a grid
 * container.
 * @param grid The grid container to modify.
 * @param row_gap Vertical gap between rows (`>= 0`).
 * @param column_gap Horizontal gap between columns (`>= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_set_gap(cmn_grid_t *grid, int row_gap,
                                            int column_gap);

/**
 * @brief Gets the row and column gaps of a grid container.
 * @param grid The grid container to query.
 * @param out_row_gap Output pointer receiving the row gap.
 * @param out_column_gap Output pointer receiving the column gap.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_get_gap(const cmn_grid_t *grid,
                                            int *out_row_gap,
                                            int *out_column_gap);

/**
 * @brief Sets the inner padding insets of a grid container.
 * @param grid The grid container to modify.
 * @param padding Padding insets (`top`, `right`, `bottom`, `left` >= 0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_set_padding(cmn_grid_t *grid,
                                                cmn_edges_t padding);

/**
 * @brief Gets the inner padding insets of a grid container.
 * @param grid The grid container to query.
 * @param out_padding Output pointer receiving the padding insets.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_get_padding(const cmn_grid_t *grid,
                                                cmn_edges_t *out_padding);

/**
 * @brief Adds a child widget to a grid container with optional cell placement /
 * span styling and binds its native parent.
 * @param grid The parent grid container.
 * @param child The child widget to add.
 * @param style Optional pointer to grid item style (if NULL, auto-placement is
 * used).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_add_child(
    cmn_grid_t *grid, void *child, const cmn_grid_item_style_t *style);

/**
 * @brief Updates the grid placement style of an existing child widget in a grid
 * container.
 * @param grid The grid container.
 * @param child The child widget already added to `grid`.
 * @param style New grid item style to apply.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if `child` is not in
 * `grid`, or CMN_ERR_INVAL on invalid arguments.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_set_child_style(
    cmn_grid_t *grid, const void *child, const cmn_grid_item_style_t *style);

/**
 * @brief Retrieves the computed bounding rectangle of a child widget inside a
 * grid container.
 * @param grid The grid container.
 * @param child The child widget to query.
 * @param out_rect Output pointer receiving the computed `cmn_rect_t`.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if `child` is not in
 * `grid`, or CMN_ERR_INVAL on NULL arguments.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_get_child_rect(const cmn_grid_t *grid,
                                                   const void *child,
                                                   cmn_rect_t *out_rect);

/**
 * @brief Registers a responsive breakpoint rule on a grid container that
 * overrides the number of equal-width (`1fr`) columns when `available_width <=
 * max_width`.
 * @param grid The grid container to configure.
 * @param max_width Maximum container width in pixels for this breakpoint (`>
 * 0`).
 * @param col_count Number of columns (`1 .. 32`) when width `<= max_width`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_add_breakpoint(cmn_grid_t *grid,
                                                   int max_width,
                                                   size_t col_count);

/**
 * @brief Gets the effective number of columns after the most recent layout
 * pass.
 * @param grid The grid container to query.
 * @param out_col_count Output pointer receiving the active column count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_grid_get_effective_columns(const cmn_grid_t *grid, size_t *out_col_count);

/**
 * @brief Computes the responsive CSS Grid layout for the given available width
 * and height, updating all child rectangles, nested layout containers, and
 * native widget bounds.
 * @param grid The grid container to layout.
 * @param available_width Available width in pixels (`>= 0`).
 * @param available_height Available height in pixels (`>= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_compute_layout(cmn_grid_t *grid,
                                                   int available_width,
                                                   int available_height);

/* --- Upcasts, Bounds, and Window Resize Integration --- */

/**
 * @brief Upcasts a `cmn_flex_t` to a `cmn_container_t`.
 * @param flex The flex container instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e
cmn_flex_as_container(cmn_flex_t *flex, cmn_container_t **out_container);

/**
 * @brief Upcasts a `cmn_flex_t` to a `cmn_widget_t`.
 * @param flex The flex container instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e cmn_flex_as_widget(cmn_flex_t *flex,
                                              cmn_widget_t **out_widget);

/**
 * @brief Upcasts a `cmn_grid_t` to a `cmn_container_t`.
 * @param grid The grid container instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e
cmn_grid_as_container(cmn_grid_t *grid, cmn_container_t **out_container);

/**
 * @brief Upcasts a `cmn_grid_t` to a `cmn_widget_t`.
 * @param grid The grid container instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e cmn_grid_as_widget(cmn_grid_t *grid,
                                              cmn_widget_t **out_widget);

/**
 * @brief Sets the explicit or intrinsic bounding rectangle of any widget.
 * @param widget The widget to update.
 * @param bounds The bounding rectangle (`width >= 0`, `height >= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_set_bounds(void *widget,
                                                 cmn_rect_t bounds);

/**
 * @brief Gets the current computed or configured bounding rectangle of any
 * widget.
 * @param widget The widget to query.
 * @param out_bounds Output pointer receiving the `cmn_rect_t`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_get_bounds(const void *widget,
                                                 cmn_rect_t *out_bounds);

/**
 * @brief Callback function type invoked when a window is resized.
 * @param window The resized window.
 * @param width New client width in pixels.
 * @param height New client height in pixels.
 * @param userdata User data pointer registered with `cmn_window_set_on_resize`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_window_resize_fn)(cmn_window_t *window, int width,
                                            int height, void *userdata);

/**
 * @brief Attaches a root responsive layout container (`cmn_flex_t *` or
 * `cmn_grid_t *`) to a window and immediately computes its layout for the
 * window's current size. Subsequent window resizes automatically recompute the
 * layout.
 * @param window The target window.
 * @param layout_container The root `cmn_flex_t *` or `cmn_grid_t *` container.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_layout(cmn_window_t *window,
                                                 void *layout_container);

/**
 * @brief Resizes a window and automatically recomputes any attached responsive
 * layout (`cmn_flex_t` or `cmn_grid_t`) and invokes any registered resize
 * callback.
 * @param window The window to resize.
 * @param width New width in pixels (`> 0`).
 * @param height New height in pixels (`> 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_size(cmn_window_t *window, int width,
                                               int height);

/**
 * @brief Gets the current client dimensions of a window.
 * @param window The window to query.
 * @param out_width Output pointer receiving the width in pixels.
 * @param out_height Output pointer receiving the height in pixels.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_get_size(const cmn_window_t *window,
                                               int *out_width, int *out_height);

/**
 * @brief Registers a callback to be invoked whenever the window is resized.
 * @param window The window to observe.
 * @param on_resize Callback function (or NULL to clear).
 * @param userdata User data pointer passed to `on_resize`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_on_resize(
    cmn_window_t *window, cmn_window_resize_fn on_resize, void *userdata);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_LAYOUT_H */
