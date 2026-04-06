/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cIntervalManager.h
 * @author drose
 * @date 2002-09-10
 */

#ifndef CINTERVALMANAGER_H
#define CINTERVALMANAGER_H

#include "directbase.h"
#include "cInterval.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pmap.h"
#include "vector_int.h"
#include "pmutex.h"

class EventQueue;

/**
 * This object holds a number of currently-playing intervals and is
 * responsible for advancing them each frame as needed.
 *
 * There is normally only one IntervalManager object in the world, and it is
 * the responsibility of the scripting language to call step() on this object
 * once each frame, and to then process the events indicated by
 * get_next_event().
 *
 * It is also possible to create multiple IntervalManager objects for special
 * needs.
 */
class EXPCL_DIRECT_INTERVAL CIntervalManager {
PUBLISHED:
  CIntervalManager();
  ~CIntervalManager();

  INLINE void set_event_queue(EventQueue *event_queue);
  INLINE EventQueue *get_event_queue() const;

  int add_c_interval(CInterval *interval, bool external);
  int find_c_interval(const std::string &name) const;

  CInterval *get_c_interval(int index) const;
  void remove_c_interval(int index);

  int interrupt();
  int get_num_intervals() const;
  int get_max_index() const;

  void step();
  int get_next_event();
  int get_next_removal();

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

  static CIntervalManager *get_global_ptr();

private:
  void finish_interval(CInterval *interval);
  void remove_index(int index);

  enum Flags {
    F_external      = 0x0001,
    F_meta_interval = 0x0002,
  };
  class IntervalDef {
  public:
    PT(CInterval) _interval;
    int _flags;
    int _next_slot;
  };
  typedef pvector<IntervalDef> Intervals;
  Intervals _intervals;
  typedef pmap<std::string, int> NameIndex;
  NameIndex _name_index;
  typedef vector_int Removed;
  Removed _removed;
  EventQueue *_event_queue;

  int _first_slot;
  int _next_event_index;

  Mutex _lock;

  static CIntervalManager *_global_ptr;
};

INLINE std::ostream &operator << (std::ostream &out, const CInterval &ival_mgr);

#include "cIntervalManager.I"

#endif
