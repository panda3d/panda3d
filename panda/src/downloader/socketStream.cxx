// Filename: socketStream.cxx
// Created by:  drose (19Oct02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "socketStream.h"
#include "datagram.h"
#include "datagramIterator.h"

#ifdef HAVE_SSL

////////////////////////////////////////////////////////////////////
//     Function: SSReader::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SSReader::
~SSReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: SSReader::do_receive_datagram
//       Access: Private
//  Description: Receives a datagram over the socket by expecting a
//               little-endian 16-bit byte count as a prefix.  If the
//               socket stream is non-blocking, may return false if
//               the data is not available; otherwise, returns false
//               only if the socket closes.
////////////////////////////////////////////////////////////////////
bool SSReader::
do_receive_datagram(Datagram &dg) {
  if (_data_expected == 0) {
    // Read the first two bytes: the datagram length.
    while (_data_so_far.length() < 2) {
      int ch = _istream->get();
      if (_istream->eof()) {
        _istream->clear();
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
    int ch = _istream->get();
    if (_istream->eof()) {
      _istream->clear();
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

#ifdef SIMULATE_NETWORK_DELAY
////////////////////////////////////////////////////////////////////
//     Function: SSReader::start_delay
//       Access: Published
//  Description: Enables a simulated network latency.  All datagrams
//               received from this point on will be held for a random
//               interval of least min_delay seconds, and no more than
//               max_delay seconds, before being visible.  It is as if
//               datagrams suddenly took much longer to arrive.
//
//               This should *only* be called if the underlying socket
//               is non-blocking.  If you call this on a blocking
//               socket, it will force all datagrams to be held up
//               until the socket closes.
////////////////////////////////////////////////////////////////////
void SSReader::
start_delay(double min_delay, double max_delay) {
  _min_delay = min_delay;
  _delay_variance = max(max_delay - min_delay, 0.0);
  _delay_active = true;
}
#endif  // SIMULATE_NETWORK_DELAY

#ifdef SIMULATE_NETWORK_DELAY
////////////////////////////////////////////////////////////////////
//     Function: SSReader::stop_delay
//       Access: Published
//  Description: Disables the simulated network latency started by a
//               previous call to start_delay().  Datagrams will once
//               again be visible as soon as they are received.
////////////////////////////////////////////////////////////////////
void SSReader::
stop_delay() {
  _delay_active = false;
}
#endif  // SIMULATE_NETWORK_DELAY

#ifdef SIMULATE_NETWORK_DELAY
////////////////////////////////////////////////////////////////////
//     Function: SSReader::delay_datagram
//       Access: Private
//  Description: Adds the datagram to the delay queue for a random
//               time interval.
////////////////////////////////////////////////////////////////////
void SSReader::
delay_datagram(const Datagram &datagram) {
  nassertv(_delay_active);

  double now = ClockObject::get_global_clock()->get_frame_time();
  double reveal_time = now + _min_delay;
  
  if (_delay_variance > 0.0) {
    reveal_time += _delay_variance * ((double)rand() / (double)RAND_MAX);
  }
  _delayed.push_back(DelayedDatagram());
  DelayedDatagram &dd = _delayed.back();
  dd._reveal_time = reveal_time;
  dd._datagram = datagram;
}
#endif  // SIMULATE_NETWORK_DELAY

#ifdef SIMULATE_NETWORK_DELAY
////////////////////////////////////////////////////////////////////
//     Function: SSReader::get_delayed
//       Access: Private
//  Description: Checks the delayed queue for any now available
//               datagrams.  If any are available, returns true and
//               fills datagram with its value.
////////////////////////////////////////////////////////////////////
bool SSReader::
get_delayed(Datagram &datagram) {
  if (_delayed.empty()) {
    return false;
  }
  const DelayedDatagram &dd = _delayed.front();
  if (_delay_active) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    if (dd._reveal_time > now) {
      // Not yet.
      return false;
    }
  }

  datagram = dd._datagram;
  _delayed.pop_front();

  return true;
}
#endif  // SIMULATE_NETWORK_DELAY

////////////////////////////////////////////////////////////////////
//     Function: SSWriter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SSWriter::
~SSWriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: SSWriter::send_datagram
//       Access: Public
//  Description: Transmits the indicated datagram over the socket by
//               prepending it with a little-endian 16-bit byte count.
//               Does not return until the data is sent or the
//               connection is closed, even if the socket stream is
//               non-blocking.
////////////////////////////////////////////////////////////////////
bool SSWriter::
send_datagram(const Datagram &dg) {
  Datagram header;
  header.add_uint16(dg.get_length());

  // These two writes don't generate two socket calls, because the
  // socket stream is always buffered.
  _ostream->write((const char *)header.get_data(), header.get_length());
  _ostream->write((const char *)dg.get_data(), dg.get_length());

  // Now flush the buffer immediately, forcing the data to be sent
  // (unless collect-tcp mode is in effect).
  flush();

  return !is_closed();
}

#endif  // HAVE_SSL
