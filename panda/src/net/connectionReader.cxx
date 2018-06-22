/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connectionReader.cxx
 * @author drose
 * @date 2000-02-08
 */

#include "connectionReader.h"
#include "dcast.h"
#include "connectionManager.h"
#include "netDatagram.h"
#include "datagramTCPHeader.h"
#include "datagramUDPHeader.h"
#include "config_net.h"
#include "trueClock.h"
#include "socket_udp.h"
#include "socket_tcp.h"
#include "mutexHolder.h"
#include "lightMutexHolder.h"
#include "pnotify.h"
#include "atomicAdjust.h"
#include "config_downloader.h"

using std::min;

static const int read_buffer_size = maximum_udp_datagram + datagram_udp_header_size;

/**
 *
 */
ConnectionReader::SocketInfo::
SocketInfo(const PT(Connection) &connection) :
  _connection(connection)
{
  _busy = false;
  _error = false;
}

/**
 *
 */
bool ConnectionReader::SocketInfo::
is_udp() const {
  return (_connection->get_socket()->is_exact_type(Socket_UDP::get_class_type()));
}

/**
 *
 */
Socket_IP *ConnectionReader::SocketInfo::
get_socket() const {
  return _connection->get_socket();
}

/**
 *
 */
ConnectionReader::ReaderThread::
ReaderThread(ConnectionReader *reader, const std::string &thread_name,
             int thread_index) :
  Thread(make_thread_name(thread_name, thread_index),
         make_thread_name(thread_name, thread_index)),
  _reader(reader),
  _thread_index(thread_index)
{
}

/**
 *
 */
void ConnectionReader::ReaderThread::
thread_main() {
  _reader->thread_run(_thread_index);
}

/**
 * Creates a new ConnectionReader with the indicated number of threads to
 * handle requests.  If num_threads is 0, the sockets will only be read by
 * polling, during an explicit poll() call.  (QueuedConnectionReader will do
 * this automatically.)
 */
ConnectionReader::
ConnectionReader(ConnectionManager *manager, int num_threads,
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
  _polling = (num_threads <= 0);

  _shutdown = false;

  _next_index = 0;
  _num_results = 0;

  _currently_polling_thread = -1;

  std::string reader_thread_name = thread_name;
  if (thread_name.empty()) {
    reader_thread_name = "ReaderThread";
  }
  int i;
  for (i = 0; i < num_threads; i++) {
    PT(ReaderThread) thread = new ReaderThread(this, reader_thread_name, i);
    _threads.push_back(thread);
  }
  for (i = 0; i < num_threads; i++) {
    _threads[i]->start(net_thread_priority, true);
  }

  _manager->add_reader(this);
}

/**
 *
 */
ConnectionReader::
~ConnectionReader() {
  if (_manager != nullptr) {
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
}

/**
 * Adds a new socket to the list of sockets the ConnectionReader will monitor.
 * A datagram that comes in on any of the monitored sockets will be reported.
 * In the case of a ConnectionListener, this adds a new rendezvous socket; any
 * activity on any of the monitored sockets will cause a connection to be
 * accepted.
 *
 * The return value is true if the connection was added, false if it was
 * already there.
 *
 * add_connection() is thread-safe, and may be called at will by any thread.
 */
bool ConnectionReader::
add_connection(Connection *connection) {
  nassertr(connection != nullptr, false);

  LightMutexHolder holder(_sockets_mutex);

  // Make sure it's not already on the _sockets list.
  Sockets::const_iterator si;
  for (si = _sockets.begin(); si != _sockets.end(); ++si) {
    if ((*si)->_connection == connection) {
      // Whoops, already there.
      return false;
    }
  }

  _sockets.push_back(new SocketInfo(connection));

  return true;
}

/**
 * Removes a socket from the list of sockets being monitored.  Returns true if
 * the socket was correctly removed, false if it was not on the list in the
 * first place.
 *
 * remove_connection() is thread-safe, and may be called at will by any
 * thread.
 */
bool ConnectionReader::
remove_connection(Connection *connection) {
  LightMutexHolder holder(_sockets_mutex);

  // Walk through the list of sockets to find the one we're removing.
  Sockets::iterator si;
  si = _sockets.begin();
  while (si != _sockets.end() && (*si)->_connection != connection) {
    ++si;
  }
  if (si == _sockets.end()) {
    return false;
  }

  _removed_sockets.push_back(*si);
  _sockets.erase(si);

  return true;
}

/**
 * Returns true if the indicated connection has been added to the
 * ConnectionReader and is being monitored properly, false if it is not known,
 * or if there was some error condition detected on the connection.  (If there
 * was an error condition, normally the ConnectionManager would have been
 * informed and closed the connection.)
 */
bool ConnectionReader::
is_connection_ok(Connection *connection) {
  LightMutexHolder holder(_sockets_mutex);

  // Walk through the list of sockets to find the one we're asking about.
  Sockets::iterator si;
  si = _sockets.begin();
  while (si != _sockets.end() && (*si)->_connection != connection) {
    ++si;
  }
  if (si == _sockets.end()) {
    // Don't know that connection.
    return false;
  }

  SocketInfo *sinfo = (*si);
  bool is_ok = !sinfo->_error;

  return is_ok;
}

/**
 * Explicitly polls the available sockets to see if any of them have any
 * noise.  This function does nothing unless this is a polling-type
 * ConnectionReader, i.e.  it was created with zero threads (and is_polling()
 * will return true).
 *
 * It is not necessary to call this explicitly for a QueuedConnectionReader.
 */
void ConnectionReader::
poll() {
  if (!_polling) {
    return;
  }

  SocketInfo *sinfo = get_next_available_socket(false, -2);
  if (sinfo != nullptr) {
    double max_poll_cycle = get_net_max_poll_cycle();
    if (max_poll_cycle < 0.0) {
      // Continue to read all data.
      while (sinfo != nullptr) {
        process_incoming_data(sinfo);
        sinfo = get_next_available_socket(false, -2);
      }

    } else {
      // Read only until a certain amount of time has elapsed.
      TrueClock *global_clock = TrueClock::get_global_ptr();
      double stop = global_clock->get_short_time() + max_poll_cycle;

      while (sinfo != nullptr) {
        process_incoming_data(sinfo);
        if (global_clock->get_short_time() >= stop) {
          return;
        }
        sinfo = get_next_available_socket(false, -2);
      }
    }
  }
}

/**
 * Returns a pointer to the ConnectionManager object that serves this
 * ConnectionReader.
 */
ConnectionManager *ConnectionReader::
get_manager() const {
  return _manager;
}

/**
 * Returns the number of threads the ConnectionReader has been created with.
 */
int ConnectionReader::
get_num_threads() const {
  return _threads.size();
}

/**
 * Sets the ConnectionReader into raw mode (or turns off raw mode).  In raw
 * mode, datagram headers are not expected; instead, all the data available on
 * the pipe is treated as a single datagram.
 *
 * This is similar to set_tcp_header_size(0), except that it also turns off
 * headers for UDP packets.
 */
void ConnectionReader::
set_raw_mode(bool mode) {
  _raw_mode = mode;
}

/**
 * Returns the current setting of the raw mode flag.  See set_raw_mode().
 */
bool ConnectionReader::
get_raw_mode() const {
  return _raw_mode;
}

/**
 * Sets the header size of TCP packets.  At the present, legal values for this
 * are 0, 2, or 4; this specifies the number of bytes to use encode the
 * datagram length at the start of each TCP datagram.  Sender and receiver
 * must independently agree on this.
 */
void ConnectionReader::
set_tcp_header_size(int tcp_header_size) {
  _tcp_header_size = tcp_header_size;
}

/**
 * Returns the current setting of TCP header size.  See set_tcp_header_size().
 */
int ConnectionReader::
get_tcp_header_size() const {
  return _tcp_header_size;
}

/**
 * Terminates all threads cleanly.  Normally this is only called by the
 * destructor, but it may be called explicitly before destruction.
 */
void ConnectionReader::
shutdown() {
  if (_shutdown) {
    return;
  }

  // First, begin the shutdown.  This will tell our threads we want them to
  // quit.
  _shutdown = true;

  // Now wait for all of our threads to terminate.
  Threads::iterator ti;
  for (ti = _threads.begin(); ti != _threads.end(); ++ti) {
    (*ti)->join();
  }
}

/**
 * Attempts to read all the possible data from the indicated connection, which
 * has just delivered a write error (and has therefore already been closed).
 * If the connection is not monitered by this reader, does nothing.
 */
void ConnectionReader::
flush_read_connection(Connection *connection) {
  // Ensure it doesn't get deleted.
  SocketInfo sinfo(connection);

  if (!remove_connection(connection)) {
    // Not already in the reader.
    return;
  }

  // The connection was previously in the reader, but has now been removed.
  // Now we can flush it completely.  We check if there is any read data
  // available on just this one socket; we can do this right here in this
  // thread, since we've already removed this connection from the reader.

  Socket_fdset fdset;
  fdset.clear();
  fdset.setForSocket(*(sinfo.get_socket()));
  int num_results = fdset.WaitForRead(true, 0);
  while (num_results != 0) {
    sinfo._busy = true;
    if (!process_incoming_data(&sinfo)) {
      break;
    }
    fdset.setForSocket(*(sinfo.get_socket()));
    num_results = fdset.WaitForRead(true, 0);
  }
}

/**
 * This should normally only be called when the associated ConnectionManager
 * destructs.  It resets the ConnectionManager pointer to NULL so we don't
 * have a floating pointer.  This makes the ConnectionReader invalid;
 * presumably it also will be destructed momentarily.
 */
void ConnectionReader::
clear_manager() {
  _manager = nullptr;
}

/**
 * To be called when a socket has been fully read and is ready for polling for
 * additional data.
 */
void ConnectionReader::
finish_socket(SocketInfo *sinfo) {
  nassertv(sinfo->_busy);

  // By marking the SocketInfo nonbusy, we make it available for future polls.
  sinfo->_busy = false;
}

/**
 * This is run within a thread when the call to select() indicates there is
 * data available on a socket.  Returns true if the data is read successfully,
 * false on failure (for instance, because the connection is closed).
 */
bool ConnectionReader::
process_incoming_data(SocketInfo *sinfo) {
  if (_raw_mode) {
    if (sinfo->is_udp()) {
      return process_raw_incoming_udp_data(sinfo);
    } else {
      return process_raw_incoming_tcp_data(sinfo);
    }
  } else {
    if (sinfo->is_udp()) {
      return process_incoming_udp_data(sinfo);
    } else {
      return process_incoming_tcp_data(sinfo);
    }
  }
}

/**
 *
 */
bool ConnectionReader::
process_incoming_udp_data(SocketInfo *sinfo) {
  Socket_UDP *socket;
  DCAST_INTO_R(socket, sinfo->get_socket(), false);
  Socket_Address addr;

  // Read as many bytes as we can.
  char buffer[read_buffer_size];
  int bytes_read = read_buffer_size;

  bool okflag = socket->GetPacket(buffer, &bytes_read, addr);

  if (!okflag) {
    finish_socket(sinfo);
    return false;

  } else if (bytes_read == 0) {
    // The socket was closed (!).  This shouldn't happen with a UDP
    // connection.  Oh well.  Report that and return.
    if (_manager != nullptr) {
      _manager->connection_reset(sinfo->_connection, 0);
    }
    finish_socket(sinfo);
    return false;
  }

  // Since we are not running in raw mode, we decode the header to determine
  // how big the datagram is.  This means we must have read at least a full
  // header.
  if (bytes_read < datagram_udp_header_size) {
    net_cat.error()
      << "Did not read entire header, discarding UDP datagram.\n";
    finish_socket(sinfo);
    return true;
  }

  DatagramUDPHeader header(buffer);

  char *dp = buffer + datagram_udp_header_size;
  bytes_read -= datagram_udp_header_size;

  NetDatagram datagram(dp, bytes_read);

  // Now that we've read all the data, it's time to finish the socket so
  // another thread can read the next datagram.
  finish_socket(sinfo);

  if (_shutdown) {
    return false;
  }

  // And now do whatever we need to do to process the datagram.
  if (!header.verify_datagram(datagram)) {
    net_cat.error()
      << "Ignoring invalid UDP datagram.\n";
  } else {
    datagram.set_connection(sinfo->_connection);
    datagram.set_address(NetAddress(addr));

    if (net_cat.is_spam()) {
      net_cat.spam()
        << "Received UDP datagram with "
        << datagram_udp_header_size + datagram.get_length()
        << " bytes on " << (void *)datagram.get_connection()
        << " from " << datagram.get_address() << "\n";
    }

    receive_datagram(datagram);
  }

  return true;
}

/**
 *
 */
bool ConnectionReader::
process_incoming_tcp_data(SocketInfo *sinfo) {
  Socket_TCP *socket;
  DCAST_INTO_R(socket, sinfo->get_socket(), false);

  // Read only the header bytes to start with.
  char buffer[read_buffer_size];
  int header_bytes_read = 0;

  // First, we have to read the first _tcp_header_size bytes.
  while (header_bytes_read < _tcp_header_size) {
    int bytes_read =
      socket->RecvData(buffer + header_bytes_read,
                       _tcp_header_size - header_bytes_read);
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
    while (bytes_read < 0 && socket->GetLastError() == LOCAL_BLOCKING_ERROR &&
           socket->Active()) {
      Thread::force_yield();
      bytes_read = socket->RecvData(buffer + header_bytes_read,
                                    _tcp_header_size - header_bytes_read);
    }
#endif  // SIMPLE_THREADS

    if (bytes_read <= 0) {
      // The socket was closed.  Report that and return.
      if (_manager != nullptr) {
        _manager->connection_reset(sinfo->_connection, 0);
      }
      finish_socket(sinfo);
      return false;
    }

    header_bytes_read += bytes_read;
    Thread::consider_yield();
  }

  // Now we must decode the header to determine how big the datagram is.  This
  // means we must have read at least a full header.
  if (header_bytes_read != _tcp_header_size) {
    // This should actually be impossible, by the read-loop logic above.
    net_cat.error()
      << "Did not read entire header, discarding TCP datagram.\n";
    finish_socket(sinfo);
    return true;
  }

  DatagramTCPHeader header(buffer, _tcp_header_size);
  int size = header.get_datagram_size(_tcp_header_size);

  // We have to loop until the entire datagram is read.
  NetDatagram datagram;

  while (!_shutdown && (int)datagram.get_length() < size) {
    int bytes_read;

    int read_bytes = read_buffer_size;
#ifdef SIMPLE_THREADS
    // In the SIMPLE_THREADS case, we want to limit the number of bytes we
    // read in a single epoch, to minimize the impact on the other threads.
    read_bytes = min(read_buffer_size, (int)net_max_read_per_epoch);
#endif

    bytes_read =
      socket->RecvData(buffer, min(read_bytes,
                                   (int)(size - datagram.get_length())));
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
    while (bytes_read < 0 && socket->GetLastError() == LOCAL_BLOCKING_ERROR &&
           socket->Active()) {
      Thread::force_yield();
      bytes_read =
        socket->RecvData(buffer, min(read_bytes,
                                     (int)(size - datagram.get_length())));
    }
#endif  // SIMPLE_THREADS

    char *dp = buffer;

    if (bytes_read <= 0) {
      // The socket was closed.  Report that and return.
      if (_manager != nullptr) {
        _manager->connection_reset(sinfo->_connection, 0);
      }
      finish_socket(sinfo);
      return false;
    }

    int datagram_bytes =
      min(bytes_read, (int)(size - datagram.get_length()));
    datagram.append_data(dp, datagram_bytes);

    if (bytes_read > datagram_bytes) {
      // There were some extra bytes at the end of the datagram.  Maybe the
      // beginning of the next datagram?  Huh.
      net_cat.error()
        << "Discarding " << bytes_read - datagram_bytes
        << " bytes following TCP datagram.\n";
    }
    Thread::consider_yield();
  }

  // Now that we've read all the data, it's time to finish the socket so
  // another thread can read the next datagram.
  finish_socket(sinfo);

  if (_shutdown) {
    return false;
  }

  // And now do whatever we need to do to process the datagram.
  if (!header.verify_datagram(datagram, _tcp_header_size)) {
    net_cat.error()
      << "Ignoring invalid TCP datagram.\n";
  } else {
    datagram.set_connection(sinfo->_connection);
    datagram.set_address(NetAddress(socket->GetPeerName()));

    if (net_cat.is_spam()) {
      net_cat.spam()
        << "Received TCP datagram with "
        << _tcp_header_size + datagram.get_length()
        << " bytes on " << (void *)datagram.get_connection()
        << " from " << datagram.get_address() << "\n";
    }

    receive_datagram(datagram);
  }

  return true;
}

/**
 *
 */
bool ConnectionReader::
process_raw_incoming_udp_data(SocketInfo *sinfo) {
  Socket_UDP *socket;
  DCAST_INTO_R(socket, sinfo->get_socket(), false);
  Socket_Address addr;

  // Read as many bytes as we can.
  char buffer[read_buffer_size];
  int bytes_read = read_buffer_size;

  bool okflag = socket->GetPacket(buffer, &bytes_read, addr);

  if (!okflag) {
    finish_socket(sinfo);
    return false;

  } else if (bytes_read == 0) {
    // The socket was closed (!).  This shouldn't happen with a UDP
    // connection.  Oh well.  Report that and return.
    if (_manager != nullptr) {
      _manager->connection_reset(sinfo->_connection, 0);
    }
    finish_socket(sinfo);
    return false;
  }

  // In raw mode, we simply extract all the bytes and make that a datagram.
  NetDatagram datagram(buffer, bytes_read);

  // Now that we've read all the data, it's time to finish the socket so
  // another thread can read the next datagram.
  finish_socket(sinfo);

  if (_shutdown) {
    return false;
  }

  datagram.set_connection(sinfo->_connection);
  datagram.set_address(NetAddress(addr));

  if (net_cat.is_spam()) {
    net_cat.spam()
      << "Received raw UDP datagram with " << datagram.get_length()
      << " bytes on " << (void *)datagram.get_connection()
      << " from " << datagram.get_address() << "\n";
  }

  receive_datagram(datagram);

  return true;
}

/**
 *
 */
bool ConnectionReader::
process_raw_incoming_tcp_data(SocketInfo *sinfo) {
  Socket_TCP *socket;
  DCAST_INTO_R(socket, sinfo->get_socket(), false);

  // Read as many bytes as we can.
  char buffer[read_buffer_size];
  int bytes_read = socket->RecvData(buffer, read_buffer_size);
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  while (bytes_read < 0 && socket->GetLastError() == LOCAL_BLOCKING_ERROR &&
         socket->Active()) {
    Thread::force_yield();
    bytes_read = socket->RecvData(buffer, read_buffer_size);
  }
#endif  // SIMPLE_THREADS

  if (bytes_read <= 0) {
    // The socket was closed.  Report that and return.
    if (_manager != nullptr) {
      _manager->connection_reset(sinfo->_connection, 0);
    }
    finish_socket(sinfo);
    return false;
  }

  // In raw mode, we simply extract all the bytes and make that a datagram.
  NetDatagram datagram(buffer, bytes_read);

  // Now that we've read all the data, it's time to finish the socket so
  // another thread can read the next datagram.
  finish_socket(sinfo);

  if (_shutdown) {
    return false;
  }

  datagram.set_connection(sinfo->_connection);
  datagram.set_address(NetAddress(socket->GetPeerName()));

  if (net_cat.is_spam()) {
    net_cat.spam()
      << "Received raw TCP datagram with " << datagram.get_length()
      << " bytes on " << (void *)datagram.get_connection()
      << " from " << datagram.get_address() << "\n";
  }

  receive_datagram(datagram);

  return true;
}

/**
 * This is the actual executing function for each thread.
 */
void ConnectionReader::
thread_run(int thread_index) {
  nassertv(!_polling);
  nassertv(_threads[thread_index] == Thread::get_current_thread());

  while (!_shutdown) {
    SocketInfo *sinfo =
      get_next_available_socket(true, thread_index);
    if (sinfo != nullptr) {
      process_incoming_data(sinfo);
      Thread::consider_yield();
    } else {
      Thread::force_yield();
    }
  }
}


/**
 * Polls the known connections for activity and returns the next one known to
 * have activity, or NULL if no activity is detected within the timeout
 * interval.
 *
 * This function may block indefinitely if it is being called by multiple
 * threads; if there are no other threads, it may block only if allow_block is
 * true.
 */
ConnectionReader::SocketInfo *ConnectionReader::
get_next_available_socket(bool allow_block, int current_thread_index) {
  // Go to sleep on the select() mutex.  This guarantees that only one thread
  // is in this function at a time.
  MutexHolder holder(_select_mutex);

  do {
    // First, check the result from the previous select call.  If there are
    // any sockets remaining there, process them first.
    while (!_shutdown && _num_results > 0) {
      nassertr(_next_index < (int)_selecting_sockets.size(), nullptr);
      int i = _next_index;
      _next_index++;

      if (_fdset.IsSetFor(*_selecting_sockets[i]->get_socket())) {
        _num_results--;
        SocketInfo *sinfo = _selecting_sockets[i];

        // Some noise on this socket.
        sinfo->_busy = true;
        return sinfo;
      }
    }

    bool interrupted;
    do {
      interrupted = false;

      // Ok, no results from previous select calls.  Prepare to set up for a
      // new select.

      // First, report to anyone else who cares that we're the thread about to
      // do the poll.  That way, if any new sockets come available while we're
      // polling, we can service them.
      AtomicAdjust::set(_currently_polling_thread, current_thread_index);

      rebuild_select_list();

      // Now we can execute the select.
      _num_results = 0;
      _next_index = 0;

      if (!_shutdown) {
        uint32_t timeout = (uint32_t)(get_net_max_block() * 1000.0);
        if (!allow_block) {
          timeout = 0;
        }
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
        // In the presence of SIMPLE_THREADS, we never wait at all, but rather
        // we yield the thread if we come up empty (so that we won't block the
        // entire process).
        timeout = 0;
#endif

        _num_results = _fdset.WaitForRead(false, timeout);
      }

      if (_num_results == 0 && allow_block) {
        // If we reached net_max_block, go back and reconsider.  (We never
        // timeout indefinitely, so we can check the shutdown flag every once
        // in a while.)
        interrupted = true;
        Thread::force_yield();

      } else if (_num_results < 0) {
        // If we had an error, just return.  But yield the timeslice first.
        Thread::force_yield();
        return nullptr;
      }
    } while (!_shutdown && interrupted);

    AtomicAdjust::set(_currently_polling_thread, current_thread_index);

    // Repeat the above until we (a) find a socket with actual noise on it, or
    // (b) return from PR_Poll() with no sockets available.
  } while (!_shutdown && _num_results > 0);

  return nullptr;
}


/**
 * Rebuilds the _fdset and _selecting_sockets arrays based on the sockets that
 * are currently available for selecting.
 */
void ConnectionReader::
rebuild_select_list() {
  _fdset.clear();
  _selecting_sockets.clear();

  LightMutexHolder holder(_sockets_mutex);
  Sockets::const_iterator si;
  for (si = _sockets.begin(); si != _sockets.end(); ++si) {
    SocketInfo *sinfo = (*si);
    if (!sinfo->_busy && !sinfo->_error) {
      _fdset.setForSocket(*sinfo->get_socket());
      _selecting_sockets.push_back(sinfo);
    }
  }

  // This is also a fine time to delete the contents of the _removed_sockets
  // list.
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
}

/**
 * Adds the sockets from this ConnectionReader (or ConnectionListener) to the
 * indicated fdset.  This is used by ConnectionManager::block() to build an
 * fdset of all attached readers.
 */
void ConnectionReader::
accumulate_fdset(Socket_fdset &fdset) {
  LightMutexHolder holder(_sockets_mutex);
  Sockets::const_iterator si;
  for (si = _sockets.begin(); si != _sockets.end(); ++si) {
    SocketInfo *sinfo = (*si);
    if (!sinfo->_busy && !sinfo->_error) {
      fdset.setForSocket(*sinfo->get_socket());
    }
  }
}
