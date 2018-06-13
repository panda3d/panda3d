/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClientImpl.cxx
 * @author drose
 * @date 2004-12-23
 */

#include "pStatClientImpl.h"

// This file only defines anything if DO_PSTATS is defined.
#ifdef DO_PSTATS

#include "pStatClient.h"
#include "pStatClientControlMessage.h"
#include "pStatServerControlMessage.h"
#include "pStatCollector.h"
#include "pStatThread.h"
#include "config_pstatclient.h"
#include "pStatProperties.h"
#include "cmath.h"

#include <algorithm>

#if defined(WIN32_VC) || defined(WIN64_VC)
#include <Winsock2.h>
#include <windows.h>
#endif

/**
 *
 */
PStatClientImpl::
PStatClientImpl(PStatClient *client) :
  _clock(TrueClock::get_global_ptr()),
  _delta(0.0),
  _last_frame(0.0),
  _client(client),
  _reader(this, 0),
  _writer(this, pstats_threaded_write ? 1 : 0)
{
  _writer.set_max_queue_size(pstats_max_queue_size);
  _reader.set_tcp_header_size(4);
  _writer.set_tcp_header_size(4);
  _is_connected = false;
  _got_udp_port = false;
  _collectors_reported = 0;
  _threads_reported = 0;

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
    csincos(pstats_tcp_ratio * (3.14159265f / 2.0f),
            &_udp_count_factor,
            &_tcp_count_factor);
  }
}

/**
 *
 */
PStatClientImpl::
~PStatClientImpl() {
  nassertv(!_is_connected);
}

/**
 * Called only by PStatClient::client_connect().
 */
bool PStatClientImpl::
client_connect(std::string hostname, int port) {
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
  // Make sure we're not queuing up multiple TCP sockets--we expect immediate
  // writes of our TCP datagrams.
  _tcp_connection->set_collect_tcp(false);

  _reader.add_connection(_tcp_connection);
  _is_connected = true;

  _udp_connection = open_UDP_connection();

  send_hello();

#ifdef DEBUG_THREADS
  MutexDebug::increment_pstats();
#endif // DEBUG_THREADS

  return _is_connected;
}

/**
 * Called only by PStatClient::client_disconnect().
 */
void PStatClientImpl::
client_disconnect() {
  if (_is_connected) {
#ifdef DEBUG_THREADS
    MutexDebug::decrement_pstats();
#endif // DEBUG_THREADS
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

/**
 * Called by the PStatThread interface at the beginning of every frame, for
 * each thread.  This resets the clocks for the new frame and transmits the
 * data for the previous frame.
 */
void PStatClientImpl::
new_frame(int thread_index) {
  nassertv(thread_index >= 0 && thread_index < _client->_num_threads);

  PStatClient::InternalThread *pthread = _client->get_thread_ptr(thread_index);

  // If we're the main thread, we should exchange control packets with the
  // server.
  if (thread_index == 0) {
    transmit_control_data();
  }

  // If we've got the UDP port by the time the frame starts, it's time to
  // become active and start actually tracking data.
  if (_got_udp_port) {
    pthread->_is_active = true;
  }

  if (!pthread->_is_active) {
    return;
  }

  double frame_start = get_real_time();
  int frame_number = -1;
  PStatFrameData frame_data;

  if (!pthread->_frame_data.is_empty()) {
    // Collector 0 is the whole frame.
    _client->stop(0, thread_index, frame_start);

    // Fill up the level data for all the collectors who have level data for
    // this pthread.
    int num_collectors = _client->_num_collectors;
    PStatClient::CollectorPointer *collectors =
      (PStatClient::CollectorPointer *)_client->_collectors;
    for (int i = 0; i < num_collectors; i++) {
      const PStatClient::PerThreadData &ptd =
        collectors[i]->_per_thread[thread_index];
      if (ptd._has_level) {
        pthread->_frame_data.add_level(i, ptd._level);
      }
    }
    pthread->_frame_data.swap(frame_data);
    frame_number = pthread->_frame_number;
  }

  pthread->_frame_data.clear();
  pthread->_frame_number++;
  _client->start(0, thread_index, frame_start);

  // Also record the time for the PStats operation itself.
  int current_thread_index = Thread::get_current_thread()->get_pstats_index();
  int pstats_index = PStatClient::_pstats_pcollector.get_index();
  _client->start(pstats_index, current_thread_index, frame_start);

  if (frame_number != -1) {
    transmit_frame_data(thread_index, frame_number, frame_data);
  }
  _client->stop(pstats_index, current_thread_index, get_real_time());
}

/**
 * Slightly lower-level interface than new_frame that takes a set of frame
 * data.
 */
void PStatClientImpl::
add_frame(int thread_index, const PStatFrameData &frame_data) {
  nassertv(thread_index >= 0 && thread_index < _client->_num_threads);

  PStatClient::InternalThread *pthread = _client->get_thread_ptr(thread_index);

  // If we're the main thread, we should exchange control packets with the
  // server.
  if (thread_index == 0) {
    transmit_control_data();
  }

  // If we've got the UDP port by the time the frame starts, it's time to
  // become active and start actually tracking data.
  if (_got_udp_port) {
    pthread->_is_active = true;
  }

  if (!pthread->_is_active) {
    return;
  }

  int frame_number = pthread->_frame_number++;

  // Also record the time for the PStats operation itself.
  int current_thread_index = Thread::get_current_thread()->get_pstats_index();
  int pstats_index = PStatClient::_pstats_pcollector.get_index();
  _client->start(pstats_index, current_thread_index);

  if (frame_number != -1) {
    transmit_frame_data(thread_index, frame_number, frame_data);
  }
  _client->stop(pstats_index, current_thread_index);
}

/**
 * Should be called once per frame per thread to transmit the latest data to
 * the PStatServer.
 */
void PStatClientImpl::
transmit_frame_data(int thread_index, int frame_number,
                    const PStatFrameData &frame_data) {
  nassertv(thread_index >= 0 && thread_index < _client->_num_threads);
  PStatClient::InternalThread *thread = _client->get_thread_ptr(thread_index);
  if (_is_connected && thread->_is_active) {

    // We don't want to send too many packets in a hurry and flood the server.
    // Check that enough time has elapsed for us to send a new packet.  If
    // not, we'll drop this packet on the floor and send a new one next time
    // around.
    double now = get_real_time();
    if (now >= thread->_next_packet) {
      // We don't want to send more than _max_rate UDP-size packets per
      // second, per thread.
      double packet_delay = 1.0 / _max_rate;

      // Send new data.
      NetDatagram datagram;
      // We always start with a zero byte, to differentiate it from a control
      // message.
      datagram.add_uint8(0);

      datagram.add_uint16(thread_index);
      datagram.add_uint32(frame_number);

      bool sent;

      if (!frame_data.write_datagram(datagram, _client)) {
        // Too many events to fit in a single datagram.  Maybe it was a long
        // frame load or something.  Just drop the datagram.
        sent = false;

      } else if (_writer.is_valid_for_udp(datagram)) {
        if (_udp_count * _udp_count_factor < _tcp_count * _tcp_count_factor) {
          // Send this one as a UDP packet.
          nassertv(_got_udp_port);
          sent = _writer.send(datagram, _udp_connection, _server);
          _udp_count++;

          if (_udp_count == 0) {
            // Wraparound!
            _udp_count = 1;
            _tcp_count = 1;
          }

        } else {
          // Send this one as a TCP packet.
          sent = _writer.send(datagram, _tcp_connection);
          _tcp_count++;

          if (_tcp_count == 0) {
            // Wraparound!
            _udp_count = 1;
            _tcp_count = 1;
          }
        }

      } else {
        sent = _writer.send(datagram, _tcp_connection);
        // If our packets are so large that we must ship them via TCP, then
        // artificially slow down the packet rate even further.
        int packet_ratio =
          (datagram.get_length() + maximum_udp_datagram - 1) /
          maximum_udp_datagram;
        packet_delay *= (double)packet_ratio;
      }

      thread->_next_packet = now + packet_delay;

      if (!sent) {
        if (pstats_cat.is_debug()) {
          pstats_cat.debug()
            << "Couldn't send packet.\n";
        }
      }
    }
  }
}

/**
 * Should be called once a frame to exchange control information with the
 * server.
 */
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


/**
 * Returns the current machine's hostname.
 */
std::string PStatClientImpl::
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

/**
 * Sends the initial greeting message to the server.
 */
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
  _writer.send(datagram, _tcp_connection, true);
}

/**
 * Sends over any information about new Collectors that the user code might
 * have recently created.
 */
void PStatClientImpl::
report_new_collectors() {
  // Empirically, we determined that you can't send more than about 1400
  // collectors at once without exceeding the 64K limit on a single datagram.
  // So we limit ourselves here to sending only half that many.
  static const int max_collectors_at_once = 700;

  while (_is_connected && _collectors_reported < _client->_num_collectors) {
    PStatClientControlMessage message;
    message._type = PStatClientControlMessage::T_define_collectors;
    int i = 0;
    while (_collectors_reported < _client->_num_collectors &&
           i < max_collectors_at_once) {
      message._collectors.push_back(_client->get_collector_def(_collectors_reported));
      _collectors_reported++;
      i++;
    }

    Datagram datagram;
    message.encode(datagram);
    _writer.send(datagram, _tcp_connection, true);
  }
}

/**
 * Sends over any information about new Threads that the user code might have
 * recently created.
 */
void PStatClientImpl::
report_new_threads() {
  while (_is_connected && _threads_reported < _client->_num_threads) {
    PStatClientControlMessage message;
    message._type = PStatClientControlMessage::T_define_threads;
    message._first_thread_index = _threads_reported;
    PStatClient::ThreadPointer *threads =
      (PStatClient::ThreadPointer *)_client->_threads;
    while (_threads_reported < _client->_num_threads) {
      message._names.push_back(threads[_threads_reported]->_name);
      _threads_reported++;
    }

    Datagram datagram;
    message.encode(datagram);
    _writer.send(datagram, _tcp_connection, true);
  }
}

/**
 * Called when a control message has been received by the server over the TCP
 * connection.
 */
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

/**
 * Called by the internal net code when the connection has been lost.
 */
void PStatClientImpl::
connection_reset(const PT(Connection) &connection, bool) {
  if (connection == _tcp_connection) {
    client_disconnect();
  } else if (connection == _udp_connection) {
    pstats_cat.warning()
      << "Trouble sending UDP; switching to TCP only.\n";
    _tcp_count_factor = 0.0f;
    _udp_count_factor = 1.0f;
  } else {
    pstats_cat.warning()
      << "Ignoring spurious connection_reset() message\n";
  }
}

#endif // DO_PSTATS
