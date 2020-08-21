/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stTerrain.h
 * @author drose
 * @date 2010-10-11
 */

#ifndef STTERRAIN_H
#define STTERRAIN_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "geomVertexData.h"
#include "speedtree_api.h"

/**
 * This is the abstract base class that defines the interface needed to
 * describe a terrain for rendering by SpeedTree.  To use it, you must
 * subclass and override the appropriate virtual methods.  Or, consider just
 * using STBasicTerrain.
 *
 * A terrain is defined as a 2-d height function over all space: get_height(x,
 * y) may be called for any point in space and it should return a reasonable
 * value.  A terrain also provides normal maps and splat maps, as rendered by
 * SpeedTree's Terrain.hlsl shader file.
 */
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
  INLINE PN_stdfloat get_splat_layer_tiling(int n) const;
  INLINE LColor get_splat_layer_color(int n) const;

  INLINE const GeomVertexFormat *get_vertex_format();

  INLINE PN_stdfloat get_min_height() const;
  INLINE PN_stdfloat get_max_height() const;

  virtual PN_stdfloat get_height(PN_stdfloat x, PN_stdfloat y) const=0;
  virtual PN_stdfloat get_smooth_height(PN_stdfloat x, PN_stdfloat y, PN_stdfloat radius) const;
  virtual PN_stdfloat get_slope(PN_stdfloat x, PN_stdfloat y) const;

  bool placement_is_acceptable(PN_stdfloat x, PN_stdfloat y,
                               PN_stdfloat height_min, PN_stdfloat height_max,
                               PN_stdfloat slope_min, PN_stdfloat slope_max);

  virtual void fill_vertices(GeomVertexData *data,
                             PN_stdfloat start_x, PN_stdfloat start_y,
                             PN_stdfloat size_xy, int num_xy) const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

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
    PN_stdfloat _tiling;
    UnalignedLVecBase4 _color;
  };
  typedef pvector<SplatLayer> SplatLayers;

protected:
  bool _is_valid;

  Filename _normal_map;
  Filename _splat_map;
  SplatLayers _splat_layers;

  CPT(GeomVertexFormat) _vertex_format;
  VertexAttribs _st_vertex_attribs;

  PN_stdfloat _min_height;
  PN_stdfloat _max_height;

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

INLINE std::ostream &operator << (std::ostream &out, const STTerrain &terrain) {
  terrain.output(out);
  return out;
}

#include "stTerrain.I"

#endif
