// Filename: socket_udp.h
// Created by:  drose (01Mar07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef __SOCKET_UDP_H__
#define __SOCKET_UDP_H__

#include "socket_udp_incoming.h"

/////////////////////////////////////////////////////////////////////
// Class : Socket_UDP
//
// Description : Base functionality for a combination UDP Reader and
//               Writer.  This duplicates code from
//               Socket_UDP_Outgoing, to avoid the problems of
//               multiple inheritance.
/////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_NATIVENET Socket_UDP : public Socket_UDP_Incoming
{
public:
PUBLISHED:
    inline Socket_UDP() { }

    // use this interface for a tagreted UDP connection
    inline bool InitToAddress(const Socket_Address & address);
public:
    inline bool Send(const char * data, int len);
PUBLISHED:
    inline bool Send(const string &data);
    // use this interface for a none tagreted UDP connection
    inline bool InitNoAddress();
public:
    inline bool SendTo(const char * data, int len, const Socket_Address & address);
PUBLISHED:
    inline bool SendTo(const string &data, const Socket_Address & address);
    inline bool SetToBroadCast();
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_UDP_Incoming::init_type();
    register_type(_type_handle, "Socket_UDP",
                  Socket_UDP_Incoming::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};
//////////////////////////////////////////////////////////////
// Function name : Socket_UDP:SetToBroadCast
// Description     : Ask the OS to let us receive BROADCASt packets on  this port..
// Return type  : bool
// Argument         : void
//////////////////////////////////////////////////////////////
inline bool Socket_UDP::SetToBroadCast()
{
    int optval = 1;
    
    if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
        return false;
    return true;
}
////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP::InitToAddress
// Description     : Connects the Socket to a Specified address
//
// Return type  : inline bool
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP::InitToAddress(const Socket_Address & address)
{
    if (InitNoAddress() != true)
        return false;
    
    if (DO_CONNECT(_socket, &address.GetAddressInfo()) != 0)
        return ErrorClose();
    
    return true;
}
////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP::InitNoAddress
// Description     : This will set a udp up for targeted sends..
//
// Return type  : inline bool
// Argument         : void
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP::InitNoAddress()
{
    Close();
    _socket = DO_NEWUDP();
    if (_socket == BAD_SOCKET)
        return false;
    
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP::Send
// Description     : Send data to connected address
//
// Return type  : inline bool
// Argument         : char * data
// Argument         : int len
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP::Send(const char * data, int len)
{
  return (DO_SOCKET_WRITE(_socket, data, len) == len);
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP::Send
// Description     : Send data to connected address
//
// Return type  : inline bool
// Argument         : const string &data
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP::Send(const string &data)
{
  return Send(data.data(), data.size());
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP::SendTo
// Description     : Send data to specified address
//
// Return type  : inline bool
// Argument         : char * data
// Argument         : int len
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP::SendTo(const char * data, int len, const Socket_Address & address)
{
    return (DO_SOCKET_WRITE_TO(_socket, data, len, &address.GetAddressInfo()) == len);
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP::SendTo
// Description     : Send data to specified address
//
// Return type  : inline bool
// Argument         : const string &data
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP::SendTo(const string &data, const Socket_Address & address)
{
  return SendTo(data.data(), data.size(), address);
}

#endif //__SOCKET_UDP_H__
