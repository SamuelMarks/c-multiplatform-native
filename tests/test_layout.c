/* clang-format off */
#include "greatest.h"
#include "../src/cmn_internal.h"
#include "cmn/app.h"
#include "cmn/layout.h"
#include "cmn/memory.h"
#include "cmn/widget.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file test_layout.c
 * @brief Unit tests for the responsive layout system (Flexbox, Grid,
 * Breakpoints, and Window Resize reflow).
 */

/** @brief External test helper `cmn_mock_alloc_set`. */
extern cmn_error_e cmn_mock_alloc_set(int fail, int count);

/**
 * @brief Tests Flexbox lifecycle, getters/setters, upcasting, and polymorphic
 * destruction.
 */
TEST test_flex_lifecycle_and_properties(void) {
  cmn_flex_t *flex = NULL;
  cmn_button_t *btn = NULL;
  cmn_container_t *cont = NULL;
  cmn_widget_t *wid = NULL;
  cmn_flex_direction_e dir = CMN_FLEX_COLUMN;
  cmn_flex_wrap_e wrap = CMN_FLEX_WRAP;
  cmn_justify_content_e justify = CMN_JUSTIFY_CENTER;
  cmn_align_items_e align = CMN_ALIGN_CENTER;
  int rgap = 0;
  int cgap = 0;
  cmn_edges_t pad;
  cmn_edges_t out_pad;

  /* Allocation failure check */
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_flex_create(&flex));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_create(&flex));
  ASSERT_NEQ(NULL, flex);

  /* Default getters */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_direction(flex, &dir));
  ASSERT_EQ(CMN_FLEX_ROW, dir);
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_wrap(flex, &wrap));
  ASSERT_EQ(CMN_FLEX_NOWRAP, wrap);
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_justify_content(flex, &justify));
  ASSERT_EQ(CMN_JUSTIFY_FLEX_START, justify);
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_align_items(flex, &align));
  ASSERT_EQ(CMN_ALIGN_STRETCH, align);
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_gap(flex, &rgap, &cgap));
  ASSERT_EQ(0, rgap);
  ASSERT_EQ(0, cgap);
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_padding(flex, &out_pad));
  ASSERT_EQ(0, out_pad.top);

  /* Setters and getters */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_direction(flex, CMN_FLEX_COLUMN_REVERSE));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_direction(flex, &dir));
  ASSERT_EQ(CMN_FLEX_COLUMN_REVERSE, dir);

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_wrap(flex, CMN_FLEX_WRAP_REVERSE));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_wrap(flex, &wrap));
  ASSERT_EQ(CMN_FLEX_WRAP_REVERSE, wrap);

  ASSERT_EQ(CMN_SUCCESS,
            cmn_flex_set_justify_content(flex, CMN_JUSTIFY_SPACE_EVENLY));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_justify_content(flex, &justify));
  ASSERT_EQ(CMN_JUSTIFY_SPACE_EVENLY, justify);

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_align_items(flex, CMN_ALIGN_FLEX_END));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_align_items(flex, &align));
  ASSERT_EQ(CMN_ALIGN_FLEX_END, align);

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_gap(flex, 8, 12));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_gap(flex, &rgap, &cgap));
  ASSERT_EQ(8, rgap);
  ASSERT_EQ(12, cgap);

  pad.top = 10;
  pad.right = 15;
  pad.bottom = 20;
  pad.left = 25;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_padding(flex, pad));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_padding(flex, &out_pad));
  ASSERT_EQ(10, out_pad.top);
  ASSERT_EQ(15, out_pad.right);
  ASSERT_EQ(20, out_pad.bottom);
  ASSERT_EQ(25, out_pad.left);

  /* Invalid enum / negative value checks */
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_direction(flex, (cmn_flex_direction_e)-1));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_direction(flex, (cmn_flex_direction_e)99));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_wrap(flex, (cmn_flex_wrap_e)-1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_wrap(flex, (cmn_flex_wrap_e)99));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_justify_content(flex, (cmn_justify_content_e)-1));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_justify_content(flex, (cmn_justify_content_e)99));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_align_items(flex, CMN_ALIGN_AUTO));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_align_items(flex, (cmn_align_items_e)99));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_gap(flex, -1, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_gap(flex, 0, -1));
  pad.top = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_padding(flex, pad));
  pad.top = 0;
  pad.right = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_padding(flex, pad));
  pad.right = 0;
  pad.bottom = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_padding(flex, pad));
  pad.bottom = 0;
  pad.left = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_padding(flex, pad));

  /* Wrong kind checks */
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btn));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_direction((cmn_flex_t *)btn, CMN_FLEX_ROW));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_get_direction((const cmn_flex_t *)btn, &dir));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_wrap((cmn_flex_t *)btn, CMN_FLEX_NOWRAP));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_wrap((const cmn_flex_t *)btn, &wrap));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_justify_content(
                               (cmn_flex_t *)btn, CMN_JUSTIFY_FLEX_START));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_get_justify_content((const cmn_flex_t *)btn, &justify));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_align_items((cmn_flex_t *)btn, CMN_ALIGN_STRETCH));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_get_align_items((const cmn_flex_t *)btn, &align));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_gap((cmn_flex_t *)btn, 0, 0));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_get_gap((const cmn_flex_t *)btn, &rgap, &cgap));
  pad.left = 0;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_padding((cmn_flex_t *)btn, pad));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_get_padding((const cmn_flex_t *)btn, &out_pad));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_destroy((cmn_flex_t **)&btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btn));

  /* Upcasts & polymorphic destroy */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_as_container(flex, &cont));
  ASSERT_EQ((void *)flex, (void *)cont);
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_as_widget(flex, &wid));
  ASSERT_EQ((void *)flex, (void *)wid);
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&wid));
  ASSERT_EQ(NULL, wid);

  PASS();
}

/**
 * @brief Tests Flexbox row/column layout, grow/shrink, justify-content,
 * align-items, align-self, margins, and line wrapping.
 */
TEST test_flex_layout_engine(void) {
  cmn_flex_t *flex = NULL;
  cmn_button_t *b1 = NULL;
  cmn_button_t *b2 = NULL;
  cmn_button_t *b3 = NULL;
  cmn_flex_item_style_t s1;
  cmn_flex_item_style_t s2;
  cmn_flex_item_style_t s3;
  cmn_rect_t r1;
  cmn_rect_t r2;
  cmn_rect_t r3;
  cmn_edges_t pad;

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_create(&flex));
  /* Empty layout compute */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 400, 200));

  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&b1));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&b2));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&b3));

  /* Add children with NULL style and explicit styles */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_add_child(flex, b1, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&s2));
  s2.flex_basis = 100;
  s2.flex_grow = 1.0f;
  s2.align_self = CMN_ALIGN_CENTER;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_add_child(flex, b2, &s2));

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&s3));
  s3.flex_basis = 80;
  s3.flex_grow = 1.0f;
  s3.align_self = CMN_ALIGN_FLEX_END;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_add_child(flex, b3, &s3));

  /* Update b1 style via cmn_flex_set_child_style */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&s1));
  s1.flex_basis = 100;
  s1.flex_grow = 0.0f;
  s1.align_self = CMN_ALIGN_FLEX_START;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b1, &s1));

  pad.top = 10;
  pad.right = 10;
  pad.bottom = 10;
  pad.left = 10;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_padding(flex, pad));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_gap(flex, 0, 10));

  /* Available width = 500 -> inner = 480, gaps = 20, basis = 280 -> free = 180
   * b2 and b3 each grow by 90 -> b1=100, b2=190, b3=170 */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 500, 120));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b2, &r2));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b3, &r3));

  ASSERT_EQ(10, r1.x);
  ASSERT_EQ(10, r1.y);
  ASSERT_EQ(100, r1.width);
  ASSERT_EQ(32, r1.height);

  ASSERT_EQ(120, r2.x);
  ASSERT_EQ(10 + (100 - 32) / 2, r2.y);
  ASSERT_EQ(190, r2.width);

  ASSERT_EQ(320, r3.x);
  ASSERT_EQ(110 - 32, r3.y);
  ASSERT_EQ(170, r3.width);

  /* Test flex-shrink when space is smaller than basis */
  s1.flex_shrink = 1.0f;
  s2.flex_grow = 0.0f;
  s2.flex_shrink = 1.0f;
  s2.align_self = CMN_ALIGN_AUTO;
  s3.flex_grow = 0.0f;
  s3.flex_shrink = 0.0f;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b1, &s1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b2, &s2));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b3, &s3));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 240, 100));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b2, &r2));
  ASSERT_EQ(60, r1.width);
  ASSERT_EQ(60, r2.width);
  ASSERT_EQ(80, r2.height); /* stretched to cross_avail */

  /* Test all justify-content modes with fixed items (grow=0, shrink=0) */
  s1.flex_shrink = 0.0f;
  s1.flex_basis = 50;
  s2.flex_shrink = 0.0f;
  s2.flex_basis = 50;
  s3.flex_shrink = 0.0f;
  s3.flex_basis = 50;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b1, &s1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b2, &s2));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b3, &s3));

  ASSERT_EQ(CMN_SUCCESS,
            cmn_flex_set_justify_content(flex, CMN_JUSTIFY_FLEX_END));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 400, 100));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT_EQ(220, r1.x);

  ASSERT_EQ(CMN_SUCCESS,
            cmn_flex_set_justify_content(flex, CMN_JUSTIFY_CENTER));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 400, 100));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT_EQ(115, r1.x);

  ASSERT_EQ(CMN_SUCCESS,
            cmn_flex_set_justify_content(flex, CMN_JUSTIFY_SPACE_BETWEEN));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 400, 100));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b3, &r3));
  ASSERT_EQ(340, r3.x);

  ASSERT_EQ(CMN_SUCCESS,
            cmn_flex_set_justify_content(flex, CMN_JUSTIFY_SPACE_AROUND));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 400, 100));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT_EQ(45, r1.x);

  ASSERT_EQ(CMN_SUCCESS,
            cmn_flex_set_justify_content(flex, CMN_JUSTIFY_SPACE_EVENLY));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 400, 100));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT_EQ(62, r1.x);

  /* Test ROW_REVERSE, COLUMN, COLUMN_REVERSE, WRAP, WRAP_REVERSE */
  ASSERT_EQ(CMN_SUCCESS,
            cmn_flex_set_justify_content(flex, CMN_JUSTIFY_FLEX_START));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_direction(flex, CMN_FLEX_ROW_REVERSE));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 400, 100));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b3, &r3));
  ASSERT_EQ(10, r3.x);

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_direction(flex, CMN_FLEX_COLUMN));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 200, 300));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b2, &r2));
  ASSERT_EQ(10, r1.y);
  ASSERT_EQ(60, r2.y);

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_direction(flex, CMN_FLEX_COLUMN_REVERSE));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 200, 300));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b3, &r3));
  ASSERT_EQ(10, r3.y);

  /* Multi-line wrapping */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_direction(flex, CMN_FLEX_ROW));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_wrap(flex, CMN_FLEX_WRAP));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_gap(flex, 8, 10));
  s1.flex_basis = 100;
  s2.flex_basis = 100;
  s3.flex_basis = 100;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b1, &s1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b2, &s2));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b3, &s3));
  /* Width 250 -> inner 230 -> b1 (100) + gap (10) + b2 (100) = 210 fits on line
   * 0; b3 wraps to line 1 */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 250, 200));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b3, &r3));
  ASSERT_EQ(10, r1.y);
  ASSERT_EQ(10 + 32 + 8, r3.y);

  /* Wrap reverse */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_wrap(flex, CMN_FLEX_WRAP_REVERSE));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 250, 200));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b3, &r3));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_child_rect(flex, b1, &r1));
  ASSERT(r3.y < r1.y);

  /* Zero available dimensions clamping */
  s1.min_width = 150;
  s1.max_width = 500;
  s1.flex_shrink = 2.0f;
  s1.margin.top = 100;
  s1.margin.bottom = 100;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_wrap(flex, CMN_FLEX_NOWRAP));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b1, &s1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 5, 5));

  /* Also test negative deficit shrinking below 0 when min_width = 0, and
   * max_width clamping */
  s1.min_width = 0;
  s1.max_width = 50;
  s1.flex_basis = 10;
  s1.flex_shrink = 10.0f;
  s2.flex_basis = 300;
  s2.max_width = 250;
  s2.flex_shrink = 1.0f;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b1, &s1));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_child_style(flex, b2, &s2));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_compute_layout(flex, 50, 50));

  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&b1));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&b2));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&b3));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_destroy(&flex));
  PASS();
}

/**
 * @brief Tests Flexbox validation and capacity limits (children and
 * breakpoints).
 */
TEST test_flex_validation_and_limits(void) {
  cmn_flex_t *flex = NULL;
  cmn_button_t *btns[33];
  cmn_menu_t *menu = NULL;
  cmn_flex_item_style_t st;
  cmn_rect_t rect;
  int i;

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_create(&flex));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&st));

  /* Invalid flex item styles */
  st.flex_grow = -1.0f;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&st));
  st.flex_shrink = -1.0f;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&st));
  st.min_width = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&st));
  st.min_height = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&st));
  st.align_self = (cmn_align_items_e)-1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&st));
  st.align_self = (cmn_align_items_e)99;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&st));
  st.margin.top = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  st.margin.top = 0;
  st.margin.right = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  st.margin.right = 0;
  st.margin.bottom = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  st.margin.bottom = 0;
  st.margin.left = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, flex, &st));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&st));

  /* Adding non-widget (menu) fails cmn_widget_set_parent */
  if (cmn_menu_create(&menu) == CMN_SUCCESS) {
    ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_child(flex, menu, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_menu_destroy(&menu));
  }

  /* Fill up to CMN_LAYOUT_MAX_CHILDREN (32) */
  for (i = 0; i < 33; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btns[i]));
    if (i < 32) {
      ASSERT_EQ(CMN_SUCCESS, cmn_flex_add_child(flex, btns[i], NULL));
    } else {
      ASSERT_EQ(CMN_ERR_NOMEM, cmn_flex_add_child(flex, btns[i], NULL));
    }
  }

  /* Child not found in get_child_rect / set_child_style */
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_flex_get_child_rect(flex, btns[32], &rect));
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_flex_set_child_style(flex, btns[32], &st));
  st.flex_grow = -1.0f;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_set_child_style(flex, btns[0], &st));

  /* Wrong kind for child APIs */
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_add_child((cmn_flex_t *)btns[0], btns[1], NULL));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_set_child_style((cmn_flex_t *)btns[0], btns[1], &st));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_get_child_rect((const cmn_flex_t *)btns[0],
                                                   btns[1], &rect));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_compute_layout((cmn_flex_t *)btns[0], 100, 100));

  /* Breakpoint validation & capacity (8) */
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_add_breakpoint((cmn_flex_t *)btns[0], 500, CMN_FLEX_COLUMN,
                                    CMN_FLEX_NOWRAP));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_add_breakpoint(flex, 0, CMN_FLEX_COLUMN, CMN_FLEX_NOWRAP));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_add_breakpoint(flex, 500, (cmn_flex_direction_e)-1,
                                    CMN_FLEX_NOWRAP));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_flex_add_breakpoint(flex, 500, (cmn_flex_direction_e)99,
                                    CMN_FLEX_NOWRAP));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_breakpoint(flex, 500, CMN_FLEX_COLUMN,
                                                   (cmn_flex_wrap_e)-1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_flex_add_breakpoint(flex, 500, CMN_FLEX_COLUMN,
                                                   (cmn_flex_wrap_e)99));
  for (i = 0; i < 8; ++i) {
    ASSERT_EQ(CMN_SUCCESS,
              cmn_flex_add_breakpoint(flex, 800 - i * 50, CMN_FLEX_COLUMN,
                                      CMN_FLEX_WRAP));
  }
  ASSERT_EQ(CMN_ERR_NOMEM,
            cmn_flex_add_breakpoint(flex, 200, CMN_FLEX_COLUMN, CMN_FLEX_WRAP));

  for (i = 0; i < 33; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btns[i]));
  }
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_destroy(&flex));
  PASS();
}

/**
 * @brief Tests CSS Grid lifecycle, track units (`px`, `fr`, `auto`, `minmax`),
 * `auto-fit`, cell spans, and item alignment.
 */
TEST test_grid_layout_engine(void) {
  cmn_grid_t *grid = NULL;
  cmn_button_t *b1 = NULL;
  cmn_button_t *b2 = NULL;
  cmn_button_t *b3 = NULL;
  cmn_button_t *b4 = NULL;
  cmn_grid_track_t cols[3];
  cmn_grid_track_t rows[2];
  cmn_grid_item_style_t gs;
  cmn_rect_t r1;
  cmn_rect_t r2;
  cmn_rect_t r3;
  cmn_rect_t r4;
  cmn_edges_t pad;
  cmn_edges_t out_pad;
  int rgap = 0;
  int cgap = 0;
  size_t eff_cols = 0;
  cmn_container_t *cont = NULL;
  cmn_widget_t *wid = NULL;

  /* Allocation failure check */
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(1, 0));
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_grid_create(&grid));
  ASSERT_EQ(CMN_SUCCESS, cmn_mock_alloc_set(0, 0));

  ASSERT_EQ(CMN_SUCCESS, cmn_grid_create(&grid));
  ASSERT_NEQ(NULL, grid);

  /* Empty grid layout */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_compute_layout(grid, 600, 400));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_effective_columns(grid, &eff_cols));
  ASSERT_EQ(1u, eff_cols);

  /* Configure columns: 100px, 1fr, minmax(80, 120) */
  cols[0].unit = CMN_GRID_UNIT_PX;
  cols[0].value = 100.0f;
  cols[0].min_px = 0;
  cols[0].max_px = 0;

  cols[1].unit = CMN_GRID_UNIT_FR;
  cols[1].value = 2.0f;
  cols[1].min_px = 0;
  cols[1].max_px = 0;

  cols[2].unit = CMN_GRID_UNIT_MINMAX;
  cols[2].value = 0.0f;
  cols[2].min_px = 80;
  cols[2].max_px = 120;

  ASSERT_EQ(CMN_SUCCESS, cmn_grid_set_columns(grid, cols, 3));

  /* Configure rows: 50px, auto */
  rows[0].unit = CMN_GRID_UNIT_PX;
  rows[0].value = 50.0f;
  rows[0].min_px = 0;
  rows[0].max_px = 0;

  rows[1].unit = CMN_GRID_UNIT_AUTO;
  rows[1].value = 0.0f;
  rows[1].min_px = 0;
  rows[1].max_px = 0;

  ASSERT_EQ(CMN_SUCCESS, cmn_grid_set_rows(grid, rows, 2));

  ASSERT_EQ(CMN_SUCCESS, cmn_grid_set_gap(grid, 10, 10));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_gap(grid, &rgap, &cgap));
  ASSERT_EQ(10, rgap);
  ASSERT_EQ(10, cgap);

  pad.top = 10;
  pad.right = 10;
  pad.bottom = 10;
  pad.left = 10;
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_set_padding(grid, pad));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_padding(grid, &out_pad));
  ASSERT_EQ(10, out_pad.left);

  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&b1));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&b2));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&b3));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&b4));

  /* Add children with auto placement and explicit span placement */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_child(grid, b1, NULL));

  ASSERT_EQ(CMN_SUCCESS, cmn_grid_item_style_init(&gs));
  gs.justify_self = CMN_ALIGN_CENTER;
  gs.align_self = CMN_ALIGN_CENTER;
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_child(grid, b2, &gs));

  ASSERT_EQ(CMN_SUCCESS, cmn_grid_item_style_init(&gs));
  gs.justify_self = CMN_ALIGN_FLEX_END;
  gs.align_self = CMN_ALIGN_FLEX_END;
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_child(grid, b3, &gs));

  /* b4 spans 2 columns on row 1 */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_item_style_init(&gs));
  gs.row_start = 1;
  gs.col_start = 0;
  gs.row_span = 1;
  gs.col_span = 2;
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_child(grid, b4, &gs));

  ASSERT_EQ(CMN_SUCCESS, cmn_grid_compute_layout(grid, 600, 300));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_child_rect(grid, b1, &r1));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_child_rect(grid, b2, &r2));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_child_rect(grid, b3, &r3));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_child_rect(grid, b4, &r4));

  ASSERT_EQ(10, r1.x);
  ASSERT_EQ(10, r1.y);
  ASSERT_EQ(100, r1.width);
  ASSERT_EQ(50, r1.height);

  /* b4 starts at row 1 (y = 10 + 50 + 10 = 70) and spans col 0 + gap + col 1 */
  ASSERT_EQ(10, r4.x);
  ASSERT_EQ(70, r4.y);
  ASSERT(r4.width > 200);

  /* Test responsive auto-fit columns */
  gs.row_start = -1;
  gs.col_start = -1;
  gs.col_span = 1;
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_set_child_style(grid, b4, &gs));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_set_auto_fit_columns(grid, 180, 4));

  /* At width 1200 -> exceeds max_cols (4), clamps to 4 columns */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_compute_layout(grid, 1200, 300));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_effective_columns(grid, &eff_cols));
  ASSERT_EQ(4u, eff_cols);

  /* At width 420 -> 2 columns fit */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_compute_layout(grid, 420, 300));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_effective_columns(grid, &eff_cols));
  ASSERT_EQ(2u, eff_cols);

  /* At width 150 -> clamps to 1 column */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_compute_layout(grid, 150, 300));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_get_effective_columns(grid, &eff_cols));
  ASSERT_EQ(1u, eff_cols);

  /* At width 5, height 5 (smaller than padding 10+10=20) -> inner_w/inner_h
   * clamp to 0 */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_compute_layout(grid, 5, 5));

  /* Upcasts & polymorphic destroy */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_as_container(grid, &cont));
  ASSERT_EQ((void *)grid, (void *)cont);
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_as_widget(grid, &wid));
  ASSERT_EQ((void *)grid, (void *)wid);

  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&b1));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&b2));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&b3));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&b4));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_destroy(&wid));
  ASSERT_EQ(NULL, wid);

  PASS();
}

/**
 * @brief Tests Grid validation, edge cases, and capacity limits.
 */
TEST test_grid_validation_and_limits(void) {
  cmn_grid_t *grid = NULL;
  cmn_button_t *btns[33];
  cmn_menu_t *menu = NULL;
  cmn_grid_track_t bad_track;
  cmn_grid_item_style_t gs;
  cmn_edges_t pad;
  cmn_rect_t rect;
  size_t eff_cols = 0;
  int rg = 0;
  int cg = 0;
  int i;

  ASSERT_EQ(CMN_SUCCESS, cmn_grid_create(&grid));

  /* Track validation */
  bad_track.unit = CMN_GRID_UNIT_PX;
  bad_track.value = 10.0f;
  bad_track.min_px = 0;
  bad_track.max_px = 0;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(grid, NULL, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(grid, &bad_track, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(grid, &bad_track, 33));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_rows(grid, NULL, 1));

  bad_track.unit = (cmn_grid_track_unit_e)-1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(grid, &bad_track, 1));
  bad_track.unit = (cmn_grid_track_unit_e)99;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(grid, &bad_track, 1));

  bad_track.unit = CMN_GRID_UNIT_PX;
  bad_track.value = -5.0f;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(grid, &bad_track, 1));

  bad_track.unit = CMN_GRID_UNIT_MINMAX;
  bad_track.value = 0.0f;
  bad_track.min_px = -1;
  bad_track.max_px = 10;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(grid, &bad_track, 1));
  bad_track.min_px = 50;
  bad_track.max_px = 20;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_columns(grid, &bad_track, 1));

  /* Auto-fit validation */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_auto_fit_columns(grid, 0, 4));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_auto_fit_columns(grid, 100, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_auto_fit_columns(grid, 100, 33));

  /* Gap and padding validation */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_gap(grid, -1, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_gap(grid, 0, -1));
  pad.top = -1;
  pad.right = 0;
  pad.bottom = 0;
  pad.left = 0;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_padding(grid, pad));
  pad.top = 0;
  pad.right = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_padding(grid, pad));
  pad.right = 0;
  pad.bottom = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_padding(grid, pad));
  pad.bottom = 0;
  pad.left = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_padding(grid, pad));

  /* Grid item style validation */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_item_style_init(&gs));
  gs.row_start = -2;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_item_style_init(&gs));
  gs.col_start = -2;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_item_style_init(&gs));
  gs.justify_self = (cmn_align_items_e)-1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  gs.justify_self = (cmn_align_items_e)99;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  gs.justify_self = CMN_ALIGN_STRETCH;
  gs.align_self = (cmn_align_items_e)-1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  gs.align_self = (cmn_align_items_e)99;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  gs.align_self = CMN_ALIGN_STRETCH;
  gs.margin.top = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  gs.margin.top = 0;
  gs.margin.right = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  gs.margin.right = 0;
  gs.margin.bottom = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));
  gs.margin.bottom = 0;
  gs.margin.left = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, grid, &gs));

  if (cmn_menu_create(&menu) == CMN_SUCCESS) {
    ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_child(grid, menu, NULL));
    ASSERT_EQ(CMN_SUCCESS, cmn_menu_destroy(&menu));
  }

  /* Fill 32 children */
  for (i = 0; i < 33; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&btns[i]));
    if (i < 32) {
      ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_child(grid, btns[i], NULL));
    } else {
      ASSERT_EQ(CMN_ERR_NOMEM, cmn_grid_add_child(grid, btns[i], NULL));
    }
  }

  /* Test explicit collision and out-of-bounds col_start / row_span clamping */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_item_style_init(&gs));
  gs.row_start = 0;
  gs.col_start = 5;
  gs.row_span = 35;
  gs.col_span = 5;
  gs.margin.left = 200;
  gs.margin.top = 200;
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_set_child_style(grid, btns[0], &gs));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_compute_layout(grid, 10, 10));

  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_grid_get_child_rect(grid, btns[32], &rect));
  ASSERT_EQ(CMN_ERR_NOT_FOUND, cmn_grid_set_child_style(grid, btns[32], &gs));
  gs.row_start = -2;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_child_style(grid, btns[0], &gs));

  /* Wrong kind checks on grid APIs */
  bad_track.unit = CMN_GRID_UNIT_FR;
  bad_track.value = 1.0f;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_destroy((cmn_grid_t **)&btns[0]));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_set_columns((cmn_grid_t *)btns[0], &bad_track, 1));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_set_rows((cmn_grid_t *)btns[0], &bad_track, 1));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_set_auto_fit_columns((cmn_grid_t *)btns[0], 100, 2));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_gap((cmn_grid_t *)btns[0], 0, 0));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_get_gap((const cmn_grid_t *)btns[0], &rg, &cg));
  pad.left = 0;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_set_padding((cmn_grid_t *)btns[0], pad));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_get_padding((const cmn_grid_t *)btns[0], &pad));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_add_child((cmn_grid_t *)btns[0], btns[1], NULL));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_set_child_style((cmn_grid_t *)btns[0], btns[1], &gs));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_child_rect((const cmn_grid_t *)btns[0],
                                                   btns[1], &rect));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_add_breakpoint((cmn_grid_t *)btns[0], 400, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_get_effective_columns(
                               (const cmn_grid_t *)btns[0], &eff_cols));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_grid_compute_layout((cmn_grid_t *)btns[0], 100, 100));

  /* Grid breakpoint validation & capacity (8) */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_breakpoint(grid, 0, 1));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_breakpoint(grid, 500, 0));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_grid_add_breakpoint(grid, 500, 33));
  for (i = 0; i < 8; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_breakpoint(grid, 800 - i * 50, 2));
  }
  ASSERT_EQ(CMN_ERR_NOMEM, cmn_grid_add_breakpoint(grid, 100, 1));

  for (i = 0; i < 33; ++i) {
    ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&btns[i]));
  }
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_destroy(&grid));
  PASS();
}

/** @brief Resize callback tracking struct for `test_responsive_window_resize`.
 */
struct resize_spy_s {
  int call_count;
  int last_w;
  int last_h;
  cmn_error_e return_rc;
};

/**
 * @brief Window resize callback spy used in unit tests.
 * @param window The resized window.
 * @param width New width.
 * @param height New height.
 * @param userdata Pointer to `struct resize_spy_s`.
 * @return Configured `cmn_error_e`.
 */
static cmn_error_e on_window_resized_spy(cmn_window_t *window, int width,
                                         int height, void *userdata) {
  struct resize_spy_s *spy = (struct resize_spy_s *)userdata;
  (void)window;
  spy->call_count++;
  spy->last_w = width;
  spy->last_h = height;
  return spy->return_rc;
}

/**
 * @brief Tests responsive window resizing with nested Flex and Grid containers
 * and breakpoint switching (side-by-side on desktop -> stacked vertically on
 * mobile width).
 */
TEST test_responsive_window_resize(void) {
  cmn_app_t *app = NULL;
  cmn_window_t *win = NULL;
  cmn_flex_t *root_flex = NULL;
  cmn_grid_t *nested_grid = NULL;
  cmn_flex_t *nested_flex = NULL;
  cmn_button_t *sidebar_btn = NULL;
  cmn_button_t *card1 = NULL;
  cmn_button_t *card2 = NULL;
  cmn_flex_item_style_t fs;
  cmn_rect_t r_sidebar;
  cmn_rect_t r_grid;
  cmn_rect_t r_card1;
  cmn_rect_t r_card2;
  cmn_flex_direction_e active_dir;
  size_t eff_cols = 0;
  int w = 0;
  int h = 0;
  struct resize_spy_s spy;

  spy.call_count = 0;
  spy.last_w = 0;
  spy.last_h = 0;
  spy.return_rc = CMN_SUCCESS;

  ASSERT_EQ(CMN_SUCCESS, cmn_app_create(&app));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_create(app, &win));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_create(&root_flex));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_create(&nested_grid));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_create(&nested_flex));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&sidebar_btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&card1));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_create(&card2));

  /* Responsive root_flex: ROW by default, switches to COLUMN when width <= 600
   * and COLUMN_REVERSE when width <= 360 */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_set_direction(root_flex, CMN_FLEX_ROW));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_add_breakpoint(
                             root_flex, 600, CMN_FLEX_COLUMN, CMN_FLEX_NOWRAP));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_flex_add_breakpoint(root_flex, 360, CMN_FLEX_COLUMN_REVERSE,
                                    CMN_FLEX_NOWRAP));

  /* Sidebar button has basis 200 */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&fs));
  fs.flex_basis = 200;
  fs.flex_grow = 0.0f;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_add_child(root_flex, sidebar_btn, &fs));

  /* Nested grid grows to fill remaining space (flex_grow = 1) */
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_item_style_init(&fs));
  fs.flex_basis = 200;
  fs.flex_grow = 1.0f;
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_add_child(root_flex, nested_grid, &fs));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_add_child(root_flex, nested_flex, &fs));

  /* Nested grid has 2 columns by default, 1 column when width <= 300 */
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_set_auto_fit_columns(nested_grid, 140, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_breakpoint(nested_grid, 300, 1));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_breakpoint(nested_grid, 500, 2));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_child(nested_grid, card1, NULL));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_add_child(nested_grid, card2, NULL));

  /* Attach root_flex to window and register resize callback */
  ASSERT_EQ(CMN_SUCCESS,
            cmn_window_set_on_resize(win, on_window_resized_spy, &spy));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_layout(win, root_flex));

  /* 1. Desktop size: 1000 x 600 -> root_flex is ROW (sidebar left of grid) */
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_size(win, 1000, 600));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_get_size(win, &w, &h));
  ASSERT_EQ(1000, w);
  ASSERT_EQ(600, h);
  ASSERT_EQ(1, spy.call_count);
  ASSERT_EQ(1000, spy.last_w);

  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_direction(root_flex, &active_dir));
  ASSERT_EQ(CMN_FLEX_ROW, active_dir);
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_bounds(sidebar_btn, &r_sidebar));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_bounds(nested_grid, &r_grid));
  /* Sidebar is at x=0, nested_grid is placed to the right of sidebar (x=200) */
  ASSERT_EQ(0, r_sidebar.x);
  ASSERT_EQ(200, r_grid.x);
  ASSERT_EQ(400, r_grid.width);

  /* Inside nested_grid (width 400 <= 500 breakpoint -> 2 columns), card1 and
   * card2 are side by side */
  ASSERT_EQ(CMN_SUCCESS,
            cmn_grid_get_effective_columns(nested_grid, &eff_cols));
  ASSERT_EQ(2u, eff_cols);
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_bounds(card1, &r_card1));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_bounds(card2, &r_card2));
  ASSERT_EQ(0, r_card1.x);
  ASSERT_EQ(200, r_card2.x);
  ASSERT_EQ(r_card1.y, r_card2.y);

  /* 2. Resize window to tablet/mobile: 500 x 600 -> root_flex switches to
   * COLUMN (sidebar above nested_grid) */
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_size(win, 500, 600));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_direction(root_flex, &active_dir));
  ASSERT_EQ(CMN_FLEX_COLUMN, active_dir);
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_bounds(sidebar_btn, &r_sidebar));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_bounds(nested_grid, &r_grid));
  ASSERT_EQ(0, r_sidebar.x);
  ASSERT_EQ(0, r_sidebar.y);
  ASSERT_EQ(0, r_grid.x);
  ASSERT_EQ(200, r_grid.y);

  /* 2b. Resize window to 300 x 600 -> matches both 600 and 360 breakpoints,
   * picks narrower 360 (COLUMN_REVERSE) */
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_size(win, 300, 600));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_get_direction(root_flex, &active_dir));
  ASSERT_EQ(CMN_FLEX_COLUMN_REVERSE, active_dir);
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_get_bounds(NULL, &r_sidebar));

  /* 3. Attach grid directly as window layout root and resize to narrow width */
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_layout(win, nested_grid));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_set_size(win, 280, 400));
  ASSERT_EQ(CMN_SUCCESS,
            cmn_grid_get_effective_columns(nested_grid, &eff_cols));
  ASSERT_EQ(1u, eff_cols);
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_bounds(card1, &r_card1));
  ASSERT_EQ(CMN_SUCCESS, cmn_widget_get_bounds(card2, &r_card2));
  /* In 1-column mode, card2 is placed below card1 */
  ASSERT_EQ(0, r_card1.x);
  ASSERT_EQ(0, r_card2.x);
  ASSERT(r_card2.y > r_card1.y);

  /* Test callback error propagation */
  spy.return_rc = CMN_ERR_IO;
  ASSERT_EQ(CMN_ERR_IO, cmn_window_set_size(win, 320, 480));

  /* Validation on window & bounds APIs */
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_layout(win, sidebar_btn));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_window_set_layout((cmn_window_t *)sidebar_btn, root_flex));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_size(win, 0, 100));
  ASSERT_EQ(CMN_ERR_INVAL, cmn_window_set_size(win, 100, 0));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_window_set_size((cmn_window_t *)sidebar_btn, 100, 100));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_window_get_size((const cmn_window_t *)sidebar_btn, &w, &h));
  ASSERT_EQ(CMN_ERR_INVAL,
            cmn_window_set_on_resize((cmn_window_t *)sidebar_btn, NULL, NULL));

  r_card1.width = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_bounds(card1, r_card1));
  r_card1.width = 10;
  r_card1.height = -1;
  ASSERT_EQ(CMN_ERR_INVAL, cmn_widget_set_bounds(card1, r_card1));

  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&sidebar_btn));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&card1));
  ASSERT_EQ(CMN_SUCCESS, cmn_button_destroy(&card2));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_destroy(&nested_flex));
  ASSERT_EQ(CMN_SUCCESS, cmn_grid_destroy(&nested_grid));
  ASSERT_EQ(CMN_SUCCESS, cmn_flex_destroy(&root_flex));
  ASSERT_EQ(CMN_SUCCESS, cmn_window_destroy(&win));
  ASSERT_EQ(CMN_SUCCESS, cmn_app_destroy(&app));
  PASS();
}

/**
 * @brief Test suite for responsive Flexbox and Grid layout.
 */
SUITE(layout_suite) {
  RUN_TEST(test_flex_lifecycle_and_properties);
  RUN_TEST(test_flex_layout_engine);
  RUN_TEST(test_flex_validation_and_limits);
  RUN_TEST(test_grid_layout_engine);
  RUN_TEST(test_grid_validation_and_limits);
  RUN_TEST(test_responsive_window_resize);
}
