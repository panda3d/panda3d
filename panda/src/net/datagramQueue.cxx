// Filename: datagramQueue.cxx
// Created by:  drose (08Feb00)
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

#include "datagramQueue.h"
#include "config_net.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DatagramQueue::
DatagramQueue() {
  _shutdown = false;
  _cvlock = PR_NewLock();
  _cv = PR_NewCondVar(_cvlock);
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

  PR_DestroyCondVar(_cv);
  PR_DestroyLock(_cvlock);
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
  PR_Lock(_cvlock);
  _shutdown = true;
  PR_NotifyAllCondVar(_cv);
  PR_Unlock(_cvlock);
}


////////////////////////////////////////////////////////////////////
//     Function: DatagramQueue::insert
//       Access: Public
//  Description: Inserts the indicated datagram onto the end of the
//               queue, and returns.  If the queue is empty and any
//               threads are waiting on the queue, this will wake one
//               of them up.  Returns true if successful, false if the
//               queue was full.
////////////////////////////////////////////////////////////////////
bool DatagramQueue::
insert(const NetDatagram &data) {
  PR_Lock(_cvlock);
  bool enqueue_ok = ((int)_queue.size() < _max_queue_size);
  if (enqueue_ok) {
#ifdef __ICL
    _queue.push_back(new NetDatagram(data));
#else
    _queue.push_back(data);
#endif
  }
  PR_NotifyCondVar(_cv);
  PR_Unlock(_cvlock);
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

  PR_Lock(_cvlock);
  while (_queue.empty() && !_shutdown) {
    PR_WaitCondVar(_cv, PR_INTERVAL_NO_TIMEOUT);
  }

  if (_shutdown) {
    PR_Unlock(_cvlock);
    return false;
  }

  nassertr(!_queue.empty(), false);
#ifdef __ICL
  NetDatagram *ptr = _queue.front();
  result = *ptr;
  delete ptr;
#else
  result = _queue.front();
#endif
  _queue.pop_front();

  PR_Unlock(_cvlock);
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
  PR_Lock(_cvlock);
  _max_queue_size = max_size;
  PR_Unlock(_cvlock);
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
  PR_Lock(_cvlock);
  int size = _queue.size();
  PR_Unlock(_cvlock);
  return size;
}
