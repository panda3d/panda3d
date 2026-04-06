/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltHeader.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTHEADER_H
#define FLTHEADER_H

#include "pandatoolbase.h"

#include "fltBeadID.h"
#include "fltVertex.h"
#include "fltMaterial.h"
#include "fltTexture.h"
#include "fltLightSourceDefinition.h"
#include "fltEyepoint.h"
#include "fltTrackplane.h"
#include "fltInstanceDefinition.h"

#include "pathReplace.h"
#include "pointerTo.h"
#include "filename.h"
#include "dSearchPath.h"
#include "distanceUnit.h"
#include "pvector.h"
#include "pset.h"
#include "pmap.h"

/**
 * This is the first bead in the file, the top of the bead hierarchy, and the
 * primary interface to reading and writing a Flt file.  You always read a Flt
 * file by creating a header and calling read_flt(), which fills in its
 * children beads automatically; you write a Flt file by creating a header,
 * adding its children, and calling write_flt().
 */
class FltHeader : public FltBeadID {
public:
  FltHeader(PathReplace *path_replace);

  virtual void apply_converted_filenames();

  void set_path_replace(PathReplace *path_replace);
  PathReplace *get_path_replace();
  const PathReplace *get_path_replace() const;
  Filename convert_path(const Filename &orig_filename,
                        const DSearchPath &additional_path = DSearchPath());

  void set_flt_filename(const Filename &flt_filename);
  const Filename &get_flt_filename() const;

  FltError read_flt(Filename filename);
  FltError read_flt(std::istream &in);
  FltError write_flt(Filename filename);
  FltError write_flt(std::ostream &out);

  enum AttrUpdate {
    AU_none,
    AU_if_missing,
    AU_always
  };

  void set_auto_attr_update(AttrUpdate attr);
  AttrUpdate get_auto_attr_update() const;

  enum Units {
    U_meters               = 0,
    U_kilometers           = 1,
    U_feet                 = 4,
    U_inches               = 5,
    U_nautical_miles       = 8
  };

  enum Flags {
    F_save_vertex_normals  = 0x80000000
  };

  enum ProjectionType {
    PT_flat_earth          = 0,
    PT_trapezoidal         = 1,
    PT_round_earth         = 2,
    PT_lambert             = 3,
    PT_utm                 = 4
  };

  enum VertexStorageType {
    VTS_double             = 1
  };

  enum DatabaseOrigin {
    DO_open_flight         = 100,
    DO_dig                 = 200,
    DO_es_ct6              = 300,
    DO_psp                 = 400,
    DO_ge_civ              = 600,
    DO_es_gdf              = 700,
  };

  enum EarthModel {
    EM_wgs84               = 0,
    EM_wgs72               = 1,
    EM_bessel              = 2,
    EM_clarke_1866         = 3,
    EM_nad27               = 4
  };

  int _format_revision_level;
  int _edit_revision_level;
  std::string _last_revision;
  int _next_group_id;
  int _next_lod_id;
  int _next_object_id;
  int _next_face_id;
  int _unit_multiplier;
  Units _vertex_units;
  bool _texwhite_new;
  unsigned int _flags;
  ProjectionType _projection_type;
  int _next_dof_id;
  VertexStorageType _vertex_storage_type;
  DatabaseOrigin _database_origin;
  double _sw_x, _sw_y;
  double _delta_x, _delta_y;
  int _next_sound_id;
  int _next_path_id;
  int _next_clip_id;
  int _next_text_id;
  int _next_bsp_id;
  int _next_switch_id;
  double _sw_lat, _sw_long;
  double _ne_lat, _ne_long;
  double _origin_lat, _origin_long;
  double _lambert_upper_lat, _lambert_lower_lat;
  int _next_light_id;
  int _next_road_id;
  int _next_cat_id;
  EarthModel _earth_model;
  int _next_adaptive_id;
  int _next_curve_id;
  double _delta_z;
  double _radius;
  int _next_mesh_id;

public:
  int get_flt_version() const;
  void set_flt_version(int version);
  static int min_flt_version();
  static int max_flt_version();
  bool check_version() const;

  DistanceUnit get_units() const;

  // Accessors into the instance pool.
  bool has_instance(int instance_index) const;
  FltInstanceDefinition *get_instance(int instance_index) const;
  void clear_instances();
  void add_instance(FltInstanceDefinition *instance);
  void remove_instance(int instance_index);


  // Accessors into the vertex palette.
  int get_num_vertices() const;
  FltVertex *get_vertex(int n) const;
  void clear_vertices();
  void add_vertex(FltVertex *vertex);

  FltVertex *get_vertex_by_offset(int offset);
  int get_offset_by_vertex(FltVertex *vertex);


  // Accessors into the color palette.  This is read-only; why would you want
  // to mess with building a new color palette?
  int get_num_colors() const;
  LColor get_color(int color_index) const;
  LRGBColor get_rgb(int color_index) const;
  bool has_color_name(int color_index) const;
  std::string get_color_name(int color_index) const;

  int get_closest_color(const LColor &color) const;
  int get_closest_rgb(const LRGBColor &color) const;

  int get_num_color_entries() const;
  int get_num_color_shades() const;

  // These functions are mainly used behind-the-scenes to decode the strange
  // forest of color options defined for faces and vertices.
  LColor get_color(int color_index, bool use_packed_color,
                   const FltPackedColor &packed_color,
                   int transparency);
  LRGBColor get_rgb(int color_index, bool use_packed_color,
                    const FltPackedColor &packed_color);

  // Accessors into the material palette.
  bool has_material(int material_index) const;
  FltMaterial *get_material(int material_index) const;
  void clear_materials();
  void add_material(FltMaterial *material);
  void remove_material(int material_index);


  // Accessors into the texture palette.
  bool has_texture(int texture_index) const;
  FltTexture *get_texture(int texture_index) const;
  void clear_textures();
  void add_texture(FltTexture *texture);
  void remove_texture(int texture_index);


  // Accessors into the light source palette.
  bool has_light_source(int light_index) const;
  FltLightSourceDefinition *get_light_source(int light_index) const;
  void clear_light_sources();
  void add_light_source(FltLightSourceDefinition *light_source);
  void remove_light_source(int light_index);


  // Accessors into the eyepointtrackplane palette.
  bool got_eyepoint_trackplane_palette() const;
  void set_eyepoint_trackplane_palette(bool flag);

  int get_num_eyepoints() const;
  FltEyepoint *get_eyepoint(int n);
  int get_num_trackplanes() const;
  FltTrackplane *get_trackplane(int n);

private:
  // Instance subtrees.  These are standalone subtrees, which may be
  // referenced by various points in the hierarchy, stored by instance ID
  // number.
  typedef pmap<int, PT(FltInstanceDefinition)> Instances;
  Instances _instances;


  // Support for the vertex palette.
  int update_vertex_lookups();

  typedef pvector<PT(FltVertex)> Vertices;
  typedef pset<FltVertex *> UniqueVertices;

  typedef pmap<int, FltVertex *> VerticesByOffset;
  typedef pmap<FltVertex *, int> OffsetsByVertex;

  Vertices _vertices;
  UniqueVertices _unique_vertices;
  VerticesByOffset _vertices_by_offset;
  OffsetsByVertex _offsets_by_vertex;

  bool _vertex_lookups_stale;

  // This is maintained while the header is being read, to map the vertices to
  // their corresponding offsets in the vertex palette.
  int _current_vertex_offset;


  // Support for the color palette.
  bool _got_color_palette;
  typedef pvector<FltPackedColor> Colors;
  typedef pmap<int, std::string> ColorNames;
  Colors _colors;
  ColorNames _color_names;


  // Support for the material palette.
  bool _got_14_material_palette;
  typedef pmap<int, PT(FltMaterial)> Materials;
  Materials _materials;
  int _next_material_index;


  // Support for the texture palette.
  AttrUpdate _auto_attr_update;
  typedef pmap<int, PT(FltTexture)> Textures;
  Textures _textures;
  int _next_pattern_index;


  // Support for the light source palette.
  typedef pmap<int, PT(FltLightSourceDefinition)> LightSources;
  LightSources _light_sources;


  // Support for the eyepointtrackplane palette.
  bool _got_eyepoint_trackplane_palette;
  FltEyepoint _eyepoints[10];
  FltTrackplane _trackplanes[10];

  // This pointer is used to resolve references in the flt file.
  PT(PathReplace) _path_replace;
  Filename _flt_filename;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool extract_ancillary(FltRecordReader &reader);

  virtual bool build_record(FltRecordWriter &writer) const;
  virtual FltError write_ancillary(FltRecordWriter &writer) const;

private:
  bool extract_vertex(FltRecordReader &reader);
  bool extract_color_palette(FltRecordReader &reader);
  bool extract_material(FltRecordReader &reader);
  bool extract_14_material_palette(FltRecordReader &reader);
  bool extract_texture(FltRecordReader &reader);
  bool extract_texture_map(FltRecordReader &reader);
  bool extract_light_source(FltRecordReader &reader);
  bool extract_eyepoint_palette(FltRecordReader &reader);

  FltError write_vertex_palette(FltRecordWriter &writer) const;
  FltError write_color_palette(FltRecordWriter &writer) const;
  FltError write_material_palette(FltRecordWriter &writer) const;
  FltError write_texture_palette(FltRecordWriter &writer) const;
  FltError write_light_source_palette(FltRecordWriter &writer) const;
  FltError write_eyepoint_palette(FltRecordWriter &writer) const;

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
    register_type(_type_handle, "FltHeader",
                  FltBeadID::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
