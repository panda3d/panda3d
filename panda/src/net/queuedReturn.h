// Filename: queuedReturn.h
// Created by:  drose (25Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef QUEUEDRETURN_H
#define QUEUEDRETURN_H

#include <pandabase.h>

#include "connectionListener.h"
#include "connection.h"
#include "netAddress.h"

#include <prlock.h>
#include <deque>

////////////////////////////////////////////////////////////////////
// 	 Class : QueuedReturn
// Description : This is the implementation of a family of things that
//               queue up their return values for later retrieval by
//               client code, like QueuedConnectionReader,
//               QueuedConnectionListener, QueuedConnectionManager.
////////////////////////////////////////////////////////////////////
template<class Thing>
class QueuedReturn {
PUBLISHED:
  void set_max_queue_size(int max_size);
  int get_max_queue_size() const;
  int get_current_queue_size() const;

protected:
  QueuedReturn();
  ~QueuedReturn();

  INLINE bool thing_available() const;
  bool get_thing(Thing &thing);
 
  bool enqueue_thing(const Thing &thing);
  bool enqueue_unique_thing(const Thing &thing);

private:
  PRLock *_mutex;
  deque<Thing> _things;
  bool _available;
  int _max_queue_size;
};

#include "queuedReturn.I"

#endif

