// Filename: config_loader.h
// Created by:  drose (19Mar00)
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

#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <pandabase.h>

#include <dconfig.h>
#include <notifyCategoryProxy.h>

class DSearchPath;

NotifyCategoryDecl(loader, EXPCL_PANDA, EXPTP_PANDA);

extern const bool asynchronous_loads;
const DSearchPath &get_bam_path();

extern Config::ConfigTable::Symbol *load_file_type;

#endif
