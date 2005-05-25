// Filename: lens.h
// Created by:  drose (18Feb99)
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

#ifndef LENS_H
#define LENS_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "luse.h"
#include "geom.h"
#include "updateSeq.h"
#include "qpgeomVertexData.h"
#include "pointerTo.h"

class BoundingVolume;

////////////////////////////////////////////////////////////////////
//       Class : Lens
// Description : A base class for any number of different kinds of
//               lenses, linear and otherwise.  Presently, this
//               includes perspective and orthographic lenses.
//
//               A Lens object is the main part of a Camera node
//               (defined in sgraph), which defines the fundamental
//               interface to point-of-view for rendering.  Lenses are
//               also used in other contexts, however; for instance, a
//               Spotlight is also defined using a lens.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Lens : public TypedWritableReferenceCount {
public:
  Lens();
  Lens(const Lens &copy);
  void operator = (const Lens &copy);

PUBLISHED:
  virtual PT(Lens) make_copy() const=0;

  INLINE bool extrude(const LPoint2f &point2d,
                      LPoint3f &near_point, LPoint3f &far_point) const;
  INLINE bool extrude(const LPoint3f &point2d,
                      LPoint3f &near_point, LPoint3f &far_point) const;
  INLINE bool extrude_vec(const LPoint2f &point2d, LVector3f &vec3d) const;
  INLINE bool extrude_vec(const LPoint3f &point2d, LVector3f &vec3d) const;
  INLINE bool project(const LPoint3f &point3d, LPoint3f &point2d) const;
  INLINE bool project(const LPoint3f &point3d, LPoint2f &point2d) const;

  INLINE void set_change_event(const string &event);
  INLINE const string &get_change_event() const;

  void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;

  void clear();

  void set_film_size(float width);
  INLINE void set_film_size(float width, float height);
  void set_film_size(const LVecBase2f &film_size);
  const LVecBase2f &get_film_size() const;

  INLINE void set_film_offset(float x, float y);
  INLINE void set_film_offset(const LVecBase2f &film_offset);
  INLINE const LVector2f &get_film_offset() const;

  void set_focal_length(float focal_length);
  float get_focal_length() const;

  void set_fov(float fov);
  INLINE void set_fov(float hfov, float vfov);
  void set_fov(const LVecBase2f &fov);
  const LVecBase2f &get_fov() const;
  INLINE float get_hfov() const;
  INLINE float get_vfov() const;

  void set_aspect_ratio(float aspect_ratio);
  float get_aspect_ratio() const;

  INLINE void set_near(float near_distance);
  INLINE float get_near() const;
  INLINE void set_far(float far_distance);
  INLINE float get_far() const;
  INLINE void set_near_far(float near_distance, float far_distance);

  static float get_default_near();
  static float get_default_far();
  
  INLINE void set_view_hpr(float h, float p, float r);
  void set_view_hpr(const LVecBase3f &view_hpr);
  const LVecBase3f &get_view_hpr() const;
  INLINE void set_view_vector(float x, float y, float z, float i, float j, float k);
  void set_view_vector(const LVector3f &view_vector, const LVector3f &up_vector);
  const LVector3f &get_view_vector() const;
  const LVector3f &get_up_vector() const;
  LPoint3f get_nodal_point() const;
  void set_iod_offset(float offset);
  float get_iod_offset() const;

  void set_view_mat(const LMatrix4f &view_mat);
  const LMatrix4f &get_view_mat() const;
  void clear_view_mat();

  void set_keystone(const LVecBase2f &keystone);
  INLINE const LVecBase2f &get_keystone() const;
  void clear_keystone();
  
  // These flags are passed in as the last parameter to control the
  // behavior of set_frustum_from_corners().  See the documentation
  // for that method for an explanation of each flag.
  enum FromCorners {
    FC_roll         = 0x0001,
    FC_camera_plane = 0x0002,
    FC_off_axis     = 0x0004,
    FC_aspect_ratio = 0x0008,
    FC_shear        = 0x0010,
    FC_keystone     = 0x0020,
  };
  void set_frustum_from_corners(const LVecBase3f &ul, const LVecBase3f &ur,
                                const LVecBase3f &ll, const LVecBase3f &lr,
                                int flags);

  void recompute_all();

  virtual bool is_linear() const;
  virtual bool is_perspective() const;
  virtual bool is_orthographic() const;
  virtual PT(Geom) make_geometry();

  virtual PT(BoundingVolume) make_bounds() const;

  const LMatrix4f &get_projection_mat() const;
  const LMatrix4f &get_projection_mat_inv() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

public:
  INLINE const UpdateSeq &get_last_change() const;

protected:
  INLINE void adjust_user_flags(int clear_flags, int set_flags);
  INLINE void adjust_comp_flags(int clear_flags, int set_flags);

  void throw_change_event();

  const LMatrix4f &get_film_mat() const;
  const LMatrix4f &get_film_mat_inv() const;

  const LMatrix4f &get_lens_mat() const;
  const LMatrix4f &get_lens_mat_inv() const;

  virtual bool extrude_impl(const LPoint3f &point2d,
                            LPoint3f &near_point, LPoint3f &far_point) const;
  virtual bool extrude_vec_impl(const LPoint3f &point2d, LVector3f &vec) const;
  virtual bool project_impl(const LPoint3f &point3d, LPoint3f &point2d) const;

  virtual void compute_film_size();
  virtual void compute_focal_length();
  virtual void compute_fov();
  virtual void compute_aspect_ratio();
  virtual void compute_view_hpr();
  virtual void compute_view_vector();
  virtual void compute_iod_offset();
  virtual void compute_projection_mat();
  virtual void compute_film_mat();
  virtual void compute_lens_mat();

  virtual float fov_to_film(float fov, float focal_length, bool horiz) const;
  virtual float fov_to_focal_length(float fov, float film_size, bool horiz) const;
  virtual float film_to_fov(float film_size, float focal_length, bool horiz) const;

private:
  static void resequence_fov_triad(char &newest, char &older_a, char &older_b);
  int define_geom_coords();
  int define_geom_data();
  static void build_shear_mat(LMatrix4f &shear_mat,
                              const LPoint3f &cul, const LPoint3f &cur,
                              const LPoint3f &cll, const LPoint3f &clr);
  static float sqr_dist_to_line(const LPoint3f &point, const LPoint3f &origin, 
                                const LVector3f &vec);

protected:
  string _change_event;
  UpdateSeq _last_change;
  CoordinateSystem _cs;

  LVecBase2f _film_size;
  LVector2f _film_offset;
  float _focal_length;
  LVecBase2f _fov;
  float _aspect_ratio;
  float _near_distance, _far_distance;

  LVecBase3f _view_hpr;
  LVector3f _view_vector, _up_vector;
  float _iod_offset;
  LVecBase2f _keystone;

  LMatrix4f _film_mat, _film_mat_inv;
  LMatrix4f _lens_mat, _lens_mat_inv;
  LMatrix4f _projection_mat, _projection_mat_inv;

  enum UserFlags {
    // Parameters the user may have explicitly specified.
    UF_film_width          = 0x0001,
    UF_film_height         = 0x0002,
    UF_focal_length        = 0x0004,
    UF_hfov                = 0x0008,
    UF_vfov                = 0x0010,
    UF_aspect_ratio        = 0x0020,
    UF_view_hpr            = 0x0040,
    UF_view_vector         = 0x0080,
    UF_iod_offset          = 0x0100,
    UF_view_mat            = 0x0200,
    UF_keystone            = 0x0400,
  };

  enum CompFlags {
    // Values that may need to be recomputed.
    CF_film_mat            = 0x0001,
    CF_film_mat_inv        = 0x0002,
    CF_lens_mat            = 0x0004,
    CF_lens_mat_inv        = 0x0008,
    CF_projection_mat      = 0x0010,
    CF_projection_mat_inv  = 0x0020,
    CF_mat                 = 0x003f,  // all of the above.

    CF_focal_length        = 0x0040,
    CF_fov                 = 0x0080,
    CF_film_size           = 0x0100,
    CF_aspect_ratio        = 0x0200,
    CF_view_hpr            = 0x0400,
    CF_view_vector         = 0x0800,
    CF_iod_offset          = 0x1000,
  };
  short _user_flags;
  short _comp_flags;

  // The user may only specify two of these three parameters.
  // Specifying the third parameter wipes out the first one specified.
  // We therefore need to remember the order in which the user has
  // specified these three parameters.  A bit of a mess.
  char _focal_length_seq, _fov_seq, _film_size_seq;

  PTA_Vertexf _geom_coords;
  PT(qpGeomVertexData) _geom_data;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Lens",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

EXPCL_PANDA INLINE ostream &operator << (ostream &out, const Lens &lens);

#include "lens.I"

#endif

