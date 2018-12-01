/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomDrawCallbackData.h
 * @author drose
 * @date 2009-03-13
 */

#ifndef GEOMDRAWCALLBACKDATA_H
#define GEOMDRAWCALLBACKDATA_H

#include "pandabase.h"
#include "callbackData.h"

// Forward declarations
class CullableObject;
class GraphicsStateGuardianBase;

/**
 * This specialization on CallbackData is passed when the callback is
 * initiated from deep within the draw traversal, for a particular Geom.
 */
class EXPCL_PANDA_PGRAPH GeomDrawCallbackData : public CallbackData {
public:
  INLINE GeomDrawCallbackData(CullableObject *obj,
                              GraphicsStateGuardianBase *gsg, bool force);

PUBLISHED:
  virtual void output(std::ostream &out) const;

  INLINE CullableObject *get_object() const;
  INLINE GraphicsStateGuardianBase *get_gsg() const;
  INLINE bool get_force() const;

  INLINE void set_lost_state(bool lost_state);
  INLINE bool get_lost_state() const;

  virtual void upcall();

private:
  CullableObject *_obj;
  GraphicsStateGuardianBase *_gsg;
  bool _force;
  bool _lost_state;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CallbackData::init_type();
    register_type(_type_handle, "GeomDrawCallbackData",
                  CallbackData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "geomDrawCallbackData.I"

#endif
