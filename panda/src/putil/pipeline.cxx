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
  ReMutexHolder holder(_lock);
  Cyclers::iterator ci;
  for (ci = _cyclers.begin(); ci != _cyclers.end(); ++ci) {
    (*ci)->cycle();
  }
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
  bool inserted = _cyclers.insert(cycler).second;
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
  ReMutexHolder holder(_lock);
  Cyclers::iterator ci = _cyclers.find(cycler);
  nassertv(ci != _cyclers.end());
  _cyclers.erase(ci);
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
