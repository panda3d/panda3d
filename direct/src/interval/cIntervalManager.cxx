/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cIntervalManager.cxx
 * @author drose
 * @date 2002-09-10
 */

#include "cIntervalManager.h"
#include "cMetaInterval.h"
#include "dcast.h"
#include "eventQueue.h"
#include "mutexHolder.h"

CIntervalManager *CIntervalManager::_global_ptr;

/**
 *
 */
CIntervalManager::
CIntervalManager() {
  _first_slot = 0;
  _next_event_index = 0;
  _event_queue = EventQueue::get_global_event_queue();
}

/**
 *
 */
CIntervalManager::
~CIntervalManager() {
  nassertv(_name_index.empty());
}

/**
 * Adds the interval to the manager, and returns a unique index for the
 * interval.  This index will be unique among all the currently added
 * intervals, but not unique across all intervals ever added to the manager.
 * The maximum index value will never exceed the maximum number of intervals
 * added at any given time.
 *
 * If the external flag is true, the interval is understood to also be stored
 * in the scripting language data structures.  In this case, it will be
 * available for information returned by get_next_event() and
 * get_next_removal().  If external is false, the interval's index will never
 * be returned by these two functions.
 */
int CIntervalManager::
add_c_interval(CInterval *interval, bool external) {
  MutexHolder holder(_lock);

  // First, check the name index.  If we already have an interval by this
  // name, it gets finished and removed.
  NameIndex::iterator ni = _name_index.find(interval->get_name());
  if (ni != _name_index.end()) {
    int old_index = (*ni).second;
    nassertr(old_index >= 0 && old_index < (int)_intervals.size(), -1)
    CInterval *old_interval = _intervals[old_index]._interval;
    if (old_interval == interval) {
      // No, it's the same interval that was already here.  In this case,
      // don't finish the interval; just return it.
      return old_index;
    }
    finish_interval(old_interval);
    remove_index(old_index);
    _name_index.erase(ni);
  }

  int slot;

  if (_first_slot >= (int)_intervals.size()) {
    // All the slots are filled; make a new slot.
    nassertr(_first_slot == (int)_intervals.size(), -1);
    slot = (int)_intervals.size();
    _intervals.push_back(IntervalDef());
    _first_slot = (int)_intervals.size();

  } else {
    // Some slot is available; use it.
    slot = _first_slot;
    nassertr(slot >= 0 && slot < (int)_intervals.size(), -1);
    _first_slot = _intervals[slot]._next_slot;
  }

  IntervalDef &def = _intervals[slot];
  def._interval = interval;
  def._flags = 0;
  if (external) {
    def._flags |= F_external;
  }
  if (interval->is_of_type(CMetaInterval::get_class_type())) {
    def._flags |= F_meta_interval;
  }
  def._next_slot = -1;

  _name_index[interval->get_name()] = slot;
  nassertr(_first_slot >= 0, slot);
  return slot;
}

/**
 * Returns the index associated with the named interval, if there is such an
 * interval, or -1 if there is not.
 */
int CIntervalManager::
find_c_interval(const std::string &name) const {
  MutexHolder holder(_lock);

  NameIndex::const_iterator ni = _name_index.find(name);
  if (ni != _name_index.end()) {
    return (*ni).second;
  }
  return -1;
}

/**
 * Returns the interval associated with the given index.
 */
CInterval *CIntervalManager::
get_c_interval(int index) const {
  MutexHolder holder(_lock);

  nassertr(index >= 0 && index < (int)_intervals.size(), nullptr);
  return _intervals[index]._interval;
}

/**
 * Removes the indicated interval from the queue immediately.  It will not be
 * returned from get_next_removal(), and none of its pending events, if any,
 * will be returned by get_next_event().
 */
void CIntervalManager::
remove_c_interval(int index) {
  MutexHolder holder(_lock);

  nassertv(index >= 0 && index < (int)_intervals.size());
  IntervalDef &def = _intervals[index];
  nassertv(def._interval != nullptr);

  NameIndex::iterator ni = _name_index.find(def._interval->get_name());
  nassertv(ni != _name_index.end());
  nassertv((*ni).second == index);
  _name_index.erase(ni);

  def._interval = nullptr;
  def._next_slot = _first_slot;
  _first_slot = index;
}

/**
 * Pauses or finishes (removes from the active queue) all intervals tagged
 * with auto_pause or auto_finish set to true.  These are intervals that
 * someone fired up but won't necessarily expect to clean up; they can be
 * interrupted at will when necessary.
 *
 * Returns the number of intervals affected.
 */
int CIntervalManager::
interrupt() {
  MutexHolder holder(_lock);

  int num_paused = 0;

  NameIndex::iterator ni;
  ni = _name_index.begin();
  while (ni != _name_index.end()) {
    int index = (*ni).second;
    const IntervalDef &def = _intervals[index];
    nassertr(def._interval != nullptr, num_paused);
    if (def._interval->get_auto_pause() || def._interval->get_auto_finish()) {
      // This interval may be interrupted.
      if (def._interval->get_auto_pause()) {
        // It may be interrupted simply by pausing it.
        if (interval_cat.is_debug()) {
          interval_cat.debug()
            << "Auto-pausing " << def._interval->get_name() << "\n";
        }
        if (def._interval->get_state() == CInterval::S_started) {
          def._interval->priv_interrupt();
        }

      } else {
        // It should be interrupted by finishing it.
        if (interval_cat.is_debug()) {
          interval_cat.debug()
            << "Auto-finishing " << def._interval->get_name() << "\n";
        }
        switch (def._interval->get_state()) {
        case CInterval::S_initial:
          def._interval->priv_instant();
          break;

        case CInterval::S_final:
          break;

        default:
          def._interval->priv_finalize();
        }
      }

      // Now carefully remove it from the active list.
      NameIndex::iterator prev;
      prev = ni;
      ++ni;
      _name_index.erase(prev);
      remove_index(index);
      num_paused++;

    } else {
      // The interval should remain on the active list.
      ++ni;
    }
  }

  return num_paused;
}

/**
 * Returns the number of currently active intervals.
 */
int CIntervalManager::
get_num_intervals() const {
  MutexHolder holder(_lock);

  return _name_index.size();
}

/**
 * Returns one more than the largest interval index number in the manager.  If
 * you walk through all the values between (0, get_max_index()] and call
 * get_c_interval() on each number, you will retrieve all of the managed
 * intervals (and possibly a number of NULL pointers as well).
 */
int CIntervalManager::
get_max_index() const {
  MutexHolder holder(_lock);

  return _intervals.size();
}

/**
 * This should be called every frame to do the processing for all the active
 * intervals.  It will call step_play() for each interval that has been added
 * and that has not yet been removed.
 *
 * After each call to step(), the scripting language should call
 * get_next_event() and get_next_removal() repeatedly to process all the high-
 * level (e.g.  Python-interval-based) events and to manage the high-level
 * list of intervals.
 */
void CIntervalManager::
step() {
  MutexHolder holder(_lock);

  NameIndex::iterator ni;
  ni = _name_index.begin();
  while (ni != _name_index.end()) {
    int index = (*ni).second;
    const IntervalDef &def = _intervals[index];
    nassertv(def._interval != nullptr);
    if (!def._interval->step_play()) {
      // This interval is finished and wants to be removed from the active
      // list.
      NameIndex::iterator prev;
      prev = ni;
      ++ni;
      _name_index.erase(prev);
      remove_index(index);

    } else {
      // The interval can remain on the active list.
      ++ni;
    }
  }

  _next_event_index = 0;
}

/**
 * This should be called by the scripting language after each call to step().
 * It returns the index number of the next interval that has events requiring
 * servicing by the scripting language, or -1 if no more intervals have any
 * events pending.
 *
 * If this function returns something other than -1, it is the scripting
 * language's responsibility to query the indicated interval for its next
 * event via get_event_index(), and eventually pop_event().
 *
 * Then get_next_event() should be called again until it returns -1.
 */
int CIntervalManager::
get_next_event() {
  MutexHolder holder(_lock);

  while (_next_event_index < (int)_intervals.size()) {
    IntervalDef &def = _intervals[_next_event_index];
    if (def._interval != nullptr) {
      if ((def._flags & F_external) != 0 &&
          def._interval->check_t_callback()) {
        return _next_event_index;
      }
      if ((def._flags & F_meta_interval) != 0) {
        CMetaInterval *meta_interval;
        DCAST_INTO_R(meta_interval, def._interval, -1);
        if (meta_interval->is_event_ready()) {
          nassertr((def._flags & F_external) != 0, -1);
          return _next_event_index;
        }
      }
    }
    _next_event_index++;
  }

  return -1;
}

/**
 * This should be called by the scripting language after each call to step().
 * It returns the index number of an interval that was recently removed, or -1
 * if no intervals were removed.
 *
 * If this returns something other than -1, the scripting language should
 * clean up its own data structures accordingly, and then call
 * get_next_removal() again.
 */
int CIntervalManager::
get_next_removal() {
  MutexHolder holder(_lock);

  if (!_removed.empty()) {
    int index = _removed.back();
    _removed.pop_back();

    nassertr(index >= 0 && index < (int)_intervals.size(), -1);
    IntervalDef &def = _intervals[index];
    def._interval = nullptr;
    def._next_slot = _first_slot;
    _first_slot = index;
    return index;
  }

  return -1;
}

/**
 *
 */
void CIntervalManager::
output(std::ostream &out) const {
  MutexHolder holder(_lock);

  out << "CIntervalManager, " << (int)_name_index.size() << " intervals.";
}

/**
 *
 */
void CIntervalManager::
write(std::ostream &out) const {
  MutexHolder holder(_lock);

  // We need to write this line so that it's clear what's going on when there
  // are no intervals in the list.
  out << (int)_name_index.size() << " intervals.\n";

  NameIndex::const_iterator ni;
  for (ni = _name_index.begin(); ni != _name_index.end(); ++ni) {
    int index = (*ni).second;
    nassertv(index >= 0 && index < (int)_intervals.size());
    const IntervalDef &def = _intervals[index];
    nassertv(def._interval != nullptr);
    out << *def._interval << "\n";
  }

  if (!_removed.empty()) {
    out << "\nRemoved:\n";
    Removed::const_iterator ri;
    for (ri = _removed.begin(); ri != _removed.end(); ++ri) {
      int index = (*ri);
      nassertv(index >= 0 && index < (int)_intervals.size());
      const IntervalDef &def = _intervals[index];
      nassertv(def._interval != nullptr);
      out << "(R)" << *def._interval << "\n";
    }
  }
}

/**
 * Returns the pointer to the one global CIntervalManager object.
 */
CIntervalManager *CIntervalManager::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new CIntervalManager;
  }
  return _global_ptr;
}

/**
 * Explicitly finishes the indicated interval in preparation for moving it to
 * the removed queue.
 */
void CIntervalManager::
finish_interval(CInterval *interval) {
  switch (interval->get_state()) {
  case CInterval::S_initial:
    interval->priv_instant();
    break;

  case CInterval::S_final:
    break;

  default:
    interval->priv_finalize();
  }
}

/**
 * Removes the indicated index number from the active list, either by moving
 * it to the removed queue if it is flagged external, or by simply making the
 * slot available again if it is not.  Assumes the lock is already held.
 */
void CIntervalManager::
remove_index(int index) {
  nassertv(_lock.debug_is_locked());
  nassertv(index >= 0 && index < (int)_intervals.size());
  IntervalDef &def = _intervals[index];
  if ((def._flags & F_external) != 0) {
    _removed.push_back(index);
  } else {
    def._interval = nullptr;
    def._next_slot = _first_slot;
    _first_slot = index;
  }
}
