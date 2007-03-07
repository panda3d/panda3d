// Filename: datagramQueue.h
// Created by:  drose (07Feb00)
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

#ifndef DATAGRAMQUEUE_H
#define DATAGRAMQUEUE_H

#include "pandabase.h"

#include "netDatagram.h"
#include "pmutex.h"
#include "conditionVarFull.h"
#include "pdeque.h"

////////////////////////////////////////////////////////////////////
//       Class : DatagramQueue
// Description : A thread-safe, FIFO queue of NetDatagrams.  This is used
//               by ConnectionWriter for queuing up datagrams for
//               its various threads to write to sockets.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DatagramQueue {
public:
  DatagramQueue();
  ~DatagramQueue();
  void shutdown();

  bool insert(const NetDatagram &data);
  bool extract(NetDatagram &result);

  void set_max_queue_size(int max_size);
  int get_max_queue_size() const;
  int get_current_queue_size() const;

private:
  Mutex _cvlock;
  ConditionVarFull _cv;

  typedef pdeque<NetDatagram> QueueType;
  QueueType _queue;
  bool _shutdown;
  int _max_queue_size;
};

#endif

