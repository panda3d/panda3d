// Filename: connectionWriter.h
// Created by:  drose (08Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef CONNECTIONWRITER_H
#define CONNECTIONWRITER_H

#include <pandabase.h>

#include "datagramQueue.h"
#include "connection.h"

#include <pointerTo.h>

#include <prthread.h>
#include <vector>

class ConnectionManager;
class NetAddress;

////////////////////////////////////////////////////////////////////
// 	 Class : ConnectionWriter
// Description : This class handles threaded delivery of datagrams to
//               various TCP or UDP sockets.
//
//               A ConnectionWriter may define an arbitrary number of
//               threads (at least one) to write its datagrams to
//               sockets.  The number of threads is specified at
//               construction time and cannot be changed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ConnectionWriter {
public:
  ConnectionWriter(ConnectionManager *manager, int num_threads);
  ~ConnectionWriter();

  bool send(const Datagram &datagram,
	    const PT(Connection) &connection);

  bool send(const Datagram &datagram,
	    const PT(Connection) &connection,
	    const NetAddress &address);

  ConnectionManager *get_manager() const;
  bool is_immediate() const;
  int get_num_threads() const;

protected:
  void clear_manager();
 
private:
  static void thread_start(void *data);
  void thread_run();
  bool send_datagram(const NetDatagram &datagram);

protected:
  ConnectionManager *_manager;

private:
  DatagramQueue _queue;

  typedef vector<PRThread *> Threads;
  Threads _threads;
  bool _immediate;

friend class ConnectionManager;
};

#endif


