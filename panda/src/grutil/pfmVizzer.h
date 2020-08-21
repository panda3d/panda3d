/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfmVizzer.h
 * @author drose
 * @date 2012-09-30
 */

#ifndef PFMVIZZER_H
#define PFMVIZZER_H

#include "pandabase.h"
#include "nodePath.h"
#include "internalName.h"
#include "lens.h"
#include "pfmFile.h"

class GeomNode;
class Lens;
class GeomVertexWriter;

/**
 * This class aids in the visualization and manipulation of PfmFile objects.
 */
class EXPCL_PANDA_GRUTIL PfmVizzer {
PUBLISHED:
  explicit PfmVizzer(PfmFile &pfm);
  INLINE ~PfmVizzer();
  INLINE PfmFile &get_pfm();
  INLINE const PfmFile &get_pfm() const;

  BLOCKING void project(const Lens *lens, const PfmFile *undist_lut = nullptr);
  BLOCKING void extrude(const Lens *lens);

  INLINE void set_vis_inverse(bool vis_inverse);
  INLINE bool get_vis_inverse() const;
  INLINE void set_flat_texcoord_name(InternalName *flat_texcoord_name);
  INLINE void clear_flat_texcoord_name();
  INLINE InternalName *get_flat_texcoord_name() const;
  INLINE void set_vis_2d(bool vis_2d);
  INLINE bool get_vis_2d() const;
  INLINE void set_keep_beyond_lens(bool keep_beyond_lens);
  INLINE bool get_keep_beyond_lens() const;

  INLINE void set_vis_blend(const PNMImage *vis_blend);
  INLINE void clear_vis_blend();
  INLINE const PNMImage *get_vis_blend() const;

  INLINE void set_aux_pfm(const PfmFile *pfm);
  INLINE void clear_aux_pfm();
  INLINE const PfmFile *get_aux_pfm() const;

  enum ColumnType {
    CT_texcoord2,
    CT_texcoord3,
    CT_vertex1,
    CT_vertex2,
    CT_vertex3,
    CT_normal3,
    CT_blend1,
    CT_aux_vertex1,
    CT_aux_vertex2,
    CT_aux_vertex3,
  };
  void clear_vis_columns();
  void add_vis_column(ColumnType source, ColumnType target,
                      InternalName *name,
                      const TransformState *transform = nullptr, const Lens *lens = nullptr,
                      const PfmFile *undist_lut = nullptr);

  BLOCKING NodePath generate_vis_points() const;

  enum MeshFace {
    MF_front = 0x01,
    MF_back  = 0x02,
    MF_both  = 0x03,
  };
  BLOCKING NodePath generate_vis_mesh(MeshFace face = MF_front) const;

  BLOCKING double calc_max_u_displacement() const;
  BLOCKING double calc_max_v_displacement() const;
  BLOCKING void make_displacement(PNMImage &result, double max_u, double max_v, bool for_32bit) const;
  BLOCKING void make_displacement(PfmFile &result, double max_u, double max_v, bool for_32bit) const;

private:
  bool uses_aux_pfm() const;
  void r_fill_displacement(PNMImage &result, int xi, int yi,
                           double nxi, double nyi, double u_scale, double v_scale,
                           int distance) const;
  void r_fill_displacement(PfmFile &result, int xi, int yi,
                           double nxi, double nyi, double u_scale, double v_scale,
                           int distance) const;

  void make_vis_mesh_geom(GeomNode *gnode, bool inverted) const;


  class VisColumn {
  public:
    INLINE VisColumn();
    bool add_data(const PfmVizzer &vizzer, GeomVertexWriter &vwriter, int xi, int yi, bool reverse_normals) const;
    bool transform_point(LPoint2f &point) const;
    bool transform_point(LPoint3f &point) const;
    bool transform_vector(LVector3f &vec) const;

  public:
    ColumnType _source;
    ColumnType _target;
    PT(InternalName) _name;
    CPT(TransformState) _transform;
    CPT(Lens) _lens;
    const PfmFile *_undist_lut;
  };
  typedef pvector<VisColumn> VisColumns;

  static void add_vis_column(VisColumns &vis_columns,
                             ColumnType source, ColumnType target,
                             InternalName *name,
                             const TransformState *transform = nullptr,
                             const Lens *lens = nullptr, const PfmFile *undist_lut = nullptr);
  void build_auto_vis_columns(VisColumns &vis_columns, bool for_points) const;
  CPT(GeomVertexFormat) make_array_format(const VisColumns &vis_columns) const;

private:
  PfmFile &_pfm;
  const PfmFile *_aux_pfm;

  bool _vis_inverse;
  PT(InternalName) _flat_texcoord_name;
  bool _vis_2d;
  bool _keep_beyond_lens;
  const PNMImage *_vis_blend;

  VisColumns _vis_columns;

  friend class VisColumn;
};

#include "pfmVizzer.I"

#endif
