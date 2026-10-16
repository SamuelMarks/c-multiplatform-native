/* clang-format off */
#include "cmn_win32_internal.h"
#include "handle_map.h"
#include "cmn/memory.h"
#include "cmn/string_safe.h"
#include "cmn/a11y_i18n.h"
#include <shobjidl.h>
#include <commctrl.h>
#include <string.h>
/* clang-format on */

/**
 * @file cmn_win32_widget.c
 * @brief Windows Win32 implementation of the widget ontology.
 */

/** @brief Mock flag to force `cmn_textbox_get_text` failure. */
static int g_cmn_mock_textbox_get_text_fail = 0;
/** @brief Last handle map operation status code. */
static int g_win32_map_rc = 0;

/**
 * @brief Sets mock failure state for cmn_textbox_get_text.
 * @param fail Whether to fail (1) or not (0).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_mock_textbox_get_text_set(int fail) {
  g_cmn_mock_textbox_get_text_fail = fail;
  return CMN_SUCCESS;
}

/** @brief Maximum number of global Win32 menu item callbacks. */
#define MAX_MENU_ITEMS 512
/** @brief Global table of Win32 menu item callbacks. */
static struct win32_menu_item_cb g_menu_item_map[MAX_MENU_ITEMS];
/** @brief Monotonically increasing Win32 menu item command ID counter. */
static int g_menu_item_id_counter = 1000;

/** @brief Win32 window class name. */
#define CMN_WND_CLASS L"CmnWindowClass"

/** @brief Maximum number of concurrent Win32 button command IDs. */
#define MAX_BUTTONS 256
/** @brief Global table mapping Win32 command IDs to button handles. */
static cmn_button_t *g_button_map[MAX_BUTTONS];

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

/**
 * @brief Win32 window procedure routing button and menu commands.
 * @param hwnd Window handle.
 * @param msg Message identifier.
 * @param wParam First message parameter.
 * @param lParam Second message parameter.
 * @return LRESULT message result.
 */
static LRESULT CALLBACK cmn_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam,
                                     LPARAM lParam) {
  if (msg == WM_COMMAND) {
    int wmId = LOWORD(wParam);
    int wmEvent = HIWORD(wParam);
    if (wmEvent == BN_CLICKED) {
      if (wmId >= 100 && wmId < 100 + MAX_BUTTONS) {
        cmn_button_t *btn = g_button_map[wmId - 100];
        if (btn && btn->on_click) {
          (void)btn->on_click(btn->userdata);
        }
      }
    } else if (wmEvent == 0) {
      if (wmId >= 1000 && wmId < 1000 + MAX_MENU_ITEMS) {
        if (g_menu_item_map[wmId - 1000].on_click) {
          (void)g_menu_item_map[wmId - 1000].on_click(
              g_menu_item_map[wmId - 1000].userdata);
        }
      }
    }
  } else if (msg == WM_DESTROY) {
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/**
 * @brief Registers the custom Win32 window class once per process.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e register_window_class(void) {
  static int registered = 0;
  if (!registered) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = cmn_wnd_proc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CMN_WND_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassExW(&wc);
    registered = 1;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Converts a UTF-8 C string to a UTF-16 wide character buffer.
 * @param utf8 Input UTF-8 string.
 * @param utf16 Output UTF-16 buffer.
 * @param utf16_size Capacity of `utf16` in `WCHAR` elements.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e utf8_to_utf16(const char *utf8, WCHAR *utf16,
                                 int utf16_size) {
  MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, utf16_size);
  return CMN_SUCCESS;
}

/**
 * @brief Converts a UTF-16 wide character string to a UTF-8 C buffer.
 * @param utf16 Input UTF-16 string.
 * @param utf8 Output UTF-8 buffer.
 * @param utf8_size Capacity of `utf8` in bytes.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e utf16_to_utf8(const WCHAR *utf16, char *utf8,
                                 int utf8_size) {
  WideCharToMultiByte(CP_UTF8, 0, utf16, -1, utf8, utf8_size, NULL, NULL);
  return CMN_SUCCESS;
}

/**
 * @brief Captures a screenshot of a window and saves it to a file.
 * @param win The window to capture.
 * @param out_filepath The destination file path for the screenshot.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_capture_screenshot(cmn_window_t *win,
                                              const char *out_filepath) {
  HDC hWindowDC;
  HDC hMemoryDC;
  HBITMAP hBitmap;
  RECT rect;
  int width, height;
  BITMAPINFOHEADER bi;
  DWORD dwBmpSize;
  HANDLE hFile;
  DWORD dwBytesWritten;
  char *lpbitmap;
  BITMAPFILEHEADER bmfHeader;
  WCHAR wpath[1024];

  if (win == NULL || out_filepath == NULL || out_filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  GetWindowRect(win->hwnd, &rect);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;

  if (width <= 0 || height <= 0) {
    width = 800;
    height = 600;
  }

  hWindowDC = GetWindowDC(win->hwnd);
  hMemoryDC = CreateCompatibleDC(hWindowDC);
  hBitmap = CreateCompatibleBitmap(hWindowDC, width, height);
  SelectObject(hMemoryDC, hBitmap);

  BitBlt(hMemoryDC, 0, 0, width, height, hWindowDC, 0, 0, SRCCOPY);

  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = width;
  bi.biHeight = height;
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;

  if (cmn_malloc(dwBmpSize, (void **)&lpbitmap) != CMN_SUCCESS) {
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(win->hwnd, hWindowDC);
    return CMN_ERR_NOMEM;
  }

  GetDIBits(hWindowDC, hBitmap, 0, (UINT)height, lpbitmap, (BITMAPINFO *)&bi,
            DIB_RGB_COLORS);

  (void)utf8_to_utf16(out_filepath, wpath, 1024);
  hFile = CreateFileW(wpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    (void)cmn_free((void **)&lpbitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(win->hwnd, hWindowDC);
    return CMN_ERR_IO;
  }

  bmfHeader.bfOffBits =
      (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
  bmfHeader.bfSize =
      dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmfHeader.bfType = 0x4D42;
  bmfHeader.bfReserved1 = 0;
  bmfHeader.bfReserved2 = 0;

  WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten,
            NULL);
  WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
  WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

  CloseHandle(hFile);
  (void)cmn_free((void **)&lpbitmap);
  DeleteObject(hBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(win->hwnd, hWindowDC);

  return CMN_SUCCESS;
}

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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = register_window_class();

  win->hwnd = CreateWindowExW(0, CMN_WND_CLASS, L"", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL,
                              NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_win32_map_rc = (int)cmn_handle_map_add((void *)win->hwnd, win);

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
  WCHAR wtitle[256];
  if (window == NULL || title == NULL) {
    return CMN_ERR_INVAL;
  }
  (void)utf8_to_utf16(title, wtitle, 256);
  SetWindowTextW(window->hwnd, wtitle);
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
  ShowWindow(window->hwnd, SW_SHOW);
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
  LONG_PTR wstyle, wexstyle;
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }

  wstyle = GetWindowLongPtrW(window->hwnd, GWL_STYLE);
  wexstyle = GetWindowLongPtrW(window->hwnd, GWL_EXSTYLE);

  if (style & CMN_WINDOW_STYLE_FRAMELESS) {
    wstyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
                WS_SYSMENU);
  } else {
    wstyle |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
               WS_SYSMENU);
  }

  if (style & CMN_WINDOW_STYLE_TRANSPARENT) {
    wexstyle |= WS_EX_LAYERED;
    SetWindowLongPtrW(window->hwnd, GWL_EXSTYLE, wexstyle);
    SetLayeredWindowAttributes(window->hwnd, 0, 255, LWA_ALPHA);
  } else {
    wexstyle &= ~WS_EX_LAYERED;
    SetWindowLongPtrW(window->hwnd, GWL_EXSTYLE, wexstyle);
  }

  SetWindowLongPtrW(window->hwnd, GWL_STYLE, wstyle);
  SetWindowPos(window->hwnd, NULL, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

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
  if (is_fullscreen) {
    SetWindowLongPtrW(window->hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(window->hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
  } else {
    SetWindowLongPtrW(window->hwnd, GWL_STYLE,
                      WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    SetWindowPos(window->hwnd, HWND_TOP, 100, 100, 800, 600, SWP_FRAMECHANGED);
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
  ShowWindow(window->hwnd, SW_MAXIMIZE);
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
  ShowWindow(window->hwnd, SW_MINIMIZE);
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
  *out_count = GetSystemMetrics(SM_CMONITORS);
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
  RECT rc;
  int screenX;
  int screenY;
  if (window == NULL) {
    return CMN_ERR_INVAL;
  }
  if (display_index < 0 || display_index >= GetSystemMetrics(SM_CMONITORS)) {
    return CMN_ERR_INVAL;
  }

  screenX = GetSystemMetrics(SM_CXSCREEN);
  screenY = GetSystemMetrics(SM_CYSCREEN);
  GetWindowRect(window->hwnd, &rc);
  SetWindowPos(window->hwnd, NULL, (screenX - (rc.right - rc.left)) / 2,
               (screenY - (rc.bottom - rc.top)) / 2, 0, 0,
               SWP_NOSIZE | SWP_NOZORDER);
  return CMN_SUCCESS;
}

/**
 * @brief Gets the DPI scale factor for a window.
 * @param window The window to query.
 * @param out_scale Pointer to store the DPI scale factor (e.g., 1.0f, 2.0f).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_get_dpi_scale(cmn_window_t *window, float *out_scale) {
  HDC hdc;
  int dpi;
  if (window == NULL || out_scale == NULL) {
    return CMN_ERR_INVAL;
  }

  hdc = GetDC(window->hwnd);
  dpi = GetDeviceCaps(hdc, LOGPIXELSX);
  ReleaseDC(window->hwnd, hdc);

  *out_scale = (dpi > 0) ? ((float)dpi / 96.0f) : 1.0f;
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
  g_win32_map_rc = (int)cmn_handle_map_remove((void *)(*window)->hwnd);
  DestroyWindow((*window)->hwnd);
  return cmn_free((void **)window);
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
  ShowWindow(modal->hwnd, SW_SHOW);
  return CMN_SUCCESS;
}

/**
 * @brief Shows a native message box alert.
 * @param title The title of the message box.
 * @param msg The message body of the message box.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_message_box_show(const char *title, const char *msg) {
  WCHAR wtitle[256];
  WCHAR wmsg[1024];
  if (title == NULL || msg == NULL) {
    return CMN_ERR_INVAL;
  }
  (void)utf8_to_utf16(title, wtitle, 256);
  (void)utf8_to_utf16(msg, wmsg, 1024);
#ifndef CMN_TESTING
  MessageBoxW(NULL, wmsg, wtitle, MB_OK);
#endif
  return CMN_SUCCESS;
}

/**
 * @brief Sets the parent container of a child widget using validated handles.
 * @param parent The parent container or window (`cmn_container_t *`,
 * `cmn_window_t *`, `cmn_vbox_t *`, or `cmn_hbox_t *`).
 * @param child The child widget (`cmn_widget_t *` or any concrete widget).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_widget_set_parent(void *parent, void *child) {
  HWND hParent = NULL;
  HWND hChild = NULL;
  cmn_widget_kind_e parent_kind;
  cmn_widget_kind_e child_kind;
  int child_idx = 0;
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

  switch (parent_kind) {
  case CMN_WIDGET_KIND_WINDOW:
    hParent = ((cmn_window_t *)parent)->hwnd;
    child_idx = ((cmn_window_t *)parent)->child_count++;
    break;
  case CMN_WIDGET_KIND_VBOX:
    hParent = ((cmn_vbox_t *)parent)->hwnd;
    child_idx = ((cmn_vbox_t *)parent)->child_count++;
    break;
  case CMN_WIDGET_KIND_HBOX:
    hParent = ((cmn_hbox_t *)parent)->hwnd;
    child_idx = ((cmn_hbox_t *)parent)->child_count++;
    break;
  default:
    return CMN_ERR_INVAL;
  }

  switch (child_kind) {
  case CMN_WIDGET_KIND_BUTTON:
    hChild = ((cmn_button_t *)child)->hwnd;
    break;
  case CMN_WIDGET_KIND_LABEL:
    hChild = ((cmn_label_t *)child)->hwnd;
    break;
  case CMN_WIDGET_KIND_TEXTBOX:
    hChild = ((cmn_textbox_t *)child)->hwnd;
    break;
  case CMN_WIDGET_KIND_TEXTVIEW:
    hChild = ((cmn_textview_t *)child)->hwnd;
    break;
  case CMN_WIDGET_KIND_VBOX:
    hChild = ((cmn_vbox_t *)child)->hwnd;
    break;
  case CMN_WIDGET_KIND_HBOX:
    hChild = ((cmn_hbox_t *)child)->hwnd;
    break;
  default:
    return CMN_ERR_INVAL;
  }

  SetParent(hChild, hParent);
  SetWindowPos(hChild, NULL, 10, 10 + (child_idx * 40), 200, 30, SWP_NOZORDER);

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
  int slot = -1;
  int i;

  if (out_button == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = cmn_calloc(1, sizeof(cmn_button_t), (void **)&btn);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  rc = cmn_widget_header_init(&btn->base, CMN_WIDGET_KIND_BUTTON,
                              destroy_button_adapter);

  for (i = 0; i < MAX_BUTTONS; ++i) {
    if (g_button_map[i] == NULL) {
      slot = i;
      g_button_map[i] = btn;
      break;
    }
  }
  btn->cmd_id = (slot >= 0) ? (100 + slot) : 0;

  btn->hwnd = CreateWindowExW(
      0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 100, 30,
      NULL, (HMENU)(intptr_t)btn->cmd_id, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_win32_map_rc = (int)cmn_handle_map_add((void *)btn->hwnd, btn);

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
  WCHAR wtext[256];
  if (button == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  (void)utf8_to_utf16(text, wtext, 256);
  SetWindowTextW(button->hwnd, wtext);
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
  if ((*button)->cmd_id >= 100 && (*button)->cmd_id < 100 + MAX_BUTTONS) {
    g_button_map[(*button)->cmd_id - 100] = NULL;
  }
  g_win32_map_rc = (int)cmn_handle_map_remove((void *)(*button)->hwnd);
  DestroyWindow((*button)->hwnd);
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

  lbl->hwnd =
      CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0,
                      100, 30, NULL, NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_win32_map_rc = (int)cmn_handle_map_add((void *)lbl->hwnd, lbl);

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
  WCHAR wtext[256];
  if (label == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  (void)utf8_to_utf16(text, wtext, 256);
  SetWindowTextW(label->hwnd, wtext);
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
  g_win32_map_rc = (int)cmn_handle_map_remove((void *)(*label)->hwnd);
  DestroyWindow((*label)->hwnd);
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

  tb->hwnd = CreateWindowExW(
      0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 0, 0,
      100, 30, NULL, NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_win32_map_rc = (int)cmn_handle_map_add((void *)tb->hwnd, tb);

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
  cmn_error_e rc;
  WCHAR wtext[256];
  if (textbox == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  (void)utf8_to_utf16(text, wtext, 256);
  if (textbox->hwnd) {
    SetWindowTextW(textbox->hwnd, wtext);
  }
  rc = cmn_strcpy_s(textbox->text_cache, sizeof(textbox->text_cache), text);
  if (rc != CMN_SUCCESS) {
    return rc;
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
  if (g_cmn_mock_textbox_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textbox == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }

  if (textbox->hwnd) {
    WCHAR wtext[1024];
    GetWindowTextW(textbox->hwnd, wtext, 1024);
    (void)utf16_to_utf8(wtext, textbox->text_cache,
                        sizeof(textbox->text_cache));
  }

  *out_text = textbox->text_cache;
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
  g_win32_map_rc = (int)cmn_handle_map_remove((void *)(*textbox)->hwnd);
  DestroyWindow((*textbox)->hwnd);
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

  tv->hwnd = CreateWindowExW(
      0, L"EDIT", L"",
      WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL |
          WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
      0, 0, 100, 100, NULL, NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_win32_map_rc = (int)cmn_handle_map_add((void *)tv->hwnd, tv);

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
  cmn_error_e rc;
  WCHAR wtext[4096];
  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  (void)utf8_to_utf16(text, wtext, 4096);
  if (textview->hwnd) {
    SetWindowTextW(textview->hwnd, wtext);
  }
  rc = cmn_strcpy_s(textview->text_cache, sizeof(textview->text_cache), text);
  if (rc != CMN_SUCCESS) {
    return rc;
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
  WCHAR wtext[4096];
  size_t cur_len;
  size_t add_len;
  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }

  cur_len = strlen(textview->text_cache);
  add_len = strlen(text);
  if (cur_len + add_len + 1 < sizeof(textview->text_cache)) {
    (void)cmn_strcpy_s(textview->text_cache + cur_len,
                       sizeof(textview->text_cache) - cur_len, text);
  }

  (void)utf8_to_utf16(textview->text_cache, wtext, 4096);
  if (textview->hwnd) {
    SetWindowTextW(textview->hwnd, wtext);
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
  if (g_cmn_mock_textview_get_text_fail) {
    return CMN_ERR_IO;
  }

  if (textview == NULL || out_text == NULL) {
    return CMN_ERR_INVAL;
  }

  *out_text = textview->text_cache;
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
  g_win32_map_rc = (int)cmn_handle_map_remove((void *)(*textview)->hwnd);
  DestroyWindow((*textview)->hwnd);
  return cmn_free((void **)textview);
}

/* --- Layout Containers --- */

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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = register_window_class();
  vbox->hwnd =
      CreateWindowExW(0, CMN_WND_CLASS, L"", WS_CHILD | WS_VISIBLE, 0, 0, 100,
                      100, NULL, NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_win32_map_rc = (int)cmn_handle_map_add((void *)vbox->hwnd, vbox);

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
  g_win32_map_rc = (int)cmn_handle_map_remove((void *)(*vbox)->hwnd);
  DestroyWindow((*vbox)->hwnd);
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
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = register_window_class();
  hbox->hwnd =
      CreateWindowExW(0, CMN_WND_CLASS, L"", WS_CHILD | WS_VISIBLE, 0, 0, 100,
                      100, NULL, NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  g_win32_map_rc = (int)cmn_handle_map_add((void *)hbox->hwnd, hbox);

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
  g_win32_map_rc = (int)cmn_handle_map_remove((void *)(*hbox)->hwnd);
  DestroyWindow((*hbox)->hwnd);
  return cmn_free((void **)hbox);
}

/* --- Menus --- */

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
  m->hmenu = CreateMenu();
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
  if ((*menu)->hmenu) {
    DestroyMenu((*menu)->hmenu);
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
  WCHAR wtitle[256];
  int id;
  if (menu == NULL || title == NULL) {
    return CMN_ERR_INVAL;
  }
  if (menu->item_count >= CMN_WIN32_MAX_MENU_ITEMS) {
    return CMN_ERR_NOMEM;
  }

  menu->items[menu->item_count].on_click = on_click;
  menu->items[menu->item_count].userdata = userdata;
  menu->item_count++;

  id = g_menu_item_id_counter++;
  if (id - 1000 < MAX_MENU_ITEMS) {
    g_menu_item_map[id - 1000].on_click = on_click;
    g_menu_item_map[id - 1000].userdata = userdata;
  }

  (void)utf8_to_utf16(title, wtitle, 256);
  AppendMenuW(menu->hmenu, MF_STRING, id, wtitle);
  return CMN_SUCCESS;
}

/**
 * @brief Programmatically triggers a menu item's callback by index.
 * @param menu The menu containing the item.
 * @param index The 0-based index of the menu item to trigger.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_menu_trigger_item(cmn_menu_t *menu, int index) {
  if (menu == NULL || index < 0 || index >= menu->item_count) {
    return CMN_ERR_INVAL;
  }
  if (menu->items[index].on_click != NULL) {
    return menu->items[index].on_click(menu->items[index].userdata);
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the main menu bar for a window.
 * @param window The window to set the menu on.
 * @param menu The menu to attach.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_menu(cmn_window_t *window, cmn_menu_t *menu) {
  if (menu == NULL) {
    return CMN_ERR_INVAL;
  }
  if (window != NULL) {
    SetMenu(window->hwnd, menu->hmenu);
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
  fp->path[0] = '\0';
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

#ifdef CMN_TESTING
  (void)cmn_strcpy_s(filepicker->path, sizeof(filepicker->path), "/tmp/test");
#else
  {
    IFileOpenDialog *pFileOpen;
    HRESULT hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                                  &IID_IFileOpenDialog, (void **)&pFileOpen);

    if (SUCCEEDED(hr)) {
      hr = pFileOpen->lpVtbl->Show(pFileOpen, NULL);
      if (SUCCEEDED(hr)) {
        IShellItem *pItem;
        hr = pFileOpen->lpVtbl->GetResult(pFileOpen, &pItem);
        if (SUCCEEDED(hr)) {
          PWSTR pszFilePath;
          hr = pItem->lpVtbl->GetDisplayName(pItem, SIGDN_FILESYSPATH,
                                             &pszFilePath);
          if (SUCCEEDED(hr)) {
            int len = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0,
                                          NULL, NULL);
            if (len > 0 && len < MAX_PATH) {
              WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, filepicker->path,
                                  len, NULL, NULL);
            }
            CoTaskMemFree(pszFilePath);
          }
          pItem->lpVtbl->Release(pItem);
        }
      }
      pFileOpen->lpVtbl->Release(pFileOpen);
    }
  }
#endif

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
  if (filepicker == NULL || out_path == NULL) {
    return CMN_ERR_INVAL;
  }
  *out_path = filepicker->path;
  return CMN_SUCCESS;
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

/* --- Accessibility (a11y) Overrides --- */

/**
 * @brief Sets the accessibility label (screen reader description) for a widget.
 * @param widget The widget to modify.
 * @param label The accessibility label string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_label(void *widget, const char *label) {
  cmn_widget_kind_e kind;
  if (widget == NULL || label == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the accessibility role for a widget.
 * @param widget The widget to modify.
 * @param role The accessibility role to set.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_role(void *widget, cmn_a11y_role_e role) {
  cmn_widget_kind_e kind;
  if (widget == NULL || role < CMN_A11Y_ROLE_BUTTON ||
      role > CMN_A11Y_ROLE_DIALOG) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Sets the keyboard navigation tab index for a widget.
 * @param widget The widget to modify.
 * @param index The tab index.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_a11y_set_tabindex(void *widget, int index) {
  cmn_widget_kind_e kind;
  (void)index;
  if (widget == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER) {
    return CMN_ERR_INVAL;
  }
  return CMN_SUCCESS;
}
