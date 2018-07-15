/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connectionReader.h
 * @author drose
 * @date 2000-02-08
 */

#ifndef CONNECTIONREADER_H
#define CONNECTIONREADER_H

#include "pandabase.h"

#include "connection.h"

#include "pointerTo.h"
#include "pmutex.h"
#include "lightMutex.h"
#include "pvector.h"
#include "pset.h"
#include "socket_fdset.h"
#include "atomicAdjust.h"

class NetDatagram;
class ConnectionManager;
class Socket_Address;
class Socket_IP;

/**
 * This is an abstract base class for a family of classes that listen for
 * activity on a socket and respond to it, for instance by reading a datagram
 * and serving it (or queueing it up for later service).
 *
 * A ConnectionReader may define an arbitrary number of threads (at least one)
 * to process datagrams coming in from an arbitrary number of sockets that it
 * is monitoring.  The number of threads is specified at construction time and
 * cannot be changed, but the set of sockets that is to be monitored may be
 * constantly modified at will.
 *
 * This is an abstract class because it doesn't define how to process each
 * received datagram.  See QueuedConnectionReader.  Also note that
 * ConnectionListener derives from this class, extending it to accept
 * connections on a rendezvous socket rather than read datagrams.
 */
class EXPCL_PANDA_NET ConnectionReader {
PUBLISHED:
  // The implementation here used to involve NSPR's multi-wait interface, but
  // that got too complicated to manage.  It turns out to be difficult to
  // protect against memory leaks caused by race conditions in that interface,
  // as designed.

  // Instead, we do our own multi-wait type stuff.  Only one thread at a time
  // can extract the next-available socket with activity on it.  That thread
  // will either (a) simply extract the next socket from the arrays returned
  // by a previous call to PR_Poll(), or (b) execute (and possibly block on) a
  // new call to PR_Poll().

  explicit ConnectionReader(ConnectionManager *manager, int num_threads,
                            const std::string &thread_name = std::string());
  virtual ~ConnectionReader();

  bool add_connection(Connection *connection);
  bool remove_connection(Connection *connection);
  bool is_connection_ok(Connection *connection);

  void poll();

  ConnectionManager *get_manager() const;
  INLINE bool is_polling() const;
  int get_num_threads() const;

  void set_raw_mode(bool mode);
  bool get_raw_mode() const;

  void set_tcp_header_size(int tcp_header_size);
  int get_tcp_header_size() const;

  void shutdown();

protected:
  virtual void flush_read_connection(Connection *connection);
  virtual void receive_datagram(const NetDatagram &datagram)=0;

  class SocketInfo {
  public:
    SocketInfo(const PT(Connection) &connection);
    bool is_udp() const;
    Socket_IP *get_socket() const;

    PT(Connection) _connection;
    bool _busy;
    bool _error;
  };
  typedef pvector<SocketInfo *> Sockets;

  void clear_manager();
  void finish_socket(SocketInfo *sinfo);

  virtual bool process_incoming_data(SocketInfo *sinfo);
  virtual bool process_incoming_udp_data(SocketInfo *sinfo);
  virtual bool process_incoming_tcp_data(SocketInfo *sinfo);
  virtual bool process_raw_incoming_udp_data(SocketInfo *sinfo);
  virtual bool process_raw_incoming_tcp_data(SocketInfo *sinfo);

protected:
  ConnectionManager *_manager;

  // These structures track the total set of sockets (connections) we know
  // about.
  Sockets _sockets;
  // This is the list of recently-removed sockets.  We can't actually delete
  // them until they're no longer _busy.
  Sockets _removed_sockets;
  // Any operations on _sockets are protected by this mutex.
  LightMutex _sockets_mutex;

private:
  void thread_run(int thread_index);

  SocketInfo *get_next_available_socket(bool allow_block,
                                        int current_thread_index);

  void rebuild_select_list();
  void accumulate_fdset(Socket_fdset &fdset);

private:
  bool _raw_mode;
  int _tcp_header_size;
  bool _shutdown;

  class ReaderThread : public Thread {
  public:
    ReaderThread(ConnectionReader *reader, const std::string &thread_name,
                 int thread_index);
    virtual void thread_main();

    ConnectionReader *_reader;
    int _thread_index;
  };

  typedef pvector< PT(ReaderThread) > Threads;
  Threads _threads;
  bool _polling;

  // These structures are used to manage selecting for noise on available
  // sockets.
  Socket_fdset _fdset;
  Sockets _selecting_sockets;
  int _next_index;
  int _num_results;
  // Threads go to sleep on this mutex waiting for their chance to read a
  // socket.
  Mutex _select_mutex;

  // This is atomically updated with the index (in _threads) of the thread
  // that is currently waiting on the PR_Poll() call.  It contains -1 if no
  // thread is so waiting.
  AtomicAdjust::Integer _currently_polling_thread;

  friend class ConnectionManager;
  friend class ReaderThread;
};

#include "connectionReader.I"

#endif
