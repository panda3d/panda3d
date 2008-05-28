// Filename: eggSurface.h
// Created by:  drose (15Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef EGGSURFACE_H
#define EGGSURFACE_H

#include "pandabase.h"

#include "eggPrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggSurface
// Description : A parametric surface of some kind.  See
//               EggNurbsSurface.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggSurface : public EggPrimitive {
PUBLISHED:
  INLINE EggSurface(const string &name = "");
  INLINE EggSurface(const EggSurface &copy);
  INLINE EggSurface &operator = (const EggSurface &copy);

  INLINE void set_u_subdiv(int subdiv);
  INLINE int get_u_subdiv() const;
  INLINE void set_v_subdiv(int subdiv);
  INLINE int get_v_subdiv() const;

private:
  int _u_subdiv;
  int _v_subdiv;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggSurface",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggSurface.I"

#endif
