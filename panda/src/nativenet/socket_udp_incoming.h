#ifndef __SOCKET_UDP_INCOMING_H__
#define __SOCKET_UDP_INCOMING_H__

#include "pandabase.h"
#include "socket_ip.h"

/////////////////////////////////////////////////////////////////////
// Class : Socket_UDP_Incoming
//
// Description : Base functionality for a UDP Reader
//
//
/////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_NATIVENET Socket_UDP_Incoming : public Socket_IP
{
PUBLISHED:
    inline Socket_UDP_Incoming() { }

    inline bool OpenForInput(const Socket_Address & address);
    inline bool OpenForInputMCast(const Socket_Address & address );
    inline bool GetPacket(char * data, int *max_len, Socket_Address & address);
    inline bool SendTo(const char * data, int len, const Socket_Address & address);
    inline bool InitNoAddress();
    inline bool SetToBroadCast();
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_IP::init_type();
    register_type(_type_handle, "Socket_UDP_Incoming",
                  Socket_IP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

//////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Incoming::tToBroadCast
// Description     : Flips the OS bits that allow for brodcast
//      packets to com in on this port
//
// Return type  : bool
// Argument         : void
//////////////////////////////////////////////////////////////
inline bool Socket_UDP_Incoming::SetToBroadCast()
{
    int optval = 1;
    
    if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
        return false;
    return true;
}
//////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Incoming::InitNoAddress
// Description     : Set this socket to work with out a bound external address..
// Return type  : inline bool
// Argument         : void
//////////////////////////////////////////////////////////////
inline bool Socket_UDP_Incoming::InitNoAddress()
{
    Close();
    _socket = DO_NEWUDP();
    if (_socket == BAD_SOCKET)
        return false;
    
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Incoming::OpenForInput
// Description     : Starts a UDP socket listening on a port
//
// Return type  : bool
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP_Incoming::OpenForInput(const Socket_Address & address)
{
    Close();
    _socket = DO_NEWUDP();
    if (_socket == BAD_SOCKET)
        return ErrorClose();
    
    if (DO_BIND(_socket, &address.GetAddressInfo()) != 0)
        return ErrorClose();
    
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Incoming::OpenForInput
// Description     : Starts a UDP socket listening on a port
//
// Return type  : bool
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP_Incoming::OpenForInputMCast(const Socket_Address & address)
{
    Close();
    _socket = DO_NEWUDP();
    if (_socket == BAD_SOCKET)
        return ErrorClose();

    Socket_Address  wa1(address.get_port());        
    if (DO_BIND(_socket, &wa1.GetAddressInfo()) != 0)
        return ErrorClose();
    
     struct ip_mreq imreq;
     memset(&imreq,0,sizeof(imreq));
     imreq.imr_multiaddr.s_addr = address.GetAddressInfo().sin_addr.s_addr;
     imreq.imr_interface.s_addr = INADDR_ANY; // use DEFAULT interface

    int status = setsockopt(GetSocket(), IPPROTO_IP, IP_ADD_MEMBERSHIP, 
              (const char *)&imreq, sizeof(struct ip_mreq));

    if(status != 0)
        return false;
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Incoming::GetPacket
// Description     :  Grabs a dataset off the listening UDP socket
//      and fills in the source address information
//
// Return type  : bool
// Argument         : char * data
// Argument         : int *max_len
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP_Incoming::GetPacket(char * data, int *max_len, Socket_Address & address)
{
    int val = DO_RECV_FROM(_socket, data, *max_len, &address.GetAddressInfo());
    *max_len = 0;
    
    if (val <= 0) 
    {
        if (GetLastError() != LOCAL_BLOCKING_ERROR) // im treating a blocking error as a 0 lenght read
            return false;
    } else
        *max_len = val;
    
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : SocketUDP_Outgoing::SendTo
// Description     : Send data to specified address
//
// Return type  : inline bool
// Argument         : char * data
// Argument         : int len
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP_Incoming::SendTo(const char * data, int len, const Socket_Address & address)
{
    return (DO_SOCKET_WRITE_TO(_socket, data, len, &address.GetAddressInfo()) == len);
}




#endif //__SOCKET_UDP_INCOMING_H__
