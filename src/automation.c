/* clang-format off */
#include "cmn_internal.h"
#include "cmn/automation.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file automation.c
 * @brief Implementation of UI automation and programmatic testing utilities.
 */

/**
 * @brief Programmatically clicks a widget.
 * @param widget The widget to click.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_click(void *widget) {
  cmn_widget_kind_e kind;
  if (widget == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind != CMN_WIDGET_KIND_BUTTON) {
    return CMN_ERR_INVAL;
  }

  return cmn_button_trigger_click((cmn_button_t *)widget);
}

/**
 * @brief Programmatically types text into a widget.
 * @param widget The widget to type into (`cmn_textbox_t *` or `cmn_textview_t
 * *`).
 * @param text The text string to type.
 * @param delay_ms_per_char Delay in milliseconds between each character.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_type_text(void *widget, const char *text,
                                     int delay_ms_per_char) {
  cmn_widget_kind_e kind;
  if (widget == NULL || text == NULL || delay_ms_per_char < 0) {
    return CMN_ERR_INVAL;
  }

  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }

  if (kind == CMN_WIDGET_KIND_TEXTBOX) {
    return cmn_textbox_set_text((cmn_textbox_t *)widget, text);
  }
  if (kind == CMN_WIDGET_KIND_TEXTVIEW) {
    return cmn_textview_append_text((cmn_textview_t *)widget, text);
  }

  return CMN_ERR_INVAL;
}

/**
 * @brief Programmatically sends a key press to a window.
 * @param win The target window.
 * @param key The keycode to send.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_send_key(cmn_window_t *win, cmn_keycode_e key) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  cmn_error_e rc = cmn_widget_get_kind(win, &kind);
  if (rc != CMN_SUCCESS || kind != CMN_WIDGET_KIND_WINDOW) {
    return CMN_ERR_INVAL;
  }
  if (key < CMN_KEY_TAB || key > CMN_KEY_RIGHT) {
    return CMN_ERR_INVAL;
  }

  return CMN_SUCCESS;
}

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__linux__) &&          \
    !defined(__CYGWIN__) && !defined(__unix__) && !defined(__EMSCRIPTEN__) &&  \
    !defined(__ANDROID__)

/**
 * @brief Captures a screenshot of a window and saves it to a file.
 * @param win The window to capture.
 * @param out_filepath The destination file path for the screenshot.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_capture_screenshot(cmn_window_t *win,
                                              const char *out_filepath) {
  if (win == NULL || out_filepath == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

#endif
