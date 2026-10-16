/* clang-format off */
#include "cmn_macos_internal.h"
#include "handle_map.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include <ApplicationServices/ApplicationServices.h>
#include <objc/runtime.h>
/* clang-format on */

/**
 * @file cmn_macos_widget.m
 * @brief macOS AppKit implementation of the widget ontology.
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

/**
 * @brief Objective-C target for button click events.
 */
@interface CmnButtonTarget : NSObject
/**
 * @brief The associated C button widget.
 */
@property(nonatomic, assign) cmn_button_t *cmn_button;
/**
 * @brief Action method triggered when the button is clicked.
 * @param sender The object that triggered the action.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
- (cmn_error_e)onButtonClicked:(id)sender;
@end

@implementation CmnButtonTarget
- (cmn_error_e)onButtonClicked:(id)sender {
  (void)sender;
  if (self.cmn_button && self.cmn_button->on_click) {
    return self.cmn_button->on_click(self.cmn_button->userdata);
  }
  return CMN_SUCCESS;
}
@end

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
 * @brief Polymorphic destructor adapter for `cmn_menu_t`.
 * @param ptr Address of the menu pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_menu_adapter(void **ptr) {
  return cmn_menu_destroy((cmn_menu_t **)ptr);
}

/**
 * @brief Polymorphic destructor adapter for `cmn_filepicker_t`.
 * @param ptr Address of the filepicker pointer.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e destroy_filepicker_adapter(void **ptr) {
  return cmn_filepicker_destroy((cmn_filepicker_t **)ptr);
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

  @autoreleasepool {
    NSRect frame = NSMakeRect(0, 0, 800, 600);
    NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                           NSWindowStyleMaskResizable |
                           NSWindowStyleMaskMiniaturizable;

    win->ns_window =
        [[NSWindow alloc] initWithContentRect:frame
                                    styleMask:styleMask
                                      backing:NSBackingStoreBuffered
                                        defer:NO];
    [win->ns_window setReleasedWhenClosed:NO];
    [win->ns_window center];
    win->app = app;
    rc = cmn_handle_map_add((__bridge void *)win->ns_window, win);
  }

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
  if (window == NULL || title == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    [window->ns_window setTitle:[NSString stringWithUTF8String:title]];
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
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSUInteger mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                      NSWindowStyleMaskResizable |
                      NSWindowStyleMaskMiniaturizable;

    if (style & CMN_WINDOW_STYLE_FRAMELESS) {
      mask = NSWindowStyleMaskBorderless | NSWindowStyleMaskResizable;
      [window->ns_window setMovableByWindowBackground:YES];
    }

    [window->ns_window setStyleMask:mask];

    if (style & CMN_WINDOW_STYLE_TRANSPARENT) {
      [window->ns_window setOpaque:NO];
      [window->ns_window setBackgroundColor:[NSColor clearColor]];
    } else {
      [window->ns_window setOpaque:YES];
      [window->ns_window setBackgroundColor:[NSColor windowBackgroundColor]];
    }
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
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    BOOL is_currently_fs =
        ([window->ns_window styleMask] & NSWindowStyleMaskFullScreen) != 0;
    if ((is_fullscreen && !is_currently_fs) ||
        (!is_fullscreen && is_currently_fs)) {
      [window->ns_window toggleFullScreen:nil];
    }
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
  @autoreleasepool {
    [window->ns_window zoom:nil];
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
  @autoreleasepool {
    [window->ns_window miniaturize:nil];
  }
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
  @autoreleasepool {
    *out_count = (int)[[NSScreen screens] count];
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
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSArray<NSScreen *> *screens = [NSScreen screens];
    if (display_index < 0 || display_index >= (int)[screens count]) {
      return CMN_ERR_INVAL;
    }
    [window->ns_window center];
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
  @autoreleasepool {
    *out_scale = (float)[window->ns_window backingScaleFactor];
  }
  return CMN_SUCCESS;
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
  @autoreleasepool {
    [window->ns_window makeKeyAndOrderFront:nil];
  }
  return CMN_SUCCESS;
}

/**
 * @brief Destroys a window and frees its memory.
 * @param window Pointer to the window pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_destroy(cmn_window_t **window) {
  if (window == NULL || *window == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    (void)cmn_handle_map_remove((__bridge void *)(*window)->ns_window);
    [(*window)->ns_window close];
    (*window)->ns_window = nil;
  }
  return cmn_free((void **)window);
}

/* --- Multiwindow & Dialogs --- */

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
  @autoreleasepool {
    [parent->ns_window beginSheet:modal->ns_window completionHandler:nil];
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
  @autoreleasepool {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[NSString stringWithUTF8String:title]];
    [alert setInformativeText:[NSString stringWithUTF8String:msg]];
    [alert addButtonWithTitle:@"OK"];
#ifndef CMN_TESTING
    [alert runModal];
#endif
  }
  return CMN_SUCCESS;
}

/* --- Base Widget API --- */

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
  cmn_error_e rc;

  if (parent == NULL || child == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_widget_get_kind(parent, &parent_kind);
  if (rc != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_widget_get_kind(child, &child_kind);
  if (rc != CMN_SUCCESS) {
    return CMN_ERR_INVAL;
  }

  @autoreleasepool {
    NSView *parent_view = nil;
    NSView *child_view = nil;
    BOOL is_stack_parent = NO;

    switch (parent_kind) {
    case CMN_WIDGET_KIND_WINDOW:
      parent_view = [((cmn_window_t *)parent)->ns_window contentView];
      break;
    case CMN_WIDGET_KIND_VBOX:
      parent_view = ((cmn_vbox_t *)parent)->ns_stack;
      is_stack_parent = YES;
      break;
    case CMN_WIDGET_KIND_HBOX:
      parent_view = ((cmn_hbox_t *)parent)->ns_stack;
      is_stack_parent = YES;
      break;
    default:
      return CMN_ERR_INVAL;
    }

    switch (child_kind) {
    case CMN_WIDGET_KIND_BUTTON:
      child_view = ((cmn_button_t *)child)->ns_button;
      break;
    case CMN_WIDGET_KIND_LABEL:
      child_view = ((cmn_label_t *)child)->ns_label;
      break;
    case CMN_WIDGET_KIND_TEXTBOX:
      child_view = ((cmn_textbox_t *)child)->ns_textbox;
      break;
    case CMN_WIDGET_KIND_TEXTVIEW:
      child_view = ((cmn_textview_t *)child)->ns_scrollview;
      break;
    case CMN_WIDGET_KIND_VBOX:
      child_view = ((cmn_vbox_t *)child)->ns_stack;
      break;
    case CMN_WIDGET_KIND_HBOX:
      child_view = ((cmn_hbox_t *)child)->ns_stack;
      break;
    default:
      return CMN_ERR_INVAL;
    }

    [child_view removeFromSuperview];
    if (is_stack_parent) {
      [(NSStackView *)parent_view addArrangedSubview:child_view];
    } else {
      [parent_view addSubview:child_view];
    }
  }
  return CMN_SUCCESS;
}

/* --- Button --- */

/**
 * @brief Creates a new button widget.
 * @param out_button Pointer to store the created button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_create(cmn_button_t **out_button) {
  cmn_error_e rc;
  cmn_button_t *btn = NULL;

  if (out_button == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_button_t), (void **)&btn);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&btn->base, CMN_WIDGET_KIND_BUTTON,
                              destroy_button_adapter);

  @autoreleasepool {
    btn->ns_button = [NSButton buttonWithTitle:@"" target:nil action:nil];
    [btn->ns_button setBezelStyle:NSBezelStyleRounded];

    CmnButtonTarget *target = [[CmnButtonTarget alloc] init];
    target.cmn_button = btn;

    [btn->ns_button setTarget:target];
    [btn->ns_button setAction:@selector(onButtonClicked:)];

    btn->target = target;
    rc = cmn_handle_map_add((__bridge void *)btn->ns_button, btn);
  }

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
  @autoreleasepool {
    [button->ns_button setTitle:[NSString stringWithUTF8String:text]];
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
  cmn_error_e rc = CMN_SUCCESS;
  if (button == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    rc = [(CmnButtonTarget *)button->target onButtonClicked:nil];
  }
  return rc;
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
  @autoreleasepool {
    (void)cmn_handle_map_remove((__bridge void *)(*button)->ns_button);
    ((CmnButtonTarget *)(*button)->target).cmn_button = NULL;
    [(*button)->ns_button setTarget:nil];
    [(*button)->ns_button removeFromSuperview];
    (*button)->ns_button = nil;
    (*button)->target = nil;
  }
  return cmn_free((void **)button);
}

/* --- Label --- */

/**
 * @brief Creates a new label widget.
 * @param out_label Pointer to store the created label.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_create(cmn_label_t **out_label) {
  cmn_error_e rc;
  cmn_label_t *lbl = NULL;

  if (out_label == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_label_t), (void **)&lbl);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&lbl->base, CMN_WIDGET_KIND_LABEL,
                              destroy_label_adapter);

  @autoreleasepool {
    lbl->ns_label = [NSTextField labelWithString:@""];
    rc = cmn_handle_map_add((__bridge void *)lbl->ns_label, lbl);
  }

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
  @autoreleasepool {
    [label->ns_label setStringValue:[NSString stringWithUTF8String:text]];
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
  @autoreleasepool {
    (void)cmn_handle_map_remove((__bridge void *)(*label)->ns_label);
    [(*label)->ns_label removeFromSuperview];
    (*label)->ns_label = nil;
  }
  return cmn_free((void **)label);
}

/* --- TextBox --- */

/**
 * @brief Creates a new text box widget.
 * @param out_textbox Pointer to store the created text box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_create(cmn_textbox_t **out_textbox) {
  cmn_error_e rc;
  cmn_textbox_t *tb = NULL;

  if (out_textbox == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_textbox_t), (void **)&tb);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&tb->base, CMN_WIDGET_KIND_TEXTBOX,
                              destroy_textbox_adapter);

  @autoreleasepool {
    tb->ns_textbox = [NSTextField textFieldWithString:@""];
    rc = cmn_handle_map_add((__bridge void *)tb->ns_textbox, tb);
  }

  *out_textbox = tb;
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
  @autoreleasepool {
    [textbox->ns_textbox setStringValue:[NSString stringWithUTF8String:text]];
  }
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
  cmn_error_e rc = CMN_SUCCESS;
  if (g_cmn_mock_textbox_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textbox == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSString *val = [textbox->ns_textbox stringValue];
    rc = cmn_strcpy_s(textbox->text_cache, sizeof(textbox->text_cache),
                      [val UTF8String]);
    *out_text = textbox->text_cache;
  }
  return rc;
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
  @autoreleasepool {
    (void)cmn_handle_map_remove((__bridge void *)(*textbox)->ns_textbox);
    [(*textbox)->ns_textbox removeFromSuperview];
    (*textbox)->ns_textbox = nil;
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
  cmn_error_e rc;
  cmn_textview_t *tv = NULL;

  if (out_textview == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_textview_t), (void **)&tv);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&tv->base, CMN_WIDGET_KIND_TEXTVIEW,
                              destroy_textview_adapter);

  @autoreleasepool {
    tv->ns_scrollview = [[NSScrollView alloc] init];
    [tv->ns_scrollview setHasVerticalScroller:YES];
    [tv->ns_scrollview setHasHorizontalScroller:YES];
    [tv->ns_scrollview setAutohidesScrollers:YES];

    NSSize contentSize = [tv->ns_scrollview contentSize];
    tv->ns_textview = [[NSTextView alloc]
        initWithFrame:NSMakeRect(0, 0, contentSize.width, contentSize.height)];
    [tv->ns_textview setMinSize:NSMakeSize(0.0, contentSize.height)];
    [tv->ns_textview setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
    [tv->ns_textview setVerticallyResizable:YES];
    [tv->ns_textview setHorizontallyResizable:NO];
    [tv->ns_textview setAutoresizingMask:NSViewWidthSizable];
    [[tv->ns_textview textContainer]
        setContainerSize:NSMakeSize(contentSize.width, FLT_MAX)];
    [[tv->ns_textview textContainer] setWidthTracksTextView:YES];
    [tv->ns_textview setEditable:NO];

    [tv->ns_scrollview setDocumentView:tv->ns_textview];
    rc = cmn_handle_map_add((__bridge void *)tv->ns_scrollview, tv);
  }

  *out_textview = tv;
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
  @autoreleasepool {
    [textview->ns_textview setString:[NSString stringWithUTF8String:text]];
  }
  return CMN_SUCCESS;
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
  @autoreleasepool {
    NSAttributedString *attrString = [[NSAttributedString alloc]
        initWithString:[NSString stringWithUTF8String:text]];
    [[textview->ns_textview textStorage] appendAttributedString:attrString];
    [textview->ns_textview
        scrollRangeToVisible:NSMakeRange(
                                 [[textview->ns_textview string] length], 0)];
  }
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
  cmn_error_e rc = CMN_SUCCESS;
  if (g_cmn_mock_textview_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textview == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSString *val = [textview->ns_textview string];
    rc = cmn_strcpy_s(textview->text_cache, sizeof(textview->text_cache),
                      [val UTF8String]);
    *out_text = textview->text_cache;
  }
  return rc;
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
  @autoreleasepool {
    (void)cmn_handle_map_remove((__bridge void *)(*textview)->ns_scrollview);
    [(*textview)->ns_scrollview removeFromSuperview];
    (*textview)->ns_textview = nil;
    (*textview)->ns_scrollview = nil;
  }
  return cmn_free((void **)textview);
}

/* --- Automation --- */

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
    NSView *view = [win->ns_window contentView];
    NSRect bounds = [view bounds];
    NSBitmapImageRep *rep = [view bitmapImageRepForCachingDisplayInRect:bounds];
    [view cacheDisplayInRect:bounds toBitmapImageRep:rep];
    NSData *pngData = [rep representationUsingType:NSBitmapImageFileTypePNG
                                        properties:@{}];
    NSString *path = [NSString stringWithUTF8String:out_filepath];
    BOOL written = [pngData writeToFile:path atomically:YES];
    if (!written) {
      return CMN_ERR_IO;
    }
  }
  return CMN_SUCCESS;
}

/* --- Layouts --- */

/**
 * @brief Creates a vertical box layout container.
 * @param out_vbox Pointer to store the created vertical box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_vbox_create(cmn_vbox_t **out_vbox) {
  cmn_error_e rc;
  cmn_vbox_t *vbox = NULL;

  if (out_vbox == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_vbox_t), (void **)&vbox);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&vbox->base, CMN_WIDGET_KIND_VBOX,
                              destroy_vbox_adapter);

  @autoreleasepool {
    vbox->ns_stack = [[NSStackView alloc] init];
    [vbox->ns_stack setOrientation:NSUserInterfaceLayoutOrientationVertical];
    [vbox->ns_stack setSpacing:10.0];
    [vbox->ns_stack setAlignment:NSLayoutAttributeCenterX];
    rc = cmn_handle_map_add((__bridge void *)vbox->ns_stack, vbox);
  }

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
  @autoreleasepool {
    (void)cmn_handle_map_remove((__bridge void *)(*vbox)->ns_stack);
    [(*vbox)->ns_stack removeFromSuperview];
    (*vbox)->ns_stack = nil;
  }
  return cmn_free((void **)vbox);
}

/**
 * @brief Creates a horizontal box layout container.
 * @param out_hbox Pointer to store the created horizontal box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_hbox_create(cmn_hbox_t **out_hbox) {
  cmn_error_e rc;
  cmn_hbox_t *hbox = NULL;

  if (out_hbox == NULL) {
    return CMN_ERR_INVAL;
  }

  rc = cmn_calloc(1, sizeof(cmn_hbox_t), (void **)&hbox);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&hbox->base, CMN_WIDGET_KIND_HBOX,
                              destroy_hbox_adapter);

  @autoreleasepool {
    hbox->ns_stack = [[NSStackView alloc] init];
    [hbox->ns_stack setOrientation:NSUserInterfaceLayoutOrientationHorizontal];
    [hbox->ns_stack setSpacing:10.0];
    [hbox->ns_stack setAlignment:NSLayoutAttributeCenterY];
    rc = cmn_handle_map_add((__bridge void *)hbox->ns_stack, hbox);
  }

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
  @autoreleasepool {
    (void)cmn_handle_map_remove((__bridge void *)(*hbox)->ns_stack);
    [(*hbox)->ns_stack removeFromSuperview];
    (*hbox)->ns_stack = nil;
  }
  return cmn_free((void **)hbox);
}

/* --- Menus --- */

/**
 * @brief Objective-C target for menu item action events.
 */
@interface CmnMenuItemTarget : NSObject
/**
 * @brief The callback function for the menu item.
 */
@property(nonatomic, assign) cmn_error_e (*on_click)(void *);
/**
 * @brief User data passed to the callback.
 */
@property(nonatomic, assign) void *userdata;
/**
 * @brief Action method triggered when the menu item is selected.
 * @param sender The object that triggered the action.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
- (cmn_error_e)action:(id)sender;
@end

@implementation CmnMenuItemTarget
- (cmn_error_e)action:(id)sender {
  (void)sender;
  if (self.on_click) {
    return self.on_click(self.userdata);
  }
  return CMN_SUCCESS;
}
@end

/**
 * @brief Creates a new menu.
 * @param out_menu Pointer to store the created menu.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_create(cmn_menu_t **out_menu) {
  cmn_error_e rc;
  cmn_menu_t *m = NULL;
  if (out_menu == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_menu_t), (void **)&m);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&m->base, CMN_WIDGET_KIND_MENU,
                              destroy_menu_adapter);

  @autoreleasepool {
    m->ns_menu = [[NSMenu alloc] init];
    m->targets = [[NSMutableArray alloc] init];
  }
  *out_menu = m;
  return CMN_SUCCESS;
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
  @autoreleasepool {
    [(*menu)->targets removeAllObjects];
    (*menu)->targets = nil;
    (*menu)->ns_menu = nil;
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
  if (menu == NULL || title == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSString *ns_title = [NSString stringWithUTF8String:title];
    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:ns_title
                                                  action:NULL
                                           keyEquivalent:@""];

    CmnMenuItemTarget *target = [[CmnMenuItemTarget alloc] init];
    target.on_click = on_click;
    target.userdata = userdata;
    if (on_click != NULL) {
      item.target = target;
      item.action = @selector(action:);
    }
    [menu->targets addObject:target];
    [menu->ns_menu addItem:item];
  }
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers a menu item's callback by index.
 * @param menu The menu containing the item.
 * @param index The 0-based index of the menu item to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_trigger_item(cmn_menu_t *menu, int index) {
  cmn_error_e rc = CMN_SUCCESS;
  if (menu == NULL || index < 0) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    if ((NSUInteger)index >= [menu->targets count]) {
      return CMN_ERR_INVAL;
    }
    CmnMenuItemTarget *target = [menu->targets objectAtIndex:(NSUInteger)index];
    rc = [target action:nil];
  }
  return rc;
}

/**
 * @brief Sets the main menu bar for a window.
 * @param window The window to set the menu on.
 * @param menu The menu to attach.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_menu(cmn_window_t *window, cmn_menu_t *menu) {
  (void)window;
  if (menu == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    [NSApp setMainMenu:menu->ns_menu];
  }
  return CMN_SUCCESS;
}

/* --- File Picker --- */

/**
 * @brief Creates a new file picker dialog.
 * @param out_filepicker Pointer to store the created file picker.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_create(cmn_filepicker_t **out_filepicker) {
  cmn_error_e rc;
  cmn_filepicker_t *fp = NULL;
  if (out_filepicker == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_filepicker_t), (void **)&fp);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&fp->base, CMN_WIDGET_KIND_FILEPICKER,
                              destroy_filepicker_adapter);

  @autoreleasepool {
    fp->ns_path = @"";
  }
  *out_filepicker = fp;
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

  @autoreleasepool {
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];

#ifdef CMN_TESTING
    filepicker->ns_path = @"/tmp/test";
#else
    if ([panel runModal] == NSModalResponseOK) {
      NSURL *url = [[panel URLs] objectAtIndex:0];
      filepicker->ns_path = [url path];
    }
#endif
  }
  return CMN_SUCCESS;
}

/**
 * @brief Gets the selected file path from the file picker.
 * @param filepicker The file picker to query.
 * @param out_path Pointer to store the selected file path string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_get_path(cmn_filepicker_t *filepicker,
                                    const char **out_path) {
  cmn_error_e rc = CMN_SUCCESS;
  if (filepicker == NULL || out_path == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    rc = cmn_strcpy_s(filepicker->path_cache, sizeof(filepicker->path_cache),
                      [filepicker->ns_path UTF8String]);
    *out_path = filepicker->path_cache;
  }
  return rc;
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
  @autoreleasepool {
    (*filepicker)->ns_path = nil;
  }
  return cmn_free((void **)filepicker);
}
