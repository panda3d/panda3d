// Filename: connection.cxx
// Created by:  jns (07Feb00)
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

#include "connection.h"
#include "connectionManager.h"
#include "netDatagram.h"
#include "datagramTCPHeader.h"
#include "datagramUDPHeader.h"
#include "pprerror.h"
#include "config_net.h"
#include "config_express.h" // for collect_tcp
#include "clockObject.h"


////////////////////////////////////////////////////////////////////
//     Function: Connection::Constructor
//       Access: Published
//  Description: Creates a connection.  Normally this constructor
//               should not be used directly by user code; use one of
//               the methods in ConnectionManager to make a new
//               connection.
////////////////////////////////////////////////////////////////////
Connection::
Connection(ConnectionManager *manager, PRFileDesc *socket) :
  _manager(manager),
  _socket(socket)
{
  _write_mutex = PR_NewLock();
  _collect_tcp = collect_tcp;
  _collect_tcp_interval = collect_tcp_interval;
  _queued_data_start = 0.0;
  _queued_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::Destructor
//       Access: Published
//  Description: Closes a connection.
////////////////////////////////////////////////////////////////////
Connection::
~Connection() {
  net_cat.info()
    << "Deleting connection " << (void *)this << "\n";

  if (_socket != (PRFileDesc *)NULL) {
    flush();

    PRStatus result = PR_Close(_socket);
    if (result != PR_SUCCESS) {
      pprerror("PR_Close");
    }
  }

  PR_DestroyLock(_write_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::get_address
//       Access: Published
//  Description: Returns the address bound to this connection, if it
//               is a TCP connection.
////////////////////////////////////////////////////////////////////
NetAddress Connection::
get_address() const {
  PRNetAddr addr;
  if (PR_GetSockName(_socket, &addr) != PR_SUCCESS) {
    pprerror("PR_GetSockName");
  }

  return NetAddress(addr);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::get_manager
//       Access: Published
//  Description: Returns a pointer to the ConnectionManager object
//               that serves this connection.
////////////////////////////////////////////////////////////////////
ConnectionManager *Connection::
get_manager() const {
  return _manager;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::get_socket
//       Access: Published
//  Description: Returns the internal NSPR pointer that defines the
//               connection.
////////////////////////////////////////////////////////////////////
PRFileDesc *Connection::
get_socket() const {
  return _socket;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_collect_tcp
//       Access: Published
//  Description: Enables or disables "collect-tcp" mode.  In this
//               mode, individual TCP packets are not sent
//               immediately, but rather they are collected together
//               and accumulated to be sent periodically as one larger
//               TCP packet.  This cuts down on overhead from the
//               TCP/IP protocol, especially if many small packets
//               need to be sent on the same connection, but it
//               introduces additional latency (since packets must be
//               held before they can be sent).
//
//               See set_collect_tcp_interval() to specify the
//               interval of time for which to hold packets before
//               sending them.
//
//               If you enable this mode, you may also need to
//               periodically call consider_flush() to flush the queue
//               if no packets have been sent recently.
////////////////////////////////////////////////////////////////////
void Connection::
set_collect_tcp(bool collect_tcp) {
  _collect_tcp = collect_tcp;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::get_collect_tcp
//       Access: Published
//  Description: Returns the current setting of "collect-tcp" mode.
//               See set_collect_tcp().
////////////////////////////////////////////////////////////////////
bool Connection::
get_collect_tcp() const {
  return _collect_tcp;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_collect_tcp_interval
//       Access: Published
//  Description: Specifies the interval in time, in seconds, for which
//               to hold TCP packets before sending all of the
//               recently received packets at once.  This only has
//               meaning if "collect-tcp" mode is enabled; see
//               set_collect_tcp().
////////////////////////////////////////////////////////////////////
void Connection::
set_collect_tcp_interval(double interval) {
  _collect_tcp_interval = interval;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::get_collect_tcp_interval
//       Access: Published
//  Description: Returns the interval in time, in seconds, for which
//               to hold TCP packets before sending all of the
//               recently received packets at once.  This only has
//               meaning if "collect-tcp" mode is enabled; see
//               set_collect_tcp().
////////////////////////////////////////////////////////////////////
double Connection::
get_collect_tcp_interval() const {
  return _collect_tcp_interval;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::consider_flush
//       Access: Published
//  Description: Sends the most recently queued TCP datagram(s) if
//               enough time has elapsed.  This only has meaning if
//               set_collect_tcp() has been set to true.
////////////////////////////////////////////////////////////////////
bool Connection::
consider_flush() {
  PR_Lock(_write_mutex);

  if (!_collect_tcp) {
    return do_flush();

  } else {
    double elapsed = 
      ClockObject::get_global_clock()->get_real_time() - _queued_data_start;
    // If the elapsed time is negative, someone must have reset the
    // clock back, so just go ahead and flush.
    if (elapsed < 0.0 || elapsed >= _collect_tcp_interval) {
      return do_flush();
    }
  }

  PR_Unlock(_write_mutex);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::flush
//       Access: Published
//  Description: Sends the most recently queued TCP datagram(s) now.
//               This only has meaning if set_collect_tcp() has been
//               set to true.
////////////////////////////////////////////////////////////////////
bool Connection::
flush() {
  PR_Lock(_write_mutex);
  return do_flush();
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_nonblock
//       Access: Published
//  Description: Sets whether nonblocking I/O should be in effect.
////////////////////////////////////////////////////////////////////
void Connection::
set_nonblock(bool flag) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_Nonblocking;
  data.value.non_blocking = flag;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_linger
//       Access: Published
//  Description: Sets the time to linger on close if data is present.
//               If flag is false, when you close a socket with data
//               available the system attempts to deliver the data to
//               the peer (the default behavior).  If flag is false
//               but time is zero, the system discards any undelivered
//               data when you close the socket.  If flag is false but
//               time is nonzero, the system waits up to time seconds
//               to deliver the data.
////////////////////////////////////////////////////////////////////
void Connection::
set_linger(bool flag, double time) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_Linger;
  data.value.linger.polarity = flag;
  data.value.linger.linger = PRIntervalTime(time * PR_INTERVAL_MIN);
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_reuse_addr
//       Access: Published
//  Description: Sets whether local address reuse is allowed.
////////////////////////////////////////////////////////////////////
void Connection::
set_reuse_addr(bool flag) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_Reuseaddr;
  data.value.reuse_addr = flag;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_keep_alive
//       Access: Published
//  Description: Sets whether the connection is periodically tested to
//               see if it is still alive.
////////////////////////////////////////////////////////////////////
void Connection::
set_keep_alive(bool flag) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_Keepalive;
  data.value.keep_alive = flag;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_recv_buffer_size
//       Access: Published
//  Description: Sets the size of the receive buffer, in bytes.
////////////////////////////////////////////////////////////////////
void Connection::
set_recv_buffer_size(int size) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_RecvBufferSize;
  data.value.recv_buffer_size = size;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_send_buffer_size
//       Access: Published
//  Description: Sets the size of the send buffer, in bytes.
////////////////////////////////////////////////////////////////////
void Connection::
set_send_buffer_size(int size) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_SendBufferSize;
  data.value.send_buffer_size = size;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_ip_time_to_live
//       Access: Published
//  Description: Sets IP time-to-live.
////////////////////////////////////////////////////////////////////
void Connection::
set_ip_time_to_live(int ttl) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_IpTimeToLive;
  data.value.ip_ttl = ttl;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_ip_type_of_service
//       Access: Published
//  Description: Sets IP type-of-service and precedence.
////////////////////////////////////////////////////////////////////
void Connection::
set_ip_type_of_service(int tos) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_IpTypeOfService;
  data.value.tos = tos;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_no_delay
//       Access: Published
//  Description: If flag is true, this disables the Nagle algorithm,
//               and prevents delaying of send to coalesce packets.
////////////////////////////////////////////////////////////////////
void Connection::
set_no_delay(bool flag) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_NoDelay;
  data.value.no_delay = flag;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_max_segment
//       Access: Published
//  Description: Sets the maximum segment size.
////////////////////////////////////////////////////////////////////
void Connection::
set_max_segment(int size) {
  PRSocketOptionData data;
  data.option = PR_SockOpt_MaxSegment;
  data.value.max_segment = size;
  PR_SetSocketOption(_socket, &data);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::send_datagram
//       Access: Private
//  Description: This method is intended only to be called by
//               ConnectionWriter.  It atomically writes the given
//               datagram to the socket, returning true on success,
//               false on failure.  If the socket seems to be closed,
//               it notifies the ConnectionManager.
////////////////////////////////////////////////////////////////////
bool Connection::
send_datagram(const NetDatagram &datagram) {
  nassertr(_socket != (PRFileDesc *)NULL, false);

  if (PR_GetDescType(_socket) == PR_DESC_SOCKET_UDP) {
    // We have to send UDP right away.
    PR_Lock(_write_mutex);
    DatagramUDPHeader header(datagram);
    string data;
    data += header.get_header();
    data += datagram.get_message();

    PRInt32 bytes_to_send = data.length();
    PRInt32 result;
    result = PR_SendTo(_socket,
                       data.data(), bytes_to_send,
                       0,
                       datagram.get_address().get_addr(),
                       PR_INTERVAL_NO_TIMEOUT);
    PRErrorCode errcode = PR_GetError();

    if (net_cat.is_debug()) {
      header.verify_datagram(datagram);
    }

    PR_Unlock(_write_mutex);
    return check_send_error(result, errcode, bytes_to_send);
  }

  // We might queue up TCP packets for later sending.
  DatagramTCPHeader header(datagram);

  PR_Lock(_write_mutex);
  _queued_data += header.get_header();
  _queued_data += datagram.get_message();
  _queued_count++;
  
  if (net_cat.is_debug()) {
    header.verify_datagram(datagram);
  }

  if (!_collect_tcp || 
      ClockObject::get_global_clock()->get_real_time() - _queued_data_start >= _collect_tcp_interval) {
    return do_flush();
  }

  PR_Unlock(_write_mutex);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::send_raw_datagram
//       Access: Private
//  Description: This method is intended only to be called by
//               ConnectionWriter.  It atomically writes the given
//               datagram to the socket, without the Datagram header.
////////////////////////////////////////////////////////////////////
bool Connection::
send_raw_datagram(const NetDatagram &datagram) {
  nassertr(_socket != (PRFileDesc *)NULL, false);

  if (PR_GetDescType(_socket) == PR_DESC_SOCKET_UDP) {
    // We have to send UDP right away.

    string data = datagram.get_message();
    PRInt32 bytes_to_send = data.length();

    if (net_cat.is_spam()) {
      net_cat.spam()
        << "Sending UDP datagram with " 
        << bytes_to_send << " bytes to " << (void *)this << "\n";
    }

    PR_Lock(_write_mutex);
    PRInt32 result;
    result = PR_SendTo(_socket,
                       data.data(), bytes_to_send,
                       0,
                       datagram.get_address().get_addr(),
                       PR_INTERVAL_NO_TIMEOUT);
    PRErrorCode errcode = PR_GetError();

    PR_Unlock(_write_mutex);
    return check_send_error(result, errcode, bytes_to_send);
  }

  // We might queue up TCP packets for later sending.

  PR_Lock(_write_mutex);
  _queued_data += datagram.get_message();
  _queued_count++;

  if (!_collect_tcp || 
      ClockObject::get_global_clock()->get_real_time() - _queued_data_start >= _collect_tcp_interval) {
    return do_flush();
  }

  PR_Unlock(_write_mutex);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::do_flush
//       Access: Private
//  Description: The private implementation of flush(), this assumes
//               the _write_mutex has already been locked on entry.
//               It will be unlocked on return.
////////////////////////////////////////////////////////////////////
bool Connection::
do_flush() {
  PRInt32 bytes_to_send = _queued_data.length();
  if (bytes_to_send == 0) {
    _queued_count = 0;
    _queued_data_start = ClockObject::get_global_clock()->get_real_time();
    PR_Unlock(_write_mutex);
    return true;
  }

  if (net_cat.is_spam()) {
    net_cat.spam()
      << "Sending " << _queued_count << " TCP datagram(s) with " 
      << bytes_to_send << " total bytes to " << (void *)this << "\n";
  }

  PRInt32 result;
  result = PR_Send(_socket,
                   _queued_data.data(), bytes_to_send,
                   0,
                   PR_INTERVAL_NO_TIMEOUT);
  PRErrorCode errcode = PR_GetError();

  _queued_data = string();
  _queued_count = 0;
  _queued_data_start = ClockObject::get_global_clock()->get_real_time();

  PR_Unlock(_write_mutex);

  return check_send_error(result, errcode, bytes_to_send);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::check_send_error
//       Access: Private
//  Description: Checks the return value of a PR_Send() or PR_SendTo()
//               call.
////////////////////////////////////////////////////////////////////
bool Connection::
check_send_error(PRInt32 result, PRErrorCode errcode, PRInt32 bytes_to_send) {
  if (result < 0) {
    if (errcode == PR_CONNECT_RESET_ERROR
#ifdef PR_SOCKET_SHUTDOWN_ERROR
        || errcode == PR_SOCKET_SHUTDOWN_ERROR
        || errcode == PR_CONNECT_ABORTED_ERROR
#endif
        ) {
      // The connection has been reset; tell our manager about it
      // and ignore it.
      if (_manager != (ConnectionManager *)NULL) {
        _manager->connection_reset(this, errcode);
      }

    } else if (errcode != PR_PENDING_INTERRUPT_ERROR) {
      pprerror("PR_SendTo");
    }

    return false;

  } else if (result != bytes_to_send) {
    net_cat.error() << "Not enough bytes sent to socket.\n";
    return false;
  }

  return true;
}
