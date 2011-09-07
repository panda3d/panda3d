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

  // We maintain all of the cyclers in the world on one of two linked
  // lists.  Cyclers that are "clean", which is to say, they have the
  // same value across all pipeline stages, are stored on the _clean
  // list.  Cyclers that are "dirty", which have different values
  // across some pipeline stages, are stored instead on the _dirty
  // list.  Cyclers can move themselves from clean to dirty by calling
  // add_dirty_cycler(), and cyclers get moved from dirty to clean
  // during cycle().

  // To visit each cycler once requires traversing both lists.
  _clean.make_head();
  _dirty.make_head();

  // We also store the total count of all cyclers, clean and dirty, in
  // _num_cyclers; and the count of only dirty cyclers in
  // _num_dirty_cyclers.
  _num_cyclers = 0;
  _num_dirty_cyclers = 0;

  // This flag is true only during the call to cycle().
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
  nassertv(_num_dirty_cyclers == 0);
  _clean.clear_head();
  _dirty.clear_head();
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
  if (pipeline_cat.is_debug()) {
    pipeline_cat.debug()
      << "Beginning the pipeline cycle\n";
  }

  pvector< PT(CycleData) > saved_cdatas;
  saved_cdatas.reserve(_num_dirty_cyclers);
  {
    ReMutexHolder holder(_lock);
    if (_num_stages == 1) {
      // No need to cycle if there's only one stage.
      nassertv(_dirty._next == &_dirty);
      return;
    }
    
    nassertv(!_cycling);
    _cycling = true;
    
    // Move the dirty list to prev_dirty, for processing.
    PipelineCyclerLinks prev_dirty;
    prev_dirty.make_head();
    prev_dirty.take_list(_dirty);
    _num_dirty_cyclers = 0;

    switch (_num_stages) {
    case 2:
      while (prev_dirty._next != &prev_dirty) {
        PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)prev_dirty._next;
        cycler->remove_from_list();
        ReMutexHolder holder2(cycler->_lock);
        
        // We save the result of cycle(), so that we can defer the
        // side-effects that might occur when CycleDatas destruct, at
        // least until the end of this loop.
        saved_cdatas.push_back(cycler->cycle_2());
        
        if (cycler->_dirty) {
          // The cycler is still dirty after cycling.  Keep it on the
          // dirty list for next time.
          cycler->insert_before(&_dirty);
          ++_num_dirty_cyclers;
        } else {
          // The cycler is now clean.  Add it back to the clean list.
          cycler->insert_before(&_clean);
#ifdef DEBUG_THREADS
          inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
        }
      }
      break;

    case 3:
      while (prev_dirty._next != &prev_dirty) {
        PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)prev_dirty._next;
        cycler->remove_from_list();
        ReMutexHolder holder2(cycler->_lock);
        
        saved_cdatas.push_back(cycler->cycle_3());
        
        if (cycler->_dirty) {
          cycler->insert_before(&_dirty);
          ++_num_dirty_cyclers;
        } else {
          cycler->insert_before(&_clean);
#ifdef DEBUG_THREADS
          inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
        }
      }
      break;

    default:
      while (prev_dirty._next != &prev_dirty) {
        PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)prev_dirty._next;
        cycler->remove_from_list();
        ReMutexHolder holder2(cycler->_lock);
        
        saved_cdatas.push_back(cycler->cycle());
        
        if (cycler->_dirty) {
          cycler->insert_before(&_dirty);
          ++_num_dirty_cyclers;
        } else {
          cycler->insert_before(&_clean);
#ifdef DEBUG_THREADS
          inc_cycler_type(_dirty_cycler_types, cycler->get_parent_type(), -1);
#endif
        }
      }
      break;
    }
      
    // Now we're ready for the next frame.
    prev_dirty.clear_head();
    _cycling = false;
  }

  // And now it's safe to let the CycleData pointers in saved_cdatas
  // destruct, which may cause cascading deletes, and which will in
  // turn cause PipelineCyclers to remove themselves from (or add
  // themselves to) the _dirty list.
  saved_cdatas.clear();

  if (pipeline_cat.is_debug()) {
    pipeline_cat.debug()
      << "Finished the pipeline cycle\n";
  }

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
    for (links = _clean._next; links != &_clean; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.acquire();
    }
    for (links = _dirty._next; links != &_dirty; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.acquire();
    }

    _num_stages = num_stages;

    for (links = _clean._next; links != &_clean; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->set_num_stages(num_stages);
    }
    for (links = _dirty._next; links != &_dirty; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->set_num_stages(num_stages);
    }

    // Now release them all.
    int count = 0;
    for (links = _clean._next; links != &_clean; links = links->_next) {
      PipelineCyclerTrueImpl *cycler = (PipelineCyclerTrueImpl *)links;
      cycler->_lock.release();
      ++count;
    }
    for (links = _dirty._next; links != &_dirty; links = links->_next) {
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

  cycler->insert_before(&_clean);
  ++_num_cyclers;
  
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

  // Remove it from the "clean" list and add it to the "dirty" list.
  cycler->remove_from_list();
  cycler->insert_before(&_dirty);
  cycler->_dirty = true;
  ++_num_dirty_cyclers;

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
    --_num_dirty_cyclers;
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
