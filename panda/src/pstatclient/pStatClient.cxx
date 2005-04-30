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

#include "pStatClientImpl.h"
#include "pStatClientControlMessage.h"
#include "pStatServerControlMessage.h"
#include "pStatCollector.h"
#include "pStatThread.h"
#include "config_pstats.h"
#include "pStatProperties.h"

PStatCollector PStatClient::_total_size_pcollector("Memory usage");
PStatCollector PStatClient::_cpp_size_pcollector("Memory usage:C++");
PStatCollector PStatClient::_interpreter_size_pcollector("Memory usage:Interpreter");
PStatCollector PStatClient::_pstats_pcollector("*:PStats");

PStatClient *PStatClient::_global_pstats = NULL;


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
  _impl(NULL)
{
  // We always have a collector at index 0 named "Frame".  This tracks
  // the total frame time and is the root of all other collectors.  We
  // have to make this one by hand since it's the root.
  Collector collector(0, "Frame");
  //collector._def = new PStatCollectorDef(0, "Frame");
  //collector._def->_parent_index = 0;
  //collector._def->_suggested_color.set(0.5, 0.5, 0.5);
  _collectors.push_back(collector);

  // We also always have a thread at index 0 named "Main".
  make_thread("Main");
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatClient::
~PStatClient() {
  disconnect();

  if (has_impl()) {
    delete _impl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector
//       Access: Published
//  Description: Returns the nth collector.
////////////////////////////////////////////////////////////////////
PStatCollector PStatClient::
get_collector(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), PStatCollector());
  return PStatCollector((PStatClient *)this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_name
//       Access: Published
//  Description: Returns the name of the indicated collector.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_collector_name(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), string());

  return _collectors[index].get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_fullname
//       Access: Published
//  Description: Returns the "full name" of the indicated collector.
//               This will be the concatenation of all of the
//               collector's parents' names (except Frame) and the
//               collector's own name.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_collector_fullname(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), string());

  int parent_index = _collectors[index].get_parent_index();
  if (parent_index == 0) {
    return _collectors[index].get_name();
  } else {
    return get_collector_fullname(parent_index) + ":" + 
      _collectors[index].get_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_thread
//       Access: Published
//  Description: Returns the nth thread.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_thread(int index) const {
  nassertr(index >= 0 && index < (int)_threads.size(), PStatThread());
  return PStatThread((PStatClient *)this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_main_thread
//       Access: Published
//  Description: Returns a handle to the client's "Main", or default,
//               thread.  This is where collectors will be started and
//               stopped if they don't specify otherwise.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_main_thread() const {
  return PStatThread((PStatClient *)this, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::main_tick
//       Access: Published, Static
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
//     Function: PStatClient::client_main_tick
//       Access: Published
//  Description: A convenience function to call new_frame() on the
//               the given client's main thread.
////////////////////////////////////////////////////////////////////
void PStatClient::
client_main_tick() {
  if (has_impl()) {
    _impl->client_main_tick();
  }
  get_main_thread().new_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_disconnect
//       Access: Published
//  Description: The nonstatic implementation of disconnect().
////////////////////////////////////////////////////////////////////
void PStatClient::
client_disconnect() {
  if (has_impl()) {
    _impl->client_disconnect();
  }

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
//     Function: PStatClient::get_global_pstats
//       Access: Published, Static
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
  if (parent.get_name() == name) {
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
  _collectors.push_back(Collector(parent_index, name));

  Collector &collector = _collectors.back();

  // collector._def = new PStatCollectorDef(new_index, name);
  // collector._def->set_parent(*_collectors[parent_index]._def);
  // initialize_collector_def(this, collector._def);

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

  return (client_is_connected() &&
          _collectors[collector_index].is_active() &&
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

  return (_collectors[collector_index].is_active() &&
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

  if (client_is_connected() && 
      _collectors[collector_index].is_active() &&
      _threads[thread_index]._is_active) {
    if (_collectors[collector_index]._per_thread[thread_index]._nested_count == 0) {
      // This collector wasn't already started in this thread; record
      // a new data point.
      _threads[thread_index]._frame_data.add_start(collector_index, 
                                                   get_clock().get_real_time());
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

  if (client_is_connected() && 
      _collectors[collector_index].is_active() &&
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

  if (client_is_connected() && 
      _collectors[collector_index].is_active() &&
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
                                                  get_clock().get_real_time());
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

  if (client_is_connected() &&
      _collectors[collector_index].is_active() &&
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
  _collectors[collector_index]._per_thread[thread_index]._has_level = false;
  _collectors[collector_index]._per_thread[thread_index]._level = 0.0;
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
  // We don't want to condition this on whether the client is already
  // connected or the collector is already active, since we might
  // connect the client later, and we will want to have an accurate
  // value at that time.
  level *= get_collector_def(collector_index)->_factor;
  _collectors[collector_index]._per_thread[thread_index]._has_level = true;
  _collectors[collector_index]._per_thread[thread_index]._level = level;
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
  increment *= get_collector_def(collector_index)->_factor;
  _collectors[collector_index]._per_thread[thread_index]._has_level = true;
  _collectors[collector_index]._per_thread[thread_index]._level += increment;
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
    get_collector_def(collector_index)->_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Collector::make_def
//       Access: Private
//  Description: Creates the new PStatCollectorDef for this collector.
////////////////////////////////////////////////////////////////////
void PStatClient::Collector::
make_def(const PStatClient *client, int this_index) {
  _def = new PStatCollectorDef(this_index, _name);
  if (_parent_index != this_index) {
    const PStatCollectorDef *parent_def = 
      client->_collectors[_parent_index].get_def(client, _parent_index);
    _def->set_parent(*parent_def);
  }
  initialize_collector_def(client, _def);
}

#endif // DO_PSTATS
