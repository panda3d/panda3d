// Filename: config_interrogatedb.h
// Created by:  drose (01Aug00)
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

#ifndef CONFIG_INTERROGATEDB_H
#define CONFIG_INTERROGATEDB_H

#include "dtoolbase.h"
#include "notifyCategoryProxy.h"
#include "dSearchPath.h"

NotifyCategoryDecl(interrogatedb, EXPCL_DTOOLCONFIG, EXPTP_DTOOLCONFIG);

const DSearchPath &get_interrogatedb_path();

#endif
