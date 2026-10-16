#ifndef CMN_AUTOMATION_H
#define CMN_AUTOMATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
#include "cmn/widget.h"
/* clang-format on */

/**
 * @file automation.h
 * @brief UI Automation and programmatic testing utilities.
 */

/**
 * @defgroup cmn_automation UI Automation
 * @brief UI Automation module.
 * @{
 */

/**
 * @brief Virtual keycodes for keyboard automation.
 */
typedef enum cmn_keycode_e {
  /** @brief Tab key. */
  CMN_KEY_TAB = 1,
  /** @brief Enter/Return key. */
  CMN_KEY_ENTER = 2,
  /** @brief Spacebar key. */
  CMN_KEY_SPACE = 3,
  /** @brief Escape key. */
  CMN_KEY_ESCAPE = 4,
  /** @brief Up arrow key. */
  CMN_KEY_UP = 5,
  /** @brief Down arrow key. */
  CMN_KEY_DOWN = 6,
  /** @brief Left arrow key. */
  CMN_KEY_LEFT = 7,
  /** @brief Right arrow key. */
  CMN_KEY_RIGHT = 8
} cmn_keycode_e;

/**
 * @brief Programmatically clicks a widget.
 * @param widget The widget to click.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_automation_click(void *widget);

/**
 * @brief Programmatically types text into a widget.
 * @param widget The widget to type into (`cmn_textbox_t *` or `cmn_textview_t
 * *`).
 * @param text The text string to type.
 * @param delay_ms_per_char Delay in milliseconds between each character.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_automation_type_text(void *widget,
                                                    const char *text,
                                                    int delay_ms_per_char);

/**
 * @brief Programmatically sends a key press to a window.
 * @param win The target window.
 * @param key The keycode to send.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_automation_send_key(cmn_window_t *win,
                                                   cmn_keycode_e key);

/**
 * @brief Captures a screenshot of a window and saves it to a file.
 * @param win The window to capture.
 * @param out_filepath The destination file path for the screenshot.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_automation_capture_screenshot(cmn_window_t *win, const char *out_filepath);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_AUTOMATION_H */
