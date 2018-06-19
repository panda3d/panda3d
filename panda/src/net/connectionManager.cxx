/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connectionManager.cxx
 * @author jns
 * @date 2000-02-07
 */

#include "connectionManager.h"
#include "connection.h"
#include "connectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "config_net.h"
#include "socket_udp.h"
#include "socket_tcp_listen.h"
#include "lightMutexHolder.h"
#include "trueClock.h"

#if defined(CPPPARSER)
#elif defined(WIN32_VC) || defined(WIN64_VC)
#include <winsock2.h>  // For gethostname()
#include <Iphlpapi.h> // For GetAdaptersAddresses()
#elif defined(__ANDROID__)
#include <net/if.h>
#else
#include <net/if.h>
#include <ifaddrs.h>
#endif

using std::stringstream;
using std::string;

/**
 *
 */
ConnectionManager::
ConnectionManager() : _set_mutex("ConnectionManager::_set_mutex")
{
  _interfaces_scanned = false;
}

/**
 *
 */
ConnectionManager::
~ConnectionManager() {
  // Notify all of our associated readers and writers that we're gone.
  Readers::iterator ri;
  for (ri = _readers.begin(); ri != _readers.end(); ++ri) {
    (*ri)->clear_manager();
  }
  Writers::iterator wi;
  for (wi = _writers.begin(); wi != _writers.end(); ++wi) {
    (*wi)->clear_manager();
  }
}


/**
 * Opens a socket for sending and/or receiving UDP packets.  If the port
 * number is greater than zero, the UDP connection will be opened for
 * listening on the indicated port; otherwise, it will be useful only for
 * sending.
 *
 * Use a ConnectionReader and ConnectionWriter to handle the actual
 * communication.
 */
PT(Connection) ConnectionManager::
open_UDP_connection(uint16_t port) {
  return open_UDP_connection("", port);
}

/**
 * Opens a socket for sending and/or receiving UDP packets.  If the port
 * number is greater than zero, the UDP connection will be opened for
 * listening on the indicated port; otherwise, it will be useful only for
 * sending.
 *
 * This variant accepts both a hostname and port to listen on a particular
 * interface; if the hostname is empty, all interfaces will be available,
 * both IPv4 and IPv6.
 *
 * If for_broadcast is true, this UDP connection will be configured to send
 * and/or receive messages on the broadcast address (255.255.255.255);
 * otherwise, these messages may be automatically filtered by the OS.
 *
 * Use a ConnectionReader and ConnectionWriter to handle the actual
 * communication.
 */
PT(Connection) ConnectionManager::
open_UDP_connection(const string &hostname, uint16_t port, bool for_broadcast) {
  Socket_UDP *socket = new Socket_UDP;

  if (port > 0) {
    bool okflag;
    NetAddress address;
    if (hostname.empty()) {
      // The empty string means to listen on both IPv4 and IPv6 interfaces.
      okflag = socket->OpenForInput(port);
    } else {
      address.set_host(hostname, port);
      okflag = socket->OpenForInput(address.get_addr());
    }

    if (!okflag) {
      if (hostname.empty()) {
        net_cat.error()
          << "Unable to bind to port " << port << " for UDP.\n";
      } else {
        net_cat.error()
          << "Unable to bind to " << address << " for UDP.\n";
      }
      delete socket;
      return PT(Connection)();
    }

    const char *broadcast_note = "";
    if (for_broadcast) {
      socket->SetToBroadCast();
      broadcast_note = "broadcast ";
    }

    if (hostname.empty()) {
      net_cat.info()
        << "Creating UDP " << broadcast_note << "connection for port " << port << "\n";
    } else {
      net_cat.info()
        << "Creating UDP " << broadcast_note << "connection for " << address << "\n";
    }

  } else {
    if (!socket->InitNoAddress()) {
      net_cat.error()
        << "Unable to initialize outgoing UDP.\n";
      delete socket;
      return PT(Connection)();
    }

    const char *broadcast_note = "";
    if (for_broadcast) {
      socket->SetToBroadCast();
      broadcast_note = "broadcast ";
    }

    net_cat.info()
      << "Creating outgoing UDP " << broadcast_note << "connection\n";
  }

  PT(Connection) connection = new Connection(this, socket);
  new_connection(connection);
  return connection;
}

/**
 * Creates a socket to be used as a rendezvous socket for a server to listen
 * for TCP connections.  The socket returned by this call should only be added
 * to a ConnectionListener (not to a generic ConnectionReader).
 *
 * This variant of this method accepts a single port, and will listen to that
 * port on all available interfaces, both IPv4 and IPv6.
 *
 * backlog is the maximum length of the queue of pending connections.
 */
PT(Connection) ConnectionManager::
open_TCP_server_rendezvous(uint16_t port, int backlog) {
  Socket_TCP_Listen *socket = new Socket_TCP_Listen;
  if (!socket->OpenForListen(port, backlog)) {
    net_cat.info()
      << "Unable to listen to port " << port << " for TCP.\n";
    delete socket;
    return PT(Connection)();
  }

  net_cat.info()
    << "Listening for TCP connections on port " << port << "\n";

  PT(Connection) connection = new Connection(this, socket);
  new_connection(connection);
  return connection;
}

/**
 * Creates a socket to be used as a rendezvous socket for a server to listen
 * for TCP connections.  The socket returned by this call should only be added
 * to a ConnectionListener (not to a generic ConnectionReader).
 *
 * This variant of this method accepts a "hostname", which is usually just an
 * IP address in dotted notation, and a port number.  It will listen on the
 * interface indicated by the IP address.  If the IP address is empty string,
 * it will listen on all interfaces.
 *
 * backlog is the maximum length of the queue of pending connections.
 */
PT(Connection) ConnectionManager::
open_TCP_server_rendezvous(const string &hostname, uint16_t port, int backlog) {
  if (hostname.empty()) {
    return open_TCP_server_rendezvous(port, backlog);
  } else {
    NetAddress address;
    address.set_host(hostname, port);
    return open_TCP_server_rendezvous(address, backlog);
  }
}

/**
 * Creates a socket to be used as a rendezvous socket for a server to listen
 * for TCP connections.  The socket returned by this call should only be added
 * to a ConnectionListener (not to a generic ConnectionReader).
 *
 * This variant of this method accepts a NetAddress, which allows you to
 * specify a specific interface to listen to.
 *
 * backlog is the maximum length of the queue of pending connections.
 */
PT(Connection) ConnectionManager::
open_TCP_server_rendezvous(const NetAddress &address, int backlog) {
  Socket_TCP_Listen *socket = new Socket_TCP_Listen;
  if (!socket->OpenForListen(address.get_addr(), backlog)) {
    net_cat.info()
      << "Unable to listen to " << address << " for TCP.\n";
    delete socket;
    return PT(Connection)();
  }

  net_cat.info()
    << "Listening for TCP connections on " << address << "\n";

  PT(Connection) connection = new Connection(this, socket);
  new_connection(connection);
  return connection;
}

/**
 * Attempts to establish a TCP client connection to a server at the indicated
 * address.  If the connection is not established within timeout_ms
 * milliseconds, a null connection is returned.
 */
PT(Connection) ConnectionManager::
open_TCP_client_connection(const NetAddress &address, int timeout_ms) {
  Socket_TCP *socket = new Socket_TCP;

  // We always open the connection with non-blocking mode first, so we can
  // implement the timeout.
  bool okflag = socket->ActiveOpenNonBlocking(address.get_addr());
  if (okflag && socket->GetLastError() == LOCAL_CONNECT_BLOCKING) {
    // Now wait for the socket to connect.
    TrueClock *clock = TrueClock::get_global_ptr();
    double start = clock->get_short_time();
    Thread::force_yield();
    Socket_fdset fset;
    fset.setForSocket(*socket);
    int ready = fset.WaitForWrite(true, 0);
    while (ready == 0) {
      double elapsed = clock->get_short_time() - start;
      if (elapsed * 1000.0 > timeout_ms) {
        // Timeout.
        okflag = false;
        break;
      }
      Thread::force_yield();
      fset.setForSocket(*socket);
      ready = fset.WaitForWrite(true, 0);
    }
  }

  if (okflag) {
    // So, the connect() operation finished, but did it succeed or fail?
    if (socket->GetPeerName().is_any()) {
      // No peer means it failed.
      okflag = false;
    }
  }

  if (!okflag) {
    net_cat.error()
      << "Unable to open TCP connection to server " << address << "\n";
    delete socket;
    return PT(Connection)();
  }

#if !defined(HAVE_THREADS) || !defined(SIMPLE_THREADS)
  // Now we have opened the socket in nonblocking mode.  Unless we're using
  // SIMPLE_THREADS, though, we really want the socket in blocking mode (since
  // that's what we support here).  Change it.
  socket->SetBlocking();

#endif  // SIMPLE_THREADS

  net_cat.info()
    << "Opened TCP connection to server " << address << "\n";

  PT(Connection) connection = new Connection(this, socket);
  new_connection(connection);
  return connection;
}

/**
 * This is a shorthand version of the function to directly establish
 * communications to a named host and port.
 */
PT(Connection) ConnectionManager::
open_TCP_client_connection(const string &hostname, uint16_t port,
                           int timeout_ms) {
  NetAddress address;
  if (!address.set_host(hostname, port)) {
    return PT(Connection)();
  }

  return open_TCP_client_connection(address, timeout_ms);
}

/**
 * Terminates a UDP or TCP socket previously opened.  This also removes it
 * from any associated ConnectionReader or ConnectionListeners.
 *
 * The socket itself may not be immediately closed--it will not be closed
 * until all outstanding pointers to it are cleared, including any pointers
 * remaining in NetDatagrams recently received from the socket.
 *
 * The return value is true if the connection was marked to be closed, or
 * false if close_connection() had already been called (or the connection did
 * not belong to this ConnectionManager).  In neither case can you infer
 * anything about whether the connection has *actually* been closed yet based
 * on the return value.
 */
bool ConnectionManager::
close_connection(const PT(Connection) &connection) {
  if (connection != nullptr) {
    connection->flush();
  }

  {
    LightMutexHolder holder(_set_mutex);
    Connections::iterator ci = _connections.find(connection);
    if (ci == _connections.end()) {
      // Already closed, or not part of this ConnectionManager.
      return false;
    }
    _connections.erase(ci);

    Readers::iterator ri;
    for (ri = _readers.begin(); ri != _readers.end(); ++ri) {
      (*ri)->remove_connection(connection);
    }
  }

  Socket_IP *socket = connection->get_socket();

  // We can't *actually* close the connection right now, because there might
  // be outstanding pointers to it.  But we can at least shut it down.  It
  // will be eventually closed when all the pointers let go.

  net_cat.info()
    << "Shutting down connection " << (void *)connection
    << " locally.\n";
  socket->Close();

  return true;
}


/**
 * Blocks the process for timeout number of seconds, or until any data is
 * available on any of the non-threaded ConnectionReaders or
 * ConnectionListeners, whichever comes first.  The return value is true if
 * there is data available (but you have to iterate through all readers to
 * find it), or false if the timeout occurred without any data.
 *
 * If the timeout value is negative, this will block forever or until data is
 * available.
 *
 * This only works if all ConnectionReaders and ConnectionListeners are non-
 * threaded.  If any threaded ConnectionReaders are part of the
 * ConnectionManager, the timeout value is implicitly treated as 0.
 */
bool ConnectionManager::
wait_for_readers(double timeout) {
  bool block_forever = false;
  if (timeout < 0.0) {
    block_forever = true;
    timeout = 0.0;
  }

  TrueClock *clock = TrueClock::get_global_ptr();
  double now = clock->get_short_time();
  double stop = now + timeout;
  do {
    Socket_fdset fdset;
    fdset.clear();
    bool any_threaded = false;

    {
      LightMutexHolder holder(_set_mutex);

      Readers::iterator ri;
      for (ri = _readers.begin(); ri != _readers.end(); ++ri) {
        ConnectionReader *reader = (*ri);
        if (reader->is_polling()) {
          // If it's a polling reader, we can wait for its socket.  (If it's a
          // threaded reader, we can't do anything here.)
          reader->accumulate_fdset(fdset);
        } else {
          any_threaded = true;
          stop = now;
          block_forever = false;
        }
      }
    }

    double wait_timeout = get_net_max_block();
    if (!block_forever) {
      wait_timeout = std::min(wait_timeout, stop - now);
    }

    uint32_t wait_timeout_ms = (uint32_t)(wait_timeout * 1000.0);
    if (any_threaded) {
      // If there are any threaded ConnectionReaders, we can't block at all.
      wait_timeout_ms = 0;
    }
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
    // In the presence of SIMPLE_THREADS, we never wait at all, but rather we
    // yield the thread if we come up empty (so that we won't block the entire
    // process).
    wait_timeout_ms = 0;
#endif
    int num_results = fdset.WaitForRead(false, wait_timeout_ms);
    if (num_results != 0) {
      // If we got an answer (or an error), return success.  The caller can
      // then figure out what happened.
      if (num_results < 0) {
        // Go ahead and yield the timeslice if we got an error.
        Thread::force_yield();
      }
      return true;
    }

    // No answer yet, so yield and wait some more.  We don't actually block
    // forever, even in the threaded case, so we can detect ConnectionReaders
    // being added and removed and such.
    Thread::force_yield();

    now = clock->get_short_time();
  } while (now < stop || block_forever);

  // Timeout occurred; no data.
  return false;
}

/**
 * Returns the name of this particular machine on the network, if available,
 * or the empty string if the hostname cannot be determined.
 */
string ConnectionManager::
get_host_name() {
  char temp_buff[1024];
  if (gethostname(temp_buff, 1024) == 0) {
    return string(temp_buff);
  }

  return string();
}

/**
 * Repopulates the list reported by get_num_interface()/get_interface().  It
 * is not necessary to call this explicitly, unless you want to re-determine
 * the connected interfaces (for instance, if you suspect the hardware has
 * recently changed).
 */
void ConnectionManager::
scan_interfaces() {
  LightMutexHolder holder(_set_mutex);
  _interfaces.clear();
  _interfaces_scanned = true;

#ifdef WIN32_VC
  int flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
  ULONG family = support_ipv6 ? AF_UNSPEC : AF_INET;
  ULONG buffer_size = 0;
  ULONG result = GetAdaptersAddresses(family, flags, nullptr, nullptr, &buffer_size);
  if (result == ERROR_BUFFER_OVERFLOW) {
    IP_ADAPTER_ADDRESSES *addresses = (IP_ADAPTER_ADDRESSES *)PANDA_MALLOC_ARRAY(buffer_size);
    result = GetAdaptersAddresses(family, flags, nullptr, addresses, &buffer_size);
    if (result == ERROR_SUCCESS) {
      IP_ADAPTER_ADDRESSES *p = addresses;
      while (p != nullptr) {
        // p->AdapterName appears to be a GUID.  Not sure if this is actually
        // useful to anyone; we'll store the "friendly name" instead.
        TextEncoder encoder;
        encoder.set_wtext(std::wstring(p->FriendlyName));
        string friendly_name = encoder.get_text();

        Interface iface;
        iface.set_name(friendly_name);

        if (p->PhysicalAddressLength > 0) {
          iface.set_mac_address(format_mac_address((const unsigned char *)p->PhysicalAddress, p->PhysicalAddressLength));
        }

        if (p->OperStatus == IfOperStatusUp) {
          // Prefixes are a linked list, in the order Network IP, Adapter IP,
          // Broadcast IP (plus more).
          NetAddress addresses[3];
          IP_ADAPTER_PREFIX *m = p->FirstPrefix;
          int mc = 0;
          while (m != nullptr && mc < 3) {
            addresses[mc] = NetAddress(Socket_Address(*m->Address.lpSockaddr));
            m = m->Next;
            ++mc;
          }

          if (mc > 1) {
            iface.set_ip(addresses[1]);
          }

          if (mc > 2) {
            iface.set_broadcast(addresses[2]);

            // Now, we can infer the netmask by the difference between the
            // network address (the first address) and the broadcast address
            // (the last address).
            if (addresses[0].get_addr().get_family() == AF_INET &&
                addresses[2].get_addr().get_family() == AF_INET) {
              uint32_t netmask = addresses[0].get_ip() - addresses[2].get_ip() - 1;
              Socket_Address sa;
              sa.set_host(netmask, 0);
              iface.set_netmask(NetAddress(sa));
            }
          }
        }

        _interfaces.push_back(iface);
        p = p->Next;
      }
    }
    PANDA_FREE_ARRAY(addresses);
  }

#elif defined(__ANDROID__)
  // TODO: implementation using netlink_socket?

#else  // WIN32_VC
  struct ifaddrs *ifa;
  if (getifaddrs(&ifa) != 0) {
    // Failure.
    net_cat.error()
      << "Failed to call getifaddrs\n";
    return;
  }

  struct ifaddrs *p = ifa;
  while (p != nullptr) {
    if (p->ifa_addr->sa_family == AF_INET ||
        (support_ipv6 && p->ifa_addr->sa_family == AF_INET6)) {
      Interface iface;
      iface.set_name(p->ifa_name);
      if (p->ifa_addr != nullptr) {
        iface.set_ip(NetAddress(Socket_Address(*p->ifa_addr)));
      }
      if (p->ifa_netmask != nullptr) {
        iface.set_netmask(NetAddress(Socket_Address(*p->ifa_netmask)));
      }
      if ((p->ifa_flags & IFF_BROADCAST) && p->ifa_broadaddr != nullptr) {
        iface.set_broadcast(NetAddress(Socket_Address(*p->ifa_broadaddr)));
      } else if ((p->ifa_flags & IFF_POINTOPOINT) && p->ifa_dstaddr != nullptr) {
        iface.set_p2p(NetAddress(Socket_Address(*p->ifa_dstaddr)));
      }
      _interfaces.push_back(iface);
    }

    p = p->ifa_next;
  }

  freeifaddrs(ifa);

#endif // WIN32_VC
}

/**
 * This returns the number of usable network interfaces detected on this
 * machine.  See scan_interfaces() to repopulate this list.
 */
size_t ConnectionManager::
get_num_interfaces() {
  if (!_interfaces_scanned) {
    scan_interfaces();
  }
  LightMutexHolder holder(_set_mutex);
  return _interfaces.size();
}

/**
 * Returns the nth usable network interface detected on this machine.
 * See scan_interfaces() to repopulate this list.
 */
const ConnectionManager::Interface &ConnectionManager::
get_interface(size_t n) {
  if (!_interfaces_scanned) {
    scan_interfaces();
  }
  LightMutexHolder holder(_set_mutex);
  nassertr(n < _interfaces.size(), _interfaces[0]);
  return _interfaces[n];
}

/**
 * This internal function is called whenever a new connection is established.
 * It allows the ConnectionManager to save all of the pointers to open
 * connections so they can't be inadvertently deleted until close_connection()
 * is called.
 */
void ConnectionManager::
new_connection(const PT(Connection) &connection) {
  LightMutexHolder holder(_set_mutex);
  _connections.insert(connection);
}

/**
 * An internal function called by ConnectionWriter only when a write failure
 * has occurred.  This method ensures that all of the read data has been
 * flushed from the pipe before the connection is fully removed.
 */
void ConnectionManager::
flush_read_connection(Connection *connection) {
  Readers readers;
  {
    LightMutexHolder holder(_set_mutex);
    Connections::iterator ci = _connections.find(connection);
    if (ci == _connections.end()) {
      // Already closed, or not part of this ConnectionManager.
      return;
    }
    _connections.erase(ci);

    // Get a copy first, so we can release the lock before traversing.
    readers = _readers;
  }
  Readers::iterator ri;
  for (ri = readers.begin(); ri != readers.end(); ++ri) {
    (*ri)->flush_read_connection(connection);
  }

  Socket_IP *socket = connection->get_socket();
  socket->Close();
}

/**
 * An internal function called by the ConnectionReader, ConnectionWriter, or
 * ConnectionListener when a connection has been externally reset.  This adds
 * the connection to the queue of those which have recently been reset.
 */
void ConnectionManager::
connection_reset(const PT(Connection) &connection, bool okflag) {
  if (net_cat.is_info()) {
    if (okflag) {
      net_cat.info()
        << "Connection " << (void *)connection
        << " was closed normally by the other end";

    } else {
      net_cat.info()
        << "Lost connection " << (void *)connection
        << " unexpectedly\n";
    }
  }

  // Turns out we do need to explicitly mark the connection as closed
  // immediately, rather than waiting for the user to do it, since otherwise
  // we'll keep trying to listen for noise on the socket and we'll always hear
  // a "yes" answer.
  close_connection(connection);
}

/**
 * This internal function is called by ConnectionReader when it is
 * constructed.
 */
void ConnectionManager::
add_reader(ConnectionReader *reader) {
  LightMutexHolder holder(_set_mutex);
  _readers.insert(reader);
}

/**
 * This internal function is called by ConnectionReader when it is destructed.
 */
void ConnectionManager::
remove_reader(ConnectionReader *reader) {
  LightMutexHolder holder(_set_mutex);
  _readers.erase(reader);
}

/**
 * This internal function is called by ConnectionWriter when it is
 * constructed.
 */
void ConnectionManager::
add_writer(ConnectionWriter *writer) {
  LightMutexHolder holder(_set_mutex);
  _writers.insert(writer);
}

/**
 * This internal function is called by ConnectionWriter when it is destructed.
 */
void ConnectionManager::
remove_writer(ConnectionWriter *writer) {
  LightMutexHolder holder(_set_mutex);
  _writers.erase(writer);
}

/**
 * Formats a device's MAC address into a string.
 */
string ConnectionManager::
format_mac_address(const unsigned char *data, size_t data_size) {
  std::stringstream strm;
  for (size_t di = 0; di < data_size; ++di) {
    if (di != 0) {
      strm << "-";
    }
    strm << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)data[di];
  }

  return strm.str();
}

/**
 *
 */
void ConnectionManager::Interface::
output(std::ostream &out) const {
  out << get_name() << " [";
  if (has_ip()) {
    out << " " << get_ip().get_ip_string();
  }
  if (has_netmask()) {
    out << " netmask " << get_netmask().get_ip_string();
  }
  if (has_broadcast()) {
    out << " broadcast " << get_broadcast().get_ip_string();
  }
  if (has_p2p()) {
    out << " p2p " << get_p2p().get_ip_string();
  }
  out << " ]";
}
