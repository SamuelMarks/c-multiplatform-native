#ifndef CMN_TYPES_H
#define CMN_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#if defined(_MSC_VER) && (_MSC_VER < 1600)
    /* MSVC 2005 does not have stdint.h or stdbool.h */
    typedef signed __int8 int8_t;
    typedef unsigned __int8 uint8_t;
    typedef signed __int16 int16_t;
    typedef unsigned __int16 uint16_t;
    typedef signed __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef signed __int64 int64_t;
    typedef unsigned __int64 uint64_t;
#else
    #include <stdint.h>
#endif

#if !defined(__cplusplus)
    #ifndef _STDBOOL_H
        /** @brief Standard boolean header guard macro. */
        #define _STDBOOL_H
        #ifndef __STDBOOL_H
            /** @brief Alternate standard boolean header guard macro. */
            #define __STDBOOL_H
        #endif
        #ifndef __bool_true_false_are_defined
            /** @brief C99 stdbool indicator macro. */
            #define __bool_true_false_are_defined 1
        #endif
        /** @brief Boolean type for strict C89 compatibility across all translation units. */
        typedef unsigned char bool;
        /** @brief Boolean true constant. */
        #define true 1
        /** @brief Boolean false constant. */
        #define false 0
    #endif
#endif

#include <stddef.h>
/* clang-format on */

/**
 * @file types.h
 * @brief Standard integer, boolean, and opaque handle types for C89
 * compatibility.
 */

/**
 * @defgroup cmn_types Core Types
 * @brief Core Types module.
 * @{
 */

#if defined(_MSC_VER)
/** @brief Format specifier for signed 64-bit integers (`int64_t`). */
#define CMN_INT64_FMT "%I64d"
/** @brief Format specifier for unsigned 64-bit integers (`uint64_t`). */
#define CMN_UINT64_FMT "%I64u"
/** @brief Format specifier for `size_t`. */
#define CMN_SIZE_FMT "%Iu"
#elif defined(__LP64__) || defined(_LP64)
/** @brief Format specifier for signed 64-bit integers (`int64_t`). */
#define CMN_INT64_FMT "%ld"
/** @brief Format specifier for unsigned 64-bit integers (`uint64_t`). */
#define CMN_UINT64_FMT "%lu"
/** @brief Format specifier for `size_t`. */
#define CMN_SIZE_FMT "%lu"
#else
/** @brief Format specifier for signed 64-bit integers (`int64_t`). */
#define CMN_INT64_FMT "%lld"
/** @brief Format specifier for unsigned 64-bit integers (`uint64_t`). */
#define CMN_UINT64_FMT "%llu"
/** @brief Format specifier for `size_t`. */
#define CMN_SIZE_FMT "%lu"
#endif

/** @brief Format specifier for signed 32-bit integers (`int32_t` / `int`). */
#define CMN_INT32_FMT "%d"
/** @brief Format specifier for unsigned 32-bit integers (`uint32_t` / `unsigned
 * int`). */
#define CMN_UINT32_FMT "%u"

/**
 * @brief Strongly-typed C89 boolean enumeration unaffected by `<stdbool.h>`.
 */
typedef enum cmn_bool_e {
  /** @brief Logical false (0). */
  CMN_FALSE = 0,
  /** @brief Logical true (1). */
  CMN_TRUE = 1
} cmn_bool_t;

/**
 * @brief Discriminant enumeration identifying concrete widget and container
 * kinds.
 */
typedef enum cmn_widget_kind_e {
  /** @brief Uninitialized or invalid widget kind. */
  CMN_WIDGET_KIND_NONE = 0,
  /** @brief Native top-level window handle (`cmn_window_t`). */
  CMN_WIDGET_KIND_WINDOW = 1,
  /** @brief Vertical box layout container (`cmn_vbox_t`). */
  CMN_WIDGET_KIND_VBOX = 2,
  /** @brief Horizontal box layout container (`cmn_hbox_t`). */
  CMN_WIDGET_KIND_HBOX = 3,
  /** @brief Push button control (`cmn_button_t`). */
  CMN_WIDGET_KIND_BUTTON = 4,
  /** @brief Static text label control (`cmn_label_t`). */
  CMN_WIDGET_KIND_LABEL = 5,
  /** @brief Single-line editable text box (`cmn_textbox_t`). */
  CMN_WIDGET_KIND_TEXTBOX = 6,
  /** @brief Multi-line scrollable text view (`cmn_textview_t`). */
  CMN_WIDGET_KIND_TEXTVIEW = 7,
  /** @brief Native menu bar / popup menu (`cmn_menu_t`). */
  CMN_WIDGET_KIND_MENU = 8,
  /** @brief Native file picker dialog (`cmn_filepicker_t`). */
  CMN_WIDGET_KIND_FILEPICKER = 9,
  /** @brief Responsive CSS Flexbox-inspired layout container (`cmn_flex_t`). */
  CMN_WIDGET_KIND_FLEX = 10,
  /** @brief Responsive CSS Grid-inspired layout container (`cmn_grid_t`). */
  CMN_WIDGET_KIND_GRID = 11,
  /** @brief Native checkbox or tri-state checkbox (`cmn_checkbox_t`). */
  CMN_WIDGET_KIND_CHECKBOX = 12,
  /** @brief Native radio button (`cmn_radiobutton_t`). */
  CMN_WIDGET_KIND_RADIOBUTTON = 13,
  /** @brief Native flat or toggle button (`cmn_togglebutton_t`). */
  CMN_WIDGET_KIND_TOGGLEBUTTON = 14,
  /** @brief Native horizontal or vertical separator line (`cmn_line_t`). */
  CMN_WIDGET_KIND_LINE = 15,
  /** @brief Native horizontal or vertical slider (`cmn_slider_t`). */
  CMN_WIDGET_KIND_SLIDER = 16,
  /** @brief Native progress bar (`cmn_progress_t`). */
  CMN_WIDGET_KIND_PROGRESS = 17,
  /** @brief Native stepper / up-down spin control (`cmn_updown_t`). */
  CMN_WIDGET_KIND_UPDOWN = 18,
  /** @brief Native drop-down pop-up selector (`cmn_popup_t`). */
  CMN_WIDGET_KIND_POPUP = 19,
  /** @brief Native editable combo box (`cmn_combo_t`). */
  CMN_WIDGET_KIND_COMBO = 20,
  /** @brief Native scrollable list box (`cmn_listbox_t`). */
  CMN_WIDGET_KIND_LISTBOX = 21,
  /** @brief Native image view control (`cmn_imageview_t`). */
  CMN_WIDGET_KIND_IMAGEVIEW = 22,
  /** @brief Native multi-column table view (`cmn_tableview_t`). */
  CMN_WIDGET_KIND_TABLEVIEW = 23,
  /** @brief Native custom 2D drawing view (`cmn_view_t`). */
  CMN_WIDGET_KIND_VIEW = 24,
  /** @brief Native embedded web browser view (`cmn_webview_t`). */
  CMN_WIDGET_KIND_WEBVIEW = 25,
  /** @brief Native grouping or scrollable panel container (`cmn_panel_t`). */
  CMN_WIDGET_KIND_PANEL = 26,
  /** @brief Native tabbed container (`cmn_tabs_t`). */
  CMN_WIDGET_KIND_TABS = 27,
  /** @brief Native horizontal or vertical split view (`cmn_splitview_t`). */
  CMN_WIDGET_KIND_SPLITVIEW = 28,
  /** @brief Native color picker dialog (`cmn_colorpicker_t`). */
  CMN_WIDGET_KIND_COLORPICKER = 29
} cmn_widget_kind_e;

/**
 * @brief Tri-state check enumeration (`CMN_CHECK_OFF`, `CMN_CHECK_ON`,
 * `CMN_CHECK_MIXED`).
 */
typedef enum cmn_check_state_e {
  /** @brief Unchecked / off state (`0`). */
  CMN_CHECK_OFF = 0,
  /** @brief Checked / on state (`1`). */
  CMN_CHECK_ON = 1,
  /** @brief Indeterminate / mixed state (`2`). */
  CMN_CHECK_MIXED = 2
} cmn_check_state_e;

/**
 * @brief Image scaling modes.
 */
typedef enum cmn_scale_mode_e {
  /** @brief Automatic natural size scaling. */
  CMN_SCALE_AUTO = 0,
  /** @brief No scaling, unscaled centered image. */
  CMN_SCALE_NONE = 1,
  /** @brief Stretch to fill widget bounds. */
  CMN_SCALE_ASPECT = 2,
  /** @brief Uniformly fit preserving aspect ratio. */
  CMN_SCALE_FIT = 3
} cmn_scale_mode_e;

/**
 * @brief Mouse cursor shapes.
 */
typedef enum cmn_cursor_e {
  /** @brief Standard arrow pointer cursor. */
  CMN_CURSOR_ARROW = 0,
  /** @brief Pointing hand cursor. */
  CMN_CURSOR_HAND = 1,
  /** @brief Text selection I-beam cursor. */
  CMN_CURSOR_IBEAM = 2,
  /** @brief Crosshair precision cursor. */
  CMN_CURSOR_CROSS = 3,
  /** @brief Horizontal resize cursor. */
  CMN_CURSOR_SIZEWE = 4,
  /** @brief Vertical resize cursor. */
  CMN_CURSOR_SIZENS = 5
} cmn_cursor_e;

/**
 * @brief File picker dialog operation modes.
 */
typedef enum cmn_filepicker_mode_e {
  /** @brief Select an existing file to open. */
  CMN_FILEPICKER_OPEN_FILE = 0,
  /** @brief Choose a destination path to save a file. */
  CMN_FILEPICKER_SAVE_FILE = 1,
  /** @brief Select an existing directory folder. */
  CMN_FILEPICKER_SELECT_DIR = 2
} cmn_filepicker_mode_e;

/**
 * @brief Horizontal text alignment modes.
 */
typedef enum cmn_text_align_e {
  /** @brief Left-aligned text. */
  CMN_TEXT_ALIGN_LEFT = 0,
  /** @brief Center-aligned text. */
  CMN_TEXT_ALIGN_CENTER = 1,
  /** @brief Right-aligned text. */
  CMN_TEXT_ALIGN_RIGHT = 2,
  /** @brief Justified text. */
  CMN_TEXT_ALIGN_JUSTIFY = 3
} cmn_text_align_e;

/**
 * @brief Clipboard operations for editable controls.
 */
typedef enum cmn_clipboard_op_e {
  /** @brief Copy selection to clipboard. */
  CMN_CLIPBOARD_COPY = 0,
  /** @brief Cut selection to clipboard. */
  CMN_CLIPBOARD_CUT = 1,
  /** @brief Paste clipboard contents. */
  CMN_CLIPBOARD_PASTE = 2
} cmn_clipboard_op_e;

/**
 * @brief Font style flags.
 */
typedef enum cmn_font_style_e {
  /** @brief Regular upright font style. */
  CMN_FONT_STYLE_NORMAL = 0,
  /** @brief Bold weight flag. */
  CMN_FONT_STYLE_BOLD = 1,
  /** @brief Italic slant flag. */
  CMN_FONT_STYLE_ITALIC = 2,
  /** @brief Underline decoration flag. */
  CMN_FONT_STYLE_UNDERLINE = 4,
  /** @brief Strikeout decoration flag. */
  CMN_FONT_STYLE_STRIKEOUT = 8
} cmn_font_style_e;

/** @brief Opaque base handle for any UI widget. */
typedef struct cmn_widget_s cmn_widget_t;
/** @brief Opaque base handle for any UI container (window, vbox, hbox, flex,
 * grid, panel). */
typedef struct cmn_container_s cmn_container_t;
/** @brief Opaque handle for an application instance. */
typedef struct cmn_app_s cmn_app_t;
/** @brief Opaque handle for a window. */
typedef struct cmn_window_s cmn_window_t;
/** @brief Opaque handle for a button widget. */
typedef struct cmn_button_s cmn_button_t;
/** @brief Opaque handle for a checkbox or tri-state checkbox widget. */
typedef struct cmn_checkbox_s cmn_checkbox_t;
/** @brief Opaque handle for a radio button widget. */
typedef struct cmn_radiobutton_s cmn_radiobutton_t;
/** @brief Opaque handle for a flat or toggle button widget. */
typedef struct cmn_togglebutton_s cmn_togglebutton_t;
/** @brief Opaque handle for a label widget. */
typedef struct cmn_label_s cmn_label_t;
/** @brief Opaque handle for a separator line widget. */
typedef struct cmn_line_s cmn_line_t;
/** @brief Opaque handle for a slider widget. */
typedef struct cmn_slider_s cmn_slider_t;
/** @brief Opaque handle for a progress bar widget. */
typedef struct cmn_progress_s cmn_progress_t;
/** @brief Opaque handle for a stepper / up-down widget. */
typedef struct cmn_updown_s cmn_updown_t;
/** @brief Opaque handle for a text box widget. */
typedef struct cmn_textbox_s cmn_textbox_t;
/** @brief Opaque handle for a multi-line scrollable text view widget. */
typedef struct cmn_textview_s cmn_textview_t;
/** @brief Opaque handle for a pop-up drop-down selector widget. */
typedef struct cmn_popup_s cmn_popup_t;
/** @brief Opaque handle for an editable combo box widget. */
typedef struct cmn_combo_s cmn_combo_t;
/** @brief Opaque handle for a scrollable list box widget. */
typedef struct cmn_listbox_s cmn_listbox_t;
/** @brief Opaque handle for an image view widget. */
typedef struct cmn_imageview_s cmn_imageview_t;
/** @brief Opaque handle for a multi-column table view widget. */
typedef struct cmn_tableview_s cmn_tableview_t;
/** @brief Opaque handle for a custom 2D drawing view widget. */
typedef struct cmn_view_s cmn_view_t;
/** @brief Opaque handle for an embedded web browser view widget. */
typedef struct cmn_webview_s cmn_webview_t;
/** @brief Opaque handle for a grouping or scrollable panel container. */
typedef struct cmn_panel_s cmn_panel_t;
/** @brief Opaque handle for a tabbed container widget. */
typedef struct cmn_tabs_s cmn_tabs_t;
/** @brief Opaque handle for a split view container widget. */
typedef struct cmn_splitview_s cmn_splitview_t;
/** @brief Opaque handle for a color picker dialog. */
typedef struct cmn_colorpicker_s cmn_colorpicker_t;
/** @brief Opaque handle for a vertical box layout. */
typedef struct cmn_vbox_s cmn_vbox_t;
/** @brief Opaque handle for a horizontal box layout. */
typedef struct cmn_hbox_s cmn_hbox_t;
/** @brief Opaque handle for a responsive flexbox layout container. */
typedef struct cmn_flex_s cmn_flex_t;
/** @brief Opaque handle for a responsive grid layout container. */
typedef struct cmn_grid_s cmn_grid_t;
/** @brief Opaque handle for a reactive signal. */
typedef struct cmn_signal_s cmn_signal_t;
/** @brief Opaque handle for a computed signal. */
typedef struct cmn_computed_s cmn_computed_t;
/** @brief Opaque handle for a reactive effect. */
typedef struct cmn_effect_s cmn_effect_t;
/** @brief Opaque handle for a form. */
typedef struct cmn_form_s cmn_form_t;
/** @brief Opaque handle for a wizard form. */
typedef struct cmn_wizard_s cmn_wizard_t;
/** @brief Opaque handle for a router. */
typedef struct cmn_router_s cmn_router_t;

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_TYPES_H */
