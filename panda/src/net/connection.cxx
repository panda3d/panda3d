// Filename: connection.cxx
// Created by:  jns (07Feb00)
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

#include "connection.h"
#include "connectionManager.h"
#include "netDatagram.h"
#include "datagramTCPHeader.h"
#include "datagramUDPHeader.h"
#include "pprerror.h"
#include "config_net.h"

#include <prerror.h>

////////////////////////////////////////////////////////////////////
//     Function: Connection::Constructor
//       Access: Public
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
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::Destructor
//       Access: Public
//  Description: Closes a connection.
////////////////////////////////////////////////////////////////////
Connection::
~Connection() {
  net_cat.info()
    << "Deleting connection " << (void *)this << "\n";

  if (_socket != (PRFileDesc *)NULL) {
    PRStatus result = PR_Close(_socket);
    if (result != PR_SUCCESS) {
      pprerror("PR_Close");
    }
  }

  PR_DestroyLock(_write_mutex);
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::get_address
//       Access: Public
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
//       Access: Public
//  Description: Returns a pointer to the ConnectionManager object
//               that serves this connection.
////////////////////////////////////////////////////////////////////
ConnectionManager *Connection::
get_manager() const {
  return _manager;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::get_socket
//       Access: Public
//  Description: Returns the internal NSPR pointer that defines the
//               connection.
////////////////////////////////////////////////////////////////////
PRFileDesc *Connection::
get_socket() const {
  return _socket;
}

////////////////////////////////////////////////////////////////////
//     Function: Connection::set_nonblock
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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

  PR_Lock(_write_mutex);

  PRInt32 bytes_sent;
  PRInt32 result;
  if (PR_GetDescType(_socket) == PR_DESC_SOCKET_UDP) {
    DatagramUDPHeader header(datagram);
    string data = header.get_header() + datagram.get_message();
    bytes_sent = data.length();
    result = PR_SendTo(_socket,
                       data.data(), bytes_sent,
                       0,
                       datagram.get_address().get_addr(),
                       PR_INTERVAL_NO_TIMEOUT);

    if (net_cat.is_debug()) {
      header.verify_datagram(datagram);
    }
  } else {
    DatagramTCPHeader header(datagram);
    string data = header.get_header() + datagram.get_message();
    bytes_sent = data.length();
    result = PR_Send(_socket,
                     data.data(), bytes_sent,
                     0,
                     PR_INTERVAL_NO_TIMEOUT);

    if (net_cat.is_debug()) {
      header.verify_datagram(datagram);
    }
  }

  PRErrorCode errcode = PR_GetError();

  PR_Unlock(_write_mutex);

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
        _manager->connection_reset(this);
      }

    } else if (errcode != PR_PENDING_INTERRUPT_ERROR) {
      pprerror("PR_SendTo");
    }

    return false;

  } else if (result != bytes_sent) {
    net_cat.error() << "Not enough bytes sent to socket.\n";
    return false;
  }

  return true;
}
