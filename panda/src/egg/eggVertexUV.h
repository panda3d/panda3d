// Filename: eggVertexUV.h
// Created by:  drose (20Jul04)
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

#ifndef EGGVERTEXUV_H
#define EGGVERTEXUV_H

#include "pandabase.h"

#include "eggMorphList.h"
#include "eggNamedObject.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggVertexUV
// Description : The set of UV's that may or may not be assigned to a
//               vertex.  To support multitexturing, there may be
//               multiple sets of UV's on a particular vertex, each
//               with its own name.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggVertexUV : public EggNamedObject {
PUBLISHED:
  EggVertexUV(const string &name, const TexCoordd &uv);
  EggVertexUV(const EggVertexUV &copy);
  EggVertexUV &operator = (const EggVertexUV &copy);
  virtual ~EggVertexUV();

  INLINE const TexCoordd &get_uv() const;
  INLINE void set_uv(const TexCoordd &texCoord);

  void write(ostream &out, int indent_level) const;
  int compare_to(const EggVertexUV &other) const;

  EggMorphTexCoordList _duvs;

private:
  TexCoordd _uv;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNamedObject::init_type();
    register_type(_type_handle, "EggVertexUV",
                  EggNamedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggVertexUV.I"

#endif

