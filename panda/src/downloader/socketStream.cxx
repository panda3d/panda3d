/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socketStream.cxx
 * @author drose
 * @date 2002-10-19
 */

#include "socketStream.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "httpChannel.h"
#include "config_downloader.h"

#ifdef HAVE_OPENSSL

/**
 *
 */
SSReader::
SSReader(std::istream *stream) : _istream(stream) {
  _data_expected = 0;
  _tcp_header_size = tcp_header_size;

#ifdef SIMULATE_NETWORK_DELAY
  _delay_active = false;
  _min_delay = 0.0;
  _delay_variance = 0.0;
#endif  // SIMULATE_NETWORK_DELAY
}

/**
 *
 */
SSReader::
~SSReader() {
}

/**
 * Receives a datagram over the socket by expecting a little-endian 16-bit
 * byte count as a prefix.  If the socket stream is non-blocking, may return
 * false if the data is not available; otherwise, returns false only if the
 * socket closes.
 */
bool SSReader::
do_receive_datagram(Datagram &dg) {
  if (_tcp_header_size == 0) {
    _data_expected = _data_so_far.size();
  }
  if (_data_expected == 0) {
    // Read the first two bytes: the datagram length.
    while ((int)_data_so_far.size() < _tcp_header_size) {
      int ch = _istream->get();
      if (ch == EOF || _istream->fail()) {
        _istream->clear();
        return false;
      }
      _data_so_far.push_back((unsigned char)ch);
    }

    Datagram header(_data_so_far);
    DatagramIterator di(header);
    if (_tcp_header_size == 2) {
      _data_expected = di.get_uint16();
    } else if (_tcp_header_size == 4) {
      _data_expected = di.get_uint32();
    }
    _data_so_far.erase(_data_so_far.begin(), _data_so_far.begin() + _tcp_header_size);

    if (_data_expected == 0) {
      // Empty datagram.
      dg.clear();
      return true;
    }
  }

  // Read the next n bytes until the datagram is filled.

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

  size_t read_count = std::min(_data_expected - _data_so_far.size(), buffer_size);
  _istream->read(buffer, read_count);
  size_t count = _istream->gcount();
  while (count != 0) {
    _data_so_far.insert(_data_so_far.end(), buffer, buffer + count);

    read_count = std::min(_data_expected - _data_so_far.size(),
                     buffer_size);
    _istream->read(buffer, read_count);
    count = _istream->gcount();
  }

  if (_data_so_far.size() < _data_expected) {
    // Not yet here.  Clear the istream error flag and return false to
    // indicate more coming.
    _istream->clear();
    return false;
  }

  dg.clear();
  dg.append_data(_data_so_far);

  _data_expected = 0;
  _data_so_far.clear();

  return true;
}

#ifdef SIMULATE_NETWORK_DELAY
/**
 * Enables a simulated network latency.  All datagrams received from this
 * point on will be held for a random interval of least min_delay seconds, and
 * no more than max_delay seconds, before being visible.  It is as if
 * datagrams suddenly took much longer to arrive.
 *
 * This should *only* be called if the underlying socket is non-blocking.  If
 * you call this on a blocking socket, it will force all datagrams to be held
 * up until the socket closes.
 */
void SSReader::
start_delay(double min_delay, double max_delay) {
  _min_delay = min_delay;
  _delay_variance = std::max(max_delay - min_delay, 0.0);
  _delay_active = true;
}
#endif  // SIMULATE_NETWORK_DELAY

#ifdef SIMULATE_NETWORK_DELAY
/**
 * Disables the simulated network latency started by a previous call to
 * start_delay().  Datagrams will once again be visible as soon as they are
 * received.
 */
void SSReader::
stop_delay() {
  _delay_active = false;
}
#endif  // SIMULATE_NETWORK_DELAY

#ifdef SIMULATE_NETWORK_DELAY
/**
 * Adds the datagram to the delay queue for a random time interval.
 */
void SSReader::
delay_datagram(const Datagram &datagram) {
  nassertv(_delay_active);

  double now = TrueClock::get_global_ptr()->get_short_time();
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
/**
 * Checks the delayed queue for any now available datagrams.  If any are
 * available, returns true and fills datagram with its value.
 */
bool SSReader::
get_delayed(Datagram &datagram) {
  if (_delayed.empty()) {
    return false;
  }
  const DelayedDatagram &dd = _delayed.front();
  if (_delay_active) {
    double now = TrueClock::get_global_ptr()->get_short_time();
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

/**
 *
 */
SSWriter::
SSWriter(std::ostream *stream) : _ostream(stream) {
  _collect_tcp = collect_tcp;
  _collect_tcp_interval = collect_tcp_interval;
  _queued_data_start = 0.0;
  _tcp_header_size = tcp_header_size;
}

/**
 *
 */
SSWriter::
~SSWriter() {
}

/**
 * Transmits the indicated datagram over the socket by prepending it with a
 * little-endian 16-bit byte count.  Does not return until the data is sent or
 * the connection is closed, even if the socket stream is non-blocking.
 */
bool SSWriter::
send_datagram(const Datagram &dg) {
  Datagram header;
  if (_tcp_header_size == 2) {
    if (dg.get_length() >= 0x10000) {
      downloader_cat.error()
        << "Attempt to send TCP datagram of " << dg.get_length()
        << " bytes--too long!\n";
      nassert_raise("Datagram too long");
      return false;
    }

    header.add_uint16(dg.get_length());
  } else if (_tcp_header_size == 4) {
    header.add_uint32(dg.get_length());
  }

  // These two writes don't generate two socket calls, because the socket
  // stream is always buffered.
  _ostream->write((const char *)header.get_data(), header.get_length());
  _ostream->write((const char *)dg.get_data(), dg.get_length());

  // Now flush the buffer immediately, forcing the data to be sent (unless
  // collect-tcp mode is in effect).
  flush();

  return !is_closed();
}

/**
 *
 */
ISocketStream::
~ISocketStream() {
  // This should already have been cleared by the subclass destructor.
  nassertv(_channel == nullptr);
}

#endif  // HAVE_OPENSSL
