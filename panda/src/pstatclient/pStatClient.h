/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClient.h
 * @author drose
 * @date 2000-07-09
 */

#ifndef PSTATCLIENT_H
#define PSTATCLIENT_H

#include "pandabase.h"

#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "reMutex.h"
#include "lightMutex.h"
#include "reMutexHolder.h"
#include "lightMutexHolder.h"
#include "pmap.h"
#include "thread.h"
#include "weakPointerTo.h"
#include "vector_int.h"
#include "atomicAdjust.h"
#include "numeric_types.h"
#include "bitArray.h"

class PStatClientImpl;
class PStatCollector;
class PStatCollectorDef;
class PStatThread;
class GraphicsStateGuardian;

/**
 * Manages the communications to report statistics via a network connection to
 * a remote PStatServer.
 *
 * Normally, there is only one PStatClient in the world, although it is
 * possible to have multiple PStatClients if extraordinary circumstances
 * require in.  Since each PStatCollector registers itself with the
 * PStatClient when it is created, having multiple PStatClients requires
 * special care when constructing the various PStatCollectors.
 *
 * If DO_PSTATS is not defined, we don't want to use stats at all.  This class
 * is therefore defined as a stub class.
 */
#ifdef DO_PSTATS
class EXPCL_PANDA_PSTATCLIENT PStatClient : public Thread::PStatsCallback {
public:
  PStatClient();
  ~PStatClient();

PUBLISHED:
  void set_client_name(const std::string &name);
  std::string get_client_name() const;
  void set_max_rate(double rate);
  double get_max_rate() const;

  INLINE int get_num_collectors() const;
  PStatCollector get_collector(int index) const;
  MAKE_SEQ(get_collectors, get_num_collectors, get_collector);
  INLINE PStatCollectorDef *get_collector_def(int index) const;
  std::string get_collector_name(int index) const;
  std::string get_collector_fullname(int index) const;

  INLINE int get_num_threads() const;
  PStatThread get_thread(int index) const;
  MAKE_SEQ(get_threads, get_num_threads, get_thread);
  INLINE std::string get_thread_name(int index) const;
  INLINE std::string get_thread_sync_name(int index) const;
  INLINE PT(Thread) get_thread_object(int index) const;

  PStatThread get_main_thread() const;
  PStatThread get_current_thread() const;

  double get_real_time() const;

  MAKE_PROPERTY(client_name, get_client_name, set_client_name);
  MAKE_PROPERTY(max_rate, get_max_rate, set_max_rate);
  MAKE_SEQ_PROPERTY(collectors, get_num_collectors, get_collector);
  MAKE_SEQ_PROPERTY(threads, get_num_threads, get_thread);
  MAKE_PROPERTY(main_thread, get_main_thread);
  MAKE_PROPERTY(current_thread, get_current_thread);
  MAKE_PROPERTY(real_time, get_real_time);

  INLINE static bool connect(const std::string &hostname = std::string(), int port = -1);
  INLINE static void disconnect();
  INLINE static bool is_connected();

  INLINE static void resume_after_pause();

  static void main_tick();
  static void thread_tick(const std::string &sync_name);

  void client_main_tick();
  void client_thread_tick(const std::string &sync_name);
  bool client_connect(std::string hostname, int port);
  void client_disconnect();
  bool client_is_connected() const;

  void client_resume_after_pause();

  static PStatClient *get_global_pstats();

private:
  INLINE bool has_impl() const;
  INLINE PStatClientImpl *get_impl();
  INLINE const PStatClientImpl *get_impl() const;
  void make_impl() const;

  PStatCollector make_collector_with_relname(int parent_index, std::string relname);
  PStatCollector make_collector_with_name(int parent_index, const std::string &name);
  PStatThread do_get_current_thread() const;
  PStatThread make_thread(Thread *thread);
  PStatThread do_make_thread(Thread *thread);
  PStatThread make_gpu_thread(const std::string &name);

  bool is_active(int collector_index, int thread_index) const;
  bool is_started(int collector_index, int thread_index) const;

  void start(int collector_index, int thread_index);
  void start(int collector_index, int thread_index, double as_of);
  void stop(int collector_index, int thread_index);
  void stop(int collector_index, int thread_index, double as_of);

  void clear_level(int collector_index, int thread_index);
  void set_level(int collector_index, int thread_index, double level);
  void add_level(int collector_index, int thread_index, double increment);
  double get_level(int collector_index, int thread_index) const;

  static void start_clock_wait();
  static void start_clock_busy_wait();
  static void stop_clock_wait();

  class Collector;
  class InternalThread;
  void add_collector(Collector *collector);
  void add_thread(InternalThread *thread);

  INLINE Collector *get_collector_ptr(int collector_index) const;
  INLINE InternalThread *get_thread_ptr(int thread_index) const;

  virtual void deactivate_hook(Thread *thread);
  virtual void activate_hook(Thread *thread);

private:
  // This mutex protects everything in this class.
  ReMutex _lock;

  typedef pmap<std::string, int> ThingsByName;
  typedef pmap<std::string, vector_int> MultiThingsByName;
  MultiThingsByName _threads_by_name, _threads_by_sync_name;

  // This is for the data that is per-collector, per-thread.  A vector of
  // these is stored in each Collector object, below, indexed by thread index.
  class PerThreadData {
  public:
    PerThreadData();
    bool _has_level;
    double _level;
    int _nested_count;
  };
  typedef pvector<PerThreadData> PerThread;

  // This is where the meat of the Collector data is stored.  (All the stuff
  // in PStatCollector and PStatCollectorDef is just fluff.)
  class EXPCL_PANDA_PSTATCLIENT Collector {
  public:
    INLINE Collector(int parent_index, const std::string &name);
    INLINE int get_parent_index() const;
    INLINE const std::string &get_name() const;
    INLINE bool is_active() const;
    INLINE PStatCollectorDef *get_def(const PStatClient *client, int this_index) const;

  private:
    void make_def(const PStatClient *client, int this_index);

  private:
    // This pointer is initially NULL, and will be filled in when it is first
    // needed.
    PStatCollectorDef *_def;

    // This data is used to create the PStatCollectorDef when it is needed.
    int _parent_index;
    std::string _name;

  public:
    // Relations to other collectors.
    ThingsByName _children;
    PerThread _per_thread;
  };
  typedef Collector *CollectorPointer;
  AtomicAdjust::Pointer _collectors;  // CollectorPointer *_collectors;
  AtomicAdjust::Integer _collectors_size;  // size of the allocated array
  AtomicAdjust::Integer _num_collectors;   // number of in-use elements within the array

  // This defines a single thread, i.e.  a separate chain of execution,
  // independent of all other threads.  Timing and level data are maintained
  // separately for each thread.
  class InternalThread {
  public:
    InternalThread(Thread *thread);
    InternalThread(const std::string &name, const std::string &sync_name = "Main");

    WPT(Thread) _thread;
    std::string _name;
    std::string _sync_name;
    PStatFrameData _frame_data;
    bool _is_active;
    int _frame_number;
    double _next_packet;

    bool _thread_active;
    BitArray _active_collectors;  // no longer used.

    // This mutex is used to protect writes to _frame_data for this particular
    // thread, as well as writes to the _per_thread data for this particular
    // thread in the Collector class, above.
    LightMutex _thread_lock;
  };
  typedef InternalThread *ThreadPointer;
  AtomicAdjust::Pointer _threads;  // ThreadPointer *_threads;
  AtomicAdjust::Integer _threads_size;  // size of the allocated array
  AtomicAdjust::Integer _num_threads;   // number of in-use elements within the array

  mutable PStatClientImpl *_impl;

  static PStatCollector _heap_total_size_pcollector;
  static PStatCollector _heap_overhead_size_pcollector;
  static PStatCollector _heap_single_size_pcollector;
  static PStatCollector _heap_single_other_size_pcollector;
  static PStatCollector _heap_array_size_pcollector;
  static PStatCollector _heap_array_other_size_pcollector;
  static PStatCollector _heap_external_size_pcollector;
  static PStatCollector _mmap_size_pcollector;

  static PStatCollector _mmap_nf_unused_size_pcollector;
  static PStatCollector _mmap_dc_active_other_size_pcollector;
  static PStatCollector _mmap_dc_inactive_other_size_pcollector;
  static PStatCollector _pstats_pcollector;
  static PStatCollector _clock_wait_pcollector;
  static PStatCollector _clock_busy_wait_pcollector;
  static PStatCollector _thread_block_pcollector;

  static PStatClient *_global_pstats;

  friend class Collector;
  friend class PStatCollector;
  friend class PStatThread;
  friend class PStatClientImpl;
  friend class GraphicsStateGuardian;
};

#include "pStatClient.I"

#else  // DO_PSTATS

class EXPCL_PANDA_PSTATCLIENT PStatClient {
public:
  PStatClient() { }
  ~PStatClient() { }

  void set_client_name(const std::string &name);
  std::string get_client_name() const;
  void set_max_rate(double rate);
  double get_max_rate() const;

  PStatCollector get_collector(int index) const;
  std::string get_collector_name(int index) const;
  std::string get_collector_fullname(int index) const;

  INLINE int get_num_threads() const { return 0; }
  PStatThread get_thread(int index) const;
  INLINE std::string get_thread_name(int index) const { return ""; }
  INLINE std::string get_thread_sync_name(int index) const { return ""; }
  INLINE PT(Thread) get_thread_object(int index) const { return nullptr; }

  PStatThread get_main_thread() const;
  PStatThread get_current_thread() const;

  double get_real_time() const;

PUBLISHED:
  INLINE static bool connect(const std::string & = std::string(), int = -1) { return false; }
  INLINE static void disconnect() { }
  INLINE static bool is_connected() { return false; }
  INLINE static void resume_after_pause() { }

  static void main_tick();
  static void thread_tick(const std::string &);

public:
  void client_main_tick();
  void client_thread_tick(const std::string &sync_name);
  bool client_connect(std::string hostname, int port);
  void client_disconnect();
  bool client_is_connected() const;

  void client_resume_after_pause();

  static PStatClient *get_global_pstats();

private:
  // These are used by inline PStatCollector methods, so they need to be
  // stubbed out for ABI compatibility.
  PStatCollector make_collector_with_relname(int parent_index, std::string relname);
  PStatThread make_thread(Thread *thread);

  bool is_active(int collector_index, int thread_index) const;
  bool is_started(int collector_index, int thread_index) const;

  void start(int collector_index, int thread_index);
  void start(int collector_index, int thread_index, double as_of);
  void stop(int collector_index, int thread_index);
  void stop(int collector_index, int thread_index, double as_of);

  void clear_level(int collector_index, int thread_index);
  void set_level(int collector_index, int thread_index, double level);
  void add_level(int collector_index, int thread_index, double increment);
  double get_level(int collector_index, int thread_index) const;
};

#endif  // DO_PSTATS

#endif
