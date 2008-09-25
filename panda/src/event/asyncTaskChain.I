// Filename: asyncTaskChain.I
// Created by:  drose (23Aug06)
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


////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::is_started
//       Access: Published
//  Description: Returns true if the thread(s) have been started and
//               are ready to service requests, false otherwise.  If
//               this is false, the next call to add() or add_and_do()
//               will automatically start the threads.
////////////////////////////////////////////////////////////////////
INLINE bool AsyncTaskChain::
is_started() const {
  return (_state == S_started);
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::set_tick_clock
//       Access: Published
//  Description: Sets the tick_clock flag.  When this is true,
//               get_clock()->tick() will be called automatically at
//               each task epoch.  This is false by default.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTaskChain::
set_tick_clock(bool clock) {
  _tick_clock = clock;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTaskChain::get_tick_clock
//       Access: Published
//  Description: Returns the tick_clock flag..  See set_tick_clock().
////////////////////////////////////////////////////////////////////
INLINE bool AsyncTaskChain::
get_tick_clock() const {
  return _tick_clock;
}
