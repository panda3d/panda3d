/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatGPUTimer.I
 * @author rdb
 * @date 2014-08-21
 */

#ifdef DO_PSTATS

/**
 *
 */
INLINE PStatGPUTimer::
PStatGPUTimer(GraphicsStateGuardian *gsg, PStatCollector &collector) :
  PStatTimer(collector),
  _gsg(gsg)
{
  if (gsg->get_timer_queries_active() && collector.is_active()) {
    gsg->issue_timer_query(collector.get_index());
    // cerr << "issuing " << collector << " active " << collector.is_active()
    // << "\n";
  }
}

/**
 *
 */
INLINE PStatGPUTimer::
PStatGPUTimer(GraphicsStateGuardian *gsg, PStatCollector &collector, Thread *current_thread) :
  PStatTimer(collector, current_thread),
  _gsg(gsg)
{
  if (gsg->get_timer_queries_active() && collector.is_active()) {
    gsg->issue_timer_query(collector.get_index());
  }
}

/**
 *
 */
INLINE PStatGPUTimer::
~PStatGPUTimer() {
  if (_gsg->get_timer_queries_active() && _collector.is_active()) {
    _gsg->issue_timer_query(_collector.get_index() | 0x8000);
  }
}

#endif
