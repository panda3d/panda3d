/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file queuedConnectionManager.h
 * @author drose
 * @date 2000-02-09
 */

#ifndef QUEUEDCONNECTIONMANAGER_H
#define QUEUEDCONNECTIONMANAGER_H

#include "pandabase.h"

#include "connectionManager.h"
#include "queuedReturn.h"
#include "pdeque.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_NET, EXPTP_PANDA_NET, QueuedReturn< PT(Connection) >);

/**
 * This flavor of ConnectionManager will queue up all of the reset-connection
 * messages from the ConnectionReaders and ConnectionWriters and report them
 * to the client on demand.
 *
 * When a reset connection has been discovered via
 * reset_connection_available()/get_reset_connection(), it is still the
 * responsibility of the client to call close_connection() on that connection
 * to free up its resources.
 */
class EXPCL_PANDA_NET QueuedConnectionManager : public ConnectionManager,
                                public QueuedReturn< PT(Connection) > {
PUBLISHED:
  QueuedConnectionManager();
  ~QueuedConnectionManager();

  bool reset_connection_available() const;
  bool get_reset_connection(PT(Connection) &connection);

protected:
  virtual void connection_reset(const PT(Connection) &connection,
                                bool okflag);
};

#endif
