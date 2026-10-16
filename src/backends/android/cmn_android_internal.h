#ifndef CMN_ANDROID_INTERNAL_H
#define CMN_ANDROID_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn_internal.h"
#include "cmn/app.h"
#include "cmn/widget.h"
#include <jni.h>
/* clang-format on */

/**
 * @file cmn_android_internal.h
 * @brief Internal Android JNI widget and window structure definitions.
 */

/**
 * @brief Internal representation of an Android Activity window.
 */
struct cmn_window_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  jobject activity;         /**< Global JNI reference to Android Activity. */
};

/**
 * @brief Internal representation of an Android Button view.
 */
struct cmn_button_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  jobject button_view;      /**< JNI reference to `android.widget.Button`. */
  cmn_button_click_fn on_click; /**< Click callback function. */
  void *userdata;               /**< User data passed to `on_click`. */
};

/**
 * @brief Internal representation of an Android TextView label.
 */
struct cmn_label_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  jobject text_view;        /**< JNI reference to `android.widget.TextView`. */
};

/**
 * @brief Internal representation of an Android EditText widget.
 */
struct cmn_textbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  jobject edit_text;        /**< JNI reference to `android.widget.EditText`. */
  char text_cache[1024];    /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of an Android ScrollView + TextView widget.
 */
struct cmn_textview_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  jobject scroll_view;   /**< JNI reference to `android.widget.ScrollView`. */
  jobject text_view;     /**< JNI reference to `android.widget.TextView`. */
  char text_cache[4096]; /**< UTF-8 text cache buffer. */
};

/**
 * @brief Internal representation of an Android vertical LinearLayout.
 */
struct cmn_vbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  jobject linear_layout; /**< JNI reference to `android.widget.LinearLayout`. */
};

/**
 * @brief Internal representation of an Android horizontal LinearLayout.
 */
struct cmn_hbox_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  jobject linear_layout; /**< JNI reference to `android.widget.LinearLayout`. */
};

/**
 * @brief Internal representation of an Android menu stub.
 */
struct cmn_menu_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  int dummy;                /**< Placeholder field. */
};

/**
 * @brief Internal representation of an Android file picker stub.
 */
struct cmn_filepicker_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
  char path[256];           /**< Selected UTF-8 file path. */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_ANDROID_INTERNAL_H */
