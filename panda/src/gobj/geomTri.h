// Filename: geomTri.h
// Created by:  charles (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMTRI_H
#define GEOMTRI_H

#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomTri
// Description : Triangle Primitive
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomTri : public Geom {
public:
  GeomTri() { }
  virtual Geom *make_copy() const;
  virtual void print_draw_immediate() const;
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg) const;

  virtual int get_num_vertices_per_prim() const {
    return 3;
  }
  virtual int get_num_more_vertices_than_components() const {
    return 0;
  }
  virtual bool uses_components() const {
    return false;
  }

  virtual int get_length(int) const {
    return 3;
  }

  virtual PTA_ushort get_tris() const;

public:
  static void register_with_read_factory(void);
  static TypedWriteable *make_GeomTri(const FactoryParams &params);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomTri",
		  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // GEOMTRI_H
