// Filename: geomPolygon.h
// Created by:  charles (13Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMPOLYGON_H
#define GEOMPOLYGON_H

#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomPolygon
// Description : Polygon Primitive
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomPolygon : public Geom {
public:
  GeomPolygon() : Geom() { }
  virtual Geom *make_copy() const;
  virtual void print_draw_immediate() const;
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg) const;

  virtual int get_num_vertices_per_prim() const {
    return 0;
  }
  virtual int get_num_more_vertices_than_components() const {
    return 0;
  }
  virtual bool uses_components() const {
    return false;
  }

  virtual int get_length(int prim) const {
    return _primlengths[prim];
  }

  virtual Geom *explode() const {
    return new GeomPolygon(*this);
  }

public:
  static void register_with_read_factory(void);
  static TypedWriteable *make_GeomPolygon(const FactoryParams &params);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomPolygon",
		  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // GEOMPOLYGON_H
