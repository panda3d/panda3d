// Filename: queuedConnectionManager.h
// Created by:  drose (09Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef QUEUEDCONNECTIONMANAGER_H
#define QUEUEDCONNECTIONMANAGER_H

#include <pandabase.h>

#include "connectionManager.h"
#include "queuedReturn.h"

#include <prlock.h>
#include <deque>

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, QueuedReturn< PT(Connection) >);

////////////////////////////////////////////////////////////////////
// 	 Class : QueuedConnectionManager
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
				public QueuedReturn<PT(Connection)> {
public:
  QueuedConnectionManager();
  ~QueuedConnectionManager();

  bool reset_connection_available() const;
  bool get_reset_connection(PT(Connection) &connection);

protected:
  virtual void connection_reset(const PT(Connection) &connection);
};

#endif
