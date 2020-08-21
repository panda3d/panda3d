/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatGPUTimer.h
 * @author rdb
 * @date 2014-08-21
 */

#ifndef PSTATGPUTIMER_H
#define PSTATGPUTIMER_H

#include "pandabase.h"
#include "pStatTimer.h"
#include "pStatCollector.h"
#include "config_pstatclient.h"
#include "timerQueryContext.h"

class Thread;
class GraphicsStateGuardian;

/**
 * This is a special type of PStatTimer that also uses a timer query on the
 * GSG to measure how long a task actually takes to execute on the GPU, rather
 * than how long it took for the API commands to be queued up.
 *
 * This class may only be used on the draw thread.
 *
 * At present, it tracks both the CPU time (like a regular PStatTimer does)
 * and the GPU time, which is recorded using a special PStatThread.
 */
class EXPCL_PANDA_DISPLAY PStatGPUTimer : public PStatTimer {
public:
#ifdef DO_PSTATS
  INLINE PStatGPUTimer(GraphicsStateGuardian *gsg,
                       PStatCollector &collector);
  INLINE PStatGPUTimer(GraphicsStateGuardian *gsg,
                       PStatCollector &collector,
                       Thread *current_thread);
  INLINE ~PStatGPUTimer();

  GraphicsStateGuardian *_gsg;

private:
#else // DO_PSTATS

  INLINE PStatGPUTimer(GraphicsStateGuardian *, PStatCollector &col)
    : PStatTimer(col) { }
  INLINE PStatGPUTimer(GraphicsStateGuardian *, PStatCollector &col, Thread *)
    : PStatTimer(col) { }
  INLINE ~PStatGPUTimer() { }

#endif  // DO_PSTATS
};

#include "pStatGPUTimer.I"

#endif
