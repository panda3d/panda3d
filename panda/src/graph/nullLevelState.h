// Filename: nullLevelState.h
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

#ifndef NULLLEVELSTATE_H
#define NULLLEVELSTATE_H

#include <pandabase.h>

///////////////////////////////////////////////////////////////////
//       Class : NullLevelState
// Description : This is an empty class designed to be passed to a
//               traverser that doesn't care about tracking states
//               between levels.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NullLevelState {
public:
  INLINE NullLevelState() { }
  INLINE NullLevelState(const NullLevelState &) { }
  INLINE void operator = (const NullLevelState &) { }
};

#endif

