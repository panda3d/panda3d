// Filename: pipeline.h
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

#ifndef PIPELINE_H
#define PIPELINE_H

#include "pandabase.h"
#include "namable.h"
#include "pset.h"
#include "reMutex.h"

struct PipelineCyclerTrueImpl;

////////////////////////////////////////////////////////////////////
//       Class : Pipeline
// Description : This class manages a staged pipeline of data, for
//               instance the render pipeline, so that each stage of
//               the pipeline can simultaneously access different
//               copies of the same data.  It actually maintains a
//               collection of PipelineCycler objects, and manages the
//               turning of all of them at once.
//
//               There is one default Pipeline object, the render
//               pipeline.  Other specialty pipelines may be created
//               as needed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Pipeline : public Namable {
public:
  Pipeline(const string &name);
  ~Pipeline();

  INLINE static Pipeline *get_render_pipeline();

  void cycle();

  void set_num_stages(int num_stages);
  INLINE void set_min_stages(int min_stages);
  INLINE int get_num_stages() const;

#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
  void add_cycler(PipelineCyclerTrueImpl *cycler);
  void add_dirty_cycler(PipelineCyclerTrueImpl *cycler);
  void remove_cycler(PipelineCyclerTrueImpl *cycler);

  INLINE int get_num_cyclers() const;
  INLINE int get_num_dirty_cyclers() const;
#endif  // DO_PIPELINING && HAVE_THREADS

private:
  int _num_stages;

  static void make_render_pipeline();
  static Pipeline *_render_pipeline;

#if defined(DO_PIPELINING) && defined(HAVE_THREADS)
  typedef pset<PipelineCyclerTrueImpl *> Cyclers;
  Cyclers _cyclers;
  Cyclers _dirty_cyclers;

  // This is true only during cycle().
  bool _cycling;

  ReMutex _lock;
#endif  // DO_PIPELINING && HAVE_THREADS
};

#include "pipeline.I"

#endif

