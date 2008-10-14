// Filename: pipeline.cxx
// Created by:  drose (21Feb02)
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

#include "pipeline.h"
#include "pipelineCyclerTrueImpl.h"
#include "reMutexHolder.h"
#include "configVariableInt.h"
#include "config_pipeline.h"

Pipeline *Pipeline::_render_pipeline = (Pipeline *)NULL;

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Pipeline::
Pipeline(const string &name, int num_stages) :
  Namable(name)
#ifdef THREADED_PIPELINE
  , _lock("Pipeline")
#endif
{
#ifdef THREADED_PIPELINE
  // Set up the linked list of cyclers to be a circular list that
  // begins with this object.
  _prev = this;
  _next = this;

  _num_cyclers = 0;
  _cycling = false;

#endif  // THREADED_PIPELINE

  set_num_stages(num_stages);
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Pipeline::
~Pipeline() {
#ifdef THREADED_PIPELINE
  nassertv(_num_cyclers == 0);
  nassertv(_prev == this && _next == this);
  _prev = NULL;
  _next = NULL;
  nassertv(!_cycling);
#endif  // THREADED_PIPELINE
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::cycle
//       Access: Public
//  Description: Flows all the pipeline data down to the next stage.
////////////////////////////////////////////////////////////////////
void Pipeline::
cycle() {
#ifdef THREADED_PIPELINE
  pvector< PT(CycleData) > saved_cdatas;
  saved_cdatas.reserve(_dirty_cyclers.size());
  {
    ReMutexHolder holder(_lock);
    if (_num_stages == 1) {
      // No need to cycle if there's only one stage.
      nassertv(_dirty_cyclers.empty());
      return;
    }
    
    nassertv(!_cycling);
    _cycling = true;
    
    Cyclers next_dirty_cyclers;

    Cyclers::iterator ci;
    switch (_num_stages) {
    case 2:
      for (ci = _dirty_cyclers.begin(); ci != _dirty_cyclers.end(); ++ci) {
        PipelineCyclerTrueImpl *cycler = (*ci);
        ReMutexHolder holder2(cycler->_lock);
        
        // We save the result of cycle(), so that we can defer the
        // side-effects that might occur when CycleDatas destruct, at
        // least until the end of this loop.
        saved_cdatas.push_back(cycler->cycle_2());
        
        if (cycler->_dirty) {
          // The cycler is still dirty after cycling.  Preserve it in the
          // set for next time.
          bool inserted = next_dirty_cyclers.insert(cycler).second;
          nassertv(inserted);
        } else {
#ifdef DEBUG_THREADS
          inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
        }
      }
      break;

    case 3:
      for (ci = _dirty_cyclers.begin(); ci != _dirty_cyclers.end(); ++ci) {
        PipelineCyclerTrueImpl *cycler = (*ci);
        ReMutexHolder holder2(cycler->_lock);
        
        saved_cdatas.push_back(cycler->cycle_3());
        if (cycler->_dirty) {
          bool inserted = next_dirty_cyclers.insert(cycler).second;
          nassertv(inserted);
        } else {
#ifdef DEBUG_THREADS
          inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
        }
      }
      break;

    default:
      for (ci = _dirty_cyclers.begin(); ci != _dirty_cyclers.end(); ++ci) {
        PipelineCyclerTrueImpl *cycler = (*ci);
        ReMutexHolder holder2(cycler->_lock);
        
        saved_cdatas.push_back(cycler->cycle());
        if (cycler->_dirty) {
          bool inserted = next_dirty_cyclers.insert(cycler).second;
          nassertv(inserted);
        } else {
#ifdef DEBUG_THREADS
          inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
        }
      }
      break;
    }
      
    // Finally, we're ready for the next frame.
    _dirty_cyclers.swap(next_dirty_cyclers);
    _cycling = false;
  }

  // And now it's safe to let the CycleData pointers in saved_cdatas
  // destruct, which may cause cascading deletes, and which will in
  // turn case PipelineCyclers to remove themselves from (or add
  // themselves to) the _dirty_cyclers list.

#endif  // THREADED_PIPELINE
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::set_num_stages
//       Access: Public
//  Description: Specifies the number of stages required for the
//               pipeline.
////////////////////////////////////////////////////////////////////
void Pipeline::
set_num_stages(int num_stages) {
  nassertv(num_stages >= 1);
#ifdef THREADED_PIPELINE
  ReMutexHolder holder(_lock);
  if (num_stages != _num_stages) {

    // We need to lock every PipelineCycler object attached to this
    // pipeline before we can adjust the number of stages.
    PipelineCyclerLinks *links;
    for (links = this->_next; links != this; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.acquire();
    }

    _num_stages = num_stages;

    for (links = this->_next; links != this; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->set_num_stages(num_stages);
    }

    // Now release them all.
    int count = 0;
    for (links = this->_next; links != this; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.release();
      ++count;
    }
    nassertv(count == _num_cyclers);
  }

#else  // THREADED_PIPELINE
  if (num_stages != 1) {
    pipeline_cat.warning()
      << "Requested " << num_stages
      << " pipeline stages but multithreaded render pipelines not enabled in build.\n";
  }
  _num_stages = 1;
#endif  // THREADED_PIPELINE
}

#ifdef THREADED_PIPELINE
////////////////////////////////////////////////////////////////////
//     Function: Pipeline::add_cycler
//       Access: Public
//  Description: Adds the indicated cycler to the list of cyclers
//               associated with the pipeline.  This method only
//               exists when true pipelining is configured on.
////////////////////////////////////////////////////////////////////
void Pipeline::
add_cycler(PipelineCyclerTrueImpl *cycler) {
  ReMutexHolder holder(_lock);
  nassertv(!cycler->_dirty);
  nassertv(!_cycling);
  ++_num_cyclers;
  cycler->insert_before(this);
  
#ifdef DEBUG_THREADS
  inc_cycler_type(_all_cycler_types, cycler->get_parent_type(), 1);
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
////////////////////////////////////////////////////////////////////
//     Function: Pipeline::add_dirty_cycler
//       Access: Public
//  Description: Marks the indicated cycler as "dirty", meaning it
//               will need to be cycled next frame.  This both adds it
//               to the "dirty" set and also sets the "dirty" flag
//               within the cycler.  This method only exists when true
//               pipelining is configured on.
////////////////////////////////////////////////////////////////////
void Pipeline::
add_dirty_cycler(PipelineCyclerTrueImpl *cycler) {
  nassertv(cycler->_lock.debug_is_locked());

  ReMutexHolder holder(_lock);
  nassertv(_num_stages != 1);
  nassertv(!_cycling);
  nassertv(!cycler->_dirty);
  cycler->_dirty = true;

  bool inserted = _dirty_cyclers.insert(cycler).second;
  nassertv(inserted);

#ifdef DEBUG_THREADS
  inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), 1);
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
////////////////////////////////////////////////////////////////////
//     Function: Pipeline::remove_cycler
//       Access: Public
//  Description: Removes the indicated cycler from the list of cyclers
//               associated with the pipeline.  This method only
//               exists when true pipelining is configured on.
////////////////////////////////////////////////////////////////////
void Pipeline::
remove_cycler(PipelineCyclerTrueImpl *cycler) {
  nassertv(cycler->_lock.debug_is_locked());

  ReMutexHolder holder(_lock);
  nassertv(!_cycling);

  --_num_cyclers;
  cycler->remove_from_list();

#ifdef DEBUG_THREADS
  inc_cycler_type(_all_cycler_types, cycler->get_parent_type(), -1);
#endif

  if (cycler->_dirty) {
    cycler->_dirty = false;
    Cyclers::iterator ci = _dirty_cyclers.find(cycler);
    nassertv(ci != _dirty_cyclers.end());
    _dirty_cyclers.erase(ci);
#ifdef DEBUG_THREADS
    inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
  }
}
#endif  // THREADED_PIPELINE

#if defined(THREADED_PIPELINE) && defined(DEBUG_THREADS) 
////////////////////////////////////////////////////////////////////
//     Function: Pipeline::iterate_all_cycler_types
//       Access: Public
//  Description: Walks through the list of all the different
//               PipelineCycler types in the universe.  For each one,
//               calls the indicated callback function with the
//               TypeHandle of the respective type (actually, the
//               result of cycler::get_parent_type()) and the count of
//               pipeline cyclers of that type.  Mainly used for
//               PStats reporting.
////////////////////////////////////////////////////////////////////
void Pipeline::
iterate_all_cycler_types(CallbackFunc *func, void *data) const {
  ReMutexHolder holder(_lock);
  TypeCount::const_iterator ci;
  for (ci = _all_cycler_types.begin(); ci != _all_cycler_types.end(); ++ci) {
    func((*ci).first, (*ci).second, data);
  }
}
#endif  // THREADED_PIPELINE && DEBUG_THREADS

#if defined(THREADED_PIPELINE) && defined(DEBUG_THREADS) 
////////////////////////////////////////////////////////////////////
//     Function: Pipeline::iterate_dirty_cycler_types
//       Access: Public
//  Description: Walks through the list of all the different
//               PipelineCycler types, for only the dirty
//               PipelineCyclers.  See also
//               iterate_all_cycler_types().
////////////////////////////////////////////////////////////////////
void Pipeline::
iterate_dirty_cycler_types(CallbackFunc *func, void *data) const {
  ReMutexHolder holder(_lock);
  TypeCount::const_iterator ci;
  for (ci = _dirty_cycler_types.begin(); ci != _dirty_cycler_types.end(); ++ci) {
    func((*ci).first, (*ci).second, data);
  }
}
#endif  // THREADED_PIPELINE && DEBUG_THREADS

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::make_render_pipeline
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void Pipeline::
make_render_pipeline() {
  ConfigVariableInt pipeline_stages
    ("pipeline-stages", 1,
     PRC_DESC("The initial number of stages in the render pipeline.  This is "
              "only meaningful if threaded pipelining is compiled into "
              "Panda.  In most cases, you should not set this at all anyway, "
              "since the pipeline can automatically grow stages as needed, "
              "but it will not remove stages automatically, and having more "
              "pipeline stages than your application requires will incur "
              "additional runtime overhead."));

  nassertv(_render_pipeline == (Pipeline *)NULL);
  _render_pipeline = new Pipeline("render", pipeline_stages);
}

#if defined(THREADED_PIPELINE) && defined(DEBUG_THREADS) 
////////////////////////////////////////////////////////////////////
//     Function: Pipeline::inc_cycler_type
//       Access: Private, Static
//  Description: Increments (or decrements, according to added) the
//               value for TypeHandle in the indicated TypeCount map.
//               This is used in DEBUG_THREADS mode to track the types
//               of PipelineCyclers that are coming and going, mainly
//               for PStats reporting.
//
//               It is assumed the lock is held during this call.
////////////////////////////////////////////////////////////////////
void Pipeline::
inc_cycler_type(TypeCount &count, TypeHandle type, int addend) {
  TypeCount::iterator ci = count.find(type);
  if (ci == count.end()) {
    ci = count.insert(TypeCount::value_type(type, 0)).first;
  }
  (*ci).second += addend;
  nassertv((*ci).second >= 0);
}
#endif  // THREADED_PIPELINE && DEBUG_THREADS
