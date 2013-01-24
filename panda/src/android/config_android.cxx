// Filename: config_android.cxx
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
#include "pnmFileTypeAndroid.h"
#include "pnmFileTypeRegistry.h"
#include "dconfig.h"
#include "pandaSystem.h"

NotifyCategoryDef(android, "");

struct android_app *panda_android_app = NULL;

jclass    jni_PandaActivity;
jmethodID jni_PandaActivity_readBitmapSize;
jmethodID jni_PandaActivity_readBitmap;

jclass   jni_BitmapFactory_Options;
jfieldID jni_BitmapFactory_Options_outWidth;
jfieldID jni_BitmapFactory_Options_outHeight;

////////////////////////////////////////////////////////////////////
//     Function: init_libandroid
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes
//               in this library can be used.  Normally, this is
//               called by JNI_OnLoad.
////////////////////////////////////////////////////////////////////
void
init_libandroid() {
  PNMFileTypeRegistry *tr = PNMFileTypeRegistry::get_global_ptr();
  PNMFileTypeAndroid::init_type();
  PNMFileTypeAndroid::register_with_read_factory();
  tr->register_type(new PNMFileTypeAndroid);
}

////////////////////////////////////////////////////////////////////
//     Function: JNI_OnLoad
//  Description: Called by Java when loading this library.
//               Initializes the global class references and the
//               method IDs.
////////////////////////////////////////////////////////////////////
jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
  init_libandroid();

  JNIEnv *env = get_jni_env();
  assert(env != NULL);

  jni_PandaActivity = env->FindClass("org/panda3d/android/PandaActivity");
  jni_PandaActivity = (jclass) env->NewGlobalRef(jni_PandaActivity);

  jni_PandaActivity_readBitmapSize = env->GetStaticMethodID(jni_PandaActivity,
                   "readBitmapSize", "(J)Landroid/graphics/BitmapFactory$Options;");

  jni_PandaActivity_readBitmap = env->GetStaticMethodID(jni_PandaActivity,
                   "readBitmap", "(JI)Landroid/graphics/Bitmap;");

  jni_BitmapFactory_Options = env->FindClass("android/graphics/BitmapFactory$Options");
  jni_BitmapFactory_Options = (jclass) env->NewGlobalRef(jni_BitmapFactory_Options);

  jni_BitmapFactory_Options_outWidth = env->GetFieldID(jni_BitmapFactory_Options, "outWidth", "I");
  jni_BitmapFactory_Options_outHeight = env->GetFieldID(jni_BitmapFactory_Options, "outHeight", "I");

  return JNI_VERSION_1_4;
}

////////////////////////////////////////////////////////////////////
//     Function: JNI_OnUnload
//  Description: Called by Java when unloading this library.
//               Destroys the global class references.
////////////////////////////////////////////////////////////////////
void JNI_OnUnload(JavaVM *jvm, void *reserved) {
  JNIEnv *env = get_jni_env();

  env->DeleteGlobalRef(jni_PandaActivity);
  env->DeleteGlobalRef(jni_BitmapFactory_Options);
}
