#ifndef HELLO_WORLD_APP_H
#define HELLO_WORLD_APP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/app.h"
#include "cmn/layout.h"
#include "cmn/widget.h"
#include "cmn/error.h"
/* clang-format on */

/**
 * @file app.h
 * @brief Hello World 3-row native counter application logic.
 */

/**
 * @defgroup example_hello_world Hello World Example
 * @brief Hello World Example module.
 * @{
 */

/**
 * @brief Structure holding the hello world app state and UI components.
 */
typedef struct hello_world_app_s {
  cmn_app_t *app;           /**< Application instance. */
  cmn_window_t *window;     /**< Main window. */
  cmn_flex_t *flex;         /**< Responsive Flexbox column layout container. */
  cmn_label_t *label;       /**< Greeting label. */
  cmn_button_t *button;     /**< Clickable button. */
  cmn_textview_t *textview; /**< Scrollable text view for logs. */
  unsigned int click_count; /**< Number of times button clicked. */
} hello_world_app_t;

/**
 * @brief Initializes the hello world application.
 * @param out_app Pointer to receive the initialized app context.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e hello_world_app_init(hello_world_app_t **out_app);

/**
 * @brief Runs the hello world application.
 * @param app The initialized app context.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e hello_world_app_run(hello_world_app_t *app);

/**
 * @brief Tears down the hello world application.
 * @param app The app context pointer to free.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
CMN_NO_DISCARD cmn_error_e hello_world_app_destroy(hello_world_app_t **app);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HELLO_WORLD_APP_H */
