// Filename: socketStream.cxx
// Created by:  drose (19Oct02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "socketStream.h"
#include "datagram.h"
#include "datagramIterator.h"

#ifdef HAVE_SSL

////////////////////////////////////////////////////////////////////
//     Function: ISocketStream::receive_datagram
//       Access: Public
//  Description: Receives a datagram over the socket by expecting a
//               little-endian 16-bit byte count as a prefix.  If the
//               socket stream is non-blocking, may return false if
//               the data is not available; otherwise, returns false
//               only if the socket closes.
////////////////////////////////////////////////////////////////////
bool ISocketStream::
receive_datagram(Datagram &dg) {
  if (_data_expected == 0) {
    // Read the first two bytes: the datagram length.
    while (_data_so_far.length() < 2) {
      int ch = get();
      if (eof()) {
        clear();
        return false;
      }
      _data_so_far += (char)ch;
    }

    Datagram header(_data_so_far);
    DatagramIterator di(header);
    _data_expected = di.get_uint16();
    _data_so_far = string();

    if (_data_expected == 0) {
      // Empty datagram.
      dg.clear();
      return true;
    }
  }

  // Read the next n bytes until the datagram is filled.
  while (_data_so_far.length() < _data_expected) {
    int ch = get();
    if (eof()) {
      clear();
      return false;
    }
    _data_so_far += (char)ch;
  }

  dg.clear();
  dg.append_data(_data_so_far);

  _data_expected = 0;
  _data_so_far = string();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OSocketStream::send_datagram
//       Access: Public
//  Description: Transmits the indicated datagram over the socket by
//               prepending it with a little-endian 16-bit byte count.
//               Does not return until the data is sent or the
//               connection is closed, even if the socket stream is
//               non-blocking.
////////////////////////////////////////////////////////////////////
bool OSocketStream::
send_datagram(const Datagram &dg) {
  Datagram header;
  header.add_uint16(dg.get_length());
  write((const char *)header.get_data(), header.get_length());
  write((const char *)dg.get_data(), dg.get_length());
  flush();

  return !is_closed();
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::receive_datagram
//       Access: Public
//  Description: Receives a datagram over the socket by expecting a
//               little-endian 16-bit byte count as a prefix.  If the
//               socket stream is non-blocking, may return false if
//               the data is not available; otherwise, returns false
//               only if the socket closes.
////////////////////////////////////////////////////////////////////
bool SocketStream::
receive_datagram(Datagram &dg) {
  if (_data_expected == 0) {
    // Read the first two bytes: the datagram length.
    while (_data_so_far.length() < 2) {
      int ch = get();
      if (eof()) {
        clear();
        return false;
      }
      _data_so_far += (char)ch;
    }

    Datagram header(_data_so_far);
    DatagramIterator di(header);
    _data_expected = di.get_uint16();
    _data_so_far = string();

    if (_data_expected == 0) {
      // Empty datagram.
      dg.clear();
      return true;
    }
  }

  // Read the next n bytes until the datagram is filled.
  while (_data_so_far.length() < _data_expected) {
    int ch = get();
    if (eof()) {
      clear();
      return false;
    }
    _data_so_far += (char)ch;
  }

  dg.clear();
  dg.append_data(_data_so_far);

  _data_expected = 0;
  _data_so_far = string();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SocketStream::send_datagram
//       Access: Public
//  Description: Transmits the indicated datagram over the socket by
//               prepending it with a little-endian 16-bit byte count.
//               Does not return until the data is sent or the
//               connection is closed, even if the socket stream is
//               non-blocking.
////////////////////////////////////////////////////////////////////
bool SocketStream::
send_datagram(const Datagram &dg) {
  Datagram header;
  header.add_uint16(dg.get_length());
  write((const char *)header.get_data(), header.get_length());
  write((const char *)dg.get_data(), dg.get_length()); 
  flush();

  return !is_closed();
}

#endif  // HAVE_SSL
