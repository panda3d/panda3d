/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatListener.cxx
 * @author drose
 * @date 2000-07-09
 */

#include "pStatListener.h"
#include "pStatServer.h"
#include "pStatReader.h"

/**
 *
 */
PStatListener::
PStatListener(PStatServer *manager) :
  ConnectionListener(manager, manager->is_thread_safe() ? 1 : 0),
  _manager(manager)
{
}

/**
 * An internal function called by ConnectionListener() when a new TCP
 * connection has been established.
 */
void PStatListener::
connection_opened(const PT(Connection) &,
                  const NetAddress &address,
                  const PT(Connection) &new_connection) {
  PStatMonitor *monitor = _manager->make_monitor();
  if (monitor == nullptr) {
    nout << "Couldn't create monitor!\n";
    return;
  }

  nout << "Got new connection from " << address << "\n";

  // Make sure this connection doesn't queue up TCP packets we write to it.
  new_connection->set_collect_tcp(false);

  PStatReader *reader = new PStatReader(_manager, monitor);
  _manager->add_reader(new_connection, reader);
  reader->set_tcp_connection(new_connection);
}
