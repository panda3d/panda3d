// Filename: pStatClientImpl.cxx
// Created by:  drose (23Dec04)
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

#include "pStatClientImpl.h"

// This file only defines anything if DO_PSTATS is defined.
#ifdef DO_PSTATS

#include "pStatClient.h"
#include "pStatClientControlMessage.h"
#include "pStatServerControlMessage.h"
#include "pStatCollector.h"
#include "pStatThread.h"
#include "config_pstats.h"
#include "pStatProperties.h"
#include "cmath.h"
#include "mathNumbers.h"

#include <algorithm>

#ifdef WIN32_VC
#include <windows.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatClientImpl::
PStatClientImpl(PStatClient *client) :
  _client(client),
  _reader(this, 0),
  _writer(this, pstats_threaded_write ? 1 : 0)
{
  _is_connected = false;
  _got_udp_port = false;
  _collectors_reported = 0;
  _threads_reported = 0;

  // Make sure our clock is in "normal" mode.
  _clock.set_mode(ClockObject::M_normal);

  _client_name = pstats_name;
  _max_rate = pstats_max_rate;

  _tcp_count = 1;
  _udp_count = 1;

  if (pstats_tcp_ratio >= 1.0f) {
    _tcp_count_factor = 0.0f;
    _udp_count_factor = 1.0f;

  } else if (pstats_tcp_ratio <= 0.0f) {
    _tcp_count_factor = 1.0f;
    _udp_count_factor = 0.0f;

  } else {
    csincos(pstats_tcp_ratio * MathNumbers::pi_f / 2.0f,
            &_udp_count_factor,
            &_tcp_count_factor);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatClientImpl::
~PStatClientImpl() {
  nassertv(!_is_connected);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::client_connect
//       Access: Public
//  Description: Called only by PStatClient::client_connect().
////////////////////////////////////////////////////////////////////
bool PStatClientImpl::
client_connect(string hostname, int port) {
  nassertr(!_is_connected, true);

  if (hostname.empty()) {
    hostname = pstats_host;
  }
  if (port < 0) {
    port = pstats_port;
  }

  if (!_server.set_host(hostname, port)) {
    pstats_cat.error()
      << "Unknown host: " << hostname << "\n";
    return false;
  }

  _tcp_connection = open_TCP_client_connection(_server, 5000);

  if (_tcp_connection.is_null()) {
    pstats_cat.error()
      << "Couldn't connect to PStatServer at " << hostname << ":"
      << port << "\n";
    return false;
  }
  // Make sure we're not queuing up multiple TCP sockets--we expect
  // immediate writes of our TCP datagrams.
  _tcp_connection->set_collect_tcp(false);

  _reader.add_connection(_tcp_connection);
  _is_connected = true;

  _udp_connection = open_UDP_connection();

  send_hello();

  return _is_connected;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::client_disconnect
//       Access: Public
//  Description: Called only by PStatClient::client_disconnect().
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
client_disconnect() {
  if (_is_connected) {
    _reader.remove_connection(_tcp_connection);
    close_connection(_tcp_connection);
    close_connection(_udp_connection);
  }

  _tcp_connection.clear();
  _udp_connection.clear();

  _is_connected = false;
  _got_udp_port = false;

  _collectors_reported = 0;
  _threads_reported = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::new_frame
//       Access: Public
//  Description: Called by the PStatThread interface at the beginning
//               of every frame, for each thread.  This resets the
//               clocks for the new frame and transmits the data for
//               the previous frame.
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
new_frame(int thread_index) {
  nassertv(thread_index >= 0 && thread_index < (int)_client->_threads.size());

  PStatClient::Thread &thread = _client->_threads[thread_index];

  // If we're the main thread, we should exchange control packets with
  // the server.
  if (thread_index == 0) {
    transmit_control_data();
  }

  // If we've got the UDP port by the time the frame starts, it's
  // time to become active and start actually tracking data.
  if (_got_udp_port) {
    thread._is_active = true;
  }

  if (!thread._is_active) {
    return;
  }

  float frame_start = _clock.get_real_time();

  if (!thread._frame_data.is_empty()) {
    // Collector 0 is the whole frame.
    _client->stop(0, thread_index, frame_start);

    // Fill up the level data for all the collectors who have level
    // data for this thread.
    int num_collectors = _client->_collectors.size();
    for (int i = 0; i < num_collectors; i++) {
      const PStatClient::PerThreadData &ptd = 
        _client->_collectors[i]._per_thread[thread_index];
      if (ptd._has_level) {
        thread._frame_data.add_level(i, ptd._level);
      }
    }
    transmit_frame_data(thread_index);
  }

  thread._frame_data.clear();
  thread._frame_number++;
  _client->start(0, thread_index, frame_start);

  // Also record the time for the PStats operation itself.
  int pstats_index = PStatClient::_pstats_pcollector.get_index();
  _client->start(pstats_index, thread_index, frame_start);
  _client->stop(pstats_index, thread_index, _clock.get_real_time());
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::transmit_frame_data
//       Access: Private
//  Description: Should be called once per frame per thread to
//               transmit the latest data to the PStatServer.
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
transmit_frame_data(int thread_index) {
  nassertv(thread_index >= 0 && thread_index < (int)_client->_threads.size());
  if (_is_connected && _client->_threads[thread_index]._is_active) {

    // We don't want to send too many packets in a hurry and flood the
    // server.  Check that enough time has elapsed for us to send a
    // new packet.  If not, we'll drop this packet on the floor and
    // send a new one next time around.
    float now = _clock.get_real_time();
    if (now >= _client->_threads[thread_index]._next_packet) {
      // We don't want to send more than _max_rate UDP-size packets
      // per second, per thread.
      float packet_delay = 1.0 / _max_rate;

      // Send new data.
      NetDatagram datagram;
      // We always start with a zero byte, to differentiate it from a
      // control message.
      datagram.add_uint8(0);

      datagram.add_uint16(thread_index);
      datagram.add_uint32(_client->_threads[thread_index]._frame_number);
      _client->_threads[thread_index]._frame_data.write_datagram(datagram);

      if (_writer.is_valid_for_udp(datagram)) {
        if (_udp_count * _udp_count_factor < _tcp_count * _tcp_count_factor) {
          // Send this one as a UDP packet.
          nassertv(_got_udp_port);
          _writer.send(datagram, _udp_connection, _server);
          _udp_count++;

          if (_udp_count == 0) {
            // Wraparound!
            _udp_count = 1;
            _tcp_count = 1;
          }

        } else {
          // Send this one as a TCP packet.
          _writer.send(datagram, _tcp_connection);
          _tcp_count++;

          if (_tcp_count == 0) {
            // Wraparound!
            _udp_count = 1;
            _tcp_count = 1;
          }
        }

      } else {
        _writer.send(datagram, _tcp_connection);
        // If our packets are so large that we must ship them via TCP,
        // then artificially slow down the packet rate even further.
        int packet_ratio =
          (datagram.get_length() + maximum_udp_datagram - 1) /
          maximum_udp_datagram;
        packet_delay *= (float)packet_ratio;
      }

      _client->_threads[thread_index]._next_packet = now + packet_delay;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::transmit_control_data
//       Access: Private
//  Description: Should be called once a frame to exchange control
//               information with the server.
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
transmit_control_data() {
  // Check for new messages from the server.
  while (_is_connected && _reader.data_available()) {
    NetDatagram datagram;

    if (_reader.get_data(datagram)) {
      PStatServerControlMessage message;
      if (message.decode(datagram)) {
        handle_server_control_message(message);

      } else {
        pstats_cat.error()
          << "Got unexpected message from server.\n";
      }
    }
  }

  if (_is_connected) {
    report_new_collectors();
    report_new_threads();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::get_hostname
//       Access: Private
//  Description: Returns the current machine's hostname.
////////////////////////////////////////////////////////////////////
string PStatClientImpl::
get_hostname() {
  if (_hostname.empty()) {
    char temp_buff[1024];
    if (gethostname(temp_buff, 1024) == 0) {
      _hostname = temp_buff;
    } else {
      _hostname = "unknown";
    }
  }
  return _hostname;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::send_hello
//       Access: Private
//  Description: Sends the initial greeting message to the server.
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
send_hello() {
  nassertv(_is_connected);

  PStatClientControlMessage message;
  message._type = PStatClientControlMessage::T_hello;
  message._client_hostname = get_hostname();
  message._client_progname = _client_name;
  message._major_version = get_current_pstat_major_version();
  message._minor_version = get_current_pstat_minor_version();

  Datagram datagram;
  message.encode(datagram);
  _writer.send(datagram, _tcp_connection);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::report_new_collectors
//       Access: Private
//  Description: Sends over any information about new Collectors that
//               the user code might have recently created.
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
report_new_collectors() {
  nassertv(_is_connected);

  if (_collectors_reported < (int)_client->_collectors.size()) {
    // Empirically, we determined that you can't send more than about
    // 1400 collectors at once without exceeding the 64K limit on a
    // single datagram.  So we limit ourselves here to sending only
    // half that many.
    static const int max_collectors_at_once = 700;

    while (_collectors_reported < (int)_client->_collectors.size()) {
      PStatClientControlMessage message;
      message._type = PStatClientControlMessage::T_define_collectors;
      int i = 0;
      while (_collectors_reported < (int)_client->_collectors.size() &&
             i < max_collectors_at_once) {
        message._collectors.push_back(_client->get_collector_def(_collectors_reported));
        _collectors_reported++;
        i++;
      }

      Datagram datagram;
      message.encode(datagram);
      _writer.send(datagram, _tcp_connection);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::report_new_threads
//       Access: Private
//  Description: Sends over any information about new Threads that
//               the user code might have recently created.
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
report_new_threads() {
  nassertv(_is_connected);

  if (_threads_reported < (int)_client->_threads.size()) {
    PStatClientControlMessage message;
    message._type = PStatClientControlMessage::T_define_threads;
    message._first_thread_index = _threads_reported;
    while (_threads_reported < (int)_client->_threads.size()) {
      message._names.push_back(_client->_threads[_threads_reported]._name);
      _threads_reported++;
    }

    Datagram datagram;
    message.encode(datagram);
    _writer.send(datagram, _tcp_connection);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::handle_server_control_message
//       Access: Private
//  Description: Called when a control message has been received by
//               the server over the TCP connection.
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
handle_server_control_message(const PStatServerControlMessage &message) {
  switch (message._type) {
  case PStatServerControlMessage::T_hello:
    pstats_cat.info()
      << "Connected to " << message._server_progname << " on "
      << message._server_hostname << "\n";

    _server.set_port(message._udp_port);
    _got_udp_port = true;
    break;

  default:
    pstats_cat.error()
      << "Invalid control message received from server.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientImpl::connection_reset
//       Access: Private, Virtual
//  Description: Called by the internal net code when the connection
//               has been lost.
////////////////////////////////////////////////////////////////////
void PStatClientImpl::
connection_reset(const PT(Connection) &connection, PRErrorCode) {
  if (connection == _tcp_connection) {
    _client->client_disconnect();
  } else {
    pstats_cat.warning()
      << "Ignoring spurious connection_reset() message\n";
  }
}

#endif // DO_PSTATS
