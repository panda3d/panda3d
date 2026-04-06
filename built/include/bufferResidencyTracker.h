/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bufferResidencyTracker.h
 * @author drose
 * @date 2006-03-16
 */

#ifndef BUFFERRESIDENCYTRACKER_H
#define BUFFERRESIDENCYTRACKER_H

#include "pandabase.h"
#include "bufferContextChain.h"
#include "pStatCollector.h"

class BufferContext;

/**
 * This class is used to keep track of the current state of all the
 * BufferContexts for a particular graphics context: whether each one is
 * active (rendered this frame) or inactive (not rendered this frame), and
 * whether it is resident or nonresident in video memory.
 *
 * The primary purpose of this class is to facilitate PStats reporting of
 * video card memory usage.
 */
class EXPCL_PANDA_GOBJ BufferResidencyTracker {
public:
  BufferResidencyTracker(const std::string &pgo_name, const std::string &type_name);
  ~BufferResidencyTracker();

  void begin_frame(Thread *current_thread);
  void end_frame(Thread *current_thread);
  void set_levels();

  INLINE BufferContextChain &get_inactive_nonresident();
  INLINE BufferContextChain &get_active_nonresident();
  INLINE BufferContextChain &get_inactive_resident();
  INLINE BufferContextChain &get_active_resident();

  void write(std::ostream &out, int indent_level) const;

private:
  void move_inactive(BufferContextChain &inactive, BufferContextChain &active);

private:
  enum State {
    // Individual bits.
    S_active   = 0x01,
    S_resident = 0x02,

    // Aggregate bits: unions of the above.
    S_inactive_nonresident = 0x00,
    S_active_nonresident   = 0x01,
    S_inactive_resident    = 0x02,
    S_active_resident      = 0x03,

    // The total number of different states.
    S_num_states = 4,
  };

  // One chain for each of the possible states, ordered as above.
  BufferContextChain _chains[S_num_states];

  // A couple of PStatCollectors just to organize names.
  static PStatCollector _gmem_collector;
  PStatCollector _pgo_collector;

  // One PStatCollector for each state.  These are ordered in reverse order
  // that we would like them to appear in the PStats graph.
  PStatCollector _active_resident_collector;
  PStatCollector _active_nonresident_collector;
  PStatCollector _inactive_resident_collector;
  PStatCollector _inactive_nonresident_collector;

  // The frame number currently considered "active".
  int _active_frame;
  friend class BufferContext;
};

#include "bufferResidencyTracker.I"

#endif
