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
  EggVertexUV(const string &name, const TexCoord3d &uvw);
  EggVertexUV(const EggVertexUV &copy);
  EggVertexUV &operator = (const EggVertexUV &copy);
  virtual ~EggVertexUV();

  INLINE void set_name(const string &name);

  INLINE int get_num_dimensions() const;
  INLINE bool has_w() const;
  INLINE TexCoordd get_uv() const;
  INLINE const TexCoord3d &get_uvw() const;
  INLINE void set_uv(const TexCoordd &texCoord);
  INLINE void set_uvw(const TexCoord3d &texCoord);

  INLINE bool has_tangent() const;
  INLINE const Normald &get_tangent() const;
  INLINE void set_tangent(const Normald &tangent);
  INLINE void clear_tangent();

  INLINE bool has_binormal() const;
  INLINE const Normald &get_binormal() const;
  INLINE void set_binormal(const Normald &binormal);
  INLINE void clear_binormal();

  void transform(const LMatrix4d &mat);

  void write(ostream &out, int indent_level) const;
  int compare_to(const EggVertexUV &other) const;

  EggMorphTexCoordList _duvs;

private:
  enum Flags {
    F_has_tangent   = 0x001,
    F_has_binormal  = 0x002,
    F_has_w         = 0x004,
  };

  int _flags;
  Normald _tangent;
  Normald _binormal;
  TexCoord3d _uvw;

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

