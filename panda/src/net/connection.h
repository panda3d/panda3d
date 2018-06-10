/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connection.h
 * @author jns
 * @date 2000-02-07
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include "pandabase.h"
#include "referenceCount.h"
#include "netAddress.h"
#include "lightReMutex.h"

class Socket_IP;
class ConnectionManager;
class NetDatagram;

/**
 * Represents a single TCP or UDP socket for input or output.
 */
class EXPCL_PANDA_NET Connection : public ReferenceCount {
PUBLISHED:
  explicit Connection(ConnectionManager *manager, Socket_IP *socket);
  ~Connection();

  NetAddress get_address() const;
  ConnectionManager *get_manager() const;

  Socket_IP *get_socket() const;

  void set_collect_tcp(bool collect_tcp);
  bool get_collect_tcp() const;
  void set_collect_tcp_interval(double interval);
  double get_collect_tcp_interval() const;

  BLOCKING bool consider_flush();
  BLOCKING bool flush();

  // Socket options.  void set_nonblock(bool flag);
  void set_linger(bool flag, double time);
  void set_reuse_addr(bool flag);
  void set_keep_alive(bool flag);
  void set_recv_buffer_size(int size);
  void set_send_buffer_size(int size);
  void set_ip_time_to_live(int ttl);
  void set_ip_type_of_service(int tos);
  void set_no_delay(bool flag);
  void set_max_segment(int size);

private:
  bool send_datagram(const NetDatagram &datagram, int tcp_header_size);
  bool send_raw_datagram(const NetDatagram &datagram);
  bool do_flush();
  bool check_send_error(bool okflag);

  ConnectionManager *_manager;
  Socket_IP *_socket;
  LightReMutex _write_mutex;

  bool _collect_tcp;
  double _collect_tcp_interval;
  double _queued_data_start;
  std::string _queued_data;
  int _queued_count;

  friend class ConnectionWriter;
};

#endif
