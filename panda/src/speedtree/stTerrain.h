// Filename: stTerrain.h
// Created by:  drose (11Oct10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef STTERRAIN_H
#define STTERRAIN_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "geomVertexData.h"
#include "speedtree_api.h"

////////////////////////////////////////////////////////////////////
//       Class : STTerrain
// Description : This is the abstract base class that defines the
//               interface needed to describe a terrain for rendering
//               by SpeedTree.  To use it, you must subclass and
//               override the appropriate virtual methods.  Or,
//               consider just using STBasicTerrain.
//
//               A terrain is defined as a 2-d height function over
//               all space: get_height(x, y) may be called for any
//               point in space and it should return a reasonable
//               value.  A terrain also provides normal maps and splat
//               maps, as rendered by SpeedTree's Terrain.hlsl shader
//               file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDASPEEDTREE STTerrain : public TypedReferenceCount, public Namable {
protected:  
  STTerrain();
  STTerrain(const STTerrain &copy);
PUBLISHED:
  virtual ~STTerrain();

  virtual void clear();
  virtual void load_data()=0;

  INLINE bool is_valid() const;

  INLINE const Filename &get_normal_map() const;
  INLINE const Filename &get_splat_map() const;

  INLINE int get_num_splat_layers() const;
  INLINE const Filename &get_splat_layer(int n) const;
  INLINE float get_splat_layer_tiling(int n) const;
  INLINE const LVecBase4f &get_splat_layer_color(int n) const;

  INLINE const GeomVertexFormat *get_vertex_format();

  INLINE float get_min_height() const;
  INLINE float get_max_height() const;

  virtual float get_height(float x, float y) const=0;
  virtual float get_smooth_height(float x, float y, float radius) const;
  virtual float get_slope(float x, float y) const;

  bool placement_is_acceptable(float x, float y,
			       float height_min, float height_max, 
			       float slope_min, float slope_max);

  virtual void fill_vertices(GeomVertexData *data,
			     float start_x, float start_y,
			     float size_xy, int num_xy) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

public:
  const SpeedTree::SVertexAttribDesc *get_st_vertex_format() const;

protected:
  bool set_vertex_format(const GeomVertexFormat *format);

  typedef pvector<SpeedTree::SVertexAttribDesc> VertexAttribs;
  static bool convert_vertex_format(VertexAttribs &st_vertex_attribs,
				    const GeomVertexFormat *format);
  static bool convert_vertex_column(SpeedTree::SVertexAttribDesc &st_attrib,
				    const GeomVertexColumn *column);

protected:
  class SplatLayer {
  public:
    Filename _filename;
    float _tiling;
    LVecBase4f _color;
  };
  typedef pvector<SplatLayer> SplatLayers;

protected:
  bool _is_valid;

  Filename _normal_map;
  Filename _splat_map;
  SplatLayers _splat_layers;

  CPT(GeomVertexFormat) _vertex_format;
  VertexAttribs _st_vertex_attribs;

  float _min_height;
  float _max_height;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "STTerrain",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const STTerrain &terrain) {
  terrain.output(out);
  return out;
}

#include "stTerrain.I"

#endif
