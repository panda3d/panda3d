// Filename: queuedConnectionManager.h
// Created by:  drose (09Feb00)
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

#ifndef QUEUEDCONNECTIONMANAGER_H
#define QUEUEDCONNECTIONMANAGER_H

#include "pandabase.h"

#include "connectionManager.h"
#include "queuedReturn.h"
#include "pdeque.h"

#include <prlock.h>

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, QueuedReturn< PT(Connection) >);

////////////////////////////////////////////////////////////////////
//       Class : QueuedConnectionManager
// Description : This flavor of ConnectionManager will queue up all of
//               the reset-connection messages from the
//               ConnectionReaders and ConnectionWriters and report
//               them to the client on demand.
//
//               When a reset connection has been discovered via
//               reset_connection_available()/get_reset_connection(),
//               it is still the responsibility of the client to call
//               close_connection() on that connection to free up its
//               resources.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA QueuedConnectionManager : public ConnectionManager,
                                public QueuedReturn< PT(Connection) > {
PUBLISHED:
  QueuedConnectionManager();
  ~QueuedConnectionManager();

  bool reset_connection_available() const;
  bool get_reset_connection(PT(Connection) &connection);

protected:
  virtual void connection_reset(const PT(Connection) &connection, 
                                PRErrorCode errcode);
};

#endif
