// Filename: pStatTimer.I
// Created by:  drose (11Jul00)
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


#ifdef DO_PSTATS

////////////////////////////////////////////////////////////////////
//     Function: PStatTimer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PStatTimer::
PStatTimer(PStatCollector &collector) :
  _collector(collector)
{
  _collector.start();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatTimer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PStatTimer::
~PStatTimer() {
  _collector.stop();
}

#endif
