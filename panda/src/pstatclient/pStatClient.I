// Filename: pStatClient.I
// Created by:  drose (16Jul00)
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
//     Function: PStatClient::set_client_name
//       Access: Public
//  Description: Sets the name of the client.  This is reported to the
//               PStatsServer, and will presumably be written in the
//               title bar or something.
////////////////////////////////////////////////////////////////////
INLINE void PStatClient::
set_client_name(const string &name) {
  _client_name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_client_name
//       Access: Public
//  Description: Retrieves the name of the client as set.
////////////////////////////////////////////////////////////////////
INLINE string PStatClient::
get_client_name() const {
  return _client_name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::set_max_rate
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
INLINE void PStatClient::
set_max_rate(float rate) {
  _max_rate = rate;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_max_rate
//       Access: Public
//  Description: Returns the maximum number of packets that will be
//               sent to the server per second, per thread.  See
//               set_max_rate().
////////////////////////////////////////////////////////////////////
INLINE float PStatClient::
get_max_rate() const {
  return _max_rate;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::connect
//       Access: Published
//  Description: Attempts to establish a connection to the indicated
//               PStatServer.  Returns true if successful, false on
//               failure.
////////////////////////////////////////////////////////////////////
INLINE bool PStatClient::
connect(const string &hostname, int port) {
  return get_global_pstats()->client_connect(hostname, port);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::disconnect
//       Access: Published
//  Description: Closes the connection previously established.
////////////////////////////////////////////////////////////////////
INLINE void PStatClient::
disconnect() {
  get_global_pstats()->client_disconnect();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::is_connected
//       Access: Published
//  Description: Returns true if the client believes it is connected
//               to a working PStatServer, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatClient::
is_connected() {
  return get_global_pstats()->client_is_connected();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::resume_after_pause
//       Access: Published
//  Description: Resumes the PStatClient after the simulation has been
//               paused for a while.  This allows the stats to
//               continue exactly where it left off, instead of
//               leaving a big gap that would represent a chug.
////////////////////////////////////////////////////////////////////
INLINE void PStatClient::
resume_after_pause() {
  get_global_pstats()->client_resume_after_pause();
}
