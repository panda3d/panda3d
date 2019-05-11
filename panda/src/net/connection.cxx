/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file connection.cxx
 * @author jns
 * @date 2000-02-07
 */

#include "connection.h"
#include "connectionManager.h"
#include "netDatagram.h"
#include "datagramTCPHeader.h"
#include "datagramUDPHeader.h"
#include "config_net.h"
#include "config_express.h" // for collect_tcp
#include "trueClock.h"
#include "pnotify.h"
#include "lightReMutexHolder.h"
#include "socket_ip.h"
#include "socket_tcp.h"
#include "socket_udp.h"
#include "dcast.h"


/**
 * Creates a connection.  Normally this constructor should not be used
 * directly by user code; use one of the methods in ConnectionManager to make
 * a new connection.
 */
Connection::
Connection(ConnectionManager *manager, Socket_IP *socket) :
  _manager(manager),
  _socket(socket)
{
  _collect_tcp = collect_tcp;
  _collect_tcp_interval = collect_tcp_interval;
  _queued_data_start = 0.0;
  _queued_count = 0;

#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  // In the presence of SIMPLE_THREADS, we use non-blocking IO.  We simulate
  // blocking by yielding the thread.
  if (_socket->SetNonBlocking() != ALL_OK) {
    net_cat.warning()
      << "Unable to set non-blocking status on socket\n";
  }
#endif
}

/**
 * Closes a connection.
 */
Connection::
~Connection() {
  net_cat.info()
    << "Deleting connection " << (void *)this << "\n";

  if (_socket != nullptr) {
    flush();

    _socket->Close();
    delete _socket;
  }
}

/**
 * Returns the address bound to this connection, if it is a TCP connection.
 */
NetAddress Connection::
get_address() const {
  Socket_Address addr = _socket->GetPeerName();
  return NetAddress(addr);
}

/**
 * Returns a pointer to the ConnectionManager object that serves this
 * connection.
 */
ConnectionManager *Connection::
get_manager() const {
  return _manager;
}

/**
 * Returns the internal Socket_IP that defines the connection.
 */
Socket_IP *Connection::
get_socket() const {
  return _socket;
}

/**
 * Enables or disables "collect-tcp" mode.  In this mode, individual TCP
 * packets are not sent immediately, but rather they are collected together
 * and accumulated to be sent periodically as one larger TCP packet.  This
 * cuts down on overhead from the TCP/IP protocol, especially if many small
 * packets need to be sent on the same connection, but it introduces
 * additional latency (since packets must be held before they can be sent).
 *
 * See set_collect_tcp_interval() to specify the interval of time for which to
 * hold packets before sending them.
 *
 * If you enable this mode, you may also need to periodically call
 * consider_flush() to flush the queue if no packets have been sent recently.
 */
void Connection::
set_collect_tcp(bool collect_tcp) {
  _collect_tcp = collect_tcp;
}

/**
 * Returns the current setting of "collect-tcp" mode.  See set_collect_tcp().
 */
bool Connection::
get_collect_tcp() const {
  return _collect_tcp;
}

/**
 * Specifies the interval in time, in seconds, for which to hold TCP packets
 * before sending all of the recently received packets at once.  This only has
 * meaning if "collect-tcp" mode is enabled; see set_collect_tcp().
 */
void Connection::
set_collect_tcp_interval(double interval) {
  _collect_tcp_interval = interval;
}

/**
 * Returns the interval in time, in seconds, for which to hold TCP packets
 * before sending all of the recently received packets at once.  This only has
 * meaning if "collect-tcp" mode is enabled; see set_collect_tcp().
 */
double Connection::
get_collect_tcp_interval() const {
  return _collect_tcp_interval;
}

/**
 * Sends the most recently queued TCP datagram(s) if enough time has elapsed.
 * This only has meaning if set_collect_tcp() has been set to true.
 */
bool Connection::
consider_flush() {
  LightReMutexHolder holder(_write_mutex);

  if (!_collect_tcp) {
    return do_flush();

  } else {
    double elapsed =
      TrueClock::get_global_ptr()->get_short_time() - _queued_data_start;
    // If the elapsed time is negative, someone must have reset the clock
    // back, so just go ahead and flush.
    if (elapsed < 0.0 || elapsed >= _collect_tcp_interval) {
      return do_flush();
    }
  }

  return true;
}

/**
 * Sends the most recently queued TCP datagram(s) now.  This only has meaning
 * if set_collect_tcp() has been set to true.
 */
bool Connection::
flush() {
  LightReMutexHolder holder(_write_mutex);
  return do_flush();
}


/**
 * Sets whether nonblocking I/O should be in effect.
 */
/*
This method is disabled.  We don't provide enough interface to use
non-blocking I/O effectively at this level, so we shouldn't provide
this call.  Specifically, we don't provide a way to query whether an
operation failed because it would have blocked or not.
void Connection::
set_nonblock(bool flag) {
  if (flag) {
    _socket->SetNonBlocking();
  } else {
    _socket->SetBlocking();
  }
}
*/

/**
 * Sets the time to linger on close if data is present.  If flag is false,
 * when you close a socket with data available the system attempts to deliver
 * the data to the peer (the default behavior).  If flag is false but time is
 * zero, the system discards any undelivered data when you close the socket.
 * If flag is false but time is nonzero, the system waits up to time seconds
 * to deliver the data.
 */
void Connection::
set_linger(bool flag, double time) {
  Socket_TCP *tcp;
  DCAST_INTO_V(tcp, _socket);

  if (flag) {
    tcp->SetLinger((int)time);
  } else {
    tcp->DontLinger();
  }
}

/**
 * Sets whether local address reuse is allowed.
 */
void Connection::
set_reuse_addr(bool flag) {
  _socket->SetReuseAddress(flag);
}

/**
 * Sets whether the connection is periodically tested to see if it is still
 * alive.
 */
void Connection::
set_keep_alive(bool flag) {
  // TODO.
}

/**
 * Sets the size of the receive buffer, in bytes.
 */
void Connection::
set_recv_buffer_size(int size) {
  _socket->SetRecvBufferSize(size);
}

/**
 * Sets the size of the send buffer, in bytes.
 */
void Connection::
set_send_buffer_size(int size) {
  Socket_TCP *tcp;
  DCAST_INTO_V(tcp, _socket);

  tcp->SetSendBufferSize(size);
}

/**
 * Sets IP time-to-live.
 */
void Connection::
set_ip_time_to_live(int ttl) {
  // TODO.
}

/**
 * Sets IP type-of-service and precedence.
 */
void Connection::
set_ip_type_of_service(int tos) {
  // TODO.
}

/**
 * If flag is true, this disables the Nagle algorithm, and prevents delaying
 * of send to coalesce packets.
 */
void Connection::
set_no_delay(bool flag) {
  Socket_TCP *tcp;
  DCAST_INTO_V(tcp, _socket);

  tcp->SetNoDelay(flag);
}

/**
 * Sets the maximum segment size.
 */
void Connection::
set_max_segment(int size) {
  // TODO.
}

/**
 * This method is intended only to be called by ConnectionWriter.  It
 * atomically writes the given datagram to the socket, returning true on
 * success, false on failure.  If the socket seems to be closed, it notifies
 * the ConnectionManager.
 */
bool Connection::
send_datagram(const NetDatagram &datagram, int tcp_header_size) {
  nassertr(_socket != nullptr, false);

  if (_socket->is_exact_type(Socket_UDP::get_class_type())) {
    // We have to send UDP right away.
    Socket_UDP *udp;
    DCAST_INTO_R(udp, _socket, false);

    LightReMutexHolder holder(_write_mutex);
    DatagramUDPHeader header(datagram);
    std::string data;
    data += header.get_header();
    data += datagram.get_message();

    if (net_cat.is_debug()) {
      header.verify_datagram(datagram);
    }

    int bytes_to_send = data.length();
    Socket_Address addr = datagram.get_address().get_addr();

    bool okflag = udp->SendTo(data, addr);
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
    while (!okflag && udp->GetLastError() == LOCAL_BLOCKING_ERROR && udp->Active()) {
      Thread::force_yield();
      okflag = udp->SendTo(data, addr);
    }
#endif  // SIMPLE_THREADS

    if (net_cat.is_spam()) {
      net_cat.spam()
        << "Sent UDP datagram with "
        << bytes_to_send << " bytes to " << (void *)this
        << ", ok = " << okflag << "\n";
    }

    return check_send_error(okflag);
  }

  // We might queue up TCP packets for later sending.
  if (tcp_header_size == 2 && datagram.get_length() >= 0x10000) {
    net_cat.error()
      << "Attempt to send TCP datagram of " << datagram.get_length()
      << " bytes--too long!\n";
    nassert_raise("Datagram too long");
    return false;
  }

  DatagramTCPHeader header(datagram, tcp_header_size);

  LightReMutexHolder holder(_write_mutex);
  _queued_data += header.get_header();
  _queued_data += datagram.get_message();
  _queued_count++;

  if (net_cat.is_debug()) {
    header.verify_datagram(datagram, tcp_header_size);
  }

  if (!_collect_tcp ||
      TrueClock::get_global_ptr()->get_short_time() - _queued_data_start >= _collect_tcp_interval) {
    return do_flush();
  }

  return true;
}

/**
 * This method is intended only to be called by ConnectionWriter.  It
 * atomically writes the given datagram to the socket, without the Datagram
 * header.
 */
bool Connection::
send_raw_datagram(const NetDatagram &datagram) {
  nassertr(_socket != nullptr, false);

  if (_socket->is_exact_type(Socket_UDP::get_class_type())) {
    // We have to send UDP right away.
    Socket_UDP *udp;
    DCAST_INTO_R(udp, _socket, false);

    std::string data = datagram.get_message();

    LightReMutexHolder holder(_write_mutex);
    Socket_Address addr = datagram.get_address().get_addr();
    bool okflag = udp->SendTo(data, addr);
#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
    while (!okflag && udp->GetLastError() == LOCAL_BLOCKING_ERROR && udp->Active()) {
      Thread::force_yield();
      okflag = udp->SendTo(data, addr);
    }
#endif  // SIMPLE_THREADS

    if (net_cat.is_spam()) {
      net_cat.spam()
        << "Sent UDP datagram with "
        << data.size() << " bytes to " << (void *)this
        << ", ok = " << okflag << "\n";
    }

    return check_send_error(okflag);
  }

  // We might queue up TCP packets for later sending.
  LightReMutexHolder holder(_write_mutex);
  _queued_data += datagram.get_message();
  _queued_count++;

  if (!_collect_tcp ||
      TrueClock::get_global_ptr()->get_short_time() - _queued_data_start >= _collect_tcp_interval) {
    return do_flush();
  }

  return true;
}

/**
 * The private implementation of flush(), this assumes the _write_mutex is
 * already held.
 */
bool Connection::
do_flush() {
  if (_queued_data.empty()) {
    _queued_count = 0;
    _queued_data_start = TrueClock::get_global_ptr()->get_short_time();
    return true;
  }

  if (net_cat.is_spam()) {
    net_cat.spam()
      << "Sending " << _queued_count << " TCP datagram(s) with "
      << _queued_data.length() << " total bytes to " << (void *)this << "\n";
  }

  Socket_TCP *tcp;
  DCAST_INTO_R(tcp, _socket, false);

  std::string sending_data;
  _queued_data.swap(sending_data);

  _queued_count = 0;
  _queued_data_start = TrueClock::get_global_ptr()->get_short_time();

#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)
  int max_send = net_max_write_per_epoch;
  int data_sent = tcp->SendData(sending_data.data(), std::min((size_t)max_send, sending_data.size()));
  bool okflag = (data_sent == (int)sending_data.size());
  if (!okflag) {
    int total_sent = 0;
    if (data_sent > 0) {
      total_sent += data_sent;
    }

    while (!okflag && tcp->Active() &&
           (data_sent > 0 || tcp->GetLastError() == LOCAL_BLOCKING_ERROR)) {
      if (data_sent == 0) {
        Thread::force_yield();
      } else {
        Thread::consider_yield();
      }
      data_sent = tcp->SendData(sending_data.data() + total_sent, std::min((size_t)max_send, sending_data.size() - total_sent));
      if (data_sent > 0) {
        total_sent += data_sent;
      }
      okflag = (total_sent == (int)sending_data.size());
    }
  }

#else  // SIMPLE_THREADS
  int data_sent = tcp->SendData(sending_data);
  bool okflag = (data_sent == (int)sending_data.size());

#endif  // SIMPLE_THREADS

  return check_send_error(okflag);
}

/**
 * Checks the return value of a Send() or SendTo() call.
 */
bool Connection::
check_send_error(bool okflag) {
  if (!okflag) {
    static ConfigVariableBool abort_send_error("abort-send-error", false);
    if (abort_send_error) {
      nassert_raise("send error");
      return false;
    }

    // Assume any error means the connection has been reset; tell our manager
    // about it and ignore it.
    if (_manager != nullptr) {
      _manager->flush_read_connection(this);
      _manager->connection_reset(this, okflag);
    }
    return false;
  }

  return true;
}
