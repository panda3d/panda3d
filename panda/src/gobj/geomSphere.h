// Filename: geomSphere.h
// Created by:  drose (01Oct99)
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

#ifndef GEOMSPHERE_H
#define GEOMSPHERE_H

#include "pandabase.h"

#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomSphere
// Description : A sphere primitive.  This isn't actually a primitive
//               under OpenGL (or probably most backends), but it's
//               handy to be able to render a sphere from time to
//               time, and it may be a primitive on some backends.
//               This will render the best way the backend knows how
//               to render a sphere.
//
//               This structure contains a sequence of spheres.  Each
//               two consecutive vertices defines a sphere, where the
//               first vertex represents the center, and the second
//               vertex represents a point on the surface.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomSphere : public Geom {
public:
  GeomSphere() : Geom() { }
  virtual Geom *make_copy() const;
  virtual void print_draw_immediate() const { }
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc);

  virtual int get_num_vertices_per_prim() const {
    return 2;
  }
  virtual int get_num_more_vertices_than_components() const {
    return 0;
  }
  virtual bool uses_components() const {
    return false;
  }

  virtual int get_length(int) const {
    return 2;
  }

  virtual Geom *explode() const {
    return new GeomSphere(*this);
  }

public:
  static void register_with_read_factory(void);

  static TypedWritable *make_GeomSphere(const FactoryParams &params);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomSphere",
                  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
