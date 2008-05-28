// Filename: eggTriangleFan.h
// Created by:  drose (23Mar05)
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

#ifndef EGGTRIANGLEFAN_H
#define EGGTRIANGLEFAN_H

#include "pandabase.h"

#include "eggCompositePrimitive.h"

////////////////////////////////////////////////////////////////////
//       Class : EggTriangleFan
// Description : A connected fan of triangles.  This does not
//               normally appear in an egg file; it is typically
//               generated as a result of meshing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggTriangleFan : public EggCompositePrimitive {
PUBLISHED:
  INLINE EggTriangleFan(const string &name = "");
  INLINE EggTriangleFan(const EggTriangleFan &copy);
  INLINE EggTriangleFan &operator = (const EggTriangleFan &copy);
  virtual ~EggTriangleFan();

  virtual void write(ostream &out, int indent_level) const;
  virtual void apply_first_attribute();

protected:
  virtual int get_num_lead_vertices() const;
  virtual bool do_triangulate(EggGroupNode *container) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggCompositePrimitive::init_type();
    register_type(_type_handle, "EggTriangleFan",
                  EggCompositePrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggTriangleFan.I"

#endif
