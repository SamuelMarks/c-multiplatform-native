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
  CMN_WIDGET_KIND_GRID = 11
} cmn_widget_kind_e;

/** @brief Opaque base handle for any UI widget. */
typedef struct cmn_widget_s cmn_widget_t;
/** @brief Opaque base handle for any UI container (window, vbox, hbox, flex,
 * grid). */
typedef struct cmn_container_s cmn_container_t;
/** @brief Opaque handle for an application instance. */
typedef struct cmn_app_s cmn_app_t;
/** @brief Opaque handle for a window. */
typedef struct cmn_window_s cmn_window_t;
/** @brief Opaque handle for a button widget. */
typedef struct cmn_button_s cmn_button_t;
/** @brief Opaque handle for a label widget. */
typedef struct cmn_label_s cmn_label_t;
/** @brief Opaque handle for a text box widget. */
typedef struct cmn_textbox_s cmn_textbox_t;
/** @brief Opaque handle for a multi-line scrollable text view widget. */
typedef struct cmn_textview_s cmn_textview_t;
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
