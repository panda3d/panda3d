// Filename: connectionReader.h
// Created by:  drose (08Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CONNECTIONREADER_H
#define CONNECTIONREADER_H

#include "pandabase.h"

#include "connection.h"

#include "pointerTo.h"

#include <prio.h>
#include <prthread.h>
#include <prlock.h>
#include "pvector.h"
#include "pset.h"

class NetDatagram;
class ConnectionManager;

////////////////////////////////////////////////////////////////////
//       Class : ConnectionReader
// Description : This is an abstract base class for a family of
//               classes that listen for activity on a socket and
//               respond to it, for instance by reading a datagram and
//               serving it (or queueing it up for later service).
//
//               A ConnectionReader may define an arbitrary number of
//               threads (at least one) to process datagrams coming in
//               from an arbitrary number of sockets that it is
//               monitoring.  The number of threads is specified at
//               construction time and cannot be changed, but the set
//               of sockets that is to be monitored may be constantly
//               modified at will.
//
//               This is an abstract class because it doesn't define
//               how to process each received datagram.  See
//               QueuedConnectionReader.  Also note that
//               ConnectionListener derives from this class, extending
//               it to accept connections on a rendezvous socket
//               rather than read datagrams.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ConnectionReader {
PUBLISHED:
  // The implementation here used to involve NSPR's multi-wait
  // interface, but that got too complicated to manage.  It turns out
  // to be difficult to protect against memory leaks caused by race
  // conditions in that interface, as designed.

  // Instead, we do our own multi-wait type stuff.  Only one thread at
  // a time can extract the next-available socket with activity on it.
  // That thread will either (a) simply extract the next socket from
  // the arrays returned by a previous call to PR_Poll(), or (b)
  // execute (and possibly block on) a new call to PR_Poll().

  ConnectionReader(ConnectionManager *manager, int num_threads);
  virtual ~ConnectionReader();

  bool add_connection(const PT(Connection) &connection);
  bool remove_connection(const PT(Connection) &connection);
  bool is_connection_ok(const PT(Connection) &connection);

  void poll();

  ConnectionManager *get_manager() const;
  bool is_polling() const;
  int get_num_threads() const;

  void set_raw_mode(bool mode);
  bool get_raw_mode() const;

protected:
  virtual void receive_datagram(const NetDatagram &datagram)=0;

  class SocketInfo {
  public:
    SocketInfo(const PT(Connection) &connection);
    bool is_udp() const;
    PRFileDesc *get_socket() const;

    PT(Connection) _connection;
    bool _busy;
    bool _error;
  };

  void shutdown();
  void clear_manager();
  void finish_socket(SocketInfo *sinfo);

  virtual void process_incoming_data(SocketInfo *sinfo);
  virtual void process_incoming_udp_data(SocketInfo *sinfo);
  virtual void process_incoming_tcp_data(SocketInfo *sinfo);
  virtual void process_raw_incoming_udp_data(SocketInfo *sinfo);
  virtual void process_raw_incoming_tcp_data(SocketInfo *sinfo);

private:
  static void thread_start(void *data);
  void thread_run();

  SocketInfo *get_next_available_socket(PRIntervalTime timeout,
                                        PRInt32 current_thread_index);

  void rebuild_poll_list();

protected:
  ConnectionManager *_manager;

private:
  bool _raw_mode;
  bool _shutdown;

  typedef pvector<PRThread *> Threads;
  Threads _threads;
  PRLock *_startup_mutex;
  bool _polling;

  // These structures are used to manage polling for noise on
  // available sockets.
  typedef pvector<PRPollDesc> Poll;
  typedef pvector<SocketInfo *> Sockets;
  Poll _poll;
  Sockets _polled_sockets;
  int _next_index;
  int _num_results;
  // Threads go to sleep on this mutex waiting for their chance to
  // read a socket.
  PRLock *_select_mutex;

  // This is atomically updated with the index (in _threads) of the
  // thread that is currently waiting on the PR_Poll() call.  It
  // contains -1 if no thread is so waiting.
  PRInt32 _currently_polling_thread;

  // These structures track the total set of sockets (connections) we
  // know about.
  Sockets _sockets;
  // This is the list of recently-removed sockets.  We can't actually
  // delete them until they're no longer _busy.
  Sockets _removed_sockets;
  // Threads may set this true to force the polling thread to rebuild
  // its Poll() list.
  bool _reexamine_sockets;
  // Any operations on _sockets are protected by this mutex.
  PRLock *_sockets_mutex;


friend class ConnectionManager;
};

#endif
