#ifndef CMN_WIDGET_H
#define CMN_WIDGET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
/* clang-format on */

/**
 * @file widget.h
 * @brief Core UI widgets, containers, menus, and windowing ontology.
 */

/**
 * @defgroup cmn_widget Widgets and Windowing
 * @brief Widgets and Windowing module.
 * @{
 */

/* --- Window --- */

/**
 * @brief Window style flags enumeration and bitmask.
 */
typedef enum cmn_window_style_e {
  /** @brief Default titled, resizable, closable window style. */
  CMN_WINDOW_STYLE_DEFAULT = 0,
  /** @brief Frameless borderless window style. */
  CMN_WINDOW_STYLE_FRAMELESS = 1 << 0,
  /** @brief Transparent background window style. */
  CMN_WINDOW_STYLE_TRANSPARENT = 1 << 1
} cmn_window_style_e;

/**
 * @brief Creates a new window for the given application.
 * @param app The application instance.
 * @param out_window Pointer to store the created window.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_create(cmn_app_t *app,
                                             cmn_window_t **out_window);

/**
 * @brief Sets the title of a window.
 * @param window The window to modify.
 * @param title The new title string (UTF-8).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_title(cmn_window_t *window,
                                                const char *title);

/**
 * @brief Sets the style of a window (e.g., frameless, transparent).
 * @param window The window to modify.
 * @param style Bitmask of `cmn_window_style_e` flags.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_style(cmn_window_t *window,
                                                cmn_window_style_e style);

/**
 * @brief Sets the fullscreen state of a window.
 * @param window The window to modify.
 * @param is_fullscreen `true` (non-zero) for fullscreen, `false` (0) for
 * windowed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_fullscreen(cmn_window_t *window,
                                                     cmn_bool_t is_fullscreen);

/**
 * @brief Maximizes a window.
 * @param window The window to maximize.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_maximize(cmn_window_t *window);

/**
 * @brief Minimizes a window.
 * @param window The window to minimize.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_minimize(cmn_window_t *window);

/**
 * @brief Centers a window on a specific display.
 * @param window The window to center.
 * @param display_index The 0-based index of the display.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_center_on_display(cmn_window_t *window,
                                                        int display_index);

/**
 * @brief Gets the DPI scale factor for a window.
 * @param window The window to query.
 * @param out_scale Pointer to store the DPI scale factor (e.g., 1.0f, 2.0f).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_get_dpi_scale(cmn_window_t *window,
                                                    float *out_scale);

/**
 * @brief Gets the number of available displays/monitors.
 * @param out_count Pointer to store the display count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_display_get_count(int *out_count);

/**
 * @brief Shows a window on the screen.
 * @param window The window to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_show(cmn_window_t *window);

/**
 * @brief Destroys a window and frees its memory.
 * @param window Pointer to the window pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_destroy(cmn_window_t **window);

/* --- Multiwindow & Dialogs --- */

/**
 * @brief Shows a modal dialog attached to a parent window.
 * @param parent The parent window.
 * @param modal The modal window to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_dialog_show_modal(cmn_window_t *parent,
                                                 cmn_window_t *modal);

/**
 * @brief Shows a native message box alert.
 * @param title The title of the message box.
 * @param msg The message body of the message box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_message_box_show(const char *title,
                                                const char *msg);

/* --- Type-Safe Upcasts & Polymorphic Widget API --- */

/**
 * @brief Upcasts a `cmn_window_t` to a `cmn_container_t`.
 * @param window The window instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e
cmn_window_as_container(cmn_window_t *window, cmn_container_t **out_container);

/**
 * @brief Upcasts a `cmn_vbox_t` to a `cmn_container_t`.
 * @param vbox The vertical box instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e
cmn_vbox_as_container(cmn_vbox_t *vbox, cmn_container_t **out_container);

/**
 * @brief Upcasts a `cmn_hbox_t` to a `cmn_container_t`.
 * @param hbox The horizontal box instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e
cmn_hbox_as_container(cmn_hbox_t *hbox, cmn_container_t **out_container);

/**
 * @brief Upcasts a `cmn_vbox_t` to a `cmn_widget_t`.
 * @param vbox The vertical box instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e cmn_vbox_as_widget(cmn_vbox_t *vbox,
                                              cmn_widget_t **out_widget);

/**
 * @brief Upcasts a `cmn_hbox_t` to a `cmn_widget_t`.
 * @param hbox The horizontal box instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e cmn_hbox_as_widget(cmn_hbox_t *hbox,
                                              cmn_widget_t **out_widget);

/**
 * @brief Upcasts a `cmn_button_t` to a `cmn_widget_t`.
 * @param button The button instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_as_widget(cmn_button_t *button,
                                                cmn_widget_t **out_widget);

/**
 * @brief Upcasts a `cmn_label_t` to a `cmn_widget_t`.
 * @param label The label instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_as_widget(cmn_label_t *label,
                                               cmn_widget_t **out_widget);

/**
 * @brief Upcasts a `cmn_textbox_t` to a `cmn_widget_t`.
 * @param textbox The textbox instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_as_widget(cmn_textbox_t *textbox,
                                                 cmn_widget_t **out_widget);

/**
 * @brief Upcasts a `cmn_textview_t` to a `cmn_widget_t`.
 * @param textview The textview instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if arguments are NULL.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_as_widget(cmn_textview_t *textview,
                                                  cmn_widget_t **out_widget);

/**
 * @brief Polymorphically destroys any widget handle created by cmn.
 * @param widget Pointer to the widget pointer to destroy and set to NULL.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_destroy(cmn_widget_t **widget);

/**
 * @brief Sets the parent container of a child widget using validated handles.
 * @param parent The parent container or window (`cmn_container_t *`,
 * `cmn_window_t *`, `cmn_vbox_t *`, or `cmn_hbox_t *`).
 * @param child The child widget (`cmn_widget_t *` or any concrete widget).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_set_parent(void *parent, void *child);

/* --- Button --- */

/**
 * @brief Callback function type for button click events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_button_click_fn)(void *userdata);

/**
 * @brief Creates a new button widget.
 * @param out_button Pointer to store the created button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_create(cmn_button_t **out_button);

/**
 * @brief Sets the text of a button.
 * @param button The button to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_set_text(cmn_button_t *button,
                                               const char *text);

/**
 * @brief Sets the click callback for a button.
 * @param button The button to modify.
 * @param on_click The callback function.
 * @param userdata User data to pass to the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_set_on_click(cmn_button_t *button,
                                                   cmn_button_click_fn on_click,
                                                   void *userdata);

/**
 * @brief Programmatically triggers a button's click event.
 * @param button The button to click.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_trigger_click(cmn_button_t *button);

/**
 * @brief Destroys a button and frees its memory.
 * @param button Pointer to the button pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_destroy(cmn_button_t **button);

/* --- Label --- */

/**
 * @brief Creates a new label widget.
 * @param out_label Pointer to store the created label.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_create(cmn_label_t **out_label);

/**
 * @brief Sets the text of a label.
 * @param label The label to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_set_text(cmn_label_t *label,
                                              const char *text);

/**
 * @brief Destroys a label and frees its memory.
 * @param label Pointer to the label pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_destroy(cmn_label_t **label);

/* --- TextBox --- */

/**
 * @brief Creates a new text box widget.
 * @param out_textbox Pointer to store the created text box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_create(cmn_textbox_t **out_textbox);

/**
 * @brief Sets the text of a text box.
 * @param textbox The text box to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_set_text(cmn_textbox_t *textbox,
                                                const char *text);

/**
 * @brief Configures whether a text box masks its characters using the host OS
 * native password field behavior (`NSSecureTextField`, `ES_PASSWORD` /
 * `EM_SETPASSWORDCHAR`, `gtk_entry_set_visibility`, `<input type="password">`,
 * `secureTextEntry`).
 * @param textbox The text box to modify.
 * @param is_password Non-zero (`1`) to enable password masking, `0` for plain
 * text.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_set_password(cmn_textbox_t *textbox,
                                                    int is_password);

/**
 * @brief Gets the current text of a text box.
 * @param textbox The text box to query.
 * @param out_text Pointer to store the text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_get_text(cmn_textbox_t *textbox,
                                                const char **out_text);

/**
 * @brief Destroys a text box and frees its memory.
 * @param textbox Pointer to the text box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_destroy(cmn_textbox_t **textbox);

#ifdef CMN_TESTING
/**
 * @brief Sets mock failure state for cmn_textbox_get_text.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_mock_textbox_get_text_set(int fail);
#endif

/* --- TextView (Multi-line Scrollable) --- */

/**
 * @brief Creates a new multi-line scrollable text view widget.
 * @param out_textview Pointer to store the created text view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_create(cmn_textview_t **out_textview);

/**
 * @brief Sets the entire text of a text view.
 * @param textview The text view to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_set_text(cmn_textview_t *textview,
                                                 const char *text);

/**
 * @brief Appends text to the end of a text view and scrolls to the bottom.
 * @param textview The text view to modify.
 * @param text The text string to append.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_append_text(cmn_textview_t *textview,
                                                    const char *text);

/**
 * @brief Gets the current text of a text view.
 * @param textview The text view to query.
 * @param out_text Pointer to store the text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_get_text(cmn_textview_t *textview,
                                                 const char **out_text);

/**
 * @brief Destroys a text view and frees its memory.
 * @param textview Pointer to the text view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_destroy(cmn_textview_t **textview);

#ifdef CMN_TESTING
/**
 * @brief Sets mock failure state for cmn_textview_get_text.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_mock_textview_get_text_set(int fail);
#endif

/* --- Layout Containers --- */

/**
 * @brief Creates a vertical box layout container.
 * @param out_vbox Pointer to store the created vertical box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_vbox_create(cmn_vbox_t **out_vbox);

/**
 * @brief Destroys a vertical box layout container.
 * @param vbox Pointer to the vertical box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_vbox_destroy(cmn_vbox_t **vbox);

/**
 * @brief Creates a horizontal box layout container.
 * @param out_hbox Pointer to store the created horizontal box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_hbox_create(cmn_hbox_t **out_hbox);

/**
 * @brief Destroys a horizontal box layout container.
 * @param hbox Pointer to the horizontal box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_hbox_destroy(cmn_hbox_t **hbox);

/* --- Menus & System Tray --- */

/** @brief Opaque handle for a menu. */
typedef struct cmn_menu_s cmn_menu_t;
/** @brief Opaque handle for a menu item. */
typedef struct cmn_menu_item_s cmn_menu_item_t;

/**
 * @brief Creates a new menu.
 * @param out_menu Pointer to store the created menu.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_create(cmn_menu_t **out_menu);

/**
 * @brief Destroys a menu and frees its memory.
 * @param menu Pointer to the menu pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_destroy(cmn_menu_t **menu);

/**
 * @brief Adds an item to a menu.
 * @param menu The menu to add the item to.
 * @param title The text of the menu item.
 * @param on_click The callback function when the item is clicked.
 * @param userdata User data passed to the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_add_item(cmn_menu_t *menu,
                                             const char *title,
                                             cmn_error_e (*on_click)(void *),
                                             void *userdata);

/**
 * @brief Programmatically triggers a menu item's callback by index.
 * @param menu The menu containing the item.
 * @param index The 0-based index of the menu item to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_trigger_item(cmn_menu_t *menu, int index);

/**
 * @brief Sets the main menu bar for a window.
 * @param window The window to set the menu on.
 * @param menu The menu to attach.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_menu(cmn_window_t *window,
                                               cmn_menu_t *menu);

/* --- Native File Picker --- */

/** @brief Opaque handle for a file picker dialog. */
typedef struct cmn_filepicker_s cmn_filepicker_t;

/**
 * @brief Creates a new file picker dialog.
 * @param out_filepicker Pointer to store the created file picker.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_filepicker_create(cmn_filepicker_t **out_filepicker);

/**
 * @brief Shows the file picker dialog and waits for user selection.
 * @param filepicker The file picker to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_filepicker_show_dialog(cmn_filepicker_t *filepicker);

/**
 * @brief Gets the selected file path from the file picker.
 * @param filepicker The file picker to query.
 * @param out_path Pointer to store the selected file path string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_filepicker_get_path(cmn_filepicker_t *filepicker,
                                                   const char **out_path);

/**
 * @brief Destroys a file picker and frees its memory.
 * @param filepicker Pointer to the file picker pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_filepicker_destroy(cmn_filepicker_t **filepicker);

#ifdef CMN_TESTING
/**
 * @brief Sets mock bypass state for cmn_filepicker_show_dialog.
 * @param bypass 1 to bypass, 0 for normal behavior.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_mock_filepicker_show_dialog_set(int bypass);
#endif

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_WIDGET_H */
