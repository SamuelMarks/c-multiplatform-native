/* clang-format off */
#include "cmn/app.h"
#include "cmn/memory.h"
#include <Cocoa/Cocoa.h>
/* clang-format on */

/**
 * @file cmn_macos_app.m
 * @brief macOS AppKit implementation of the application lifecycle.
 */

/**
 * @brief Internal representation of the macOS application.
 */
struct cmn_app_s {
  NSApplication *ns_app; /**< Shared NSApplication instance. */
  id delegate;           /**< CmnAppDelegate instance. */
};

/**
 * @brief AppKit delegate managing application launch and window termination.
 */
@interface CmnAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation CmnAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
  (void)notification;
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp activateIgnoringOtherApps:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)sender {
  (void)sender;
  return YES;
}
@end

/**
 * @brief Creates a new application instance.
 * @param out_app Pointer to store the created application handle.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_create(cmn_app_t **out_app) {
  cmn_error_e rc;
  cmn_app_t *app = NULL;

  if (out_app == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_app_t), (void **)&app);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  @autoreleasepool {
    app->ns_app = [NSApplication sharedApplication];
    app->delegate = [[CmnAppDelegate alloc] init];
    [app->ns_app setDelegate:app->delegate];
  }

  *out_app = app;
  return CMN_SUCCESS;
}

#ifdef CMN_TESTING
/**
 * @brief Invokes the macOS application delegate methods for unit test
 * verification.
 * @param app The application instance whose delegate should be exercised.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if app is NULL.
 */
cmn_error_e cmn_macos_test_exercise_delegate(cmn_app_t *app) {
  if (app == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    CmnAppDelegate *del = (CmnAppDelegate *)app->delegate;
    [del applicationDidFinishLaunching:[NSNotification
                                           notificationWithName:@"Test"
                                                         object:nil]];
    (void)[del applicationShouldTerminateAfterLastWindowClosed:app->ns_app];
  }
  return CMN_SUCCESS;
}
#endif

/**
 * @brief Starts the main event loop of the application.
 * @param app The application instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_run(cmn_app_t *app) {
  if (app == NULL) {
    return CMN_ERR_INVAL;
  }

#ifndef CMN_TESTING
  @autoreleasepool {
    [app->ns_app run];
  }
#endif

  return CMN_SUCCESS;
}

/**
 * @brief Signals the application's main event loop to quit.
 * @param app The application instance.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_quit(cmn_app_t *app) {
  if (app == NULL) {
    return CMN_ERR_INVAL;
  }

  @autoreleasepool {
    [app->ns_app stop:nil];
    NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:0.0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [app->ns_app postEvent:event atStart:YES];
  }

  return CMN_SUCCESS;
}

/**
 * @brief Destroys an application instance and frees its resources.
 * @param app Pointer to the application handle to be destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_destroy(cmn_app_t **app) {
  if (app == NULL || *app == NULL) {
    return CMN_ERR_INVAL;
  }

  @autoreleasepool {
    [(*app)->ns_app setDelegate:nil];
#if !__has_feature(objc_arc)
    [(*app)->delegate release];
#endif
  }

  return cmn_free((void **)app);
}

/**
 * @brief Posts a task to be executed on the main UI thread.
 * @param app The application instance.
 * @param task The function pointer to execute.
 * @param userdata User data passed to the task function.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_app_post_task(cmn_app_t *app, cmn_error_e (*task)(void *),
                              void *userdata) {
  if (app == NULL || task == NULL) {
    return CMN_ERR_INVAL;
  }
#ifdef CMN_TESTING
  return task(userdata);
#else
  dispatch_async(dispatch_get_main_queue(), ^{
    (void)task(userdata);
  });
  return CMN_SUCCESS;
#endif
}
