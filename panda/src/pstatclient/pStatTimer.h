// Filename: pStatTimer.h
// Created by:  drose (11Jul00)
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

#ifndef PSTATTIMER_H
#define PSTATTIMER_H

#include "pandabase.h"

#include "pStatCollector.h"

////////////////////////////////////////////////////////////////////
//       Class : PStatTimer
// Description : A lightweight class that can be used to automatically
//               start and stop a PStatCollector around a section of
//               code.  It's intended to be used in the following way:
//               create a local PStatTimer variable to start the
//               Collector, and when the PStatTimer variable goes out
//               of scope (for instance, at the end of the function),
//               it will automatically stop the Collector.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatTimer {
public:
#ifdef DO_PSTATS
  INLINE PStatTimer(PStatCollector &collector);
  INLINE ~PStatTimer();

private:
  PStatCollector &_collector;

#else // DO_PSTATS

  INLINE PStatTimer(PStatCollector &) { }
  INLINE ~PStatTimer() { }

#endif  // DO_PSTATS
};

#include "pStatTimer.I"

#endif

