// Filename: connectionWriter.cxx
// Created by:  drose (08Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "connectionWriter.h"
#include "connectionManager.h"
#include "pprerror.h"
#include "config_net.h"

#include <notify.h>
#include <prerror.h>

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::Constructor
//       Access: Public
//  Description: Creates a new ConnectionWriter with the indicated
//               number of threads to handle output.
//
//               If num_threads is 0, all datagrams will be sent
//               immediately instead of queueing for later
//               transmission by a thread.
////////////////////////////////////////////////////////////////////
ConnectionWriter::
ConnectionWriter(ConnectionManager *manager, int num_threads) :
  _manager(manager)
{
  _immediate = (num_threads <= 0);

  for (int i = 0; i < num_threads; i++) {
    PRThread *thread = 
      PR_CreateThread(PR_USER_THREAD,
		      thread_start, (void *)this,
		      PR_PRIORITY_NORMAL,
		      PR_GLOBAL_THREAD, // Since thread will mostly do I/O.
		      PR_JOINABLE_THREAD, 
		      0);  // Select a suitable stack size.

    nassertv(thread != (PRThread *)NULL);
    _threads.push_back(thread);
  }

  _manager->add_writer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ConnectionWriter::
~ConnectionWriter() {
  if (_manager != (ConnectionManager *)NULL) {
    _manager->remove_writer(this);
  }

  // First, shutdown the queue.  This will tell our threads they're
  // done.
  _queue.shutdown();

  // Now wait for all threads to terminate.
  Threads::iterator ti;
  for (ti = _threads.begin(); ti != _threads.end(); ++ti) {
    // Interrupt the thread just in case it was stuck waiting for I/O.
    PRStatus result = PR_Interrupt(*ti);
    if (result != PR_SUCCESS) {
      pprerror("PR_Interrupt");
    }

    result = PR_JoinThread(*ti);
    if (result != PR_SUCCESS) {
      pprerror("PR_JoinThread");
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::send
//       Access: Public
//  Description: Enqueues a datagram for transmittal on the indicated
//               socket.  Since the host address is not specified with
//               this form, this function should only be used for
//               sending TCP packets.  Use the other send() method for
//               sending UDP packets.
//
//               Returns true if successful, false if there was an
//               error.  In the normal, threaded case, this function
//               only returns false if the send queue is filled; it's
//               impossible to detect a transmission error at this
//               point.
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
send(const Datagram &datagram, const PT(Connection) &connection) {
  nassertr(connection != (Connection *)NULL, false);
  nassertr(PR_GetDescType(connection->get_socket()) == PR_DESC_SOCKET_TCP, false);

  if (net_cat.is_debug()) {
    net_cat.debug()
      << "Sending TCP datagram of " << datagram.get_length() 
      << " bytes\n";
  }

  NetDatagram copy(datagram);
  copy.set_connection(connection);
    
  if (_immediate) {
    return connection->send_datagram(copy);
  } else {
    return _queue.insert(copy);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::send
//       Access: Public
//  Description: Enqueues a datagram for transmittal on the indicated
//               socket.  This form of the function allows the
//               specification of a destination host address, and so
//               is appropriate for UDP packets.  Use the other send()
//               method for sending TCP packets.
//
//               Returns true if successful, false if there was an
//               error.  In the normal, threaded case, this function
//               only returns false if the send queue is filled; it's
//               impossible to detect a transmission error at this
//               point.
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
send(const Datagram &datagram, const PT(Connection) &connection,
     const NetAddress &address) {
  nassertr(connection != (Connection *)NULL, false);
  nassertr(PR_GetDescType(connection->get_socket()) == PR_DESC_SOCKET_UDP, false);

  if (net_cat.is_debug()) {
    net_cat.debug()
      << "Sending UDP datagram of " << datagram.get_length() 
      << " bytes\n";
  }

  if (PR_GetDescType(connection->get_socket()) == PR_DESC_SOCKET_UDP &&
      (int)datagram.get_length() > maximum_udp_datagram) {
    net_cat.warning()
      << "Attempt to send UDP datagram of " << datagram.get_length()
      << " bytes, more than the\n"
      << "currently defined maximum of " << maximum_udp_datagram
      << " bytes.\n";
  }

  NetDatagram copy(datagram);
  copy.set_connection(connection);
  copy.set_address(address);

  if (_immediate) {
    return connection->send_datagram(copy);
  } else {
    return _queue.insert(copy);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::is_valid_for_udp
//       Access: Public
//  Description: Returns true if the datagram is small enough to be
//               sent over a UDP packet, false otherwise.
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
is_valid_for_udp(const Datagram &datagram) const {
  return (int)datagram.get_length() <= maximum_udp_datagram;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::get_manager
//       Access: Public
//  Description: Returns a pointer to the ConnectionManager object
//               that serves this ConnectionWriter.
////////////////////////////////////////////////////////////////////
ConnectionManager *ConnectionWriter::
get_manager() const {
  return _manager;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::is_immediate
//       Access: Public
//  Description: Returns true if the writer is an immediate writer,
//               i.e. it has no threads.
////////////////////////////////////////////////////////////////////
bool ConnectionWriter::
is_immediate() const {
  return _immediate;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::get_num_threads
//       Access: Public
//  Description: Returns the number of threads the ConnectionWriter
//               has been created with.
////////////////////////////////////////////////////////////////////
int ConnectionWriter::
get_num_threads() const {
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::clear_manager
//       Access: Protected
//  Description: This should normally only be called when the
//               associated ConnectionManager destructs.  It resets
//               the ConnectionManager pointer to NULL so we don't
//               have a floating pointer.  This makes the
//               ConnectionWriter invalid; presumably it also will be
//               destructed momentarily.
////////////////////////////////////////////////////////////////////
void ConnectionWriter::
clear_manager() {
  _manager = (ConnectionManager *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::thread_start
//       Access: Private, Static
//  Description: The static wrapper around the thread's executing
//               function.  This must be a static member function
//               because it is passed through the C interface to
//               PR_CreateThread().
////////////////////////////////////////////////////////////////////
void ConnectionWriter::
thread_start(void *data) {
  ((ConnectionWriter *)data)->thread_run();
}

////////////////////////////////////////////////////////////////////
//     Function: ConnectionWriter::thread_run
//       Access: Private
//  Description: This is the actual executing function for each
//               thread.
////////////////////////////////////////////////////////////////////
void ConnectionWriter::
thread_run() {
  nassertv(!_immediate);

  NetDatagram datagram;
  while (_queue.extract(datagram)) {
    datagram.get_connection()->send_datagram(datagram);
  }
}
