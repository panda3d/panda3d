// Filename: fltGeometry.h
// Created by:  drose (28Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTGEOMETRY_H
#define FLTGEOMETRY_H

#include <pandatoolbase.h>

#include "fltBeadID.h"
#include "fltPackedColor.h"
#include "fltHeader.h"

#include <luse.h>

class FltTexture;
class FltMaterial;

////////////////////////////////////////////////////////////////////
// 	 Class : FltGeometry
// Description : This is a base class for both FltFace and FltMesh,
//               which are two different kinds of geometric primitives
//               that might be encountered in a MultiGen file.  They
//               have similar properties.
////////////////////////////////////////////////////////////////////
class FltGeometry : public FltBeadID {
public:
  FltGeometry(FltHeader *header);

  enum DrawType {
    DT_solid_backface      = 0,
    DT_solid_no_backface   = 1,
    DT_wireframe           = 2,
    DT_wireframe_close     = 3,
    DT_wireframe_highlight = 4,
    DT_omni_light          = 5,
    DT_uni_light           = 6,
    DT_bi_light            = 7
  };

  enum BillboardType {
    BT_none                = 0,
    BT_fixed               = 1,
    BT_axial               = 2,
    BT_point               = 4
  };

  enum Flags {
    F_terrain              = 0x80000000,
    F_no_color             = 0x40000000,
    F_no_alt_color         = 0x20000000,
    F_packed_color         = 0x10000000,
    F_terrain_footprint    = 0x08000000,
    F_hidden               = 0x04000000
  };

  enum LightMode {
    LM_face_no_normal      = 0,
    LM_vertex_no_normal    = 1,
    LM_face_with_normal    = 2,
    LM_vertex_with_normal  = 3
  };

  int _ir_color;
  int _relative_priority;
  DrawType _draw_type;
  bool _texwhite;
  int _color_name_index;
  int _alt_color_name_index;
  BillboardType _billboard_type;
  int _detail_texture_index;
  int _texture_index;
  int _material_index;
  int _dfad_material_code;
  int _dfad_feature_id;
  int _ir_material_code;
  int _transparency;
  int _lod_generation_control;
  int _line_style_index;
  unsigned int _flags;
  LightMode _light_mode;
  FltPackedColor _packed_color;
  FltPackedColor _alt_packed_color;
  int _texture_mapping_index;
  int _color_index;
  int _alt_color_index;

public:
  INLINE bool has_texture() const;
  INLINE FltTexture *get_texture() const;

  INLINE bool has_material() const;
  INLINE FltMaterial *get_material() const;

  INLINE bool has_color() const;
  Colorf get_color() const;
  RGBColorf get_rgb() const;

  bool has_alt_color() const;
  Colorf get_alt_color() const;
  RGBColorf get_alt_rgb() const;


protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltBeadID::init_type();
    register_type(_type_handle, "FltGeometry",
		  FltBeadID::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "fltGeometry.I"

#endif


