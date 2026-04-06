/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltTexture.h
 * @author drose
 * @date 2000-08-25
 */

#ifndef FLTTEXTURE_H
#define FLTTEXTURE_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "filename.h"
#include "luse.h"

/**
 * Represents a single texture in the texture palette.
 */
class FltTexture : public FltRecord {
public:
  FltTexture(FltHeader *header);

  virtual void apply_converted_filenames();

  std::string _orig_filename;
  Filename _converted_filename;
  int _pattern_index;
  int _x_location;
  int _y_location;

  Filename get_texture_filename() const;
  void set_texture_filename(const Filename &filename);
  Filename get_attr_filename() const;
  FltError read_attr_data();
  FltError write_attr_data() const;
  FltError write_attr_data(Filename attr_filename) const;

  // The remaining fields are from the attr file.
  enum FileFormat {
    FF_none             = -1,
    FF_att_8_pattern    = 0,
    FF_att_8_template   = 1,
    FF_sgi_i            = 2,
    FF_sgi_ia           = 3,
    FF_sgi_rgb          = 4,
    FF_sgi_rgba         = 5
  };

  enum Minification {
    MN_point            = 0,
    MN_bilinear         = 1,
    MN_OB_mipmap        = 2,  // obsolete
    MN_mipmap_point     = 3,
    MN_mipmap_linear    = 4,
    MN_mipmap_bilinear  = 5,
    MN_mipmap_trilinear = 6,
    MN_bicubic          = 8,
    MN_bilinear_gequal  = 9,
    MN_bilinear_lequal  = 10,
    MN_bicubic_gequal   = 11,
    MN_bicubic_lequal   = 12
  };

  enum Magnification {
    MG_point            = 0,
    MG_bilinear         = 1,
    MG_bicubic          = 3,
    MG_sharpen          = 4,
    MG_add_detail       = 5,
    MG_modulate_detail  = 6,
    MG_bilinear_gequal  = 7,
    MG_bilinear_lequal  = 8,
    MG_bicubic_gequal   = 9,
    MG_bicubic_lequal   = 10
  };

  enum RepeatType {
    RT_repeat           = 0,
    RT_clamp            = 1
  };

  enum EnvironmentType {
    ET_modulate         = 0,
    ET_blend            = 1,
    ET_decal            = 2,
    ET_color            = 3
  };

  enum InternalFormat {
    IF_default          = 0,
    IF_i_12a_4          = 1,
    IF_ia_8             = 2,
    IF_rgb_5            = 3,
    IF_rgba_4           = 4,
    IF_ia_12            = 5,
    IF_rgba_8           = 6,
    IF_rgba_12          = 7,
    IF_i_16             = 8,  // shadow mode only
    IF_rgb_12           = 9
  };

  enum ExternalFormat {
    EF_default          = 0,
    EF_pack_8           = 1,
    EF_pack_16          = 2
  };

  enum ProjectionType {
    PT_flat_earth       = 0,
    PT_lambert          = 3,
    PT_utm              = 4,
    PT_undefined        = 7
  };

  enum EarthModel {
    EM_wgs84            = 0,
    EM_wgs72            = 1,
    EM_bessel           = 2,
    EM_clarke_1866      = 3,
    EM_nad27            = 4
  };

  enum ImageOrigin {
    IO_lower_left       = 0,
    IO_upper_left       = 1
  };

  enum PointsUnits {
    PU_degrees          = 0,
    PU_meters           = 1,
    PU_pixels           = 2
  };

  enum Hemisphere {
    H_southern          = 0,
    H_northern          = 1,
  };

  struct LODScale {
    PN_stdfloat _lod;
    PN_stdfloat _scale;
  };

  struct GeospecificControlPoint {
    LPoint2d _uv;
    LPoint2d _real_earth;
  };

  typedef pvector<GeospecificControlPoint> GeospecificControlPoints;

  struct SubtextureDef {
    std::string _name;
    int _left;
    int _bottom;
    int _right;
    int _top;
  };
  typedef pvector<SubtextureDef> SubtextureDefs;

  int _num_texels_u;
  int _num_texels_v;
  int _real_world_size_u;
  int _real_world_size_v;
  int _up_vector_x;
  int _up_vector_y;
  FileFormat _file_format;
  Minification _min_filter;
  Magnification _mag_filter;
  RepeatType _repeat;
  RepeatType _repeat_u;
  RepeatType _repeat_v;
  int _modify_flag;
  int _x_pivot_point;
  int _y_pivot_point;
  EnvironmentType _env_type;
  bool _intensity_is_alpha; // if true, a one-channel image is actually
                            // an alpha image, not an intensity image.
  double _float_real_world_size_u;
  double _float_real_world_size_v;
  int _imported_origin_code;
  int _kernel_version;
  InternalFormat _internal_format;
  ExternalFormat _external_format;
  bool _use_mipmap_kernel;
  PN_stdfloat _mipmap_kernel[8];
  bool _use_lod_scale;
  LODScale _lod_scale[8];
  PN_stdfloat _clamp;
  Magnification _mag_filter_alpha;
  Magnification _mag_filter_color;
  double _lambert_conic_central_meridian;
  double _lambert_conic_upper_latitude;
  double _lambert_conic_lower_latitude;
  bool _use_detail;
  int _detail_j;
  int _detail_k;
  int _detail_m;
  int _detail_n;
  int _detail_scramble;
  bool _use_tile;
  PN_stdfloat _tile_lower_left_u;
  PN_stdfloat _tile_lower_left_v;
  PN_stdfloat _tile_upper_right_u;
  PN_stdfloat _tile_upper_right_v;
  ProjectionType _projection;
  EarthModel _earth_model;
  int _utm_zone;
  ImageOrigin _image_origin;
  PointsUnits _geospecific_points_units;
  Hemisphere _geospecific_hemisphere;
  std::string _comment;
  int _file_version;
  GeospecificControlPoints _geospecific_control_points;
  SubtextureDefs _subtexture_defs;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

private:
  FltError unpack_attr(const Datagram &datagram);
  FltError pack_attr(Datagram &datagram) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltRecord::init_type();
    register_type(_type_handle, "FltTexture",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltHeader;
};

#endif
