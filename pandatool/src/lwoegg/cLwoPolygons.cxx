// Filename: cLwoPolygons.cxx
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "cLwoPolygons.h"
#include "lwoToEggConverter.h"
#include "cLwoPoints.h"
#include "cLwoLayer.h"

#include <eggPolygon.h>

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
    nout << "Unknown geometry type " << _polygons->_polygon_type
	 << "; treating as FACE.\n";
    make_faces();
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
      _egg_group->add_child(egg_poly.p());
    }
  }
}

