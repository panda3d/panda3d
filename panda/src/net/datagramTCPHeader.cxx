// Filename: datagramTCPHeader.cxx
// Created by:  drose (08Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "datagramTCPHeader.h"
#include "netDatagram.h"
#include "datagramIterator.h"
#include "config_net.h"

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: DatagramTCPHeader::Constructor
//       Access: Public
//  Description: This constructor creates a header based on an
//               already-constructed NetDatagram.
////////////////////////////////////////////////////////////////////
DatagramTCPHeader::
DatagramTCPHeader(const NetDatagram &datagram) {
  const string &str = datagram.get_message();
  PRUint16 size = str.length();
  nassertv(size == str.length());

  // Now pack the header.
  _header.add_uint16(size);
  nassertv((int)_header.get_length() == datagram_tcp_header_size);
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramTCPHeader::Constructor
//       Access: Public
//  Description: This constructor decodes a header from a block of
//               data of length datagram_tcp_header_size, presumably
//               just read from a socket.
////////////////////////////////////////////////////////////////////
DatagramTCPHeader::
DatagramTCPHeader(const void *data) : _header(data, datagram_tcp_header_size) {
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramTCPHeader::get_datagram_size
//       Access: Public
//  Description: Returns the number of bytes in the associated
//               datagram.
////////////////////////////////////////////////////////////////////
int DatagramTCPHeader::
get_datagram_size() const {
  DatagramIterator di(_header);
  return di.get_uint16();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramTCPHeader::get_header
//       Access: Public
//  Description: Returns a pointer to a block of data of length
//               datagram_tcp_header_size, which can be written to the
//               network as the header information.
////////////////////////////////////////////////////////////////////
const string &DatagramTCPHeader::
get_header() const {
  return _header.get_message();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramTCPHeader::verify_datagram
//       Access: Public
//  Description: Verifies that the indicated datagram has the
//               appropriate length.  Returns true if it matches,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool DatagramTCPHeader::
verify_datagram(const NetDatagram &datagram) const {
  const string &str = datagram.get_message();
  PRUint16 size = str.length();
  nassertr(size == str.length(), false);

  if (size == get_datagram_size()) {
    return true;
  }

  if (net_cat.is_debug()) {
    net_cat.debug()
      << "Invalid datagram!\n";
    if (size != get_datagram_size()) {
      net_cat.debug()
	<< "  size is " << size << " bytes, header reports " 
	<< get_datagram_size() << "\n";
    }
    
    // We write the hex dump into a ostringstream first, to guarantee
    // an atomic write to the output stream in case we're threaded.
    
    ostringstream hex;
    datagram.dump_hex(hex);
    hex << "\n";
    net_cat.debug() << hex.str();
  }

  return false;
}
