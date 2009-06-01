#ifndef __SOCKET_ADDRESS_H__
#define __SOCKET_ADDRESS_H__

#include "pandabase.h"
#include "numeric_types.h"
#include "socket_portable.h"

///////////////////////////////////
// Class : Socket_Address
//
// Description: A simple place to store and munipulate tcp and port address for
//    communication layer
//
//////////////////////////////
class EXPCL_PANDA_NATIVENET Socket_Address
{
public:
    typedef struct sockaddr_in AddressType;
    Socket_Address(const AddressType &inaddr);
    AddressType & GetAddressInfo() { return _addr; }
    const AddressType & GetAddressInfo() const   { return _addr; }
PUBLISHED:
    
    inline Socket_Address(short port = 0);
    inline Socket_Address(const Socket_Address &inaddr);

    inline virtual ~Socket_Address();
    
    inline bool set_any_IP(int port);
    inline bool set_port(int port);
    inline bool set_broadcast(int port);
    
    inline bool set_host(const std::string &hostname, int port) ;
    inline bool set_host(const std::string &hostname) ;
    inline bool set_host(unsigned int ip4adr, int port);
    inline void clear();
    
    inline unsigned short get_port() const;
    inline std::string get_ip() const ;
    inline std::string get_ip_port() const;
    inline unsigned long GetIPAddressRaw() const;
    
    inline bool operator== (const Socket_Address &in) const;
    inline bool operator < (const Socket_Address &in) const;
    

    inline bool isMcastRange();

private:
    AddressType _addr;
    
};

//////////////////////////////////////////////////////////////
// Function name : Socket_Address::GetIPAdddressRaw
// Description   : Return a RAW sockaddr_in
//////////////////////////////////////////////////////////////
inline unsigned long  Socket_Address::GetIPAddressRaw() const
{
    return _addr.sin_addr.s_addr;
}

///////////////////////////////////////////////////////////////////
// Function name : Socket_Address
// Description   : Constructor that lets us set a port value
////////////////////////////////////////////////////////////////////
inline Socket_Address::Socket_Address(short port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(port);
}

////////////////////////////////////////////////////////////////////
// Function name : Socket_Address Constructor
// Description   : Copy Constructor
////////////////////////////////////////////////////////////////////
inline Socket_Address::Socket_Address(const Socket_Address &inaddr)
{
    _addr.sin_family = inaddr._addr.sin_family;
    _addr.sin_addr.s_addr = inaddr._addr.sin_addr.s_addr;
    _addr.sin_port = inaddr._addr.sin_port;
}

inline Socket_Address::Socket_Address(const AddressType &inaddr)
{
    _addr.sin_family = inaddr.sin_family;
    _addr.sin_addr.s_addr = inaddr.sin_addr.s_addr;
    _addr.sin_port = inaddr.sin_port;
}

////////////////////////////////////////////////////////////////////
// Function name : ~Socket_Address::Socket_Address
// Description   : Normal Destructor
////////////////////////////////////////////////////////////////////
inline Socket_Address::~Socket_Address()
{}

//////////////////////////////////////////////////////////////
// Function name : Socket_Address::operator==
// Description   : Allow for normal == operation on a address item..
//      Will simplify the use in sorted containers..
//////////////////////////////////////////////////////////////
inline bool Socket_Address::operator==(const Socket_Address &in) const
{
    return ((_addr.sin_family == in._addr.sin_family) &&
        (_addr.sin_addr.s_addr == in._addr.sin_addr.s_addr) &&
        (_addr.sin_port == in._addr.sin_port)
        );
}

////////////////////////////////////////////////////////////////////
// Function name : set_broadcast
// Description   : Set to the broadcast address and a specified port
////////////////////////////////////////////////////////////////////
inline bool Socket_Address::set_broadcast(int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = 0xffffffff;
    _addr.sin_port = htons(port);
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : set_any_IP
// Description   : Set to any address and a specified port
////////////////////////////////////////////////////////////////////
inline bool Socket_Address::set_any_IP(int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(port);
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : set_port
// Description   : Set to a specified port
////////////////////////////////////////////////////////////////////
inline bool Socket_Address::set_port(int port)
{
    _addr.sin_port = htons(port);
    return true;
}

////////////////////////////////////////////////////////////////////
// Function name : clear
// Description   : Set the internal values to a suitable known value
////////////////////////////////////////////////////////////////////
inline void Socket_Address::clear()
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(0);
}

////////////////////////////////////////////////////////////////////
// Function name : get_port
// Description   : Get the port portion as an integer
////////////////////////////////////////////////////////////////////
inline unsigned short Socket_Address::get_port() const
{
    return ntohs(_addr.sin_port);
}

////////////////////////////////////////////////////////////////////
// Function name : get_ip
// Description   : Return the ip address portion in dot notation string
////////////////////////////////////////////////////////////////////
inline std::string Socket_Address::get_ip() const
{
    return std::string(inet_ntoa(_addr.sin_addr));
}

////////////////////////////////////////////////////////////////////
// Function name : get_ip_port
// Description   : Return the ip address/port in dot notation string
////////////////////////////////////////////////////////////////////
inline std::string Socket_Address::get_ip_port() const
{
    char buf1[100];  // 100 is more than enough for any ip address:port combo..
    sprintf(buf1, "%s:%d", inet_ntoa(_addr.sin_addr), get_port());
    return std::string(buf1);
}

////////////////////////////////////////////////////////////////////
// Function name : set_host
// Description   : this function will take a port and string-based tcp address and initialize
//      the address with the information
//
// Return type  : bool (address is undefined after an error)
////////////////////////////////////////////////////////////////////
inline bool Socket_Address::set_host(const std::string &hostname, int port) 
{
    struct hostent  *hp = NULL;

    //
    // hmm inet_addr does not resolve 255.255.255.255 on ME/98 ??
    //
    // * HACK * ?? 
    if(hostname  == "255.255.255.255")
        return set_broadcast(port);
    //
    //

    PN_uint32 addr =  (long)inet_addr (hostname.c_str());                
    if(addr == INADDR_NONE)
    {
        hp = gethostbyname(hostname.c_str());
        if(hp== NULL)
            return false;
        else
            memcpy(&(_addr.sin_addr),hp->h_addr_list[0] ,  (unsigned int) hp->h_length);                    
    }
    else
        (void) memcpy(&_addr.sin_addr,&addr,sizeof(addr));

    _addr.sin_port = htons(port);
    _addr.sin_family = AF_INET;
    return true;
}

//////////////////////////////////////////////////////////////
// Function name : Socket_Address::set_host
// Description   :
//////////////////////////////////////////////////////////////
inline bool Socket_Address::set_host(const std::string &hostname)
{
    std::string::size_type pos = hostname.find(':');
    if (pos == std::string::npos)
        return false;
    
    std::string host = hostname.substr(0, pos);
    std::string port = hostname.substr(pos + 1, 100);;
    
    int port_dig = atoi(port.c_str());
    
    return set_host(host, port_dig);
}

//////////////////////////////////////////////////////////////
// Function name : Socket_Address::set_host
// Description   :
//////////////////////////////////////////////////////////////
inline bool Socket_Address::set_host(PN_uint32 in_hostname, int port)
{
    memcpy(&_addr.sin_addr, &in_hostname, sizeof(in_hostname));
    _addr.sin_port = htons(port);
    _addr.sin_family = AF_INET;
    return true;
}

//////////////////////////////////////////////////////////////
// Function name : <
// Description   :
//////////////////////////////////////////////////////////////
inline bool Socket_Address::operator < (const Socket_Address &in) const
{
    if (_addr.sin_port < in._addr.sin_port)
        return true;
    
    if (_addr.sin_port > in._addr.sin_port)
        return false;
    
    if (_addr.sin_addr.s_addr < in._addr.sin_addr.s_addr)
        return true;
    
    if (_addr.sin_addr.s_addr > in._addr.sin_addr.s_addr)
        return false;
    
    
    return (_addr.sin_family < in._addr.sin_family);
}
//////////////////////////////////////////////////////////////
// Function name : isMcastRange
// Description   : return true if the address is in the mcast range.
//////////////////////////////////////////////////////////////
inline bool Socket_Address::isMcastRange(void)
{
    PN_uint32  address = ntohl(_addr.sin_addr.s_addr);
//224.0.0.0-239.255.255.255 .. e0,ef
    if(address >= 0xe0000000 && address < 0xefffffff)
        return true;
    return false;
}


#endif //__SOCKET_ADDRESS_H__
