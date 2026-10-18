/* clang-format off */
#include "screenshot_gen.h"
#include "cmn/string_safe.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file main.c
 * @brief Command-line entry point for the real native OS screenshot generator.
 */

/**
 * @brief Runs the native screenshot generator with optional
 * `--output-dir <path>` and `--platform <name>`.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Process exit code (`0` on `CMN_SUCCESS`).
 */
int main(int argc, char **argv) {
  const char *out_dir = "../cc0-assets/c-multiplatform-native/screenshots";
  const char *plat = NULL;
  const char *web_html_dir = NULL;
  cmn_error_e rc;
  int i;

  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--output-dir") == 0 && i + 1 < argc) {
      out_dir = argv[i + 1];
      ++i;
    } else if (strcmp(argv[i], "--platform") == 0 && i + 1 < argc) {
      plat = argv[i + 1];
      ++i;
    } else if (strcmp(argv[i], "--emit-web-html") == 0 && i + 1 < argc) {
      web_html_dir = argv[i + 1];
      ++i;
    }
  }

  if (web_html_dir != NULL) {
    static const char *const names[4] = {"button_web.html", "inputs_web.html",
                                         "hello_world_web.html",
                                         "password_flow_web.html"};
    for (i = 0; i < 4; ++i) {
      char html_path[1024];
      (void)cmn_sprintf_s(html_path, sizeof(html_path), "%s/%s", web_html_dir,
                          names[i]);
      rc = cmn_screenshot_write_web_html(i, html_path);
      if (rc != CMN_SUCCESS) {
        return (int)rc;
      }
    }
    return 0;
  }

  rc = cmn_screenshot_generate_all(out_dir, plat);
  if (rc != CMN_SUCCESS) {
    printf("Screenshot generation failed: " CMN_INT32_FMT "%c", (int)rc, 10);
    return (int)rc;
  }

  printf("Captured native screenshots in %s%c", out_dir, 10);
  return 0;
}
