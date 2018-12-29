/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stateMunger.h
 * @author drose
 * @date 2005-05-04
 */

#ifndef STATEMUNGER_H
#define STATEMUNGER_H

#include "pandabase.h"
#include "geomMunger.h"
#include "renderState.h"

/**
 * This is just a simple derivative of GeomMunger that adds the ability to
 * munge states.  That functionality can't be declared in the base class,
 * since it doesn't really know about RenderState.
 */
class EXPCL_PANDA_PGRAPH StateMunger : public GeomMunger {
public:
  INLINE StateMunger(GraphicsStateGuardianBase *gsg);
  virtual ~StateMunger();
  CPT(RenderState) munge_state(const RenderState *state);

  INLINE bool should_munge_state() const;

protected:
  virtual CPT(RenderState) munge_state_impl(const RenderState *state);

  bool _should_munge_state;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomMunger::init_type();
    register_type(_type_handle, "StateMunger",
                  GeomMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "stateMunger.I"

#endif
