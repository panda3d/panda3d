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
#include "thread.h"

#include "config_display.h"
// #define OPENGLES_1 #include "config_androiddisplay.h"

#include <android_native_app_glue.h>

// struct android_app* panda_android_app = NULL;

extern int main(int argc, char **argv);

/**
 * This function is called by native_app_glue to initialize the program.  It
 * simply stores the android_app object and calls main() normally.
 *
 * Note that this does not run in the main thread, but in a thread created
 * specifically for this activity by android_native_app_glue.
 */
void android_main(struct android_app* app) {
  panda_android_app = app;

  // Attach the app thread to the Java VM.
  JNIEnv *env;
  ANativeActivity* activity = app->activity;
  int status = activity->vm->AttachCurrentThread(&env, nullptr);
  if (status < 0 || env == nullptr) {
    android_cat.error() << "Failed to attach thread to JVM!\n";
    return;
  }

  jclass activity_class = env->GetObjectClass(activity->clazz);

  // Get the current Java thread name.  This just helps with debugging.
  jmethodID methodID = env->GetStaticMethodID(activity_class, "getCurrentThreadName", "()Ljava/lang/String;");
  jstring jthread_name = (jstring) env->CallStaticObjectMethod(activity_class, methodID);

  string thread_name;
  if (jthread_name != nullptr) {
    const char *c_str = env->GetStringUTFChars(jthread_name, nullptr);
    thread_name.assign(c_str);
    env->ReleaseStringUTFChars(jthread_name, c_str);
  }

  // Before we make any Panda calls, we must make the thread known to Panda.
  // This will also cause the JNIEnv pointer to be stored on the thread.
  // Note that we must keep a reference to this thread around.
  PT(Thread) current_thread = Thread::bind_thread(thread_name, "android_app");

  android_cat.info()
    << "New native activity started on " << *current_thread << "\n";

  // Fetch the data directory.
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
  methodID = env->GetMethodID(activity_class, "getPackageCodePath", "()Ljava/lang/String;");
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

  // Create bogus argc and argv for calling the main function.
  char *argv[] = {nullptr};
  int argc = 0;

  while (!app->destroyRequested) {
    // Call the main function.  This will not return until the app is done.
    android_cat.info() << "Calling main()\n";
    main(argc, argv);

    if (app->destroyRequested) {
      // The app closed responding to a destroy request.
      break;
    }

    // Ask Android to clean up the activity.
    android_cat.info() << "Exited from main(), finishing activity\n";
    ANativeActivity_finish(activity);

    // We still need to keep an event loop going until Android gives us leave
    // to end the process.
    int looper_id;
    int events;
    struct android_poll_source *source;
    while ((looper_id = ALooper_pollAll(-1, nullptr, &events, (void**)&source)) >= 0) {
      // Process this event, but intercept application command events.
      if (looper_id == LOOPER_ID_MAIN) {
        int8_t cmd = android_app_read_cmd(app);
        android_app_pre_exec_cmd(app, cmd);
        android_app_post_exec_cmd(app, cmd);

        // I don't think we can get a resume command after we call finish(),
        // but let's handle it just in case.
        if (cmd == APP_CMD_RESUME ||
            cmd == APP_CMD_DESTROY) {
          break;
        }
      } else if (source != nullptr) {
        source->process(app, source);
      }
    }
  }

  android_cat.info() << "Destroy requested, exiting from android_main\n";

  // Detach the thread before exiting.
  activity->vm->DetachCurrentThread();
}
