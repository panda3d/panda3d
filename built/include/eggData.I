/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggData.I
 * @author drose
 * @date 1999-02-11
 */

/**
 *
 */
INLINE EggData::
EggData() {
  _auto_resolve_externals = false;
  _had_absolute_pathnames = false;
  _coordsys = CS_default;
  _egg_timestamp = 0;
}


/**
 *
 */
INLINE EggData::
EggData(const EggData &copy) :
  EggGroupNode(copy),
  _auto_resolve_externals(copy._auto_resolve_externals),
  _had_absolute_pathnames(copy._had_absolute_pathnames),
  _coordsys(copy._coordsys),
  _egg_filename(copy._egg_filename),
  _egg_timestamp(copy._egg_timestamp)
{
}

/**
 *
 */
INLINE EggData &EggData::
operator = (const EggData &copy) {
  EggGroupNode::operator = (copy);
  _auto_resolve_externals = copy._auto_resolve_externals;
  _had_absolute_pathnames = copy._had_absolute_pathnames;
  _coordsys = copy._coordsys;
  _egg_filename = copy._egg_filename;
  _egg_timestamp = copy._egg_timestamp;
  return *this;
}

/**
 * Indicates whether the EggData object will automatically resolve any
 * external references when read() is called.  The default is false.
 */
INLINE void EggData::
set_auto_resolve_externals(bool resolve) {
  _auto_resolve_externals = resolve;
}

/**
 * Indicates whether the EggData object will automatically resolve any
 * external references when read() is called.  The default is false.
 */
INLINE bool EggData::
get_auto_resolve_externals() const {
  return _auto_resolve_externals;
}

/**
 * Returns true if the data processed in the last call to read() contained
 * absolute pathnames, or false if those pathnames were all relative.
 *
 * This method is necessary because if auto_resolve_externals() is in effect,
 * it may modify the pathnames to be absolute whether or not they were as
 * loaded from disk.  This method can be used to query the state of the
 * original egg file from disk.
 */
INLINE bool EggData::
original_had_absolute_pathnames() const {
  return _had_absolute_pathnames;
}

/**
 * Returns the coordinate system in which the egg file is defined.
 */
INLINE CoordinateSystem EggData::
get_coordinate_system() const {
  return _coordsys;
}


/**
 * Sets the filename--especially the directory part--in which the egg file is
 * considered to reside.  This is also implicitly set by read().
 */
INLINE void EggData::
set_egg_filename(const Filename &egg_filename) {
  _egg_filename = egg_filename;
}

/**
 * Returns the directory in which the egg file is considered to reside.
 */
INLINE const Filename &EggData::
get_egg_filename() const {
  return _egg_filename;
}

/**
 * Sets the timestamp of the egg file on disk, at the time it was opened for
 * reading.  This is also implicitly set by read().
 */
INLINE void EggData::
set_egg_timestamp(time_t egg_timestamp) {
  _egg_timestamp = egg_timestamp;
}

/**
 * Returns the timestamp of the egg file on disk, at the time it was opened
 * for reading, or 0 if this information is not available.
 */
INLINE time_t EggData::
get_egg_timestamp() const {
  return _egg_timestamp;
}

/**
 * Recomputes all the vertex normals for polygon geometry at this group node
 * and below so that they accurately reflect the vertex positions.  A shared
 * edge between two polygons (even in different groups) is considered smooth
 * if the angle between the two edges is less than threshold degrees.
 *
 * This function also removes degenerate polygons that do not have enough
 * vertices to define a normal.  It does not affect normals for other kinds of
 * primitives like Nurbs or Points.
 *
 * This function does not remove or adjust vertices in the vertex pool; it
 * only adds new vertices with the correct normals.  Thus, it is a good idea
 * to call remove_unused_vertices() after calling this.
 */
INLINE void EggData::
recompute_vertex_normals(double threshold) {
  EggGroupNode::recompute_vertex_normals(threshold, _coordsys);
}

/**
 * Recomputes all the polygon normals for polygon geometry at this group node
 * and below so that they accurately reflect the vertex positions.  Normals
 * are removed from the vertices and defined only on polygons, giving the
 * geometry a faceted appearance.
 *
 * This function also removes degenerate polygons that do not have enough
 * vertices to define a normal.  It does not affect normals for other kinds of
 * primitives like Nurbs or Points.
 *
 * This function does not remove or adjust vertices in the vertex pool; it
 * only adds new vertices with the normals removed.  Thus, it is a good idea
 * to call remove_unused_vertices() after calling this.
 */
INLINE void EggData::
recompute_polygon_normals() {
  EggGroupNode::recompute_polygon_normals(_coordsys);
}

/**
 * Removes all normals from primitives, and the vertices they reference, at
 * this node and below.
 *
 * This function does not remove or adjust vertices in the vertex pool; it
 * only adds new vertices with the normal removed.  Thus, it is a good idea to
 * call remove_unused_vertices() after calling this.
 */
INLINE void EggData::
strip_normals() {
  EggGroupNode::strip_normals();
}
