/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connectionWriter.cxx
 * @author drose
 * @date 2000-02-08
 */

#include "connectionWriter.h"
#include "connectionManager.h"
#include "datagramTCPHeader.h"
#include "config_net.h"
#include "socket_tcp.h"
#include "socket_udp.h"
#include "pnotify.h"
#include "config_downloader.h"

/**
 *
 */
ConnectionWriter::WriterThread::
WriterThread(ConnectionWriter *writer, const std::string &thread_name,
             int thread_index) :
  Thread(make_thread_name(thread_name, thread_index),
         make_thread_name(thread_name, thread_index)),
  _writer(writer),
  _thread_index(thread_index)
{
}

/**
 *
 */
void ConnectionWriter::WriterThread::
thread_main() {
  _writer->thread_run(_thread_index);
}

/**
 * Creates a new ConnectionWriter with the indicated number of threads to
 * handle output.
 *
 * If num_threads is 0, all datagrams will be sent immediately instead of
 * queueing for later transmission by a thread.
 */
ConnectionWriter::
ConnectionWriter(ConnectionManager *manager, int num_threads,
                 const std::string &thread_name) :
  _manager(manager)
{
  if (!Thread::is_threading_supported()) {
#ifndef NDEBUG
    if (num_threads != 0) {
      if (net_cat.is_debug()) {
        net_cat.debug()
          << "Threading support is not available.\n";
      }
    }
#endif  // NDEBUG
    num_threads = 0;
  }

  _raw_mode = false;
  _tcp_header_size = tcp_header_size;
  _immediate = (num_threads <= 0);
  _shutdown = false;

  std::string writer_thread_name = thread_name;
  if (thread_name.empty()) {
    writer_thread_name = "WriterThread";
  }
  int i;
  for (i = 0; i < num_threads; i++) {
    PT(WriterThread) thread = new WriterThread(this, writer_thread_name, i);
    _threads.push_back(thread);
  }
  for (i = 0; i < num_threads; i++) {
    _threads[i]->start(net_thread_priority, true);
  }

  _manager->add_writer(this);
}

/**
 *
 */
ConnectionWriter::
~ConnectionWriter() {
  if (_manager != nullptr) {
    _manager->remove_writer(this);
  }

  shutdown();
}

/**
 * Limits the number of packets that may be pending on the outbound queue.
 * This only has an effect when using threads; if num_threads is 0, then all
 * packets are sent immediately.
 */
void ConnectionWriter::
set_max_queue_size(int max_size) {
  _queue.set_max_queue_size(max_size);
}

/**
 * Returns the maximum size the queue is allowed to grow to.  See
 * set_max_queue_size().
 */
int ConnectionWriter::
get_max_queue_size() const {
  return _queue.get_max_queue_size();
}

/**
 * Returns the current number of things in the queue.
 */
int ConnectionWriter::
get_current_queue_size() const {
  return _queue.get_current_queue_size();
}


/**
 * Enqueues a datagram for transmittal on the indicated socket.  Since the
 * host address is not specified with this form, this function should only be
 * used for sending TCP packets.  Use the other send() method for sending UDP
 * packets.
 *
 * Returns true if successful, false if there was an error.  In the normal,
 * threaded case, this function only returns false if the send queue is
 * filled; it's impossible to detect a transmission error at this point.
 *
 * If block is true, this will not return false if the send queue is filled;
 * instead, it will wait until there is space available.
 */
bool ConnectionWriter::
send(const Datagram &datagram, const PT(Connection) &connection, bool block) {
  nassertr(!_shutdown, false);
  nassertr(connection != nullptr, false);
  nassertr(connection->get_socket()->is_exact_type(Socket_TCP::get_class_type()), false);

  NetDatagram copy(datagram);
  copy.set_connection(connection);

  if (_immediate) {
    if (_raw_mode) {
      return connection->send_raw_datagram(copy);
    } else {
      return connection->send_datagram(copy, _tcp_header_size);
    }
  } else {
    return _queue.insert(copy, block);
  }
}


/**
 * Enqueues a datagram for transmittal on the indicated socket.  This form of
 * the function allows the specification of a destination host address, and so
 * is appropriate for UDP packets.  Use the other send() method for sending
 * TCP packets.
 *
 * Returns true if successful, false if there was an error.  In the normal,
 * threaded case, this function only returns false if the send queue is
 * filled; it's impossible to detect a transmission error at this point.
 *
 * If block is true, this will not return false if the send queue is filled;
 * instead, it will wait until there is space available.
 */
bool ConnectionWriter::
send(const Datagram &datagram, const PT(Connection) &connection,
     const NetAddress &address, bool block) {
  nassertr(!_shutdown, false);
  nassertr(connection != nullptr, false);
  nassertr(connection->get_socket()->is_exact_type(Socket_UDP::get_class_type()), false);

  if ((int)datagram.get_length() > maximum_udp_datagram) {
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
    return _queue.insert(copy, block);
  }
}

/**
 * Returns true if the datagram is small enough to be sent over a UDP packet,
 * false otherwise.
 */
bool ConnectionWriter::
is_valid_for_udp(const Datagram &datagram) const {
  return (int)datagram.get_length() <= maximum_udp_datagram;
}

/**
 * Returns a pointer to the ConnectionManager object that serves this
 * ConnectionWriter.
 */
ConnectionManager *ConnectionWriter::
get_manager() const {
  return _manager;
}

/**
 * Returns true if the writer is an immediate writer, i.e.  it has no threads.
 */
bool ConnectionWriter::
is_immediate() const {
  return _immediate;
}

/**
 * Returns the number of threads the ConnectionWriter has been created with.
 */
int ConnectionWriter::
get_num_threads() const {
  return _threads.size();
}

/**
 * Sets the ConnectionWriter into raw mode (or turns off raw mode).  In raw
 * mode, datagrams are not sent along with their headers; the bytes in the
 * datagram are simply sent down the pipe.
 *
 * Setting the ConnectionWriter to raw mode must be done with care.  This can
 * only be done when the matching ConnectionReader is also set to raw mode, or
 * when the ConnectionWriter is communicating to a process that does not
 * expect datagrams.
 */
void ConnectionWriter::
set_raw_mode(bool mode) {
  _raw_mode = mode;
}

/**
 * Returns the current setting of the raw mode flag.  See set_raw_mode().
 */
bool ConnectionWriter::
get_raw_mode() const {
  return _raw_mode;
}

/**
 * Sets the header size of TCP packets.  At the present, legal values for this
 * are 0, 2, or 4; this specifies the number of bytes to use encode the
 * datagram length at the start of each TCP datagram.  Sender and receiver
 * must independently agree on this.
 */
void ConnectionWriter::
set_tcp_header_size(int tcp_header_size) {
  _tcp_header_size = tcp_header_size;
}

/**
 * Returns the current setting of TCP header size.  See set_tcp_header_size().
 */
int ConnectionWriter::
get_tcp_header_size() const {
  return _tcp_header_size;
}

/**
 * Stops all the threads and cleans them up.  This is called automatically by
 * the destructor, but it may be called explicitly before destruction.
 */
void ConnectionWriter::
shutdown() {
  if (_shutdown) {
    return;
  }
  _shutdown = true;

  // First, shutdown the queue.  This will tell our threads they're done.
  _queue.shutdown();

  // Now wait for all threads to terminate.
  Threads::iterator ti;
  for (ti = _threads.begin(); ti != _threads.end(); ++ti) {
    (*ti)->join();
  }
  _threads.clear();
}

/**
 * This should normally only be called when the associated ConnectionManager
 * destructs.  It resets the ConnectionManager pointer to NULL so we don't
 * have a floating pointer.  This makes the ConnectionWriter invalid;
 * presumably it also will be destructed momentarily.
 */
void ConnectionWriter::
clear_manager() {
  _manager = nullptr;
  shutdown();
}

/**
 * This is the actual executing function for each thread.
 */
void ConnectionWriter::
thread_run(int thread_index) {
  nassertv(!_immediate);

  NetDatagram datagram;
  while (_queue.extract(datagram)) {
    if (_raw_mode) {
      datagram.get_connection()->send_raw_datagram(datagram);
    } else {
      datagram.get_connection()->send_datagram(datagram, _tcp_header_size);
    }
    Thread::consider_yield();
  }
}
