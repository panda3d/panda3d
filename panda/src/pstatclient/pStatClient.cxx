// Filename: pStatClient.cxx
// Created by:  drose (09Jul00)
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

#include "pStatClient.h"

#ifdef DO_PSTATS
// This file only defines anything interesting if DO_PSTATS is
// defined.

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
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN
#endif

PStatClient *PStatClient::_global_pstats = NULL;

PStatCollector _total_size_pcollector("Memory usage");
PStatCollector _cpp_size_pcollector("Memory usage:C++");
PStatCollector _interpreter_size_pcollector("Memory usage:Interpreter");
PStatCollector _pstats_pcollector("App:PStats");

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::PerThreadData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatClient::PerThreadData::
PerThreadData() {
  _has_level = false;
  _level = 0.0;
  _nested_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatClient::
PStatClient() :
  _reader(this, 0),
  _writer(this, get_pstats_threaded_write() ? 1 : 0)
{
  _is_connected = false;
  _got_udp_port = false;
  _collectors_reported = 0;
  _threads_reported = 0;

  // Make sure our clock is in "normal" mode.
  _clock.set_mode(ClockObject::M_normal);

  // We always have a collector at index 0 named "Frame".  This tracks
  // the total frame time and is the root of all other collectors.  We
  // have to make this one by hand since it's the root.
  Collector collector;
  collector._def = new PStatCollectorDef(0, "Frame");
  collector._def->_parent_index = 0;
  collector._def->_suggested_color.set(0.5, 0.5, 0.5);
  _collectors.push_back(collector);

  // We also always have a thread at index 0 named "Main".
  make_thread("Main");

  _client_name = get_pstats_name();
  _max_rate = get_pstats_max_rate();

  _tcp_count = 1;
  _udp_count = 1;

  double pstats_tcp_ratio = get_pstats_tcp_ratio();

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
//     Function: PStatClient::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatClient::
~PStatClient() {
  disconnect();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_num_collectors
//       Access: Public
//  Description: Returns the total number of collectors the Client
//               knows about.
////////////////////////////////////////////////////////////////////
int PStatClient::
get_num_collectors() const {
  return _collectors.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector
//       Access: Public
//  Description: Returns the nth collector.
////////////////////////////////////////////////////////////////////
PStatCollector PStatClient::
get_collector(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), PStatCollector());
  return PStatCollector((PStatClient *)this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_def
//       Access: Public
//  Description: Returns the definition body of the nth collector.
////////////////////////////////////////////////////////////////////
const PStatCollectorDef &PStatClient::
get_collector_def(int index) const {
#ifndef NDEBUG
  static PStatCollectorDef bogus;
  nassertr(index >= 0 && index < (int)_collectors.size(), bogus);
#endif

  return *_collectors[index]._def;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_name
//       Access: Public
//  Description: Returns the name of the indicated collector.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_collector_name(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), string());

  const PStatCollectorDef *def = _collectors[index]._def;
  return def->_name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_fullname
//       Access: Public
//  Description: Returns the "full name" of the indicated collector.
//               This will be the concatenation of all of the
//               collector's parents' names (except Frame) and the
//               collector's own name.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_collector_fullname(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), string());

  const PStatCollectorDef *def = _collectors[index]._def;
  if (def->_parent_index == 0) {
    return def->_name;
  } else {
    return get_collector_fullname(def->_parent_index) + ":" + def->_name;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_num_threads
//       Access: Public
//  Description: Returns the total number of threads the Client
//               knows about.
////////////////////////////////////////////////////////////////////
int PStatClient::
get_num_threads() const {
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_thread
//       Access: Public
//  Description: Returns the nth thread.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_thread(int index) const {
  nassertr(index >= 0 && index < (int)_threads.size(), PStatThread());
  return PStatThread((PStatClient *)this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_thread_name
//       Access: Public
//  Description: Returns the name of the indicated thread.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_thread_name(int index) const {
  nassertr(index >= 0 && index < (int)_threads.size(), string());
  return _threads[index]._name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_clock
//       Access: Public
//  Description: Returns a reference to the PStatClient's clock
//               object.  It keeps its own clock, instead of using the
//               global clock object, so the stats won't get mucked up
//               if you put the global clock in non-real-time mode or
//               something.
//
//               On second thought, it works better to use the global
//               clock, so we don't lose a lot of time in the stats
//               while we're waiting at the prompt.
////////////////////////////////////////////////////////////////////
const ClockObject &PStatClient::
get_clock() const {
  return _clock;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_main_thread
//       Access: Public
//  Description: Returns a handle to the client's "Main", or default,
//               thread.  This is where collectors will be started and
//               stopped if they don't specify otherwise.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_main_thread() const {
  return PStatThread((PStatClient *)this, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::make_collector_with_relname
//       Access: Private
//  Description: Returns a PStatCollector suitable for measuring
//               categories with the indicated name.  This is normally
//               called by a PStatCollector constructor.
//
//               The name may contain colons; if it does, it specifies
//               a relative path to the client indicated by the parent
//               index.
////////////////////////////////////////////////////////////////////
PStatCollector PStatClient::
make_collector_with_relname(int parent_index, string relname) {
  if (relname.empty()) {
    relname = "Unnamed";
  }

  // Skip any colons at the beginning of the name.
  size_t start = 0;
  while (start < relname.size() && relname[start] == ':') {
    start++;
  }

  // If the name contains a colon (after the initial colon), it means
  // we are making a nested collector.
  size_t colon = relname.find(':', start);
  while (colon != string::npos) {
    string parent_name = relname.substr(start, colon - start);
    PStatCollector parent_collector =
      make_collector_with_name(parent_index, parent_name);
    parent_index = parent_collector._index;
    relname = relname.substr(colon + 1);
    start = 0;
    colon = relname.find(':');
  }

  string name = relname.substr(start);
  return make_collector_with_name(parent_index, name);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::make_collector_with_name
//       Access: Private
//  Description: Returns a PStatCollector suitable for measuring
//               categories with the indicated name.  This is normally
//               called by a PStatCollector constructor.
//
//               The name should not contain colons.
////////////////////////////////////////////////////////////////////
PStatCollector PStatClient::
make_collector_with_name(int parent_index, const string &name) {
  nassertr(parent_index >= 0 && parent_index < (int)_collectors.size(),
           PStatCollector());

  Collector &parent = _collectors[parent_index];

  // A special case: if we asked for a child the same name as its
  // parent, we really meant the parent.  That is, "Frame:Frame" is
  // really the same collector as "Frame".
  if (parent._def->_name == name) {
    return PStatCollector(this, parent_index);
  }

  ThingsByName::const_iterator ni = parent._children.find(name);

  if (ni != parent._children.end()) {
    // We already had a collector by this name; return it.
    int index = (*ni).second;
    nassertr(index >= 0 && index < (int)_collectors.size(), PStatCollector());
    return PStatCollector(this, (*ni).second);
  }

  // Create a new collector for this name.
  int new_index = _collectors.size();
  parent._children.insert(ThingsByName::value_type(name, new_index));

  // Extending the vector invalidates the parent reference, above.
  _collectors.push_back(Collector());
  Collector &collector = _collectors.back();
  collector._def = new PStatCollectorDef(new_index, name);

  collector._def->set_parent(*_collectors[parent_index]._def);
  initialize_collector_def(this, collector._def);

  // We need one PerThreadData for each thread.
  while (collector._per_thread.size() < _threads.size()) {
    collector._per_thread.push_back(PerThreadData());
  }

  return PStatCollector(this, new_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::make_thread
//       Access: Private
//  Description: Returns a PStatThread with the indicated name
//               suitable for grouping collectors.  This is normally
//               called by a PStatThread constructor.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
make_thread(const string &name) {
  ThingsByName::const_iterator ni =
    _threads_by_name.find(name);

  if (ni != _threads_by_name.end()) {
    // We already had a thread by this name; return it.
    int index = (*ni).second;
    nassertr(index >= 0 && index < (int)_threads.size(), PStatThread());
    return PStatThread(this, (*ni).second);
  }

  // Create a new thread for this name.
  int new_index = _threads.size();
  _threads_by_name.insert(ThingsByName::value_type(name, new_index));

  Thread thread;
  thread._name = name;
  thread._is_active = false;
  thread._next_packet = 0.0;
  thread._frame_number = 0;

  _threads.push_back(thread);

  // We need an additional PerThreadData for this thread in all of the
  // collectors.
  Collectors::iterator ci;
  for (ci = _collectors.begin(); ci != _collectors.end(); ++ci) {
    (*ci)._per_thread.push_back(PerThreadData());
    nassertr((*ci)._per_thread.size() == _threads.size(), PStatThread());
  }

  return PStatThread(this, new_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::main_tick
//       Access: Public, Static
//  Description: A convenience function to call new_frame() on the
//               global PStatClient's main thread.
////////////////////////////////////////////////////////////////////
void PStatClient::
main_tick() {
  // We have code here to report the memory usage.  We can't put this
  // code inside the MemoryUsage class, where it fits a little better,
  // simply because MemoryUsage is a very low-level class that doesn't
  // know about PStatClient.

#ifdef DO_MEMORY_USAGE
  if (MemoryUsage::has_total_size()) {
    _total_size_pcollector.set_level(MemoryUsage::get_total_size());
  }
  if (MemoryUsage::has_cpp_size()) {
    _cpp_size_pcollector.set_level(MemoryUsage::get_cpp_size());
  }
  if (MemoryUsage::has_interpreter_size()) {
    _interpreter_size_pcollector.set_level(MemoryUsage::get_interpreter_size());
  }
#endif

  get_global_pstats()->client_main_tick();
}  

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::main_tick
//       Access: Public, Static
//  Description: A convenience function to call new_frame() on the
//               the given client's main thread.
////////////////////////////////////////////////////////////////////
void PStatClient::
client_main_tick() {
  _clock.tick();
  get_main_thread().new_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_global_pstats
//       Access: Public, Static
//  Description: Returns a pointer to the global PStatClient object.
//               It's legal to declare your own PStatClient locally,
//               but it's also convenient to have a global one that
//               everyone can register with.  This is the global one.
////////////////////////////////////////////////////////////////////
PStatClient *PStatClient::
get_global_pstats() {
  if (_global_pstats == (PStatClient *)NULL) {
    _global_pstats = new PStatClient;
  }
  return _global_pstats;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_connect
//       Access: Private
//  Description: The nonstatic implementation of connect().
////////////////////////////////////////////////////////////////////
bool PStatClient::
client_connect(string hostname, int port) {
  client_disconnect();

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
//     Function: PStatClient::client_disconnect
//       Access: Private
//  Description: The nonstatic implementation of disconnect().
////////////////////////////////////////////////////////////////////
void PStatClient::
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

  Threads::iterator ti;
  for (ti = _threads.begin(); ti != _threads.end(); ++ti) {
    (*ti)._frame_number = 0;
    (*ti)._is_active = false;
    (*ti)._next_packet = 0.0;
    (*ti)._frame_data.clear();
  }

  Collectors::iterator ci;
  for (ci = _collectors.begin(); ci != _collectors.end(); ++ci) {
    PerThread::iterator ii;
    for (ii = (*ci)._per_thread.begin();
         ii != (*ci)._per_thread.end();
         ++ii) {
      (*ii)._nested_count = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_is_connected
//       Access: Public
//  Description: The nonstatic implementation of is_connected().
////////////////////////////////////////////////////////////////////
bool PStatClient::
client_is_connected() const {
  return _is_connected;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_resume_after_pause
//       Access: Published
//  Description: Resumes the PStatClient after the simulation has been
//               paused for a while.  This allows the stats to
//               continue exactly where it left off, instead of
//               leaving a big gap that would represent a chug.
////////////////////////////////////////////////////////////////////
void PStatClient::
client_resume_after_pause() {
  // Simply reset the clock to the beginning of the last frame.  This
  // may lose a frame, but on the other hand we won't skip a whole
  // slew of frames either.

  double frame_time = _clock.get_frame_time();
  _clock.set_real_time(frame_time);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::is_active
//       Access: Private
//  Description: Returns true if the indicated collector/thread
//               combination is active, and we are transmitting stats
//               data, or false otherwise.
//
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::is_active().
////////////////////////////////////////////////////////////////////
bool PStatClient::
is_active(int collector_index, int thread_index) const {
  nassertr(collector_index >= 0 && collector_index < (int)_collectors.size(), false);
  nassertr(thread_index >= 0 && thread_index < (int)_threads.size(), false);

  return (_is_connected &&
          _collectors[collector_index]._def->_is_active &&
          _threads[thread_index]._is_active);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::is_started
//       Access: Private
//  Description: Returns true if the indicated collector/thread
//               combination has been started, or false otherwise.
//
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::is_started().
////////////////////////////////////////////////////////////////////
bool PStatClient::
is_started(int collector_index, int thread_index) const {
  nassertr(collector_index >= 0 && collector_index < (int)_collectors.size(), false);
  nassertr(thread_index >= 0 && thread_index < (int)_threads.size(), false);

  return (_collectors[collector_index]._def->_is_active &&
          _threads[thread_index]._is_active &&
          _collectors[collector_index]._per_thread[thread_index]._nested_count != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::start
//       Access: Private
//  Description: Marks the indicated collector index as started.
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::start().
////////////////////////////////////////////////////////////////////
void PStatClient::
start(int collector_index, int thread_index) {
#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < (int)_collectors.size());
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());
#endif

  if (_collectors[collector_index]._def->_is_active &&
      _threads[thread_index]._is_active) {
    if (_collectors[collector_index]._per_thread[thread_index]._nested_count == 0) {
      // This collector wasn't already started in this thread; record
      // a new data point.
      _threads[thread_index]._frame_data.add_start(collector_index, 
                                                   _clock.get_real_time());
    }
    _collectors[collector_index]._per_thread[thread_index]._nested_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::start
//       Access: Private
//  Description: Marks the indicated collector index as started.
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::start().
////////////////////////////////////////////////////////////////////
void PStatClient::
start(int collector_index, int thread_index, float as_of) {
#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < (int)_collectors.size());
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());
#endif

  if (_collectors[collector_index]._def->_is_active &&
      _threads[thread_index]._is_active) {
    if (_collectors[collector_index]._per_thread[thread_index]._nested_count == 0) {
      // This collector wasn't already started in this thread; record
      // a new data point.
      _threads[thread_index]._frame_data.add_start(collector_index, as_of);
    }
    _collectors[collector_index]._per_thread[thread_index]._nested_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::stop
//       Access: Private
//  Description: Marks the indicated collector index as stopped.
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::stop().
////////////////////////////////////////////////////////////////////
void PStatClient::
stop(int collector_index, int thread_index) {
#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < (int)_collectors.size());
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());
#endif

  if (_collectors[collector_index]._def->_is_active &&
      _threads[thread_index]._is_active) {
    if (_collectors[collector_index]._per_thread[thread_index]._nested_count == 0) {
      pstats_cat.warning()
        << "Collector " << get_collector_fullname(collector_index)
        << " was already stopped in thread " << get_thread_name(thread_index)
        << "!\n";
      return;
    }

    _collectors[collector_index]._per_thread[thread_index]._nested_count--;

    if (_collectors[collector_index]._per_thread[thread_index]._nested_count == 0) {
      // This collector has now been completely stopped; record a new
      // data point.
      _threads[thread_index]._frame_data.add_stop(collector_index,
                                                  _clock.get_real_time());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::stop
//       Access: Private
//  Description: Marks the indicated collector index as stopped.
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::stop().
////////////////////////////////////////////////////////////////////
void PStatClient::
stop(int collector_index, int thread_index, float as_of) {
#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < (int)_collectors.size());
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());
#endif

  if (_collectors[collector_index]._def->_is_active &&
      _threads[thread_index]._is_active) {
    if (_collectors[collector_index]._per_thread[thread_index]._nested_count == 0) {
      pstats_cat.warning()
        << "Collector " << get_collector_fullname(collector_index)
        << " was already stopped in thread " << get_thread_name(thread_index)
        << "!\n";
      return;
    }

    _collectors[collector_index]._per_thread[thread_index]._nested_count--;

    if (_collectors[collector_index]._per_thread[thread_index]._nested_count == 0) {
      // This collector has now been completely stopped; record a new
      // data point.
      _threads[thread_index]._frame_data.add_stop(collector_index, as_of);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::clear_level
//       Access: Private
//  Description: Removes the level value from the indicated collector.
//               The collector will no longer be reported as having
//               any particular level value.
//
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::clear_level().
////////////////////////////////////////////////////////////////////
void PStatClient::
clear_level(int collector_index, int thread_index) {
  if (_collectors[collector_index]._def->_is_active) {
    _collectors[collector_index]._per_thread[thread_index]._has_level = false;
    _collectors[collector_index]._per_thread[thread_index]._level = 0.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::set_level
//       Access: Private
//  Description: Sets the level value for the indicated collector to
//               the given amount.
//
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::set_level().
////////////////////////////////////////////////////////////////////
void PStatClient::
set_level(int collector_index, int thread_index, float level) {
  if (_collectors[collector_index]._def->_is_active) {
    level *= _collectors[collector_index]._def->_factor;
    _collectors[collector_index]._per_thread[thread_index]._has_level = true;
    _collectors[collector_index]._per_thread[thread_index]._level = level;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::add_level
//       Access: Private
//  Description: Adds the given value (which may be negative) to the
//               current value for the given collector.  If the
//               collector does not already have a level value, it is
//               initialized to 0.
//
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::add_level().
////////////////////////////////////////////////////////////////////
void PStatClient::
add_level(int collector_index, int thread_index, float increment) {
  if (_collectors[collector_index]._def->_is_active) {
    increment *= _collectors[collector_index]._def->_factor;
    _collectors[collector_index]._per_thread[thread_index]._has_level = true;
    _collectors[collector_index]._per_thread[thread_index]._level += increment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_level
//       Access: Private
//  Description: Returns the current level value of the given collector.
//
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::get_level().
////////////////////////////////////////////////////////////////////
float PStatClient::
get_level(int collector_index, int thread_index) const {
  return _collectors[collector_index]._per_thread[thread_index]._level /
    _collectors[collector_index]._def->_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::new_frame
//       Access: Private
//  Description: Called by the PStatThread interface at the beginning
//               of every frame, for each thread.  This resets the
//               clocks for the new frame and transmits the data for
//               the previous frame.
////////////////////////////////////////////////////////////////////
void PStatClient::
new_frame(int thread_index) {
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());

  Thread &thread = _threads[thread_index];

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
    stop(0, thread_index, frame_start);

    // Fill up the level data for all the collectors who have level
    // data for this thread.
    int num_collectors = _collectors.size();
    for (int i = 0; i < num_collectors; i++) {
      const PerThreadData &ptd = _collectors[i]._per_thread[thread_index];
      if (ptd._has_level) {
        thread._frame_data.add_level(i, ptd._level);
      }
    }
    transmit_frame_data(thread_index);
  }

  thread._frame_data.clear();
  thread._frame_number++;
  start(0, thread_index, frame_start);

  // Also record the time for the PStats operation itself.
  start(_pstats_pcollector.get_index(), thread_index, frame_start);
  stop(_pstats_pcollector.get_index(), thread_index, _clock.get_real_time());
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::transmit_frame_data
//       Access: Private
//  Description: Should be called once per frame per thread to
//               transmit the latest data to the PStatServer.
////////////////////////////////////////////////////////////////////
void PStatClient::
transmit_frame_data(int thread_index) {
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());
  if (_is_connected && _threads[thread_index]._is_active) {

    // We don't want to send too many packets in a hurry and flood the
    // server.  Check that enough time has elapsed for us to send a
    // new packet.  If not, we'll drop this packet on the floor and
    // send a new one next time around.
    float now = _clock.get_real_time();
    if (now >= _threads[thread_index]._next_packet) {
      // We don't want to send more than _max_rate UDP-size packets
      // per second, per thread.
      float packet_delay = 1.0 / _max_rate;

      // Send new data.
      NetDatagram datagram;
      // We always start with a zero byte, to differentiate it from a
      // control message.
      datagram.add_uint8(0);

      datagram.add_uint16(thread_index);
      datagram.add_uint32(_threads[thread_index]._frame_number);
      _threads[thread_index]._frame_data.write_datagram(datagram);

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

      _threads[thread_index]._next_packet = now + packet_delay;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::transmit_control_data
//       Access: Private
//  Description: Should be called once a frame to exchange control
//               information with the server.
////////////////////////////////////////////////////////////////////
void PStatClient::
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
//     Function: PStatClient::get_hostname
//       Access: Private
//  Description: Returns the current machine's hostname.
////////////////////////////////////////////////////////////////////
string PStatClient::
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
//     Function: PStatClient::send_hello
//       Access: Private
//  Description: Sends the initial greeting message to the server.
////////////////////////////////////////////////////////////////////
void PStatClient::
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
//     Function: PStatClient::report_new_collectors
//       Access: Private
//  Description: Sends over any information about new Collectors that
//               the user code might have recently created.
////////////////////////////////////////////////////////////////////
void PStatClient::
report_new_collectors() {
  nassertv(_is_connected);

  if (_collectors_reported < (int)_collectors.size()) {
    PStatClientControlMessage message;
    message._type = PStatClientControlMessage::T_define_collectors;
    while (_collectors_reported < (int)_collectors.size()) {
      message._collectors.push_back(_collectors[_collectors_reported]._def);
      _collectors_reported++;
    }

    Datagram datagram;
    message.encode(datagram);
    _writer.send(datagram, _tcp_connection);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::report_new_threads
//       Access: Private
//  Description: Sends over any information about new Threads that
//               the user code might have recently created.
////////////////////////////////////////////////////////////////////
void PStatClient::
report_new_threads() {
  nassertv(_is_connected);

  if (_threads_reported < (int)_threads.size()) {
    PStatClientControlMessage message;
    message._type = PStatClientControlMessage::T_define_threads;
    message._first_thread_index = _threads_reported;
    while (_threads_reported < (int)_threads.size()) {
      message._names.push_back(_threads[_threads_reported]._name);
      _threads_reported++;
    }

    Datagram datagram;
    message.encode(datagram);
    _writer.send(datagram, _tcp_connection);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::handle_server_control_message
//       Access: Private
//  Description: Called when a control message has been received by
//               the server over the TCP connection.
////////////////////////////////////////////////////////////////////
void PStatClient::
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
//     Function: PStatClient::connection_reset
//       Access: Private, Virtual
//  Description: Called by the internal net code when the connection
//               has been lost.
////////////////////////////////////////////////////////////////////
void PStatClient::
connection_reset(const PT(Connection) &connection, PRErrorCode) {
  if (connection == _tcp_connection) {
    disconnect();
  } else {
    pstats_cat.warning()
      << "Ignoring spurious connection_reset() message\n";
  }
}

#endif // DO_PSTATS
