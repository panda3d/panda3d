// Filename: config_express.h
// Created by:  cary (04Jan00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_EXPRESS_H__
#define __CONFIG_EXPRESS_H__

#include <pandabase.h>
#include <notifyCategoryProxy.h>
#include <dconfig.h>

ConfigureDecl(config_express, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);
NotifyCategoryDecl(express, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);

// Actually, we can't determine this config variable the normal way,
// because we must be able to access it at static init time.  Instead
// of declaring it a global constant, we'll make it a member of
// MemoryUsage.

//extern EXPCL_PANDAEXPRESS const bool track_memory_usage;

bool EXPCL_PANDAEXPRESS get_leak_memory();
bool EXPCL_PANDAEXPRESS get_never_destruct();
bool EXPCL_PANDAEXPRESS get_use_high_res_clock();

extern const int patchfile_window_size;
extern const int patchfile_increment_size;
extern const int patchfile_buffer_size;
extern const int patchfile_zone_size;

#endif /* __CONFIG_UTIL_H__ */
