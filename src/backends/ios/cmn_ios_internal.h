#ifndef CMN_IOS_INTERNAL_H
#define CMN_IOS_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn_internal.h"
#include "cmn/app.h"
#include "cmn/widget.h"
#import <UIKit/UIKit.h>
/* clang-format on */

/**
 * @file cmn_ios_internal.h
 * @brief Internal iOS UIKit widget and window structure definitions.
 */

/**
 * @brief Internal representation of an iOS UIKit window.
 */
struct cmn_window_s {
  cmn_widget_header_t base;  /**< Tagged polymorphic widget header. */
  UIWindow *ui_window;       /**< Native UIWindow instance. */
  UIViewController *root_vc; /**< Root UIViewController instance. */
  cmn_app_t *app;            /**< Parent application handle. */
};

/**
 * @brief Internal representation of an iOS UIButton widget.
 */
struct cmn_button_s {
  cmn_widget_header_t base;     /**< Tagged polymorphic widget header. */
  UIButton *ui_button;          /**< Native UIButton instance. */
  cmn_button_click_fn on_click; /**< Click callback function. */
  void *userdata;               /**< User data passed to `on_click`. */
  id target;                    /**< Objective-C target instance. */
};

/**
 * @brief Internal representation of an iOS UILabel widget.
 */
struct cmn_label_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  UILabel *ui_label;        /**< Native UILabel instance. */
};

/**
 * @brief Internal representation of an iOS UITextField widget.
 */
struct cmn_textbox_s {
  cmn_widget_header_t base;  /**< Tagged polymorphic widget header. */
  UITextField *ui_textfield; /**< Native UITextField instance. */
  char text_cache[1024];     /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of an iOS UITextView widget.
 */
struct cmn_textview_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  UITextView *ui_textview;  /**< Native UITextView instance. */
  char text_cache[4096];    /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of an iOS vertical UIStackView container.
 */
struct cmn_vbox_s {
  cmn_widget_header_t base;  /**< Tagged polymorphic widget header. */
  UIStackView *ui_stackview; /**< Native vertical UIStackView instance. */
};

/**
 * @brief Internal representation of an iOS horizontal UIStackView container.
 */
struct cmn_hbox_s {
  cmn_widget_header_t base;  /**< Tagged polymorphic widget header. */
  UIStackView *ui_stackview; /**< Native horizontal UIStackView instance. */
};

/**
 * @brief Internal representation of an iOS menu stub.
 */
struct cmn_menu_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  int dummy;                /**< Placeholder field. */
};

/**
 * @brief Internal representation of an iOS file picker dialog.
 */
struct cmn_filepicker_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  char path[256];           /**< Selected UTF-8 file path. */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_IOS_INTERNAL_H */
