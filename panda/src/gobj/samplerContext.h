/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file samplerContext.h
 * @author rdb
 * @date 2014-12-11
 */

#ifndef SAMPLERCONTEXT_H
#define SAMPLERCONTEXT_H

#include "pandabase.h"

#include "adaptiveLru.h"
#include "simpleLru.h"
#include "samplerState.h"
#include "savedContext.h"

/**
 * This is a special class object that holds a handle to the sampler state
 * object given by the graphics back-end for a particular combination of
 * texture sampling settings.
 *
 * Some graphics back-ends (like OpenGL) use mutable sampler objects, whereas
 * others (Direct3D 10+) use immutable ones.  In Panda3D, each unique sampler
 * state has its own SamplerContext, which simplifies the implementation and
 * makes redundant sampler objects impossible.
 */
class EXPCL_PANDA_GOBJ SamplerContext : public SavedContext, public SimpleLruPage {
public:
  INLINE SamplerContext(const SamplerState &sampler);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedContext::init_type();
    register_type(_type_handle, "SamplerContext",
                  SavedContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
};

inline std::ostream &operator << (std::ostream &out, const SamplerContext &context) {
  context.output(out);
  return out;
}

#include "samplerContext.I"

#endif
