/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glLatencyQueryContext_src.h
 * @author rdb
 * @date 2014-09-24
 */

class GraphicsStateGuardian;

#ifndef OPENGLES  // Timer queries not supported by OpenGL ES.

/**
 * This is a special variant of GLTimerQueryContext that measures the command
 * latency, ie.  the time it takes for the GPU to actually get to the commands
 * we are issuing right now.
 */
class EXPCL_GL CLP(LatencyQueryContext) : public CLP(TimerQueryContext) {
public:
  CLP(LatencyQueryContext)(CLP(GraphicsStateGuardian) *glgsg, int pstats_index);

  ALLOC_DELETED_CHAIN(CLP(LatencyQueryContext));

  virtual double get_timestamp() const;

  GLint64 _timestamp;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CLP(TimerQueryContext)::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "LatencyQueryContext",
                  CLP(TimerQueryContext)::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glLatencyQueryContext_src.I"

#endif  // OPENGLES
