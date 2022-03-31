/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackObject.h
 * @author drose
 * @date 2009-03-13
 */

#ifndef CALLBACKOBJECT_H
#define CALLBACKOBJECT_H

#include "pandabase.h"
#include "typedReferenceCount.h"

class CallbackData;

/**
 * This is a generic object that can be assigned to a callback at various
 * points in the rendering process.  This is actually a base class for a
 * handful of specialized callback object types.  You can also subclass it
 * yourself to make your own callback handler.
 */
class EXPCL_PANDA_PUTIL CallbackObject : public TypedReferenceCount {
protected:
  INLINE CallbackObject();
public:
  ALLOC_DELETED_CHAIN(CallbackObject);

PUBLISHED:
  virtual void output(std::ostream &out) const;

  EXTENSION(static PT(CallbackObject) make(PyObject *function));

public:
  virtual void do_callback(CallbackData *cbdata);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CallbackObject",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

inline std::ostream &operator << (std::ostream &out, const CallbackObject &cbo) {
  cbo.output(out);
  return out;
}

#include "callbackObject.I"

#endif
