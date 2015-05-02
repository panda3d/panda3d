#ifndef __SOCKET_TCP_LISTEN_H__
#define __SOCKET_TCP_LISTEN_H__

#include "pandabase.h"
#include "socket_ip.h"
#include "socket_tcp.h"

/////////////////////////////////////////////////////////////////////
// Class : Socket_TCP_Listen
// Description : Base functionality for a TCP rendezvous socket
/////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_NATIVENET Socket_TCP_Listen : public Socket_IP
{
public:
PUBLISHED:
    Socket_TCP_Listen() {};
    ~Socket_TCP_Listen() {};
    inline bool OpenForListen(const Socket_Address & Inaddess, int backlog_size = 1024);
    inline bool GetIncomingConnection(Socket_TCP & newsession, Socket_Address &address);    
public:
    inline bool GetIncomingConnection(SOCKET & newsession, Socket_Address &address);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_IP::init_type();
    register_type(_type_handle, "Socket_TCP_Listen",
                  Socket_IP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
// Function name : OpenForListen
// Description   : This function will initialize a listening Socket
////////////////////////////////////////////////////////////////////
inline bool Socket_TCP_Listen::OpenForListen(const Socket_Address & Inaddess, int backlog_size )
{
    ErrorClose();
    _socket = DO_NEWTCP();
    
    SetReuseAddress();
    
    if (DO_BIND(_socket, &Inaddess.GetAddressInfo()) != 0) {
      return ErrorClose();
    }
    
    if (DO_LISTEN(_socket, backlog_size) != 0) {
      return ErrorClose();
    }

    return true;
}
////////////////////////////////////////////////////////////////////
// Function name : GetIncomingConnection
// Description   : This function is used to accept new connections
////////////////////////////////////////////////////////////////////
inline bool Socket_TCP_Listen::GetIncomingConnection(SOCKET & newsession, Socket_Address &address)
{
    newsession = DO_ACCEPT(_socket, &address.GetAddressInfo());
    if (newsession == BAD_SOCKET)
        return false;
    return true;
}


inline bool Socket_TCP_Listen::GetIncomingConnection(Socket_TCP & newsession, Socket_Address &address)
{
    SOCKET sck;
    if(!GetIncomingConnection(sck,address))
        return false;

    newsession.SetSocket(sck);
    return true;
}

#endif //__SOCKET_TCP_LISTEN_H__
