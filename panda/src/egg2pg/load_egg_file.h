// Filename: load_egg_file.h
// Created by:  drose (26Feb02)
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

#ifndef LOAD_EGG_FILE_H
#define LOAD_EGG_FILE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "coordinateSystem.h"

class EggData;

BEGIN_PUBLISH
////////////////////////////////////////////////////////////////////
//     Function: load_egg_file
//  Description: A convenience function; the primary interface to this
//               package.  Loads up the indicated egg file, and
//               returns the root of a scene graph.  Returns NULL if
//               the file cannot be read for some reason.
//
//               Also see the EggLoader class, which can exercise a
//               bit more manual control over the loading process.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEGG PT(PandaNode)
load_egg_file(const string &filename, CoordinateSystem cs = CS_default);

////////////////////////////////////////////////////////////////////
//     Function: load_egg_data
//  Description: Another convenience function; works like
//               load_egg_file() but starts from an already-filled
//               EggData structure.  The structure is destroyed in the
//               loading.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEGG PT(PandaNode)
load_egg_data(EggData &data, CoordinateSystem cs = CS_default);
END_PUBLISH

#endif
