/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file queryContext.h
 * @author drose
 * @date 2006-03-27
 */

#ifndef QUERYCONTEXT_H
#define QUERYCONTEXT_H

#include "pandabase.h"
#include "typedReferenceCount.h"

class PreparedGraphicsObjects;

/**
 * This is a base class for queries that might require a round-trip to the
 * graphics engine.  The idea is that when you ask the GSG to make a
 * particular query, it returns a QueryContext, which does not necessarily
 * have the answer right away (but it will eventually).
 *
 * Unlike SavedContext, QueryContext is reference-counted.  It removes itself
 * from the GSG when the last reference goes away.  You're responsible for
 * keeping the pointer to the QueryContext as long as you are interested in
 * the answer.
 */
class EXPCL_PANDA_GOBJ QueryContext : public TypedReferenceCount {
public:
  INLINE QueryContext();
  virtual ~QueryContext();

  virtual bool is_answer_ready() const=0;
  virtual void waiting_for_answer();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "QueryContext",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "queryContext.I"

#endif
