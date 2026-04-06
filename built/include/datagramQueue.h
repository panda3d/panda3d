/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramQueue.h
 * @author drose
 * @date 2000-02-07
 */

#ifndef DATAGRAMQUEUE_H
#define DATAGRAMQUEUE_H

#include "pandabase.h"

#include "netDatagram.h"
#include "pmutex.h"
#include "conditionVar.h"
#include "pdeque.h"

/**
 * A thread-safe, FIFO queue of NetDatagrams.  This is used by
 * ConnectionWriter for queuing up datagrams for its various threads to write
 * to sockets.
 */
class EXPCL_PANDA_NET DatagramQueue {
public:
  DatagramQueue();
  ~DatagramQueue();
  void shutdown();

  bool insert(const NetDatagram &data, bool block = false);
  bool extract(NetDatagram &result);

  void set_max_queue_size(int max_size);
  int get_max_queue_size() const;
  int get_current_queue_size() const;

private:
  Mutex _cvlock;
  ConditionVar _cv;  // signaled when queue contents change.

  typedef pdeque<NetDatagram> QueueType;
  QueueType _queue;
  bool _shutdown;
  int _max_queue_size;
};

#endif
