// Filename: cLwoPolygons.cxx
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "cLwoPolygons.h"
#include "lwoToEggConverter.h"
#include "cLwoPoints.h"
#include "cLwoLayer.h"
#include "cLwoSurface.h"

#include <lwoPolygonTags.h>
#include <lwoTags.h>
#include <eggPolygon.h>
#include <deg_2_rad.h>

////////////////////////////////////////////////////////////////////
//     Function: CLwoPolygons::add_ptags
//       Access: Public
//  Description: Associates the indicated PolygonTags and Tags with
//               the polygons in this chunk.  This may define features
//               such as per-polygon surfaces, parts, and smoothing
//               groups.
////////////////////////////////////////////////////////////////////
void CLwoPolygons::
add_ptags(const LwoPolygonTags *lwo_ptags, const LwoTags *tags) {
  if (_tags != (LwoTags *)NULL && _tags != tags) {
    nout << "Multiple Tags fields in effect on the same polygons.\n";
  }
  _tags = tags;

  IffId type = lwo_ptags->_tag_type;

  bool inserted = _ptags.insert(PTags::value_type(type, lwo_ptags)).second;
  if (!inserted) {
    nout << "Multiple polygon tags on the same polygons of type " 
	 << type << "\n";

  } else {
    if (type == IffId("SURF")) {
      _surf_ptags = lwo_ptags;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoPolygons::get_surface
//       Access: Public
//  Description: Returns the surface associated with the given
//               polygon, or NULL if no surface is associated.
////////////////////////////////////////////////////////////////////
CLwoSurface *CLwoPolygons::
get_surface(int polygon_index) const {
  if (_surf_ptags == (LwoPolygonTags *)NULL) {
    // No surface definitions.
    return (CLwoSurface *)NULL;
  }

  if (!_surf_ptags->has_tag(polygon_index)) {
    // The polygon isn't tagged.
    return (CLwoSurface *)NULL;
  }

  int tag_index = _surf_ptags->get_tag(polygon_index);
  if (_tags == (LwoTags *)NULL || tag_index < 0 || 
      tag_index >= _tags->get_num_tags()) {
    // The tag index is out-of-bounds.
    nout << "Invalid polygon tag index " << tag_index << "\n";
    return (CLwoSurface *)NULL;
  }

  string tag = _tags->get_tag(tag_index);

  // Now look up the surface name in the header.
  CLwoSurface *surface = _converter->get_surface(tag);
  if (surface == (CLwoSurface *)NULL) {
    nout << "Unknown surface " << tag << "\n";
    return (CLwoSurface *)NULL;
  }

  return surface;
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoPolygons::make_egg
//       Access: Public
//  Description: Creates the egg structures associated with this
//               Lightwave object.
////////////////////////////////////////////////////////////////////
void CLwoPolygons::
make_egg() {
  // First, we need a temporary group to hold all of the polygons
  // we'll create.
  _egg_group = new EggGroup;

  if (_polygons->_polygon_type == IffId("CURV")) {
    nout << "Ignoring Catmull-Rom splines.\n";

  } else if (_polygons->_polygon_type == IffId("PTCH")) {
    nout << "Ignoring subdivision patches.\n";

  } else if (_polygons->_polygon_type == IffId("MBAL")) {
    nout << "Ignoring metaballs.\n";

  } else if (_polygons->_polygon_type == IffId("BONE")) {
    nout << "Ignoring bones.\n";

  } else if (_polygons->_polygon_type == IffId("FACE")) {
    make_faces();

  } else {
    nout << "Ignoring unknown geometry type " << _polygons->_polygon_type
	 << ".\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoPolygons::connect_egg
//       Access: Public
//  Description: Connects all the egg structures together.
////////////////////////////////////////////////////////////////////
void CLwoPolygons::
connect_egg() {
  nassertv(_points->_layer->_egg_group != (EggGroup *)NULL);
  nassertv(_egg_group != (EggGroup *)NULL);
  _points->_layer->_egg_group->steal_children(*_egg_group);
}


////////////////////////////////////////////////////////////////////
//     Function: CLwoPolygons::make_faces
//       Access: Public
//  Description: Generates "face" polygons, i.e. actual polygons.
////////////////////////////////////////////////////////////////////
void CLwoPolygons::
make_faces() {
  float smooth_angle = -1.0;

  int num_polygons = _polygons->get_num_polygons();
  for (int i = 0; i < num_polygons; i++) {
    LwoPolygons::Polygon *poly = _polygons->get_polygon(i);
    
    PT(EggPolygon) egg_poly = new EggPolygon;

    bool is_valid = true;

    // Set up the vertices.
    const LwoPoints *points = _points->_points;
    int num_points = points->get_num_points();
    EggVertexPool *egg_vpool = _points->_egg_vpool;

    // We reverse the vertex ordering to compensate for Lightwave's
    // clockwise ordering convention.
    vector_int::reverse_iterator vi;
    for (vi = poly->_vertices.rbegin(); 
	 vi != poly->_vertices.rend() && is_valid; 
	 ++vi) {
      int vindex = (*vi);
      if (vindex < 0 || vindex >= num_points) {
	nout << "Invalid vertex index " << vindex << " in polygon.\n";
	is_valid = false;

      } else {
	EggVertex egg_vert;
	egg_vert.set_pos(LCAST(double, points->get_point(vindex)));
	EggVertex *new_vert = egg_vpool->create_unique_vertex(egg_vert);
	egg_poly->add_vertex(new_vert);
      }
    }

    if (is_valid) {
      CLwoSurface *surface = get_surface(i);
      if (surface != (CLwoSurface *)NULL) {
	surface->apply_properties(egg_poly, smooth_angle);
      }

      _egg_group->add_child(egg_poly.p());
    }
  }

  CoordinateSystem cs = _converter->get_egg_data().get_coordinate_system();
  if (smooth_angle > 0.0) {
    _egg_group->recompute_vertex_normals(rad_2_deg(smooth_angle), cs);
  } else {
    _egg_group->recompute_polygon_normals(cs);
  }
}

