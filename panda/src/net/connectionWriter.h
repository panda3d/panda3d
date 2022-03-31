/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connectionWriter.h
 * @author drose
 * @date 2000-02-08
 */

#ifndef CONNECTIONWRITER_H
#define CONNECTIONWRITER_H

#include "pandabase.h"
#include "datagramQueue.h"
#include "connection.h"
#include "pointerTo.h"
#include "thread.h"
#include "pvector.h"

class ConnectionManager;
class NetAddress;

/**
 * This class handles threaded delivery of datagrams to various TCP or UDP
 * sockets.
 *
 * A ConnectionWriter may define an arbitrary number of threads (0 or more) to
 * write its datagrams to sockets.  The number of threads is specified at
 * construction time and cannot be changed.
 */
class EXPCL_PANDA_NET ConnectionWriter {
PUBLISHED:
  explicit ConnectionWriter(ConnectionManager *manager, int num_threads,
                            const std::string &thread_name = std::string());
  ~ConnectionWriter();

  void set_max_queue_size(int max_size);
  int get_max_queue_size() const;
  int get_current_queue_size() const;

  BLOCKING bool send(const Datagram &datagram,
                     const PT(Connection) &connection,
                     bool block = false);

  BLOCKING bool send(const Datagram &datagram,
                     const PT(Connection) &connection,
                     const NetAddress &address,
                     bool block = false);

  bool is_valid_for_udp(const Datagram &datagram) const;

  ConnectionManager *get_manager() const;
  bool is_immediate() const;
  int get_num_threads() const;

  void set_raw_mode(bool mode);
  bool get_raw_mode() const;

  void set_tcp_header_size(int tcp_header_size);
  int get_tcp_header_size() const;

  void shutdown();

protected:
  void clear_manager();

private:
  void thread_run(int thread_index);
  bool send_datagram(const NetDatagram &datagram);

protected:
  ConnectionManager *_manager;

private:
  bool _raw_mode;
  int _tcp_header_size;
  DatagramQueue _queue;
  bool _shutdown;

  class WriterThread : public Thread {
  public:
    WriterThread(ConnectionWriter *writer, const std::string &thread_name,
                 int thread_index);
    virtual void thread_main();

    ConnectionWriter *_writer;
    int _thread_index;
  };

  typedef pvector< PT(WriterThread) > Threads;
  Threads _threads;

  bool _immediate;

  friend class ConnectionManager;
  friend class WriterThread;
};

#endif
