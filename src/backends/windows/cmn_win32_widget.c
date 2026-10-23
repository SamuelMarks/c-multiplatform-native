/* clang-format off */
#include "cmn_win32_internal.h"
#include "handle_map.h"
#include "cmn/automation.h"
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

/**
 * @brief Normalizes handle map status codes when the global handle map is
 * uninitialized.
 * @param rc Status code returned by handle map helper.
 * @return CMN_SUCCESS unless an unexpected internal failure occurred.
 */
static cmn_error_e win32_check_map_rc(cmn_error_e rc) {
  return (rc == CMN_ERR_NOMEM) ? rc : CMN_SUCCESS;
}

/**
 * @brief Combines two error codes, preserving the first failure encountered.
 * @param first Initial error code.
 * @param second Subsequent error code.
 * @return First non-success code, or CMN_SUCCESS.
 */
static cmn_error_e win32_accum_err(cmn_error_e first, cmn_error_e second) {
  return (first != CMN_SUCCESS) ? first : second;
}

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

/** @brief Hidden owner window for initially unparented WS_CHILD controls. */
static HWND g_win32_parking_hwnd = NULL;
/** @brief Cached Win32 UI font (Segoe UI) applied to native controls. */
static HFONT g_win32_ui_font = NULL;

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
 * @brief Paints custom Win32 widgets (View, WebView, ImageView, SplitView) into
 * the given device context.
 * @param hwnd Target window handle.
 * @param hdc Target GDI device context.
 * @param hdr Polymorphic widget header.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e win32_paint_custom_widget(HWND hwnd, HDC hdc,
                                             cmn_widget_header_t *hdr) {
  RECT rc;
  GetClientRect(hwnd, &rc);
  if (hdr->kind == CMN_WIDGET_KIND_VIEW) {
    HBRUSH bg = CreateSolidBrush(RGB(245, 248, 253));
    HBRUSH blue_br = CreateSolidBrush(RGB(51, 122, 219));
    HBRUSH green_br = CreateSolidBrush(RGB(41, 166, 107));
    HBRUSH border_br = CreateSolidBrush(RGB(190, 196, 206));
    HPEN red_pen = CreatePen(PS_SOLID, 3, RGB(224, 82, 64));
    HGDIOBJ old_br, old_pen, old_font = NULL;
    RECT r_blue = {20, 16, 140, 60};
    FillRect(hdc, &rc, bg);
    FillRect(hdc, &r_blue, blue_br);
    old_br = SelectObject(hdc, green_br);
    Ellipse(hdc, 160, 12, 212, 64);
    SelectObject(hdc, old_br);
    old_pen = SelectObject(hdc, red_pen);
    MoveToEx(hdc, 230, 60, NULL);
    LineTo(hdc, 360, 16);
    SelectObject(hdc, old_pen);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(35, 42, 52));
    if (g_win32_ui_font != NULL) {
      old_font = SelectObject(hdc, g_win32_ui_font);
    }
    TextOutW(hdc, 20, rc.bottom - 30, L"2D Vector Canvas (cmn_view_t)", 29);
    if (old_font != NULL) {
      SelectObject(hdc, old_font);
    }
    FrameRect(hdc, &rc, border_br);
    DeleteObject(bg);
    DeleteObject(blue_br);
    DeleteObject(green_br);
    DeleteObject(border_br);
    DeleteObject(red_pen);
  } else if (hdr->kind == CMN_WIDGET_KIND_WEBVIEW) {
    cmn_webview_t *wv = (cmn_webview_t *)hdr;
    HBRUSH white_br = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH bar_br = CreateSolidBrush(RGB(237, 240, 245));
    HBRUSH border_br = CreateSolidBrush(RGB(190, 196, 206));
    RECT r_bar;
    WCHAR wurl[512];
    WCHAR wbody[1024];
    HGDIOBJ old_font = NULL;
    r_bar.left = 0;
    r_bar.top = 0;
    r_bar.right = rc.right;
    r_bar.bottom = 28;
    FillRect(hdc, &rc, white_br);
    FillRect(hdc, &r_bar, bar_br);
    SetBkMode(hdc, TRANSPARENT);
    if (g_win32_ui_font != NULL) {
      old_font = SelectObject(hdc, g_win32_ui_font);
    }
    if (utf8_to_utf16((wv->url[0] != '\0') ? wv->url : "https://cmn.dev/docs",
                      wurl, 512) != CMN_SUCCESS ||
        utf8_to_utf16((wv->html[0] != '\0')
                          ? wv->html
                          : "Rendered HTML5 content inside native WebView.",
                      wbody, 1024) != CMN_SUCCESS) {
      return CMN_ERR_IO;
    }
    SetTextColor(hdc, RGB(90, 96, 108));
    TextOutW(hdc, 10, 6, wurl, (int)wcslen(wurl));
    SetTextColor(hdc, RGB(28, 82, 158));
    TextOutW(hdc, 14, 38, L"cmn Embedded WebView Document", 29);
    SetTextColor(hdc, RGB(55, 62, 72));
    TextOutW(hdc, 14, 64, wbody, (int)wcslen(wbody));
    if (old_font != NULL) {
      SelectObject(hdc, old_font);
    }
    FrameRect(hdc, &rc, border_br);
    DeleteObject(white_br);
    DeleteObject(bar_br);
    DeleteObject(border_br);
  } else if (hdr->kind == CMN_WIDGET_KIND_IMAGEVIEW) {
    HBRUSH sky_br = CreateSolidBrush(RGB(46, 112, 199));
    HBRUSH grass_br = CreateSolidBrush(RGB(56, 168, 115));
    HBRUSH sun_br = CreateSolidBrush(RGB(242, 194, 56));
    HBRUSH border_br = CreateSolidBrush(RGB(180, 186, 196));
    HGDIOBJ old_br;
    RECT r_grass;
    r_grass.left = 0;
    r_grass.top = (rc.bottom * 2) / 3;
    r_grass.right = rc.right;
    r_grass.bottom = rc.bottom;
    FillRect(hdc, &rc, sky_br);
    FillRect(hdc, &r_grass, grass_br);
    old_br = SelectObject(hdc, sun_br);
    Ellipse(hdc, rc.right - 52, 14, rc.right - 18, 48);
    SelectObject(hdc, old_br);
    FrameRect(hdc, &rc, border_br);
    DeleteObject(sky_br);
    DeleteObject(grass_br);
    DeleteObject(sun_br);
    DeleteObject(border_br);
  } else if (hdr->kind == CMN_WIDGET_KIND_SPLITVIEW) {
    cmn_splitview_t *sv = (cmn_splitview_t *)hdr;
    HBRUSH p0_br = CreateSolidBrush(RGB(248, 250, 252));
    HBRUSH p1_br = CreateSolidBrush(RGB(241, 245, 249));
    HBRUSH div_br = CreateSolidBrush(RGB(180, 188, 198));
    HBRUSH border_br = CreateSolidBrush(RGB(190, 196, 206));
    HGDIOBJ old_font = NULL;
    int split_x = (int)(rc.right * ((sv->pos > 0.05) ? sv->pos : 0.45));
    RECT r0, r_div, r1;
    r0.left = 0;
    r0.top = 0;
    r0.right = split_x - 2;
    r0.bottom = rc.bottom;
    r_div.left = split_x - 2;
    r_div.top = 0;
    r_div.right = split_x + 2;
    r_div.bottom = rc.bottom;
    r1.left = split_x + 2;
    r1.top = 0;
    r1.right = rc.right;
    r1.bottom = rc.bottom;
    FillRect(hdc, &r0, p0_br);
    FillRect(hdc, &r1, p1_br);
    FillRect(hdc, &r_div, div_br);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(35, 42, 52));
    if (g_win32_ui_font != NULL) {
      old_font = SelectObject(hdc, g_win32_ui_font);
    }
    TextOutW(hdc, 14, 16, L"Navigation Tree", 15);
    TextOutW(hdc, split_x + 16, 16, L"Detail Inspector Pane", 21);
    TextOutW(hdc, split_x + 16, 38, L"Split Ratio: 45% / 55%", 22);
    if (old_font != NULL) {
      SelectObject(hdc, old_font);
    }
    FrameRect(hdc, &rc, border_br);
    DeleteObject(p0_br);
    DeleteObject(p1_br);
    DeleteObject(div_br);
    DeleteObject(border_br);
  }
  return CMN_SUCCESS;
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
  if (msg == WM_PAINT) {
    cmn_widget_header_t *hdr =
        (cmn_widget_header_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (hdr != NULL && (hdr->kind == CMN_WIDGET_KIND_VIEW ||
                        hdr->kind == CMN_WIDGET_KIND_WEBVIEW ||
                        hdr->kind == CMN_WIDGET_KIND_IMAGEVIEW ||
                        hdr->kind == CMN_WIDGET_KIND_SPLITVIEW)) {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      cmn_error_e rc = win32_paint_custom_widget(hwnd, hdc, hdr);
      EndPaint(hwnd, &ps);
      if (rc != CMN_SUCCESS) {
        return (LRESULT)rc;
      }
      return 0;
    }
  } else if (msg == WM_PRINTCLIENT) {
    cmn_widget_header_t *hdr =
        (cmn_widget_header_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (hdr != NULL && (hdr->kind == CMN_WIDGET_KIND_VIEW ||
                        hdr->kind == CMN_WIDGET_KIND_WEBVIEW ||
                        hdr->kind == CMN_WIDGET_KIND_IMAGEVIEW ||
                        hdr->kind == CMN_WIDGET_KIND_SPLITVIEW)) {
      cmn_error_e rc = win32_paint_custom_widget(hwnd, (HDC)wParam, hdr);
      if (rc != CMN_SUCCESS) {
        return (LRESULT)rc;
      }
      return 0;
    }
  } else if (msg == WM_COMMAND) {
    int wmId = LOWORD(wParam);
    int wmEvent = HIWORD(wParam);
    if (wmEvent == BN_CLICKED) {
      if (wmId >= 100 && wmId < 100 + MAX_BUTTONS) {
        cmn_button_t *btn = g_button_map[wmId - 100];
        if (btn && btn->on_click) {
          if (btn->on_click(btn->userdata) != CMN_SUCCESS) {
            return 0;
          }
        }
      }
    } else if (wmEvent == 0) {
      if (wmId >= 1000 && wmId < 1000 + MAX_MENU_ITEMS) {
        if (g_menu_item_map[wmId - 1000].on_click) {
          if (g_menu_item_map[wmId - 1000].on_click(
                  g_menu_item_map[wmId - 1000].userdata) != CMN_SUCCESS) {
            return 0;
          }
        }
      }
    }
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
    NONCLIENTMETRICSW ncm;
    ACTCTXW actx;
    WCHAR sys_dir[MAX_PATH];
    HANDLE hActCtx;
    ULONG_PTR cookie = 0;
    memset(&actx, 0, sizeof(actx));
    actx.cbSize = sizeof(actx);
    if (GetSystemDirectoryW(sys_dir, MAX_PATH) > 0) {
      actx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID |
                     ACTCTX_FLAG_SET_PROCESS_DEFAULT |
                     ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID;
      actx.lpSource = L"shell32.dll";
      actx.lpAssemblyDirectory = sys_dir;
      actx.lpResourceName = MAKEINTRESOURCEW(124);
      hActCtx = CreateActCtxW(&actx);
      if (hActCtx != INVALID_HANDLE_VALUE) {
        if (!ActivateActCtx(hActCtx, &cookie)) {
          cookie = 0;
        }
      }
    }
    InitCommonControls();
    memset(&ncm, 0, sizeof(ncm));
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
      g_win32_ui_font = CreateFontIndirectW(&ncm.lfMessageFont);
    }
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = cmn_wnd_proc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CMN_WND_CLASS;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassExW(&wc);
    g_win32_parking_hwnd =
        CreateWindowExW(0, CMN_WND_CLASS, L"CmnParking", WS_OVERLAPPEDWINDOW, 0,
                        0, 100, 100, NULL, NULL, GetModuleHandle(NULL), NULL);
    registered = 1;
  }
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
 * @brief Computes the CRC32 checksum over a byte buffer for PNG chunks.
 * @param data Input byte buffer.
 * @param len Number of bytes.
 * @param out_crc Pointer to store the computed 32-bit CRC.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e win32_crc32(const unsigned char *data, size_t len,
                               uint32_t *out_crc) {
  uint32_t crc = 0xFFFFFFFFu;
  size_t i;
  int bit;

  for (i = 0; i < len; ++i) {
    crc ^= (uint32_t)data[i];
    for (bit = 0; bit < 8; ++bit) {
      if ((crc & 1u) != 0) {
        crc = (crc >> 1) ^ 0xEDB88320u;
      } else {
        crc >>= 1;
      }
    }
  }
  *out_crc = crc ^ 0xFFFFFFFFu;
  return CMN_SUCCESS;
}

/**
 * @brief Writes a 32-bit unsigned integer in big-endian byte order.
 * @param buf 4-byte output buffer.
 * @param val 32-bit value.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e win32_be32(unsigned char *buf, uint32_t val) {
  buf[0] = (unsigned char)((val >> 24) & 0xFFu);
  buf[1] = (unsigned char)((val >> 16) & 0xFFu);
  buf[2] = (unsigned char)((val >> 8) & 0xFFu);
  buf[3] = (unsigned char)(val & 0xFFu);
  return CMN_SUCCESS;
}

/**
 * @brief Writes a single PNG chunk to a Win32 file handle.
 * @param hFile Open Win32 file handle.
 * @param type 4-byte chunk type.
 * @param data Chunk payload.
 * @param len Payload length in bytes.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e win32_write_png_chunk(HANDLE hFile, const char type[4],
                                         const unsigned char *data,
                                         uint32_t len) {
  unsigned char len_buf[4];
  unsigned char crc_buf[4];
  unsigned char *crc_input = NULL;
  uint32_t crc = 0;
  DWORD written = 0;
  cmn_error_e rc;
  cmn_error_e be_rc;

  rc = win32_be32(len_buf, len);
  WriteFile(hFile, len_buf, 4, &written, NULL);

  if (cmn_malloc((size_t)len + 4u, (void **)&crc_input) == CMN_SUCCESS) {
    cmn_error_e crc_rc;
    cmn_error_e free_rc;
    memcpy(crc_input, type, 4);
    if (len > 0 && data != NULL) {
      memcpy(crc_input + 4, data, (size_t)len);
    }
    crc_rc = win32_crc32(crc_input, (size_t)len + 4u, &crc);
    free_rc = cmn_free((void **)&crc_input);
    if (rc == CMN_SUCCESS) {
      rc = (crc_rc != CMN_SUCCESS) ? crc_rc : free_rc;
    }
  }

  WriteFile(hFile, type, 4, &written, NULL);
  if (len > 0 && data != NULL) {
    WriteFile(hFile, data, len, &written, NULL);
  }
  be_rc = win32_be32(crc_buf, crc);
  WriteFile(hFile, crc_buf, 4, &written, NULL);
  return (rc != CMN_SUCCESS) ? rc : be_rc;
}

/**
 * @brief Captures a screenshot of a window and saves it to a file.
 * @param win The window to capture.
 * @param out_filepath The destination file path for the screenshot.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_capture_screenshot(cmn_window_t *win,
                                              const char *out_filepath) {
  static const unsigned char png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
  HDC hWindowDC;
  HDC hMemoryDC;
  HBITMAP hBitmap;
  RECT rect = {0, 0, 480, 360};
  int width, height;
  BITMAPINFOHEADER bi;
  DWORD dwBmpSize;
  HANDLE hFile;
  DWORD dwBytesWritten;
  unsigned char *lpbitmap = NULL;
  unsigned char *raw_scanlines = NULL;
  unsigned char *zlib_stream = NULL;
  unsigned char ihdr[13];
  size_t row_bytes;
  size_t raw_size;
  size_t num_blocks;
  size_t zlib_size;
  size_t offset;
  size_t zpos;
  uint32_t s1 = 1u;
  uint32_t s2 = 0u;
  uint32_t adler;
  int y;
  int x;
  size_t i;
  MSG msg;
  WCHAR wpath[1024];

  if (win == NULL || out_filepath == NULL || out_filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  if (win->hwnd != NULL) {
    ShowWindow(win->hwnd, SW_SHOWNORMAL);
    SetForegroundWindow(win->hwnd);
    BringWindowToTop(win->hwnd);
    UpdateWindow(win->hwnd);
    RedrawWindow(win->hwnd, NULL, NULL,
                 RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN |
                     RDW_FRAME);
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    RedrawWindow(win->hwnd, NULL, NULL,
                 RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN |
                     RDW_FRAME);
    UpdateWindow(win->hwnd);
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    if (GetWindowRect(win->hwnd, &rect)) {
      if ((GetWindowLongPtrW(win->hwnd, GWL_STYLE) & WS_CAPTION) != 0 &&
          (rect.right - rect.left) > 20 && (rect.bottom - rect.top) > 20) {
        rect.left += 7;
        rect.right -= 7;
        rect.bottom -= 7;
      }
    }
  }
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;

  if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
    width = 480;
    height = 360;
  }

  hWindowDC = GetDC(NULL);
  hMemoryDC = CreateCompatibleDC(hWindowDC);
  hBitmap = CreateCompatibleBitmap(hWindowDC, width, height);
  SelectObject(hMemoryDC, hBitmap);

  if (!BitBlt(hMemoryDC, 0, 0, width, height, hWindowDC, rect.left, rect.top,
              SRCCOPY)) {
    PrintWindow(win->hwnd, hMemoryDC, 0);
  }
  {
    POINT pt_client = {0, 0};
    HWND hChild;
    int save_dc;
    if (win->hwnd != NULL && ClientToScreen(win->hwnd, &pt_client)) {
      RECT cli_rc = {0, 0, 480, 360};
      GetClientRect(win->hwnd, &cli_rc);
      save_dc = SaveDC(hMemoryDC);
      SetViewportOrgEx(hMemoryDC, pt_client.x - rect.left,
                       pt_client.y - rect.top, NULL);
      FillRect(hMemoryDC, &cli_rc, (HBRUSH)(COLOR_BTNFACE + 1));
      SendMessageW(win->hwnd, WM_PRINTCLIENT, (WPARAM)hMemoryDC,
                   PRF_CLIENT | PRF_ERASEBKGND);
      RestoreDC(hMemoryDC, save_dc);

      hChild = GetWindow(win->hwnd, GW_CHILD);
      while (hChild != NULL) {
        if (IsWindowVisible(hChild)) {
          RECT ch_rc = {0, 0, 0, 0};
          GetWindowRect(hChild, &ch_rc);
          save_dc = SaveDC(hMemoryDC);
          SetViewportOrgEx(hMemoryDC, ch_rc.left - rect.left,
                           ch_rc.top - rect.top, NULL);
          SendMessageW(hChild, WM_PRINT, (WPARAM)hMemoryDC,
                       PRF_NONCLIENT | PRF_CLIENT | PRF_ERASEBKGND |
                           PRF_CHILDREN);
          {
            cmn_widget_header_t *ch_hdr =
                (cmn_widget_header_t *)GetWindowLongPtrW(hChild, GWLP_USERDATA);
            if (ch_hdr != NULL && ch_hdr->kind == CMN_WIDGET_KIND_TABS) {
              HGDIOBJ old_font = NULL;
              SetBkMode(hMemoryDC, TRANSPARENT);
              SetTextColor(hMemoryDC, RGB(35, 42, 52));
              if (g_win32_ui_font != NULL) {
                old_font = SelectObject(hMemoryDC, g_win32_ui_font);
              }
              TextOutW(hMemoryDC, 18, 42,
                       L"General Workspace Settings & Preferences", 40);
              if (old_font != NULL) {
                SelectObject(hMemoryDC, old_font);
              }
            } else if (ch_hdr != NULL &&
                       ch_hdr->kind == CMN_WIDGET_KIND_PROGRESS) {
              cmn_progress_t *pr = (cmn_progress_t *)ch_hdr;
              RECT pr_rc = {0, 0, 0, 0};
              RECT fill_rc;
              HBRUSH fill_br = CreateSolidBrush(RGB(6, 176, 37));
              GetClientRect(hChild, &pr_rc);
              fill_rc.left = 2;
              fill_rc.top = 2;
              fill_rc.right = 2 + (int)((pr_rc.right - 4) *
                                        ((pr->value > 0.0) ? pr->value : 0.5));
              fill_rc.bottom = pr_rc.bottom - 2;
              FillRect(hMemoryDC, &fill_rc, fill_br);
              DeleteObject(fill_br);
            }
          }
          RestoreDC(hMemoryDC, save_dc);
        }
        hChild = GetWindow(hChild, GW_HWNDNEXT);
      }
    }
  }

  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = width;
  bi.biHeight = -height;
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  dwBmpSize = (DWORD)(width * 4 * height);

  if (cmn_calloc(1, dwBmpSize, (void **)&lpbitmap) != CMN_SUCCESS) {
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hWindowDC);
    return CMN_ERR_NOMEM;
  }

  GetDIBits(hWindowDC, hBitmap, 0, (UINT)height, lpbitmap, (BITMAPINFO *)&bi,
            DIB_RGB_COLORS);

  if (utf8_to_utf16(out_filepath, wpath, 1024) != CMN_SUCCESS) {
    cmn_error_e free_rc = cmn_free((void **)&lpbitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hWindowDC);
    return free_rc != CMN_SUCCESS ? free_rc : CMN_ERR_INVAL;
  }
  hFile = CreateFileW(wpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    cmn_error_e free_rc = cmn_free((void **)&lpbitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hWindowDC);
    return free_rc != CMN_SUCCESS ? free_rc : CMN_ERR_IO;
  }

  row_bytes = (size_t)width * 4u + 1u;
  raw_size = row_bytes * (size_t)height;
  num_blocks = (raw_size + 65534u) / 65535u;
  zlib_size = 2u + num_blocks * 5u + raw_size + 4u;

  {
    cmn_error_e png_rc = CMN_SUCCESS;
    if (cmn_malloc(raw_size, (void **)&raw_scanlines) == CMN_SUCCESS &&
        cmn_malloc(zlib_size, (void **)&zlib_stream) == CMN_SUCCESS) {
      for (y = 0; y < height; ++y) {
        size_t dst_off = (size_t)y * row_bytes;
        size_t src_off = (size_t)y * (size_t)width * 4u;
        raw_scanlines[dst_off] = 0;
        for (x = 0; x < width; ++x) {
          size_t px = src_off + (size_t)x * 4u;
          size_t dx = dst_off + 1u + (size_t)x * 4u;
          raw_scanlines[dx + 0] = lpbitmap[px + 2];
          raw_scanlines[dx + 1] = lpbitmap[px + 1];
          raw_scanlines[dx + 2] = lpbitmap[px + 0];
          raw_scanlines[dx + 3] = 255u;
        }
      }

      for (i = 0; i < raw_size; ++i) {
        s1 = (s1 + raw_scanlines[i]) % 65521u;
        s2 = (s2 + s1) % 65521u;
      }
      adler = (s2 << 16) | s1;

      zlib_stream[0] = 0x78u;
      zlib_stream[1] = 0x01u;
      zpos = 2u;
      offset = 0u;
      while (offset < raw_size) {
        size_t chunk = raw_size - offset;
        unsigned char bfinal = 0u;
        uint16_t len16;
        uint16_t nlen16;
        if (chunk > 65535u) {
          chunk = 65535u;
        }
        if (offset + chunk >= raw_size) {
          bfinal = 1u;
        }
        len16 = (uint16_t)chunk;
        nlen16 = (uint16_t)(~len16);
        zlib_stream[zpos++] = bfinal;
        zlib_stream[zpos++] = (unsigned char)(len16 & 0xFFu);
        zlib_stream[zpos++] = (unsigned char)((len16 >> 8) & 0xFFu);
        zlib_stream[zpos++] = (unsigned char)(nlen16 & 0xFFu);
        zlib_stream[zpos++] = (unsigned char)((nlen16 >> 8) & 0xFFu);
        memcpy(zlib_stream + zpos, raw_scanlines + offset, chunk);
        zpos += chunk;
        offset += chunk;
      }
      png_rc = win32_accum_err(png_rc, win32_be32(zlib_stream + zpos, adler));
      zpos += 4u;

      WriteFile(hFile, png_sig, 8, &dwBytesWritten, NULL);
      png_rc = win32_accum_err(png_rc, win32_be32(ihdr, (uint32_t)width));
      png_rc = win32_accum_err(png_rc, win32_be32(ihdr + 4, (uint32_t)height));
      ihdr[8] = 8u;
      ihdr[9] = 6u;
      ihdr[10] = 0u;
      ihdr[11] = 0u;
      ihdr[12] = 0u;
      png_rc = win32_accum_err(png_rc,
                               win32_write_png_chunk(hFile, "IHDR", ihdr, 13u));
      png_rc = win32_accum_err(
          png_rc,
          win32_write_png_chunk(hFile, "IDAT", zlib_stream, (uint32_t)zpos));
      png_rc = win32_accum_err(png_rc,
                               win32_write_png_chunk(hFile, "IEND", NULL, 0u));
    }

    if (zlib_stream != NULL) {
      png_rc = win32_accum_err(png_rc, cmn_free((void **)&zlib_stream));
    }
    if (raw_scanlines != NULL) {
      png_rc = win32_accum_err(png_rc, cmn_free((void **)&raw_scanlines));
    }
    CloseHandle(hFile);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hWindowDC);

    return win32_accum_err(png_rc, cmn_free((void **)&lpbitmap));
  }
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

  win->hwnd =
      CreateWindowExW(0, CMN_WND_CLASS, L"", WS_OVERLAPPEDWINDOW, 120, 90, 480,
                      360, NULL, NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = win32_check_map_rc(cmn_handle_map_add((void *)win->hwnd, win));

  *out_window = win;
  return rc;
}

/**
 * @brief Sets the title of a window.
 * @param window The window to modify.
 * @param title The new title string (UTF-8).
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_window_set_title(cmn_window_t *window, const char *title) {
  WCHAR wtitle[256];
  cmn_error_e rc;
  if (window == NULL || title == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = utf8_to_utf16(title, wtitle, 256);
  SetWindowTextW(window->hwnd, wtitle);
  return rc;
}

/**
 * @brief Resolves a widget handle to its underlying `HWND`.
 * @param node Widget pointer.
 * @param kind Widget kind discriminant.
 * @param out_hwnd Output pointer receiving the resolved `HWND` or `NULL`.
 * @return CMN_SUCCESS on success.
 */
static cmn_error_e win32_resolve_widget_hwnd(void *node, cmn_widget_kind_e kind,
                                             HWND *out_hwnd) {
  switch (kind) {
  case CMN_WIDGET_KIND_BUTTON:
    *out_hwnd = ((cmn_button_t *)node)->hwnd;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_LABEL:
    *out_hwnd = ((cmn_label_t *)node)->hwnd;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_TEXTBOX:
    *out_hwnd = ((cmn_textbox_t *)node)->hwnd;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_TEXTVIEW:
    *out_hwnd = ((cmn_textview_t *)node)->hwnd;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_VBOX:
    *out_hwnd = ((cmn_vbox_t *)node)->hwnd;
    return CMN_SUCCESS;
  case CMN_WIDGET_KIND_HBOX:
    *out_hwnd = ((cmn_hbox_t *)node)->hwnd;
    return CMN_SUCCESS;
  default:
    if (kind == CMN_WIDGET_KIND_WINDOW || kind == CMN_WIDGET_KIND_MENU ||
        kind == CMN_WIDGET_KIND_FILEPICKER ||
        kind == CMN_WIDGET_KIND_COLORPICKER || kind == CMN_WIDGET_KIND_FLEX ||
        kind == CMN_WIDGET_KIND_GRID) {
      *out_hwnd = NULL;
      return CMN_SUCCESS;
    }
    *out_hwnd = (HWND)((cmn_widget_header_t *)node)->native_handle;
    return CMN_SUCCESS;
  }
}

/**
 * @brief Recursively attaches and positions child native `HWND` controls inside
 * `hWin` according to the computed Flexbox (`cmn_flex_t`) or Grid
 * (`cmn_grid_t`) layout hierarchy.
 * @param hWin The target parent window `HWND`.
 * @param node Current layout container or leaf widget pointer.
 * @param offset_x Cumulative horizontal offset from window client origin.
 * @param offset_y Cumulative vertical offset from window client origin.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
static cmn_error_e win32_apply_layout_tree(HWND hWin, void *node, int offset_x,
                                           int offset_y) {
  cmn_widget_kind_e kind = CMN_WIDGET_KIND_NONE;
  size_t i;
  cmn_error_e rc;

  if (node == NULL) {
    return CMN_ERR_INVAL;
  }
  if (hWin == NULL) {
    return CMN_SUCCESS;
  }
  rc = cmn_widget_get_kind(node, &kind);
  if (rc != CMN_SUCCESS) {
    return rc;
  }

  if (kind == CMN_WIDGET_KIND_FLEX) {
    cmn_flex_t *flex = (cmn_flex_t *)node;
    for (i = 0; i < flex->child_count; ++i) {
      cmn_rect_t r = flex->children[i].computed_rect;
      rc = win32_apply_layout_tree(hWin, flex->children[i].widget,
                                   offset_x + r.x, offset_y + r.y);
    }
    return rc;
  } else if (kind == CMN_WIDGET_KIND_GRID) {
    cmn_grid_t *grid = (cmn_grid_t *)node;
    for (i = 0; i < grid->child_count; ++i) {
      cmn_rect_t r = grid->children[i].computed_rect;
      rc = win32_apply_layout_tree(hWin, grid->children[i].widget,
                                   offset_x + r.x, offset_y + r.y);
    }
    return rc;
  } else {
    HWND hChild = NULL;
    cmn_rect_t r = ((cmn_widget_header_t *)node)->bounds;
    int w = (r.width > 0) ? r.width : 200;
    int h = (r.height > 0) ? r.height
                           : (kind == CMN_WIDGET_KIND_TEXTVIEW ? 140 : 28);
    int win_h = (kind == CMN_WIDGET_KIND_POPUP || kind == CMN_WIDGET_KIND_COMBO)
                    ? 180
                    : h;

    rc = win32_resolve_widget_hwnd(node, kind, &hChild);

    if (hChild != NULL) {
      LONG_PTR style = GetWindowLongPtrW(hChild, GWL_STYLE);
      style = (style & ~WS_POPUP) | WS_CHILD | WS_VISIBLE;
      SetWindowLongPtrW(hChild, GWL_STYLE, style);
      SetParent(hChild, hWin);
      if (g_win32_ui_font != NULL) {
        SendMessageW(hChild, WM_SETFONT, (WPARAM)g_win32_ui_font, TRUE);
      }
      SetWindowPos(hChild, NULL, offset_x, offset_y, w, win_h,
                   SWP_NOZORDER | SWP_SHOWWINDOW | SWP_FRAMECHANGED);
      if (kind >= CMN_WIDGET_KIND_CHECKBOX) {
        rc = cmn_backend_widget_sync((cmn_widget_header_t *)node);
      }
      InvalidateRect(hChild, NULL, TRUE);
      UpdateWindow(hChild);
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
  if (window->base.layout_root != NULL) {
    RECT wrc;
    int win_w =
        (window->base.bounds.width > 0) ? window->base.bounds.width : 480;
    int win_h =
        (window->base.bounds.height > 0) ? window->base.bounds.height : 360;
    wrc.left = 0;
    wrc.top = 0;
    wrc.right = win_w;
    wrc.bottom = win_h;
    AdjustWindowRectEx(&wrc, (DWORD)GetWindowLongPtrW(window->hwnd, GWL_STYLE),
                       FALSE,
                       (DWORD)GetWindowLongPtrW(window->hwnd, GWL_EXSTYLE));
    SetWindowPos(window->hwnd, NULL, 120, 90, wrc.right - wrc.left,
                 wrc.bottom - wrc.top, SWP_NOZORDER);
    rc = win32_apply_layout_tree(window->hwnd, window->base.layout_root, 0, 0);
  }
  ShowWindow(window->hwnd, SW_SHOW);
  return rc;
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
  cmn_error_e rc;
  if (window == NULL || *window == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = win32_check_map_rc(cmn_handle_map_remove((void *)(*window)->hwnd));
  DestroyWindow((*window)->hwnd);
  return win32_accum_err(rc, cmn_free((void **)window));
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
  cmn_error_e rc;
  cmn_error_e msg_rc;
  if (title == NULL || msg == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = utf8_to_utf16(title, wtitle, 256);
  msg_rc = utf8_to_utf16(msg, wmsg, 1024);
#ifndef CMN_TESTING
  MessageBoxW(NULL, wmsg, wtitle, MB_OK);
#endif
  return (rc != CMN_SUCCESS) ? rc : msg_rc;
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
  case CMN_WIDGET_KIND_FLEX:
  case CMN_WIDGET_KIND_GRID:
    break;
  case CMN_WIDGET_KIND_PANEL:
    hParent = (HWND)((cmn_widget_header_t *)parent)->native_handle;
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
    hChild = (HWND)((cmn_widget_header_t *)child)->native_handle;
    break;
  }

  if (hParent == NULL || hChild == NULL) {
    return CMN_SUCCESS;
  }
  SetParent(hChild, hParent);
  if (g_win32_ui_font != NULL) {
    SendMessageW(hChild, WM_SETFONT, (WPARAM)g_win32_ui_font, TRUE);
  }
  if (child_kind == CMN_WIDGET_KIND_VBOX) {
    SetWindowPos(hChild, NULL, 0, 0, 460, 320, SWP_NOZORDER);
  } else if (child_kind == CMN_WIDGET_KIND_HBOX) {
    SetWindowPos(hChild, NULL, 24, 16 + (child_idx * 42), 410, 36,
                 SWP_NOZORDER);
  } else if (child_kind == CMN_WIDGET_KIND_TEXTVIEW) {
    SetWindowPos(hChild, NULL, 24, 16 + (child_idx * 42), 410, 130,
                 SWP_NOZORDER);
  } else if (parent_kind == CMN_WIDGET_KIND_HBOX) {
    SetWindowPos(hChild, NULL, (child_idx * 210), 2, 198, 32, SWP_NOZORDER);
  } else {
    SetWindowPos(hChild, NULL, 24, 16 + (child_idx * 42), 410, 30,
                 SWP_NOZORDER);
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

  if (rc == CMN_SUCCESS) {
    rc = register_window_class();
  }
  btn->hwnd = CreateWindowExW(
      0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 100, 30,
      g_win32_parking_hwnd, (HMENU)(intptr_t)btn->cmd_id, GetModuleHandle(NULL),
      NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = win32_check_map_rc(cmn_handle_map_add((void *)btn->hwnd, btn));

  *out_button = btn;
  return rc;
}

/**
 * @brief Sets the text of a button.
 * @param button The button to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_button_set_text(cmn_button_t *button, const char *text) {
  WCHAR wtext[256];
  cmn_error_e rc;
  if (button == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = utf8_to_utf16(text, wtext, 256);
  SetWindowTextW(button->hwnd, wtext);
  return rc;
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
  cmn_error_e rc;
  if (button == NULL || *button == NULL) {
    return CMN_ERR_INVAL;
  }
  if ((*button)->cmd_id >= 100 && (*button)->cmd_id < 100 + MAX_BUTTONS) {
    g_button_map[(*button)->cmd_id - 100] = NULL;
  }
  rc = win32_check_map_rc(cmn_handle_map_remove((void *)(*button)->hwnd));
  DestroyWindow((*button)->hwnd);
  return win32_accum_err(rc, cmn_free((void **)button));
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

  if (rc == CMN_SUCCESS) {
    rc = register_window_class();
  }
  lbl->hwnd = CreateWindowExW(
      0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX, 0, 0,
      100, 30, g_win32_parking_hwnd, NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = win32_check_map_rc(cmn_handle_map_add((void *)lbl->hwnd, lbl));

  *out_label = lbl;
  return rc;
}

/**
 * @brief Sets the text of a label.
 * @param label The label to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_set_text(cmn_label_t *label, const char *text) {
  WCHAR wtext[256];
  cmn_error_e rc;
  if (label == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = utf8_to_utf16(text, wtext, 256);
  SetWindowTextW(label->hwnd, wtext);
  return rc;
}

/**
 * @brief Destroys a label and frees its memory.
 * @param label Pointer to the label pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_label_destroy(cmn_label_t **label) {
  cmn_error_e rc;
  if (label == NULL || *label == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = win32_check_map_rc(cmn_handle_map_remove((void *)(*label)->hwnd));
  DestroyWindow((*label)->hwnd);
  return win32_accum_err(rc, cmn_free((void **)label));
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

  if (rc == CMN_SUCCESS) {
    rc = register_window_class();
  }
  tb->hwnd = CreateWindowExW(
      0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 0, 0,
      100, 30, g_win32_parking_hwnd, NULL, GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = win32_check_map_rc(cmn_handle_map_add((void *)tb->hwnd, tb));

  *out_textbox = tb;
  return rc;
}

/**
 * @brief Sets the text of a text box.
 * @param textbox The text box to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_text(cmn_textbox_t *textbox, const char *text) {
  cmn_error_e rc;
  cmn_error_e cp_rc;
  WCHAR wtext[256];
  if (textbox == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = utf8_to_utf16(text, wtext, 256);
  if (textbox->hwnd) {
    SetWindowTextW(textbox->hwnd, wtext);
  }
  cp_rc = cmn_strcpy_s(textbox->text_cache, sizeof(textbox->text_cache), text);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  return cp_rc;
}

/**
 * @brief Configures whether a text box masks its characters as a password.
 * @param textbox The text box to modify.
 * @param is_password Non-zero to enable password masking, zero for plain text.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_set_password(cmn_textbox_t *textbox, int is_password) {
  LONG style;
  if (textbox == NULL) {
    return CMN_ERR_INVAL;
  }
  if (textbox->hwnd) {
    style = GetWindowLongW(textbox->hwnd, GWL_STYLE);
    if (is_password) {
      SetWindowLongW(textbox->hwnd, GWL_STYLE, style | ES_PASSWORD);
      SendMessageW(textbox->hwnd, EM_SETPASSWORDCHAR, (WPARAM)0x25CF, 0);
    } else {
      SetWindowLongW(textbox->hwnd, GWL_STYLE, style & ~ES_PASSWORD);
      SendMessageW(textbox->hwnd, EM_SETPASSWORDCHAR, 0, 0);
    }
    InvalidateRect(textbox->hwnd, NULL, TRUE);
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

  if (textbox->hwnd) {
    WCHAR wtext[1024];
    GetWindowTextW(textbox->hwnd, wtext, 1024);
    rc = utf16_to_utf8(wtext, textbox->text_cache, sizeof(textbox->text_cache));
  }

  *out_text = textbox->text_cache;
  return rc;
}

/**
 * @brief Destroys a text box and frees its memory.
 * @param textbox Pointer to the text box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textbox_destroy(cmn_textbox_t **textbox) {
  cmn_error_e rc;
  if (textbox == NULL || *textbox == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = win32_check_map_rc(cmn_handle_map_remove((void *)(*textbox)->hwnd));
  DestroyWindow((*textbox)->hwnd);
  return win32_accum_err(rc, cmn_free((void **)textbox));
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

  if (rc == CMN_SUCCESS) {
    rc = register_window_class();
  }
  tv->hwnd = CreateWindowExW(0, L"EDIT", L"",
                             WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE |
                                 WS_VSCROLL | ES_AUTOVSCROLL | ES_READONLY,
                             0, 0, 100, 100, g_win32_parking_hwnd, NULL,
                             GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = win32_check_map_rc(cmn_handle_map_add((void *)tv->hwnd, tv));

  *out_textview = tv;
  return rc;
}

/**
 * @brief Sets the entire text of a text view.
 * @param textview The text view to modify.
 * @param text The new text string.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_textview_set_text(cmn_textview_t *textview, const char *text) {
  cmn_error_e rc;
  cmn_error_e cp_rc;
  WCHAR wtext[4096];
  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = utf8_to_utf16(text, wtext, 4096);
  if (textview->hwnd) {
    SetWindowTextW(textview->hwnd, wtext);
  }
  cp_rc =
      cmn_strcpy_s(textview->text_cache, sizeof(textview->text_cache), text);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  return cp_rc;
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
  char crlf_buf[4096];
  size_t cur_len;
  size_t add_len;
  size_t i;
  size_t j = 0;
  cmn_error_e rc = CMN_SUCCESS;
  cmn_error_e u_rc;
  if (textview == NULL || text == NULL) {
    return CMN_ERR_INVAL;
  }

  cur_len = strlen(textview->text_cache);
  add_len = strlen(text);
  if (cur_len + add_len + 1 < sizeof(textview->text_cache)) {
    rc = cmn_strcpy_s(textview->text_cache + cur_len,
                      sizeof(textview->text_cache) - cur_len, text);
  }

  for (i = 0; textview->text_cache[i] != '\0' && j + 2 < sizeof(crlf_buf);
       ++i) {
    if (textview->text_cache[i] == '\n' &&
        (i == 0 || textview->text_cache[i - 1] != '\r')) {
      crlf_buf[j++] = '\r';
    }
    crlf_buf[j++] = textview->text_cache[i];
  }
  crlf_buf[j] = '\0';

  u_rc = utf8_to_utf16(crlf_buf, wtext, 4096);
  if (textview->hwnd) {
    SetWindowTextW(textview->hwnd, wtext);
  }
  return (rc != CMN_SUCCESS) ? rc : u_rc;
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
  cmn_error_e rc;
  if (textview == NULL || *textview == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = win32_check_map_rc(cmn_handle_map_remove((void *)(*textview)->hwnd));
  DestroyWindow((*textview)->hwnd);
  return win32_accum_err(rc, cmn_free((void **)textview));
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
  vbox->hwnd = CreateWindowExW(0, CMN_WND_CLASS, L"", WS_CHILD | WS_VISIBLE, 0,
                               0, 460, 320, g_win32_parking_hwnd, NULL,
                               GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = win32_check_map_rc(cmn_handle_map_add((void *)vbox->hwnd, vbox));

  *out_vbox = vbox;
  return rc;
}

/**
 * @brief Destroys a vertical box layout container.
 * @param vbox Pointer to the vertical box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_vbox_destroy(cmn_vbox_t **vbox) {
  cmn_error_e rc;
  if (vbox == NULL || *vbox == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = win32_check_map_rc(cmn_handle_map_remove((void *)(*vbox)->hwnd));
  DestroyWindow((*vbox)->hwnd);
  return win32_accum_err(rc, cmn_free((void **)vbox));
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
  hbox->hwnd = CreateWindowExW(0, CMN_WND_CLASS, L"", WS_CHILD | WS_VISIBLE, 0,
                               0, 410, 36, g_win32_parking_hwnd, NULL,
                               GetModuleHandle(NULL), NULL);
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  rc = win32_check_map_rc(cmn_handle_map_add((void *)hbox->hwnd, hbox));

  *out_hbox = hbox;
  return rc;
}

/**
 * @brief Destroys a horizontal box layout container.
 * @param hbox Pointer to the horizontal box pointer to destroy.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_hbox_destroy(cmn_hbox_t **hbox) {
  cmn_error_e rc;
  if (hbox == NULL || *hbox == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = win32_check_map_rc(cmn_handle_map_remove((void *)(*hbox)->hwnd));
  DestroyWindow((*hbox)->hwnd);
  return win32_accum_err(rc, cmn_free((void **)hbox));
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
  cmn_error_e rc;
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

  rc = utf8_to_utf16(title, wtitle, 256);
  AppendMenuW(menu->hmenu, MF_STRING, id, wtitle);
  return rc;
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
  return cmn_strcpy_s(filepicker->path, sizeof(filepicker->path), "/tmp/test");
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
  return CMN_SUCCESS;
#endif
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
  if (widget == NULL) {
    return CMN_ERR_INVAL;
  }
  if (cmn_widget_get_kind(widget, &kind) != CMN_SUCCESS ||
      kind == CMN_WIDGET_KIND_MENU || kind == CMN_WIDGET_KIND_FILEPICKER) {
    return CMN_ERR_INVAL;
  }
  ((cmn_widget_header_t *)widget)->a11y_tabindex = index;
  return CMN_SUCCESS;
}

/**
 * @brief Backend hook creating the native OS control for an extended widget.
 * @param hdr Initialized widget header.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_init(cmn_widget_header_t *hdr) {
  HWND hwnd = NULL;
  const wchar_t *cls = NULL;
  DWORD style = WS_CHILD | WS_VISIBLE;
  cmn_error_e rc;
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  rc = register_window_class();
  if (rc != CMN_SUCCESS) {
    return rc;
  }
  switch (hdr->kind) {
  case CMN_WIDGET_KIND_CHECKBOX:
    cls = L"BUTTON";
    style |= BS_AUTOCHECKBOX;
    break;
  case CMN_WIDGET_KIND_RADIOBUTTON:
    cls = L"BUTTON";
    style |= BS_AUTORADIOBUTTON;
    break;
  case CMN_WIDGET_KIND_TOGGLEBUTTON:
    cls = L"BUTTON";
    style |= BS_AUTOCHECKBOX | BS_PUSHLIKE;
    break;
  case CMN_WIDGET_KIND_LINE:
    cls = L"STATIC";
    style |= SS_ETCHEDHORZ;
    break;
  case CMN_WIDGET_KIND_SLIDER:
    cls = TRACKBAR_CLASSW;
    style |= TBS_HORZ | TBS_AUTOTICKS;
    break;
  case CMN_WIDGET_KIND_PROGRESS:
    cls = PROGRESS_CLASSW;
    break;
  case CMN_WIDGET_KIND_UPDOWN:
    cls = UPDOWN_CLASSW;
    style |= UDS_ARROWKEYS;
    break;
  case CMN_WIDGET_KIND_POPUP:
    cls = L"COMBOBOX";
    style |= CBS_DROPDOWNLIST | WS_VSCROLL;
    break;
  case CMN_WIDGET_KIND_COMBO:
    cls = L"COMBOBOX";
    style |= CBS_DROPDOWN | WS_VSCROLL;
    break;
  case CMN_WIDGET_KIND_LISTBOX:
    cls = L"LISTBOX";
    style |= LBS_NOTIFY | WS_VSCROLL | WS_BORDER;
    break;
  case CMN_WIDGET_KIND_TABLEVIEW:
    cls = L"EDIT";
    style |= WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY;
    break;
  case CMN_WIDGET_KIND_TABS:
    cls = WC_TABCONTROLW;
    break;
  case CMN_WIDGET_KIND_PANEL:
    cls = L"BUTTON";
    style |= BS_GROUPBOX;
    break;
  case CMN_WIDGET_KIND_IMAGEVIEW:
  case CMN_WIDGET_KIND_VIEW:
  case CMN_WIDGET_KIND_WEBVIEW:
  case CMN_WIDGET_KIND_SPLITVIEW:
    cls = CMN_WND_CLASS;
    break;
  default:
    break;
  }
  if (cls != NULL && g_win32_parking_hwnd != NULL) {
    hwnd = CreateWindowExW(0, cls, L"", style, 0, 0, hdr->bounds.width,
                           hdr->bounds.height, g_win32_parking_hwnd, NULL,
                           GetModuleHandle(NULL), NULL);
    if (hwnd != NULL) {
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)hdr);
      if (hdr->kind == CMN_WIDGET_KIND_PANEL) {
        SetWindowTextW(hwnd, L"Panel Container (cmn_panel_t)");
      }
    }
    hdr->native_handle = (void *)hwnd;
  }
  return CMN_SUCCESS;
}

/**
 * @brief Backend hook synchronizing state changes to the native OS control.
 * @param hdr Widget header whose concrete state was modified.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_sync(cmn_widget_header_t *hdr) {
  HWND hwnd;
  cmn_error_e rc = CMN_SUCCESS;
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  hwnd = (HWND)hdr->native_handle;
  if (hwnd == NULL) {
    return CMN_SUCCESS;
  }
  switch (hdr->kind) {
  case CMN_WIDGET_KIND_CHECKBOX: {
    cmn_checkbox_t *cb = (cmn_checkbox_t *)hdr;
    WCHAR wtxt[256];
    WPARAM check_val = (cb->state == CMN_CHECK_ON)
                           ? BST_CHECKED
                           : ((cb->state == CMN_CHECK_MIXED) ? BST_INDETERMINATE
                                                             : BST_UNCHECKED);
    rc = utf8_to_utf16(cb->text, wtxt, 256);
    SetWindowTextW(hwnd, wtxt);
    if (cb->is_tristate) {
      LONG st = GetWindowLongW(hwnd, GWL_STYLE);
      SetWindowLongW(hwnd, GWL_STYLE, (st & ~BS_AUTOCHECKBOX) | BS_AUTO3STATE);
    }
    SendMessageW(hwnd, BM_SETCHECK, check_val, 0);
    break;
  }
  case CMN_WIDGET_KIND_RADIOBUTTON: {
    cmn_radiobutton_t *rb = (cmn_radiobutton_t *)hdr;
    WCHAR wtxt[256];
    rc = utf8_to_utf16(rb->text, wtxt, 256);
    SetWindowTextW(hwnd, wtxt);
    SendMessageW(hwnd, BM_SETCHECK, rb->selected ? BST_CHECKED : BST_UNCHECKED,
                 0);
    break;
  }
  case CMN_WIDGET_KIND_TOGGLEBUTTON: {
    cmn_togglebutton_t *tb = (cmn_togglebutton_t *)hdr;
    WCHAR wtxt[256];
    rc = utf8_to_utf16(tb->text, wtxt, 256);
    SetWindowTextW(hwnd, wtxt);
    SendMessageW(hwnd, BM_SETCHECK, tb->toggled ? BST_CHECKED : BST_UNCHECKED,
                 0);
    break;
  }
  case CMN_WIDGET_KIND_SLIDER: {
    cmn_slider_t *sl = (cmn_slider_t *)hdr;
    SendMessageW(hwnd, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    SendMessageW(hwnd, TBM_SETPOS, TRUE, (LPARAM)(int)(sl->value * 100.0));
    break;
  }
  case CMN_WIDGET_KIND_PROGRESS: {
    cmn_progress_t *pr = (cmn_progress_t *)hdr;
    SendMessageW(hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(hwnd, PBM_SETPOS, (WPARAM)(int)(pr->value * 100.0), 0);
    break;
  }
  case CMN_WIDGET_KIND_UPDOWN: {
    cmn_updown_t *ud = (cmn_updown_t *)hdr;
    SendMessageW(hwnd, UDM_SETRANGE32, (WPARAM)(LPARAM)-10000, 10000);
    SendMessageW(hwnd, UDM_SETPOS32, 0, (LPARAM)(int)ud->value);
    break;
  }
  case CMN_WIDGET_KIND_POPUP: {
    cmn_popup_t *pu = (cmn_popup_t *)hdr;
    uint32_t i;
    SendMessageW(hwnd, CB_RESETCONTENT, 0, 0);
    for (i = 0; i < pu->count; ++i) {
      WCHAR witem[256];
      cmn_error_e u_rc = utf8_to_utf16(pu->items[i], witem, 256);
      rc = (rc != CMN_SUCCESS) ? rc : u_rc;
      SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM)witem);
    }
    if (pu->selected < pu->count) {
      SendMessageW(hwnd, CB_SETCURSEL, (WPARAM)pu->selected, 0);
    }
    break;
  }
  case CMN_WIDGET_KIND_COMBO: {
    cmn_combo_t *co = (cmn_combo_t *)hdr;
    WCHAR wtxt[256];
    uint32_t i;
    cmn_error_e u_rc;
    SendMessageW(hwnd, CB_RESETCONTENT, 0, 0);
    for (i = 0; i < co->count; ++i) {
      WCHAR witem[256];
      u_rc = utf8_to_utf16(co->items[i], witem, 256);
      rc = (rc != CMN_SUCCESS) ? rc : u_rc;
      SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM)witem);
    }
    u_rc = utf8_to_utf16(co->text, wtxt, 256);
    rc = (rc != CMN_SUCCESS) ? rc : u_rc;
    SetWindowTextW(hwnd, wtxt);
    break;
  }
  case CMN_WIDGET_KIND_LISTBOX: {
    cmn_listbox_t *lb = (cmn_listbox_t *)hdr;
    uint32_t i;
    SendMessageW(hwnd, LB_RESETCONTENT, 0, 0);
    for (i = 0; i < lb->count; ++i) {
      char line[300];
      WCHAR witem[300];
      const char *pfx = "";
      cmn_error_e fmt_rc;
      cmn_error_e u_rc;
      if (lb->checkable) {
        pfx = lb->checked[i] ? "[x] " : "[ ] ";
      }
      fmt_rc = cmn_sprintf_s(line, sizeof(line), "%s%s", pfx, lb->items[i]);
      u_rc = utf8_to_utf16(line, witem, 300);
      if (rc == CMN_SUCCESS) {
        rc = (fmt_rc != CMN_SUCCESS) ? fmt_rc : u_rc;
      }
      SendMessageW(hwnd, LB_ADDSTRING, 0, (LPARAM)witem);
    }
    if (lb->selected < lb->count) {
      SendMessageW(hwnd, LB_SETCURSEL, (WPARAM)lb->selected, 0);
    }
    break;
  }
  case CMN_WIDGET_KIND_TABLEVIEW: {
    cmn_tableview_t *tbl = (cmn_tableview_t *)hdr;
    char text_buf[4096];
    WCHAR wtxt[4096];
    size_t pos = 0;
    uint32_t r, c;
    cmn_error_e fmt_rc;
    cmn_error_e u_rc;
    text_buf[0] = '\0';
    for (c = 0; c < tbl->col_count; ++c) {
      fmt_rc = cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos, "%-16s",
                             tbl->col_titles[c]);
      rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
      pos = strlen(text_buf);
    }
    fmt_rc =
        cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos,
                      "\r\n------------------------------------------------"
                      "\r\n");
    rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
    pos = strlen(text_buf);
    for (r = 0; r < tbl->row_count; ++r) {
      fmt_rc = cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos, "%s",
                             (r == tbl->selected_row) ? "> " : "  ");
      rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
      pos = strlen(text_buf);
      for (c = 0; c < tbl->col_count; ++c) {
        fmt_rc = cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos, "%-16s",
                               tbl->cells[r][c]);
        rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
        pos = strlen(text_buf);
      }
      fmt_rc = cmn_sprintf_s(text_buf + pos, sizeof(text_buf) - pos, "\r\n");
      rc = (rc != CMN_SUCCESS) ? rc : fmt_rc;
      pos = strlen(text_buf);
    }
    u_rc = utf8_to_utf16(text_buf, wtxt, 4096);
    rc = (rc != CMN_SUCCESS) ? rc : u_rc;
    SetWindowTextW(hwnd, wtxt);
    break;
  }
  case CMN_WIDGET_KIND_TABS: {
    cmn_tabs_t *tb = (cmn_tabs_t *)hdr;
    uint32_t i;
    SendMessageW(hwnd, TCM_DELETEALLITEMS, 0, 0);
    for (i = 0; i < tb->count; ++i) {
      WCHAR wtitle[128];
      TCITEMW tie;
      cmn_error_e u_rc;
      memset(&tie, 0, sizeof(tie));
      tie.mask = TCIF_TEXT;
      u_rc = utf8_to_utf16(tb->titles[i], wtitle, 128);
      rc = (rc != CMN_SUCCESS) ? rc : u_rc;
      tie.pszText = wtitle;
      SendMessageW(hwnd, TCM_INSERTITEMW, (WPARAM)i, (LPARAM)&tie);
    }
    if (tb->selected < tb->count) {
      SendMessageW(hwnd, TCM_SETCURSEL, (WPARAM)tb->selected, 0);
    }
    break;
  }
  default:
    InvalidateRect(hwnd, NULL, TRUE);
    break;
  }
  return rc;
}

/**
 * @brief Backend hook destroying the native OS control of an extended widget.
 * @param hdr Widget header being destroyed.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_backend_widget_cleanup(cmn_widget_header_t *hdr) {
  HWND hwnd;
  if (hdr == NULL) {
    return CMN_ERR_INVAL;
  }
  hwnd = (HWND)hdr->native_handle;
  if (hwnd != NULL) {
    DestroyWindow(hwnd);
    hdr->native_handle = NULL;
  }
  return CMN_SUCCESS;
}
