// Filename: pipeline.h
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

#ifndef PIPELINE_H
#define PIPELINE_H

#include "pandabase.h"
#include "pipelineCyclerLinks.h"
#include "namable.h"
#include "pset.h"
#include "reMutex.h"
#include "reMutexHolder.h"
#include "selectThreadImpl.h"  // for THREADED_PIPELINE definition

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
class EXPCL_PANDA_PIPELINE Pipeline : public Namable {
public:
  Pipeline(const string &name, int num_stages);
  ~Pipeline();

  INLINE static Pipeline *get_render_pipeline();

  void cycle();

  void set_num_stages(int num_stages);
  INLINE void set_min_stages(int min_stages);
  INLINE int get_num_stages() const;

#ifdef THREADED_PIPELINE
  void add_cycler(PipelineCyclerTrueImpl *cycler);
  void add_dirty_cycler(PipelineCyclerTrueImpl *cycler);
  void remove_cycler(PipelineCyclerTrueImpl *cycler);

  INLINE int get_num_cyclers() const;
  INLINE int get_num_dirty_cyclers() const;
  
#ifdef DEBUG_THREADS
  typedef void CallbackFunc(TypeHandle type, int count, void *data);
  void iterate_all_cycler_types(CallbackFunc *func, void *data) const;
  void iterate_dirty_cycler_types(CallbackFunc *func, void *data) const;
#endif  // DEBUG_THREADS

#endif  // THREADED_PIPELINE

private:
  int _num_stages;

  static void make_render_pipeline();
  static Pipeline *_render_pipeline;

#ifdef THREADED_PIPELINE
  PipelineCyclerLinks _clean;
  PipelineCyclerLinks _dirty;

  int _num_cyclers;
  int _num_dirty_cyclers;

#ifdef DEBUG_THREADS
  typedef pmap<TypeHandle, int> TypeCount;
  TypeCount _all_cycler_types, _dirty_cycler_types;
  
  static void inc_cycler_type(TypeCount &count, TypeHandle type, int addend);
#endif  // DEBUG_THREADS

  // This is true only during cycle().
  bool _cycling;

  ReMutex _lock;
#endif  // THREADED_PIPELINE
};

#include "pipeline.I"

#endif

