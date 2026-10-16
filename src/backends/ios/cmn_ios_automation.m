/* clang-format off */
#include "cmn_ios_internal.h"
#include "cmn/automation.h"
#include "cmn/error.h"
/* clang-format on */

/**
 * @file cmn_ios_automation.m
 * @brief iOS UIKit implementation of screenshot capture.
 */

/**
 * @brief Captures a screenshot of a window and saves it to a file.
 * @param win The window to capture.
 * @param out_filepath The destination file path for the screenshot.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_capture_screenshot(cmn_window_t *win,
                                              const char *out_filepath) {
  if (win == NULL || out_filepath == NULL || out_filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  @autoreleasepool {
    if (win->ui_window == nil) {
      return CMN_ERR_INVAL;
    }

    UIGraphicsImageRenderer *renderer =
        [[UIGraphicsImageRenderer alloc] initWithBounds:win->ui_window.bounds];
    UIImage *image = [renderer
        imageWithActions:^(UIGraphicsImageRendererContext *rendererContext) {
          (void)rendererContext;
          [win->ui_window drawViewHierarchyInRect:win->ui_window.bounds
                               afterScreenUpdates:YES];
        }];

    if (image == nil) {
      return CMN_ERR_IO;
    }

    NSData *pngData = UIImagePNGRepresentation(image);
    if (pngData == nil) {
      return CMN_ERR_IO;
    }

    NSString *path = [NSString stringWithUTF8String:out_filepath];
    BOOL success = [pngData writeToFile:path atomically:YES];
    if (!success) {
      return CMN_ERR_IO;
    }
  }

  return CMN_SUCCESS;
}
