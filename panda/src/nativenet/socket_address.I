// Filename: socket_address.I
// Created by:  rdb (19Oct14)
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


//////////////////////////////////////////////////////////////
//     Function: Socket_Address::GetIPAdddressRaw
//       Access: Public
//  Description: Return a RAW sockaddr_in
//////////////////////////////////////////////////////////////
INLINE unsigned long Socket_Address::
GetIPAddressRaw() const {
  return _addr.sin_addr.s_addr;
}

///////////////////////////////////////////////////////////////////
//     Function: Socket_Address
//       Access: Published
//  Description: Constructor that lets us set a port value
////////////////////////////////////////////////////////////////////
INLINE Socket_Address::
Socket_Address(unsigned short port) {
  _addr.sin_family = AF_INET;
  _addr.sin_addr.s_addr = INADDR_ANY;
  _addr.sin_port = htons(port);
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::Copy constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Socket_Address::
Socket_Address(const Socket_Address &inaddr) {
  _addr.sin_family = inaddr._addr.sin_family;
  _addr.sin_addr.s_addr = inaddr._addr.sin_addr.s_addr;
  _addr.sin_port = inaddr._addr.sin_port;
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Socket_Address::
Socket_Address(const AddressType &inaddr) {
  _addr.sin_family = inaddr.sin_family;
  _addr.sin_addr.s_addr = inaddr.sin_addr.s_addr;
  _addr.sin_port = inaddr.sin_port;
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::~Destructor
//       Access: Published
//  Description: Normal Destructor
////////////////////////////////////////////////////////////////////
INLINE Socket_Address::
~Socket_Address() {
}

//////////////////////////////////////////////////////////////
//     Function: Socket_Address::operator ==
//       Access: Published
//  Description:
//////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
operator == (const Socket_Address &in) const {
  return ((_addr.sin_family == in._addr.sin_family) &&
          (_addr.sin_addr.s_addr == in._addr.sin_addr.s_addr) &&
          (_addr.sin_port == in._addr.sin_port));
}

//////////////////////////////////////////////////////////////
//     Function: Socket_Address::operator !=
//       Access: Published
//  Description:
//////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
operator != (const Socket_Address &in) const {
  return ((_addr.sin_family != in._addr.sin_family) ||
          (_addr.sin_addr.s_addr != in._addr.sin_addr.s_addr) ||
          (_addr.sin_port != in._addr.sin_port));
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::set_broadcast
//       Access: Published
//  Description: Set to the broadcast address and a specified port
////////////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
set_broadcast(int port) {
  _addr.sin_family = AF_INET;
  _addr.sin_addr.s_addr = 0xffffffff;
  _addr.sin_port = htons(port);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::set_any_IP
//       Access: Published
//  Description: Set to any address and a specified port
////////////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
set_any_IP(int port) {
  _addr.sin_family = AF_INET;
  _addr.sin_addr.s_addr = INADDR_ANY;
  _addr.sin_port = htons(port);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::set_port
//       Access: Published
//  Description: Set to a specified port
////////////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
set_port(int port) {
  _addr.sin_port = htons(port);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::clear
//       Access: Published
//  Description: Set the internal values to a suitable known value
////////////////////////////////////////////////////////////////////
INLINE void Socket_Address::
clear() {
  _addr.sin_family = AF_INET;
  _addr.sin_addr.s_addr = INADDR_ANY;
  _addr.sin_port = htons(0);
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::get_port
//       Access: Published
//  Description: Get the port portion as an integer
////////////////////////////////////////////////////////////////////
INLINE unsigned short Socket_Address::
get_port() const {
  return ntohs(_addr.sin_port);
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::get_ip
//       Access: Published
//  Description: Return the IP address portion in dot notation string
////////////////////////////////////////////////////////////////////
INLINE std::string Socket_Address::
get_ip() const {
  return std::string(inet_ntoa(_addr.sin_addr));
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::get_ip_port
//       Access: Published
//  Description: Return the ip address/port in dot notation string
////////////////////////////////////////////////////////////////////
INLINE std::string Socket_Address::
get_ip_port() const {
  char buf1[100];  // 100 is more than enough for any ip address:port combo..
  sprintf(buf1, "%s:%d", inet_ntoa(_addr.sin_addr), get_port());
  return std::string(buf1);
}

////////////////////////////////////////////////////////////////////
//     Function: Socket_Address::set_host
//       Access: Published
//  Description: This function will take a port and string-based
//               TCP address and initialize the address with this
//               information.  Returns true on success; on failure,
//               it returns false and the address may be undefined.
////////////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
set_host(const std::string &hostname, int port) {
  struct hostent *hp = NULL;

  //
  // hmm inet_addr does not resolve 255.255.255.255 on ME/98 ??
  //
  // * HACK * ??
  if (hostname  == "255.255.255.255") {
    return set_broadcast(port);
  }
  //
  //

  PN_uint32 addr =  (long)inet_addr(hostname.c_str());
  if (addr == INADDR_NONE) {
    hp = gethostbyname(hostname.c_str());
    if (hp == NULL) {
      return false;
    } else {
      memcpy(&_addr.sin_addr, hp->h_addr_list[0], (unsigned int) hp->h_length);
    }
  } else {
    memcpy(&_addr.sin_addr, &addr, sizeof(addr));
  }

  _addr.sin_port = htons(port);
  _addr.sin_family = AF_INET;
  return true;
}

//////////////////////////////////////////////////////////////
//     Function: Socket_Address::set_host
//       Access: Published
//  Description:
//////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
set_host(const std::string &hostname) {
  std::string::size_type pos = hostname.find(':');
  if (pos == std::string::npos)
    return false;

  std::string host = hostname.substr(0, pos);
  std::string port = hostname.substr(pos + 1, 100);;

  int port_dig = atoi(port.c_str());
  return set_host(host, port_dig);
}

//////////////////////////////////////////////////////////////
//     Function: Socket_Address::set_host
//       Access: Published
//  Description:
//////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
set_host(PN_uint32 in_hostname, int port) {
  memcpy(&_addr.sin_addr, &in_hostname, sizeof(in_hostname));
  _addr.sin_port = htons(port);
  _addr.sin_family = AF_INET;
  return true;
}

//////////////////////////////////////////////////////////////
//     Function: <
//       Access: Published
//  Description:
//////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
operator < (const Socket_Address &in) const {
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
//     Function: is_mcast_range
//       Access: Published
//  Description: True if the address is in the multicast range.
//////////////////////////////////////////////////////////////
INLINE bool Socket_Address::
is_mcast_range(void) const {
  PN_uint32 address = ntohl(_addr.sin_addr.s_addr);
  //224.0.0.0-239.255.255.255 .. e0,ef
  return (address >= 0xe0000000 && address < 0xefffffff);
}
