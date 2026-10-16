#ifndef CMN_INTERNAL_H
#define CMN_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmn/error.h"
#include "cmn/types.h"
/* clang-format on */

/**
 * @file cmn_internal.h
 * @brief Internal shared header for tagged polymorphic widget structures.
 */

/**
 * @defgroup cmn_internal Internal Polymorphic Header
 * @brief Internal Polymorphic Header module.
 * @{
 */

/** @brief Magic tag value used to validate initialized `cmn_widget_header_t`
 * instances. */
#define CMN_WIDGET_MAGIC 0x434D4E57u

/**
 * @brief Polymorphic destructor callback signature for concrete widgets.
 * @param widget_ptr Address of the widget pointer to destroy and null out.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
typedef cmn_error_e (*cmn_widget_destroy_fn)(void **widget_ptr);

/**
 * @brief Common tagged header placed as the first member of every concrete
 * widget and container structure.
 */
typedef struct cmn_widget_header_s {
  uint32_t magic;         /**< Validation magic (`CMN_WIDGET_MAGIC`). */
  cmn_widget_kind_e kind; /**< Concrete widget kind discriminant. */
  cmn_widget_destroy_fn destroy_fn; /**< Concrete destructor callback. */
} cmn_widget_header_t;

/**
 * @brief Base widget structure wrapping `cmn_widget_header_t`.
 */
struct cmn_widget_s {
  cmn_widget_header_t base; /**< Tagged polymorphic widget header. */
};

/**
 * @brief Base container structure wrapping `cmn_widget_header_t`.
 */
struct cmn_container_s {
  cmn_widget_header_t base; /**< Tagged polymorphic container header. */
};

/**
 * @brief Initializes a `cmn_widget_header_t` for a newly allocated object.
 * @param header Pointer to the header to initialize.
 * @param kind Concrete widget kind.
 * @param destroy_fn Destructor callback for this widget kind.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if header is NULL or kind is
 * invalid.
 */
CMN_NO_DISCARD cmn_error_e
cmn_widget_header_init(cmn_widget_header_t *header, cmn_widget_kind_e kind,
                       cmn_widget_destroy_fn destroy_fn);

/**
 * @brief Validates a raw pointer as an initialized `cmn_widget_header_t` and
 * retrieves its kind.
 * @param ptr Pointer to inspect.
 * @param out_kind Output pointer receiving the widget kind.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if ptr is NULL or
 * uninitialized.
 */
CMN_NO_DISCARD cmn_error_e cmn_widget_get_kind(const void *ptr,
                                               cmn_widget_kind_e *out_kind);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMN_INTERNAL_H */
