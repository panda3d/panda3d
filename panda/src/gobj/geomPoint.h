// Filename: geomPoint.h
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

#ifndef GEOMPOINT_H
#define GEOMPOINT_H

#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomPoint
// Description : Point Primitive
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomPoint : public Geom {
public:
  GeomPoint() : Geom() { _size = 1.0f; }
  virtual Geom *make_copy() const;

  virtual void print_draw_immediate() const;
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc);

  virtual int get_num_vertices_per_prim() const {
    return 1;
  }
  virtual int get_num_more_vertices_than_components() const {
    return 0;
  }
  virtual bool uses_components() const {
    return false;
  }

  virtual int get_length(int) const {
    return 1;
  }

  virtual Geom *explode() const {
    return new GeomPoint(*this);
  }

  INLINE void set_size(float size) { _size = size; }
  INLINE float get_size(void) const { return _size; }

protected:
  float _size;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_GeomPoint(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomPoint",
                  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // GEOMPOINT_H
