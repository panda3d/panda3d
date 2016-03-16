/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glTimerQueryContext_src.h
 * @author rdb
 * @date 2014-08-22
 */

#include "pandabase.h"
#include "timerQueryContext.h"
#include "deletedChain.h"
#include "clockObject.h"

class GraphicsStateGuardian;

#ifndef OPENGLES  // Timer queries not supported by OpenGL ES.

/**
 * This class manages a timer query that can be used by a PStatGPUTimer to
 * measure the time a task takes to execute on the GPU. This records the
 * current timestamp; a pair of these is usually used to get the elapsed time.
 */
class EXPCL_GL CLP(TimerQueryContext) : public TimerQueryContext {
public:
  INLINE CLP(TimerQueryContext)(CLP(GraphicsStateGuardian) *glgsg,
                                int pstats_index);
  virtual ~CLP(TimerQueryContext)();

  ALLOC_DELETED_CHAIN(CLP(TimerQueryContext));

  virtual bool is_answer_ready() const;
  virtual void waiting_for_answer();
  virtual double get_timestamp() const;

  GLuint _index;
  WPT(CLP(GraphicsStateGuardian)) _glgsg;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TimerQueryContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "TimerQueryContext",
                  TimerQueryContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glTimerQueryContext_src.I"

#endif  // OPENGLES
