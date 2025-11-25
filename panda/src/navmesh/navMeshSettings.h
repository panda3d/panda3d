/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshSettings.h
 * @author Ashwani / 
 * @date 2024
 */

#ifndef NAVMESHSETTINGS_H
#define NAVMESHSETTINGS_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

/**
 * This class contains all the settings required for generating a navigation mesh.
 * It encapsulates parameters for the Recast navigation mesh generation process.
 */
class EXPCL_PANDA_NAVMESH NavMeshSettings : public TypedReferenceCount {
PUBLISHED:
  NavMeshSettings();
  NavMeshSettings(const NavMeshSettings &copy);
  ~NavMeshSettings();

  // Cell size in world units
  INLINE void set_cell_size(PN_stdfloat cs);
  INLINE PN_stdfloat get_cell_size() const;

  // Cell height in world units
  INLINE void set_cell_height(PN_stdfloat ch);
  INLINE PN_stdfloat get_cell_height() const;

  // Agent height in world units
  INLINE void set_agent_height(PN_stdfloat height);
  INLINE PN_stdfloat get_agent_height() const;

  // Agent radius in world units
  INLINE void set_agent_radius(PN_stdfloat radius);
  INLINE PN_stdfloat get_agent_radius() const;

  // Agent max climb in world units
  INLINE void set_agent_max_climb(PN_stdfloat climb);
  INLINE PN_stdfloat get_agent_max_climb() const;

  // Agent max slope in degrees
  INLINE void set_agent_max_slope(PN_stdfloat slope);
  INLINE PN_stdfloat get_agent_max_slope() const;

  // Region minimum size (area)
  INLINE void set_region_min_size(PN_stdfloat size);
  INLINE PN_stdfloat get_region_min_size() const;

  // Region merge size (area)
  INLINE void set_region_merge_size(PN_stdfloat size);
  INLINE PN_stdfloat get_region_merge_size() const;

  // Edge max length
  INLINE void set_edge_max_len(PN_stdfloat len);
  INLINE PN_stdfloat get_edge_max_len() const;

  // Edge max error
  INLINE void set_edge_max_error(PN_stdfloat error);
  INLINE PN_stdfloat get_edge_max_error() const;

  // Vertices per polygon
  INLINE void set_verts_per_poly(int verts);
  INLINE int get_verts_per_poly() const;

  // Detail sample distance
  INLINE void set_detail_sample_dist(PN_stdfloat dist);
  INLINE PN_stdfloat get_detail_sample_dist() const;

  // Detail sample max error
  INLINE void set_detail_sample_max_error(PN_stdfloat error);
  INLINE PN_stdfloat get_detail_sample_max_error() const;

  void output(std::ostream &out) const;

private:
  PN_stdfloat _cell_size;
  PN_stdfloat _cell_height;
  PN_stdfloat _agent_height;
  PN_stdfloat _agent_radius;
  PN_stdfloat _agent_max_climb;
  PN_stdfloat _agent_max_slope;
  PN_stdfloat _region_min_size;
  PN_stdfloat _region_merge_size;
  PN_stdfloat _edge_max_len;
  PN_stdfloat _edge_max_error;
  int _verts_per_poly;
  PN_stdfloat _detail_sample_dist;
  PN_stdfloat _detail_sample_max_error;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "NavMeshSettings",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "navMeshSettings.I"

#endif // NAVMESHSETTINGS_H

