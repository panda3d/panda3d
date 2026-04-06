/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connectionManager.h
 * @author jns
 * @date 2000-02-07
 */

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "pandabase.h"

#include "netDatagram.h"
#include "connection.h"
#include "pointerTo.h"
#include "pset.h"
#include "pvector.h"
#include "lightMutex.h"

class NetAddress;
class ConnectionReader;
class ConnectionWriter;

/**
 * The primary interface to the low-level networking layer in this package.  A
 * ConnectionManager is used to establish and destroy TCP and UDP connections.
 * Communication on these connections, once established, is handled via
 * ConnectionReader, ConnectionWriter, and ConnectionListener.
 *
 * You may use this class directly if you don't care about tracking which
 * connections have been unexpectedly closed; otherwise, you should use
 * QueuedConnectionManager to get reports about these events (or derive your
 * own class to handle these events properly).
 */
class EXPCL_PANDA_NET ConnectionManager {
PUBLISHED:
  ConnectionManager();
  virtual ~ConnectionManager();

  PT(Connection) open_UDP_connection(uint16_t port = 0);
  PT(Connection) open_UDP_connection(const std::string &hostname, uint16_t port, bool for_broadcast = false);

  BLOCKING PT(Connection) open_TCP_server_rendezvous(uint16_t port, int backlog);
  BLOCKING PT(Connection) open_TCP_server_rendezvous(const std::string &hostname,
                                                     uint16_t port, int backlog);
  BLOCKING PT(Connection) open_TCP_server_rendezvous(const NetAddress &address,
                                                     int backlog);
  BLOCKING PT(Connection) open_TCP_client_connection(const NetAddress &address,
                                                     int timeout_ms);
  BLOCKING PT(Connection) open_TCP_client_connection(const std::string &hostname,
                                                     uint16_t port, int timeout_ms);

  bool close_connection(const PT(Connection) &connection);
  BLOCKING bool wait_for_readers(double timeout);

  static std::string get_host_name();

  class EXPCL_PANDA_NET Interface {
  PUBLISHED:
    const std::string &get_name() const { return _name; }
    const std::string &get_mac_address() const { return _mac_address; }
    bool has_ip() const { return (_flags & F_has_ip) != 0; }
    const NetAddress &get_ip() const { return _ip; }
    bool has_netmask() const { return (_flags & F_has_netmask) != 0; }
    const NetAddress &get_netmask() const { return _netmask; }
    bool has_broadcast() const { return (_flags & F_has_broadcast) != 0; }
    const NetAddress &get_broadcast() const { return _broadcast; }
    bool has_p2p() const { return (_flags & F_has_p2p) != 0; }
    const NetAddress &get_p2p() const { return _p2p; }

    void output(std::ostream &out) const;

  public:
    Interface() { _flags = 0; }
    void set_name(const std::string &name) { _name = name; }
    void set_mac_address(const std::string &mac_address) { _mac_address = mac_address; }
    void set_ip(const NetAddress &ip) { _ip = ip; _flags |= F_has_ip; }
    void set_netmask(const NetAddress &ip) { _netmask = ip; _flags |= F_has_netmask; }
    void set_broadcast(const NetAddress &ip) { _broadcast = ip; _flags |= F_has_broadcast; }
    void set_p2p(const NetAddress &ip) { _p2p = ip; _flags |= F_has_p2p; }

  private:
    std::string _name;
    std::string _mac_address;

    NetAddress _ip;
    NetAddress _netmask;
    NetAddress _broadcast;
    NetAddress _p2p;
    int _flags;

    enum Flags {
      F_has_ip        = 0x001,
      F_has_netmask   = 0x002,
      F_has_broadcast = 0x004,
      F_has_p2p       = 0x008,
    };
  };

  void scan_interfaces();
  size_t get_num_interfaces();
  const Interface &get_interface(size_t n);
  MAKE_SEQ(get_interfaces, get_num_interfaces, get_interface);

  MAKE_PROPERTY(host_name, get_host_name);
  MAKE_SEQ_PROPERTY(interfaces, get_num_interfaces, get_interface);

protected:
  void new_connection(const PT(Connection) &connection);
  virtual void flush_read_connection(Connection *connection);
  virtual void connection_reset(const PT(Connection) &connection,
                                bool okflag);

  void add_reader(ConnectionReader *reader);
  void remove_reader(ConnectionReader *reader);
  void add_writer(ConnectionWriter *writer);
  void remove_writer(ConnectionWriter *writer);

  std::string format_mac_address(const unsigned char *data, size_t data_size);

  typedef phash_set< PT(Connection) > Connections;
  typedef phash_set<ConnectionReader *, pointer_hash> Readers;
  typedef phash_set<ConnectionWriter *, pointer_hash> Writers;
  Connections _connections;
  Readers _readers;
  Writers _writers;
  LightMutex _set_mutex;

  typedef pvector<Interface> Interfaces;
  Interfaces _interfaces;
  bool _interfaces_scanned;

private:
  friend class ConnectionReader;
  friend class ConnectionWriter;
  friend class ConnectionListener;
  friend class Connection;
};

INLINE std::ostream &operator << (std::ostream &out, const ConnectionManager::Interface &iface) {
  iface.output(out);
  return out;
}

#endif
