// Filename: config_express.h
// Created by:  cary (04Jan00)
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

#ifndef __CONFIG_EXPRESS_H__
#define __CONFIG_EXPRESS_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableList.h"
#include "configVariableFilename.h"

// Include these so interrogate can find them.
#include "executionEnvironment.h"
#include "lineStream.h"

#ifdef ANDROID
#include <jni.h>
#endif

ConfigureDecl(config_express, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);
NotifyCategoryDecl(express, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);
NotifyCategoryDecl(clock, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);

// Actually, we can't determine this config variable the normal way,
// because we must be able to access it at static init time.  Instead
// of declaring it a global constant, we'll make it a member of
// MemoryUsage.

//extern EXPCL_PANDAEXPRESS const bool track_memory_usage;

EXPCL_PANDAEXPRESS bool get_use_high_res_clock();
EXPCL_PANDAEXPRESS bool get_paranoid_clock();
EXPCL_PANDAEXPRESS bool get_paranoid_inheritance();
EXPCL_PANDAEXPRESS bool get_verify_dcast();

extern ConfigVariableInt patchfile_window_size;
extern ConfigVariableInt patchfile_increment_size;
extern ConfigVariableInt patchfile_buffer_size;
extern ConfigVariableInt patchfile_zone_size;

extern ConfigVariableBool keep_temporary_files;
extern ConfigVariableBool multifile_always_binary;

extern EXPCL_PANDAEXPRESS ConfigVariableBool collect_tcp;
extern EXPCL_PANDAEXPRESS ConfigVariableDouble collect_tcp_interval;

// Expose the Config variable for Python access.
BEGIN_PUBLISH
EXPCL_PANDAEXPRESS DConfig &get_config_express();
END_PUBLISH

extern EXPCL_PANDAEXPRESS void init_libexpress();

#ifdef ANDROID
extern EXPCL_PANDAEXPRESS JavaVM *get_java_vm();
extern EXPCL_PANDAEXPRESS JNIEnv *get_jni_env();
#endif

#endif /* __CONFIG_UTIL_H__ */
