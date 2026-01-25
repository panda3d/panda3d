/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_android.h
 * @author rdb
 * @date 2013-01-12
 */

#ifndef CONFIG_ANDROID_H
#define CONFIG_ANDROID_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

#include <android/native_activity.h>
#include <jni.h>

NotifyCategoryDecl(android, EXPORT_CLASS, EXPORT_TEMPL);
extern void init_libandroid();

extern EXPORT_CLASS struct android_app* panda_android_app;

extern jclass    jni_PandaActivity;
extern jmethodID jni_PandaActivity_readBitmapHeader;
extern jmethodID jni_PandaActivity_readBitmap;
extern jmethodID jni_PandaActivity_createBitmap;
extern jmethodID jni_PandaActivity_compressBitmap;
extern jmethodID jni_PandaActivity_setWindowTitle;
extern jmethodID jni_PandaActivity_showToast;

extern jclass   jni_BitmapFactory_Options;
extern jfieldID jni_BitmapFactory_Options_outWidth;
extern jfieldID jni_BitmapFactory_Options_outHeight;

EXPORT_CLASS Filename android_find_library(ANativeActivity *activity, const std::string &lib);
EXPORT_CLASS void android_set_title(ANativeActivity *activity, const std::string &title);
EXPORT_CLASS void android_show_toast(ANativeActivity *activity, const std::string &message, int duration);

// Used to support pyjnius
extern "C" {
  EXPORT_CLASS void *SDL_AndroidGetJNIEnv(void);
};

#endif
