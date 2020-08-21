/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatTimer.h
 * @author drose
 * @date 2000-07-11
 */

#ifndef PSTATTIMER_H
#define PSTATTIMER_H

#include "pandabase.h"

#include "pStatCollector.h"

class Thread;

/**
 * A lightweight class that can be used to automatically start and stop a
 * PStatCollector around a section of code.  It's intended to be used in the
 * following way: create a local PStatTimer variable to start the Collector,
 * and when the PStatTimer variable goes out of scope (for instance, at the
 * end of the function), it will automatically stop the Collector.
 */
class PStatTimer {
public:
#ifdef DO_PSTATS
  INLINE PStatTimer(PStatCollector &collector);
  INLINE PStatTimer(PStatCollector &collector, Thread *current_thread);
  INLINE ~PStatTimer();

protected:
  PStatCollector &_collector;
  PStatThread _thread;
#else // DO_PSTATS

  INLINE PStatTimer(PStatCollector &) { }
  INLINE PStatTimer(PStatCollector &, Thread *) { }
  INLINE ~PStatTimer() { }

#endif  // DO_PSTATS
};

#include "pStatTimer.I"

#endif
