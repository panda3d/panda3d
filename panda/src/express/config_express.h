// Filename: config_express.h
// Created by:  cary (04Jan00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_EXPRESS_H__
#define __CONFIG_EXPRESS_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "clockObject.h"
#include "dconfig.h"

// We include these files to force them to be instrumented by
// interrogate.
#include "globPattern.h"
#include "configPage.h"
#include "configPageManager.h"
#include "configVariable.h"
#include "configVariableBool.h"
#include "configVariableDouble.h"
#include "configVariableInt.h"
#include "configVariableList.h"
#include "configVariableManager.h"
#include "configVariableString.h"

ConfigureDecl(config_express, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);
NotifyCategoryDecl(express, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);
NotifyCategoryDecl(thread, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);

// Actually, we can't determine this config variable the normal way,
// because we must be able to access it at static init time.  Instead
// of declaring it a global constant, we'll make it a member of
// MemoryUsage.

//extern EXPCL_PANDAEXPRESS const bool track_memory_usage;

EXPCL_PANDAEXPRESS bool get_leak_memory();
EXPCL_PANDAEXPRESS bool get_never_destruct();
EXPCL_PANDAEXPRESS bool get_use_high_res_clock();
EXPCL_PANDAEXPRESS bool get_paranoid_clock();
EXPCL_PANDAEXPRESS bool get_paranoid_inheritance();
EXPCL_PANDAEXPRESS bool get_verify_dcast();

extern const int patchfile_window_size;
extern const int patchfile_increment_size;
extern const int patchfile_buffer_size;
extern const int patchfile_zone_size;

extern const bool keep_temporary_files;
extern const double average_frame_rate_interval;

extern ClockObject::Mode clock_mode;
extern const double clock_frame_rate;
extern const double clock_degrade_factor;
extern const double max_dt;
extern const double sleep_precision;

extern const string encryption_algorithm;
extern const int encryption_key_length;
extern const int encryption_iteration_count;

extern EXPCL_PANDAEXPRESS const bool use_vfs;

extern EXPCL_PANDAEXPRESS const bool collect_tcp;
extern EXPCL_PANDAEXPRESS const double collect_tcp_interval;

// Expose the Config variable for Python access.
BEGIN_PUBLISH
typedef Config::Config<ConfigureGetConfig_config_express> ConfigExpress;
EXPCL_PANDAEXPRESS ConfigExpress &get_config_express();
END_PUBLISH

extern EXPCL_PANDAEXPRESS void init_libexpress();

#endif /* __CONFIG_UTIL_H__ */
