/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgMouseWatcherParameter.h
 * @author drose
 * @date 2001-07-05
 */

#ifndef PGMOUSEWATCHERPARAMETER_H
#define PGMOUSEWATCHERPARAMETER_H

#include "pandabase.h"

#include "mouseWatcherParameter.h"
#include "typedWritableReferenceCount.h"

/**
 * This specialization on MouseWatcherParameter allows us to tag on additional
 * elements to events for the gui system, and also inherits from
 * TypedWritableReferenceCount so we can attach this thing to an event.
 */
class EXPCL_PANDA_PGUI PGMouseWatcherParameter : public TypedWritableReferenceCount, public MouseWatcherParameter {
  // For now, this must inherit from TypedWritableReferenceCount on the left,
  // because MSVC++ wants to make that base class be the one at the front of
  // the structure, not MouseWatcherParameter for some reason, and interrogate
  // assumes that whichever base class is on the left will be at the front of
  // the structure.
public:
  INLINE PGMouseWatcherParameter();
  INLINE PGMouseWatcherParameter(const MouseWatcherParameter &copy);
  INLINE void operator = (const MouseWatcherParameter &copy);
  virtual ~PGMouseWatcherParameter();

PUBLISHED:
  void output(std::ostream &out) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "PGMouseWatcherParameter",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pgMouseWatcherParameter.I"

#endif
