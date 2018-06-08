/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geoMipTerrain.h
 * @author rdb
 * @date 2007-06-29
 */

#ifndef GEOMIPTERRAIN_H
#define GEOMIPTERRAIN_H

#include "pandabase.h"

#include "luse.h"
#include "pandaNode.h"
#include "pointerTo.h"

#include "pnmImage.h"
#include "nodePath.h"

#include "texture.h"

/**
 * GeoMipTerrain, meaning Panda3D GeoMipMapping, can convert a heightfield
 * image into a 3D terrain, consisting of several GeomNodes.  It uses the
 * GeoMipMapping algorithm, or Geometrical MipMapping, based on the LOD (Level
 * of Detail) algorithm.  For more information about the GeoMipMapping
 * algoritm, see this paper, written by Willem H. de Boer:
 * http://flipcode.com/articles/article_geomipmaps.pdf
 */
class EXPCL_PANDA_GRUTIL GeoMipTerrain : public TypedObject {
PUBLISHED:
  INLINE explicit GeoMipTerrain(const std::string &name);
  INLINE ~GeoMipTerrain();

  INLINE PNMImage &heightfield();
  bool set_heightfield(const Filename &filename, PNMFileType *type = nullptr);
  INLINE bool set_heightfield(const PNMImage &image);
  INLINE PNMImage &color_map();
  INLINE bool set_color_map(const Filename &filename,
                                  PNMFileType *type = nullptr);
  INLINE bool set_color_map(const PNMImage &image);
  INLINE bool set_color_map(const Texture *image);
  INLINE bool set_color_map(const std::string &path);
  INLINE bool has_color_map() const;
  INLINE void clear_color_map();
  void calc_ambient_occlusion(PN_stdfloat radius = 32, PN_stdfloat contrast = 2.0f, PN_stdfloat brightness = 0.75f);
  double get_elevation(double x, double y);
  LVector3 get_normal(int x, int y);
  INLINE LVector3 get_normal(unsigned short mx, unsigned short my,
                                                          int x,int y);
  INLINE void set_bruteforce(bool bf);
  INLINE bool get_bruteforce();

  // The flatten mode specifies whether the terrain nodes are flattened
  // together after each terrain update.
  enum AutoFlattenMode {
    // FM_off: don't ever flatten the terrain.
    AFM_off     = 0,
    // FM_light: the terrain is flattened using flatten_light.
    AFM_light   = 1,
    // FM_medium: the terrain is flattened using flatten_medium.
    AFM_medium  = 2,
    // FM_strong: the terrain is flattened using flatten_strong.
    AFM_strong  = 3,
  };

  INLINE void set_auto_flatten(int mode);

  // The focal point is the point at which the terrain will have the highest
  // quality (lowest level of detail). Parts farther away from the focal point
  // will have a lower quality (higher level of detail). The focal point is
  // not taken in respect if bruteforce is set true.
  INLINE void set_focal_point(const LPoint2d &fp);
  INLINE void set_focal_point(const LPoint2f &fp);
  INLINE void set_focal_point(const LPoint3d &fp);
  INLINE void set_focal_point(const LPoint3f &fp);
  INLINE void set_focal_point(double x, double y);
  INLINE void set_focal_point(NodePath fnp);
  INLINE NodePath get_focal_point() const;
  INLINE NodePath get_root() const;

  INLINE void set_block_size(unsigned short newbs);
  INLINE unsigned short get_block_size();
  INLINE unsigned short get_max_level();
  INLINE void set_min_level(unsigned short minlevel);
  INLINE unsigned short get_min_level();
  INLINE bool is_dirty();
  INLINE void set_factor(PN_stdfloat factor);
  INLINE void set_near_far(double input_near, double input_far);
  INLINE void set_near(double input_near);
  INLINE void set_far(double input_far);
  INLINE const NodePath get_block_node_path(unsigned short mx,
                                            unsigned short my);
  INLINE LVecBase2 get_block_from_pos(double x, double y);
  INLINE void set_border_stitching(bool stitching);
  INLINE bool get_border_stitching();
  INLINE double get_far();
  INLINE double get_near();
  INLINE int get_flatten_mode();

  PNMImage make_slope_image();
  void generate();
  bool update();

private:

  PT(GeomNode) generate_block(unsigned short mx, unsigned short my, unsigned short level);
  bool update_block(unsigned short mx, unsigned short my,
                    signed short level = -1, bool forced = false);
  void calc_levels();
  void auto_flatten();
  bool root_flattened();

  INLINE bool is_power_of_two(unsigned int i);
  INLINE float f_part(float i);
  INLINE double f_part(double i);
  INLINE int sfav(int n, int powlevel, int mypowlevel);
  INLINE double get_pixel_value(int x, int y);
  INLINE double get_pixel_value(unsigned short mx, unsigned short my, int x, int y);
  INLINE unsigned short lod_decide(unsigned short mx, unsigned short my);
  unsigned short get_neighbor_level(unsigned short mx, unsigned short my, short dmx, short dmy);

  NodePath _root;
  int _auto_flatten;
  bool _root_flattened;
  PNMImage _heightfield;
  PNMImage _color_map;
  bool _is_dirty;
  bool _has_color_map;
  unsigned int _xsize;
  unsigned int _ysize;
  PN_stdfloat _factor;
  double _near;
  double _far;
  bool _use_near_far; // False to use the _factor, True to use the _near and _far values.
  unsigned short _block_size;
  unsigned short _max_level; // Highest level possible for this block size
  bool _bruteforce;
  NodePath _focal_point;
  bool _focal_is_temporary;
  unsigned short _min_level;
  bool _stitching;
  pvector<pvector<NodePath> > _blocks;
  pvector<pvector<unsigned short> > _levels;
  pvector<pvector<unsigned short> > _old_levels;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "GeoMipTerrain",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "geoMipTerrain.I"

#endif /*GEOMIPTERRAIN_H*/
