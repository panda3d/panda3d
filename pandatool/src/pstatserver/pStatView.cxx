// Filename: pStatView.cxx
// Created by:  drose (10Jul00)
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

#include "pStatView.h"

#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "vector_int.h"
#include "plist.h"
#include "pset.h"

#include <algorithm>


////////////////////////////////////////////////////////////////////
//     Function: PStatView::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatView::
PStatView() {
  _constraint = 0;
  _show_level = false;
  _all_collectors_known = false;
  _level_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatView::
~PStatView() {
  clear_levels();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::constrain
//       Access: Public
//  Description: Changes the focus of the View.  By default, the View
//               reports the entire time for the frame, and all of the
//               Collectors that are directly parented to "Frame".  By
//               constraining the view to a particular collector, you
//               cause the View to zoom in on that collector's data,
//               reporting only the collector and its immediate
//               parents.
//
//               When you constrain the view, you may also specify
//               whether the view should show time data or level data
//               for the indicated collector.  If level data, it
//               reports the levels for the collector, and all of its
//               children; otherwise, it collects the elapsed time.
//
//               Changing the constraint causes the current frame's
//               data to become invalidated; you must then call
//               set_to_frame() again to get any useful data out.
////////////////////////////////////////////////////////////////////
void PStatView::
constrain(int collector, bool show_level) {
  _constraint = collector;
  _show_level = show_level;
  clear_levels();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::unconstrain
//       Access: Public
//  Description: Restores the view to the full frame.  This is
//               equivalent to calling constrain(0).
////////////////////////////////////////////////////////////////////
void PStatView::
unconstrain() {
  constrain(0, false);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::set_thread_data
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PStatView::
set_thread_data(const PStatThreadData *thread_data) {
  _thread_data = thread_data;
  _client_data = thread_data->get_client_data();
  clear_levels();
  _all_collectors_known = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::set_to_frame
//       Access: Public
//  Description: Supplies the View with the data for the current
//               frame.  This causes the View to update all of its
//               internal data to reflect the frame's data, subject to
//               the current constraint.
//
//               It is possible that calling this will increase the
//               total number of reported levels (for instance, if
//               this frame introduced a new collector that hadn't
//               been active previously).  In this case, the caller
//               must update its display or whatever to account for
//               the new level.
////////////////////////////////////////////////////////////////////
void PStatView::
set_to_frame(const PStatFrameData &frame_data) {
  nassertv(!_thread_data.is_null());
  nassertv(!_client_data.is_null());

  if (_show_level) {
    update_level_data(frame_data);
  } else {
    update_time_data(frame_data);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PStatView::all_collectors_known
//       Access: Public
//  Description: After a call to set_to_frame(), this returns true if
//               all collectors in the FrameData are known by the
//               PStatsData object, or false if some are still unknown
//               (even those that do not appear in the view).
////////////////////////////////////////////////////////////////////
bool PStatView::
all_collectors_known() const {
  return _all_collectors_known;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::get_net_value
//       Access: Public
//  Description: Returns the total value accounted for by the frame (or
//               by whatever Collector we are constrained to).  This
//               is the sum of all of the individual levels'
//               get_net_value() value.
////////////////////////////////////////////////////////////////////
float PStatView::
get_net_value() const {
  float net = 0.0;
  Levels::const_iterator li;
  for (li = _levels.begin(); li != _levels.end(); ++li) {
    net += (*li).second->_value_alone;
  }

  return net;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::get_top_level
//       Access: Public
//  Description: Returns a pointer to the level that corresponds to
//               the Collector we've constrained to.  This is the top
//               of a graph of levels; typically the next level
//               down--the children of this level--will be the levels
//               you want to display to the user.
////////////////////////////////////////////////////////////////////
const PStatViewLevel *PStatView::
get_top_level() {
  return get_level(_constraint);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::has_level
//       Access: Public
//  Description: Returns true if there is a level defined for the
//               particular collector, false otherwise.
////////////////////////////////////////////////////////////////////
bool PStatView::
has_level(int collector) const {
  Levels::const_iterator li;
  li = _levels.find(collector);
  return (li != _levels.end());
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::get_level
//       Access: Public
//  Description: Returns a pointer to the level that corresponds to
//               the indicated Collector.  If there is no such level
//               in the view, one will be created--use with caution.
//               Check has_level() first if you don't want this
//               behavior.
////////////////////////////////////////////////////////////////////
PStatViewLevel *PStatView::
get_level(int collector) {
  Levels::const_iterator li;
  li = _levels.find(collector);
  if (li != _levels.end()) {
    return (*li).second;
  }

  PStatViewLevel *level = new PStatViewLevel;
  level->_collector = collector;
  level->_parent = NULL;
  _levels[collector] = level;

  reset_level(level);
  return level;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::update_time_data
//       Access: Private
//  Description: The implementation of set_to_frame() for views that
//               show elapsed time.
////////////////////////////////////////////////////////////////////
void PStatView::
update_time_data(const PStatFrameData &frame_data) {
  _all_collectors_known = true;

  Samples samples(_client_data->get_num_collectors());
  fill_samples(samples, frame_data);

  bool any_new_levels = false;

  // Now match these samples we got up with those we already had in
  // the levels.
  Levels::iterator li, lnext;
  li = _levels.begin();
  while (li != _levels.end()) {
    // Be careful while traversing a container and calling functions
    // that could modify that container.
    lnext = li;
    ++lnext;

    PStatViewLevel *level = (*li).second;
    if (reset_level(level)) {
      any_new_levels = true;
    }

    int collector_index = level->_collector;
    if (samples[collector_index]._touched) {
      level->_value_alone = samples[collector_index]._net_time;
      samples[collector_index]._touched = false;
    }

    li = lnext;
  }

  // Finally, any samples left over in the samples list are new
  // collectors that we need to add to the Levels list.
  for (int collector_index = 0; 
       collector_index < (int)samples.size(); 
       ++collector_index) {
    if (samples[collector_index]._touched) {
      any_new_levels = true;
      PStatViewLevel *level = get_level(collector_index);
      level->_value_alone = samples[collector_index]._net_time;
    }
  }

  if (any_new_levels) {
    _level_index++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::fill_samples
//       Access: Private
//  Description: Fills up samples with the net time accumulated
//               uniquely in each collector.
////////////////////////////////////////////////////////////////////
void PStatView::
fill_samples(Samples &samples, const PStatFrameData &frame_data) {
  int num_events = frame_data.get_num_events();
  if (num_events == 0) {
    return;
  }

  cerr << "start fill_samples\n";

  Started started;

  Samples::iterator si;
  for (si = samples.begin(); si != samples.end(); ++si) {
    (*si).initialize(frame_data.get_start(), started);
    int collector_index = si - samples.begin();
    cerr << "  " << _client_data->get_collector_fullname(collector_index)
         << " " << (*si)._started << " " << (*si)._pushed
         << " " << (*si)._net_time << "\n";
  }

  for (int i = 0; i < num_events; i++) {
    int collector_index = frame_data.get_time_collector(i);
    bool is_start = frame_data.is_start(i);

    cerr << "  event: " 
         << _client_data->get_collector_fullname(collector_index)
         << " " << is_start << " " << frame_data.get_time(i) << "\n";

    if (!_client_data->has_collector(collector_index)) {
      _all_collectors_known = false;

    } else {
      nassertv(collector_index >= 0 && collector_index < (int)samples.size());

      if (_client_data->get_child_distance(_constraint, collector_index) >= 0) {
        // Here's a data point we care about: anything at constraint
        // level or below.
        if (is_start == samples[collector_index]._started) {
          if (!is_start && !samples[collector_index]._touched) {
            // A "stop" in the middle of a frame probably means the
            // collector wrapped around the end of the frame and back
            // to the beginning.  This is very difficult to record
            // properly--which collectors did it shadow from the
            // beginning the frame?  This will require going back and
            // re-interpreting the frame from the beginning.  For now,
            // we'll set a flag; when we later encounter this
            // collector's "start" time, we'll interpret this flag.
            samples[collector_index]._reset_initially_started = true;
            
          } else {
            // An extra "start" for a collector that's already started
            // is an error.
            nout << "Unexpected data point for " 
                 << _client_data->get_collector_fullname(collector_index)
                 << "\n";
          }
        } else {
          if (is_start && samples[collector_index]._reset_initially_started) {
            // OK, here's the initial start for this collector.  This
            // means that we push all of the collectors that are
            // currently active now.  It also means that all of these
            // collectors will still be pushed at the beginning of the
            // next frame (or, assuming the frames are largely
            // similar, at the beginning of this frame).
            Started::iterator sti;
            for (sti = started.begin(); sti != started.end(); ++sti) {
              (*sti)->_initially_pushed = true;
            }
            samples[collector_index]._initially_started = true;
            // Now go back and try again, re-examining the data from
            // the beginning of the frame.
            cerr << "recurse on " 
                 << _client_data->get_collector_fullname(collector_index)
                 << "\n";
            fill_samples(samples, frame_data);
            return;
          }
          samples[collector_index].data_point(frame_data.get_time(i), is_start, started);
        }
      }
    }
  }

  for (si = samples.begin(); si != samples.end(); ++si) {
    (*si).finalize(frame_data.get_end(), started);
  }

  nassertv(started.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::update_level_data
//       Access: Private
//  Description: The implementation of set_to_frame() for views that
//               show level values.
////////////////////////////////////////////////////////////////////
void PStatView::
update_level_data(const PStatFrameData &frame_data) {
  _all_collectors_known = true;


  // This tracks the set of level values we got.
  typedef pmap<int, float> GotValues;
  GotValues net_values;

  int i;
  int num_levels = frame_data.get_num_levels();
  for (i = 0; i < num_levels; i++) {
    int collector_index = frame_data.get_level_collector(i);
    float value = frame_data.get_level(i);

    if (!_client_data->has_collector(collector_index)) {
      _all_collectors_known = false;

    } else {
      if (_client_data->get_child_distance(_constraint, collector_index) >= 0) {
        net_values[collector_index] = value;
      }
    }
  }

  // Now that we've counted up the net level for each collector,
  // compute the level for each collector alone by subtracting out
  // each child from its parents.  If a parent has no data, nothing is
  // subtracted.
  GotValues alone_values = net_values;

  GotValues::iterator gi;
  for (gi = net_values.begin(); gi != net_values.end(); ++gi) {
    int collector_index = (*gi).first;
    float value = (*gi).second;

    // Walk up to the top.
    while (collector_index != 0 && collector_index != _constraint) {
      const PStatCollectorDef &def =
        _client_data->get_collector_def(collector_index);
      int parent_index = def._parent_index;
      GotValues::iterator pi = alone_values.find(parent_index);
      if (pi != alone_values.end()) {
        // The parent has data; subtract it.
        (*pi).second -= value;
      }

      collector_index = parent_index;
    }
  }


  bool any_new_levels = false;

  // Now match these samples we got up with those we already had in
  // the levels.
  Levels::iterator li, lnext;
  li = _levels.begin();
  while (li != _levels.end()) {
    // Be careful while traversing a container and calling functions
    // that could modify that container.
    lnext = li;
    ++lnext;

    PStatViewLevel *level = (*li).second;
    if (reset_level(level)) {
      any_new_levels = true;
    }

    int collector_index = level->_collector;
    GotValues::iterator gi;
    gi = alone_values.find(collector_index);
    if (gi != alone_values.end()) {
      level->_value_alone = (*gi).second;
      alone_values.erase(gi);
    }

    li = lnext;
  }

  // Finally, any values left over in the alone_values set are new
  // collectors that we need to add to the Levels list.
  if (!alone_values.empty()) {
    any_new_levels = true;

    GotValues::const_iterator gi;
    for (gi = alone_values.begin(); gi != alone_values.end(); ++gi) {
      int collector_index = (*gi).first;
      PStatViewLevel *level = get_level(collector_index);
      level->_value_alone = (*gi).second;
    }
  }

  if (any_new_levels) {
    _level_index++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::clear_levels
//       Access: Private
//  Description: Resets all the levels that have been defined so far.
////////////////////////////////////////////////////////////////////
void PStatView::
clear_levels() {
  Levels::iterator li;
  for (li = _levels.begin(); li != _levels.end(); ++li) {
    delete (*li).second;
  }
  _levels.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::reset_level
//       Access: Private
//  Description: Resets the total value of the Level to zero, and also
//               makes sure it is parented to the right Level
//               corresponding to its Collector's parent.  Since the
//               client might change its mind from time to time about
//               who the Collector is parented to, we have to update
//               this dynamically.
//
//               Returns true if any change was made to the level's
//               hierarchy, false otherwise.
////////////////////////////////////////////////////////////////////
bool PStatView::
reset_level(PStatViewLevel *level) {
  bool any_changed = false;
  level->_value_alone = 0.0;

  if (level->_collector == _constraint) {
    return false;
  }

  if (_client_data->has_collector(level->_collector)) {
    int parent_index =
      _client_data->get_collector_def(level->_collector)._parent_index;

    if (level->_parent == (PStatViewLevel *)NULL) {
      // This level didn't know its parent before, but now it does.
      PStatViewLevel *parent_level = get_level(parent_index);
      nassertr(parent_level != level, true);

      level->_parent = parent_level;
      parent_level->_children.push_back(level);
      parent_level->sort_children(_client_data);
      any_changed = true;

    } else if (level->_parent->_collector != parent_index) {
      // This level knew about its parent, but now it's something
      // different.
      PStatViewLevel *old_parent_level = level->_parent;
      nassertr(old_parent_level != level, true);

      if (parent_index != 0) {
        PStatViewLevel *new_parent_level = get_level(parent_index);
        nassertr(new_parent_level != level, true);
        level->_parent = new_parent_level;
        new_parent_level->_children.push_back(level);
        new_parent_level->sort_children(_client_data);
      } else {
        level->_parent = NULL;
      }

      PStatViewLevel::Children::iterator ci =
        find(old_parent_level->_children.begin(),
             old_parent_level->_children.end(),
             level);

      nassertr(ci != old_parent_level->_children.end(), true);
      old_parent_level->_children.erase(ci);
      any_changed = true;
    }
  }

  return any_changed;
}



////////////////////////////////////////////////////////////////////
//     Function: PStatView::FrameSample::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatView::FrameSample::
FrameSample() {
  _touched = false;
  _started = false;
  _pushed = false;
  _initially_pushed = false;
  _initially_started = false;
  _reset_initially_started = false;
  _net_time = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::FrameSample::data_point
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PStatView::FrameSample::
data_point(float time, bool is_start, PStatView::Started &started) {
  cerr << "data_point(" << time << ", " << is_start << ",)\n";
  _touched = true;
  nassertv(is_start != _started);
  
  // The _net_time value has two possible different meanings,
  // according to the current state of the frame.  When the
  // collector is not actively accumulating time, then _net_start
  // represents the total time already accumulated towards the
  // collector.  However, while the collector is actively
  // accumulating time, then _net_start is a stange value, liable to
  // be negative, which is actually the total time accumulated so
  // far less the start time of the collector.
  
  // You can tell which state _net_time is in by examining
  // _started and _pushed.  The four cases are:
  
  // When !_started && !_pushed, _net_time is valid.
  // When !_started && _pushed, _net_time is valid.
  // When _started && !_pushed, _net_time is invalid (the
  // collector is running).
  // When _started && _pushed, _net_time is valid.
  
  _started = is_start;
  
  if (_started) {
    // Starting the collector.
    if (!_pushed) {
      // (!_started && !_pushed) -> (_started && !_pushed)
      _net_time -= time;
      
      // Starting a new collector obscures, or "pushes", all of the
      // collectors already started.
      push_all(time, started);
    }
    started.push_back(this);
    
  } else {
    // Stopping the collector.
    if (_pushed) {
      // We have been pushed below some other collector.  In this
      // case, the time has already been stopped.  Just remove
      // this collector from the pushed stack.
      Started::iterator si = find(started.begin(), started.end(), this);
      nassertv(si != started.end());
      started.erase(si);
      
      // (_started && _pushed) -> (!_started && _pushed)
      nassertv(_net_time >= 0.0);
      
    } else {
      // We are still active.  Accumulate the time since we started.
      // (_started && !_pushed) -> (!_started && !_pushed)
      _net_time += time;
      nassertv(_net_time >= 0.0);
      
      // Now reveal, or "pop", the topmost of the collectors that
      // were previously obscured.
      Started::iterator si = find(started.begin(), started.end(), this);
      nassertv(si != started.end());
      started.erase(si);
      pop_one(time, started);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::FrameSample::push
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PStatView::FrameSample::
push(float time) {
  if (!_pushed) {
    _pushed = true;
    if (_started) {
      // (_started && !_pushed) -> (_started && _pushed)
      _net_time += time;
      nassertv(_net_time >= 0.0);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::FrameSample::pop
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PStatView::FrameSample::
pop(float time) {
  nassertv(_pushed);
  _pushed = false;
  if (_started) {
    // (_started && _pushed) -> (_started && !_pushed)
    _net_time -= time;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::FrameSample::push_all
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PStatView::FrameSample::
push_all(float time, PStatView::Started &started) {
  Started::iterator si;
  for (si = started.begin(); si != started.end(); ++si) {
    (*si)->push(time);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::FrameSample::pop_one
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PStatView::FrameSample::
pop_one(float time, PStatView::Started &started) {
  Started::reverse_iterator si;
  for (si = started.rbegin(); si != started.rend(); ++si) {
    if ((*si)->_pushed) {
      (*si)->pop(time);
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::FrameSample::initialize
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PStatView::FrameSample::
initialize(float start_time, PStatView::Started &started) {
  _touched = false;
  _pushed = _initially_pushed;
  _started = _initially_started;
  _reset_initially_started = false;
  _net_time = 0.0;

  if (_started) {
    started.push_back(this);
    if (!_pushed) {
      _net_time -= start_time;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatView::FrameSample::finalize
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PStatView::FrameSample::
finalize(float end_time, PStatView::Started &started) {
  if (_started) {
    data_point(end_time, false, started);
  }
}
