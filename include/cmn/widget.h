#ifndef CMN_WIDGET_H
#define CMN_WIDGET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
#include "cmn/layout.h"
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
 * @brief Configures whether a text box masks its characters as a password.
 * @param textbox The text box to modify.
 * @param is_password Non-zero to enable password masking, zero for plain text.
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

/* --- CheckBox --- */

/**
 * @brief Callback function signature for checkbox toggle events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_checkbox_toggle_fn)(void *userdata);

/**
 * @brief Creates a new 2-state checkbox widget.
 * @param out_checkbox Pointer to store the created checkbox.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_checkbox_create(cmn_checkbox_t **out_checkbox);

/**
 * @brief Creates a new 3-state checkbox widget.
 * @param out_checkbox Pointer to store the created tri-state checkbox.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_checkbox_create_tristate(cmn_checkbox_t **out_checkbox);

/**
 * @brief Sets the label text of a checkbox.
 * @param checkbox The checkbox to modify.
 * @param text The new UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_checkbox_set_text(cmn_checkbox_t *checkbox,
                                                 const char *text);

/**
 * @brief Gets the label text of a checkbox.
 * @param checkbox The checkbox to query.
 * @param out_text Pointer to store the UTF-8 text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_checkbox_get_text(const cmn_checkbox_t *checkbox,
                                                 const char **out_text);

/**
 * @brief Sets the check state of a checkbox.
 * @param checkbox The checkbox to modify.
 * @param state Desired check state (`CMN_CHECK_OFF`, `CMN_CHECK_ON`, or
 * `CMN_CHECK_MIXED` if tri-state).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_checkbox_set_state(cmn_checkbox_t *checkbox,
                                                  cmn_check_state_e state);

/**
 * @brief Gets the current check state of a checkbox.
 * @param checkbox The checkbox to query.
 * @param out_state Pointer to store the current check state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_checkbox_get_state(
    const cmn_checkbox_t *checkbox, cmn_check_state_e *out_state);

/**
 * @brief Sets the toggle callback for a checkbox.
 * @param checkbox The checkbox to modify.
 * @param cb Callback function invoked when toggled.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_checkbox_set_on_toggle(cmn_checkbox_t *checkbox,
                                                      cmn_checkbox_toggle_fn cb,
                                                      void *userdata);

/**
 * @brief Programmatically toggles a checkbox and invokes its callback.
 * @param checkbox The checkbox to toggle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_checkbox_trigger_toggle(cmn_checkbox_t *checkbox);

/**
 * @brief Upcasts a `cmn_checkbox_t` to a `cmn_widget_t`.
 * @param checkbox The checkbox instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_checkbox_as_widget(cmn_checkbox_t *checkbox,
                                                  cmn_widget_t **out_widget);

/**
 * @brief Destroys a checkbox and frees its memory.
 * @param checkbox Pointer to the checkbox pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_checkbox_destroy(cmn_checkbox_t **checkbox);

/* --- RadioButton --- */

/**
 * @brief Callback function signature for radio button selection events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_radiobutton_select_fn)(void *userdata);

/**
 * @brief Creates a new radio button widget.
 * @param out_radiobutton Pointer to store the created radio button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_radiobutton_create(cmn_radiobutton_t **out_radiobutton);

/**
 * @brief Sets the label text of a radio button.
 * @param radiobutton The radio button to modify.
 * @param text The new UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_radiobutton_set_text(cmn_radiobutton_t *radiobutton, const char *text);

/**
 * @brief Gets the label text of a radio button.
 * @param radiobutton The radio button to query.
 * @param out_text Pointer to store the UTF-8 text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_radiobutton_get_text(
    const cmn_radiobutton_t *radiobutton, const char **out_text);

/**
 * @brief Sets whether a radio button is selected.
 * @param radiobutton The radio button to modify.
 * @param selected `CMN_TRUE` if selected, `CMN_FALSE` otherwise.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_radiobutton_set_selected(
    cmn_radiobutton_t *radiobutton, cmn_bool_t selected);

/**
 * @brief Gets whether a radio button is selected.
 * @param radiobutton The radio button to query.
 * @param out_selected Pointer to store the selection state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_radiobutton_get_selected(
    const cmn_radiobutton_t *radiobutton, cmn_bool_t *out_selected);

/**
 * @brief Sets the selection callback for a radio button.
 * @param radiobutton The radio button to modify.
 * @param cb Callback function invoked when selected.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_radiobutton_set_on_select(cmn_radiobutton_t *radiobutton,
                              cmn_radiobutton_select_fn cb, void *userdata);

/**
 * @brief Programmatically selects a radio button and invokes its callback.
 * @param radiobutton The radio button to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_radiobutton_trigger_select(cmn_radiobutton_t *radiobutton);

/**
 * @brief Upcasts a `cmn_radiobutton_t` to a `cmn_widget_t`.
 * @param radiobutton The radio button instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_radiobutton_as_widget(
    cmn_radiobutton_t *radiobutton, cmn_widget_t **out_widget);

/**
 * @brief Destroys a radio button and frees its memory.
 * @param radiobutton Pointer to the radio button pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_radiobutton_destroy(cmn_radiobutton_t **radiobutton);

/* --- ToggleButton --- */

/**
 * @brief Callback function signature for toggle button events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_togglebutton_toggle_fn)(void *userdata);

/**
 * @brief Creates a new flat/toggle button widget.
 * @param out_togglebutton Pointer to store the created toggle button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_togglebutton_create(cmn_togglebutton_t **out_togglebutton);

/**
 * @brief Sets the text of a toggle button.
 * @param togglebutton The toggle button to modify.
 * @param text The new UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_togglebutton_set_text(cmn_togglebutton_t *togglebutton, const char *text);

/**
 * @brief Gets the text of a toggle button.
 * @param togglebutton The toggle button to query.
 * @param out_text Pointer to store the UTF-8 text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_togglebutton_get_text(
    const cmn_togglebutton_t *togglebutton, const char **out_text);

/**
 * @brief Sets the toggled state of a toggle button.
 * @param togglebutton The toggle button to modify.
 * @param toggled `CMN_TRUE` if pressed/active, `CMN_FALSE` otherwise.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_togglebutton_set_toggled(
    cmn_togglebutton_t *togglebutton, cmn_bool_t toggled);

/**
 * @brief Gets the toggled state of a toggle button.
 * @param togglebutton The toggle button to query.
 * @param out_toggled Pointer to store the toggled state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_togglebutton_get_toggled(
    const cmn_togglebutton_t *togglebutton, cmn_bool_t *out_toggled);

/**
 * @brief Sets the toggle callback for a toggle button.
 * @param togglebutton The toggle button to modify.
 * @param cb Callback function invoked when toggled.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_togglebutton_set_on_toggle(cmn_togglebutton_t *togglebutton,
                               cmn_togglebutton_toggle_fn cb, void *userdata);

/**
 * @brief Programmatically toggles a toggle button and invokes its callback.
 * @param togglebutton The toggle button to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_togglebutton_trigger_toggle(cmn_togglebutton_t *togglebutton);

/**
 * @brief Upcasts a `cmn_togglebutton_t` to a `cmn_widget_t`.
 * @param togglebutton The toggle button instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_togglebutton_as_widget(
    cmn_togglebutton_t *togglebutton, cmn_widget_t **out_widget);

/**
 * @brief Destroys a toggle button and frees its memory.
 * @param togglebutton Pointer to the toggle button pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_togglebutton_destroy(cmn_togglebutton_t **togglebutton);

/* --- Line Separator --- */

/**
 * @brief Creates a horizontal separator line widget.
 * @param out_line Pointer to store the created line widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_line_create_horizontal(cmn_line_t **out_line);

/**
 * @brief Creates a vertical separator line widget.
 * @param out_line Pointer to store the created line widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_line_create_vertical(cmn_line_t **out_line);

/**
 * @brief Queries whether a separator line is vertical.
 * @param line The line widget to query.
 * @param out_vertical Pointer receiving `CMN_TRUE` if vertical, `CMN_FALSE` if
 * horizontal.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_line_is_vertical(const cmn_line_t *line,
                                                cmn_bool_t *out_vertical);

/**
 * @brief Upcasts a `cmn_line_t` to a `cmn_widget_t`.
 * @param line The line widget instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_line_as_widget(cmn_line_t *line,
                                              cmn_widget_t **out_widget);

/**
 * @brief Destroys a separator line widget and frees its memory.
 * @param line Pointer to the line widget pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_line_destroy(cmn_line_t **line);

/* --- Slider --- */

/**
 * @brief Callback function signature for slider movement events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_slider_moved_fn)(void *userdata);

/**
 * @brief Creates a horizontal slider widget.
 * @param out_slider Pointer to store the created slider.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_create(cmn_slider_t **out_slider);

/**
 * @brief Creates a vertical slider widget.
 * @param out_slider Pointer to store the created vertical slider.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_slider_create_vertical(cmn_slider_t **out_slider);

/**
 * @brief Queries whether a slider is oriented vertically.
 * @param slider The slider to query.
 * @param out_vertical Pointer receiving `CMN_TRUE` if vertical, `CMN_FALSE` if
 * horizontal.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_is_vertical(const cmn_slider_t *slider,
                                                  cmn_bool_t *out_vertical);

/**
 * @brief Sets the normalized value (`[0.0, 1.0]`) of a slider.
 * @param slider The slider to modify.
 * @param value Normalized position in `[0.0, 1.0]`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_set_value(cmn_slider_t *slider,
                                                double value);

/**
 * @brief Gets the normalized value (`[0.0, 1.0]`) of a slider.
 * @param slider The slider to query.
 * @param out_value Pointer to store the current value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_get_value(const cmn_slider_t *slider,
                                                double *out_value);

/**
 * @brief Sets the discrete tick step count of a slider.
 * @param slider The slider to modify.
 * @param steps Number of discrete steps (`0` for continuous).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_set_steps(cmn_slider_t *slider,
                                                uint32_t steps);

/**
 * @brief Gets the discrete tick step count of a slider.
 * @param slider The slider to query.
 * @param out_steps Pointer to store the discrete step count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_get_steps(const cmn_slider_t *slider,
                                                uint32_t *out_steps);

/**
 * @brief Sets the movement callback for a slider.
 * @param slider The slider to modify.
 * @param cb Callback function invoked when slider value changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_set_on_moved(cmn_slider_t *slider,
                                                   cmn_slider_moved_fn cb,
                                                   void *userdata);

/**
 * @brief Programmatically triggers the movement callback of a slider.
 * @param slider The slider to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_trigger_moved(cmn_slider_t *slider);

/**
 * @brief Upcasts a `cmn_slider_t` to a `cmn_widget_t`.
 * @param slider The slider instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_as_widget(cmn_slider_t *slider,
                                                cmn_widget_t **out_widget);

/**
 * @brief Destroys a slider and frees its memory.
 * @param slider Pointer to the slider pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_slider_destroy(cmn_slider_t **slider);

/* --- Progress Bar --- */

/**
 * @brief Creates a new progress bar widget.
 * @param out_progress Pointer to store the created progress bar.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_progress_create(cmn_progress_t **out_progress);

/**
 * @brief Sets the normalized value (`[0.0, 1.0]`) of a progress bar.
 * @param progress The progress bar to modify.
 * @param value Normalized completion ratio in `[0.0, 1.0]`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_progress_set_value(cmn_progress_t *progress,
                                                  double value);

/**
 * @brief Gets the normalized value (`[0.0, 1.0]`) of a progress bar.
 * @param progress The progress bar to query.
 * @param out_value Pointer to store the current value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_progress_get_value(const cmn_progress_t *progress, double *out_value);

/**
 * @brief Configures whether a progress bar is in indeterminate animation mode.
 * @param progress The progress bar to modify.
 * @param indeterminate `CMN_TRUE` for indeterminate mode, `CMN_FALSE` for
 * determinate mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_progress_set_indeterminate(
    cmn_progress_t *progress, cmn_bool_t indeterminate);

/**
 * @brief Queries whether a progress bar is in indeterminate mode.
 * @param progress The progress bar to query.
 * @param out_indeterminate Pointer receiving `CMN_TRUE` if indeterminate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_progress_get_indeterminate(
    const cmn_progress_t *progress, cmn_bool_t *out_indeterminate);

/**
 * @brief Upcasts a `cmn_progress_t` to a `cmn_widget_t`.
 * @param progress The progress bar instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_progress_as_widget(cmn_progress_t *progress,
                                                  cmn_widget_t **out_widget);

/**
 * @brief Destroys a progress bar and frees its memory.
 * @param progress Pointer to the progress bar pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_progress_destroy(cmn_progress_t **progress);

/* --- UpDown / Stepper --- */

/**
 * @brief Callback function signature for stepper increment/decrement events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_updown_step_fn)(void *userdata);

/**
 * @brief Creates a new stepper / up-down widget.
 * @param out_updown Pointer to store the created up-down widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_create(cmn_updown_t **out_updown);

/**
 * @brief Sets the current numeric value of an up-down stepper.
 * @param updown The up-down widget to modify.
 * @param value The numeric value to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_set_value(cmn_updown_t *updown,
                                                double value);

/**
 * @brief Gets the current numeric value of an up-down stepper.
 * @param updown The up-down widget to query.
 * @param out_value Pointer to store the current numeric value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_get_value(const cmn_updown_t *updown,
                                                double *out_value);

/**
 * @brief Sets the step delta of an up-down stepper.
 * @param updown The up-down widget to modify.
 * @param step Positive step increment (`> 0.0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_set_step(cmn_updown_t *updown,
                                               double step);

/**
 * @brief Gets the step delta of an up-down stepper.
 * @param updown The up-down widget to query.
 * @param out_step Pointer to store the step increment.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_get_step(const cmn_updown_t *updown,
                                               double *out_step);

/**
 * @brief Sets the step callback for an up-down stepper.
 * @param updown The up-down widget to modify.
 * @param cb Callback function invoked when stepped up or down.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_set_on_step(cmn_updown_t *updown,
                                                  cmn_updown_step_fn cb,
                                                  void *userdata);

/**
 * @brief Programmatically steps an up-down widget (`direction > 0` for up,
 * `< 0` for down) and invokes its callback.
 * @param updown The up-down widget to step.
 * @param direction `+1` to increment, `-1` to decrement.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_trigger_step(cmn_updown_t *updown,
                                                   int direction);

/**
 * @brief Upcasts a `cmn_updown_t` to a `cmn_widget_t`.
 * @param updown The up-down instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_as_widget(cmn_updown_t *updown,
                                                cmn_widget_t **out_widget);

/**
 * @brief Destroys an up-down widget and frees its memory.
 * @param updown Pointer to the up-down pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_updown_destroy(cmn_updown_t **updown);

/* --- PopUp --- */

/**
 * @brief Callback function signature for pop-up selection events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_popup_select_fn)(void *userdata);

/**
 * @brief Creates a new drop-down pop-up selector widget.
 * @param out_popup Pointer to store the created pop-up widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_create(cmn_popup_t **out_popup);

/**
 * @brief Appends an option item to a pop-up selector.
 * @param popup The pop-up widget to modify.
 * @param text The item text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_add_item(cmn_popup_t *popup,
                                              const char *text);

/**
 * @brief Replaces the text of an existing item in a pop-up selector.
 * @param popup The pop-up widget to modify.
 * @param index Zero-based item index.
 * @param text Replacement UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_set_item(cmn_popup_t *popup,
                                              uint32_t index, const char *text);

/**
 * @brief Removes all items from a pop-up selector.
 * @param popup The pop-up widget to clear.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_clear(cmn_popup_t *popup);

/**
 * @brief Gets the number of items in a pop-up selector.
 * @param popup The pop-up widget to query.
 * @param out_count Pointer to store the item count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_get_count(const cmn_popup_t *popup,
                                               uint32_t *out_count);

/**
 * @brief Sets the selected item index of a pop-up selector.
 * @param popup The pop-up widget to modify.
 * @param index Zero-based index of the item to select.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_set_selected(cmn_popup_t *popup,
                                                  uint32_t index);

/**
 * @brief Gets the selected item index of a pop-up selector.
 * @param popup The pop-up widget to query.
 * @param out_index Pointer to store the selected item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_get_selected(const cmn_popup_t *popup,
                                                  uint32_t *out_index);

/**
 * @brief Gets the text of an item in a pop-up selector.
 * @param popup The pop-up widget to query.
 * @param index Zero-based item index.
 * @param out_text Pointer to store the item text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_get_item_text(const cmn_popup_t *popup,
                                                   uint32_t index,
                                                   const char **out_text);

/**
 * @brief Sets the selection callback for a pop-up selector.
 * @param popup The pop-up widget to modify.
 * @param cb Callback function invoked on item selection.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_set_on_select(cmn_popup_t *popup,
                                                   cmn_popup_select_fn cb,
                                                   void *userdata);

/**
 * @brief Programmatically triggers the selection callback of a pop-up selector.
 * @param popup The pop-up widget to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_trigger_select(cmn_popup_t *popup);

/**
 * @brief Upcasts a `cmn_popup_t` to a `cmn_widget_t`.
 * @param popup The pop-up instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_as_widget(cmn_popup_t *popup,
                                               cmn_widget_t **out_widget);

/**
 * @brief Destroys a pop-up selector and frees its memory.
 * @param popup Pointer to the pop-up pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_popup_destroy(cmn_popup_t **popup);

/* --- Combo --- */

/**
 * @brief Callback function signature for editable combo box change events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_combo_change_fn)(void *userdata);

/**
 * @brief Creates a new editable combo box widget.
 * @param out_combo Pointer to store the created combo box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_create(cmn_combo_t **out_combo);

/**
 * @brief Sets the editable text of a combo box.
 * @param combo The combo box to modify.
 * @param text The new UTF-8 text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_set_text(cmn_combo_t *combo,
                                              const char *text);

/**
 * @brief Gets the current editable text of a combo box.
 * @param combo The combo box to query.
 * @param out_text Pointer to store the current UTF-8 text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_get_text(const cmn_combo_t *combo,
                                              const char **out_text);

/**
 * @brief Appends a suggestion item to a combo box.
 * @param combo The combo box to modify.
 * @param text The item text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_add_item(cmn_combo_t *combo,
                                              const char *text);

/**
 * @brief Removes all suggestion items from a combo box.
 * @param combo The combo box to clear.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_clear(cmn_combo_t *combo);

/**
 * @brief Gets the number of suggestion items in a combo box.
 * @param combo The combo box to query.
 * @param out_count Pointer to store the item count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_get_count(const cmn_combo_t *combo,
                                               uint32_t *out_count);

/**
 * @brief Selects a suggestion item by index and updates the combo box text.
 * @param combo The combo box to modify.
 * @param index Zero-based item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_set_selected(cmn_combo_t *combo,
                                                  uint32_t index);

/**
 * @brief Gets the currently selected item index of a combo box.
 * @param combo The combo box to query.
 * @param out_index Pointer to store the selected item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_get_selected(const cmn_combo_t *combo,
                                                  uint32_t *out_index);

/**
 * @brief Sets the change callback for a combo box.
 * @param combo The combo box to modify.
 * @param cb Callback function invoked when text or selection changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_set_on_change(cmn_combo_t *combo,
                                                   cmn_combo_change_fn cb,
                                                   void *userdata);

/**
 * @brief Programmatically triggers the change callback of a combo box.
 * @param combo The combo box to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_trigger_change(cmn_combo_t *combo);

/**
 * @brief Upcasts a `cmn_combo_t` to a `cmn_widget_t`.
 * @param combo The combo box instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_as_widget(cmn_combo_t *combo,
                                               cmn_widget_t **out_widget);

/**
 * @brief Destroys a combo box and frees its memory.
 * @param combo Pointer to the combo box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_combo_destroy(cmn_combo_t **combo);

/* --- ListBox --- */

/**
 * @brief Callback function signature for list box selection events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_listbox_select_fn)(void *userdata);

/**
 * @brief Creates a new scrollable list box widget.
 * @param out_listbox Pointer to store the created list box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_create(cmn_listbox_t **out_listbox);

/**
 * @brief Appends an item to a list box.
 * @param listbox The list box to modify.
 * @param text The item text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_add_item(cmn_listbox_t *listbox,
                                                const char *text);

/**
 * @brief Removes an item at `index` from a list box.
 * @param listbox The list box to modify.
 * @param index Zero-based index of the item to remove.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_remove_item(cmn_listbox_t *listbox,
                                                   uint32_t index);

/**
 * @brief Clears all items from a list box.
 * @param listbox The list box to clear.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_clear(cmn_listbox_t *listbox);

/**
 * @brief Gets the number of items in a list box.
 * @param listbox The list box to query.
 * @param out_count Pointer to store the item count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_get_count(const cmn_listbox_t *listbox,
                                                 uint32_t *out_count);

/**
 * @brief Gets the text of an item in a list box.
 * @param listbox The list box to query.
 * @param index Zero-based item index.
 * @param out_text Pointer to store the item text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_get_item_text(
    const cmn_listbox_t *listbox, uint32_t index, const char **out_text);

/**
 * @brief Selects an item in a list box.
 * @param listbox The list box to modify.
 * @param index Zero-based item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_set_selected(cmn_listbox_t *listbox,
                                                    uint32_t index);

/**
 * @brief Gets the selected item index of a list box.
 * @param listbox The list box to query.
 * @param out_index Pointer to store the selected item index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_listbox_get_selected(const cmn_listbox_t *listbox, uint32_t *out_index);

/**
 * @brief Configures whether a list box displays per-row checkboxes.
 * @param listbox The list box to modify.
 * @param checkable `CMN_TRUE` to show checkboxes, `CMN_FALSE` otherwise.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_set_checkable(cmn_listbox_t *listbox,
                                                     cmn_bool_t checkable);

/**
 * @brief Queries whether a list box displays per-row checkboxes.
 * @param listbox The list box to query.
 * @param out_checkable Pointer receiving `CMN_TRUE` if checkable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_get_checkable(
    const cmn_listbox_t *listbox, cmn_bool_t *out_checkable);

/**
 * @brief Sets the checked state of a specific item in a list box.
 * @param listbox The list box to modify.
 * @param index Zero-based item index.
 * @param checked `CMN_TRUE` if checked, `CMN_FALSE` otherwise.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_set_checked(cmn_listbox_t *listbox,
                                                   uint32_t index,
                                                   cmn_bool_t checked);

/**
 * @brief Gets the checked state of a specific item in a list box.
 * @param listbox The list box to query.
 * @param index Zero-based item index.
 * @param out_checked Pointer receiving the item's checked state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_get_checked(const cmn_listbox_t *listbox,
                                                   uint32_t index,
                                                   cmn_bool_t *out_checked);

/**
 * @brief Sets the selection callback for a list box.
 * @param listbox The list box to modify.
 * @param cb Callback function invoked when selection changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_set_on_select(cmn_listbox_t *listbox,
                                                     cmn_listbox_select_fn cb,
                                                     void *userdata);

/**
 * @brief Programmatically triggers the selection callback of a list box.
 * @param listbox The list box to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_trigger_select(cmn_listbox_t *listbox);

/**
 * @brief Upcasts a `cmn_listbox_t` to a `cmn_widget_t`.
 * @param listbox The list box instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_as_widget(cmn_listbox_t *listbox,
                                                 cmn_widget_t **out_widget);

/**
 * @brief Destroys a list box and frees its memory.
 * @param listbox Pointer to the list box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_listbox_destroy(cmn_listbox_t **listbox);

/* --- ImageView --- */

/**
 * @brief Callback function signature for image view click events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_imageview_click_fn)(void *userdata);

/**
 * @brief Creates a new image view widget.
 * @param out_imageview Pointer to store the created image view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_imageview_create(cmn_imageview_t **out_imageview);

/**
 * @brief Sets the image path or URI displayed by an image view.
 * @param imageview The image view to modify.
 * @param path File path or URI string of the image.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_imageview_set_image_path(cmn_imageview_t *imageview, const char *path);

/**
 * @brief Gets the image path or URI displayed by an image view.
 * @param imageview The image view to query.
 * @param out_path Pointer to store the image path string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_imageview_get_image_path(
    const cmn_imageview_t *imageview, const char **out_path);

/**
 * @brief Sets the scaling mode of an image view.
 * @param imageview The image view to modify.
 * @param scale Scaling mode (`CMN_SCALE_AUTO`, `CMN_SCALE_NONE`,
 * `CMN_SCALE_ASPECT`, `CMN_SCALE_FIT`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_imageview_set_scale(cmn_imageview_t *imageview,
                                                   cmn_scale_mode_e scale);

/**
 * @brief Gets the scaling mode of an image view.
 * @param imageview The image view to query.
 * @param out_scale Pointer to store the scaling mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_imageview_get_scale(
    const cmn_imageview_t *imageview, cmn_scale_mode_e *out_scale);

/**
 * @brief Sets the click callback for an image view.
 * @param imageview The image view to modify.
 * @param cb Callback function invoked when clicked.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_imageview_set_on_click(
    cmn_imageview_t *imageview, cmn_imageview_click_fn cb, void *userdata);

/**
 * @brief Programmatically triggers the click callback of an image view.
 * @param imageview The image view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_imageview_trigger_click(cmn_imageview_t *imageview);

/**
 * @brief Upcasts a `cmn_imageview_t` to a `cmn_widget_t`.
 * @param imageview The image view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_imageview_as_widget(cmn_imageview_t *imageview,
                                                   cmn_widget_t **out_widget);

/**
 * @brief Destroys an image view and frees its memory.
 * @param imageview Pointer to the image view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_imageview_destroy(cmn_imageview_t **imageview);

/* --- TableView --- */

/**
 * @brief Callback function signature for table view row selection events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_tableview_select_fn)(void *userdata);

/**
 * @brief Creates a new multi-column table view widget.
 * @param out_tableview Pointer to store the created table view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_tableview_create(cmn_tableview_t **out_tableview);

/**
 * @brief Appends a text column to a table view.
 * @param tableview The table view to modify.
 * @param title Header title for the column.
 * @param width Initial column width in pixels (`> 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_add_column(cmn_tableview_t *tableview,
                                                    const char *title,
                                                    int width);

/**
 * @brief Gets the number of columns in a table view.
 * @param tableview The table view to query.
 * @param out_cols Pointer to store the column count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_get_column_count(
    const cmn_tableview_t *tableview, uint32_t *out_cols);

/**
 * @brief Gets the header title of a column in a table view.
 * @param tableview The table view to query.
 * @param col Zero-based column index.
 * @param out_title Pointer to store the column title string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_get_column_title(
    const cmn_tableview_t *tableview, uint32_t col, const char **out_title);

/**
 * @brief Sets the total row count of a table view.
 * @param tableview The table view to modify.
 * @param rows Number of rows.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_tableview_set_row_count(cmn_tableview_t *tableview, uint32_t rows);

/**
 * @brief Gets the total row count of a table view.
 * @param tableview The table view to query.
 * @param out_rows Pointer to store the row count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_get_row_count(
    const cmn_tableview_t *tableview, uint32_t *out_rows);

/**
 * @brief Sets the text of a cell at `(row, col)` in a table view.
 * @param tableview The table view to modify.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @param text Cell text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_set_cell_text(
    cmn_tableview_t *tableview, uint32_t row, uint32_t col, const char *text);

/**
 * @brief Gets the text of a cell at `(row, col)` in a table view.
 * @param tableview The table view to query.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @param out_text Pointer to store the cell text string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_tableview_get_cell_text(const cmn_tableview_t *tableview, uint32_t row,
                            uint32_t col, const char **out_text);

/**
 * @brief Selects a row in a table view.
 * @param tableview The table view to modify.
 * @param row Zero-based row index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_tableview_set_selected_row(cmn_tableview_t *tableview, uint32_t row);

/**
 * @brief Gets the currently selected row in a table view.
 * @param tableview The table view to query.
 * @param out_row Pointer to store the selected row index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_get_selected_row(
    const cmn_tableview_t *tableview, uint32_t *out_row);

/**
 * @brief Sets the row selection callback for a table view.
 * @param tableview The table view to modify.
 * @param cb Callback function invoked when row selection changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_set_on_select(
    cmn_tableview_t *tableview, cmn_tableview_select_fn cb, void *userdata);

/**
 * @brief Programmatically triggers the row selection callback of a table view.
 * @param tableview The table view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_tableview_trigger_select(cmn_tableview_t *tableview);

/**
 * @brief Upcasts a `cmn_tableview_t` to a `cmn_widget_t`.
 * @param tableview The table view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_as_widget(cmn_tableview_t *tableview,
                                                   cmn_widget_t **out_widget);

/**
 * @brief Destroys a table view and frees its memory.
 * @param tableview Pointer to the table view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_destroy(cmn_tableview_t **tableview);

/* --- Custom View --- */

/**
 * @brief Callback function signature for custom view draw/click events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_view_event_fn)(void *userdata);

/**
 * @brief Creates a new custom 2D drawing view widget.
 * @param out_view Pointer to store the created custom view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_create(cmn_view_t **out_view);

/**
 * @brief Creates a new scrollable custom 2D drawing view widget.
 * @param out_view Pointer to store the created scrollable custom view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_create_scroll(cmn_view_t **out_view);

/**
 * @brief Queries whether a custom view is scrollable.
 * @param view The custom view to query.
 * @param out_scrollable Pointer receiving `CMN_TRUE` if scrollable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_is_scrollable(const cmn_view_t *view,
                                                  cmn_bool_t *out_scrollable);

/**
 * @brief Sets the virtual content size of a custom view.
 * @param view The custom view to modify.
 * @param width Content width in pixels (`> 0`).
 * @param height Content height in pixels (`> 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_set_content_size(cmn_view_t *view,
                                                     int width, int height);

/**
 * @brief Gets the virtual content size of a custom view.
 * @param view The custom view to query.
 * @param out_width Pointer to store the content width.
 * @param out_height Pointer to store the content height.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_get_content_size(const cmn_view_t *view,
                                                     int *out_width,
                                                     int *out_height);

/**
 * @brief Sets the draw callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback function invoked on redraw.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_set_on_draw(cmn_view_t *view,
                                                cmn_view_event_fn cb,
                                                void *userdata);

/**
 * @brief Invalidates a custom view and triggers its draw callback.
 * @param view The custom view to invalidate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_invalidate(cmn_view_t *view);

/**
 * @brief Sets the click callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback function invoked when clicked.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_set_on_click(cmn_view_t *view,
                                                 cmn_view_event_fn cb,
                                                 void *userdata);

/**
 * @brief Programmatically triggers the click callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_trigger_click(cmn_view_t *view);

/**
 * @brief Upcasts a `cmn_view_t` to a `cmn_widget_t`.
 * @param view The custom view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_as_widget(cmn_view_t *view,
                                              cmn_widget_t **out_widget);

/**
 * @brief Destroys a custom view and frees its memory.
 * @param view Pointer to the custom view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_destroy(cmn_view_t **view);

/* --- WebView --- */

/**
 * @brief Creates a new embedded web browser view widget.
 * @param out_webview Pointer to store the created web view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_create(cmn_webview_t **out_webview);

/**
 * @brief Navigates a web view to the given URL.
 * @param webview The web view to navigate.
 * @param url Target URL string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_navigate(cmn_webview_t *webview,
                                                const char *url);

/**
 * @brief Gets the current URL of a web view.
 * @param webview The web view to query.
 * @param out_url Pointer to store the URL string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_get_url(const cmn_webview_t *webview,
                                               const char **out_url);

/**
 * @brief Loads raw HTML source code into a web view.
 * @param webview The web view to modify.
 * @param html UTF-8 HTML document string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_load_html(cmn_webview_t *webview,
                                                 const char *html);

/**
 * @brief Gets the last loaded HTML source code of a web view.
 * @param webview The web view to query.
 * @param out_html Pointer to store the HTML string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_get_html(const cmn_webview_t *webview,
                                                const char **out_html);

/**
 * @brief Navigates back in the web view history.
 * @param webview The web view to navigate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_go_back(cmn_webview_t *webview);

/**
 * @brief Navigates forward in the web view history.
 * @param webview The web view to navigate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_go_forward(cmn_webview_t *webview);

/**
 * @brief Upcasts a `cmn_webview_t` to a `cmn_widget_t`.
 * @param webview The web view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_as_widget(cmn_webview_t *webview,
                                                 cmn_widget_t **out_widget);

/**
 * @brief Destroys a web view and frees its memory.
 * @param webview Pointer to the web view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_webview_destroy(cmn_webview_t **webview);

/* --- Panel Container --- */

/**
 * @brief Creates a new grouping panel container.
 * @param out_panel Pointer to store the created panel.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_panel_create(cmn_panel_t **out_panel);

/**
 * @brief Creates a new scrollable panel container.
 * @param out_panel Pointer to store the created scrollable panel.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_panel_create_scroll(cmn_panel_t **out_panel);

/**
 * @brief Queries whether a panel container has scrollbars enabled.
 * @param panel The panel to query.
 * @param out_scrollable Pointer receiving `CMN_TRUE` if scrollable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_panel_is_scrollable(const cmn_panel_t *panel,
                                                   cmn_bool_t *out_scrollable);

/**
 * @brief Upcasts a `cmn_panel_t` to a `cmn_container_t`.
 * @param panel The panel instance.
 * @param out_container Output pointer receiving the container handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_panel_as_container(cmn_panel_t *panel, cmn_container_t **out_container);

/**
 * @brief Upcasts a `cmn_panel_t` to a `cmn_widget_t`.
 * @param panel The panel instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_panel_as_widget(cmn_panel_t *panel,
                                               cmn_widget_t **out_widget);

/**
 * @brief Destroys a panel container and frees its memory.
 * @param panel Pointer to the panel pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_panel_destroy(cmn_panel_t **panel);

/* --- Tabs Container --- */

/**
 * @brief Callback function signature for tab selection events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_tabs_select_fn)(void *userdata);

/**
 * @brief Creates a new tabbed container widget.
 * @param out_tabs Pointer to store the created tabs widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_create(cmn_tabs_t **out_tabs);

/**
 * @brief Appends a tab page with a title and child content widget.
 * @param tabs The tabs container to modify.
 * @param title Title label of the tab page.
 * @param content Child widget or container for the tab page (may be NULL).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_add_tab(cmn_tabs_t *tabs, const char *title,
                                            void *content);

/**
 * @brief Gets the number of tabs in a tabbed container.
 * @param tabs The tabs container to query.
 * @param out_count Pointer to store the tab count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_get_count(const cmn_tabs_t *tabs,
                                              uint32_t *out_count);

/**
 * @brief Gets the title of a tab page by index.
 * @param tabs The tabs container to query.
 * @param index Zero-based tab index.
 * @param out_title Pointer to store the tab title string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_get_tab_title(const cmn_tabs_t *tabs,
                                                  uint32_t index,
                                                  const char **out_title);

/**
 * @brief Gets the content widget associated with a tab page.
 * @param tabs The tabs container to query.
 * @param index Zero-based tab index.
 * @param out_content Pointer to store the content widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_get_tab_content(const cmn_tabs_t *tabs,
                                                    uint32_t index,
                                                    void **out_content);

/**
 * @brief Selects the active tab page by index.
 * @param tabs The tabs container to modify.
 * @param index Zero-based tab index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_set_selected(cmn_tabs_t *tabs,
                                                 uint32_t index);

/**
 * @brief Gets the active tab page index.
 * @param tabs The tabs container to query.
 * @param out_index Pointer to store the active tab index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_get_selected(const cmn_tabs_t *tabs,
                                                 uint32_t *out_index);

/**
 * @brief Sets the tab selection callback.
 * @param tabs The tabs container to modify.
 * @param cb Callback function invoked when active tab changes.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_set_on_select(cmn_tabs_t *tabs,
                                                  cmn_tabs_select_fn cb,
                                                  void *userdata);

/**
 * @brief Programmatically triggers the tab selection callback.
 * @param tabs The tabs container to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_trigger_select(cmn_tabs_t *tabs);

/**
 * @brief Upcasts a `cmn_tabs_t` to a `cmn_widget_t`.
 * @param tabs The tabs container instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_as_widget(cmn_tabs_t *tabs,
                                              cmn_widget_t **out_widget);

/**
 * @brief Destroys a tabs container and frees its memory.
 * @param tabs Pointer to the tabs pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tabs_destroy(cmn_tabs_t **tabs);

/* --- SplitView Container --- */

/**
 * @brief Creates a horizontal split view with a horizontal divider.
 * @param out_splitview Pointer to store the created split view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_splitview_create_horizontal(cmn_splitview_t **out_splitview);

/**
 * @brief Creates a vertical split view with a vertical divider.
 * @param out_splitview Pointer to store the created split view.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_splitview_create_vertical(cmn_splitview_t **out_splitview);

/**
 * @brief Queries whether a split view uses a vertical divider.
 * @param splitview The split view to query.
 * @param out_vertical Pointer receiving `CMN_TRUE` if vertical.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_splitview_is_vertical(
    const cmn_splitview_t *splitview, cmn_bool_t *out_vertical);

/**
 * @brief Attaches the two child panes of a split view.
 * @param splitview The split view to modify.
 * @param child0 First pane child widget.
 * @param child1 Second pane child widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_splitview_set_children(
    cmn_splitview_t *splitview, void *child0, void *child1);

/**
 * @brief Gets the two child panes of a split view.
 * @param splitview The split view to query.
 * @param out_child0 Pointer to store the first pane widget.
 * @param out_child1 Pointer to store the second pane widget.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_splitview_get_children(
    const cmn_splitview_t *splitview, void **out_child0, void **out_child1);

/**
 * @brief Sets the normalized divider position (`[0.0, 1.0]`) of a split view.
 * @param splitview The split view to modify.
 * @param pos Normalized divider ratio in `[0.0, 1.0]`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_splitview_set_pos(cmn_splitview_t *splitview,
                                                 double pos);

/**
 * @brief Gets the normalized divider position (`[0.0, 1.0]`) of a split view.
 * @param splitview The split view to query.
 * @param out_pos Pointer to store the divider ratio.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_splitview_get_pos(const cmn_splitview_t *splitview, double *out_pos);

/**
 * @brief Sets the visibility of pane `0` or pane `1`.
 * @param splitview The split view to modify.
 * @param pane_index Pane index (`0` or `1`).
 * @param visible `CMN_TRUE` if visible, `CMN_FALSE` if collapsed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_splitview_set_pane_visible(
    cmn_splitview_t *splitview, int pane_index, cmn_bool_t visible);

/**
 * @brief Gets the visibility of pane `0` or pane `1`.
 * @param splitview The split view to query.
 * @param pane_index Pane index (`0` or `1`).
 * @param out_visible Pointer receiving `CMN_TRUE` if visible.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_splitview_get_pane_visible(
    const cmn_splitview_t *splitview, int pane_index, cmn_bool_t *out_visible);

/**
 * @brief Upcasts a `cmn_splitview_t` to a `cmn_widget_t`.
 * @param splitview The split view instance.
 * @param out_widget Output pointer receiving the widget handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_splitview_as_widget(cmn_splitview_t *splitview,
                                                   cmn_widget_t **out_widget);

/**
 * @brief Destroys a split view and frees its memory.
 * @param splitview Pointer to the split view pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_splitview_destroy(cmn_splitview_t **splitview);

/* --- ColorPicker Dialog --- */

/**
 * @brief Callback function signature for color picker selection events.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_colorpicker_change_fn)(void *userdata);

/**
 * @brief Creates a new native color picker dialog.
 * @param out_colorpicker Pointer to store the created color picker.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_colorpicker_create(cmn_colorpicker_t **out_colorpicker);

/**
 * @brief Sets the packed RGBA (`0xRRGGBBAA`) color of a color picker.
 * @param colorpicker The color picker to modify.
 * @param rgba Packed 32-bit RGBA color value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_colorpicker_set_color(cmn_colorpicker_t *colorpicker, uint32_t rgba);

/**
 * @brief Gets the packed RGBA (`0xRRGGBBAA`) color of a color picker.
 * @param colorpicker The color picker to query.
 * @param out_rgba Pointer to store the packed 32-bit RGBA color value.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_colorpicker_get_color(
    const cmn_colorpicker_t *colorpicker, uint32_t *out_rgba);

/**
 * @brief Shows the native color picker dialog.
 * @param colorpicker The color picker dialog to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_colorpicker_show_dialog(cmn_colorpicker_t *colorpicker);

/**
 * @brief Sets the color change callback for a color picker.
 * @param colorpicker The color picker to modify.
 * @param cb Callback function invoked when a color is chosen.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_colorpicker_set_on_change(cmn_colorpicker_t *colorpicker,
                              cmn_colorpicker_change_fn cb, void *userdata);

/**
 * @brief Programmatically triggers the color change callback.
 * @param colorpicker The color picker to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_colorpicker_trigger_change(cmn_colorpicker_t *colorpicker);

/**
 * @brief Destroys a color picker and frees its memory.
 * @param colorpicker Pointer to the color picker pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_colorpicker_destroy(cmn_colorpicker_t **colorpicker);

/* --- Extended Window Controls --- */

/**
 * @brief Callback function signature for window lifecycle events (`OnClose`,
 * `OnMoved`, `OnActivate`).
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_window_event_fn)(void *userdata);

/**
 * @brief Sets the default push button activated on Enter key in a window.
 * @param window The window to modify.
 * @param button The default push button (may be NULL to clear).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_default_button(cmn_window_t *window,
                                                         cmn_button_t *button);

/**
 * @brief Gets the default push button configured for a window.
 * @param window The window to query.
 * @param out_button Pointer to store the default push button handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_get_default_button(
    const cmn_window_t *window, cmn_button_t **out_button);

/**
 * @brief Sets the active mouse cursor shape for a window.
 * @param window The window to modify.
 * @param cursor Cursor shape (`CMN_CURSOR_ARROW`, `CMN_CURSOR_HAND`, etc.).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_cursor(cmn_window_t *window,
                                                 cmn_cursor_e cursor);

/**
 * @brief Gets the active mouse cursor shape for a window.
 * @param window The window to query.
 * @param out_cursor Pointer to store the cursor shape.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_get_cursor(const cmn_window_t *window,
                                                 cmn_cursor_e *out_cursor);

/**
 * @brief Converts window client-area coordinates to screen coordinates.
 * @param window The reference window.
 * @param client_x X coordinate in window client space.
 * @param client_y Y coordinate in window client space.
 * @param out_screen_x Pointer receiving screen X coordinate.
 * @param out_screen_y Pointer receiving screen Y coordinate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_window_client_to_screen(const cmn_window_t *window, int client_x,
                            int client_y, int *out_screen_x, int *out_screen_y);

/**
 * @brief Registers the close lifecycle callback for a window.
 * @param window The window to modify.
 * @param cb Callback invoked when the window is closing.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_on_close(cmn_window_t *window,
                                                   cmn_window_event_fn cb,
                                                   void *userdata);

/**
 * @brief Programmatically triggers the close callback of a window.
 * @param window The window to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_trigger_close(cmn_window_t *window);

/**
 * @brief Registers the moved lifecycle callback for a window.
 * @param window The window to modify.
 * @param cb Callback invoked when the window moves.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_on_moved(cmn_window_t *window,
                                                   cmn_window_event_fn cb,
                                                   void *userdata);

/**
 * @brief Programmatically triggers the moved callback of a window.
 * @param window The window to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_trigger_moved(cmn_window_t *window);

/**
 * @brief Registers the activation lifecycle callback for a window.
 * @param window The window to modify.
 * @param cb Callback invoked when the window becomes active.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_set_on_activate(cmn_window_t *window,
                                                      cmn_window_event_fn cb,
                                                      void *userdata);

/**
 * @brief Programmatically triggers the activate callback of a window.
 * @param window The window to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_window_trigger_activate(cmn_window_t *window);

/* --- Extended Menu & MenuItem Controls --- */

/**
 * @brief Appends a horizontal separator item to a menu.
 * @param menu The menu to modify.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_add_separator(cmn_menu_t *menu);

/**
 * @brief Attaches a hierarchical submenu item to a menu.
 * @param menu The parent menu to modify.
 * @param title Label title for the submenu item.
 * @param submenu The child submenu to attach.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_add_submenu(cmn_menu_t *menu,
                                                const char *title,
                                                cmn_menu_t *submenu);

/**
 * @brief Sets the enabled state of a menu item by index.
 * @param menu The menu to modify.
 * @param index Zero-based menu item index (`>= 0`).
 * @param enabled `CMN_TRUE` to enable, `CMN_FALSE` to disable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_set_item_enabled(cmn_menu_t *menu,
                                                     int index,
                                                     cmn_bool_t enabled);

/**
 * @brief Gets the enabled state of a menu item by index.
 * @param menu The menu to query.
 * @param index Zero-based menu item index (`>= 0`).
 * @param out_enabled Pointer receiving `CMN_TRUE` if enabled.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_get_item_enabled(const cmn_menu_t *menu,
                                                     int index,
                                                     cmn_bool_t *out_enabled);

/**
 * @brief Sets the check state of a menu item by index.
 * @param menu The menu to modify.
 * @param index Zero-based menu item index (`>= 0`).
 * @param state Check state (`CMN_CHECK_OFF`, `CMN_CHECK_ON`,
 * `CMN_CHECK_MIXED`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_set_item_state(cmn_menu_t *menu, int index,
                                                   cmn_check_state_e state);

/**
 * @brief Gets the check state of a menu item by index.
 * @param menu The menu to query.
 * @param index Zero-based menu item index (`>= 0`).
 * @param out_state Pointer receiving the item's check state.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_get_item_state(
    const cmn_menu_t *menu, int index, cmn_check_state_e *out_state);

/**
 * @brief Displays a menu as a context popup at `(x, y)` in a window.
 * @param menu The popup menu to display.
 * @param window The parent window.
 * @param x Popup X coordinate.
 * @param y Popup Y coordinate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_menu_popup(cmn_menu_t *menu,
                                          cmn_window_t *window, int x, int y);

/* --- Extended FilePicker Controls --- */

/**
 * @brief Sets the dialog mode of a file picker.
 * @param filepicker The file picker to modify.
 * @param mode Dialog mode (`CMN_FILEPICKER_OPEN_FILE`,
 * `CMN_FILEPICKER_SAVE_FILE`, `CMN_FILEPICKER_SELECT_DIR`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_filepicker_set_mode(cmn_filepicker_t *filepicker,
                                                   cmn_filepicker_mode_e mode);

/**
 * @brief Gets the dialog mode of a file picker.
 * @param filepicker The file picker to query.
 * @param out_mode Pointer receiving the dialog mode.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_filepicker_get_mode(
    const cmn_filepicker_t *filepicker, cmn_filepicker_mode_e *out_mode);

/**
 * @brief Sets the allowed file extension filters of a file picker.
 * @param filepicker The file picker to modify.
 * @param extensions Array of file extension strings (e.g., `{"png", "jpg"}`).
 * @param count Number of extension strings in `extensions`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_filepicker_set_filetypes(
    cmn_filepicker_t *filepicker, const char *const *extensions, size_t count);

/**
 * @brief Gets the number of configured file extension filters on a file picker.
 * @param filepicker The file picker to query.
 * @param out_count Pointer receiving the filter count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_filepicker_get_filetype_count(
    const cmn_filepicker_t *filepicker, size_t *out_count);

/* --- Extended Push Button Controls --- */

/**
 * @brief Sets the hover tooltip text of a push button.
 * @param button The button to modify.
 * @param tooltip UTF-8 tooltip text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_set_tooltip(cmn_button_t *button,
                                                  const char *tooltip);

/**
 * @brief Gets the hover tooltip text of a push button.
 * @param button The button to query.
 * @param out_tooltip Pointer receiving the tooltip string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_get_tooltip(const cmn_button_t *button,
                                                  const char **out_tooltip);

/**
 * @brief Sets the icon image path of a push button.
 * @param button The button to modify.
 * @param image_path File path or resource URI of the icon image.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_set_image(cmn_button_t *button,
                                                const char *image_path);

/**
 * @brief Gets the icon image path of a push button.
 * @param button The button to query.
 * @param out_image_path Pointer receiving the image path string pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_button_get_image(const cmn_button_t *button,
                                                const char **out_image_path);

/* --- Extended Label Controls --- */

/**
 * @brief Configures whether a label wraps text across multiple lines.
 * @param label The label to modify.
 * @param multiline `CMN_TRUE` for multi-line wrapping, `CMN_FALSE` for single
 * line.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_set_multiline(cmn_label_t *label,
                                                   cmn_bool_t multiline);

/**
 * @brief Queries whether a label wraps text across multiple lines.
 * @param label The label to query.
 * @param out_multiline Pointer receiving `CMN_TRUE` if multi-line.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_get_multiline(const cmn_label_t *label,
                                                   cmn_bool_t *out_multiline);

/**
 * @brief Sets the horizontal text alignment of a label.
 * @param label The label to modify.
 * @param align Text alignment (`CMN_TEXT_ALIGN_LEFT`, `CMN_TEXT_ALIGN_CENTER`,
 * `CMN_TEXT_ALIGN_RIGHT`, `CMN_TEXT_ALIGN_JUSTIFY`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_set_align(cmn_label_t *label,
                                               cmn_text_align_e align);

/**
 * @brief Gets the horizontal text alignment of a label.
 * @param label The label to query.
 * @param out_align Pointer receiving the text alignment.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_get_align(const cmn_label_t *label,
                                               cmn_text_align_e *out_align);

/**
 * @brief Sets the foreground text RGBA color of a label.
 * @param label The label to modify.
 * @param rgba Packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_set_color(cmn_label_t *label,
                                               uint32_t rgba);

/**
 * @brief Gets the foreground text RGBA color of a label.
 * @param label The label to query.
 * @param out_rgba Pointer receiving the packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_get_color(const cmn_label_t *label,
                                               uint32_t *out_rgba);

/**
 * @brief Sets the background RGBA color of a label.
 * @param label The label to modify.
 * @param rgba Packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_set_bgcolor(cmn_label_t *label,
                                                 uint32_t rgba);

/**
 * @brief Gets the background RGBA color of a label.
 * @param label The label to query.
 * @param out_rgba Pointer receiving the packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_label_get_bgcolor(const cmn_label_t *label,
                                                 uint32_t *out_rgba);

/* --- Extended TextBox Controls --- */

/**
 * @brief Sets whether a text box is user-editable.
 * @param textbox The text box to modify.
 * @param editable `CMN_TRUE` if editable, `CMN_FALSE` if read-only.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_set_editable(cmn_textbox_t *textbox,
                                                    cmn_bool_t editable);

/**
 * @brief Gets whether a text box is user-editable.
 * @param textbox The text box to query.
 * @param out_editable Pointer receiving `CMN_TRUE` if editable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_get_editable(
    const cmn_textbox_t *textbox, cmn_bool_t *out_editable);

/**
 * @brief Sets whether a text box automatically selects all text on focus.
 * @param textbox The text box to modify.
 * @param autoselect `CMN_TRUE` to auto-select on focus.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_set_autoselect(cmn_textbox_t *textbox,
                                                      cmn_bool_t autoselect);

/**
 * @brief Gets whether a text box automatically selects all text on focus.
 * @param textbox The text box to query.
 * @param out_autoselect Pointer receiving `CMN_TRUE` if auto-select is active.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_get_autoselect(
    const cmn_textbox_t *textbox, cmn_bool_t *out_autoselect);

/**
 * @brief Selects a character range `[start, end]` in a text box.
 * @param textbox The text box to modify.
 * @param start Selection start character offset (`>= 0`).
 * @param end Selection end character offset (`>= start` or `-1` for end).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_select_range(cmn_textbox_t *textbox,
                                                    int start, int end);

/**
 * @brief Gets the selected character range `[start, end]` of a text box.
 * @param textbox The text box to query.
 * @param out_start Pointer receiving selection start offset.
 * @param out_end Pointer receiving selection end offset.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_get_selection_range(
    const cmn_textbox_t *textbox, int *out_start, int *out_end);

/**
 * @brief Performs a clipboard operation (`copy`, `cut`, `paste`) on a text box.
 * @param textbox The text box to operate on.
 * @param op Clipboard operation (`CMN_CLIPBOARD_COPY`, `CMN_CLIPBOARD_CUT`,
 * `CMN_CLIPBOARD_PASTE`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textbox_clipboard_ops(cmn_textbox_t *textbox,
                                                     cmn_clipboard_op_e op);

/* --- Extended TextView Rich Formatting Controls --- */

/**
 * @brief Sets the font family, point size, and style mask for a text view.
 * @param textview The text view to modify.
 * @param family Font family name (e.g., `"Menlo"`).
 * @param size Font size in points (`> 0.0`).
 * @param style_mask Bitmask of `cmn_font_style_e` flags.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_set_font(cmn_textview_t *textview,
                                                 const char *family,
                                                 double size,
                                                 uint32_t style_mask);

/**
 * @brief Gets the font family, point size, and style mask of a text view.
 * @param textview The text view to query.
 * @param out_family Pointer receiving the font family string pointer.
 * @param out_size Pointer receiving the font size in points.
 * @param out_style_mask Pointer receiving the style bitmask.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_get_font(const cmn_textview_t *textview,
                                                 const char **out_family,
                                                 double *out_size,
                                                 uint32_t *out_style_mask);

/**
 * @brief Sets the foreground and background RGBA colors of a text view.
 * @param textview The text view to modify.
 * @param text_rgba Foreground text RGBA color.
 * @param bg_rgba Background RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_set_colors(cmn_textview_t *textview,
                                                   uint32_t text_rgba,
                                                   uint32_t bg_rgba);

/**
 * @brief Gets the foreground and background RGBA colors of a text view.
 * @param textview The text view to query.
 * @param out_text_rgba Pointer receiving the foreground RGBA color.
 * @param out_bg_rgba Pointer receiving the background RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_textview_get_colors(const cmn_textview_t *textview, uint32_t *out_text_rgba,
                        uint32_t *out_bg_rgba);

/**
 * @brief Configures line wrapping on a text view.
 * @param textview The text view to modify.
 * @param wrap `CMN_TRUE` to wrap lines, `CMN_FALSE` for horizontal scrolling.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_set_wrap(cmn_textview_t *textview,
                                                 cmn_bool_t wrap);

/**
 * @brief Queries whether line wrapping is enabled on a text view.
 * @param textview The text view to query.
 * @param out_wrap Pointer receiving `CMN_TRUE` if line wrapping is enabled.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_textview_get_wrap(const cmn_textview_t *textview,
                                                 cmn_bool_t *out_wrap);

/* --- Extended TableView Controls --- */

/**
 * @brief Virtualized data provider callback for `cmn_tableview_t`.
 * @param row Zero-based row index requested.
 * @param col Zero-based column index requested.
 * @param out_buf Destination buffer for the cell text.
 * @param buf_size Capacity of `out_buf` in bytes.
 * @param userdata User data pointer passed when registering the callback.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_tableview_data_fn)(uint32_t row, uint32_t col,
                                             char *out_buf, size_t buf_size,
                                             void *userdata);

/**
 * @brief Registers a virtualized cell data callback on a table view.
 * @param tableview The table view to modify.
 * @param cb Data provider callback invoked when querying virtual cells.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_set_on_data(cmn_tableview_t *tableview,
                                                     cmn_tableview_data_fn cb,
                                                     void *userdata);

/**
 * @brief Sets minimum and maximum width limits for a table column.
 * @param tableview The table view to modify.
 * @param col Zero-based column index.
 * @param min_width Minimum column width in pixels (`>= 0`).
 * @param max_width Maximum column width in pixels (`>= min_width`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_set_column_limits(
    cmn_tableview_t *tableview, uint32_t col, int min_width, int max_width);

/**
 * @brief Gets the minimum and maximum width limits for a table column.
 * @param tableview The table view to query.
 * @param col Zero-based column index.
 * @param out_min_width Pointer receiving minimum column width.
 * @param out_max_width Pointer receiving maximum column width.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_tableview_get_column_limits(const cmn_tableview_t *tableview, uint32_t col,
                                int *out_min_width, int *out_max_width);

/**
 * @brief Sets the number of frozen leading columns in a table view.
 * @param tableview The table view to modify.
 * @param freeze_cols Number of leading columns to freeze (`<= col_count`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_set_column_freeze(
    cmn_tableview_t *tableview, uint32_t freeze_cols);

/**
 * @brief Gets the number of frozen leading columns in a table view.
 * @param tableview The table view to query.
 * @param out_freeze_cols Pointer receiving the frozen column count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_tableview_get_column_freeze(
    const cmn_tableview_t *tableview, uint32_t *out_freeze_cols);

/* --- Extended Custom View 2D Drawing & Input Events --- */

/**
 * @brief Records a 2D filled/stroked rectangle command on a custom view.
 * @param view The custom view to draw into.
 * @param rect Rectangle geometry (`width >= 0`, `height >= 0`).
 * @param rgba Packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_draw_rect(cmn_view_t *view, cmn_rect_t rect,
                                              uint32_t rgba);

/**
 * @brief Records a 2D line segment command on a custom view.
 * @param view The custom view to draw into.
 * @param x0 Start X coordinate.
 * @param y0 Start Y coordinate.
 * @param x1 End X coordinate.
 * @param y1 End Y coordinate.
 * @param rgba Packed 32-bit RGBA color.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_draw_line(cmn_view_t *view, int x0, int y0,
                                              int x1, int y1, uint32_t rgba);

/**
 * @brief Records a 2D text draw command on a custom view.
 * @param view The custom view to draw into.
 * @param text UTF-8 text string to render.
 * @param x Anchor X coordinate.
 * @param y Anchor Y coordinate.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_draw_text(cmn_view_t *view,
                                              const char *text, int x, int y);

/**
 * @brief Gets the total number of 2D draw commands recorded on a custom view.
 * @param view The custom view to query.
 * @param out_count Pointer receiving the draw command count.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_view_get_draw_command_count(const cmn_view_t *view, uint32_t *out_count);

/**
 * @brief Sets the mouse wheel callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback invoked on wheel scroll.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_set_on_wheel(cmn_view_t *view,
                                                 cmn_view_event_fn cb,
                                                 void *userdata);

/**
 * @brief Programmatically triggers the wheel callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_trigger_wheel(cmn_view_t *view);

/**
 * @brief Sets the mouse drag callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback invoked on pointer drag.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_set_on_drag(cmn_view_t *view,
                                                cmn_view_event_fn cb,
                                                void *userdata);

/**
 * @brief Programmatically triggers the drag callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_trigger_drag(cmn_view_t *view);

/**
 * @brief Sets the key-down callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback invoked on key press.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_set_on_key_down(cmn_view_t *view,
                                                    cmn_view_event_fn cb,
                                                    void *userdata);

/**
 * @brief Programmatically triggers the key-down callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_trigger_key_down(cmn_view_t *view);

/**
 * @brief Sets the key-up callback for a custom view.
 * @param view The custom view to modify.
 * @param cb Callback invoked on key release.
 * @param userdata User data passed to `cb`.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_set_on_key_up(cmn_view_t *view,
                                                  cmn_view_event_fn cb,
                                                  void *userdata);

/**
 * @brief Programmatically triggers the key-up callback of a custom view.
 * @param view The custom view to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_view_trigger_key_up(cmn_view_t *view);

/* --- Cell-Level Widget Controls --- */

/**
 * @brief Sets whether any widget is interactive/enabled.
 * @param widget The widget to modify.
 * @param enabled `CMN_TRUE` to enable, `CMN_FALSE` to disable.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_set_enabled(void *widget,
                                                  cmn_bool_t enabled);

/**
 * @brief Queries whether any widget is interactive/enabled.
 * @param widget The widget to query.
 * @param out_enabled Pointer receiving `CMN_TRUE` if enabled.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_get_enabled(const void *widget,
                                                  cmn_bool_t *out_enabled);

/**
 * @brief Sets whether any widget is visible in its layout cell.
 * @param widget The widget to modify.
 * @param visible `CMN_TRUE` to show, `CMN_FALSE` to hide.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_set_visible(void *widget,
                                                  cmn_bool_t visible);

/**
 * @brief Queries whether any widget is visible in its layout cell.
 * @param widget The widget to query.
 * @param out_visible Pointer receiving `CMN_TRUE` if visible.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_get_visible(const void *widget,
                                                  cmn_bool_t *out_visible);

/**
 * @brief Sets the cell padding edges around any widget.
 * @param widget The widget to modify.
 * @param padding Padding edges (`top >= 0`, `right >= 0`, `bottom >= 0`,
 * `left >= 0`).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_set_padding(void *widget,
                                                  cmn_edges_t padding);

/**
 * @brief Gets the cell padding edges around any widget.
 * @param widget The widget to query.
 * @param out_padding Pointer receiving the `cmn_edges_t` padding.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_get_padding(const void *widget,
                                                  cmn_edges_t *out_padding);

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
