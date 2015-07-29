// Filename: pStatClient.cxx
// Created by:  drose (09Jul00)
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
#include "thread.h"
#include "clockObject.h"
#include "neverFreeMemory.h"

PStatCollector PStatClient::_heap_total_size_pcollector("System memory:Heap");
PStatCollector PStatClient::_heap_overhead_size_pcollector("System memory:Heap:Overhead");
PStatCollector PStatClient::_heap_single_size_pcollector("System memory:Heap:Single");
PStatCollector PStatClient::_heap_single_other_size_pcollector("System memory:Heap:Single:Other");
PStatCollector PStatClient::_heap_array_size_pcollector("System memory:Heap:Array");
PStatCollector PStatClient::_heap_array_other_size_pcollector("System memory:Heap:Array:Other");
PStatCollector PStatClient::_heap_external_size_pcollector("System memory:Heap:External");
PStatCollector PStatClient::_mmap_size_pcollector("System memory:MMap");

PStatCollector PStatClient::_mmap_nf_unused_size_pcollector("System memory:MMap:NeverFree:Unused");
PStatCollector PStatClient::_mmap_dc_active_other_size_pcollector("System memory:MMap:NeverFree:Active:Other");
PStatCollector PStatClient::_mmap_dc_inactive_other_size_pcollector("System memory:MMap:NeverFree:Inactive:Other");
PStatCollector PStatClient::_pstats_pcollector("*:PStats");
PStatCollector PStatClient::_clock_wait_pcollector("Wait:Clock Wait:Sleep");
PStatCollector PStatClient::_clock_busy_wait_pcollector("Wait:Clock Wait:Spin");
PStatCollector PStatClient::_thread_block_pcollector("Wait:Thread block");

PStatClient *PStatClient::_global_pstats = NULL;


// This class is used to report memory usage per TypeHandle.  We
// create one of these for each TypeHandle in the system.
class TypeHandleCollector {
public:
  PStatCollector _mem_class[TypeHandle::MC_limit];
};
typedef pvector<TypeHandleCollector> TypeHandleCols;
static TypeHandleCols type_handle_cols;


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
  _lock("PStatClient::_lock"),
  _impl(NULL)
{
  _collectors = NULL;
  _collectors_size = 0;
  _num_collectors = 0;

  _threads = NULL;
  _threads_size = 0;
  _num_threads = 0;

  // We always have a collector at index 0 named "Frame".  This tracks
  // the total frame time and is the root of all other collectors.  We
  // have to make this one by hand since it's the root.
  Collector *collector = new Collector(0, "Frame");
  //collector->_def = new PStatCollectorDef(0, "Frame");
  //collector->_def->_parent_index = 0;
  //collector->_def->_suggested_color.set(0.5, 0.5, 0.5);
  add_collector(collector);

  // The main thread is always at index 0.
  make_thread(Thread::get_main_thread());
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
//     Function: PStatClient::get_collector
//       Access: Published
//  Description: Returns the nth collector.
////////////////////////////////////////////////////////////////////
PStatCollector PStatClient::
get_collector(int index) const {
  nassertr(index >= 0 && index < AtomicAdjust::get(_num_collectors), PStatCollector());
  return PStatCollector((PStatClient *)this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_name
//       Access: Published
//  Description: Returns the name of the indicated collector.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_collector_name(int index) const {
  nassertr(index >= 0 && index < AtomicAdjust::get(_num_collectors), string());

  return get_collector_ptr(index)->get_name();
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
  nassertr(index >= 0 && index < AtomicAdjust::get(_num_collectors), string());

  Collector *collector = get_collector_ptr(index);
  int parent_index = collector->get_parent_index();
  if (parent_index == 0) {
    return collector->get_name();
  } else {
    return get_collector_fullname(parent_index) + ":" +
      collector->get_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_thread
//       Access: Published
//  Description: Returns the nth thread.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_thread(int index) const {
  ReMutexHolder holder(_lock);
  nassertr(index >= 0 && index < _num_threads, PStatThread());
  return PStatThread((PStatClient *)this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_main_thread
//       Access: Published
//  Description: Returns a handle to the client's Main thread.  This
//               is the thread that started the application.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_main_thread() const {
  return PStatThread((PStatClient *)this, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_current_thread
//       Access: Published
//  Description: Returns a handle to the currently-executing thread.
//               This is the thread that PStatCollectors will be
//               counted in if they do not specify otherwise.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_current_thread() const {
  if (!client_is_connected()) {
    // No need to make the relatively expensive call to
    // Thread::get_current_thread() if we're not even connected.
    return get_main_thread();
  }
  return PStatThread(Thread::get_current_thread(), (PStatClient *)this);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::main_tick
//       Access: Published, Static
//  Description: A convenience function to call new_frame() on the
//               global PStatClient's main thread, and any other
//               threads with a sync_name of "Main".
////////////////////////////////////////////////////////////////////
void PStatClient::
main_tick() {
  // We have code here to report the memory usage.  We can't put this
  // code inside the MemoryUsage class, where it fits a little better,
  // simply because MemoryUsage is a very low-level class that doesn't
  // know about PStatClient.

#ifdef DO_MEMORY_USAGE
  if (is_connected()) {
    _heap_total_size_pcollector.set_level(MemoryUsage::get_total_size());
    _heap_overhead_size_pcollector.set_level(MemoryUsage::get_panda_heap_overhead());
    _heap_single_size_pcollector.set_level(MemoryUsage::get_panda_heap_single_size());
    _heap_array_size_pcollector.set_level(MemoryUsage::get_panda_heap_array_size());
    _heap_external_size_pcollector.set_level(MemoryUsage::get_external_size());


    _mmap_size_pcollector.set_level(MemoryUsage::get_panda_mmap_size());

    TypeRegistry *type_reg = TypeRegistry::ptr();
    int num_typehandles = type_reg->get_num_typehandles();

    while ((int)type_handle_cols.size() < num_typehandles) {
      type_handle_cols.push_back(TypeHandleCollector());
    }

    size_t single_total_usage = 0;
    size_t array_total_usage = 0;
    size_t dc_active_total_usage = 0;
    size_t dc_inactive_total_usage = 0;
    int i;
    for (i = 0; i < num_typehandles; ++i) {
      TypeHandle type = type_reg->get_typehandle(i);
      for (int mi = 0; mi < (int)TypeHandle::MC_limit; ++mi) {
        TypeHandle::MemoryClass mc = (TypeHandle::MemoryClass)mi;
        size_t usage = type.get_memory_usage(mc);

        switch (mc) {
        case TypeHandle::MC_singleton:
          single_total_usage += usage;
          break;

        case TypeHandle::MC_array:
          array_total_usage += usage;
          break;

        case TypeHandle::MC_deleted_chain_active:
          dc_active_total_usage += usage;
          break;

        case TypeHandle::MC_deleted_chain_inactive:
          dc_inactive_total_usage += usage;
          break;

        case TypeHandle::MC_limit:
          // Not used.
          break;
        }
      }
    }
    size_t min_usage = (single_total_usage + array_total_usage + dc_active_total_usage + dc_inactive_total_usage) / 1024;
    if (!pstats_mem_other) {
      min_usage = 0;
    }
    size_t single_other_usage = single_total_usage;
    size_t array_other_usage = array_total_usage;
    size_t dc_active_other_usage = dc_active_total_usage;
    size_t dc_inactive_other_usage = dc_inactive_total_usage;

    for (i = 0; i < num_typehandles; ++i) {
      TypeHandle type = type_reg->get_typehandle(i);
      for (int mi = 0; mi < (int)TypeHandle::MC_limit; ++mi) {
        TypeHandle::MemoryClass mc = (TypeHandle::MemoryClass)mi;
        PStatCollector &col = type_handle_cols[i]._mem_class[mi];
        size_t usage = type.get_memory_usage(mc);
        if (usage > min_usage || col.is_valid()) {
          // We have some memory usage on this TypeHandle.  See if we
          // have a collector for it.
          if (!col.is_valid()) {
            const char *category = "";
            switch (mc) {
            case TypeHandle::MC_singleton:
              category = "Heap:Single";
              break;

            case TypeHandle::MC_array:
              category = "Heap:Array";
              break;

            case TypeHandle::MC_deleted_chain_active:
              category = "MMap:NeverFree:Active";
              break;

            case TypeHandle::MC_deleted_chain_inactive:
              category = "MMap:NeverFree:Inactive";
              break;

            case TypeHandle::MC_limit:
              // Not used.
              break;
            }
            ostringstream strm;
            strm << "System memory:" << category << ":" << type;
            col = PStatCollector(strm.str());
          }
          col.set_level(usage);

          switch (mc) {
          case TypeHandle::MC_singleton:
            single_other_usage -= usage;
            break;

          case TypeHandle::MC_array:
            array_other_usage -= usage;
            break;

          case TypeHandle::MC_deleted_chain_active:
            dc_active_other_usage -= usage;
            break;

          case TypeHandle::MC_deleted_chain_inactive:
            dc_inactive_other_usage -= usage;
            break;

          case TypeHandle::MC_limit:
            // Not used.
            break;
          }
        }
      }
    }

    _mmap_nf_unused_size_pcollector.set_level(NeverFreeMemory::get_total_unused());

    // The remaining amount--all collectors smaller than 0.1% of the
    // total--go into "other".
    _heap_single_other_size_pcollector.set_level(single_other_usage);
    _heap_array_other_size_pcollector.set_level(array_other_usage);
    _mmap_dc_active_other_size_pcollector.set_level(dc_active_other_usage);
    _mmap_dc_inactive_other_size_pcollector.set_level(dc_inactive_other_usage);
  }
#endif  // DO_MEMORY_USAGE

  get_global_pstats()->client_main_tick();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::thread_tick
//       Access: Published, Static
//  Description: A convenience function to call new_frame() on any
//               threads with the indicated sync_name
////////////////////////////////////////////////////////////////////
void PStatClient::
thread_tick(const string &sync_name) {
  get_global_pstats()->client_thread_tick(sync_name);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_main_tick
//       Access: Published
//  Description: A convenience function to call new_frame() on the
//               given PStatClient's main thread, and any other
//               threads with a sync_name of "Main".
////////////////////////////////////////////////////////////////////
void PStatClient::
client_main_tick() {
  ReMutexHolder holder(_lock);
  if (has_impl()) {
    if (!_impl->client_is_connected()) {
      client_disconnect();
      return;
    }

    _impl->client_main_tick();

    MultiThingsByName::const_iterator ni =
      _threads_by_sync_name.find("Main");
    if (ni != _threads_by_sync_name.end()) {
      const vector_int &indices = (*ni).second;
      for (vector_int::const_iterator vi = indices.begin();
           vi != indices.end();
           ++vi) {
        _impl->new_frame(*vi);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_thread_tick
//       Access: Published
//  Description: A convenience function to call new_frame() on all of
//               the threads with the indicated sync name.
////////////////////////////////////////////////////////////////////
void PStatClient::
client_thread_tick(const string &sync_name) {
  ReMutexHolder holder(_lock);

  if (has_impl()) {
    MultiThingsByName::const_iterator ni =
      _threads_by_sync_name.find(sync_name);
    if (ni != _threads_by_sync_name.end()) {
      const vector_int &indices = (*ni).second;
      for (vector_int::const_iterator vi = indices.begin();
           vi != indices.end();
           ++vi) {
        _impl->new_frame(*vi);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_disconnect
//       Access: Published
//  Description: The nonstatic implementation of disconnect().
////////////////////////////////////////////////////////////////////
void PStatClient::
client_disconnect() {
  ReMutexHolder holder(_lock);
  if (has_impl()) {
    _impl->client_disconnect();
    delete _impl;
    _impl = NULL;
  }

  ThreadPointer *threads = (ThreadPointer *)_threads;
  for (int ti = 0; ti < _num_threads; ++ti) {
    InternalThread *thread = threads[ti];
    thread->_frame_number = 0;
    thread->_is_active = false;
    thread->_next_packet = 0.0;
    thread->_frame_data.clear();
  }

  CollectorPointer *collectors = (CollectorPointer *)_collectors;
  for (int ci = 0; ci < _num_collectors; ++ci) {
    Collector *collector = collectors[ci];
    PerThread::iterator ii;
    for (ii = collector->_per_thread.begin();
         ii != collector->_per_thread.end();
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

    ClockObject::_start_clock_wait = start_clock_wait;
    ClockObject::_start_clock_busy_wait = start_clock_busy_wait;
    ClockObject::_stop_clock_wait = stop_clock_wait;
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
  ReMutexHolder holder(_lock);

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
  ReMutexHolder holder(_lock);

  nassertr(parent_index >= 0 && parent_index < _num_collectors,
           PStatCollector());

  Collector *parent = get_collector_ptr(parent_index);

  // A special case: if we asked for a child the same name as its
  // parent, we really meant the parent.  That is, "Frame:Frame" is
  // really the same collector as "Frame".
  if (parent->get_name() == name) {
    return PStatCollector(this, parent_index);
  }

  ThingsByName::const_iterator ni = parent->_children.find(name);

  if (ni != parent->_children.end()) {
    // We already had a collector by this name; return it.
    int index = (*ni).second;
    nassertr(index >= 0 && index < _num_collectors, PStatCollector());
    return PStatCollector(this, (*ni).second);
  }

  // Create a new collector for this name.
  int new_index = _num_collectors;
  parent->_children.insert(ThingsByName::value_type(name, new_index));

  Collector *collector = new Collector(parent_index, name);
  // collector->_def = new PStatCollectorDef(new_index, name);
  // collector->_def->set_parent(*_collectors[parent_index]._def);
  // initialize_collector_def(this, collector->_def);

  // We need one PerThreadData for each thread.
  while ((int)collector->_per_thread.size() < _num_threads) {
    collector->_per_thread.push_back(PerThreadData());
  }
  add_collector(collector);

  return PStatCollector(this, new_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::do_get_current_thread
//       Access: Private
//  Description: Similar to get_current_thread, but does not grab the
//               lock.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
do_get_current_thread() const {
  Thread *thread = Thread::get_current_thread();
  int thread_index = thread->get_pstats_index();
  if (thread_index != -1) {
    return PStatThread((PStatClient *)this, thread_index);
  }

  // This is the first time we have encountered this current Thread.
  // Make a new PStatThread object for it.
  return ((PStatClient *)this)->do_make_thread(thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::make_thread
//       Access: Private
//  Description: Returns a PStatThread for the indicated Panda Thread
//               object.  This is normally called by a PStatThread
//               constructor.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
make_thread(Thread *thread) {
  ReMutexHolder holder(_lock);
  return do_make_thread(thread);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::do_make_thread
//       Access: Private
//  Description: As above, but assumes the lock is already held.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
do_make_thread(Thread *thread) {
  int thread_index = thread->get_pstats_index();
  if (thread_index != -1) {
    return PStatThread((PStatClient *)this, thread_index);
  }

  MultiThingsByName::const_iterator ni =
    _threads_by_name.find(thread->get_name());

  if (ni != _threads_by_name.end()) {
    // We have seen a thread with this name before.  Can we re-use any
    // of them?
    const vector_int &indices = (*ni).second;
    for (vector_int::const_iterator vi = indices.begin();
         vi != indices.end();
         ++vi) {
      int index = (*vi);
      nassertr(index >= 0 && index < _num_threads, PStatThread());
      ThreadPointer *threads = (ThreadPointer *)_threads;
      if (threads[index]->_thread.was_deleted() &&
          threads[index]->_sync_name == thread->get_sync_name()) {
        // Yes, re-use this one.
        threads[index]->_thread = thread;
        thread->set_pstats_index(index);
        thread->set_pstats_callback(this);
        return PStatThread(this, index);
      }
    }
  }

  // Create a new PStatsThread for this thread pointer.
  int new_index = _num_threads;
  thread->set_pstats_index(new_index);
  thread->set_pstats_callback(this);

  InternalThread *pthread = new InternalThread(thread);
  add_thread(pthread);

  return PStatThread(this, new_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::make_gpu_thread
//       Access: Private
//  Description: Returns a PStatThread representing the GPU.
//               This is normally called by the GSG only.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
make_gpu_thread(const string &name) {
  ReMutexHolder holder(_lock);
  int new_index = _num_threads;

  InternalThread *pthread = new InternalThread(name, "GPU");
  add_thread(pthread);

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
  nassertr(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors), false);
  nassertr(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads), false);

  return (client_is_connected() &&
          get_collector_ptr(collector_index)->is_active() &&
          get_thread_ptr(thread_index)->_is_active);
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
  nassertr(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors), false);
  nassertr(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads), false);

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);

  if (client_is_connected() && collector->is_active() && thread->_is_active) {
    LightMutexHolder holder(thread->_thread_lock);
    if (collector->_per_thread[thread_index]._nested_count == 0) {
      // Not started.
      return false;
    }
    // Started.
    return true;
  }

  // Not even connected.
  return false;
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
  if (!client_is_connected()) {
    return;
  }

#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors));
  nassertv(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads));
#endif

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);

  if (collector->is_active() && thread->_is_active) {
    LightMutexHolder holder(thread->_thread_lock);
    if (collector->_per_thread[thread_index]._nested_count == 0) {
      // This collector wasn't already started in this thread; record
      // a new data point.
      if (thread->_thread_active) {
        thread->_frame_data.add_start(collector_index, get_real_time());
      }
    }
    collector->_per_thread[thread_index]._nested_count++;
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
start(int collector_index, int thread_index, double as_of) {
  if (!client_is_connected()) {
    return;
  }

#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors));
  nassertv(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads));
#endif

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);

  if (collector->is_active() && thread->_is_active) {
    LightMutexHolder holder(thread->_thread_lock);
    if (collector->_per_thread[thread_index]._nested_count == 0) {
      // This collector wasn't already started in this thread; record
      // a new data point.
      if (thread->_thread_active) {
        thread->_frame_data.add_start(collector_index, as_of);
      }
    }
    collector->_per_thread[thread_index]._nested_count++;
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
  if (!client_is_connected()) {
    return;
  }

#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors));
  nassertv(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads));
#endif

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);

  if (collector->is_active() && thread->_is_active) {
    LightMutexHolder holder(thread->_thread_lock);
    if (collector->_per_thread[thread_index]._nested_count == 0) {
      if (pstats_cat.is_debug()) {
        pstats_cat.debug()
          << "Collector " << get_collector_fullname(collector_index)
          << " was already stopped in thread " << get_thread_name(thread_index)
          << "!\n";
      }
      return;
    }

    collector->_per_thread[thread_index]._nested_count--;

    if (collector->_per_thread[thread_index]._nested_count == 0) {
      // This collector has now been completely stopped; record a new
      // data point.
      if (thread->_thread_active) {
        thread->_frame_data.add_stop(collector_index, get_real_time());
      }
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
stop(int collector_index, int thread_index, double as_of) {
  if (!client_is_connected()) {
    return;
  }

#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors));
  nassertv(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads));
#endif

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);

  if (collector->is_active() && thread->_is_active) {
    LightMutexHolder holder(thread->_thread_lock);
    if (collector->_per_thread[thread_index]._nested_count == 0) {
      if (pstats_cat.is_debug()) {
        pstats_cat.debug()
          << "Collector " << get_collector_fullname(collector_index)
          << " was already stopped in thread " << get_thread_name(thread_index)
          << "!\n";
      }
      return;
    }

    collector->_per_thread[thread_index]._nested_count--;

    if (collector->_per_thread[thread_index]._nested_count == 0) {
      // This collector has now been completely stopped; record a new
      // data point.
      thread->_frame_data.add_stop(collector_index, as_of);
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
  if (!client_is_connected()) {
    return;
  }

#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors));
  nassertv(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads));
#endif

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);
  LightMutexHolder holder(thread->_thread_lock);

  collector->_per_thread[thread_index]._has_level = true;
  collector->_per_thread[thread_index]._level = 0.0;
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
set_level(int collector_index, int thread_index, double level) {
  if (!client_is_connected()) {
    return;
  }

#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors));
  nassertv(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads));
#endif

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);

  // We don't want to condition this on whether the client is already
  // connected or the collector is already active, since we might
  // connect the client later, and we will want to have an accurate
  // value at that time.
  LightMutexHolder holder(thread->_thread_lock);

  level *= collector->get_def(this, collector_index)->_factor;

  collector->_per_thread[thread_index]._has_level = true;
  collector->_per_thread[thread_index]._level = level;
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
add_level(int collector_index, int thread_index, double increment) {
  if (!client_is_connected()) {
    return;
  }

#ifdef _DEBUG
  nassertv(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors));
  nassertv(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads));
#endif

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);
  LightMutexHolder holder(thread->_thread_lock);

  increment *= collector->get_def(this, collector_index)->_factor;

  collector->_per_thread[thread_index]._has_level = true;
  collector->_per_thread[thread_index]._level += increment;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_level
//       Access: Private
//  Description: Returns the current level value of the given collector.
//               Returns 0.0 if the pstats client is not connected.
//
//               Normally you would not use this interface directly;
//               instead, call PStatCollector::get_level().
////////////////////////////////////////////////////////////////////
double PStatClient::
get_level(int collector_index, int thread_index) const {
  if (!client_is_connected()) {
    return 0.0;
  }

#ifdef _DEBUG
  nassertr(collector_index >= 0 && collector_index < AtomicAdjust::get(_num_collectors), 0.0f);
  nassertr(thread_index >= 0 && thread_index < AtomicAdjust::get(_num_threads), 0.0f);
#endif

  Collector *collector = get_collector_ptr(collector_index);
  InternalThread *thread = get_thread_ptr(thread_index);
  LightMutexHolder holder(thread->_thread_lock);

  double factor = collector->get_def(this, collector_index)->_factor;

  return collector->_per_thread[thread_index]._level / factor;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::start_clock_wait
//       Access: Private, Static
//  Description: This function is added as a hook into ClockObject, so
//               that we may time the delay for
//               ClockObject::wait_until(), used for certain special
//               clock modes.
//
//               This callback is a hack around the fact that we can't
//               let the ClockObject directly create a PStatCollector,
//               because the pstatclient module depends on putil.
////////////////////////////////////////////////////////////////////
void PStatClient::
start_clock_wait() {
  _clock_wait_pcollector.start();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::start_clock_busy_wait
//       Access: Private, Static
//  Description: This function is added as a hook into ClockObject, so
//               that we may time the delay for
//               ClockObject::wait_until(), used for certain special
//               clock modes.
//
//               This callback is a hack around the fact that we can't
//               let the ClockObject directly create a PStatCollector,
//               because the pstatclient module depends on putil.
////////////////////////////////////////////////////////////////////
void PStatClient::
start_clock_busy_wait() {
  _clock_wait_pcollector.stop();
  _clock_busy_wait_pcollector.start();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::stop_clock_wait
//       Access: Private, Static
//  Description: This function is added as a hook into ClockObject, so
//               that we may time the delay for
//               ClockObject::wait_until(), used for certain special
//               clock modes.
//
//               This callback is a hack around the fact that we can't
//               let the ClockObject directly create a PStatCollector,
//               because the pstatclient module depends on putil.
////////////////////////////////////////////////////////////////////
void PStatClient::
stop_clock_wait() {
  _clock_busy_wait_pcollector.stop();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::add_collector
//       Access: Private
//  Description: Adds a new Collector entry to the _collectors array,
//               in a thread-safe manner.  Assumes _lock is already
//               held.
////////////////////////////////////////////////////////////////////
void PStatClient::
add_collector(PStatClient::Collector *collector) {
  if (_num_collectors >= _collectors_size) {
    // We need to grow the array.  We have to be careful here, because
    // there might be clients accessing the array right now who are
    // not protected by the lock.
    int new_collectors_size = (_collectors_size == 0) ? 128 : _collectors_size * 2;
    CollectorPointer *new_collectors = new CollectorPointer[new_collectors_size];
    memcpy(new_collectors, _collectors, _num_collectors * sizeof(CollectorPointer));
    AtomicAdjust::set_ptr(_collectors, new_collectors);
    AtomicAdjust::set(_collectors_size, new_collectors_size);

    // Now, we still have the old array, which we allow to leak.  We
    // should delete it, but there might be a thread out there that's
    // still trying to access it, so we can't safely delete it; and it
    // doesn't really matter much, since it's not a big leak.  (We
    // will only reallocate the array so many times in an application,
    // and then no more.)

    new_collectors[_num_collectors] = collector;
    AtomicAdjust::inc(_num_collectors);

  } else {
    CollectorPointer *collectors = (CollectorPointer *)_collectors;
    collectors[_num_collectors] = collector;
    AtomicAdjust::inc(_num_collectors);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::add_thread
//       Access: Private
//  Description: Adds a new InternalThread entry to the _threads
//               array, in a thread-safe manner.  Assumes _lock is
//               already held.
////////////////////////////////////////////////////////////////////
void PStatClient::
add_thread(PStatClient::InternalThread *thread) {
  _threads_by_name[thread->_name].push_back(_num_threads);
  _threads_by_sync_name[thread->_sync_name].push_back(_num_threads);

  if (_num_threads >= _threads_size) {
    // We need to grow the array.  We have to be careful here, because
    // there might be clients accessing the array right now who are
    // not protected by the lock.
    int new_threads_size = (_threads_size == 0) ? 128 : _threads_size * 2;
    ThreadPointer *new_threads = new ThreadPointer[new_threads_size];
    memcpy(new_threads, _threads, _num_threads * sizeof(ThreadPointer));
    // We assume that assignment to a pointer and to an int are each
    // atomic.
    AtomicAdjust::set_ptr(_threads, new_threads);
    AtomicAdjust::set(_threads_size, new_threads_size);

    // Now, we still have the old array, which we allow to leak.  We
    // should delete it, but there might be a thread out there that's
    // still trying to access it, so we can't safely delete it; and it
    // doesn't really matter much, since it's not a big leak.  (We
    // will only reallocate the array so many times in an application,
    // and then no more.)

    new_threads[_num_threads] = thread;

  } else {
    ThreadPointer *threads = (ThreadPointer *)_threads;
    threads[_num_threads] = thread;
  }

  AtomicAdjust::inc(_num_threads);

  // We need an additional PerThreadData for this thread in all of the
  // collectors.
  CollectorPointer *collectors = (CollectorPointer *)_collectors;
  for (int ci = 0; ci < _num_collectors; ++ci) {
    Collector *collector = collectors[ci];
    collector->_per_thread.push_back(PerThreadData());
    nassertv((int)collector->_per_thread.size() == _num_threads);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::deactivate_hook
//       Access: Public, Virtual
//  Description: Called when the thread is deactivated (swapped for
//               another running thread).  This is intended to provide
//               a callback hook for PStats to assign time to
//               individual threads properly, particularly in the
//               SIMPLE_THREADS case.
////////////////////////////////////////////////////////////////////
void PStatClient::
deactivate_hook(Thread *thread) {
  // We shouldn't use a mutex here, because this code is only called
  // during the SIMPLE_THREADS case, so a mutex isn't necessary; and
  // because we are called during a context switch, so a mutex might
  // be dangerous.
  if (_impl == NULL) {
    return;
  }
  int thread_index = thread->get_pstats_index();
  InternalThread *ithread = get_thread_ptr(thread_index);

  if (ithread->_thread_active) {
    // Start _thread_block_pcollector, by hand, being careful not to
    // grab any mutexes while we do it.
    double now = _impl->get_real_time();
    ithread->_frame_data.add_start(_thread_block_pcollector.get_index(), now);
    ithread->_thread_active = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::activate_hook
//       Access: Public, Virtual
//  Description: Called when the thread is activated (resumes
//               execution).  This is intended to provide a callback
//               hook for PStats to assign time to individual threads
//               properly, particularly in the SIMPLE_THREADS case.
////////////////////////////////////////////////////////////////////
void PStatClient::
activate_hook(Thread *thread) {
  // We shouldn't use a mutex here, because this code is only called
  // during the SIMPLE_THREADS case, so a mutex isn't necessary; and
  // because we are called during a context switch, so a mutex might
  // be dangerous.
  if (_impl == NULL) {
    return;
  }

  InternalThread *ithread = get_thread_ptr(thread->get_pstats_index());

  if (!ithread->_thread_active) {
    double now = _impl->get_real_time();
    ithread->_frame_data.add_stop(_thread_block_pcollector.get_index(), now);
    ithread->_thread_active = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Collector::make_def
//       Access: Private
//  Description: Creates the new PStatCollectorDef for this collector.
////////////////////////////////////////////////////////////////////
void PStatClient::Collector::
make_def(const PStatClient *client, int this_index) {
  ReMutexHolder holder(client->_lock);
  if (_def == (PStatCollectorDef *)NULL) {
    _def = new PStatCollectorDef(this_index, _name);
    if (_parent_index != this_index) {
      const PStatCollectorDef *parent_def =
        client->get_collector_def(_parent_index);
      _def->set_parent(*parent_def);
    }
    initialize_collector_def(client, _def);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::InternalThread::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PStatClient::InternalThread::
InternalThread(Thread *thread) :
  _thread(thread),
  _name(thread->get_name()),
  _sync_name(thread->get_sync_name()),
  _is_active(false),
  _frame_number(0),
  _next_packet(0.0),
  _thread_active(true),
  _thread_lock(string("PStatClient::InternalThread ") + thread->get_name())
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::InternalThread::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PStatClient::InternalThread::
InternalThread(const string &name, const string &sync_name) :
  _thread(NULL),
  _name(name),
  _sync_name(sync_name),
  _is_active(false),
  _frame_number(0),
  _next_packet(0.0),
  _thread_active(true),
  _thread_lock(string("PStatClient::InternalThread ") + name)
{
}

#endif // DO_PSTATS
