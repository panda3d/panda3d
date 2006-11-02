// Filename: pStatCollectorForward.h
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

#ifndef PSTATCOLLECTORFORWARD_H
#define PSTATCOLLECTORFORWARD_H

#include "pandabase.h"
#include "pStatCollectorForwardBase.h"
#include "pStatCollector.h"

////////////////////////////////////////////////////////////////////
//       Class : PStatCollectorForward
// Description : This class serves as a cheap forward reference to a
//               PStatCollector, so that classes that are defined
//               before the pstats module may access the
//               PStatCollector.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatCollectorForward : public PStatCollectorForwardBase {
PUBLISHED:
  INLINE PStatCollectorForward(const PStatCollector &col);

#ifdef DO_PSTATS
  virtual void add_level(double level);

private:
  PStatCollector _col;
#endif
};

#include "pStatCollectorForward.I"

#endif
