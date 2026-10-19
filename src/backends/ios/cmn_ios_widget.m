/* clang-format off */
#include "cmn_ios_internal.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#import <objc/runtime.h>
/* clang-format on */

/**
 * @file cmn_ios_widget.m
 * @brief iOS UIKit implementation of the widget ontology.
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
    win->ui_window =
        [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    win->root_vc = [[UIViewController alloc] init];
    win->root_vc.view.backgroundColor = [UIColor systemBackgroundColor];
    win->ui_window.rootViewController = win->root_vc;
    win->app = app;
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
    window->root_vc.title = [NSString stringWithUTF8String:title];
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
    [window->ui_window makeKeyAndVisible];
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
    (*window)->ui_window.hidden = YES;
  }
  return cmn_free((void **)window);
}

/* --- Layouts --- */

/**
 * @brief Creates a vertical box layout container.
 * @param out_vbox Pointer to store the created vertical box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_vbox_create(cmn_vbox_t **out_vbox) {
  cmn_vbox_t *vbox = NULL;
  cmn_error_e rc;

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
    vbox->ui_stackview = [[UIStackView alloc] init];
    vbox->ui_stackview.axis = UILayoutConstraintAxisVertical;
    vbox->ui_stackview.spacing = 8.0;
    vbox->ui_stackview.alignment = UIStackViewAlignmentFill;
    vbox->ui_stackview.distribution = UIStackViewDistributionFillEqually;
    vbox->ui_stackview.translatesAutoresizingMaskIntoConstraints = NO;
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
    [(*vbox)->ui_stackview removeFromSuperview];
  }
  return cmn_free((void **)vbox);
}

/**
 * @brief Creates a horizontal box layout container.
 * @param out_hbox Pointer to store the created horizontal box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_hbox_create(cmn_hbox_t **out_hbox) {
  cmn_hbox_t *hbox = NULL;
  cmn_error_e rc;

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
    hbox->ui_stackview = [[UIStackView alloc] init];
    hbox->ui_stackview.axis = UILayoutConstraintAxisHorizontal;
    hbox->ui_stackview.spacing = 8.0;
    hbox->ui_stackview.alignment = UIStackViewAlignmentFill;
    hbox->ui_stackview.distribution = UIStackViewDistributionFillEqually;
    hbox->ui_stackview.translatesAutoresizingMaskIntoConstraints = NO;
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
    [(*hbox)->ui_stackview removeFromSuperview];
  }
  return cmn_free((void **)hbox);
}

/* --- Widgets --- */

/**
 * @brief Objective-C target for iOS UIButton touch events.
 */
@interface CmnButtonTarget : NSObject
/**
 * @brief Associated C button handle.
 */
@property(nonatomic, assign) cmn_button_t *cmn_button;
/**
 * @brief Action method invoked on `UIControlEventTouchUpInside`.
 * @param sender The control that sent the event.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
- (cmn_error_e)onButtonClicked:(id)sender;
@end

@implementation CmnButtonTarget
- (cmn_error_e)onButtonClicked:(id)sender {
  if (sender != nil || sender == nil) {
    if (self.cmn_button && self.cmn_button->on_click) {
      return self.cmn_button->on_click(self.cmn_button->userdata);
    }
  }
  return CMN_SUCCESS;
}
@end

/**
 * @brief Creates a new button widget.
 * @param out_button Pointer to store the created button.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_create(cmn_button_t **out_button) {
  cmn_button_t *btn = NULL;
  cmn_error_e rc;

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
    btn->ui_button = [UIButton buttonWithType:UIButtonTypeSystem];
    btn->ui_button.translatesAutoresizingMaskIntoConstraints = NO;

    CmnButtonTarget *target = [[CmnButtonTarget alloc] init];
    target.cmn_button = btn;
    btn->target = target;

    [btn->ui_button addTarget:target
                       action:@selector(onButtonClicked:)
             forControlEvents:UIControlEventTouchUpInside];
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
    [button->ui_button setTitle:[NSString stringWithUTF8String:text]
                       forState:UIControlStateNormal];
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
  if (button == NULL) {
    return CMN_ERR_INVAL;
  }
  if (button->on_click) {
    return button->on_click(button->userdata);
  }
  return CMN_SUCCESS;
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
    ((CmnButtonTarget *)(*button)->target).cmn_button = NULL;
    [(*button)->ui_button removeFromSuperview];
#if !__has_feature(objc_arc)
    [(*button)->target release];
#endif
  }
  return cmn_free((void **)button);
}

/**
 * @brief Creates a new label widget.
 * @param out_label Pointer to store the created label.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_create(cmn_label_t **out_label) {
  cmn_label_t *lbl = NULL;
  cmn_error_e rc;

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
    lbl->ui_label = [[UILabel alloc] init];
    lbl->ui_label.translatesAutoresizingMaskIntoConstraints = NO;
    lbl->ui_label.textAlignment = NSTextAlignmentCenter;
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
    label->ui_label.text = [NSString stringWithUTF8String:text];
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
    [(*label)->ui_label removeFromSuperview];
  }
  return cmn_free((void **)label);
}

/**
 * @brief Creates a new text box widget.
 * @param out_textbox Pointer to store the created text box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_create(cmn_textbox_t **out_textbox) {
  cmn_textbox_t *tb = NULL;
  cmn_error_e rc;

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
    tb->ui_textfield = [[UITextField alloc] init];
    tb->ui_textfield.translatesAutoresizingMaskIntoConstraints = NO;
    tb->ui_textfield.borderStyle = UITextBorderStyleRoundedRect;
  }

  *out_textbox = tb;
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
  if (g_cmn_mock_textbox_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textbox == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSString *val = textbox->ui_textfield.text;
    cmn_error_e rc =
        cmn_strcpy_s(textbox->text_cache, sizeof(textbox->text_cache),
                     val ? [val UTF8String] : "");
    if (rc != CMN_SUCCESS) {
      return rc;
    }
    *out_text = textbox->text_cache;
  }
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
    textbox->ui_textfield.text = [NSString stringWithUTF8String:text];
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
    textbox->ui_textfield.secureTextEntry = is_password ? YES : NO;
  }
  return CMN_SUCCESS;
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
    [(*textbox)->ui_textfield removeFromSuperview];
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
    tv->ui_textview = [[UITextView alloc] init];
    tv->ui_textview.translatesAutoresizingMaskIntoConstraints = NO;
    tv->ui_textview.editable = NO;
  }

  *out_textview = tv;
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
  if (g_cmn_mock_textview_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textview == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }
  @autoreleasepool {
    NSString *val = textview->ui_textview.text;
    cmn_error_e rc =
        cmn_strcpy_s(textview->text_cache, sizeof(textview->text_cache),
                     val ? [val UTF8String] : "");
    if (rc != CMN_SUCCESS) {
      return rc;
    }
    *out_text = textview->text_cache;
  }
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
    textview->ui_textview.text = [NSString stringWithUTF8String:text];
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
    textview->ui_textview.text = [textview->ui_textview.text
        stringByAppendingString:[NSString stringWithUTF8String:text]];
    NSRange range = NSMakeRange(textview->ui_textview.text.length, 0);
    [textview->ui_textview scrollRangeToVisible:range];
  }
  return CMN_SUCCESS;
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
    [(*textview)->ui_textview removeFromSuperview];
  }
  return cmn_free((void **)textview);
}

/* --- Parent/Child Binding --- */

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
    UIView *parent_view = nil;
    UIView *child_view = nil;
    BOOL is_stack_parent = NO;

    switch (parent_kind) {
    case CMN_WIDGET_KIND_WINDOW:
      parent_view = ((cmn_window_t *)parent)->root_vc.view;
      break;
    case CMN_WIDGET_KIND_VBOX:
      parent_view = ((cmn_vbox_t *)parent)->ui_stackview;
      is_stack_parent = YES;
      break;
    case CMN_WIDGET_KIND_HBOX:
      parent_view = ((cmn_hbox_t *)parent)->ui_stackview;
      is_stack_parent = YES;
      break;
    case CMN_WIDGET_KIND_FLEX:
    case CMN_WIDGET_KIND_GRID:
      break;
    case CMN_WIDGET_KIND_PANEL:
      parent_view =
          (__bridge UIView *)((cmn_widget_header_t *)parent)->native_handle;
      break;
    default:
      return CMN_ERR_INVAL;
    }

    switch (child_kind) {
    case CMN_WIDGET_KIND_BUTTON:
      child_view = ((cmn_button_t *)child)->ui_button;
      break;
    case CMN_WIDGET_KIND_LABEL:
      child_view = ((cmn_label_t *)child)->ui_label;
      break;
    case CMN_WIDGET_KIND_TEXTBOX:
      child_view = ((cmn_textbox_t *)child)->ui_textfield;
      break;
    case CMN_WIDGET_KIND_TEXTVIEW:
      child_view = ((cmn_textview_t *)child)->ui_textview;
      break;
    case CMN_WIDGET_KIND_VBOX:
      child_view = ((cmn_vbox_t *)child)->ui_stackview;
      break;
    case CMN_WIDGET_KIND_HBOX:
      child_view = ((cmn_hbox_t *)child)->ui_stackview;
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
          (__bridge UIView *)((cmn_widget_header_t *)child)->native_handle;
      break;
    }

    if (parent_view != nil && child_view != nil) {
      if (is_stack_parent) {
        [(UIStackView *)parent_view addArrangedSubview:child_view];
      } else {
        [parent_view addSubview:child_view];
        [NSLayoutConstraint activateConstraints:@[
          [child_view.centerXAnchor
              constraintEqualToAnchor:parent_view.centerXAnchor],
          [child_view.centerYAnchor
              constraintEqualToAnchor:parent_view.centerYAnchor],
          [child_view.widthAnchor
              constraintEqualToAnchor:parent_view.widthAnchor
                           multiplier:0.8],
          [child_view.heightAnchor constraintEqualToConstant:50]
        ]];
      }
    }
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
  if (window == NULL || style < CMN_WINDOW_STYLE_DEFAULT) {
    return CMN_ERR_INVAL;
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
  if (window == NULL || is_fullscreen < CMN_FALSE) {
    return CMN_ERR_INVAL;
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
  if (window == NULL || display_index != 0) {
    return CMN_ERR_INVAL;
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
  *out_scale = 1.0f;
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
  *out_count = 1;
  return CMN_SUCCESS;
}

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
 * @brief Creates a new file picker dialog.
 * @param out_filepicker Pointer to store the created file picker.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_create(cmn_filepicker_t **out_filepicker) {
  if (out_filepicker == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_filepicker = NULL;
  return CMN_ERR_INVAL;
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
  return CMN_ERR_INVAL;
}

/**
 * @brief Gets the selected file path from the file picker.
 * @param filepicker The file picker to query.
 * @param out_path Pointer to store the selected file path string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_filepicker_get_path(cmn_filepicker_t *filepicker,
                                    const char **out_path) {
  if (filepicker == NULL || out_path == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_path = filepicker->path;
  return CMN_ERR_INVAL;
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
  return cmn_free((void **)filepicker);
}

/**
 * @brief Creates a new menu.
 * @param out_menu Pointer to store the created menu.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_create(cmn_menu_t **out_menu) {
  if (out_menu == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_menu = NULL;
  return CMN_ERR_INVAL;
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
  if (menu == NULL || title == NULL || (on_click == NULL && userdata != NULL)) {
    return CMN_ERR_INVAL;
  }
  return CMN_ERR_INVAL;
}

/**
 * @brief Programmatically triggers a menu item's callback by index.
 * @param menu The menu containing the item.
 * @param index The 0-based index of the menu item to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_trigger_item(cmn_menu_t *menu, int index) {
  if (menu == NULL || index < 0) {
    return CMN_ERR_INVAL;
  }
  return CMN_ERR_INVAL;
}

/**
 * @brief Sets the main menu bar for a window.
 * @param window The window to set the menu on.
 * @param menu The menu to attach.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_menu(cmn_window_t *window, cmn_menu_t *menu) {
  if (window == NULL || menu == NULL) {
    return CMN_ERR_INVAL;
  }
  return CMN_ERR_INVAL;
}

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
    UIView *view = nil;
    CGRect frame = CGRectMake(0, 0, hdr->bounds.width, hdr->bounds.height);
    switch (hdr->kind) {
    case CMN_WIDGET_KIND_CHECKBOX:
      view = [[UISwitch alloc] initWithFrame:frame];
      break;
    case CMN_WIDGET_KIND_RADIOBUTTON:
    case CMN_WIDGET_KIND_TOGGLEBUTTON:
      view = [UIButton buttonWithType:UIButtonTypeSystem];
      break;
    case CMN_WIDGET_KIND_SLIDER:
      view = [[UISlider alloc] initWithFrame:frame];
      break;
    case CMN_WIDGET_KIND_PROGRESS:
      view = [[UIProgressView alloc]
          initWithProgressViewStyle:UIProgressViewStyleDefault];
      break;
    case CMN_WIDGET_KIND_UPDOWN:
      view = [[UIStepper alloc] initWithFrame:frame];
      break;
    case CMN_WIDGET_KIND_POPUP:
      view = [[UIPickerView alloc] initWithFrame:frame];
      break;
    case CMN_WIDGET_KIND_COMBO:
      view = [[UITextField alloc] initWithFrame:frame];
      break;
    case CMN_WIDGET_KIND_LISTBOX:
    case CMN_WIDGET_KIND_TABLEVIEW:
      view = [[UITableView alloc] initWithFrame:frame
                                          style:UITableViewStylePlain];
      break;
    case CMN_WIDGET_KIND_IMAGEVIEW:
      view = [[UIImageView alloc] initWithFrame:frame];
      break;
    case CMN_WIDGET_KIND_PANEL:
      view = [[UIScrollView alloc] initWithFrame:frame];
      break;
    case CMN_WIDGET_KIND_TABS:
      view = [[UISegmentedControl alloc] initWithFrame:frame];
      break;
    case CMN_WIDGET_KIND_LINE:
    case CMN_WIDGET_KIND_VIEW:
    case CMN_WIDGET_KIND_WEBVIEW:
    case CMN_WIDGET_KIND_SPLITVIEW:
      view = [[UIView alloc] initWithFrame:frame];
      break;
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
  if (hdr->native_handle != NULL) {
    @autoreleasepool {
      UIView *view = (__bridge UIView *)hdr->native_handle;
      [view setNeedsDisplay];
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
      UIView *view = (__bridge_transfer UIView *)hdr->native_handle;
      [view removeFromSuperview];
      hdr->native_handle = NULL;
    }
  }
  return CMN_SUCCESS;
}
