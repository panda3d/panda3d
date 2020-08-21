/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qtessInputEntry.I
 * @author drose
 * @date 2003-10-13
 */

/**
 *
 */
INLINE QtessInputEntry::
QtessInputEntry(const QtessInputEntry &copy) {
  (*this) = copy;
}

/**
 *
 */
INLINE void QtessInputEntry::
add_node_name(const std::string &name) {
  _node_names.push_back(GlobPattern(name));
}

/**
 *
 */
INLINE void QtessInputEntry::
set_importance(double i) {
  _importance = i;
  _type = T_importance;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_match_uu() {
  _type = T_match_uu;
  _constrain_u = nullptr;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_match_vv() {
  _type = T_match_vv;
  _constrain_v = nullptr;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_match_uv() {
  _type = T_match_uv;
  _constrain_u = nullptr;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_match_vu() {
  _type = T_match_vu;
  _constrain_v = nullptr;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_min_u(int min_u) {
  _type = T_min_u;
  _num_u = min_u;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_min_v(int min_v) {
  _type = T_min_v;
  _num_v = min_v;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_undefined() {
  _type = T_undefined;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_omit() {
  _type = T_omit;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_num_tris(int nt) {
  _num_tris = nt;
  _type = T_num_tris;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_uv(int u, int v) {
  set_uv(u, v, nullptr, 0);
}

/**
 *
 */
INLINE void QtessInputEntry::
set_per_isoparam(double pi) {
  _per_isoparam = pi;
  _type = T_per_isoparam;
}

/**
 *
 */
INLINE void QtessInputEntry::
set_per_score(double pi) {
  _per_isoparam = pi;
  _type = T_per_score;
}

/**
 *
 */
INLINE int QtessInputEntry::
get_num_surfaces() const {
  return _surfaces.size();
}


INLINE std::ostream &operator << (std::ostream &out, const QtessInputEntry &entry) {
  entry.output(out);
  return out;
}
