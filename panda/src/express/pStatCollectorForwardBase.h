// Filename: pStatCollectorForwardBase.h
// Created by:  drose (30Oct06)
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

#ifndef PSTATCOLLECTORFORWARDBASE_H
#define PSTATCOLLECTORFORWARDBASE_H

#include "pandabase.h"
#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : PStatCollectorForwardBase
// Description : This class serves as a cheap forward reference to a
//               PStatCollector, which is defined in the pstatclient
//               module (and is not directly accessible here in the
//               express module).
//
//               This is subclassed as PStatCollectorForward, which
//               defines the actual functionality.
////////////////////////////////////////////////////////////////////
class PStatCollectorForwardBase : public ReferenceCount {
PUBLISHED:
#ifdef DO_PSTATS
  virtual ~PStatCollectorForwardBase();
  virtual void add_level(double level)=0;
#else
  INLINE void add_level(double level) { }
#endif
};

#endif
