#ifndef SOCKET_ADDRESS_H
#define SOCKET_ADDRESS_H

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
class EXPCL_PANDA_NATIVENET Socket_Address {
public:
  typedef struct sockaddr_in AddressType;
  Socket_Address(const AddressType &inaddr);
  AddressType &GetAddressInfo() { return _addr; }
  const AddressType &GetAddressInfo() const { return _addr; }

PUBLISHED:
  INLINE Socket_Address(unsigned short port = 0);
  INLINE Socket_Address(const Socket_Address &inaddr);

  INLINE virtual ~Socket_Address();

  INLINE bool set_any_IP(int port);
  INLINE bool set_port(int port);
  INLINE bool set_broadcast(int port);

  INLINE bool set_host(const std::string &hostname, int port) ;
  INLINE bool set_host(const std::string &hostname) ;
  INLINE bool set_host(unsigned int ip4adr, int port);
  INLINE void clear();

  INLINE unsigned short get_port() const;
  INLINE std::string get_ip() const ;
  INLINE std::string get_ip_port() const;
  INLINE unsigned long GetIPAddressRaw() const;

  INLINE bool operator ==(const Socket_Address &in) const;
  INLINE bool operator !=(const Socket_Address &in) const;
  INLINE bool operator < (const Socket_Address &in) const;

  INLINE bool is_mcast_range() const;

private:
  AddressType _addr;
};

#include "socket_address.I"

#endif  // SOCKET_ADDRESS_H
