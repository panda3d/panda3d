// Filename: pStatClientImpl.I
// Created by:  drose (23Dec04)
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


////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::set_client_name
//       Access: Public
//  Description: Sets the name of the client.  This is reported to the
//               PStatsServer, and will presumably be written in the
//               title bar or something.
////////////////////////////////////////////////////////////////////
INLINE void PStatClientImpl::
set_client_name(const string &name) {
  _client_name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::get_client_name
//       Access: Public
//  Description: Retrieves the name of the client as set.
////////////////////////////////////////////////////////////////////
INLINE string PStatClientImpl::
get_client_name() const {
  return _client_name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::set_max_rate
//       Access: Public
//  Description: Controls the number of packets that will be sent to
//               the server.  Normally, one packet is sent per frame,
//               but this can flood the server with more packets than
//               it can handle if the frame rate is especially good
//               (e.g. if nothing is onscreen at the moment).  Set
//               this parameter to a reasonable number to prevent this
//               from happening.
//
//               This number specifies the maximum number of packets
//               that will be sent to the server per second, per
//               thread.
////////////////////////////////////////////////////////////////////
INLINE void PStatClientImpl::
set_max_rate(float rate) {
  _max_rate = rate;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::get_max_rate
//       Access: Public
//  Description: Returns the maximum number of packets that will be
//               sent to the server per second, per thread.  See
//               set_max_rate().
////////////////////////////////////////////////////////////////////
INLINE float PStatClientImpl::
get_max_rate() const {
  return _max_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::get_clock
//       Access: Public
//  Description: Returns a reference to the PStatClientImpl's clock
//               object.  It keeps its own clock, instead of using the
//               global clock object, so the stats won't get mucked up
//               if you put the global clock in non-real-time mode or
//               something.
////////////////////////////////////////////////////////////////////
INLINE const ClockObject &PStatClientImpl::
get_clock() const {
  return _clock;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::client_main_tick
//       Access: Public
//  Description: Called only by PStatClient::client_main_tick().
////////////////////////////////////////////////////////////////////
INLINE void PStatClientImpl::
client_main_tick() {
  _clock.tick();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::client_is_connected
//       Access: Public
//  Description: Called only by PStatClient::client_is_connected().
////////////////////////////////////////////////////////////////////
INLINE bool PStatClientImpl::
client_is_connected() const {
  return _is_connected;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::client_resume_after_pause
//       Access: Public
//  Description: Called only by PStatClient::client_resume_after_pause().
////////////////////////////////////////////////////////////////////
INLINE void PStatClientImpl::
client_resume_after_pause() {
  // Simply reset the clock to the beginning of the last frame.  This
  // may lose a frame, but on the other hand we won't skip a whole
  // slew of frames either.

  double frame_time = _clock.get_frame_time();
  _clock.set_real_time(frame_time);
}
