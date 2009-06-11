// Filename: config_pgraphnodes.h
// Created by:  drose (05Nov08)
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

#ifndef CONFIG_PGRAPHNODES_H
#define CONFIG_PGRAPHNODES_H

#include "pandabase.h"
#include "lodNodeType.h"
#include "configVariableEnum.h"

class DSearchPath;

ConfigureDecl(config_pgraphnodes, EXPCL_PANDA_PGRAPHNODES, EXPTP_PANDA_PGRAPHNODES);
NotifyCategoryDecl(pgraphnodes, EXPCL_PANDA_PGRAPHNODES, EXPTP_PANDA_PGRAPHNODES);

extern ConfigVariableEnum<LODNodeType> default_lod_type;

extern EXPCL_PANDA_PGRAPHNODES void init_libpgraphnodes();

#endif
