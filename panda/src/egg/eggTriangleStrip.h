// Filename: eggTriangleStrip.h
// Created by:  drose (13Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGTRIANGLESTRIP_H
#define EGGTRIANGLESTRIP_H

#include "pandabase.h"

#include "eggCompositePrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggTriangleStrip
// Description : A connected strip of triangles.  This does not
//               normally appear in an egg file; it is typically
//               generated as a result of meshing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggTriangleStrip : public EggCompositePrimitive {
PUBLISHED:
  INLINE EggTriangleStrip(const string &name = "");
  INLINE EggTriangleStrip(const EggTriangleStrip &copy);
  INLINE EggTriangleStrip &operator = (const EggTriangleStrip &copy);

  virtual void write(ostream &out, int indent_level) const;

protected:
  virtual bool do_triangulate(EggGroupNode *container) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggCompositePrimitive::init_type();
    register_type(_type_handle, "EggTriangleStrip",
                  EggCompositePrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggTriangleStrip.I"

#endif
