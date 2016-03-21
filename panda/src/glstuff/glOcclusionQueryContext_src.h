/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glOcclusionQueryContext_src.h
 * @author drose
 * @date 2006-03-27
 */

#include "pandabase.h"
#include "occlusionQueryContext.h"
#include "deletedChain.h"

class GraphicsStateGuardian;

#ifndef OPENGLES  // Occlusion queries not supported by OpenGL ES.

/**
 *
 */
class EXPCL_GL CLP(OcclusionQueryContext) : public OcclusionQueryContext {
public:
  INLINE CLP(OcclusionQueryContext)(GraphicsStateGuardian *gsg);
  virtual ~CLP(OcclusionQueryContext)();
  ALLOC_DELETED_CHAIN(CLP(OcclusionQueryContext));

  virtual bool is_answer_ready() const;
  virtual void waiting_for_answer();
  virtual int get_num_fragments() const;

  GLuint _index;
  GraphicsStateGuardian *_gsg;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OcclusionQueryContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "OcclusionQueryContext",
                  OcclusionQueryContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glOcclusionQueryContext_src.I"

#endif  // OPENGLES
