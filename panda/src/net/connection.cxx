// Filename: connection.cxx
// Created by:  jns (07Feb00)
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
  } else {
    DatagramTCPHeader header(datagram);
    string data = header.get_header() + datagram.get_message();
    bytes_sent = data.length();
    result = PR_Send(_socket,
		     data.data(), bytes_sent,
		     0,
		     PR_INTERVAL_NO_TIMEOUT);
  }

  PRErrorCode errcode = PR_GetError();

  PR_Unlock(_write_mutex);

  if (result < 0) {
    if (errcode == PR_CONNECT_RESET_ERROR) {
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
