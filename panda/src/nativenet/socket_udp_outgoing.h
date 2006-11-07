#ifndef __SOCKET_UDP_OUTGOING_H__
#define __SOCKET_UDP_OUTGOING_H__

/////////////////////////////////////////////////////////////////////
// Class : Socket_UDP_Outgoing
//
// Description : Base functionality for a UDP Sending Socket
//
//
/////////////////////////////////////////////////////////////////////
class Socket_UDP_Outgoing : public Socket_IP
{
public:
PUBLISHED:
    // use this interface for a tagreted UDP connection
    inline bool InitToAddress(Socket_Address & address);
    inline bool Send(const char * data, int len);
    // use this interface for a none tagreted UDP connection
    inline bool InitNoAddress();
    inline bool SendTo(const char * data, int len, const Socket_Address & address);
    inline bool SetToBroadCast();
};
//////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Outgoing:SetToBroadCast
// Description     : Ask the OS to let us receive BROADCASt packets on  this port..
// Return type  : bool
// Argument         : void
//////////////////////////////////////////////////////////////
inline bool Socket_UDP_Outgoing::SetToBroadCast()
{
    int optval = 1;
    
    if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
        return false;
    return true;
}
////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Outgoing::InitToAddress
// Description     : Connects the Socket to a Specified address
//
// Return type  : inline bool
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP_Outgoing::InitToAddress(Socket_Address & address)
{
    if (InitNoAddress() != true)
        return false;
    
    if (DO_CONNECT(_socket, &address.GetAddressInfo()) != 0)
        return ErrorClose();
    
    return true;
}
////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Outgoing::InitNoAddress
// Description     : This will set a udp up for targeted sends..
//
// Return type  : inline bool
// Argument         : void
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP_Outgoing::InitNoAddress()
{
    Close();
    _socket = DO_NEWUDP();
    if (_socket == BAD_SOCKET)
        return false;
    
    return true;
}
////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Outgoing::Send
// Description     : Send data to connected address
//
// Return type  : inline bool
// Argument         : char * data
// Argument         : int len
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP_Outgoing::Send(const char * data, int len)
{
    return (DO_SOCKET_WRITE(_socket, data, len) == len);
}
////////////////////////////////////////////////////////////////////
// Function name : Socket_UDP_Outgoing::SendTo
// Description     : Send data to specified address
//
// Return type  : inline bool
// Argument         : char * data
// Argument         : int len
// Argument         : NetAddress & address
////////////////////////////////////////////////////////////////////
inline bool Socket_UDP_Outgoing::SendTo(const char * data, int len, const Socket_Address & address)
{
    return (DO_SOCKET_WRITE_TO(_socket, data, len, &address.GetAddressInfo()) == len);
}

#endif //__SOCKET_UDP_OUTGOING_H__
