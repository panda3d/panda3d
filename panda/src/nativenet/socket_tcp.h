#ifndef __SOCKET_TCP_H__
#define __SOCKET_TCP_H__ 

#include "pandabase.h"
#include "socket_ip.h"

/////////////////////////////////////////////////////////////////////
// Class : Socket_TCP
//
// Description : Base functionality for a TCP connected socket
//               This class is pretty useless by itself but it does hide some of the
//               platform differences from machine to machine
//
/////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_NATIVENET Socket_TCP : public Socket_IP
{
public:    
PUBLISHED:
    inline Socket_TCP(SOCKET);
    inline Socket_TCP()    {   };    
    inline int  SetNoDelay(bool flag = true);
    inline int  SetLinger(int interval_seconds = 0);
    inline int  DontLinger();    
    inline int  SetSendBufferSize(int insize);
    //inline bool ActiveOpen(const Socket_Address & theaddress);
    inline bool ActiveOpen(const Socket_Address & theaddress, bool setdelay);
    inline bool ActiveOpenNonBlocking(const Socket_Address & theaddress);
    inline bool ErrorIs_WouldBlocking(int err);
    inline bool ShutdownSend();
    inline int  SendData(const std::string &str);
//    inline int RecvData( std::string &str, int max_len);

    std::string RecvData(int max_len);
public:
    inline int  SendData(const char * data, int size);
    inline int  RecvData(char * data, int size);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Socket_IP::init_type();
    register_type(_type_handle, "Socket_TCP",
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
// Function name : Socket_TCP::Socket_TCP
// Description   :
//////////////////////////////////////////////////////////////
inline Socket_TCP::Socket_TCP(SOCKET sck) : ::Socket_IP(sck)
{
}

////////////////////////////////////////////////////////////////////
// Function name : SetNoDelay
// Description   : Disable Nagle algorithm. Don't delay send to coalesce packets
////////////////////////////////////////////////////////////////////
inline int Socket_TCP::SetNoDelay(bool flag)
{
    int nodel = flag;
    int ret1;
    ret1 = setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char *) & nodel, sizeof(nodel));
    
    if (ret1 != 0)
        return BASIC_ERROR;
    
    return ALL_OK;
}

////////////////////////////////////////////////////////////////////
// Function name : SetLinger
// Description   : will control the behavior of SO_LINGER for a TCP socket
////////////////////////////////////////////////////////////////////
int Socket_TCP::SetLinger(int interval_seconds)
{
    linger ll;
    ll.l_linger = interval_seconds;
    ll.l_onoff = 1;
    int ret1 = setsockopt(_socket, SOL_SOCKET, SO_LINGER, (const char *) & ll, sizeof(linger));
    if (ret1 != 0)
        return BASIC_ERROR;
    return ALL_OK;
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_TCP::DontLinger
// Description   : Turn off the linger flag. The socket will quickly release
//        buffered items and free up OS resources. You may lose
//        a stream if you use this flag and do not negotiate the close
//        at the application layer.
////////////////////////////////////////////////////////////////////
int Socket_TCP::DontLinger()
{
    linger ll;
    ll.l_linger = 0;
    ll.l_onoff = 0;
    int ret1 = setsockopt(_socket, SOL_SOCKET, SO_LINGER, (const char *) & ll, sizeof(linger));
    if (ret1 != 0)
        return BASIC_ERROR;
    return ALL_OK;
}

////////////////////////////////////////////////////////////////////
// Function name : SetSendBufferSize
// Description   : Just like it sounds. Sets a buffered socket recv buffer size.
//      This function does not refuse ranges outside hard-coded OS
//      limits
////////////////////////////////////////////////////////////////////
int Socket_TCP::SetSendBufferSize(int insize)
{
    if (setsockopt(_socket, (int) SOL_SOCKET, (int) SO_SNDBUF, (char *) &insize, sizeof(int)))
        return BASIC_ERROR;
    return ALL_OK;
}

////////////////////////////////////////////////////////////////////
// Function name : ActiveOpen
// Description   : This function will try and set the socket up for active open to a specified
//       address and port provided by the input parameter
////////////////////////////////////////////////////////////////////
bool Socket_TCP::ActiveOpen(const Socket_Address & theaddress, bool setdelay)
{
    _socket = DO_NEWTCP();
    if (_socket == BAD_SOCKET)
        return false;

    if(setdelay)
        SetNoDelay();
    
    if (DO_CONNECT(_socket, &theaddress.GetAddressInfo()) != 0)
        return ErrorClose();
    
    return true;
}


////////////////////////////////////////////////////////////////////
// Function name : ActiveOpenNonBlocking
// Description   : This function will try and set the socket up for active open to a specified
//       address and port provided by the input parameter (non-blocking version)
////////////////////////////////////////////////////////////////////
bool Socket_TCP::ActiveOpenNonBlocking(const Socket_Address & theaddress)
{
    _socket = DO_NEWTCP();
    if (_socket == BAD_SOCKET)
        return false;
    
    SetNonBlocking();
    SetReuseAddress();
    
    if (DO_CONNECT(_socket, &theaddress.GetAddressInfo()) != 0) {
        if (GETERROR() != LOCAL_CONNECT_BLOCKING)
        {
            printf(" None Blockign Connect Error %d",GETERROR());
            return ErrorClose();
        }
    }
        
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_TCP::SendData
// Description   : Ok Lets Send the Data
//
// Return type  : int
//      - if error
//      0 if socket closed for write or lengh is 0
//      + bytes writen ( May be smaller than requested)
////////////////////////////////////////////////////////////////////
inline int Socket_TCP::SendData(const char * data, int size)
{
    return DO_SOCKET_WRITE(_socket, data, size);
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_TCP::RecvData
// Description   : Read the data from the connection
//
// Return type  : int
//      - if error
//      0 if socket closed for read or length is 0
//      + bytes read ( May be smaller than requested)
////////////////////////////////////////////////////////////////////
inline int Socket_TCP::RecvData(char * data, int len)
{
    int ecode = DO_SOCKET_READ(_socket, data, len);
    return ecode;
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_TCP::RecvData
// Description   : Read the data from the connection
//
// Return type  : int
//      - if error
//      0 if socket closed for read or length is 0
//      + bytes read ( May be smaller than requested)
////////////////////////////////////////////////////////////////////
inline std::string  Socket_TCP::RecvData(int max_len)
{
    std::string str;
    char    *buffer = (char *) malloc(max_len+1);
    int ecode  = RecvData(buffer,max_len);
    if(ecode > 0)
        str.assign(buffer,ecode);

    free(buffer);
    return str;
};


inline bool Socket_TCP::ErrorIs_WouldBlocking(int err)
{
    return (GETERROR() == LOCAL_BLOCKING_ERROR);
}

inline bool Socket_TCP::ShutdownSend()
{
    return do_shutdown_send(_socket);
};

/*
inline bool Socket_TCP::DoNotLinger()
{
    int bOption = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_DONTLINGER, (const char *)&bOption, sizeof(bOption)) != 0)
        return false;
    return true;
}
*/

inline int Socket_TCP::SendData(const std::string &str)
{
    return SendData(str.data(), str.size());
};

#endif //__SOCKET_TCP_H__
