// Filename: pStatProperties.h
// Created by:  drose (17May01)
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

#ifndef PSTATPROPERTIES_H
#define PSTATPROPERTIES_H

#include "pandabase.h"


class PStatClient;
class PStatCollectorDef;

EXPCL_PANDA int get_current_pstat_major_version();
EXPCL_PANDA int get_current_pstat_minor_version();

#ifdef DO_PSTATS
void initialize_collector_def(PStatClient *client, PStatCollectorDef *def);
#endif  // DO_PSTATS

#endif

