// Filename: geomQuad.h
// Created by:  charles (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMQUAD_H
#define GEOMQUAD_H

#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomQuad
// Description : Quadrilateral Primitive
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomQuad : public Geom {
public:
  GeomQuad() { }
  virtual Geom *make_copy() const;
  virtual void print_draw_immediate() const;
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg) const;

  virtual int get_num_vertices_per_prim() const {
    return 4;
  }
  virtual int get_num_more_vertices_than_components() const {
    return 0;
  }
  virtual bool uses_components() const {
    return false;
  }

  virtual int get_length(int) const {
    return 4;
  }

  virtual PTA_ushort get_tris() const;

public:
  static void register_with_read_factory(void);
  static TypedWriteable *make_GeomQuad(const FactoryParams &params);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomQuad",
		  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // GEOMQUAD_H
