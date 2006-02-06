// Filename: pipeline.cxx
// Created by:  drose (21Feb02)
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

#include "pipeline.h"
#include "pipelineCyclerTrueImpl.h"
#include "reMutexHolder.h"

Pipeline *Pipeline::_render_pipeline = (Pipeline *)NULL;

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Pipeline::
Pipeline(const string &name) :
  Namable(name)
{
  _num_stages = 1;

#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
  _cycling = false;
#endif  // DO_PIPELINING && HAVE_THREADS
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Pipeline::
~Pipeline() {
#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
  nassertv(_cyclers.empty());
  nassertv(!_cycling);
#endif  // DO_PIPELINING && HAVE_THREADS
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::cycle
//       Access: Public
//  Description: Flows all the pipeline data down to the next stage.
////////////////////////////////////////////////////////////////////
void Pipeline::
cycle() {
#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
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

#endif  // DO_PIPELINING && HAVE_THREADS
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
#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
  ReMutexHolder holder(_lock);
  if (num_stages != _num_stages) {
    
    // We need to lock every PipelineCycler object in the world before
    // we can adjust the number of stages.
    Cyclers::iterator ci;
    for (ci = _cyclers.begin(); ci != _cyclers.end(); ++ci) {
      (*ci)->_lock.lock();
    }
      
    _num_stages = num_stages;
      
    for (ci = _cyclers.begin(); ci != _cyclers.end(); ++ci) {
      (*ci)->set_num_stages(num_stages);
    }
    
    // Now release them all.
    for (ci = _cyclers.begin(); ci != _cyclers.end(); ++ci) {
      (*ci)->_lock.release();
    }
  }

#else  // DO_PIPELINING && HAVE_THREADS
  _num_stages = num_stages;
#endif  // DO_PIPELINING && HAVE_THREADS
}

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::get_num_stages
//       Access: Public
//  Description: Returns the number of stages required for the
//               pipeline.
////////////////////////////////////////////////////////////////////
int Pipeline::
get_num_stages() const {
  return _num_stages;
}


#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
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
  bool inserted = _cyclers.insert(cycler).second;
  nassertv(inserted);
}
#endif  // DO_PIPELINING && HAVE_THREADS

#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
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
  nassertv(!_cycling);
  nassertv(!cycler->_dirty);
  cycler->_dirty = true;

  bool inserted = _dirty_cyclers.insert(cycler).second;
  nassertv(inserted);
}
#endif  // DO_PIPELINING && HAVE_THREADS

#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
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
  
  Cyclers::iterator ci = _cyclers.find(cycler);
  nassertv(ci != _cyclers.end());
  _cyclers.erase(ci);

  if (cycler->_dirty) {
    cycler->_dirty = false;
    Cyclers::iterator ci = _dirty_cyclers.find(cycler);
    nassertv(ci != _dirty_cyclers.end());
    _dirty_cyclers.erase(ci);
  }
}
#endif  // DO_PIPELINING && HAVE_THREADS

////////////////////////////////////////////////////////////////////
//     Function: Pipeline::make_render_pipeline
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void Pipeline::
make_render_pipeline() {
  nassertv(_render_pipeline == (Pipeline *)NULL);
  _render_pipeline = new Pipeline("render");
}
