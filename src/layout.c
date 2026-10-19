/* clang-format off */
#include "cmn_internal.h"
#include "cmn/layout.h"
#include "cmn/widget.h"
#include "cmn/memory.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file layout.c
 * @brief Implementation of the responsive layout system inspired by CSS Flexbox
 * and CSS Grid, with breakpoint adaptation and dynamic window resize reflow.
 */

/**
 * @brief Polymorphic destructor adapter for `cmn_flex_t`.
 * @param ptr Address of the flex pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_flex_adapter(void **ptr) {
  return cmn_flex_destroy((cmn_flex_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_grid_t`.
 * @param ptr Address of the grid pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_grid_adapter(void **ptr) {
  return cmn_grid_destroy((cmn_grid_t **)ptr);
}

/**
 * @brief Clamps a value to `[min_val, max_val]` when constraints are active.
 * @param val Input value in pixels.
 * @param min_val Minimum bound (`> 0` to apply).
 * @param max_val Maximum bound (`> 0` to apply).
 * @param out_clamped Output pointer receiving the clamped value.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e clamp_dimension(int val, int min_val, int max_val,
                                   int *out_clamped) {
  int res = val;
  if (min_val > 0 && res < min_val) {
    res = min_val;
  }
  if (max_val > 0 && res > max_val) {
    res = max_val;
  }
  if (res < 0) {
    res = 0;
  }
  *out_clamped = res;
  return CMN_SUCCESS;
}

/**
 * @brief Propagates a newly computed bounding box to a child widget and
 * recursively triggers layout if the child is itself a `cmn_flex_t` or
 * `cmn_grid_t`.
 * @param child The child widget handle.
 * @param rect The computed bounding rectangle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e apply_child_layout(void *child, cmn_rect_t rect) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_error_e rc;
  cmn_error_e kind_rc;
  rc = cmn_widget_set_bounds(child, rect);
  kind_rc = cmn_widget_get_kind(child, &kind);
  rc = (rc != CMN_SUCCESS) ? rc : kind_rc;
  if (kind == CMN_WIDGET_KIND_FLEX) {
    cmn_error_e flex_rc =
        cmn_flex_compute_layout((cmn_flex_t *)child, rect.width, rect.height);
    rc = (rc != CMN_SUCCESS) ? rc : flex_rc;
  } else if (kind == CMN_WIDGET_KIND_GRID) {
    cmn_error_e grid_rc =
        cmn_grid_compute_layout((cmn_grid_t *)child, rect.width, rect.height);
    rc = (rc != CMN_SUCCESS) ? rc : grid_rc;
  }
  return rc;
}

/* --- Flexbox Implementation --- */

/**
 * @brief Initializes a `cmn_flex_item_style_t` with standard CSS-like defaults
 * (`flex_grow = 0`, `flex_shrink = 1`, `flex_basis = 0`, `align_self = AUTO`).
 * @param out_style Pointer to the style struct to initialize.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if `out_style` is NULL.
 */
cmn_error_e cmn_flex_item_style_init(cmn_flex_item_style_t *out_style) {
  if (out_style == NULL) {
    return CMN_ERR_INVAL;
  }
  out_style->flex_grow = 0.0f;
  out_style->flex_shrink = 1.0f;
  out_style->flex_basis = 0;
  out_style->min_width = 0;
  out_style->min_height = 0;
  out_style->max_width = 0;
  out_style->max_height = 0;
  out_style->align_self = CMN_ALIGN_AUTO;
  out_style->margin.top = 0;
  out_style->margin.right = 0;
  out_style->margin.bottom = 0;
  out_style->margin.left = 0;
  return CMN_SUCCESS;
}

/**
 * @brief Creates a responsive Flexbox layout container (`cmn_flex_t`).
 * @param out_flex Pointer to store the created flex container.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_create(cmn_flex_t **out_flex) {
  cmn_flex_t *flex = NULL;
  cmn_error_e rc;
  if (out_flex == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_flex_t), (void **)&flex);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = cmn_widget_header_init(&flex->base, CMN_WIDGET_KIND_FLEX,
                              destroy_flex_adapter);
  flex->direction = CMN_FLEX_ROW;
  flex->active_dir = CMN_FLEX_ROW;
  flex->wrap = CMN_FLEX_NOWRAP;
  flex->active_wrap = CMN_FLEX_NOWRAP;
  flex->justify = CMN_JUSTIFY_FLEX_START;
  flex->align_items = CMN_ALIGN_STRETCH;
  *out_flex = flex;
  return rc;
}

/**
 * @brief Destroys a Flexbox layout container and frees its memory.
 * @param flex Pointer to the flex container pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_destroy(cmn_flex_t **flex) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || *flex == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(*flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)flex);
}

/**
 * @brief Sets the main-axis direction of a flex container.
 * @param flex The flex container to modify.
 * @param direction The desired `cmn_flex_direction_e`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_set_direction(cmn_flex_t *flex,
                                   cmn_flex_direction_e direction) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || direction < CMN_FLEX_ROW ||
      direction > CMN_FLEX_COLUMN_REVERSE) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  flex->direction = direction;
  flex->active_dir = direction;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the configured (or active responsive) main-axis direction of a
 * flex container.
 * @param flex The flex container to query.
 * @param out_direction Output pointer receiving the direction.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_get_direction(const cmn_flex_t *flex,
                                   cmn_flex_direction_e *out_direction) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || out_direction == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  *out_direction = flex->active_dir;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the line-wrapping mode of a flex container.
 * @param flex The flex container to modify.
 * @param wrap The desired `cmn_flex_wrap_e`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_set_wrap(cmn_flex_t *flex, cmn_flex_wrap_e wrap) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || wrap < CMN_FLEX_NOWRAP || wrap > CMN_FLEX_WRAP_REVERSE) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  flex->wrap = wrap;
  flex->active_wrap = wrap;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the configured (or active responsive) line-wrapping mode of a
 * flex container.
 * @param flex The flex container to query.
 * @param out_wrap Output pointer receiving the wrap mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_get_wrap(const cmn_flex_t *flex,
                              cmn_flex_wrap_e *out_wrap) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || out_wrap == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  *out_wrap = flex->active_wrap;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the main-axis justification mode of a flex container.
 * @param flex The flex container to modify.
 * @param justify The desired `cmn_justify_content_e`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_set_justify_content(cmn_flex_t *flex,
                                         cmn_justify_content_e justify) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || justify < CMN_JUSTIFY_FLEX_START ||
      justify > CMN_JUSTIFY_SPACE_EVENLY) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  flex->justify = justify;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the main-axis justification mode of a flex container.
 * @param flex The flex container to query.
 * @param out_justify Output pointer receiving the justification mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_get_justify_content(const cmn_flex_t *flex,
                                         cmn_justify_content_e *out_justify) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || out_justify == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  *out_justify = flex->justify;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the default cross-axis alignment for items in a flex container.
 * @param flex The flex container to modify.
 * @param align The desired `cmn_align_items_e`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_set_align_items(cmn_flex_t *flex,
                                     cmn_align_items_e align) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || align < CMN_ALIGN_STRETCH || align > CMN_ALIGN_CENTER) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  flex->align_items = align;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the default cross-axis alignment for items in a flex container.
 * @param flex The flex container to query.
 * @param out_align Output pointer receiving the alignment mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_get_align_items(const cmn_flex_t *flex,
                                     cmn_align_items_e *out_align) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || out_align == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  *out_align = flex->align_items;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the row and column gaps (in pixels) between items in a flex
 * container.
 * @param flex The flex container to modify.
 * @param row_gap Vertical gap between lines/items (`>= 0`).
 * @param column_gap Horizontal gap between lines/items (`>= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_set_gap(cmn_flex_t *flex, int row_gap, int column_gap) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || row_gap < 0 || column_gap < 0) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  flex->row_gap = row_gap;
  flex->column_gap = column_gap;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the row and column gaps of a flex container.
 * @param flex The flex container to query.
 * @param out_row_gap Output pointer receiving the row gap.
 * @param out_column_gap Output pointer receiving the column gap.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_get_gap(const cmn_flex_t *flex, int *out_row_gap,
                             int *out_column_gap) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || out_row_gap == NULL || out_column_gap == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  *out_row_gap = flex->row_gap;
  *out_column_gap = flex->column_gap;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the inner padding insets of a flex container.
 * @param flex The flex container to modify.
 * @param padding Padding insets (`top`, `right`, `bottom`, `left` >= 0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_set_padding(cmn_flex_t *flex, cmn_edges_t padding) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || padding.top < 0 || padding.right < 0 ||
      padding.bottom < 0 || padding.left < 0) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  flex->padding = padding;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the inner padding insets of a flex container.
 * @param flex The flex container to query.
 * @param out_padding Output pointer receiving the padding insets.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_get_padding(const cmn_flex_t *flex,
                                 cmn_edges_t *out_padding) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || out_padding == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  *out_padding = flex->padding;
  return CMN_SUCCESS;
}

/**
 * @brief Validates a `cmn_flex_item_style_t` structure.
 * @param style Pointer to style to validate.
 * @return CMN_SUCCESS if valid, CMN_ERR_INVAL otherwise.
 */
static cmn_error_e
validate_flex_item_style(const cmn_flex_item_style_t *style) {
  if (style->flex_grow < 0.0f || style->flex_shrink < 0.0f ||
      style->min_width < 0 || style->min_height < 0 ||
      style->align_self < CMN_ALIGN_AUTO ||
      style->align_self > CMN_ALIGN_CENTER || style->margin.top < 0 ||
      style->margin.right < 0 || style->margin.bottom < 0 ||
      style->margin.left < 0) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Adds a child widget to a flex container with optional flex item
 * styling and binds its native parent.
 * @param flex The parent flex container.
 * @param child The child widget (`cmn_button_t *`, `cmn_flex_t *`, etc.).
 * @param style Optional pointer to item style (if NULL, default style is used).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_add_child(cmn_flex_t *flex, void *child,
                               const cmn_flex_item_style_t *style) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_flex_item_style_t resolved_style;
  cmn_error_e rc;

  if (flex == NULL || child == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_flex_item_style_init(&resolved_style);
  if (style != NULL && rc == CMN_SUCCESS) {
    rc = validate_flex_item_style(style);
    resolved_style = *style;
  }
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  if (flex->child_count >= CMN_LAYOUT_MAX_CHILDREN) {
    return CMN_ERR_NOMEM;
  }

  rc = cmn_widget_set_parent(flex, child);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  flex->children[flex->child_count].widget = child;
  flex->children[flex->child_count].style = resolved_style;
  rc = cmn_widget_get_bounds(child,
                             &flex->children[flex->child_count].intrinsic_rect);
  flex->children[flex->child_count].computed_rect =
      flex->children[flex->child_count].intrinsic_rect;
  flex->child_count++;
  return rc;
}

/**
 * @brief Updates the flex item style of an existing child widget in a flex
 * container.
 * @param flex The flex container.
 * @param child The child widget already added to `flex`.
 * @param style New flex item style to apply.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if `child` is not in
 * `flex`, or CMN_ERR_INVAL on invalid arguments.
 */
cmn_error_e cmn_flex_set_child_style(cmn_flex_t *flex, const void *child,
                                     const cmn_flex_item_style_t *style) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  if (flex == NULL || child == NULL || style == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  if (validate_flex_item_style(style) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  for (i = 0; i < flex->child_count; ++i) {
    if (flex->children[i].widget == child) {
      flex->children[i].style = *style;
      return CMN_SUCCESS;
    }
  }
  return CMN_ERR_NOT_FOUND;
}

/**
 * @brief Retrieves the computed bounding rectangle of a child widget inside a
 * flex container.
 * @param flex The flex container.
 * @param child The child widget to query.
 * @param out_rect Output pointer receiving the computed `cmn_rect_t`.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if `child` is not in
 * `flex`, or CMN_ERR_INVAL on NULL arguments.
 */
cmn_error_e cmn_flex_get_child_rect(const cmn_flex_t *flex, const void *child,
                                    cmn_rect_t *out_rect) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  if (flex == NULL || child == NULL || out_rect == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  for (i = 0; i < flex->child_count; ++i) {
    if (flex->children[i].widget == child) {
      *out_rect = flex->children[i].computed_rect;
      return CMN_SUCCESS;
    }
  }
  return CMN_ERR_NOT_FOUND;
}

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
cmn_error_e cmn_flex_add_breakpoint(cmn_flex_t *flex, int max_width,
                                    cmn_flex_direction_e direction,
                                    cmn_flex_wrap_e wrap) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (flex == NULL || max_width <= 0 || direction < CMN_FLEX_ROW ||
      direction > CMN_FLEX_COLUMN_REVERSE || wrap < CMN_FLEX_NOWRAP ||
      wrap > CMN_FLEX_WRAP_REVERSE) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }
  if (flex->breakpoint_count >= CMN_LAYOUT_MAX_BREAKPOINTS) {
    return CMN_ERR_NOMEM;
  }
  flex->breakpoints[flex->breakpoint_count].max_width = max_width;
  flex->breakpoints[flex->breakpoint_count].dir = direction;
  flex->breakpoints[flex->breakpoint_count].wrap = wrap;
  flex->breakpoint_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Internal structure representing a single line of flex items during
 * `cmn_flex_compute_layout`.
 */
struct cmn_flex_line_s {
  size_t start_idx; /**< Index of first child in this line. */
  size_t count;     /**< Number of children in this line. */
  int cross_size;   /**< Computed cross-axis size of this line. */
};

/**
 * @brief Computes the responsive Flexbox layout for the given available width
 * and height, updating all child rectangles, nested layout containers, and
 * native widget bounds.
 * @param flex The flex container to layout.
 * @param available_width Available width in pixels (`>= 0`).
 * @param available_height Available height in pixels (`>= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_flex_compute_layout(cmn_flex_t *flex, int available_width,
                                    int available_height) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_flex_direction_e dir;
  cmn_flex_wrap_e wrap;
  int best_bp_width = 0;
  int is_row;
  int is_rev;
  int main_avail;
  int cross_avail;
  int main_gap;
  int cross_gap;
  struct cmn_flex_line_s lines[CMN_LAYOUT_MAX_CHILDREN];
  size_t line_count = 0;
  int item_main[CMN_LAYOUT_MAX_CHILDREN];
  int item_cross[CMN_LAYOUT_MAX_CHILDREN];
  size_t i;
  size_t l;
  int cross_cursor;
  int total_lines_cross = 0;
  cmn_error_e rc = CMN_SUCCESS;

  if (flex == NULL || available_width < 0 || available_height < 0) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(flex, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_FLEX) {
    return CMN_ERR_INVAL;
  }

  flex->base.bounds.width = available_width;
  flex->base.bounds.height = available_height;

  /* 1. Evaluate responsive breakpoints (pick smallest matching max_width) */
  dir = flex->direction;
  wrap = flex->wrap;
  for (i = 0; i < flex->breakpoint_count; ++i) {
    if (available_width <= flex->breakpoints[i].max_width) {
      if (best_bp_width == 0 ||
          flex->breakpoints[i].max_width < best_bp_width) {
        best_bp_width = flex->breakpoints[i].max_width;
        dir = flex->breakpoints[i].dir;
        wrap = flex->breakpoints[i].wrap;
      }
    }
  }
  flex->active_dir = dir;
  flex->active_wrap = wrap;

  if (flex->child_count == 0) {
    return CMN_SUCCESS;
  }

  is_row = (dir == CMN_FLEX_ROW || dir == CMN_FLEX_ROW_REVERSE) ? 1 : 0;
  is_rev =
      (dir == CMN_FLEX_ROW_REVERSE || dir == CMN_FLEX_COLUMN_REVERSE) ? 1 : 0;

  main_avail =
      is_row ? (available_width - flex->padding.left - flex->padding.right)
             : (available_height - flex->padding.top - flex->padding.bottom);
  cross_avail =
      is_row ? (available_height - flex->padding.top - flex->padding.bottom)
             : (available_width - flex->padding.left - flex->padding.right);
  if (main_avail < 0) {
    main_avail = 0;
  }
  if (cross_avail < 0) {
    cross_avail = 0;
  }

  main_gap = is_row ? flex->column_gap : flex->row_gap;
  cross_gap = is_row ? flex->row_gap : flex->column_gap;

  /* 2. Determine initial main/cross sizes of each child */
  for (i = 0; i < flex->child_count; ++i) {
    const cmn_rect_t *intr = &flex->children[i].intrinsic_rect;
    const cmn_flex_item_style_t *st = &flex->children[i].style;
    int intrinsic_main = is_row ? intr->width : intr->height;
    int intrinsic_cross = is_row ? intr->height : intr->width;
    int base_m = (st->flex_basis > 0) ? st->flex_basis : intrinsic_main;

    {
      cmn_error_e c1_rc = clamp_dimension(
          base_m, is_row ? st->min_width : st->min_height,
          is_row ? st->max_width : st->max_height, &item_main[i]);
      cmn_error_e c2_rc = clamp_dimension(
          intrinsic_cross, is_row ? st->min_height : st->min_width,
          is_row ? st->max_height : st->max_width, &item_cross[i]);
      rc = (c1_rc != CMN_SUCCESS) ? c1_rc : c2_rc;
    }
  }

  /* 3. Partition items into flex lines */
  {
    size_t line_start = 0;
    int accum_main = 0;
    for (i = 0; i < flex->child_count; ++i) {
      const cmn_flex_item_style_t *st = &flex->children[i].style;
      int margin_main = is_row ? (st->margin.left + st->margin.right)
                               : (st->margin.top + st->margin.bottom);
      int outer_main = item_main[i] + margin_main;
      int gap_add = (i > line_start) ? main_gap : 0;

      if (wrap != CMN_FLEX_NOWRAP && i > line_start &&
          (accum_main + gap_add + outer_main) > main_avail) {
        lines[line_count].start_idx = line_start;
        lines[line_count].count = i - line_start;
        lines[line_count].cross_size = 0;
        line_count++;
        line_start = i;
        accum_main = outer_main;
      } else {
        accum_main += gap_add + outer_main;
      }
    }
    lines[line_count].start_idx = line_start;
    lines[line_count].count = flex->child_count - line_start;
    lines[line_count].cross_size = 0;
    line_count++;
  }

  /* 4. Resolve flexible lengths (grow / shrink) and line cross sizes */
  for (l = 0; l < line_count; ++l) {
    size_t s = lines[l].start_idx;
    size_t c = lines[l].count;
    int used_main = 0;
    float total_grow = 0.0f;
    float total_shrink = 0.0f;
    int max_cross = 0;
    int free_space;

    for (i = s; i < s + c; ++i) {
      const cmn_flex_item_style_t *st = &flex->children[i].style;
      int margin_main = is_row ? (st->margin.left + st->margin.right)
                               : (st->margin.top + st->margin.bottom);
      used_main += item_main[i] + margin_main;
      if (i > s) {
        used_main += main_gap;
      }
      total_grow += st->flex_grow;
      total_shrink += st->flex_shrink;
    }

    free_space = main_avail - used_main;
    if (free_space > 0 && total_grow > 0.0f) {
      int distributed = 0;
      size_t last_grow_idx = s;
      for (i = s; i < s + c; ++i) {
        if (flex->children[i].style.flex_grow > 0.0f) {
          last_grow_idx = i;
        }
      }
      for (i = s; i < s + c; ++i) {
        const cmn_flex_item_style_t *st = &flex->children[i].style;
        if (st->flex_grow > 0.0f) {
          int share =
              (i == last_grow_idx)
                  ? (free_space - distributed)
                  : (int)(((float)free_space * st->flex_grow) / total_grow);
          distributed += share;
          rc = clamp_dimension(
              item_main[i] + share, is_row ? st->min_width : st->min_height,
              is_row ? st->max_width : st->max_height, &item_main[i]);
        }
      }
    } else if (free_space < 0 && total_shrink > 0.0f) {
      int deficit = -free_space;
      int shrunk = 0;
      size_t last_shrink_idx = s;
      for (i = s; i < s + c; ++i) {
        if (flex->children[i].style.flex_shrink > 0.0f) {
          last_shrink_idx = i;
        }
      }
      for (i = s; i < s + c; ++i) {
        const cmn_flex_item_style_t *st = &flex->children[i].style;
        if (st->flex_shrink > 0.0f) {
          int share =
              (i == last_shrink_idx)
                  ? (deficit - shrunk)
                  : (int)(((float)deficit * st->flex_shrink) / total_shrink);
          shrunk += share;
          rc = clamp_dimension(
              item_main[i] - share, is_row ? st->min_width : st->min_height,
              is_row ? st->max_width : st->max_height, &item_main[i]);
        }
      }
    }

    for (i = s; i < s + c; ++i) {
      const cmn_flex_item_style_t *st = &flex->children[i].style;
      int margin_cross = is_row ? (st->margin.top + st->margin.bottom)
                                : (st->margin.left + st->margin.right);
      int outer_cross = item_cross[i] + margin_cross;
      if (outer_cross > max_cross) {
        max_cross = outer_cross;
      }
    }

    if (line_count == 1 && cross_avail > 0) {
      lines[l].cross_size = cross_avail;
    } else {
      lines[l].cross_size = max_cross;
    }
    total_lines_cross += lines[l].cross_size;
    if (l > 0) {
      total_lines_cross += cross_gap;
    }
  }

  /* 5. Position items along main and cross axes */
  cross_cursor = is_row ? flex->padding.top : flex->padding.left;
  if (wrap == CMN_FLEX_WRAP_REVERSE && cross_avail > total_lines_cross) {
    cross_cursor += (cross_avail - total_lines_cross);
  }

  for (l = 0; l < line_count; ++l) {
    size_t line_idx =
        (wrap == CMN_FLEX_WRAP_REVERSE) ? (line_count - 1u - l) : l;
    size_t s = lines[line_idx].start_idx;
    size_t c = lines[line_idx].count;
    int line_cross = lines[line_idx].cross_size;
    int used_main = 0;
    int rem_main;
    int main_start_offset = 0;
    int extra_gap = 0;
    int main_cursor;
    size_t k;

    for (i = s; i < s + c; ++i) {
      const cmn_flex_item_style_t *st = &flex->children[i].style;
      int margin_main = is_row ? (st->margin.left + st->margin.right)
                               : (st->margin.top + st->margin.bottom);
      used_main += item_main[i] + margin_main;
      if (i > s) {
        used_main += main_gap;
      }
    }

    rem_main = main_avail - used_main;
    if (rem_main < 0) {
      rem_main = 0;
    }

    switch (flex->justify) {
    case CMN_JUSTIFY_FLEX_END:
      main_start_offset = rem_main;
      break;
    case CMN_JUSTIFY_CENTER:
      main_start_offset = rem_main / 2;
      break;
    case CMN_JUSTIFY_SPACE_BETWEEN:
      if (c > 1) {
        extra_gap = rem_main / (int)(c - 1);
      }
      break;
    case CMN_JUSTIFY_SPACE_AROUND:
      if (c > 0) {
        extra_gap = rem_main / (int)c;
        main_start_offset = extra_gap / 2;
      }
      break;
    case CMN_JUSTIFY_SPACE_EVENLY:
      if (c > 0) {
        extra_gap = rem_main / (int)(c + 1);
        main_start_offset = extra_gap;
      }
      break;
    case CMN_JUSTIFY_FLEX_START:
    default:
      main_start_offset = 0;
      break;
    }

    main_cursor =
        (is_row ? flex->padding.left : flex->padding.top) + main_start_offset;

    for (k = 0; k < c; ++k) {
      size_t idx = is_rev ? (s + c - 1u - k) : (s + k);
      const cmn_flex_item_style_t *st = &flex->children[idx].style;
      cmn_align_items_e align = (st->align_self != CMN_ALIGN_AUTO)
                                    ? st->align_self
                                    : flex->align_items;
      int m_lead = is_row ? st->margin.left : st->margin.top;
      int m_trail = is_row ? st->margin.right : st->margin.bottom;
      int c_lead = is_row ? st->margin.top : st->margin.left;
      int c_trail = is_row ? st->margin.bottom : st->margin.right;
      int resolved_cross = item_cross[idx];
      int cross_pos = cross_cursor + c_lead;
      int avail_item_cross = line_cross - c_lead - c_trail;
      cmn_rect_t rect;

      if (avail_item_cross < 0) {
        avail_item_cross = 0;
      }

      if (align == CMN_ALIGN_STRETCH) {
        rc = clamp_dimension(
            avail_item_cross, is_row ? st->min_height : st->min_width,
            is_row ? st->max_height : st->max_width, &resolved_cross);
      } else if (align == CMN_ALIGN_FLEX_END) {
        cross_pos = cross_cursor + line_cross - c_trail - resolved_cross;
      } else if (align == CMN_ALIGN_CENTER) {
        cross_pos =
            cross_cursor + c_lead + (avail_item_cross - resolved_cross) / 2;
      }

      if (is_row) {
        rect.x = main_cursor + m_lead;
        rect.y = cross_pos;
        rect.width = item_main[idx];
        rect.height = resolved_cross;
      } else {
        rect.x = cross_pos;
        rect.y = main_cursor + m_lead;
        rect.width = resolved_cross;
        rect.height = item_main[idx];
      }

      flex->children[idx].computed_rect = rect;
      {
        cmn_error_e child_rc =
            apply_child_layout(flex->children[idx].widget, rect);
        rc = (rc != CMN_SUCCESS) ? rc : child_rc;
      }
      main_cursor += m_lead + item_main[idx] + m_trail + main_gap + extra_gap;
    }

    cross_cursor += line_cross + cross_gap;
  }

  return rc;
}

/* --- Grid Implementation --- */

/**
 * @brief Initializes a `cmn_grid_item_style_t` with default auto-placement
 * (`row_start = -1`, `col_start = -1`, `row_span = 1`, `col_span = 1`,
 * `justify_self = STRETCH`, `align_self = STRETCH`).
 * @param out_style Pointer to the grid item style struct to initialize.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if `out_style` is NULL.
 */
cmn_error_e cmn_grid_item_style_init(cmn_grid_item_style_t *out_style) {
  if (out_style == NULL) {
    return CMN_ERR_INVAL;
  }
  out_style->row_start = -1;
  out_style->col_start = -1;
  out_style->row_span = 1;
  out_style->col_span = 1;
  out_style->justify_self = CMN_ALIGN_STRETCH;
  out_style->align_self = CMN_ALIGN_STRETCH;
  out_style->margin.top = 0;
  out_style->margin.right = 0;
  out_style->margin.bottom = 0;
  out_style->margin.left = 0;
  return CMN_SUCCESS;
}

/**
 * @brief Creates a responsive CSS Grid-inspired layout container
 * (`cmn_grid_t`).
 * @param out_grid Pointer to store the created grid container.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_create(cmn_grid_t **out_grid) {
  cmn_grid_t *grid = NULL;
  cmn_error_e rc;
  if (out_grid == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_grid_t), (void **)&grid);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = cmn_widget_header_init(&grid->base, CMN_WIDGET_KIND_GRID,
                              destroy_grid_adapter);
  grid->col_tracks[0].unit = CMN_GRID_UNIT_FR;
  grid->col_tracks[0].value = 1.0f;
  grid->col_count = 1;
  grid->effective_col_count = 1;
  *out_grid = grid;
  return rc;
}

/**
 * @brief Destroys a Grid layout container and frees its memory.
 * @param grid Pointer to the grid container pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_destroy(cmn_grid_t **grid) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (grid == NULL || *grid == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(*grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)grid);
}

/**
 * @brief Validates an array of `cmn_grid_track_t` definitions.
 * @param tracks Track array.
 * @param count Number of tracks.
 * @return CMN_SUCCESS if valid, CMN_ERR_INVAL otherwise.
 */
static cmn_error_e validate_grid_tracks(const cmn_grid_track_t *tracks,
                                        size_t count) {
  size_t i;
  if (tracks == NULL || count == 0 || count > CMN_GRID_MAX_TRACKS) {
    return CMN_ERR_INVAL;
  }
  for (i = 0; i < count; ++i) {
    if (tracks[i].unit < CMN_GRID_UNIT_PX ||
        tracks[i].unit > CMN_GRID_UNIT_MINMAX) {
      return CMN_ERR_INVAL;
    }
    if ((tracks[i].unit == CMN_GRID_UNIT_PX ||
         tracks[i].unit == CMN_GRID_UNIT_FR) &&
        tracks[i].value < 0.0f) {
      return CMN_ERR_INVAL;
    }
    if (tracks[i].unit == CMN_GRID_UNIT_MINMAX &&
        (tracks[i].min_px < 0 || tracks[i].max_px < tracks[i].min_px)) {
      return CMN_ERR_INVAL;
    }
  }
  return CMN_SUCCESS;
}

/**
 * @brief Defines the explicit column track template (`grid-template-columns`)
 * for a grid container.
 * @param grid The grid container to modify.
 * @param tracks Array of `count` column track definitions.
 * @param count Number of column tracks (`1 .. 32`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_set_columns(cmn_grid_t *grid,
                                 const cmn_grid_track_t *tracks, size_t count) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  cmn_error_e rc;
  if (grid == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  rc = validate_grid_tracks(tracks, count);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  for (i = 0; i < count; ++i) {
    grid->col_tracks[i] = tracks[i];
  }
  grid->col_count = count;
  grid->effective_col_count = count;
  grid->auto_fit_min_width = 0;
  return CMN_SUCCESS;
}

/**
 * @brief Defines the explicit row track template (`grid-template-rows`) for a
 * grid container.
 * @param grid The grid container to modify.
 * @param tracks Array of `count` row track definitions.
 * @param count Number of row tracks (`1 .. 32`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_set_rows(cmn_grid_t *grid, const cmn_grid_track_t *tracks,
                              size_t count) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  cmn_error_e rc;
  if (grid == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  rc = validate_grid_tracks(tracks, count);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  for (i = 0; i < count; ++i) {
    grid->row_tracks[i] = tracks[i];
  }
  grid->row_count = count;
  return CMN_SUCCESS;
}

/**
 * @brief Configures responsive `repeat(auto-fit, minmax(min_col_width, 1fr))`
 * columns that automatically adjust the number of columns as the container or
 * window is resized.
 * @param grid The grid container to modify.
 * @param min_col_width Minimum column width in pixels (`> 0`).
 * @param max_cols Maximum number of columns (`> 0`, `<= 32`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_set_auto_fit_columns(cmn_grid_t *grid, int min_col_width,
                                          int max_cols) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (grid == NULL || min_col_width <= 0 || max_cols <= 0 ||
      max_cols > CMN_GRID_MAX_TRACKS) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  grid->auto_fit_min_width = min_col_width;
  grid->auto_fit_max_cols = max_cols;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the row and column gaps (in pixels) between tracks in a grid
 * container.
 * @param grid The grid container to modify.
 * @param row_gap Vertical gap between rows (`>= 0`).
 * @param column_gap Horizontal gap between columns (`>= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_set_gap(cmn_grid_t *grid, int row_gap, int column_gap) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (grid == NULL || row_gap < 0 || column_gap < 0) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  grid->row_gap = row_gap;
  grid->column_gap = column_gap;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the row and column gaps of a grid container.
 * @param grid The grid container to query.
 * @param out_row_gap Output pointer receiving the row gap.
 * @param out_column_gap Output pointer receiving the column gap.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_get_gap(const cmn_grid_t *grid, int *out_row_gap,
                             int *out_column_gap) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (grid == NULL || out_row_gap == NULL || out_column_gap == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  *out_row_gap = grid->row_gap;
  *out_column_gap = grid->column_gap;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the inner padding insets of a grid container.
 * @param grid The grid container to modify.
 * @param padding Padding insets (`top`, `right`, `bottom`, `left` >= 0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_set_padding(cmn_grid_t *grid, cmn_edges_t padding) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (grid == NULL || padding.top < 0 || padding.right < 0 ||
      padding.bottom < 0 || padding.left < 0) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  grid->padding = padding;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the inner padding insets of a grid container.
 * @param grid The grid container to query.
 * @param out_padding Output pointer receiving the padding insets.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_get_padding(const cmn_grid_t *grid,
                                 cmn_edges_t *out_padding) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (grid == NULL || out_padding == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  *out_padding = grid->padding;
  return CMN_SUCCESS;
}

/**
 * @brief Validates a `cmn_grid_item_style_t` structure.
 * @param style Pointer to style to validate.
 * @return CMN_SUCCESS if valid, CMN_ERR_INVAL otherwise.
 */
static cmn_error_e
validate_grid_item_style(const cmn_grid_item_style_t *style) {
  if (style->row_start < -1 || style->col_start < -1 ||
      style->justify_self < CMN_ALIGN_AUTO ||
      style->justify_self > CMN_ALIGN_CENTER ||
      style->align_self < CMN_ALIGN_AUTO ||
      style->align_self > CMN_ALIGN_CENTER || style->margin.top < 0 ||
      style->margin.right < 0 || style->margin.bottom < 0 ||
      style->margin.left < 0) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Adds a child widget to a grid container with optional cell placement /
 * span styling and binds its native parent.
 * @param grid The parent grid container.
 * @param child The child widget to add.
 * @param style Optional pointer to grid item style (if NULL, auto-placement is
 * used).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_add_child(cmn_grid_t *grid, void *child,
                               const cmn_grid_item_style_t *style) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_grid_item_style_t resolved_style;
  cmn_error_e rc;

  if (grid == NULL || child == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_grid_item_style_init(&resolved_style);
  if (style != NULL && rc == CMN_SUCCESS) {
    rc = validate_grid_item_style(style);
    resolved_style = *style;
  }
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  if (grid->child_count >= CMN_LAYOUT_MAX_CHILDREN) {
    return CMN_ERR_NOMEM;
  }

  rc = cmn_widget_set_parent(grid, child);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  grid->children[grid->child_count].widget = child;
  grid->children[grid->child_count].style = resolved_style;
  rc = cmn_widget_get_bounds(child,
                             &grid->children[grid->child_count].intrinsic_rect);
  grid->children[grid->child_count].computed_rect =
      grid->children[grid->child_count].intrinsic_rect;
  grid->child_count++;
  return rc;
}

/**
 * @brief Updates the grid placement style of an existing child widget in a grid
 * container.
 * @param grid The grid container.
 * @param child The child widget already added to `grid`.
 * @param style New grid item style to apply.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if `child` is not in
 * `grid`, or CMN_ERR_INVAL on invalid arguments.
 */
cmn_error_e cmn_grid_set_child_style(cmn_grid_t *grid, const void *child,
                                     const cmn_grid_item_style_t *style) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  if (grid == NULL || child == NULL || style == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  if (validate_grid_item_style(style) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  for (i = 0; i < grid->child_count; ++i) {
    if (grid->children[i].widget == child) {
      grid->children[i].style = *style;
      return CMN_SUCCESS;
    }
  }
  return CMN_ERR_NOT_FOUND;
}

/**
 * @brief Retrieves the computed bounding rectangle of a child widget inside a
 * grid container.
 * @param grid The grid container.
 * @param child The child widget to query.
 * @param out_rect Output pointer receiving the computed `cmn_rect_t`.
 * @return CMN_SUCCESS on success, CMN_ERR_NOT_FOUND if `child` is not in
 * `grid`, or CMN_ERR_INVAL on NULL arguments.
 */
cmn_error_e cmn_grid_get_child_rect(const cmn_grid_t *grid, const void *child,
                                    cmn_rect_t *out_rect) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  if (grid == NULL || child == NULL || out_rect == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  for (i = 0; i < grid->child_count; ++i) {
    if (grid->children[i].widget == child) {
      *out_rect = grid->children[i].computed_rect;
      return CMN_SUCCESS;
    }
  }
  return CMN_ERR_NOT_FOUND;
}

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
cmn_error_e cmn_grid_add_breakpoint(cmn_grid_t *grid, int max_width,
                                    size_t col_count) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (grid == NULL || max_width <= 0 || col_count == 0 ||
      col_count > CMN_GRID_MAX_TRACKS) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  if (grid->breakpoint_count >= CMN_LAYOUT_MAX_BREAKPOINTS) {
    return CMN_ERR_NOMEM;
  }
  grid->breakpoints[grid->breakpoint_count].max_width = max_width;
  grid->breakpoints[grid->breakpoint_count].col_count = col_count;
  grid->breakpoint_count++;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the effective number of columns after the most recent layout
 * pass.
 * @param grid The grid container to query.
 * @param out_col_count Output pointer receiving the active column count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_get_effective_columns(const cmn_grid_t *grid,
                                           size_t *out_col_count) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  if (grid == NULL || out_col_count == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }
  *out_col_count = grid->effective_col_count;
  return CMN_SUCCESS;
}

/**
 * @brief Resolves pixel sizes and starting offsets for a 1D array of grid
 * tracks (`px`, `fr`, `auto`, `minmax`).
 * @param tracks Track template definitions.
 * @param count Number of tracks (`>= 1`).
 * @param avail_space Total available space along this axis (after padding).
 * @param gap Gap between adjacent tracks in pixels.
 * @param intrinsic_max Array of `count` maximum intrinsic item sizes.
 * @param start_padding Leading padding offset (`left` or `top`).
 * @param out_sizes Output array of `count` resolved track sizes.
 * @param out_offsets Output array of `count` resolved track offsets.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e resolve_grid_tracks(const cmn_grid_track_t *tracks,
                                       size_t count, int avail_space, int gap,
                                       const int *intrinsic_max,
                                       int start_padding, int *out_sizes,
                                       int *out_offsets) {
  size_t i;
  int used = 0;
  float total_fr = 0.0f;
  int rem;
  int cursor = start_padding;

  if (count > 1) {
    used += (int)(count - 1) * gap;
  }

  for (i = 0; i < count; ++i) {
    switch (tracks[i].unit) {
    case CMN_GRID_UNIT_PX:
      out_sizes[i] = (int)tracks[i].value;
      used += out_sizes[i];
      break;
    case CMN_GRID_UNIT_AUTO:
      out_sizes[i] = intrinsic_max[i];
      used += out_sizes[i];
      break;
    case CMN_GRID_UNIT_MINMAX:
      out_sizes[i] = tracks[i].min_px;
      used += out_sizes[i];
      total_fr += 1.0f;
      break;
    case CMN_GRID_UNIT_FR:
    default:
      out_sizes[i] = 0;
      total_fr += (tracks[i].value > 0.0f) ? tracks[i].value : 1.0f;
      break;
    }
  }

  rem = avail_space - used;
  if (rem > 0 && total_fr > 0.0f) {
    int distributed = 0;
    size_t last_fr_idx = 0;
    for (i = 0; i < count; ++i) {
      if (tracks[i].unit == CMN_GRID_UNIT_FR ||
          tracks[i].unit == CMN_GRID_UNIT_MINMAX) {
        last_fr_idx = i;
      }
    }
    for (i = 0; i < count; ++i) {
      if (tracks[i].unit == CMN_GRID_UNIT_FR ||
          tracks[i].unit == CMN_GRID_UNIT_MINMAX) {
        float weight =
            (tracks[i].unit == CMN_GRID_UNIT_FR && tracks[i].value > 0.0f)
                ? tracks[i].value
                : 1.0f;
        int share = (i == last_fr_idx)
                        ? (rem - distributed)
                        : (int)(((float)rem * weight) / total_fr);
        distributed += share;
        out_sizes[i] += share;
        if (tracks[i].unit == CMN_GRID_UNIT_MINMAX && tracks[i].max_px > 0 &&
            out_sizes[i] > tracks[i].max_px) {
          out_sizes[i] = tracks[i].max_px;
        }
      }
    }
  }

  for (i = 0; i < count; ++i) {
    out_offsets[i] = cursor;
    cursor += out_sizes[i] + gap;
  }

  return CMN_SUCCESS;
}

/**
 * @brief Computes the responsive CSS Grid layout for the given available width
 * and height, updating all child rectangles, nested layout containers, and
 * native widget bounds.
 * @param grid The grid container to layout.
 * @param available_width Available width in pixels (`>= 0`).
 * @param available_height Available height in pixels (`>= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_grid_compute_layout(cmn_grid_t *grid, int available_width,
                                    int available_height) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_grid_track_t active_cols[CMN_GRID_MAX_TRACKS];
  cmn_grid_track_t active_rows[CMN_GRID_MAX_TRACKS];
  size_t num_cols;
  size_t num_rows;
  int inner_w;
  int inner_h;
  int best_bp_width = 0;
  size_t bp_cols = 0;
  size_t i;
  size_t r;
  size_t c;
  int item_row[CMN_LAYOUT_MAX_CHILDREN];
  int item_col[CMN_LAYOUT_MAX_CHILDREN];
  int item_rspan[CMN_LAYOUT_MAX_CHILDREN];
  int item_cspan[CMN_LAYOUT_MAX_CHILDREN];
  int col_intrinsic[CMN_GRID_MAX_TRACKS];
  int row_intrinsic[CMN_GRID_MAX_TRACKS];
  int col_sizes[CMN_GRID_MAX_TRACKS];
  int col_offsets[CMN_GRID_MAX_TRACKS];
  int row_sizes[CMN_GRID_MAX_TRACKS];
  int row_offsets[CMN_GRID_MAX_TRACKS];
  unsigned char occupied[CMN_GRID_MAX_TRACKS][CMN_GRID_MAX_TRACKS];
  cmn_error_e rc = CMN_SUCCESS;

  if (grid == NULL || available_width < 0 || available_height < 0) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(grid, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_GRID) {
    return CMN_ERR_INVAL;
  }

  grid->base.bounds.width = available_width;
  grid->base.bounds.height = available_height;

  inner_w = available_width - grid->padding.left - grid->padding.right;
  inner_h = available_height - grid->padding.top - grid->padding.bottom;
  if (inner_w < 0) {
    inner_w = 0;
  }
  if (inner_h < 0) {
    inner_h = 0;
  }

  /* 1. Determine active column tracks (breakpoints -> auto-fit -> explicit) */
  for (i = 0; i < grid->breakpoint_count; ++i) {
    if (available_width <= grid->breakpoints[i].max_width) {
      if (best_bp_width == 0 ||
          grid->breakpoints[i].max_width < best_bp_width) {
        best_bp_width = grid->breakpoints[i].max_width;
        bp_cols = grid->breakpoints[i].col_count;
      }
    }
  }

  if (bp_cols > 0) {
    num_cols = bp_cols;
    for (i = 0; i < num_cols; ++i) {
      active_cols[i].unit = CMN_GRID_UNIT_FR;
      active_cols[i].value = 1.0f;
      active_cols[i].min_px = 0;
      active_cols[i].max_px = 0;
    }
  } else if (grid->auto_fit_min_width > 0) {
    int denom = grid->auto_fit_min_width + grid->column_gap;
    int fit = (inner_w + grid->column_gap) / denom;
    if (fit < 1) {
      fit = 1;
    }
    if (fit > grid->auto_fit_max_cols) {
      fit = grid->auto_fit_max_cols;
    }
    num_cols = (size_t)fit;
    for (i = 0; i < num_cols; ++i) {
      active_cols[i].unit = CMN_GRID_UNIT_FR;
      active_cols[i].value = 1.0f;
      active_cols[i].min_px = 0;
      active_cols[i].max_px = 0;
    }
  } else {
    num_cols = grid->col_count;
    for (i = 0; i < num_cols; ++i) {
      active_cols[i] = grid->col_tracks[i];
    }
  }
  grid->effective_col_count = num_cols;

  if (grid->child_count == 0) {
    return CMN_SUCCESS;
  }

  for (r = 0; r < CMN_GRID_MAX_TRACKS; ++r) {
    col_intrinsic[r] = 0;
    row_intrinsic[r] = 0;
    for (c = 0; c < CMN_GRID_MAX_TRACKS; ++c) {
      occupied[r][c] = 0;
    }
  }

  /* 2. Place items (explicit and auto-placement) and find required rows */
  num_rows = (grid->row_count > 0) ? grid->row_count : 1;
  {
    size_t auto_r = 0;
    size_t auto_c = 0;

    for (i = 0; i < grid->child_count; ++i) {
      const cmn_grid_item_style_t *st = &grid->children[i].style;
      int rs = (st->row_span > 0) ? st->row_span : 1;
      int cs = (st->col_span > 0) ? st->col_span : 1;
      int placed_r = st->row_start;
      int placed_c = st->col_start;
      size_t rr;
      size_t cc;

      if (cs > (int)num_cols) {
        cs = (int)num_cols;
      }

      if (placed_r < 0 || placed_c < 0) {
        int found = 0;
        while (!found && auto_r < CMN_GRID_MAX_TRACKS) {
          if (auto_c + (size_t)cs > num_cols) {
            auto_r++;
            auto_c = 0;
          } else {
            int collision = 0;
            for (rr = auto_r;
                 rr < auto_r + (size_t)rs && rr < CMN_GRID_MAX_TRACKS; ++rr) {
              for (cc = auto_c; cc < auto_c + (size_t)cs; ++cc) {
                if (occupied[rr][cc]) {
                  collision = 1;
                }
              }
            }
            if (!collision) {
              placed_r = (int)auto_r;
              placed_c = (int)auto_c;
              found = 1;
            } else {
              auto_c++;
            }
          }
        }
        if (!found) {
          placed_r = (int)(CMN_GRID_MAX_TRACKS - 1);
          placed_c = 0;
        }
      } else if (placed_c + cs > (int)num_cols) {
        placed_c = (int)num_cols - cs;
      }

      if (placed_r + rs > CMN_GRID_MAX_TRACKS) {
        rs = CMN_GRID_MAX_TRACKS - placed_r;
      }

      for (rr = (size_t)placed_r; rr < (size_t)(placed_r + rs); ++rr) {
        for (cc = (size_t)placed_c; cc < (size_t)(placed_c + cs); ++cc) {
          occupied[rr][cc] = 1;
        }
      }

      if ((size_t)(placed_r + rs) > num_rows) {
        num_rows = (size_t)(placed_r + rs);
      }

      item_row[i] = placed_r;
      item_col[i] = placed_c;
      item_rspan[i] = rs;
      item_cspan[i] = cs;
    }
  }

  for (r = 0; r < num_rows; ++r) {
    if (r < grid->row_count) {
      active_rows[r] = grid->row_tracks[r];
    } else {
      active_rows[r].unit = CMN_GRID_UNIT_AUTO;
      active_rows[r].value = 0.0f;
      active_rows[r].min_px = 0;
      active_rows[r].max_px = 0;
    }
  }

  /* 3. Compute intrinsic track maxima for AUTO tracks */
  for (i = 0; i < grid->child_count; ++i) {
    const cmn_rect_t *intr = &grid->children[i].intrinsic_rect;
    const cmn_grid_item_style_t *st = &grid->children[i].style;
    int w_need = intr->width + st->margin.left + st->margin.right;
    int h_need = intr->height + st->margin.top + st->margin.bottom;

    if (item_cspan[i] == 1 && w_need > col_intrinsic[item_col[i]]) {
      col_intrinsic[item_col[i]] = w_need;
    }
    if (item_rspan[i] == 1 && h_need > row_intrinsic[item_row[i]]) {
      row_intrinsic[item_row[i]] = h_need;
    }
  }

  /* 4. Resolve column and row track sizes & offsets */
  rc = resolve_grid_tracks(active_cols, num_cols, inner_w, grid->column_gap,
                           col_intrinsic, grid->padding.left, col_sizes,
                           col_offsets);
  {
    cmn_error_e row_rc = resolve_grid_tracks(
        active_rows, num_rows, inner_h, grid->row_gap, row_intrinsic,
        grid->padding.top, row_sizes, row_offsets);
    rc = (rc != CMN_SUCCESS) ? rc : row_rc;
  }

  /* 5. Compute each child's bounding rectangle within its spanned cell(s) */
  for (i = 0; i < grid->child_count; ++i) {
    const cmn_rect_t *intr = &grid->children[i].intrinsic_rect;
    const cmn_grid_item_style_t *st = &grid->children[i].style;
    int r0 = item_row[i];
    int c0 = item_col[i];
    int rs = item_rspan[i];
    int cs = item_cspan[i];
    int cell_x = col_offsets[c0];
    int cell_y = row_offsets[r0];
    int cell_w = 0;
    int cell_h = 0;
    int s_idx;
    int avail_w;
    int avail_h;
    int final_w = intr->width;
    int final_h = intr->height;
    int final_x;
    int final_y;
    cmn_rect_t rect;

    for (s_idx = 0; s_idx < cs; ++s_idx) {
      cell_w += col_sizes[c0 + s_idx];
      if (s_idx > 0) {
        cell_w += grid->column_gap;
      }
    }
    for (s_idx = 0; s_idx < rs; ++s_idx) {
      cell_h += row_sizes[r0 + s_idx];
      if (s_idx > 0) {
        cell_h += grid->row_gap;
      }
    }

    avail_w = cell_w - st->margin.left - st->margin.right;
    avail_h = cell_h - st->margin.top - st->margin.bottom;
    if (avail_w < 0) {
      avail_w = 0;
    }
    if (avail_h < 0) {
      avail_h = 0;
    }

    final_x = cell_x + st->margin.left;
    if (st->justify_self == CMN_ALIGN_STRETCH ||
        st->justify_self == CMN_ALIGN_AUTO) {
      final_w = avail_w;
    } else if (st->justify_self == CMN_ALIGN_FLEX_END) {
      final_x = cell_x + cell_w - st->margin.right - final_w;
    } else if (st->justify_self == CMN_ALIGN_CENTER) {
      final_x = cell_x + st->margin.left + (avail_w - final_w) / 2;
    }

    final_y = cell_y + st->margin.top;
    if (st->align_self == CMN_ALIGN_STRETCH ||
        st->align_self == CMN_ALIGN_AUTO) {
      final_h = avail_h;
    } else if (st->align_self == CMN_ALIGN_FLEX_END) {
      final_y = cell_y + cell_h - st->margin.bottom - final_h;
    } else if (st->align_self == CMN_ALIGN_CENTER) {
      final_y = cell_y + st->margin.top + (avail_h - final_h) / 2;
    }

    rect.x = final_x;
    rect.y = final_y;
    rect.width = (final_w >= 0) ? final_w : 0;
    rect.height = (final_h >= 0) ? final_h : 0;

    grid->children[i].computed_rect = rect;
    {
      cmn_error_e child_rc = apply_child_layout(grid->children[i].widget, rect);
      rc = (rc != CMN_SUCCESS) ? rc : child_rc;
    }
  }

  return rc;
}

/* --- Window Resize & Responsive Layout Integration --- */

/**
 * @brief Attaches a root responsive layout container (`cmn_flex_t *` or
 * `cmn_grid_t *`) to a window and immediately computes its layout for the
 * window's current size. Subsequent window resizes automatically recompute the
 * layout.
 * @param window The target window.
 * @param layout_container The root `cmn_flex_t *` or `cmn_grid_t *` container.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_layout(cmn_window_t *window,
                                  void *layout_container) {
  cmn_widget_kind_e win_kind = CMN_WIDGET_KIND_NONE;
  cmn_widget_kind_e layout_kind = CMN_WIDGET_KIND_NONE;
  cmn_widget_header_t *win_hdr;
  cmn_error_e rc;

  if (window == NULL || layout_container == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(window, &win_kind) != CMN_SUCCESS ||
      win_kind != CMN_WIDGET_KIND_WINDOW) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(layout_container, &layout_kind) != CMN_SUCCESS ||
      (layout_kind != CMN_WIDGET_KIND_FLEX &&
       layout_kind != CMN_WIDGET_KIND_GRID)) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_widget_set_parent(window, layout_container);
  win_hdr = (cmn_widget_header_t *)window;
  win_hdr->layout_root = layout_container;

  if (layout_kind == CMN_WIDGET_KIND_FLEX) {
    cmn_error_e flex_rc =
        cmn_flex_compute_layout((cmn_flex_t *)layout_container,
                                win_hdr->bounds.width, win_hdr->bounds.height);
    rc = (rc != CMN_SUCCESS) ? rc : flex_rc;
  } else {
    cmn_error_e grid_rc =
        cmn_grid_compute_layout((cmn_grid_t *)layout_container,
                                win_hdr->bounds.width, win_hdr->bounds.height);
    rc = (rc != CMN_SUCCESS) ? rc : grid_rc;
  }
  return rc;
}

/**
 * @brief Resizes a window and automatically recomputes any attached responsive
 * layout (`cmn_flex_t` or `cmn_grid_t`) and invokes any registered resize
 * callback.
 * @param window The window to resize.
 * @param width New width in pixels (`> 0`).
 * @param height New height in pixels (`> 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_size(cmn_window_t *window, int width, int height) {
  cmn_widget_kind_e win_kind = CMN_WIDGET_KIND_NONE;
  cmn_widget_header_t *win_hdr;
  cmn_error_e rc = CMN_SUCCESS;

  if (window == NULL || width <= 0 || height <= 0) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(window, &win_kind) != CMN_SUCCESS ||
      win_kind != CMN_WIDGET_KIND_WINDOW) {
    return CMN_ERR_INVAL;
  }

  win_hdr = (cmn_widget_header_t *)window;
  win_hdr->bounds.width = width;
  win_hdr->bounds.height = height;

  if (win_hdr->layout_root != NULL) {
    cmn_widget_kind_e layout_kind = CMN_WIDGET_KIND_NONE;
    rc = cmn_widget_get_kind(win_hdr->layout_root, &layout_kind);
    if (layout_kind == CMN_WIDGET_KIND_FLEX) {
      rc = cmn_flex_compute_layout((cmn_flex_t *)win_hdr->layout_root, width,
                                   height);
    } else if (layout_kind == CMN_WIDGET_KIND_GRID) {
      rc = cmn_grid_compute_layout((cmn_grid_t *)win_hdr->layout_root, width,
                                   height);
    }
  }

  if (win_hdr->on_resize != NULL) {
    cmn_error_e cb_rc =
        win_hdr->on_resize(window, width, height, win_hdr->resize_userdata);
    if (cb_rc != CMN_SUCCESS) {
      rc = cb_rc;
    }
  }

  return rc;
}

/**
 * @brief Gets the current client dimensions of a window.
 * @param window The window to query.
 * @param out_width Output pointer receiving the width in pixels.
 * @param out_height Output pointer receiving the height in pixels.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_get_size(const cmn_window_t *window, int *out_width,
                                int *out_height) {
  cmn_widget_kind_e win_kind = CMN_WIDGET_KIND_NONE;
  const cmn_widget_header_t *win_hdr;
  if (window == NULL || out_width == NULL || out_height == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(window, &win_kind) != CMN_SUCCESS ||
      win_kind != CMN_WIDGET_KIND_WINDOW) {
    return CMN_ERR_INVAL;
  }
  win_hdr = (const cmn_widget_header_t *)window;
  *out_width = win_hdr->bounds.width;
  *out_height = win_hdr->bounds.height;
  return CMN_SUCCESS;
}

/**
 * @brief Registers a callback to be invoked whenever the window is resized.
 * @param window The window to observe.
 * @param on_resize Callback function (or NULL to clear).
 * @param userdata User data pointer passed to `on_resize`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_on_resize(cmn_window_t *window,
                                     cmn_window_resize_fn on_resize,
                                     void *userdata) {
  cmn_widget_kind_e win_kind = CMN_WIDGET_KIND_NONE;
  cmn_widget_header_t *win_hdr;
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(window, &win_kind) != CMN_SUCCESS ||
      win_kind != CMN_WIDGET_KIND_WINDOW) {
    return CMN_ERR_INVAL;
  }
  win_hdr = (cmn_widget_header_t *)window;
  win_hdr->on_resize = on_resize;
  win_hdr->resize_userdata = userdata;
  return CMN_SUCCESS;
}
