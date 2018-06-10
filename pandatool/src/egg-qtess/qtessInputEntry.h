/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qtessInputEntry.h
 * @author drose
 * @date 2003-10-13
 */

#ifndef QTESSINPUTENTRY_H
#define QTESSINPUTENTRY_H

#include "pandatoolbase.h"
#include "globPattern.h"
#include "pvector.h"

class QtessSurface;

/**
 * Stores one entry in the qtess input file.  This consists of a list of name
 * patterns and a set of tesselation parameters.
 */
class QtessInputEntry {
public:
  enum Type {
    T_undefined, T_omit, T_num_tris, T_uv, T_per_isoparam, T_per_score,
    T_importance, T_match_uu, T_match_vv, T_match_uv, T_match_vu,
    T_min_u, T_min_v
  };

  QtessInputEntry(const std::string &name = std::string());
  INLINE QtessInputEntry(const QtessInputEntry &copy);
  void operator = (const QtessInputEntry &copy);

  INLINE void add_node_name(const std::string &name);
  INLINE void set_importance(double i);
  INLINE void set_match_uu();
  INLINE void set_match_vv();
  INLINE void set_match_uv();
  INLINE void set_match_vu();
  INLINE void set_min_u(int min_u);
  INLINE void set_min_v(int min_v);
  INLINE void set_undefined();
  INLINE void set_omit();
  INLINE void set_num_tris(int nt);
  INLINE void set_uv(int u, int v);
  void set_uv(int u, int v, const std::string params[], int num_params);
  INLINE void set_per_isoparam(double pi);
  INLINE void set_per_score(double pi);
  void add_extra_u_isoparam(double u);
  void add_extra_v_isoparam(double u);

  Type match(QtessSurface *surface);
  INLINE int get_num_surfaces() const;
  int count_tris(double tri_factor = 1.0, int attempts = 0);

  static void output_extra(std::ostream &out, const pvector<double> &iso, char axis);
  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level) const;

  bool _auto_place, _auto_distribute;
  double _curvature_ratio;
  double _importance;
  QtessSurface *_constrain_u, *_constrain_v;

private:
  typedef pvector<GlobPattern> NodeNames;
  NodeNames _node_names;

  int _num_tris;
  int _num_u, _num_v;
  double _per_isoparam;
  pvector<double> _iso_u, _iso_v;
  Type _type;

  typedef pvector<QtessSurface *> Surfaces;
  Surfaces _surfaces;

  double _num_patches;
};

INLINE std::ostream &operator << (std::ostream &out, const QtessInputEntry &entry);

#include "qtessInputEntry.I"

#endif
