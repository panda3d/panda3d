// Filename: geomSprite.h
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

#ifndef GEOMSPRITE_H
#define GEOMSPRITE_H

#include "pta_float.h"
#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomSprite
// Description : Sprite Primitive
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomSprite : public Geom {
public:
  GeomSprite(Texture *tex = (Texture *) NULL,
             bool alpha_disable = false);

  virtual Geom *make_copy() const;
  virtual void print_draw_immediate() const;
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc);

  virtual int get_num_vertices_per_prim() const { return 1; }
  virtual int get_num_more_vertices_than_components() const { return 0; }
  virtual bool uses_components() const { return false; }
  virtual int get_length(int) const { return 1; }

  virtual Geom *explode() const {
    return new GeomSprite(*this); }

  static float get_frustum_top() { return 1.0f; }
  static float get_frustum_bottom() { return -1.0f; }
  static float get_frustum_left() { return -1.0f; }
  static float get_frustum_right() { return 1.0f; }

  INLINE void set_texture(Texture *tex);
  INLINE Texture *get_texture() const;

  INLINE void set_ll_uv(const TexCoordf &ll_uv);
  INLINE const TexCoordf &get_ll_uv() const;

  INLINE void set_ur_uv(const TexCoordf &ur_uv);
  INLINE const TexCoordf &get_ur_uv() const;

  INLINE void set_alpha_disable(bool a);
  INLINE bool get_alpha_disable() const;

  INLINE void set_x_texel_ratio(PTA_float x_texel_ratio, GeomBindType x_bind_type);
  INLINE void set_y_texel_ratio(PTA_float y_texel_ratio, GeomBindType y_bind_type);
  INLINE void set_thetas(PTA_float theta, GeomBindType theta_bind_type);

  INLINE GeomBindType get_x_bind_type() const;
  INLINE GeomBindType get_y_bind_type() const;
  INLINE GeomBindType get_theta_bind_type() const;

  // public so we don't have to issue them...
  PTA_float _x_texel_ratio;
  PTA_float _y_texel_ratio;
  PTA_float _theta;

protected:
  PT(Texture) _texture;
  TexCoordf _ll_uv;
  TexCoordf _ur_uv;

  bool _alpha_disable;

  GeomBindType _x_bind_type;
  GeomBindType _y_bind_type;
  GeomBindType _theta_bind_type;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

  int complete_pointers(TypedWritable **plist, BamReader *manager);
  static TypedWritable *make_GeomSprite(const FactoryParams &params);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomSprite",
                  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "geomSprite.I"

#endif // GEOMSPRITE_H
