// Filename: geomTrifan.h
// Created by:  charles (13Jul00)
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

#ifndef GEOMTRIFAN_H
#define GEOMTRIFAN_H

#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomTrifan
// Description : Triangle Fan Primitive
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomTrifan : public Geom
{
public:
  GeomTrifan( void ) : Geom() { }
  virtual Geom *make_copy() const;
  virtual void print_draw_immediate( void ) const;
  int get_num_tris( void ) const;
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc);

  virtual int get_num_vertices_per_prim() const {
    return 0;
  }
  virtual int get_num_more_vertices_than_components() const {
    return 2;
  }
  virtual bool uses_components() const {
    return true;
  }

  virtual int get_length(int prim) const {
    return _primlengths[prim];
  }

  virtual Geom *explode() const;
  virtual PTA_ushort get_tris() const;

public:
  static void register_with_read_factory(void);
  static TypedWritable *make_GeomTrifan(const FactoryParams &params);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomTrifan",
                  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // GEOMTRIFAN_H
