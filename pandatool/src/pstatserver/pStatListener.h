// Filename: pStatListener.h
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATLISTENER_H
#define PSTATLISTENER_H

#include <pandatoolbase.h>

#include <connectionListener.h>
#include <referenceCount.h>

class PStatServer;
class PStatMonitor;

////////////////////////////////////////////////////////////////////
//       Class : PStatListener
// Description : This is the TCP rendezvous socket listener.  We need
//               one of these to listen for new connections on the
//               socket(s) added to the PStatServer.
////////////////////////////////////////////////////////////////////
class PStatListener : public ConnectionListener {
public:
  PStatListener(PStatServer *manager);

protected:
  virtual void connection_opened(const PT(Connection) &rendezvous,
                                 const NetAddress &address,
                                 const PT(Connection) &new_connection);

private:
  PStatServer *_manager;
};

#endif
