// Filename: queuedReturn.h
// Created by:  drose (25Feb00)
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

#ifndef QUEUEDRETURN_H
#define QUEUEDRETURN_H

#include "pandabase.h"

#include "connectionListener.h"
#include "connection.h"
#include "netAddress.h"

#include <prlock.h>
#include "pdeque.h"

////////////////////////////////////////////////////////////////////
//       Class : QueuedReturn
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
  pdeque<Thing> _things;
  bool _available;
  int _max_queue_size;
};

#include "queuedReturn.I"

#endif

