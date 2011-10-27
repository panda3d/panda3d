// Filename: pStatClientImpl.I
// Created by:  drose (23Dec04)
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
set_max_rate(double rate) {
  _max_rate = rate;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::get_max_rate
//       Access: Public
//  Description: Returns the maximum number of packets that will be
//               sent to the server per second, per thread.  See
//               set_max_rate().
////////////////////////////////////////////////////////////////////
INLINE double PStatClientImpl::
get_max_rate() const {
  return _max_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::get_real_time
//       Access: Public
//  Description: Returns the time according to the PStatClientImpl's
//               clock object.  It keeps its own clock, instead of
//               using the global clock object, so the stats won't get
//               mucked up if you put the global clock in
//               non-real-time mode or something.
////////////////////////////////////////////////////////////////////
INLINE double PStatClientImpl::
get_real_time() const {
  return _clock->get_short_time() + _delta;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::client_main_tick
//       Access: Public
//  Description: Called only by PStatClient::client_main_tick().
////////////////////////////////////////////////////////////////////
INLINE void PStatClientImpl::
client_main_tick() {
  _last_frame = _clock->get_short_time();
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
  double delta = _clock->get_short_time() - _last_frame;
  _delta -= delta;
}
