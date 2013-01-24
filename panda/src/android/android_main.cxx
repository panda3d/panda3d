// Filename: android_main.cxx
// Created by:  rdb (12Jan13)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_android.h"
#include "config_util.h"
#include "virtualFileMountAndroidAsset.h"
#include "virtualFileSystem.h"

#include "config_display.h"
//#define OPENGLES_1
//#include "config_androiddisplay.h"

#include <android_native_app_glue.h>

//struct android_app* panda_android_app = NULL;

extern int main(int argc, char **argv);

////////////////////////////////////////////////////////////////////
//     Function: android_main
//  Description: This function is called by native_app_glue to
//               initialize the program.  It simply stores the
//               android_app object and calls main() normally.
////////////////////////////////////////////////////////////////////
void android_main(struct android_app* app) {
  panda_android_app = app;

  // Attach the current thread to the JVM.
  JNIEnv *env;
  ANativeActivity* activity = app->activity;
  int status = activity->vm->AttachCurrentThread(&env, NULL);
  if (status < 0 || env == NULL) {
    android_cat.error() << "Failed to attach thread to JVM!\n";
    return;
  }

  // Get the path to the APK.
  jclass clazz = env->GetObjectClass(activity->clazz);
  jmethodID methodID = env->GetMethodID(clazz, "getPackageCodePath", "()Ljava/lang/String;");
  jstring code_path = (jstring) env->CallObjectMethod(activity->clazz, methodID);

  const char* apk_path;
  apk_path = env->GetStringUTFChars(code_path, NULL);
  android_cat.info() << "Path to APK: " << apk_path << "\n";

  // Mount the assets directory.
  PT(VirtualFileMountAndroidAsset) asset_mount;
  asset_mount = new VirtualFileMountAndroidAsset(app->activity->assetManager, apk_path);
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->mount(asset_mount, "/android_asset", 0);

  // Release the apk_path.
  env->ReleaseStringUTFChars(code_path, apk_path);

  // Now add the asset directory to the model-path.
  get_model_path().append_directory("/android_asset");

  // Create bogus argc and argv, then call our main function.
  char *argv[] = {NULL};
  int argc = 0;
  main(argc, argv);

  // Detach the thread before exiting.
  activity->vm->DetachCurrentThread();
}
