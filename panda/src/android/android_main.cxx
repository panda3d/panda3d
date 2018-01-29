/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file android_main.cxx
 * @author rdb
 * @date 2013-01-12
 */

#include "config_android.h"
#include "config_util.h"
#include "virtualFileMountAndroidAsset.h"
#include "virtualFileSystem.h"
#include "filename.h"

#include "config_display.h"
// #define OPENGLES_1 #include "config_androiddisplay.h"

#include <android_native_app_glue.h>

// struct android_app* panda_android_app = NULL;

extern int main(int argc, char **argv);

/**
 * This function is called by native_app_glue to initialize the program.  It
 * simply stores the android_app object and calls main() normally.
 */
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

  // Fetch the data directory.
  jclass activity_class = env->GetObjectClass(activity->clazz);
  jmethodID get_appinfo = env->GetMethodID(activity_class, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");

  jobject appinfo = env->CallObjectMethod(activity->clazz, get_appinfo);
  jclass appinfo_class = env->GetObjectClass(appinfo);

  // Fetch the path to the data directory.
  jfieldID datadir_field = env->GetFieldID(appinfo_class, "dataDir", "Ljava/lang/String;");
  jstring datadir = (jstring) env->GetObjectField(appinfo, datadir_field);
  const char *data_path = env->GetStringUTFChars(datadir, nullptr);

  if (data_path != nullptr) {
    Filename::_internal_data_dir = data_path;
    android_cat.info() << "Path to data: " << data_path << "\n";

    env->ReleaseStringUTFChars(datadir, data_path);
  }

  // Fetch the path to the library directory.
  if (ExecutionEnvironment::get_dtool_name().empty()) {
    jfieldID libdir_field = env->GetFieldID(appinfo_class, "nativeLibraryDir", "Ljava/lang/String;");
    jstring libdir = (jstring) env->GetObjectField(appinfo, libdir_field);
    const char *lib_path = env->GetStringUTFChars(libdir, nullptr);

    if (lib_path != nullptr) {
      string dtool_name = string(lib_path) + "/libp3dtool.so";
      ExecutionEnvironment::set_dtool_name(dtool_name);
      android_cat.info() << "Path to dtool: " << dtool_name << "\n";

      env->ReleaseStringUTFChars(libdir, lib_path);
    }
  }

  // Get the path to the APK.
  jmethodID methodID = env->GetMethodID(activity_class, "getPackageCodePath", "()Ljava/lang/String;");
  jstring code_path = (jstring) env->CallObjectMethod(activity->clazz, methodID);

  const char* apk_path;
  apk_path = env->GetStringUTFChars(code_path, nullptr);

  // We're going to set this as binary name, which is better than the
  // default (which refers to the zygote).  Or should we set it to the
  // native library?  How do we get the path to that?
  android_cat.info() << "Path to APK: " << apk_path << "\n";
  ExecutionEnvironment::set_binary_name(apk_path);

  // Mount the assets directory.
  Filename apk_fn(apk_path);
  PT(VirtualFileMountAndroidAsset) asset_mount;
  asset_mount = new VirtualFileMountAndroidAsset(app->activity->assetManager, apk_fn);
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename asset_dir(apk_fn.get_dirname(), "assets");
  vfs->mount(asset_mount, asset_dir, 0);

  // Release the apk_path.
  env->ReleaseStringUTFChars(code_path, apk_path);

  // Now add the asset directory to the model-path.
  //TODO: prevent it from adding the directory multiple times.
  get_model_path().append_directory(asset_dir);

  // Create bogus argc and argv, then call our main function.
  char *argv[] = {NULL};
  int argc = 0;
  main(argc, argv);

  // Detach the thread before exiting.
  activity->vm->DetachCurrentThread();
}
