// Filename: config_interrogatedb.h
// Created by:  drose (01Aug00)
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

#ifndef CONFIG_INTERROGATEDB_H
#define CONFIG_INTERROGATEDB_H

#include "dtoolbase.h"
#include "notifyCategoryProxy.h"
#include "configVariableSearchPath.h"

NotifyCategoryDecl(interrogatedb, EXPCL_INTERROGATEDB, EXPTP_INTERROGATEDB);

extern ConfigVariableSearchPath interrogatedb_path;

#endif
