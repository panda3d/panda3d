// Filename: datagramQueue.h
// Created by:  drose (07Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef DATAGRAMQUEUE_H
#define DATAGRAMQUEUE_H

#include <pandabase.h>

#include "netDatagram.h"

#include <prlock.h>
#include <prcvar.h>
#include <deque>

////////////////////////////////////////////////////////////////////
// 	 Class : DatagramQueue
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
  PRLock *_cvlock;
  PRCondVar *_cv;
  deque<NetDatagram> _queue;
  bool _shutdown;
  int _max_queue_size;
};

#endif

