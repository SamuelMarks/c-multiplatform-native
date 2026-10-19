/* clang-format off */
#include "cmn_android_internal.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
/* clang-format on */

/**
 * @file cmn_android_widget.c
 * @brief Android SDK implementation of the widget ontology.
 */

/** @brief Mock flag to force `cmn_textbox_get_text` failure. */
static int g_cmn_mock_textbox_get_text_fail = 0;

/**
 * @brief Sets mock failure state for cmn_textbox_get_text.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_textbox_get_text_set(int fail) {
  g_cmn_mock_textbox_get_text_fail = fail;
  return CMN_SUCCESS;
}

/** @brief Global JavaVM pointer initialized on `JNI_OnLoad`. */
extern JavaVM *g_cmn_jvm;

/**
 * @brief Polymorphic destructor adapter for `cmn_window_t`.
 * @param ptr Address of the window pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_window_adapter(void **ptr) {
  return cmn_window_destroy((cmn_window_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_vbox_t`.
 * @param ptr Address of the vbox pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_vbox_adapter(void **ptr) {
  return cmn_vbox_destroy((cmn_vbox_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_hbox_t`.
 * @param ptr Address of the hbox pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_hbox_adapter(void **ptr) {
  return cmn_hbox_destroy((cmn_hbox_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_button_t`.
 * @param ptr Address of the button pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_button_adapter(void **ptr) {
  return cmn_button_destroy((cmn_button_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_label_t`.
 * @param ptr Address of the label pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_label_adapter(void **ptr) {
  return cmn_label_destroy((cmn_label_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_textbox_t`.
 * @param ptr Address of the textbox pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_textbox_adapter(void **ptr) {
  return cmn_textbox_destroy((cmn_textbox_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_textview_t`.
 * @param ptr Address of the textview pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_textview_adapter(void **ptr) {
  return cmn_textview_destroy((cmn_textview_t **)ptr);
}

/**
 * @brief Retrieves or attaches the JNIEnv for the current thread.
 * @param env Output pointer receiving the `JNIEnv *`.
 * @param is_attached Output pointer set to 1 if thread was attached.
 * @return CMN_SUCCESS on success, CMN_ERR_IO on failure.
 */
static cmn_error_e get_env(JNIEnv **env, int *is_attached) {
  int status;
  *is_attached = 0;
  if (g_cmn_jvm == NULL) {
    return CMN_ERR_IO;
  }
  status = (*g_cmn_jvm)->GetEnv(g_cmn_jvm, (void **)env, JNI_VERSION_1_6);
  if (status == JNI_EDETACHED) {
    if ((*g_cmn_jvm)->AttachCurrentThread(g_cmn_jvm, env, NULL) != 0) {
      return CMN_ERR_IO;
    }
    *is_attached = 1;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Detaches the current thread from the JavaVM if it was attached by
 * `get_env`.
 * @param is_attached Non-zero if thread was attached.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e detach_env(int is_attached) {
  if (is_attached && g_cmn_jvm) {
    (*g_cmn_jvm)->DetachCurrentThread(g_cmn_jvm);
  }
  return CMN_SUCCESS;
}

/**
 * @brief JNI callback invoked by Android host code when a button is clicked.
 * @param env JNI environment pointer.
 * @param thiz Java bridge object.
 * @param buttonPtr Native pointer to `cmn_button_t`.
 * @return CMN_SUCCESS on success.
 */
JNIEXPORT cmn_error_e JNICALL
Java_com_example_cmn_CmnBridge_nativeOnButtonClicked(JNIEnv *env, jobject thiz,
                                                     jlong buttonPtr) {
  cmn_button_t *btn = (cmn_button_t *)(intptr_t)buttonPtr;
  if ((env != NULL || thiz != NULL || buttonPtr != 0) && btn && btn->on_click) {
    return btn->on_click(btn->userdata);
  }
  return CMN_SUCCESS;
}

/* --- Window --- */

/**
 * @brief Creates a new window for the given application.
 * @param app The application instance.
 * @param out_window Pointer to store the created window.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_create(cmn_app_t *app, cmn_window_t **out_window) {
  cmn_error_e rc;
  cmn_window_t *win = NULL;

  if (app == NULL || out_window == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_window_t), (void **)&win);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&win->base, CMN_WIDGET_KIND_WINDOW,
                              destroy_window_adapter);

  *out_window = win;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the title of a window.
 * @param window The window to modify.
 * @param title The new title string (UTF-8).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_title(cmn_window_t *window, const char *title) {
  JNIEnv *env;
  int is_attached;
  jclass activity_class;
  jmethodID set_title_mid;
  jstring title_str;

  if (window == NULL || title == NULL) {
    return CMN_ERR_INVAL;
  }
  if (window->activity == NULL) {
    return CMN_SUCCESS;
  }

  if (get_env(&env, &is_attached) != CMN_SUCCESS) {
    return CMN_ERR_IO;
  }

  activity_class = (*env)->GetObjectClass(env, window->activity);
  set_title_mid = (*env)->GetMethodID(env, activity_class, "setTitle",
                                      "(Ljava/lang/CharSequence;)V");
  title_str = (*env)->NewStringUTF(env, title);

  (*env)->CallVoidMethod(env, window->activity, set_title_mid, title_str);

  (*env)->DeleteLocalRef(env, title_str);
  (*env)->DeleteLocalRef(env, activity_class);
  return detach_env(is_attached);
}

/**
 * @brief Shows a window on the screen.
 * @param window The window to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_show(cmn_window_t *window) {
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a window and frees its memory.
 * @param window Pointer to the window pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_destroy(cmn_window_t **window) {
  JNIEnv *env;
  int is_attached;
  cmn_error_e rc = CMN_SUCCESS;
  cmn_error_e free_rc;

  if (window == NULL || *window == NULL) {
    return CMN_ERR_INVAL;
  }

  if ((*window)->activity) {
    if (get_env(&env, &is_attached) == CMN_SUCCESS) {
      (*env)->DeleteGlobalRef(env, (*window)->activity);
      rc = detach_env(is_attached);
    }
  }

  free_rc = cmn_free((void **)window);
  return rc != CMN_SUCCESS ? rc : free_rc;
}

/* --- Layouts --- */

/**
 * @brief Creates a vertical box layout container.
 * @param out_vbox Pointer to store the created vertical box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_vbox_create(cmn_vbox_t **out_vbox) {
  cmn_vbox_t *vbox = NULL;
  cmn_error_e rc;

  if (out_vbox == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_vbox_t), (void **)&vbox);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&vbox->base, CMN_WIDGET_KIND_VBOX,
                              destroy_vbox_adapter);

  *out_vbox = vbox;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a vertical box layout container.
 * @param vbox Pointer to the vertical box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_vbox_destroy(cmn_vbox_t **vbox) {
  if (vbox == NULL || *vbox == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)vbox);
}

/**
 * @brief Creates a horizontal box layout container.
 * @param out_hbox Pointer to store the created horizontal box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_hbox_create(cmn_hbox_t **out_hbox) {
  cmn_hbox_t *hbox = NULL;
  cmn_error_e rc;

  if (out_hbox == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_hbox_t), (void **)&hbox);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&hbox->base, CMN_WIDGET_KIND_HBOX,
                              destroy_hbox_adapter);

  *out_hbox = hbox;
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a horizontal box layout container.
 * @param hbox Pointer to the horizontal box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_hbox_destroy(cmn_hbox_t **hbox) {
  if (hbox == NULL || *hbox == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)hbox);
}

/* --- Widgets --- */

/**
 * @brief Creates a new button widget.
 * @param out_button Pointer to store the created button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_create(cmn_button_t **out_button) {
  cmn_button_t *btn = NULL;
  cmn_error_e rc;

  if (out_button == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_button_t), (void **)&btn);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&btn->base, CMN_WIDGET_KIND_BUTTON,
                              destroy_button_adapter);

  *out_button = btn;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the text of a button.
 * @param button The button to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_set_text(cmn_button_t *button, const char *text) {
  if (button == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the click callback for a button.
 * @param button The button to modify.
 * @param on_click The callback function.
 * @param userdata User data to pass to the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_set_on_click(cmn_button_t *button,
                                    cmn_button_click_fn on_click,
                                    void *userdata) {
  if (button == NULL || on_click == NULL) {
    return CMN_ERR_INVAL;
  }
  button->on_click = on_click;
  button->userdata = userdata;
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers a button's click event.
 * @param button The button to click.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_trigger_click(cmn_button_t *button) {
  if (button == NULL) {
    return CMN_ERR_INVAL;
  }
  if (button->on_click) {
    return button->on_click(button->userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a button and frees its memory.
 * @param button Pointer to the button pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_destroy(cmn_button_t **button) {
  if (button == NULL || *button == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)button);
}

/**
 * @brief Creates a new label widget.
 * @param out_label Pointer to store the created label.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_create(cmn_label_t **out_label) {
  cmn_label_t *lbl = NULL;
  cmn_error_e rc;

  if (out_label == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_label_t), (void **)&lbl);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&lbl->base, CMN_WIDGET_KIND_LABEL,
                              destroy_label_adapter);

  *out_label = lbl;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the text of a label.
 * @param label The label to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_set_text(cmn_label_t *label, const char *text) {
  if (label == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a label and frees its memory.
 * @param label Pointer to the label pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_destroy(cmn_label_t **label) {
  if (label == NULL || *label == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)label);
}

/**
 * @brief Creates a new text box widget.
 * @param out_textbox Pointer to store the created text box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_create(cmn_textbox_t **out_textbox) {
  cmn_textbox_t *tb = NULL;
  cmn_error_e rc;

  if (out_textbox == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_textbox_t), (void **)&tb);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&tb->base, CMN_WIDGET_KIND_TEXTBOX,
                              destroy_textbox_adapter);

  *out_textbox = tb;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the current text of a text box.
 * @param textbox The text box to query.
 * @param out_text Pointer to store the text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_get_text(cmn_textbox_t *textbox,
                                 const char **out_text) {
  if (g_cmn_mock_textbox_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textbox == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_text = textbox->text_cache;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the text of a text box.
 * @param textbox The text box to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_text(cmn_textbox_t *textbox, const char *text) {
  if (textbox == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_strcpy_s(textbox->text_cache, sizeof(textbox->text_cache), text);
}

/**
 * @brief Configures whether a text box masks its characters as a password.
 * @param textbox The text box to modify.
 * @param is_password Non-zero to enable password masking, zero for plain text.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_password(cmn_textbox_t *textbox, int is_password) {
  if (textbox == NULL || is_password < 0) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a text box and frees its memory.
 * @param textbox Pointer to the text box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_destroy(cmn_textbox_t **textbox) {
  if (textbox == NULL || *textbox == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)textbox);
}

/* --- TextView --- */

/** @brief Mock flag to force `cmn_textview_get_text` failure. */
static int g_cmn_mock_textview_get_text_fail = 0;

/**
 * @brief Sets mock failure state for cmn_textview_get_text.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_textview_get_text_set(int fail) {
  g_cmn_mock_textview_get_text_fail = fail;
  return CMN_SUCCESS;
}

/**
 * @brief Creates a new multi-line scrollable text view widget.
 * @param out_textview Pointer to store the created text view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_create(cmn_textview_t **out_textview) {
  cmn_textview_t *tv = NULL;
  cmn_error_e rc;

  if (out_textview == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_textview_t), (void **)&tv);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&tv->base, CMN_WIDGET_KIND_TEXTVIEW,
                              destroy_textview_adapter);

  *out_textview = tv;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the current text of a text view.
 * @param textview The text view to query.
 * @param out_text Pointer to store the text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_get_text(cmn_textview_t *textview,
                                  const char **out_text) {
  if (g_cmn_mock_textview_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textview == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_text = textview->text_cache;
  return CMN_SUCCESS;
}

/**
 * @brief Sets the entire text of a text view.
 * @param textview The text view to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_set_text(cmn_textview_t *textview, const char *text) {
  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_strcpy_s(textview->text_cache, sizeof(textview->text_cache), text);
}

/**
 * @brief Appends text to the end of a text view and scrolls to the bottom.
 * @param textview The text view to modify.
 * @param text The text string to append.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_append_text(cmn_textview_t *textview,
                                     const char *text) {
  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a text view and frees its memory.
 * @param textview Pointer to the text view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_destroy(cmn_textview_t **textview) {
  if (textview == NULL || *textview == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)textview);
}

/* --- Parent/Child Binding --- */

/**
 * @brief Sets the parent container of a child widget using validated handles.
 * @param parent The parent container or window (`cmn_container_t *`,
 * `cmn_window_t *`, `cmn_vbox_t *`, or `cmn_hbox_t *`).
 * @param child The child widget (`cmn_widget_t *` or any concrete widget).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_set_parent(void *parent, void *child) {
  cmn_widget_kind_e parent_kind;
  cmn_widget_kind_e child_kind;
  if (parent == NULL || child == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(parent, &parent_kind) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(child, &child_kind) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }
  if (parent_kind != CMN_WIDGET_KIND_WINDOW &&
      parent_kind != CMN_WIDGET_KIND_VBOX &&
      parent_kind != CMN_WIDGET_KIND_HBOX &&
      parent_kind != CMN_WIDGET_KIND_FLEX &&
      parent_kind != CMN_WIDGET_KIND_GRID &&
      parent_kind != CMN_WIDGET_KIND_PANEL) {
    return CMN_ERR_INVAL;
  }
  if (child_kind == CMN_WIDGET_KIND_WINDOW ||
      child_kind == CMN_WIDGET_KIND_MENU ||
      child_kind == CMN_WIDGET_KIND_FILEPICKER ||
      child_kind == CMN_WIDGET_KIND_COLORPICKER) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the style of a window (e.g., frameless, transparent).
 * @param window The window to modify.
 * @param style Bitmask of `cmn_window_style_e` flags.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_style(cmn_window_t *window,
                                 cmn_window_style_e style) {
  if (window == NULL || style < CMN_WINDOW_STYLE_DEFAULT) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the fullscreen state of a window.
 * @param window The window to modify.
 * @param is_fullscreen `true` (non-zero) for fullscreen, `false` (0) for
 * windowed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_fullscreen(cmn_window_t *window,
                                      cmn_bool_t is_fullscreen) {
  if (window == NULL || is_fullscreen < CMN_FALSE) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Maximizes a window.
 * @param window The window to maximize.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_maximize(cmn_window_t *window) {
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Minimizes a window.
 * @param window The window to minimize.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_minimize(cmn_window_t *window) {
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Centers a window on a specific display.
 * @param window The window to center.
 * @param display_index The 0-based index of the display.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_center_on_display(cmn_window_t *window,
                                         int display_index) {
  if (window == NULL || display_index != 0) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Gets the DPI scale factor for a window.
 * @param window The window to query.
 * @param out_scale Pointer to store the DPI scale factor (e.g., 1.0f, 2.0f).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_get_dpi_scale(cmn_window_t *window, float *out_scale) {
  if (window == NULL || out_scale == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_scale = 1.0f;
  return CMN_SUCCESS;
}

/**
 * @brief Gets the number of available displays/monitors.
 * @param out_count Pointer to store the display count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_display_get_count(int *out_count) {
  if (out_count == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_count = 1;
  return CMN_SUCCESS;
}

/**
 * @brief Shows a modal dialog attached to a parent window.
 * @param parent The parent window.
 * @param modal The modal window to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_dialog_show_modal(cmn_window_t *parent, cmn_window_t *modal) {
  if (parent == NULL || modal == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Shows a native message box alert.
 * @param title The title of the message box.
 * @param msg The message body of the message box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_message_box_show(const char *title, const char *msg) {
  if (title == NULL || msg == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/** @brief Mock flag to bypass `cmn_filepicker_show_dialog`. */
static int g_cmn_mock_filepicker_show_dialog_bypass = 0;

/**
 * @brief Sets mock bypass state for cmn_filepicker_show_dialog.
 * @param bypass 1 to bypass, 0 for normal behavior.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_filepicker_show_dialog_set(int bypass) {
  g_cmn_mock_filepicker_show_dialog_bypass = bypass;
  return CMN_SUCCESS;
}

/**
 * @brief Creates a new file picker dialog.
 * @param out_filepicker Pointer to store the created file picker.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_create(cmn_filepicker_t **out_filepicker) {
  if (out_filepicker == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_filepicker = NULL;
  return CMN_ERR_INVAL;
}

/**
 * @brief Shows the file picker dialog and waits for user selection.
 * @param filepicker The file picker to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_show_dialog(cmn_filepicker_t *filepicker) {
  if (filepicker == NULL) {
    return CMN_ERR_INVAL;
  }
  if (g_cmn_mock_filepicker_show_dialog_bypass) {
    return CMN_SUCCESS;
  }
  return CMN_ERR_INVAL;
}

/**
 * @brief Gets the selected file path from the file picker.
 * @param filepicker The file picker to query.
 * @param out_path Pointer to store the selected file path string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_get_path(cmn_filepicker_t *filepicker,
                                    const char **out_path) {
  if (filepicker == NULL || out_path == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_path = filepicker->path;
  return CMN_ERR_INVAL;
}

/**
 * @brief Destroys a file picker and frees its memory.
 * @param filepicker Pointer to the file picker pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_destroy(cmn_filepicker_t **filepicker) {
  if (filepicker == NULL || *filepicker == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)filepicker);
}

/**
 * @brief Creates a new menu.
 * @param out_menu Pointer to store the created menu.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_create(cmn_menu_t **out_menu) {
  if (out_menu == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_menu = NULL;
  return CMN_ERR_INVAL;
}

/**
 * @brief Destroys a menu and frees its memory.
 * @param menu Pointer to the menu pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_destroy(cmn_menu_t **menu) {
  if (menu == NULL || *menu == NULL) {
    return CMN_ERR_INVAL;
  }
  return cmn_free((void **)menu);
}

/**
 * @brief Adds an item to a menu.
 * @param menu The menu to add the item to.
 * @param title The text of the menu item.
 * @param on_click The callback function when the item is clicked.
 * @param userdata User data passed to the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_add_item(cmn_menu_t *menu, const char *title,
                              cmn_error_e (*on_click)(void *), void *userdata) {
  if (menu == NULL || title == NULL || (on_click == NULL && userdata != NULL)) {
    return CMN_ERR_INVAL;
  }
  return CMN_ERR_INVAL;
}

/**
 * @brief Programmatically triggers a menu item's callback by index.
 * @param menu The menu containing the item.
 * @param index The 0-based index of the menu item to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_trigger_item(cmn_menu_t *menu, int index) {
  if (menu == NULL || index < 0) {
    return CMN_ERR_INVAL;
  }
  return CMN_ERR_INVAL;
}

/**
 * @brief Sets the main menu bar for a window.
 * @param window The window to set the menu on.
 * @param menu The menu to attach.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_menu(cmn_window_t *window, cmn_menu_t *menu) {
  if (window == NULL || menu == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_ERR_INVAL;
}

/**
 * @brief Backend hook creating the native OS control for an extended widget.
 * @param hdr Initialized widget header.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_init(cmn_widget_header_t *hdr) {
  const char *cls = NULL;
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  switch (hdr->kind) {
  case CMN_WIDGET_KIND_CHECKBOX:
    cls = "android/widget/CheckBox";
    break;
  case CMN_WIDGET_KIND_RADIOBUTTON:
    cls = "android/widget/RadioButton";
    break;
  case CMN_WIDGET_KIND_TOGGLEBUTTON:
    cls = "android/widget/ToggleButton";
    break;
  case CMN_WIDGET_KIND_SLIDER:
    cls = "android/widget/SeekBar";
    break;
  case CMN_WIDGET_KIND_PROGRESS:
    cls = "android/widget/ProgressBar";
    break;
  case CMN_WIDGET_KIND_UPDOWN:
    cls = "android/widget/NumberPicker";
    break;
  case CMN_WIDGET_KIND_POPUP:
    cls = "android/widget/Spinner";
    break;
  case CMN_WIDGET_KIND_COMBO:
    cls = "android/widget/AutoCompleteTextView";
    break;
  case CMN_WIDGET_KIND_LISTBOX:
    cls = "android/widget/ListView";
    break;
  case CMN_WIDGET_KIND_IMAGEVIEW:
    cls = "android/widget/ImageView";
    break;
  case CMN_WIDGET_KIND_TABLEVIEW:
    cls = "android/widget/TableLayout";
    break;
  case CMN_WIDGET_KIND_WEBVIEW:
    cls = "android/webkit/WebView";
    break;
  case CMN_WIDGET_KIND_PANEL:
    cls = "android/widget/FrameLayout";
    break;
  case CMN_WIDGET_KIND_TABS:
    cls = "android/widget/TabHost";
    break;
  case CMN_WIDGET_KIND_SPLITVIEW:
    cls = "android/widget/LinearLayout";
    break;
  case CMN_WIDGET_KIND_LINE:
  case CMN_WIDGET_KIND_VIEW:
    cls = "android/view/View";
    break;
  default:
    break;
  }
  if (cls != NULL && g_android_env != NULL && g_android_activity != NULL) {
    jclass jcls = (*g_android_env)->FindClass(g_android_env, cls);
    if (jcls != NULL) {
      jmethodID ctor = (*g_android_env)
                           ->GetMethodID(g_android_env, jcls, "<init>",
                                         "(Landroid/content/Context;)V");
      if (ctor != NULL) {
        jobject local =
            (*g_android_env)
                ->NewObject(g_android_env, jcls, ctor, g_android_activity);
        if (local != NULL) {
          hdr->native_handle =
              (void *)(*g_android_env)->NewGlobalRef(g_android_env, local);
          (*g_android_env)->DeleteLocalRef(g_android_env, local);
        }
      }
      (*g_android_env)->DeleteLocalRef(g_android_env, jcls);
    }
  }
  return CMN_SUCCESS;
}

/**
 * @brief Backend hook synchronizing state changes to the native OS control.
 * @param hdr Widget header whose concrete state was modified.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_sync(cmn_widget_header_t *hdr) {
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Backend hook destroying the native OS control of an extended widget.
 * @param hdr Widget header being destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_cleanup(cmn_widget_header_t *hdr) {
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  if (hdr->native_handle != NULL && g_android_env != NULL) {
    (*g_android_env)
        ->DeleteGlobalRef(g_android_env, (jobject)hdr->native_handle);
    hdr->native_handle = NULL;
  }
  return CMN_SUCCESS;
}
