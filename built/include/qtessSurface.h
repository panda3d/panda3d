/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qtessSurface.h
 * @author drose
 * @date 2003-10-13
 */

#ifndef QTESSSURFACE_H
#define QTESSSURFACE_H

#include "pandatoolbase.h"
#include "isoPlacer.h"
#include "eggNurbsSurface.h"
#include "eggGroup.h"
#include "eggVertex.h"
#include "nurbsSurfaceEvaluator.h"
#include "nurbsSurfaceResult.h"
#include "referenceCount.h"
#include "pointerTo.h"

/**
 * A reference to an EggNurbsSurface in the egg file, and its parameters as
 * set by the user input file and as computed in relation to the other
 * surfaces.
 */
class QtessSurface : public ReferenceCount {
public:
  QtessSurface(EggNurbsSurface *egg_surface);

  INLINE const std::string &get_name() const;
  INLINE bool is_valid() const;

  INLINE void set_importance(double importance2);
  INLINE void set_match_u(QtessSurface **match_u, bool match_u_to_u);
  INLINE void set_match_v(QtessSurface **match_v, bool match_v_to_v);
  INLINE void set_min_u(int min_u);
  INLINE void set_min_v(int min_v);

  INLINE double count_patches() const;
  INLINE int count_tris() const;

  double get_score(double ratio);

  int tesselate();
  int write_qtess_parameter(std::ostream &out);
  void omit();
  void tesselate_uv(int u, int v, bool autoplace, double ratio);
  void tesselate_specific(const pvector<double> &u_list,
                          const pvector<double> &v_list);
  void tesselate_per_isoparam(double pi, bool autoplace, double ratio);
  void tesselate_per_score(double pi, bool autoplace, double ratio);
  void tesselate_auto(int u, int v, double ratio);

private:
  void record_vertex_extras();
  INLINE int get_joint_membership_index(EggGroup *joint);
  INLINE int get_dxyz_index(const std::string &morph_name);
  INLINE int get_drgba_index(const std::string &morph_name);

  void apply_match();
  PT(EggGroup) do_uniform_tesselate(int &tris) const;
  PT(EggVertex) evaluate_vertex(double u, double v) const;

  PT(EggNurbsSurface) _egg_surface;
  PT(NurbsSurfaceEvaluator) _nurbs;
  PT(NurbsSurfaceResult) _nurbs_result;
  bool _has_vertex_color;

  // Mapping arbitrary attributes to integer extended dimension values, so we
  // can hang arbitrary data in the extra dimensional space of the surface.
  int _next_d;
  typedef std::map<EggGroup *, int> JointTable;
  JointTable _joint_table;
  typedef std::map<std::string, int> MorphTable;
  MorphTable _dxyz_table;
  MorphTable _drgba_table;

  int _num_u, _num_v;
  int _tess_u, _tess_v;
  pvector<double> _iso_u, _iso_v;  // If nonempty, isoparams at which to tess.

  // _importance is the relative importance of the surface along either axis;
  // _importance2 is this number squared, which is the value set by
  // set_importance().
  double _importance;
  double _importance2;

  // _match_u and _match_v indicate which surface we must match exactly for
  // tesselation in U or V.  This helps get edges to line up properly.  They
  // are indirect pointers because we go through the surfaces in one pass, and
  // might need to fill in the correct value later.
  QtessSurface **_match_u, **_match_v;
  bool _match_u_to_u, _match_v_to_v;

  // _min_u and _min_v specify a mininum number of quads below which we should
  // not attempt to subdivide the surface in either dimension.  This is
  // intended to prevent degenerate cases like knife-fingers.
  int _min_u, _min_v;

  IsoPlacer _u_placer, _v_placer;
  bool _got_scores;
};

#include "qtessSurface.I"

#endif
