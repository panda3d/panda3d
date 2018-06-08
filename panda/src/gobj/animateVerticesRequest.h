/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animateVerticesRequest.h
 * @author pratt
 * @date 2007-11-20
 */

#ifndef ANIMATEVERTICESREQUEST
#define ANIMATEVERTICESREQUEST

#include "pandabase.h"

#include "asyncTask.h"
#include "geomVertexData.h"
#include "pointerTo.h"

/**
 * This class object manages a single asynchronous request to animate vertices
 * on a GeomVertexData object.  animate_vertices will be called with
 * force=true (i.e.  blocking) in a sub-thread (if threading is available).
 * No result is stored or returned from this object.  It is expected that the
 * result will be cached and available for immediate use later during
 * rendering.  Thus it is important that the main thread block while these
 * requests are being run (presumably on multiple CPUs/cores), to ensure that
 * the data has been computed by the time it's needed.
 */
class EXPCL_PANDA_GOBJ AnimateVerticesRequest : public AsyncTask {
public:
  ALLOC_DELETED_CHAIN(AnimateVerticesRequest);

PUBLISHED:
  INLINE explicit AnimateVerticesRequest(GeomVertexData *geom_vertex_data);

  INLINE bool is_ready() const;

protected:
  virtual AsyncTask::DoneStatus do_task();

private:
  PT(GeomVertexData) _geom_vertex_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AsyncTask::init_type();
    register_type(_type_handle, "AnimateVerticesRequest",
                  AsyncTask::get_class_type());
    }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "animateVerticesRequest.I"

#endif
