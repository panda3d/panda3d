// Filename: datagramQueue.cxx
// Created by:  drose (08Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "datagramQueue.h"
#include "config_net.h"
#include "mutexHolder.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DatagramQueue::
DatagramQueue() : 
  _cvlock("DatagramQueue::_cvlock"),
  _cv(_cvlock) 
{
  _shutdown = false;
  _max_queue_size = get_net_max_write_queue();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DatagramQueue::
~DatagramQueue() {
  // It's an error to delete a DatagramQueue without first shutting it
  // down (and waiting for any associated threads to terminate).
  nassertv(_shutdown);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::shutdown
//       Access: Public
//  Description: Marks the queue as shutting down, which will
//               eventually cause all threads blocking on extract() to
//               return false.  The normal way to delete a
//               DatagramQueue will be to call first shutdown() and
//               then wait for all known threads to terminate.  Then
//               it is safe to delete the queue.
////////////////////////////////////////////////////////////////////
void DatagramQueue::
shutdown() {
  // Notify all of our threads that we're shutting down.  This will
  // cause any thread blocking on extract() to return false.
  MutexHolder holder(_cvlock);

  _shutdown = true;
  _cv.notify_all();
}


////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::insert
//       Access: Public
//  Description: Inserts the indicated datagram onto the end of the
//               queue, and returns.  If the queue is empty and any
//               threads are waiting on the queue, this will wake one
//               of them up.  Returns true if successful, false if the
//               queue was full.
//
//               If block is true, this will not return until
//               successful, waiting until the queue has space
//               available if necessary.
////////////////////////////////////////////////////////////////////
bool DatagramQueue::
insert(const NetDatagram &data, bool block) {
  MutexHolder holder(_cvlock);

  bool enqueue_ok = ((int)_queue.size() < _max_queue_size);
  if (block) {
    while (!enqueue_ok && !_shutdown) {
      _cv.wait();
      enqueue_ok = ((int)_queue.size() < _max_queue_size);
    }
  }

  if (enqueue_ok) {
    _queue.push_back(data);
  }
  _cv.notify();  // Only need to wake up one thread.

  return enqueue_ok;
}


////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::extract
//       Access: Public
//  Description: Extracts a datagram from the head of the queue, if
//               one is available.  If a datagram is available, this
//               will immediately return; otherwise, it will block
//               until a datagram becomes available.  Multiple threads
//               may simultaneously block on extract(); when a
//               datagram is subsequently inserted into the queue, one
//               of the threads will return from extract() with the
//               datagram.
//
//               The return value is true if the datagram is
//               successfully extracted, or false if the queue was
//               destroyed while waiting.  (In the case of a false
//               return, the thread should not attempt to operate on
//               the queue again.)
////////////////////////////////////////////////////////////////////
bool DatagramQueue::
extract(NetDatagram &result) {
  // First, clear the datagram result in case it's got an outstanding
  // connection pointer--we're about to go to sleep for a while.
  result.clear();

  MutexHolder holder(_cvlock);

  while (_queue.empty() && !_shutdown) {
    _cv.wait();
  }

  if (_shutdown) {
    return false;
  }

  nassertr(!_queue.empty(), false);
  result = _queue.front();
  _queue.pop_front();

  // Wake up any threads waiting to stuff things into the queue.
  _cv.notify_all();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::set_max_queue_size
//       Access: Public
//  Description: Sets the maximum size the queue is allowed to grow
//               to.  This is primarily for a sanity check; this is a
//               limit beyond which we can assume something bad has
//               happened.
//
//               It's also a crude check against unfortunate seg
//               faults due to the queue filling up and quietly
//               consuming all available memory.
////////////////////////////////////////////////////////////////////
void DatagramQueue::
set_max_queue_size(int max_size) {
  MutexHolder holder(_cvlock);
  _max_queue_size = max_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::get_max_queue_size
//       Access: Public
//  Description: Returns the maximum size the queue is allowed to grow
//               to.  See set_max_queue_size().
////////////////////////////////////////////////////////////////////
int DatagramQueue::
get_max_queue_size() const {
  return _max_queue_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::get_current_queue_size
//       Access: Public
//  Description: Returns the current number of things in the queue.
////////////////////////////////////////////////////////////////////
int DatagramQueue::
get_current_queue_size() const {
  MutexHolder holder(_cvlock);
  int size = _queue.size();
  return size;
}
