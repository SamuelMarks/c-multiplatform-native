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
/** @brief Last handle map operation status code. */
static int g_macos_map_rc = 0;

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
  if (sender == nil || sender != nil) {
    if (self.cmn_button && self.cmn_button->on_click) {
      return self.cmn_button->on_click(self.cmn_button->userdata);
    }
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
    NSRect frame = NSMakeRect(0, 0, 480, 340);
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
 * @brief Resolves a widget handle to its underlying `NSView`.
 * @param node Widget pointer.
 * @param kind Widget kind discriminant.
 * @param out_view Output pointer receiving the resolved `NSView *` or `nil`.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e macos_resolve_widget_view(void *node, cmn_widget_kind_e kind,
                                             NSView **out_view) {
  switch (kind) {
  case CMN_WIDGET_KIND_BUTTON:
    *out_view = ((cmn_button_t *)node)->ns_button;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_LABEL:
    *out_view = ((cmn_label_t *)node)->ns_label;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_TEXTBOX:
    *out_view = ((cmn_textbox_t *)node)->ns_textbox;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_TEXTVIEW:
    *out_view = ((cmn_textview_t *)node)->ns_scrollview;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_VBOX:
    *out_view = ((cmn_vbox_t *)node)->ns_stack;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_HBOX:
    *out_view = ((cmn_hbox_t *)node)->ns_stack;
    return CMN_SUCCESS;
  default:
    if (kind == CMN_WIDGET_KIND_WINDOW || kind == CMN_WIDGET_KIND_MENU ||
        kind == CMN_WIDGET_KIND_FILEPICKER ||
        kind == CMN_WIDGET_KIND_COLORPICKER || kind == CMN_WIDGET_KIND_FLEX ||
        kind == CMN_WIDGET_KIND_GRID) {
      *out_view = nil;
      return CMN_SUCCESS;
    }
    *out_view = (__bridge NSView *)((cmn_widget_header_t *)node)->native_handle;
    return CMN_SUCCESS;
  }
}

/**
 * @brief Recursively attaches and positions child `NSView` controls inside
 * `content_view` according to the computed Flexbox (`cmn_flex_t`) or Grid
 * (`cmn_grid_t`) layout hierarchy.
 * @param content_view The target parent `NSView`.
 * @param win_height Total content height used to flip top-left Y to AppKit Y.
 * @param node Current layout container or leaf widget pointer.
 * @param offset_x Cumulative horizontal offset from window origin.
 * @param offset_y Cumulative vertical offset from window top-left origin.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e macos_apply_layout_tree(NSView *content_view, int win_height,
                                           void *node, int offset_x,
                                           int offset_y) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  cmn_error_e rc;

  if (content_view == nil || node == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_widget_get_kind(node, &kind);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  if (kind == CMN_WIDGET_KIND_FLEX) {
    cmn_flex_t *flex = (cmn_flex_t *)node;
    for (i = 0; i < flex->child_count; ++i) {
      cmn_rect_t r = flex->children[i].computed_rect;
      rc = macos_apply_layout_tree(content_view, win_height,
                                   flex->children[i].widget, offset_x + r.x,
                                   offset_y + r.y);
    }
    return rc;
  } else if (kind == CMN_WIDGET_KIND_GRID) {
    cmn_grid_t *grid = (cmn_grid_t *)node;
    for (i = 0; i < grid->child_count; ++i) {
      cmn_rect_t r = grid->children[i].computed_rect;
      rc = macos_apply_layout_tree(content_view, win_height,
                                   grid->children[i].widget, offset_x + r.x,
                                   offset_y + r.y);
    }
    return rc;
  } else {
    NSView *child_view = nil;
    cmn_rect_t r = ((cmn_widget_header_t *)node)->bounds;
    int w = (r.width > 0) ? r.width : 200;
    int h = (r.height > 0) ? r.height
                           : (kind == CMN_WIDGET_KIND_TEXTVIEW ? 140 : 28);
    int flipped_y = win_height - offset_y - h;

    rc = macos_resolve_widget_view(node, kind, &child_view);

    if (child_view != nil) {
      [child_view removeFromSuperview];
      [child_view setTranslatesAutoresizingMaskIntoConstraints:YES];
      [child_view setFrame:NSMakeRect((CGFloat)offset_x, (CGFloat)flipped_y,
                                      (CGFloat)w, (CGFloat)h)];
      [content_view addSubview:child_view];
      if (kind >= CMN_WIDGET_KIND_CHECKBOX) {
        rc = cmn_backend_widget_sync((cmn_widget_header_t *)node);
      }
    }
  }
  return rc;
}

/**
 * @brief Shows a window on the screen.
 * @param window The window to show.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_show(cmn_window_t *window) {
  cmn_error_e rc = CMN_SUCCESS;
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    if (window->base.layout_root != NULL) {
      NSView *cv;
      int win_w =
          (window->base.bounds.width > 0) ? window->base.bounds.width : 480;
      int win_h =
          (window->base.bounds.height > 0) ? window->base.bounds.height : 360;
      [window->ns_window
          setContentSize:NSMakeSize((CGFloat)win_w, (CGFloat)win_h)];
      cv = [window->ns_window contentView];
      rc = macos_apply_layout_tree(cv, win_h, window->base.layout_root, 0, 0);
    }
    [NSApp activateIgnoringOtherApps:YES];
    [window->ns_window makeKeyAndOrderFront:nil];
  }
  return rc;
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
    g_macos_map_rc =
        (int)cmn_handle_map_remove((__bridge void *)(*window)->ns_window);
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
    case CMN_WIDGET_KIND_FLEX:
    case CMN_WIDGET_KIND_GRID:
      break;
    case CMN_WIDGET_KIND_PANEL:
      parent_view =
          (__bridge NSView *)((cmn_widget_header_t *)parent)->native_handle;
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
    case CMN_WIDGET_KIND_FLEX:
    case CMN_WIDGET_KIND_GRID:
      break;
    default:
      if (child_kind == CMN_WIDGET_KIND_WINDOW ||
          child_kind == CMN_WIDGET_KIND_MENU ||
          child_kind == CMN_WIDGET_KIND_FILEPICKER ||
          child_kind == CMN_WIDGET_KIND_COLORPICKER) {
        return CMN_ERR_INVAL;
      }
      child_view =
          (__bridge NSView *)((cmn_widget_header_t *)child)->native_handle;
      break;
    }

    if (parent_view == nil || child_view == nil) {
      return CMN_SUCCESS;
    }
    [child_view removeFromSuperview];
    if (is_stack_parent) {
      if (child_kind == CMN_WIDGET_KIND_TEXTBOX) {
        [child_view
            addConstraint:
                [NSLayoutConstraint
                    constraintWithItem:child_view
                             attribute:NSLayoutAttributeWidth
                             relatedBy:NSLayoutRelationGreaterThanOrEqual
                                toItem:nil
                             attribute:NSLayoutAttributeNotAnAttribute
                            multiplier:1.0
                              constant:280.0]];
      } else if (child_kind == CMN_WIDGET_KIND_TEXTVIEW) {
        [child_view
            addConstraint:
                [NSLayoutConstraint
                    constraintWithItem:child_view
                             attribute:NSLayoutAttributeWidth
                             relatedBy:NSLayoutRelationGreaterThanOrEqual
                                toItem:nil
                             attribute:NSLayoutAttributeNotAnAttribute
                            multiplier:1.0
                              constant:360.0]];
        [child_view
            addConstraint:
                [NSLayoutConstraint
                    constraintWithItem:child_view
                             attribute:NSLayoutAttributeHeight
                             relatedBy:NSLayoutRelationGreaterThanOrEqual
                                toItem:nil
                             attribute:NSLayoutAttributeNotAnAttribute
                            multiplier:1.0
                              constant:140.0]];
      }
      [(NSStackView *)parent_view addArrangedSubview:child_view];
    } else {
      [child_view setFrame:[parent_view bounds]];
      [child_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
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
    g_macos_map_rc =
        (int)cmn_handle_map_remove((__bridge void *)(*button)->ns_button);
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
    g_macos_map_rc =
        (int)cmn_handle_map_remove((__bridge void *)(*label)->ns_label);
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
 * @brief Configures whether a text box masks its characters as a password.
 * @param textbox The text box to modify.
 * @param is_password Non-zero to enable password masking, zero for plain text.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_password(cmn_textbox_t *textbox, int is_password) {
  if (textbox == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSTextField *old_field = textbox->ns_textbox;
    NSString *val = [old_field stringValue];
    NSView *superview = [old_field superview];
    NSTextField *new_field = nil;

    if (is_password) {
      new_field = [[NSSecureTextField alloc] initWithFrame:[old_field frame]];
    } else {
      new_field = [NSTextField textFieldWithString:@""];
      [new_field setFrame:[old_field frame]];
    }
    [new_field setBezeled:YES];
    [new_field setBezelStyle:NSTextFieldSquareBezel];
    [new_field setEditable:YES];
    [new_field setSelectable:YES];
    [new_field setStringValue:val ? val : @""];

    g_macos_map_rc = (int)cmn_handle_map_remove((__bridge void *)old_field);
    if (superview != nil) {
      if ([superview isKindOfClass:[NSStackView class]]) {
        NSStackView *stack = (NSStackView *)superview;
        NSArray<NSView *> *arranged = [stack arrangedSubviews];
        NSUInteger idx = [arranged indexOfObject:old_field];
        [old_field removeFromSuperview];
        [new_field addConstraint:
                       [NSLayoutConstraint
                           constraintWithItem:new_field
                                    attribute:NSLayoutAttributeWidth
                                    relatedBy:NSLayoutRelationGreaterThanOrEqual
                                       toItem:nil
                                    attribute:NSLayoutAttributeNotAnAttribute
                                   multiplier:1.0
                                     constant:280.0]];
        if (idx != NSNotFound) {
          [stack insertArrangedSubview:new_field atIndex:idx];
        } else {
          [stack addArrangedSubview:new_field];
        }
      } else {
        [old_field removeFromSuperview];
        [superview addSubview:new_field];
      }
    }
    textbox->ns_textbox = new_field;
    g_macos_map_rc =
        (int)cmn_handle_map_add((__bridge void *)new_field, textbox);
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
    g_macos_map_rc =
        (int)cmn_handle_map_remove((__bridge void *)(*textbox)->ns_textbox);
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
    [tv->ns_scrollview setBorderType:NSBezelBorder];

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
    [tv->ns_textview setTextColor:[NSColor labelColor]];

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
    NSDictionary *attrs = @{
      NSForegroundColorAttributeName : [NSColor labelColor],
      NSFontAttributeName : [NSFont systemFontOfSize:13.0]
    };
    NSAttributedString *attrString = [[NSAttributedString alloc]
        initWithString:[NSString stringWithUTF8String:text]
            attributes:attrs];
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
    g_macos_map_rc =
        (int)cmn_handle_map_remove((__bridge void *)(*textview)->ns_scrollview);
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
    NSView *content_view = [win->ns_window contentView];
    NSView *capture_view = [content_view superview];
    NSRect bounds;
    NSBitmapImageRep *rep;
    NSData *pngData;
    NSString *path;
    BOOL written;

    if (capture_view == nil) {
      capture_view = content_view;
    }
    [win->ns_window makeFirstResponder:nil];
    [win->ns_window displayIfNeeded];
    [capture_view layoutSubtreeIfNeeded];
    [capture_view displayIfNeeded];

    bounds = [capture_view bounds];
    rep = [capture_view bitmapImageRepForCachingDisplayInRect:bounds];
    [capture_view cacheDisplayInRect:bounds toBitmapImageRep:rep];
    pngData = [rep representationUsingType:NSBitmapImageFileTypePNG
                                properties:@{}];
    path = [NSString stringWithUTF8String:out_filepath];
    written = [pngData writeToFile:path atomically:YES];
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
    vbox->ns_stack =
        [[NSStackView alloc] initWithFrame:NSMakeRect(0, 0, 480, 340)];
    [vbox->ns_stack setOrientation:NSUserInterfaceLayoutOrientationVertical];
    [vbox->ns_stack setSpacing:12.0];
    [vbox->ns_stack setEdgeInsets:NSEdgeInsetsMake(20.0, 24.0, 20.0, 24.0)];
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
    g_macos_map_rc =
        (int)cmn_handle_map_remove((__bridge void *)(*vbox)->ns_stack);
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
    g_macos_map_rc =
        (int)cmn_handle_map_remove((__bridge void *)(*hbox)->ns_stack);
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
  if (sender == nil || sender != nil) {
    if (self.on_click) {
      return self.on_click(self.userdata);
    }
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
  if (menu == NULL || (window != NULL && window->ns_window == nil)) {
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

/**
 * @brief Custom AppKit view rendering 2D vector canvas, WebView, Tabs, and
 * SplitView widgets.
 */
@interface CmnCustomDrawView : NSView
/** @brief Associated polymorphic widget header. */
@property(nonatomic, assign) cmn_widget_header_t *hdr;
@end

@implementation CmnCustomDrawView
/**
 * @brief Draws the custom widget content inside the view bounds.
 * @param dirtyRect Region requiring redraw.
 */
- (void)drawRect:(NSRect)dirtyRect {
  NSRect b = [self bounds];
  if (dirtyRect.size.width < 0.0) {
    return;
  }
  if (self.hdr != NULL && self.hdr->kind == CMN_WIDGET_KIND_VIEW) {
    [[NSColor colorWithCalibratedRed:0.96 green:0.97 blue:0.99
                               alpha:1.0] setFill];
    NSRectFill(b);
    [[NSColor colorWithCalibratedRed:0.20 green:0.48 blue:0.86
                               alpha:1.0] setFill];
    NSRectFill(NSMakeRect(20, b.size.height - 64, 120, 44));
    [[NSColor colorWithCalibratedRed:0.16 green:0.65 blue:0.42
                               alpha:1.0] setFill];
    NSBezierPath *circle = [NSBezierPath
        bezierPathWithOvalInRect:NSMakeRect(160, b.size.height - 68, 52, 52)];
    [circle fill];
    [[NSColor colorWithCalibratedRed:0.88 green:0.32 blue:0.25
                               alpha:1.0] setStroke];
    NSBezierPath *line = [NSBezierPath bezierPath];
    [line setLineWidth:3.0];
    [line moveToPoint:NSMakePoint(230, b.size.height - 64)];
    [line lineToPoint:NSMakePoint(360, b.size.height - 20)];
    [line stroke];
    NSDictionary *attrs = @{
      NSFontAttributeName : [NSFont boldSystemFontOfSize:13.0],
      NSForegroundColorAttributeName : [NSColor labelColor]
    };
    [@"2D Vector Canvas (cmn_view_t)" drawAtPoint:NSMakePoint(20, 16)
                                   withAttributes:attrs];
    [[NSColor gridColor] setStroke];
    NSFrameRect(b);
  } else if (self.hdr != NULL && self.hdr->kind == CMN_WIDGET_KIND_WEBVIEW) {
    cmn_webview_t *wv = (cmn_webview_t *)self.hdr;
    [[NSColor whiteColor] setFill];
    NSRectFill(b);
    [[NSColor colorWithCalibratedWhite:0.93 alpha:1.0] setFill];
    NSRectFill(NSMakeRect(0, b.size.height - 28, b.size.width, 28));
    NSString *urlStr = (wv->url[0] != '\0')
                           ? [NSString stringWithUTF8String:wv->url]
                           : @"https://cmn.dev/docs";
    NSDictionary *barAttrs = @{
      NSFontAttributeName : [NSFont systemFontOfSize:11.0],
      NSForegroundColorAttributeName : [NSColor secondaryLabelColor]
    };
    [[NSString stringWithFormat:@"URL: %@", urlStr]
           drawAtPoint:NSMakePoint(10, b.size.height - 21)
        withAttributes:barAttrs];
    NSDictionary *titleAttrs = @{
      NSFontAttributeName : [NSFont boldSystemFontOfSize:15.0],
      NSForegroundColorAttributeName : [NSColor colorWithCalibratedRed:0.11
                                                                 green:0.32
                                                                  blue:0.62
                                                                 alpha:1.0]
    };
    [@"cmn Embedded WebView Document"
           drawAtPoint:NSMakePoint(14, b.size.height - 56)
        withAttributes:titleAttrs];
    NSDictionary *bodyAttrs = @{
      NSFontAttributeName : [NSFont systemFontOfSize:12.0],
      NSForegroundColorAttributeName : [NSColor darkGrayColor]
    };
    NSString *bodyStr = (wv->html[0] != '\0')
                            ? [NSString stringWithUTF8String:wv->html]
                            : @"Rendered HTML5 content inside native WebView.";
    [bodyStr
            drawInRect:NSMakeRect(14, 10, b.size.width - 28, b.size.height - 72)
        withAttributes:bodyAttrs];
    [[NSColor gridColor] setStroke];
    NSFrameRect(b);
  } else if (self.hdr != NULL && self.hdr->kind == CMN_WIDGET_KIND_TABS) {
    cmn_tabs_t *tb = (cmn_tabs_t *)self.hdr;
    uint32_t i;
    CGFloat x = 12.0;
    [[NSColor colorWithCalibratedWhite:0.96 alpha:1.0] setFill];
    NSRectFill(NSMakeRect(0, 0, b.size.width, b.size.height - 28));
    [[NSColor gridColor] setStroke];
    NSFrameRect(NSMakeRect(0, 0, b.size.width, b.size.height - 28));
    for (i = 0; i < tb->count; ++i) {
      BOOL sel = (i == tb->selected);
      NSString *t = [NSString stringWithUTF8String:tb->titles[i]];
      NSRect tabRect = NSMakeRect(x, b.size.height - 28, 96, 26);
      [(sel ? [NSColor whiteColor]
            : [NSColor colorWithCalibratedWhite:0.90 alpha:1.0]) setFill];
      NSRectFill(tabRect);
      [[NSColor gridColor] setStroke];
      NSFrameRect(tabRect);
      NSDictionary *tAttrs = @{
        NSFontAttributeName : sel ? [NSFont boldSystemFontOfSize:12.0]
                                  : [NSFont systemFontOfSize:12.0],
        NSForegroundColorAttributeName : sel
            ? [NSColor colorWithCalibratedRed:0.12
                                        green:0.38
                                         blue:0.78
                                        alpha:1.0]
            : [NSColor darkGrayColor]
      };
      [t drawAtPoint:NSMakePoint(x + 16, b.size.height - 22)
          withAttributes:tAttrs];
      x += 98.0;
    }
    NSDictionary *cAttrs = @{
      NSFontAttributeName : [NSFont systemFontOfSize:13.0],
      NSForegroundColorAttributeName : [NSColor labelColor]
    };
    [@"General Workspace Settings & Preferences"
           drawAtPoint:NSMakePoint(18, b.size.height - 68)
        withAttributes:cAttrs];
  } else if (self.hdr != NULL && self.hdr->kind == CMN_WIDGET_KIND_SPLITVIEW) {
    cmn_splitview_t *sv = (cmn_splitview_t *)self.hdr;
    CGFloat splitX = b.size.width * (sv->pos > 0.05 ? sv->pos : 0.45);
    [[NSColor colorWithCalibratedRed:0.97 green:0.98 blue:0.99
                               alpha:1.0] setFill];
    NSRectFill(NSMakeRect(0, 0, splitX - 2, b.size.height));
    [[NSColor colorWithCalibratedRed:0.94 green:0.96 blue:0.98
                               alpha:1.0] setFill];
    NSRectFill(
        NSMakeRect(splitX + 2, 0, b.size.width - splitX - 2, b.size.height));
    [[NSColor colorWithCalibratedWhite:0.72 alpha:1.0] setFill];
    NSRectFill(NSMakeRect(splitX - 2, 0, 4, b.size.height));
    NSDictionary *pAttrs = @{
      NSFontAttributeName : [NSFont boldSystemFontOfSize:12.5],
      NSForegroundColorAttributeName : [NSColor labelColor]
    };
    [@"Navigation Tree" drawAtPoint:NSMakePoint(14, b.size.height - 32)
                     withAttributes:pAttrs];
    [@"Detail Inspector Pane\nSplit Ratio: 45% / 55%"
            drawInRect:NSMakeRect(splitX + 16, 14, b.size.width - splitX - 28,
                                  b.size.height - 28)
        withAttributes:pAttrs];
    [[NSColor gridColor] setStroke];
    NSFrameRect(b);
  }
}
@end

/**
 * @brief Backend hook creating the native OS control for an extended widget.
 * @param hdr Initialized widget header.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_init(cmn_widget_header_t *hdr) {
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSView *view = nil;
    NSRect frame = NSMakeRect(0, 0, hdr->bounds.width, hdr->bounds.height);
    switch (hdr->kind) {
    case CMN_WIDGET_KIND_CHECKBOX: {
      NSButton *btn = [[NSButton alloc] initWithFrame:frame];
      [btn setButtonType:NSButtonTypeSwitch];
      [btn setTitle:@""];
      view = btn;
      break;
    }
    case CMN_WIDGET_KIND_RADIOBUTTON: {
      NSButton *btn = [[NSButton alloc] initWithFrame:frame];
      [btn setButtonType:NSButtonTypeRadio];
      [btn setTitle:@""];
      view = btn;
      break;
    }
    case CMN_WIDGET_KIND_TOGGLEBUTTON: {
      NSButton *btn = [[NSButton alloc] initWithFrame:frame];
      [btn setButtonType:NSButtonTypePushOnPushOff];
      [btn setBezelStyle:NSBezelStyleRounded];
      [btn setTitle:@""];
      view = btn;
      break;
    }
    case CMN_WIDGET_KIND_LINE: {
      NSBox *box = [[NSBox alloc] initWithFrame:frame];
      [box setBoxType:NSBoxSeparator];
      view = box;
      break;
    }
    case CMN_WIDGET_KIND_SLIDER: {
      NSSlider *slider = [[NSSlider alloc] initWithFrame:frame];
      [slider setMinValue:0.0];
      [slider setMaxValue:1.0];
      [slider setDoubleValue:0.0];
      view = slider;
      break;
    }
    case CMN_WIDGET_KIND_PROGRESS: {
      NSProgressIndicator *prog =
          [[NSProgressIndicator alloc] initWithFrame:frame];
      [prog setStyle:NSProgressIndicatorStyleBar];
      [prog setIndeterminate:NO];
      [prog setMinValue:0.0];
      [prog setMaxValue:1.0];
      [prog setDoubleValue:0.0];
      view = prog;
      break;
    }
    case CMN_WIDGET_KIND_UPDOWN: {
      NSStepper *stepper = [[NSStepper alloc] initWithFrame:frame];
      [stepper setMinValue:-1000000.0];
      [stepper setMaxValue:1000000.0];
      [stepper setIncrement:1.0];
      view = stepper;
      break;
    }
    case CMN_WIDGET_KIND_POPUP: {
      NSPopUpButton *popup = [[NSPopUpButton alloc] initWithFrame:frame
                                                        pullsDown:NO];
      view = popup;
      break;
    }
    case CMN_WIDGET_KIND_COMBO: {
      NSComboBox *combo = [[NSComboBox alloc] initWithFrame:frame];
      view = combo;
      break;
    }
    case CMN_WIDGET_KIND_LISTBOX:
    case CMN_WIDGET_KIND_TABLEVIEW: {
      NSScrollView *sv = [[NSScrollView alloc] initWithFrame:frame];
      NSTextView *tv = [[NSTextView alloc] initWithFrame:frame];
      [tv setEditable:NO];
      [tv setFont:[NSFont userFixedPitchFontOfSize:12.0]];
      [sv setDocumentView:tv];
      [sv setHasVerticalScroller:YES];
      [sv setBorderType:NSBezelBorder];
      view = sv;
      break;
    }
    case CMN_WIDGET_KIND_IMAGEVIEW: {
      NSImageView *iv = [[NSImageView alloc] initWithFrame:frame];
      [iv setImageFrameStyle:NSImageFramePhoto];
      [iv setImageScaling:NSImageScaleProportionallyUpOrDown];
      view = iv;
      break;
    }
    case CMN_WIDGET_KIND_VIEW:
    case CMN_WIDGET_KIND_WEBVIEW:
    case CMN_WIDGET_KIND_TABS:
    case CMN_WIDGET_KIND_SPLITVIEW: {
      CmnCustomDrawView *cv = [[CmnCustomDrawView alloc] initWithFrame:frame];
      cv.hdr = hdr;
      view = cv;
      break;
    }
    case CMN_WIDGET_KIND_PANEL: {
      NSBox *box = [[NSBox alloc] initWithFrame:frame];
      [box setTitle:@"Panel Container (cmn_panel_t)"];
      [box setBoxType:NSBoxPrimary];
      view = box;
      break;
    }
    default:
      break;
    }
    if (view != nil) {
      hdr->native_handle = (__bridge_retained void *)view;
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
  if (hdr->native_handle == NULL) {
    return CMN_SUCCESS;
  }
  @autoreleasepool {
    NSView *view = (__bridge NSView *)hdr->native_handle;
    switch (hdr->kind) {
    case CMN_WIDGET_KIND_CHECKBOX: {
      cmn_checkbox_t *cb = (cmn_checkbox_t *)hdr;
      NSButton *btn = (NSButton *)view;
      [btn setAllowsMixedState:(cb->is_tristate ? YES : NO)];
      [btn setTitle:[NSString stringWithUTF8String:cb->text]];
      [btn setState:(cb->state == CMN_CHECK_ON)
                        ? NSControlStateValueOn
                        : ((cb->state == CMN_CHECK_MIXED)
                               ? NSControlStateValueMixed
                               : NSControlStateValueOff)];
      break;
    }
    case CMN_WIDGET_KIND_RADIOBUTTON: {
      cmn_radiobutton_t *rb = (cmn_radiobutton_t *)hdr;
      NSButton *btn = (NSButton *)view;
      [btn setTitle:[NSString stringWithUTF8String:rb->text]];
      [btn setState:(rb->selected ? NSControlStateValueOn
                                  : NSControlStateValueOff)];
      break;
    }
    case CMN_WIDGET_KIND_TOGGLEBUTTON: {
      cmn_togglebutton_t *tb = (cmn_togglebutton_t *)hdr;
      NSButton *btn = (NSButton *)view;
      [btn setTitle:[NSString stringWithUTF8String:tb->text]];
      [btn setState:(tb->toggled ? NSControlStateValueOn
                                 : NSControlStateValueOff)];
      break;
    }
    case CMN_WIDGET_KIND_SLIDER: {
      cmn_slider_t *sl = (cmn_slider_t *)hdr;
      NSSlider *slider = (NSSlider *)view;
      [slider setDoubleValue:sl->value];
      [slider setNumberOfTickMarks:(NSInteger)sl->steps];
      break;
    }
    case CMN_WIDGET_KIND_PROGRESS: {
      cmn_progress_t *pr = (cmn_progress_t *)hdr;
      NSProgressIndicator *prog = (NSProgressIndicator *)view;
      [prog setIndeterminate:(pr->indeterminate ? YES : NO)];
      [prog setDoubleValue:pr->value];
      break;
    }
    case CMN_WIDGET_KIND_UPDOWN: {
      cmn_updown_t *ud = (cmn_updown_t *)hdr;
      NSStepper *stepper = (NSStepper *)view;
      [stepper setIncrement:ud->step];
      [stepper setDoubleValue:ud->value];
      break;
    }
    case CMN_WIDGET_KIND_POPUP: {
      cmn_popup_t *pu = (cmn_popup_t *)hdr;
      NSPopUpButton *popup = (NSPopUpButton *)view;
      uint32_t i;
      [popup removeAllItems];
      for (i = 0; i < pu->count; ++i) {
        [popup addItemWithTitle:[NSString stringWithUTF8String:pu->items[i]]];
      }
      if (pu->selected < pu->count) {
        [popup selectItemAtIndex:(NSInteger)pu->selected];
      }
      break;
    }
    case CMN_WIDGET_KIND_COMBO: {
      cmn_combo_t *co = (cmn_combo_t *)hdr;
      NSComboBox *combo = (NSComboBox *)view;
      uint32_t i;
      [combo removeAllItems];
      for (i = 0; i < co->count; ++i) {
        [combo addItemWithObjectValue:[NSString
                                          stringWithUTF8String:co->items[i]]];
      }
      [combo setStringValue:[NSString stringWithUTF8String:co->text]];
      break;
    }
    case CMN_WIDGET_KIND_LISTBOX: {
      cmn_listbox_t *lb = (cmn_listbox_t *)hdr;
      NSScrollView *sv = (NSScrollView *)view;
      NSTextView *tv = (NSTextView *)[sv documentView];
      NSMutableString *ms = [NSMutableString string];
      uint32_t i;
      for (i = 0; i < lb->count; ++i) {
        NSString *prefix = @"  ";
        if (lb->checkable) {
          prefix = lb->checked[i] ? @"[x] " : @"[ ] ";
        } else if (i == lb->selected) {
          prefix = @"> ";
        }
        [ms appendFormat:@"%@%s\n", prefix, lb->items[i]];
      }
      [tv setString:ms];
      break;
    }
    case CMN_WIDGET_KIND_TABLEVIEW: {
      cmn_tableview_t *tbl = (cmn_tableview_t *)hdr;
      NSScrollView *sv = (NSScrollView *)view;
      NSTextView *tv = (NSTextView *)[sv documentView];
      NSMutableString *ms = [NSMutableString string];
      uint32_t r, c;
      for (c = 0; c < tbl->col_count; ++c) {
        [ms appendFormat:@"%-16s", tbl->col_titles[c]];
      }
      [ms appendString:@"\n------------------------------------------------\n"];
      for (r = 0; r < tbl->row_count; ++r) {
        [ms appendString:(r == tbl->selected_row) ? @"> " : @"  "];
        for (c = 0; c < tbl->col_count; ++c) {
          [ms appendFormat:@"%-16s", tbl->cells[r][c]];
        }
        [ms appendString:@"\n"];
      }
      [tv setString:ms];
      break;
    }
    case CMN_WIDGET_KIND_IMAGEVIEW: {
      cmn_imageview_t *iv_model = (cmn_imageview_t *)hdr;
      NSImageView *iv = (NSImageView *)view;
      NSImage *img = nil;
      if (iv_model->image_path[0] != '\0') {
        img = [[NSImage alloc]
            initWithContentsOfFile:[NSString stringWithUTF8String:iv_model->
                                                                  image_path]];
      }
      if (img == nil) {
        NSSize sz = NSMakeSize(160, 100);
        img = [[NSImage alloc] initWithSize:sz];
        [img lockFocus];
        [[NSColor colorWithCalibratedRed:0.18 green:0.44 blue:0.78
                                   alpha:1.0] setFill];
        NSRectFill(NSMakeRect(0, 0, sz.width, sz.height));
        [[NSColor colorWithCalibratedRed:0.95 green:0.76 blue:0.22
                                   alpha:1.0] setFill];
        [[NSBezierPath bezierPathWithOvalInRect:NSMakeRect(110, 56, 30, 30)]
            fill];
        [[NSColor colorWithCalibratedRed:0.22 green:0.66 blue:0.45
                                   alpha:1.0] setFill];
        NSRectFill(NSMakeRect(0, 0, sz.width, 32));
        [img unlockFocus];
      }
      [iv setImage:img];
      break;
    }
    default:
      [view setNeedsDisplay:YES];
      break;
    }
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
  if (hdr->native_handle != NULL) {
    @autoreleasepool {
      NSView *view = (__bridge_transfer NSView *)hdr->native_handle;
      [view removeFromSuperview];
      hdr->native_handle = NULL;
    }
  }
  return CMN_SUCCESS;
}
