// Filename: directRenderLevelState.h
// Created by:  drose (17Apr00)
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

#ifndef DIRECTRENDERLEVELSTATE_H
#define DIRECTRENDERLEVELSTATE_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : DirectRenderLevelState
// Description : This is the state information the
//               DirectRenderTraverser retains for each level during
//               traversal.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DirectRenderLevelState {
public:
  bool _decal_mode;
};

#endif

