// Filename: pStatReader.h
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATREADER_H
#define PSTATREADER_H

#include <pandatoolbase.h>

#include "pStatClientData.h"
#include "pStatMonitor.h"

#include <connectionReader.h>
#include <connectionWriter.h>
#include <referenceCount.h>

class PStatServer;
class PStatMonitor;
class PStatClientControlMessage;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatReader
// Description : This is the class that does all the work for handling
//               communications from a single Panda client.  It reads
//               sockets received from the client and boils them down
//               into PStatData.
////////////////////////////////////////////////////////////////////
class PStatReader : public ConnectionReader {
public:
  PStatReader(PStatServer *manager, PStatMonitor *monitor);
  ~PStatReader();

  void close();
  
  void set_tcp_connection(Connection *tcp_connection);
  void lost_connection();
  void idle();

private:
  string get_hostname();
  void send_hello();

  virtual void receive_datagram(const NetDatagram &datagram);

  void handle_client_control_message(const PStatClientControlMessage &message);
  void handle_client_udp_data(const Datagram &datagram);

private:
  PStatServer *_manager;
  PT(PStatMonitor) _monitor;
  ConnectionWriter _writer;

  PT(Connection) _tcp_connection;
  PT(Connection) _udp_connection;
  int _udp_port;

  PT(PStatClientData) _client_data;

  string _hostname;
};

#endif
