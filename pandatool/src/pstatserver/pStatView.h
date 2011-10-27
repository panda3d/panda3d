// Filename: pStatView.h
// Created by:  drose (10Jul00)
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

#ifndef PSTATVIEW_H
#define PSTATVIEW_H

#include "pandatoolbase.h"

#include "pStatClientData.h"
#include "pStatThreadData.h"
#include "pStatViewLevel.h"
#include "pmap.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PStatView
// Description : A View boils down the frame data to a linear list of
//               times spent in a number of different Collectors,
//               within a particular thread.  This automatically
//               accounts for overlapping start/stop times and nested
//               Collectors in a sensible way.
////////////////////////////////////////////////////////////////////
class PStatView {
public:
  PStatView();
  ~PStatView();

  void constrain(int collector, bool show_level);
  void unconstrain();

  void set_thread_data(const PStatThreadData *thread_data);
  INLINE const PStatThreadData *get_thread_data();
  INLINE const PStatClientData *get_client_data();

  void set_to_frame(const PStatFrameData &frame_data);
  INLINE void set_to_frame(int frame_number);
  INLINE void set_to_time(double time);

  bool all_collectors_known() const;
  double get_net_value() const;

  const PStatViewLevel *get_top_level();

  bool has_level(int collector) const;
  PStatViewLevel *get_level(int collector);
 
  INLINE bool get_show_level() const;
  INLINE int get_level_index() const;

private:
  void update_time_data(const PStatFrameData &frame_data);
  void update_level_data(const PStatFrameData &frame_data);

  void clear_levels();
  bool reset_level(PStatViewLevel *level);

  int _constraint;
  bool _show_level;
  bool _all_collectors_known;

  typedef pmap<int, PStatViewLevel *> Levels;
  Levels _levels;

  int _level_index;

  CPT(PStatClientData) _client_data;
  CPT(PStatThreadData) _thread_data;
};

#include "pStatView.I"

#endif

