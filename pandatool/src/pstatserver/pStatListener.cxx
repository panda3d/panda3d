// Filename: pStatListener.cxx
// Created by:  drose (09Jul00)
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

#include "pStatListener.h"
#include "pStatServer.h"
#include "pStatReader.h"

////////////////////////////////////////////////////////////////////
//     Function: PStatListener::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatListener::
PStatListener(PStatServer *manager) :
  ConnectionListener(manager, manager->is_thread_safe() ? 1 : 0),
  _manager(manager)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatListener::connection_opened
//       Access: Protected, Virtual
//  Description: An internal function called by ConnectionListener()
//               when a new TCP connection has been established.
////////////////////////////////////////////////////////////////////
void PStatListener::
connection_opened(const PT(Connection) &,
                  const NetAddress &address,
                  const PT(Connection) &new_connection) {
  PStatMonitor *monitor = _manager->make_monitor();
  if (monitor == (PStatMonitor *)NULL) {
    nout << "Couldn't create monitor!\n";
    return;
  }

  nout << "Got new connection from " << address << "\n";

  // Make sure this connection doesn't queue up TCP packets we write
  // to it.
  new_connection->set_collect_tcp(false);

  PStatReader *reader = new PStatReader(_manager, monitor);
  _manager->add_reader(new_connection, reader);
  reader->set_tcp_connection(new_connection);
}
