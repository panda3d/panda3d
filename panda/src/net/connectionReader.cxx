// Filename: connectionReader.cxx
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

#include "connectionReader.h"
#include "connectionManager.h"
#include "netDatagram.h"
#include "datagramTCPHeader.h"
#include "datagramUDPHeader.h"
#include "pprerror.h"
#include "config_net.h"

#include "notify.h"
#include <prerror.h>
#include <pratom.h>
#include <algorithm>

static const int read_buffer_size = maximum_udp_datagram + datagram_udp_header_size;

// We have to impose a maximum timeout on the PR_Poll() call because
// PR_Poll() doesn't seem to be interruptible! (!)
static const PRUint32 max_timeout_ms = 100;

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::SocketInfo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ConnectionReader::SocketInfo::
SocketInfo(const PT(Connection) &connection) :
  _connection(connection)
{
  _busy = false;
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::SocketInfo::is_udp
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool ConnectionReader::SocketInfo::
is_udp() const {
  return (PR_GetDescType(_connection->get_socket()) == PR_DESC_SOCKET_UDP);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::SocketInfo::get_socket
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PRFileDesc *ConnectionReader::SocketInfo::
get_socket() const {
  return _connection->get_socket();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::Constructor
//       Access: Public
//  Description: Creates a new ConnectionReader with the indicated
//               number of threads to handle requests.  If num_threads
//               is 0, the sockets will only be read by polling,
//               during an explicit poll() call.
//               (QueuedConnectionReader will do this automatically.)
////////////////////////////////////////////////////////////////////
ConnectionReader::
ConnectionReader(ConnectionManager *manager, int num_threads) :
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
  _polling = (num_threads <= 0);

  _shutdown = false;
  _startup_mutex = PR_NewLock();

  _next_index = 0;
  _num_results = 0;
  _select_mutex = PR_NewLock();

  _currently_polling_thread = -1;

  _reexamine_sockets = false;
  _sockets_mutex = PR_NewLock();

  // Before we create all the threads, grab _startup_mutex.  That will
  // prevent our new threads from trying to look themselves up in the
  // _threads array before we have filled it up.
  PR_Lock(_startup_mutex);

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

  PR_Unlock(_startup_mutex);

  _manager->add_reader(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ConnectionReader::
~ConnectionReader() {
  if (_manager != (ConnectionManager *)NULL) {
    _manager->remove_reader(this);
  }

  shutdown();

  // Delete all of our old sockets.
  Sockets::iterator si;
  for (si = _sockets.begin(); si != _sockets.end(); ++si) {
    delete (*si);
  }
  for (si = _removed_sockets.begin(); si != _removed_sockets.end(); ++si) {
    SocketInfo *sinfo = (*si);
    if (!sinfo->_busy) {
      delete sinfo;
    } else {
      net_cat.error()
        << "Reentrant deletion of ConnectionReader--don't delete these\n"
        << "in response to connection_reset().\n";

      // We'll have to do the best we can to recover.
      sinfo->_connection.clear();
    }
  }

  PR_DestroyLock(_startup_mutex);
  PR_DestroyLock(_select_mutex);
  PR_DestroyLock(_sockets_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::add_connection
//       Access: Public
//  Description: Adds a new socket to the list of sockets the
//               ConnectionReader will monitor.  A datagram that comes
//               in on any of the monitored sockets will be reported.
//               In the case of a ConnectionListener, this adds a new
//               rendezvous socket; any activity on any of the
//               monitored sockets will cause a connection to be
//               accepted.
//
//               The return value is true if the connection was added,
//               false if it was already there.
//
//               add_connection() is thread-safe, and may be called at
//               will by any thread.
////////////////////////////////////////////////////////////////////
bool ConnectionReader::
add_connection(const PT(Connection) &connection) {
  nassertr(connection != (Connection *)NULL, false);

  PR_Lock(_sockets_mutex);

  // Make sure it's not already on the _sockets list.
  Sockets::const_iterator si;
  for (si = _sockets.begin(); si != _sockets.end(); ++si) {
    if ((*si)->_connection == connection) {
      // Whoops, already there.
      PR_Unlock(_sockets_mutex);
      return false;
    }
  }

  _sockets.push_back(new SocketInfo(connection));
  _reexamine_sockets = true;
  PR_Unlock(_sockets_mutex);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::remove_connection
//       Access: Public
//  Description: Removes a socket from the list of sockets being
//               monitored.  Returns true if the socket was correctly
//               removed, false if it was not on the list in the first
//               place.
//
//               remove_connection() is thread-safe, and may be called
//               at will by any thread.
////////////////////////////////////////////////////////////////////
bool ConnectionReader::
remove_connection(const PT(Connection) &connection) {
  PR_Lock(_sockets_mutex);

  // Walk through the list of sockets to find the one we're removing.
  Sockets::iterator si;
  si = _sockets.begin();
  while (si != _sockets.end() && (*si)->_connection != connection) {
    ++si;
  }
  if (si == _sockets.end()) {
    PR_Unlock(_sockets_mutex);
    return false;
  }

  _removed_sockets.push_back(*si);
  _sockets.erase(si);
  _reexamine_sockets = true;
  PR_Unlock(_sockets_mutex);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::is_connection_ok
//       Access: Public
//  Description: Returns true if the indicated connection has been
//               added to the ConnectionReader and is being monitored
//               properly, false if it is not known, or if there was
//               some error condition detected on the connection.  (If
//               there was an error condition, normally the
//               ConnectionManager would have been informed and closed
//               the connection.)
////////////////////////////////////////////////////////////////////
bool ConnectionReader::
is_connection_ok(const PT(Connection) &connection) {
  PR_Lock(_sockets_mutex);

  // Walk through the list of sockets to find the one we're asking
  // about.
  Sockets::iterator si;
  si = _sockets.begin();
  while (si != _sockets.end() && (*si)->_connection != connection) {
    ++si;
  }
  if (si == _sockets.end()) {
    // Don't know that connection.
    PR_Unlock(_sockets_mutex);
    return false;
  }

  SocketInfo *sinfo = (*si);
  bool is_ok = !sinfo->_error;
  PR_Unlock(_sockets_mutex);

  return is_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::poll
//       Access: Public
//  Description: Explicitly polls the available sockets to see if any
//               of them have any noise.  This function does nothing
//               unless this is a polling-type ConnectionReader,
//               i.e. it was created with zero threads (and
//               is_polling() will return true).
//
//               It is not necessary to call this explicitly for a
//               QueuedConnectionReader.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
poll() {
  if (!_polling) {
    return;
  }

  SocketInfo *sinfo = get_next_available_socket(PR_INTERVAL_NO_WAIT, -2);
  while (sinfo != (SocketInfo *)NULL) {
    process_incoming_data(sinfo);
    sinfo = get_next_available_socket(PR_INTERVAL_NO_WAIT, -2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::get_manager
//       Access: Public
//  Description: Returns a pointer to the ConnectionManager object
//               that serves this ConnectionReader.
////////////////////////////////////////////////////////////////////
ConnectionManager *ConnectionReader::
get_manager() const {
  return _manager;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::is_polling
//       Access: Public
//  Description: Returns true if the reader is a polling reader,
//               i.e. it has no threads.
////////////////////////////////////////////////////////////////////
bool ConnectionReader::
is_polling() const {
  return _polling;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::get_num_threads
//       Access: Public
//  Description: Returns the number of threads the ConnectionReader
//               has been created with.
////////////////////////////////////////////////////////////////////
int ConnectionReader::
get_num_threads() const {
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::set_raw_mode
//       Access: Public
//  Description: Sets the ConnectionReader into raw mode (or turns off
//               raw mode).  In raw mode, datagram headers are not
//               expected; instead, all the data available on the pipe
//               is treated as a single datagram.
//
//               This is similar to set_tcp_header_size(0), except that it
//               also turns off headers for UDP packets.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
set_raw_mode(bool mode) {
  _raw_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::get_raw_mode
//       Access: Public
//  Description: Returns the current setting of the raw mode flag.
//               See set_raw_mode().
////////////////////////////////////////////////////////////////////
bool ConnectionReader::
get_raw_mode() const {
  return _raw_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::set_tcp_header_size
//       Access: Public
//  Description: Sets the header size of TCP packets.  At the present,
//               legal values for this are 0, 2, or 4; this specifies
//               the number of bytes to use encode the datagram length
//               at the start of each TCP datagram.  Sender and
//               receiver must independently agree on this.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
set_tcp_header_size(int tcp_header_size) {
  _tcp_header_size = tcp_header_size;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::get_tcp_header_size
//       Access: Public
//  Description: Returns the current setting of TCP header size.
//               See set_tcp_header_size().
////////////////////////////////////////////////////////////////////
int ConnectionReader::
get_tcp_header_size() const {
  return _tcp_header_size;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::shutdown
//       Access: Protected
//  Description: Terminates all threads cleanly.  Normally this is
//               only called by the destructor.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
shutdown() {
  if (_shutdown) {
    return;
  }

  // First, begin the shutdown.  This will tell our threads we want
  // them to quit.
  _shutdown = true;

  // Now wait for all of our threads to terminate.
  Threads::iterator ti;
  for (ti = _threads.begin(); ti != _threads.end(); ++ti) {
    // Interrupt the thread so it can notice the shutdown.
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
//     Function: ConnectionReader::clear_manager
//       Access: Protected
//  Description: This should normally only be called when the
//               associated ConnectionManager destructs.  It resets
//               the ConnectionManager pointer to NULL so we don't
//               have a floating pointer.  This makes the
//               ConnectionReader invalid; presumably it also will be
//               destructed momentarily.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
clear_manager() {
  _manager = (ConnectionManager *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::finish_socket
//       Access: Protected
//  Description: To be called when a socket has been fully read and is
//               ready for polling for additional data.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
finish_socket(SocketInfo *sinfo) {
  nassertv(sinfo->_busy);

  // By marking the SocketInfo nonbusy, we make it available for
  // future polls.
  sinfo->_busy = false;
  _reexamine_sockets = true;

  // However, someone might be already blocking on an
  // earlier-established PR_Poll() that doesn't involve this socket.
  // That complicates things.  It means we'll have to wake that thread
  // up so it can rebuild the poll with the new socket.

  // Actually, don't bother, since it turns out that PR_Poll() isn't
  // interruptible anyway.  Sigh.  Maybe we'll revisit this later, but
  // in the meantime it means we have to rig up the PR_Poll() to
  // return every so often and check the _reexamine_sockets flag by
  // itself.

  /*
  int thread = _currently_polling_thread;
  if (thread != -1) {
    nassertv(thread >= 0 && thread < _threads.size());
    PR_Interrupt(_threads[thread]);
  }
  */
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::process_incoming_data
//       Access: Protected, Virtual
//  Description: This is run within a thread when the call to
//               PR_Poll() indicates there is data available
//               on a socket.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
process_incoming_data(SocketInfo *sinfo) {
  if (_raw_mode) {
    if (sinfo->is_udp()) {
      process_raw_incoming_udp_data(sinfo);
    } else {
      process_raw_incoming_tcp_data(sinfo);
    }
  } else {
    if (sinfo->is_udp()) {
      process_incoming_udp_data(sinfo);
    } else {
      process_incoming_tcp_data(sinfo);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::process_incoming_udp_data
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void ConnectionReader::
process_incoming_udp_data(SocketInfo *sinfo) {
  PRFileDesc *socket = sinfo->get_socket();
  PRNetAddr addr;

  // Read as many bytes as we can.
  PRInt8 buffer[read_buffer_size];
  PRInt32 bytes_read;

  bytes_read = PR_RecvFrom(socket, buffer, read_buffer_size, 0,
                           &addr, PR_INTERVAL_NO_TIMEOUT);

  if (bytes_read < 0) {
    PRErrorCode errcode = PR_GetError();
    if (errcode != PR_PENDING_INTERRUPT_ERROR) {
      pprerror("PR_RecvFrom");
    }
    finish_socket(sinfo);
    return;

  } else if (bytes_read == 0) {
    // The socket was closed (!).  This shouldn't happen with a UDP
    // connection.  Oh well.  Report that and return.
    if (_manager != (ConnectionManager *)NULL) {
      _manager->connection_reset(sinfo->_connection, 0);
    }
    finish_socket(sinfo);
    return;
  }

  // Since we are not running in raw mode, we decode the header to
  // determine how big the datagram is.  This means we must have read
  // at least a full header.
  if (bytes_read < datagram_udp_header_size) {
    net_cat.error()
      << "Did not read entire header, discarding UDP datagram.\n";
    finish_socket(sinfo);
    return;
  }
  
  DatagramUDPHeader header(buffer);
  
  PRInt8 *dp = buffer + datagram_udp_header_size;
  bytes_read -= datagram_udp_header_size;
  
  NetDatagram datagram(dp, bytes_read);
  
  // Now that we've read all the data, it's time to finish the socket
  // so another thread can read the next datagram.
  finish_socket(sinfo);
  
  if (_shutdown) {
    return;
  }
  
  // And now do whatever we need to do to process the datagram.
  if (!header.verify_datagram(datagram)) {
    net_cat.error()
      << "Ignoring invalid UDP datagram.\n";
  } else {
    datagram.set_connection(sinfo->_connection);
    datagram.set_address(NetAddress(addr));
    receive_datagram(datagram);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::process_incoming_tcp_data
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void ConnectionReader::
process_incoming_tcp_data(SocketInfo *sinfo) {
  PRFileDesc *socket = sinfo->get_socket();
  PRNetAddr addr;

  // Read only the header bytes to start with.
  PRInt8 buffer[read_buffer_size];
  PRInt32 header_bytes_read = 0;

  if (PR_GetSockName(socket, &addr) != PR_SUCCESS) {
    pprerror("PR_GetSockName");
  }

  // First, we have to read the first _tcp_header_size bytes.
  while (header_bytes_read < _tcp_header_size) {
    PRInt32 bytes_read =
      PR_Recv(socket, buffer + header_bytes_read,
              _tcp_header_size - header_bytes_read, 0,
              PR_INTERVAL_NO_TIMEOUT);

    if (bytes_read < 0) {
      PRErrorCode errcode = PR_GetError();
      if (errcode == PR_CONNECT_RESET_ERROR
#ifdef PR_SOCKET_SHUTDOWN_ERROR
          || errcode == PR_SOCKET_SHUTDOWN_ERROR
          || errcode == PR_CONNECT_ABORTED_ERROR
#endif
          ) {
        // The socket was closed.
        if (_manager != (ConnectionManager *)NULL) {
          _manager->connection_reset(sinfo->_connection, errcode);
        }

      } else if (errcode != PR_PENDING_INTERRUPT_ERROR) {
        pprerror("PR_Recv");
      }
      finish_socket(sinfo);
      return;

    } else if (bytes_read == 0) {
      // The socket was closed.  Report that and return.
      if (_manager != (ConnectionManager *)NULL) {
        _manager->connection_reset(sinfo->_connection, 0);
      }
      finish_socket(sinfo);
      return;
    }

    header_bytes_read += bytes_read;
  }

  // Now we must decode the header to determine how big the datagram
  // is.  This means we must have read at least a full header.
  if (header_bytes_read != _tcp_header_size) {
    // This should actually be impossible, by the read-loop logic
    // above.
    net_cat.error()
      << "Did not read entire header, discarding TCP datagram.\n";
    finish_socket(sinfo);
    return;
  }

  DatagramTCPHeader header(buffer, _tcp_header_size);
  PRInt32 size = header.get_datagram_size(_tcp_header_size);

  // We have to loop until the entire datagram is read.
  NetDatagram datagram;

  while (!_shutdown && (int)datagram.get_length() < size) {
    PRInt32 bytes_read;

    bytes_read =
      PR_Recv(socket, buffer,
              min((PRInt32)read_buffer_size,
                  (PRInt32)(size - datagram.get_length())),
              0, PR_INTERVAL_NO_TIMEOUT);
    PRInt8 *dp = buffer;

    if (bytes_read < 0) {
      PRErrorCode errcode = PR_GetError();
      if (errcode == PR_CONNECT_RESET_ERROR
#ifdef PR_SOCKET_SHUTDOWN_ERROR
          || errcode == PR_SOCKET_SHUTDOWN_ERROR
          || errcode == PR_CONNECT_ABORTED_ERROR
#endif
          ) {
        // The socket was closed.
        if (_manager != (ConnectionManager *)NULL) {
          _manager->connection_reset(sinfo->_connection, errcode);
        }

      } else if (errcode != PR_PENDING_INTERRUPT_ERROR) {
        pprerror("PR_Recv");
      }
      finish_socket(sinfo);
      return;

    } else if (bytes_read == 0) {
      // The socket was closed.  Report that and return.
      if (_manager != (ConnectionManager *)NULL) {
        _manager->connection_reset(sinfo->_connection, 0);
      }
      finish_socket(sinfo);
      return;
    }

    PRInt32 datagram_bytes =
      min(bytes_read, (PRInt32)(size - datagram.get_length()));
    datagram.append_data(dp, datagram_bytes);

    if (bytes_read > datagram_bytes) {
      // There were some extra bytes at the end of the datagram.  Maybe
      // the beginning of the next datagram?  Huh.
      net_cat.error()
        << "Discarding " << bytes_read - datagram_bytes
        << " bytes following TCP datagram.\n";
    }
  }

  // Now that we've read all the data, it's time to finish the socket
  // so another thread can read the next datagram.
  finish_socket(sinfo);

  if (_shutdown) {
    return;
  }

  // And now do whatever we need to do to process the datagram.
  if (!header.verify_datagram(datagram, _tcp_header_size)) {
    net_cat.error()
      << "Ignoring invalid TCP datagram.\n";
  } else {
    datagram.set_connection(sinfo->_connection);
    datagram.set_address(NetAddress(addr));
    receive_datagram(datagram);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::process_raw_incoming_udp_data
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void ConnectionReader::
process_raw_incoming_udp_data(SocketInfo *sinfo) {
  PRFileDesc *socket = sinfo->get_socket();
  PRNetAddr addr;

  // Read as many bytes as we can.
  PRInt8 buffer[read_buffer_size];
  PRInt32 bytes_read;

  bytes_read = PR_RecvFrom(socket, buffer, read_buffer_size, 0,
                           &addr, PR_INTERVAL_NO_TIMEOUT);

  if (bytes_read < 0) {
    PRErrorCode errcode = PR_GetError();
    if (errcode != PR_PENDING_INTERRUPT_ERROR) {
      pprerror("PR_RecvFrom");
    }
    finish_socket(sinfo);
    return;

  } else if (bytes_read == 0) {
    // The socket was closed (!).  This shouldn't happen with a UDP
    // connection.  Oh well.  Report that and return.
    if (_manager != (ConnectionManager *)NULL) {
      _manager->connection_reset(sinfo->_connection, 0);
    }
    finish_socket(sinfo);
    return;
  }

  // In raw mode, we simply extract all the bytes and make that a
  // datagram.
  NetDatagram datagram(buffer, bytes_read);
  
  // Now that we've read all the data, it's time to finish the socket
  // so another thread can read the next datagram.
  finish_socket(sinfo);
  
  if (_shutdown) {
    return;
  }
  
  datagram.set_connection(sinfo->_connection);
  datagram.set_address(NetAddress(addr));
  receive_datagram(datagram);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::process_raw_incoming_tcp_data
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void ConnectionReader::
process_raw_incoming_tcp_data(SocketInfo *sinfo) {
  PRFileDesc *socket = sinfo->get_socket();
  PRNetAddr addr;

  // Read as many bytes as we can.
  PRInt8 buffer[read_buffer_size];
  PRInt32 bytes_read;

  if (PR_GetSockName(socket, &addr) != PR_SUCCESS) {
    pprerror("PR_GetSockName");
  }

  bytes_read = PR_Recv(socket, buffer, read_buffer_size, 0,
                       PR_INTERVAL_NO_TIMEOUT);

  if (bytes_read < 0) {
    PRErrorCode errcode = PR_GetError();
    if (errcode != PR_PENDING_INTERRUPT_ERROR) {
      pprerror("PR_RecvFrom");
    }
    finish_socket(sinfo);
    return;

  } else if (bytes_read == 0) {
    // The socket was closed.  Report that and return.
    if (_manager != (ConnectionManager *)NULL) {
      _manager->connection_reset(sinfo->_connection, 0);
    }
    finish_socket(sinfo);
    return;
  }

  // In raw mode, we simply extract all the bytes and make that a
  // datagram.
  NetDatagram datagram(buffer, bytes_read);
  
  // Now that we've read all the data, it's time to finish the socket
  // so another thread can read the next datagram.
  finish_socket(sinfo);
  
  if (_shutdown) {
    return;
  }
  
  datagram.set_connection(sinfo->_connection);
  datagram.set_address(NetAddress(addr));
  receive_datagram(datagram);
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::thread_start
//       Access: Private, Static
//  Description: The static wrapper around the thread's executing
//               function.  This must be a static member function
//               because it is passed through the C interface to
//               PR_CreateThread().
////////////////////////////////////////////////////////////////////
void ConnectionReader::
thread_start(void *data) {
  ((ConnectionReader *)data)->thread_run();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::thread_run
//       Access: Private
//  Description: This is the actual executing function for each
//               thread.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
thread_run() {
  nassertv(!_polling);

  // First determine our own thread index.
  PR_Lock(_startup_mutex);
  Threads::const_iterator ti =
    find(_threads.begin(), _threads.end(), PR_GetCurrentThread());

  nassertv(ti != _threads.end());
  PRInt32 current_thread_index = (ti - _threads.begin());

  nassertv(_threads[current_thread_index] == PR_GetCurrentThread());
  PR_Unlock(_startup_mutex);

  while (!_shutdown) {
    SocketInfo *sinfo =
      get_next_available_socket(PR_INTERVAL_NO_TIMEOUT,
                                current_thread_index);
    if (sinfo != (SocketInfo *)NULL) {
      process_incoming_data(sinfo);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::get_next_available_socket
//       Access: Private
//  Description: Polls the known connections for activity and returns
//               the next one known to have activity, or NULL if no
//               activity is detected within the timeout interval.
//
//               This function may block indefinitely if it is being
//               called by multiple threads; if there are no other
//               threads, it may block only if timeout !=
//               PR_INTERVAL_NO_WAIT.
////////////////////////////////////////////////////////////////////
ConnectionReader::SocketInfo *ConnectionReader::
get_next_available_socket(PRIntervalTime timeout,
                          PRInt32 current_thread_index) {
  // Go to sleep on the select() mutex.  This guarantees that only one
  // thread is in this function at a time.
  PR_Lock(_select_mutex);

  int num_sockets = _polled_sockets.size();
  nassertr(num_sockets == (int)_poll.size(), NULL);

  do {
    // First, check the result from the previous PR_Poll() call.  If
    // there are any sockets remaining there, process them first.
    while (!_shutdown && _num_results > 0) {
      nassertr(_next_index < num_sockets, NULL);
      int i = _next_index;
      _next_index++;

      if (_poll[i].out_flags != 0) {
        _num_results--;
        SocketInfo *sinfo = _polled_sockets[i];

        if ((_poll[i].out_flags & PR_POLL_READ) != 0) {
          // Some genuine noise on the port.
          sinfo->_busy = true;
          _reexamine_sockets = true;
          PR_Unlock(_select_mutex);
          PR_Sleep(PR_INTERVAL_NO_WAIT);
          return sinfo;

        } else if ((_poll[i].out_flags &
                    (PR_POLL_ERR | PR_POLL_NVAL | PR_POLL_HUP)) != 0) {
          // Something bad happened to this socket.  Tell the
          // ConnectionManager to drop it.
          if (_manager != (ConnectionManager *)NULL) {
            // Perform a recv to force an error code.
            char buffer[1];
            PRInt32 got_bytes =
              PR_Recv(sinfo->get_socket(), buffer, 1, 0, PR_INTERVAL_NO_WAIT);
            if (got_bytes > 0) {
              net_cat.error()
                << "poll returned error flags " << hex << _poll[i].out_flags
                << dec << " but read " << got_bytes << " from socket.\n";
            }
            PRErrorCode errcode = PR_GetError();
            _manager->connection_reset(sinfo->_connection, errcode);
          }
          sinfo->_error = true;
          _reexamine_sockets = true;
        }
      }
    }

    bool interrupted;
    do {
      interrupted = false;

      // Ok, no results from previous PR_Poll() calls.  Prepare to set
      // up for a new poll.

      // First, report to anyone else who cares that we're the thread
      // about to do the poll.  That way, if any new sockets come
      // available while we're polling, we can service them.
      PR_AtomicSet(&_currently_polling_thread, current_thread_index);

      if (_reexamine_sockets) {
        _reexamine_sockets = false;
        rebuild_poll_list();
        num_sockets = _polled_sockets.size();
        nassertr(num_sockets == (int)_poll.size(), NULL);
      }

      // Now we can execute PR_Poll().  This basically maps to a Unix
      // select() call.
      _num_results = 0;
      _next_index = 0;

      if (!_shutdown) {
        PRIntervalTime poll_timeout =
          PR_MillisecondsToInterval(max_timeout_ms);
        if (timeout != PR_INTERVAL_NO_TIMEOUT) {
          poll_timeout = min(timeout, poll_timeout);
        }

        _num_results = PR_Poll(&_poll[0], num_sockets, poll_timeout);
      }

      if (_num_results == 0 && timeout == PR_INTERVAL_NO_TIMEOUT) {
        // If we reached max_timeout_ms, but the caller didn't request
        // a timeout, consider that an interrupt: go back and
        // reconsider.  (This is a kludge around the fact that
        // PR_Poll() appears to be non-interruptible.)
        interrupted = true;

      } else if (_num_results < 0) {
        // If our poll was interrupted by another thread, rebuild the
        // list and poll again.
        PRErrorCode errcode = PR_GetError();
        if (errcode == PR_PENDING_INTERRUPT_ERROR) {
          interrupted = true;
        } else {
          pprerror("PR_Poll");
        }
      }
    } while (!_shutdown && interrupted);

    PR_AtomicSet(&_currently_polling_thread, -1);
    // Just in case someone interrupted us while we were polling and
    // we didn't catch it, clear it now--we don't care any more.
    PR_ClearInterrupt();

    // Repeat the above until we (a) find a socket with actual noise
    // on it, or (b) return from PR_Poll() with no sockets available.
  } while (!_shutdown && _num_results > 0);

  PR_Unlock(_select_mutex);
  return (SocketInfo *)NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionReader::rebuild_poll_list
//       Access: Private
//  Description: Rebuilds the _poll and _polled_sockets arrays based
//               on the sockets that are currently available for
//               polling.
////////////////////////////////////////////////////////////////////
void ConnectionReader::
rebuild_poll_list() {
  _poll.clear();
  _polled_sockets.clear();

  PR_Lock(_sockets_mutex);
  Sockets::const_iterator si;
  for (si = _sockets.begin(); si != _sockets.end(); ++si) {
    SocketInfo *sinfo = (*si);
    if (!sinfo->_busy && !sinfo->_error) {
      PRPollDesc pd;
      pd.fd = sinfo->get_socket();
      pd.in_flags = PR_POLL_READ;
      pd.out_flags = 0;

      _poll.push_back(pd);
      _polled_sockets.push_back(sinfo);
    }
  }

  // This is also a fine time to delete the contents of the
  // _removed_sockets list.
  if (!_removed_sockets.empty()) {
    Sockets still_busy_sockets;
    for (si = _removed_sockets.begin(); si != _removed_sockets.end(); ++si) {
      SocketInfo *sinfo = (*si);
      if (sinfo->_busy) {
        still_busy_sockets.push_back(sinfo);
      } else {
        delete sinfo;
      }
    }
    _removed_sockets.swap(still_busy_sockets);
  }

  PR_Unlock(_sockets_mutex);
}
