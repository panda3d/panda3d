/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file savedContext.h
 * @author drose
 * @date 2001-06-11
 */

#ifndef SAVEDCONTEXT_H
#define SAVEDCONTEXT_H

#include "pandabase.h"

#include "typedObject.h"

/**
 * This is the base class for all GSG-specific context objects, such as
 * TextureContext and GeomContext.  It exists mainly to provide some
 * structural organization.
 */
class EXPCL_PANDA_GOBJ SavedContext : public TypedObject {
public:
  INLINE SavedContext();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "SavedContext",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

inline std::ostream &operator << (std::ostream &out, const SavedContext &context) {
  context.output(out);
  return out;
}

#include "savedContext.I"

#endif
