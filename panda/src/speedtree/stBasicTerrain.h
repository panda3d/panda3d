/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stBasicTerrain.h
 * @author drose
 * @date 2010-10-12
 */

#ifndef STBASICTERRAIN_H
#define STBASICTERRAIN_H

#include "pandabase.h"
#include "stTerrain.h"
#include "luse.h"
#include "pvector.h"

/**
 * A specific implementation of STTerrain that supports basic heightmaps
 * loaded from an image file, as described in a terrain.txt file similar to
 * those provided with the SpeedTree example application.
 */
class EXPCL_PANDASPEEDTREE STBasicTerrain : public STTerrain {
PUBLISHED:
  STBasicTerrain();
  STBasicTerrain(const STBasicTerrain &copy);
  virtual ~STBasicTerrain();

  void clear();

  bool setup_terrain(const Filename &terrain_filename);
  bool setup_terrain(std::istream &in, const Filename &pathname);

  INLINE void set_height_map(const Filename &height_map);
  INLINE const Filename &get_height_map() const;

  virtual void load_data();

  INLINE PN_stdfloat get_size() const;
  virtual PN_stdfloat get_height(PN_stdfloat x, PN_stdfloat y) const;
  virtual PN_stdfloat get_smooth_height(PN_stdfloat x, PN_stdfloat y, PN_stdfloat radius) const;
  virtual PN_stdfloat get_slope(PN_stdfloat x, PN_stdfloat y) const;

  virtual void fill_vertices(GeomVertexData *data,
                             PN_stdfloat start_x, PN_stdfloat start_y,
                             PN_stdfloat size_xy, int num_xy) const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

protected:
  bool read_height_map();
  void compute_slope(PN_stdfloat smoothing);

  INLINE PN_stdfloat interpolate(PN_stdfloat a, PN_stdfloat b, PN_stdfloat t);

private:
  static void read_quoted_filename(Filename &result, std::istream &in,
                                   const Filename &dirname);

protected:
  template<class ValueType>
  class InterpolationData {
  public:
    InterpolationData();
    void reset(int width, int height);

    ValueType get_nearest_neighbor(PN_stdfloat u, PN_stdfloat v) const;
    ValueType calc_bilinear_interpolation(PN_stdfloat u, PN_stdfloat v) const;
    ValueType calc_smooth(PN_stdfloat u, PN_stdfloat v, PN_stdfloat radius) const;
    bool is_present() const;

    int _width;
    int _height;
    pvector<ValueType> _data;
  };

protected:
  Filename _height_map;
  PN_stdfloat _size;
  PN_stdfloat _height_scale;

  InterpolationData<PN_stdfloat> _height_data;
  // InterpolationData<LVector3> _normal_data;
  InterpolationData<PN_stdfloat> _slope_data;
  // InterpolationData<unsigned char> _ao_data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    STTerrain::init_type();
    register_type(_type_handle, "STBasicTerrain",
                  STTerrain::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "stBasicTerrain.I"

#endif
