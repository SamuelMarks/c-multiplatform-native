/* clang-format off */
#include "cmn_android_internal.h"
#include "cmn/automation.h"
#include "cmn/error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @file cmn_android_automation.c
 * @brief Android JNI implementation of screenshot capture.
 */

/** @brief Global JavaVM pointer initialized on `JNI_OnLoad`. */
extern JavaVM *g_cmn_jvm;
/** @brief Definition of the global JavaVM pointer. */
JavaVM *g_cmn_jvm = NULL;

/**
 * @brief Sets the Android JavaVM instance and optionally outputs JNI version.
 * @param vm Pointer to the JavaVM instance.
 * @param out_version Optional output pointer to receive `JNI_VERSION_1_6`.
 * @return CMN_SUCCESS on success, CMN_ERR_INVAL if vm is NULL.
 */
cmn_error_e cmn_android_set_jvm(JavaVM *vm, jint *out_version) {
  if (vm == NULL) {
    return CMN_ERR_INVAL;
  }
  g_cmn_jvm = vm;
  if (out_version != NULL) {
    *out_version = JNI_VERSION_1_6;
  }
  return CMN_SUCCESS;
}

/**
 * @brief JNI entry point invoked when the native library is loaded by Android.
 * @param vm Pointer to the JavaVM instance.
 * @param reserved Reserved pointer.
 * @return `CMN_SUCCESS` on success.
 */
JNIEXPORT cmn_error_e JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  (void)reserved;
  return cmn_android_set_jvm(vm, NULL);
}

/**
 * @brief Captures a screenshot of a window and saves it to a file.
 * @param win The window to capture.
 * @param out_filepath The destination file path for the screenshot.
 * @return CMN_SUCCESS on success, error code otherwise.
 */
cmn_error_e cmn_automation_capture_screenshot(cmn_window_t *win,
                                              const char *out_filepath) {
  JNIEnv *env = NULL;
  jint res;
  int attached = 0;

  jclass activityClass = NULL;
  jmethodID getWindowMethod = NULL;
  jobject windowObj = NULL;
  jclass windowClass = NULL;
  jmethodID getDecorViewMethod = NULL;
  jobject decorViewObj = NULL;
  jclass viewClass = NULL;
  jmethodID getWidthMethod = NULL;
  jmethodID getHeightMethod = NULL;
  jint width, height;

  jclass bitmapConfigClass = NULL;
  jfieldID argb8888Field = NULL;
  jobject argb8888Obj = NULL;
  jclass bitmapClass = NULL;
  jmethodID createBitmapMethod = NULL;
  jobject bitmapObj = NULL;

  jclass canvasClass = NULL;
  jmethodID canvasConstructor = NULL;
  jobject canvasObj = NULL;
  jmethodID drawMethod = NULL;

  jclass fileOutputStreamClass = NULL;
  jmethodID fosConstructor = NULL;
  jstring pathString = NULL;
  jobject fosObj = NULL;

  jclass compressFormatClass = NULL;
  jfieldID pngField = NULL;
  jobject pngObj = NULL;
  jmethodID compressMethod = NULL;
  jboolean success;
  jmethodID closeMethod = NULL;

  cmn_error_e err = CMN_SUCCESS;

  if (win == NULL || out_filepath == NULL || out_filepath[0] == '\0') {
    return CMN_ERR_INVAL;
  }

  if (g_cmn_jvm == NULL || win->activity == NULL) {
    return CMN_ERR_INVAL;
  }

  res = (*g_cmn_jvm)->GetEnv(g_cmn_jvm, (void **)&env, JNI_VERSION_1_6);
  if (res == JNI_EDETACHED) {
    if ((*g_cmn_jvm)->AttachCurrentThread(g_cmn_jvm, &env, NULL) != 0) {
      return CMN_ERR_IO;
    }
    attached = 1;
  } else if (res != JNI_OK) {
    return CMN_ERR_IO;
  }

  activityClass = (*env)->GetObjectClass(env, win->activity);
  getWindowMethod = (*env)->GetMethodID(env, activityClass, "getWindow",
                                        "()Landroid/view/Window;");
  windowObj = (*env)->CallObjectMethod(env, win->activity, getWindowMethod);
  if (windowObj == NULL) {
    err = CMN_ERR_IO;
    goto cleanup;
  }

  windowClass = (*env)->GetObjectClass(env, windowObj);
  getDecorViewMethod = (*env)->GetMethodID(env, windowClass, "getDecorView",
                                           "()Landroid/view/View;");
  decorViewObj = (*env)->CallObjectMethod(env, windowObj, getDecorViewMethod);
  if (decorViewObj == NULL) {
    err = CMN_ERR_IO;
    goto cleanup;
  }

  viewClass = (*env)->GetObjectClass(env, decorViewObj);
  getWidthMethod = (*env)->GetMethodID(env, viewClass, "getWidth", "()I");
  getHeightMethod = (*env)->GetMethodID(env, viewClass, "getHeight", "()I");
  width = (*env)->CallIntMethod(env, decorViewObj, getWidthMethod);
  height = (*env)->CallIntMethod(env, decorViewObj, getHeightMethod);

  if (width <= 0 || height <= 0) {
    err = CMN_ERR_IO;
    goto cleanup;
  }

  bitmapConfigClass = (*env)->FindClass(env, "android/graphics/Bitmap$Config");
  argb8888Field = (*env)->GetStaticFieldID(env, bitmapConfigClass, "ARGB_8888",
                                           "Landroid/graphics/Bitmap$Config;");
  argb8888Obj =
      (*env)->GetStaticObjectField(env, bitmapConfigClass, argb8888Field);

  bitmapClass = (*env)->FindClass(env, "android/graphics/Bitmap");
  createBitmapMethod = (*env)->GetStaticMethodID(
      env, bitmapClass, "createBitmap",
      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
  bitmapObj = (*env)->CallStaticObjectMethod(
      env, bitmapClass, createBitmapMethod, width, height, argb8888Obj);
  if (bitmapObj == NULL) {
    err = CMN_ERR_NOMEM;
    goto cleanup;
  }

  canvasClass = (*env)->FindClass(env, "android/graphics/Canvas");
  canvasConstructor = (*env)->GetMethodID(env, canvasClass, "<init>",
                                          "(Landroid/graphics/Bitmap;)V");
  canvasObj = (*env)->NewObject(env, canvasClass, canvasConstructor, bitmapObj);

  drawMethod = (*env)->GetMethodID(env, viewClass, "draw",
                                   "(Landroid/graphics/Canvas;)V");
  (*env)->CallVoidMethod(env, decorViewObj, drawMethod, canvasObj);

  fileOutputStreamClass = (*env)->FindClass(env, "java/io/FileOutputStream");
  fosConstructor = (*env)->GetMethodID(env, fileOutputStreamClass, "<init>",
                                       "(Ljava/lang/String;)V");
  pathString = (*env)->NewStringUTF(env, out_filepath);
  fosObj =
      (*env)->NewObject(env, fileOutputStreamClass, fosConstructor, pathString);

  if ((*env)->ExceptionCheck(env)) {
    (*env)->ExceptionClear(env);
    err = CMN_ERR_IO;
    goto cleanup;
  }

  compressFormatClass =
      (*env)->FindClass(env, "android/graphics/Bitmap$CompressFormat");
  pngField =
      (*env)->GetStaticFieldID(env, compressFormatClass, "PNG",
                               "Landroid/graphics/Bitmap$CompressFormat;");
  pngObj = (*env)->GetStaticObjectField(env, compressFormatClass, pngField);

  compressMethod = (*env)->GetMethodID(
      env, bitmapClass, "compress",
      "(Landroid/graphics/Bitmap$CompressFormat;ILjava/io/OutputStream;)Z");
  success = (*env)->CallBooleanMethod(env, bitmapObj, compressMethod, pngObj,
                                      100, fosObj);

  closeMethod = (*env)->GetMethodID(env, fileOutputStreamClass, "close", "()V");
  (*env)->CallVoidMethod(env, fosObj, closeMethod);
  if ((*env)->ExceptionCheck(env)) {
    (*env)->ExceptionClear(env);
    err = CMN_ERR_IO;
    goto cleanup;
  }

  if (!success) {
    err = CMN_ERR_IO;
  }

cleanup:
  if (pathString)
    (*env)->DeleteLocalRef(env, pathString);
  if (fosObj)
    (*env)->DeleteLocalRef(env, fosObj);
  if (fileOutputStreamClass)
    (*env)->DeleteLocalRef(env, fileOutputStreamClass);
  if (pngObj)
    (*env)->DeleteLocalRef(env, pngObj);
  if (compressFormatClass)
    (*env)->DeleteLocalRef(env, compressFormatClass);
  if (canvasObj)
    (*env)->DeleteLocalRef(env, canvasObj);
  if (canvasClass)
    (*env)->DeleteLocalRef(env, canvasClass);
  if (bitmapObj)
    (*env)->DeleteLocalRef(env, bitmapObj);
  if (bitmapClass)
    (*env)->DeleteLocalRef(env, bitmapClass);
  if (argb8888Obj)
    (*env)->DeleteLocalRef(env, argb8888Obj);
  if (bitmapConfigClass)
    (*env)->DeleteLocalRef(env, bitmapConfigClass);
  if (viewClass)
    (*env)->DeleteLocalRef(env, viewClass);
  if (decorViewObj)
    (*env)->DeleteLocalRef(env, decorViewObj);
  if (windowClass)
    (*env)->DeleteLocalRef(env, windowClass);
  if (windowObj)
    (*env)->DeleteLocalRef(env, windowObj);
  if (activityClass)
    (*env)->DeleteLocalRef(env, activityClass);

  if (attached) {
    (*g_cmn_jvm)->DetachCurrentThread(g_cmn_jvm);
  }

  return err;
}
