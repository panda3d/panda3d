/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file waitInterval.h
 * @author drose
 * @date 2002-09-12
 */

#ifndef WAITINTERVAL_H
#define WAITINTERVAL_H

#include "directbase.h"
#include "cInterval.h"

/**
 * This interval does absolutely nothing, and is mainly useful for marking
 * time between other intervals within a sequence.
 */
class EXPCL_DIRECT_INTERVAL WaitInterval : public CInterval {
PUBLISHED:
  INLINE explicit WaitInterval(double duration);

  virtual void priv_step(double t);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CInterval::init_type();
    register_type(_type_handle, "WaitInterval",
                  CInterval::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "waitInterval.I"

#endif
