#ifndef CMN_SCREENSHOT_GEN_H
#define CMN_SCREENSHOT_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
/* clang-format on */

/**
 * @file screenshot_gen.h
 * @brief Real native OS window and widget screenshot generator in C.
 */

/**
 * @defgroup cmn_screenshot_gen Screenshot Generator
 * @brief Captures genuine OS-rendered windows and native controls via
 * `cmn_automation_capture_screenshot`.
 * @{
 */

/**
 * @brief Returns the canonical platform suffix string (`"macos"`, `"windows"`,
 * `"linux"`, or `"web"`) for the current target OS.
 * @param out_suffix Pointer to receive the static platform suffix string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_screenshot_get_platform_suffix(const char **out_suffix);

/**
 * @brief Builds a real native Buttons showcase window (`cmn_button_t` inside
 * responsive `cmn_flex_t` containers), triggers programmatic button clicks, and
 * captures the live OS window to `filepath`.
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_screenshot_capture_buttons_scene(const char *filepath);

/**
 * @brief Builds a real native Inputs showcase window (`cmn_label_t`,
 * `cmn_textbox_t`, and scrollable `cmn_textview_t`), populates them via UI
 * automation, and captures the live OS window to `filepath`.
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_screenshot_capture_inputs_scene(const char *filepath);

/**
 * @brief Builds the real Hello World 3-row counter application window
 * (`cmn_label_t`, `cmn_button_t`, `cmn_textview_t`), simulates 4 native button
 * clicks, and captures the live OS window to `filepath`.
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_screenshot_capture_hello_world_scene(const char *filepath);

/**
 * @brief Builds the real Simple Password Flow application window (`cmn_form_t`,
 * `cmn_textbox_t`, `cmn_button_t`, `cmn_label_t`), enters credentials,
 * validates the form, and captures the live OS window to `filepath`.
 * @param filepath Destination PNG file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_screenshot_capture_password_flow_scene(const char *filepath);

/**
 * @brief Writes the HTML5 DOM document corresponding to a `cmn_web_widget.c`
 * scene (`0`=buttons, `1`=inputs, `2`=hello_world, `3`=password_flow) so a real
 * headless browser can render and screenshot the exact Web DOM widgets.
 * @param scene_id Scene index (`0`..`3`).
 * @param html_filepath Destination `.html` file path.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e
cmn_screenshot_write_web_html(int scene_id, const char *html_filepath);

/**
 * @brief Captures all real native widget and application screenshots for the
 * current host OS into `output_dir` using `_<platform>.png` filenames (or
 * `platform_override` when non-NULL).
 * @param output_dir Directory path where PNG files will be written.
 * @param platform_override Optional platform suffix override (e.g., `"macos"`,
 * `"windows"`, `"linux"`, `"web"`), or `NULL` to auto-detect the host OS.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e cmn_screenshot_generate_all(
    const char *output_dir, const char *platform_override);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_SCREENSHOT_GEN_H */
