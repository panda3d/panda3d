// Filename: pStatTimer.I
// Created by:  drose (11Jul00)
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


#ifdef DO_PSTATS

////////////////////////////////////////////////////////////////////
//     Function: PStatTimer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PStatTimer::
PStatTimer(PStatCollector &collector) :
  _collector(collector),
  _thread(PStatClient::get_global_pstats()->get_current_thread())
{
  _collector.start(_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatTimer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PStatTimer::
PStatTimer(PStatCollector &collector, Thread *current_thread) :
  _collector(collector),
  _thread(current_thread)
{
  _collector.start(_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatTimer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PStatTimer::
~PStatTimer() {
  _collector.stop(_thread);
}

#endif
