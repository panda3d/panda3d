// Filename: connectionWriter.cxx
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

#include "connectionWriter.h"
#include "connectionManager.h"
#include "pprerror.h"
#include "config_net.h"

#include "notify.h"
#include <prerror.h>

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::Constructor
//       Access: Public
//  Description: Creates a new ConnectionWriter with the indicated
//               number of threads to handle output.
//
//               If num_threads is 0, all datagrams will be sent
//               immediately instead of queueing for later
//               transmission by a thread.
////////////////////////////////////////////////////////////////////
ConnectionWriter::
ConnectionWriter(ConnectionManager *manager, int num_threads) :
  _manager(manager)
{
#ifndef HAVE_THREADS
  // Although this code is written to use thread-locking primitives
  // regardless of the definition of HAVE_THREADS, it is not safe to
  // spawn multiple threads when HAVE_THREADS is not true, since we
  // might be using a non-thread-safe malloc scheme.
#ifndef NDEBUG
  if (num_threads != 0) {
    net_cat.error()
      << "Threading support is not available.\n";
  }
#endif  // NDEBUG
  num_threads = 0;
#endif  // HAVE_THREADS

  _raw_mode = false;
  _tcp_header_size = datagram_tcp16_header_size;
  _immediate = (num_threads <= 0);

  for (int i = 0; i < num_threads; i++) {
    PRThread *thread =
      PR_CreateThread(PR_USER_THREAD,
                      thread_start, (void *)this,
                      PR_PRIORITY_NORMAL,
                      PR_GLOBAL_THREAD, // Since thread will mostly do I/O.
                      PR_JOINABLE_THREAD,
                      0);  // Select a suitable stack size.

    nassertv(thread != (PRThread *)NULL);
    _threads.push_back(thread);
  }

  _manager->add_writer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ConnectionWriter::
~ConnectionWriter() {
  if (_manager != (ConnectionManager *)NULL) {
    _manager->remove_writer(this);
  }

  // First, shutdown the queue.  This will tell our threads they're
  // done.
  _queue.shutdown();

  // Now wait for all threads to terminate.
  Threads::iterator ti;
  for (ti = _threads.begin(); ti != _threads.end(); ++ti) {
    // Interrupt the thread just in case it was stuck waiting for I/O.
    PRStatus result = PR_Interrupt(*ti);
    if (result != PR_SUCCESS) {
      pprerror("PR_Interrupt");
    }

    result = PR_JoinThread(*ti);
    if (result != PR_SUCCESS) {
      pprerror("PR_JoinThread");
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::send
//       Access: Public
//  Description: Enqueues a datagram for transmittal on the indicated
//               socket.  Since the host address is not specified with
//               this form, this function should only be used for
//               sending TCP packets.  Use the other send() method for
//               sending UDP packets.
//
//               Returns true if successful, false if there was an
//               error.  In the normal, threaded case, this function
//               only returns false if the send queue is filled; it's
//               impossible to detect a transmission error at this
//               point.
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
send(const Datagram &datagram, const PT(Connection) &connection) {
  nassertr(connection != (Connection *)NULL, false);
  nassertr(PR_GetDescType(connection->get_socket()) == PR_DESC_SOCKET_TCP, false);

  NetDatagram copy(datagram);
  copy.set_connection(connection);

  if (_immediate) {
    if (_raw_mode) {
      return connection->send_raw_datagram(copy);
    } else {
      return connection->send_datagram(copy, _tcp_header_size);
    }
  } else {
    return _queue.insert(copy);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::send
//       Access: Public
//  Description: Enqueues a datagram for transmittal on the indicated
//               socket.  This form of the function allows the
//               specification of a destination host address, and so
//               is appropriate for UDP packets.  Use the other send()
//               method for sending TCP packets.
//
//               Returns true if successful, false if there was an
//               error.  In the normal, threaded case, this function
//               only returns false if the send queue is filled; it's
//               impossible to detect a transmission error at this
//               point.
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
send(const Datagram &datagram, const PT(Connection) &connection,
     const NetAddress &address) {
  nassertr(connection != (Connection *)NULL, false);
  nassertr(PR_GetDescType(connection->get_socket()) == PR_DESC_SOCKET_UDP, false);

  if (PR_GetDescType(connection->get_socket()) == PR_DESC_SOCKET_UDP &&
      (int)datagram.get_length() > maximum_udp_datagram) {
    net_cat.warning()
      << "Attempt to send UDP datagram of " << datagram.get_length()
      << " bytes, more than the\n"
      << "currently defined maximum of " << maximum_udp_datagram
      << " bytes.\n";
  }

  NetDatagram copy(datagram);
  copy.set_connection(connection);
  copy.set_address(address);

  if (_immediate) {
    if (_raw_mode) {
      return connection->send_raw_datagram(copy);
    } else {
      return connection->send_datagram(copy, _tcp_header_size);
    }
  } else {
    return _queue.insert(copy);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::is_valid_for_udp
//       Access: Public
//  Description: Returns true if the datagram is small enough to be
//               sent over a UDP packet, false otherwise.
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
is_valid_for_udp(const Datagram &datagram) const {
  return (int)datagram.get_length() <= maximum_udp_datagram;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::get_manager
//       Access: Public
//  Description: Returns a pointer to the ConnectionManager object
//               that serves this ConnectionWriter.
////////////////////////////////////////////////////////////////////
ConnectionManager *ConnectionWriter::
get_manager() const {
  return _manager;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::is_immediate
//       Access: Public
//  Description: Returns true if the writer is an immediate writer,
//               i.e. it has no threads.
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
is_immediate() const {
  return _immediate;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::get_num_threads
//       Access: Public
//  Description: Returns the number of threads the ConnectionWriter
//               has been created with.
////////////////////////////////////////////////////////////////////
int ConnectionWriter::
get_num_threads() const {
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::set_raw_mode
//       Access: Public
//  Description: Sets the ConnectionWriter into raw mode (or turns off
//               raw mode).  In raw mode, datagrams are not sent along
//               with their headers; the bytes in the datagram are
//               simply sent down the pipe.
//
//               Setting the ConnectionWriter to raw mode must be done
//               with care.  This can only be done when the matching
//               ConnectionReader is also set to raw mode, or when the
//               ConnectionWriter is communicating to a process that
//               does not expect datagrams.
////////////////////////////////////////////////////////////////////
void ConnectionWriter::
set_raw_mode(bool mode) {
  _raw_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::get_raw_mode
//       Access: Public
//  Description: Returns the current setting of the raw mode flag.
//               See set_raw_mode().
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
get_raw_mode() const {
  return _raw_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::set_tcp_header_size
//       Access: Public
//  Description: Sets the header size of TCP packets.  At the present,
//               legal values for this are 0, 2, or 4; this specifies
//               the number of bytes to use encode the datagram length
//               at the start of each TCP datagram.  Sender and
//               receiver must independently agree on this.
////////////////////////////////////////////////////////////////////
void ConnectionWriter::
set_tcp_header_size(int tcp_header_size) {
  _tcp_header_size = tcp_header_size;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::get_tcp_header_size
//       Access: Public
//  Description: Returns the current setting of TCP header size.
//               See set_tcp_header_size().
////////////////////////////////////////////////////////////////////
int ConnectionWriter::
get_tcp_header_size() const {
  return _tcp_header_size;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::clear_manager
//       Access: Protected
//  Description: This should normally only be called when the
//               associated ConnectionManager destructs.  It resets
//               the ConnectionManager pointer to NULL so we don't
//               have a floating pointer.  This makes the
//               ConnectionWriter invalid; presumably it also will be
//               destructed momentarily.
////////////////////////////////////////////////////////////////////
void ConnectionWriter::
clear_manager() {
  _manager = (ConnectionManager *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::thread_start
//       Access: Private, Static
//  Description: The static wrapper around the thread's executing
//               function.  This must be a static member function
//               because it is passed through the C interface to
//               PR_CreateThread().
////////////////////////////////////////////////////////////////////
void ConnectionWriter::
thread_start(void *data) {
  ((ConnectionWriter *)data)->thread_run();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::thread_run
//       Access: Private
//  Description: This is the actual executing function for each
//               thread.
////////////////////////////////////////////////////////////////////
void ConnectionWriter::
thread_run() {
  nassertv(!_immediate);

  NetDatagram datagram;
  while (_queue.extract(datagram)) {
    if (_raw_mode) {
      datagram.get_connection()->send_raw_datagram(datagram);
    } else {
      datagram.get_connection()->send_datagram(datagram, _tcp_header_size);
    }
  }
}
