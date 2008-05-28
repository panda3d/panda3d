// Filename: pStatCollectorForwardBase.h
// Created by:  drose (30Oct06)
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
class EXPCL_PANDAEXPRESS PStatCollectorForwardBase : public ReferenceCount {
PUBLISHED:
#ifdef DO_PSTATS
  virtual ~PStatCollectorForwardBase();
  virtual void add_level(double level)=0;
#else
  INLINE void add_level(double level) { }
#endif
};

#endif
