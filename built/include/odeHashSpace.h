/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeHashSpace.h
 * @author joswilso
 * @date 2006-12-27
 */

#ifndef ODEHASHSPACE_H
#define ODEHASHSPACE_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeSpace.h"


/**
 *
 */
class EXPCL_PANDAODE OdeHashSpace : public OdeSpace {
  friend class OdeSpace;
  friend class OdeGeom;

public:
  OdeHashSpace(dSpaceID id);

PUBLISHED:
  OdeHashSpace();
  OdeHashSpace(OdeSpace &space);
  virtual ~OdeHashSpace();

  INLINE void set_levels(int minlevel, int maxlevel);
  INLINE int get_min_level() const;
  INLINE int get_max_level() const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeSpace::init_type();
    register_type(_type_handle, "OdeHashSpace",
                  OdeSpace::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeHashSpace.I"

#endif
