// Filename: config_android.h
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

#ifndef CONFIG_ANDROID_H
#define CONFIG_ANDROID_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

#include <jni.h>

NotifyCategoryDeclNoExport(android);
extern void init_libandroid();

extern struct android_app* panda_android_app;

extern jclass    jni_PandaActivity;
extern jmethodID jni_PandaActivity_readBitmapHeader;
extern jmethodID jni_PandaActivity_readBitmap;

extern jclass   jni_BitmapFactory_Options;
extern jfieldID jni_BitmapFactory_Options_outWidth;
extern jfieldID jni_BitmapFactory_Options_outHeight;

#endif
