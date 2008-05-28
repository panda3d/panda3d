// Filename: pStatMonitor.I
// Created by:  drose (14Jul00)
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
//     Function: PStatMonitor::get_server
//       Access: Public
//  Description: Returns the server that owns this monitor.
////////////////////////////////////////////////////////////////////
INLINE PStatServer *PStatMonitor::
get_server() {
  return _server;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatMonitor::get_client_data
//       Access: Public
//  Description: Returns the client data associated with this monitor.
////////////////////////////////////////////////////////////////////
INLINE const PStatClientData *PStatMonitor::
get_client_data() const {
  return _client_data;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatMonitor::get_collector_name
//       Access: Public
//  Description: Returns the name of the indicated collector, if it is
//               known.
////////////////////////////////////////////////////////////////////
INLINE string PStatMonitor::
get_collector_name(int collector_index) {
  if (!_client_data.is_null() &&
      _client_data->has_collector(collector_index)) {
    return _client_data->get_collector_name(collector_index);
  }
  return "Unknown";
}

////////////////////////////////////////////////////////////////////
//     Function: PStatMonitor::is_client_known
//       Access: Public
//  Description: Returns true if we've yet received the "hello"
//               message from the client indicating its name, etc.
////////////////////////////////////////////////////////////////////
INLINE bool PStatMonitor::
is_client_known() const {
  return _client_known;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatMonitor::get_client_hostname
//       Access: Public
//  Description: Returns the hostname of the client we're connected
//               to, if known.  This may not be known immediately at
//               creation time, but should be learned shortly
//               thereafter when we receive the client's "hello"
//               message.  See is_client_known().
////////////////////////////////////////////////////////////////////
INLINE string PStatMonitor::
get_client_hostname() const {
  return _client_hostname;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatMonitor::get_client_progname
//       Access: Public
//  Description: Returns the program name of the client we're
//               connected to, if known.  This may not be known
//               immediately at creation time, but should be learned
//               shortly thereafter when we receive the client's
//               "hello" message.  See is_client_known().
////////////////////////////////////////////////////////////////////
INLINE string PStatMonitor::
get_client_progname() const {
  return _client_progname;
}
