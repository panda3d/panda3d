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
#include "conditionVarWin32Impl.h"
#include "conditionVarPosixImpl.h"
#include "genericThread.h"
#include "mutexHolder.h"

#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

static PStatCollector _cswitch_pcollector("Context Switches");
static PStatCollector _cswitch_sleep_pcollector("Context Switches:Sleep");
static PStatCollector _cswitch_yield_pcollector("Context Switches:Yields");
static PStatCollector _cswitch_cvar_pcollector("Context Switches:Condition Variable");
static PStatCollector _cswitch_involuntary_pcollector("Context Switches:Involuntary");
static PStatCollector _wait_sleep_pcollector("Wait:Sleep");
static PStatCollector _wait_yield_pcollector("Wait:Yield");
static PStatCollector _wait_cvar_pcollector("Wait:Condition Variable");

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
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  _writer(this, 0),
  _thread_lock("PStatsClientImpl::_thread_lock"),
  _thread_cvar(_thread_lock)
#else
  _writer(this, pstats_threaded_write ? 1 : 0)
#endif
{
#if !defined(HAVE_THREADS) || defined(SIMPLE_THREADS)
  _writer.set_max_queue_size(pstats_max_queue_size);
#endif
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

  if (pstats_thread_profiling) {
    // Replace the condition variable wait function with one that does PStats
    // statistics.
    _thread_profiling = true;

    Thread::_sleep_func = [] (double seconds) {
      Thread *current_thread = Thread::get_current_thread();
      int thread_index = current_thread->get_pstats_index();
      if (thread_index >= 0) {
        PStatClient *client = PStatClient::get_global_pstats();
        client->start(_wait_sleep_pcollector.get_index(), thread_index);
        ThreadImpl::sleep(seconds);
        client->stop(_wait_sleep_pcollector.get_index(), thread_index);
        client->add_level(_cswitch_sleep_pcollector.get_index(), thread_index, 1);
      }
      else {
        ThreadImpl::sleep(seconds);
      }
    };

    Thread::_yield_func = [] () {
      Thread *current_thread = Thread::get_current_thread();
      int thread_index = current_thread->get_pstats_index();
      if (thread_index >= 0) {
        PStatClient *client = PStatClient::get_global_pstats();
        client->start(_wait_yield_pcollector.get_index(), thread_index);
        ThreadImpl::yield();
        client->stop(_wait_yield_pcollector.get_index(), thread_index);
        client->add_level(_cswitch_yield_pcollector.get_index(), thread_index, 1);
      }
      else {
        ThreadImpl::yield();
      }
    };

#ifdef _WIN32
    ConditionVarWin32Impl::_wait_func =
      [] (PCONDITION_VARIABLE cvar, PSRWLOCK lock, DWORD time, ULONG flags) {
        Thread *current_thread = Thread::get_current_thread();
        int thread_index = current_thread->get_pstats_index();
        BOOL result;
        if (thread_index >= 0) {
          PStatClient *client = PStatClient::get_global_pstats();
          client->start(_wait_cvar_pcollector.get_index(), thread_index);
          result = SleepConditionVariableSRW(cvar, lock, time, flags);
          client->stop(_wait_cvar_pcollector.get_index(), thread_index);
          client->add_level(_cswitch_cvar_pcollector.get_index(), thread_index, 1);
        }
        else {
          result = SleepConditionVariableSRW(cvar, lock, time, flags);
        }
        return result;
      };
#endif

#ifdef HAVE_POSIX_THREADS
    ConditionVarPosixImpl::_wait_func =
      [] (pthread_cond_t *cvar, pthread_mutex_t *lock) {
        Thread *current_thread = Thread::get_current_thread();
        int thread_index = current_thread->get_pstats_index();
        int result;
        if (thread_index >= 0) {
          PStatClient *client = PStatClient::get_global_pstats();
          client->start(_wait_cvar_pcollector.get_index(), thread_index);
          result = pthread_cond_wait(cvar, lock);
          client->stop(_wait_cvar_pcollector.get_index(), thread_index);
          client->add_level(_cswitch_cvar_pcollector.get_index(), thread_index, 1);
        }
        else {
          result = pthread_cond_wait(cvar, lock);
        }
        return result;
      };

    ConditionVarPosixImpl::_timedwait_func =
      [] (pthread_cond_t *cvar, pthread_mutex_t *lock,
          const struct timespec *ts) {
        Thread *current_thread = Thread::get_current_thread();
        int thread_index = current_thread->get_pstats_index();
        int result;
        if (thread_index >= 0) {
          PStatClient *client = PStatClient::get_global_pstats();
          client->start(_wait_cvar_pcollector.get_index(), thread_index);
          result = pthread_cond_timedwait(cvar, lock, ts);
          client->stop(_wait_cvar_pcollector.get_index(), thread_index);
          client->add_level(_cswitch_cvar_pcollector.get_index(), thread_index, 1);
        }
        else {
          result = pthread_cond_timedwait(cvar, lock, ts);
        }
        return result;
      };
#endif
  }

  // Wait for the server hello.
  while (!_got_udp_port) {
    transmit_control_data();
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  if (_is_connected && pstats_threaded_write) {
    _thread = new GenericThread("PStats", "PStats", [this]() {
      this->thread_main();
    });
    if (!_thread->start(TP_low, false)) {
      _thread.clear();
    }
  }
#endif

  return _is_connected;
}

/**
 * Called only by PStatClient::client_disconnect().
 */
void PStatClientImpl::
client_disconnect() {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  // Tell the thread to shut itself down.  Note that this may be called from
  // the thread itself, so we shouldn't try to call join().
  _thread_lock.lock();
  if (_thread != nullptr) {
    _thread_should_shutdown = true;
    _thread_cvar.notify();
  }
  _thread_lock.unlock();
#endif

  if (_thread_profiling) {
    // Switch the functions back to what they were.
    Thread::_sleep_func = &ThreadImpl::sleep;
    Thread::_yield_func = &ThreadImpl::yield;

#ifdef _WIN32
    ConditionVarWin32Impl::_wait_func = &SleepConditionVariableSRW;
#endif

#ifdef HAVE_POSIX_THREADS
    ConditionVarPosixImpl::_wait_func = &pthread_cond_wait;
    ConditionVarPosixImpl::_timedwait_func = &pthread_cond_timedwait;
#endif

    _thread_profiling = false;
  }

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
new_frame(int thread_index, int frame_number) {
  double frame_start = get_real_time();

  nassertv(thread_index >= 0 && thread_index < _client->_num_threads);

  PStatClient::InternalThread *pthread = _client->get_thread_ptr(thread_index);
  nassertv(pthread != nullptr);

  // If we're the main thread, we should exchange control packets with the
  // server.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  if (thread_index == 0 && _thread == nullptr) {
#else
  if (thread_index == 0) {
#endif
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
    if (frame_number == -1) {
      frame_number = pthread->_frame_number;
    }

    // Record the number of context switches on this thread.
    if (_thread_profiling) {
      size_t total, involuntary;
      PT(Thread) thread = pthread->_thread.lock();
      if (thread != nullptr && thread->get_context_switches(total, involuntary)) {
        size_t total_this_frame = total - pthread->_context_switches;
        pthread->_context_switches = total;
        frame_data.add_level(_cswitch_pcollector.get_index(), total_this_frame);

        if (involuntary != 0) {
          size_t involuntary_this_frame = involuntary - pthread->_involuntary_context_switches;
          pthread->_involuntary_context_switches = involuntary;
          frame_data.add_level(_cswitch_involuntary_pcollector.get_index(), involuntary_this_frame);
        }
      }
      _client->clear_level(_cswitch_sleep_pcollector.get_index(), thread_index);
      _client->clear_level(_cswitch_yield_pcollector.get_index(), thread_index);
      _client->clear_level(_cswitch_cvar_pcollector.get_index(), thread_index);
    }
  }
  else if (_thread_profiling) {
    // Record the initial number of context switches.
    PT(Thread) thread = pthread->_thread.lock();
    if (thread != nullptr) {
      thread->get_context_switches(pthread->_context_switches,
                                   pthread->_involuntary_context_switches);
    }
    _client->clear_level(_cswitch_sleep_pcollector.get_index(), thread_index);
    _client->clear_level(_cswitch_yield_pcollector.get_index(), thread_index);
    _client->clear_level(_cswitch_cvar_pcollector.get_index(), thread_index);
  }

  pthread->_frame_data.clear();
  pthread->_frame_number = frame_number + 1;
  _client->start(0, thread_index, frame_start);

  // Also record the time for the PStats operation itself.
  int current_thread_index = Thread::get_current_thread()->get_pstats_index();
  int pstats_index = PStatClient::_pstats_pcollector.get_index();
  _client->start(pstats_index, current_thread_index, frame_start);

  if (!frame_data.is_empty()) {
    enqueue_frame_data(thread_index, frame_number, std::move(frame_data));
  }
  _client->stop(pstats_index, current_thread_index, get_real_time());
}

/**
 * Slightly lower-level interface than new_frame that takes a set of frame
 * data.
 */
void PStatClientImpl::
add_frame(int thread_index, int frame_number, PStatFrameData &&frame_data) {
  nassertv(thread_index >= 0 && thread_index < _client->_num_threads);

  PStatClient::InternalThread *pthread = _client->get_thread_ptr(thread_index);
  nassertv(pthread != nullptr);

  // If we're the main thread, we should exchange control packets with the
  // server.
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  if (thread_index == 0 && _thread == nullptr) {
#else
  if (thread_index == 0) {
#endif
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

  // Also record the time for the PStats operation itself.
  int current_thread_index = Thread::get_current_thread()->get_pstats_index();
  int pstats_index = PStatClient::_pstats_pcollector.get_index();
  _client->start(pstats_index, current_thread_index);

  enqueue_frame_data(thread_index, frame_number, std::move(frame_data));
  _client->stop(pstats_index, current_thread_index);
}

/**
 * Removes a thread from PStats.
 */
void PStatClientImpl::
remove_thread(int thread_index) {
  nassertv(thread_index >= 0 && thread_index < _client->_num_threads);

  PStatClientControlMessage message;
  message._type = PStatClientControlMessage::T_expire_thread;
  message._first_thread_index = thread_index;

  Datagram datagram;
  message.encode(datagram);
  _writer.send(datagram, _tcp_connection, true);
}

/**
 * Passes off the frame data to the writer thread.  If threading is disabled,
 * transmits it right away.
 */
void PStatClientImpl::
enqueue_frame_data(int thread_index, int frame_number,
                   PStatFrameData &&frame_data) {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  if (_thread != nullptr) {
    int max_size = pstats_max_queue_size;
    _thread_lock.lock();
    if (max_size < 0 || _frame_queue.size() < (size_t)max_size) {
      _frame_queue.emplace_back(thread_index, frame_number);
      frame_data.swap(_frame_queue.back()._frame_data);
    }
    _thread_cvar.notify();
    _thread_lock.unlock();
    return;
  }
#endif

  // We don't have a thread, so transmit it directly.
  if (_is_connected) {
    transmit_frame_data(thread_index, frame_number, frame_data);
  }
}

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
/**
 *
 */
void PStatClientImpl::
thread_main() {
  MutexHolder holder(_thread_lock);
  transmit_control_data();

  while (!_thread_should_shutdown) {
    while (_frame_queue.empty() && !_thread_should_shutdown) {
      _thread_cvar.wait();
    }

    while (!_frame_queue.empty()) {
      // Dequeue up to 8 at a time, to decrease the amount of times we need to
      // hold the lock.
      QueuedFrame frames[8];
      int num_frames = 0;

      while (!_frame_queue.empty() && num_frames < 8) {
        QueuedFrame &qf = _frame_queue.front();
        frames[num_frames]._thread_index = qf._thread_index;
        frames[num_frames]._frame_number = qf._frame_number;
        frames[num_frames]._frame_data.swap(qf._frame_data);
        ++num_frames;
        _frame_queue.pop_front();
      }
      _thread_lock.unlock();

      transmit_control_data();

      if (num_frames > 0) {
        for (int i = 0; i < num_frames; ++i) {
          QueuedFrame &qf = frames[i];
          transmit_frame_data(qf._thread_index, qf._frame_number, qf._frame_data);
        }
      }

      _thread_lock.lock();
    }
  }

  _thread = nullptr;
}
#endif

/**
 * Should be called once per frame per thread to transmit the latest data to
 * the PStatServer.
 */
void PStatClientImpl::
transmit_frame_data(int thread_index, int frame_number,
                    const PStatFrameData &frame_data) {
  nassertv(thread_index >= 0 && thread_index < _client->_num_threads);
  PStatClient::InternalThread *thread = _client->get_thread_ptr(thread_index);
  nassertv(thread != nullptr);

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
#ifdef _WIN32
  message._client_pid = GetCurrentProcessId();
#else
  message._client_pid = getpid();
#endif
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
      if (threads[_threads_reported] != nullptr) {
        message._names.push_back(threads[_threads_reported]->_name);
      } else {
        message._names.push_back(std::string());
      }
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
