// Filename: portalClipper.cxx
// Created by:  masad (4May04)
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

#include "portalClipper.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "transformState.h"
#include "renderState.h"
#include "fogAttrib.h"
#include "cullHandler.h"
#include "dcast.h"
#include "geomNode.h"
#include "config_pgraph.h"
#include "boundingSphere.h"
#include "geomSphere.h"
#include "colorAttrib.h"
#include "renderModeAttrib.h"
#include "cullFaceAttrib.h"
#include "depthOffsetAttrib.h"

TypeHandle PortalClipper::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PortalClipper::
PortalClipper(GeometricBoundingVolume *frustum, SceneSetup *scene_setup) {
  _previous = new GeomNode("my_frustum");
  _geom_line = new GeomLine;
  _geom_point = new GeomPoint;
  _geom_linestrip = new GeomLinestrip;

  _hex_frustum = DCAST(BoundingHexahedron, frustum);

  _scene_setup = scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PortalClipper::
~PortalClipper() {
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::move_to
//       Access: Public
//  Description: Moves the pen to the given point without drawing a
//               line.  When followed by draw_to(), this marks the
//               first point of a line segment; when followed by
//               move_to() or create(), this creates a single point.
////////////////////////////////////////////////////////////////////
void PortalClipper::
move_to(const LVecBase3f &v) {
  // We create a new SegmentList with the initial point in it.
  SegmentList segs;
  segs.push_back(Point(v, _color));

  // And add this list to the list of segments.
  _list.push_back(segs);
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::draw_to
//       Access: Public
//  Description: Draws a line segment from the pen's last position
//               (the last call to move_to or draw_to) to the
//               indicated point.  move_to() and draw_to() only update
//               tables; the actual drawing is performed when create()
//               is called.
////////////////////////////////////////////////////////////////////
void PortalClipper::
draw_to(const LVecBase3f &v) {
  if (_list.empty()) {
    // Let our first call to draw_to() be an implicit move_to().
    move_to(v);

  } else {
    // Get the current SegmentList, which was the last one we added to
    // the LineList.
    SegmentList &segs = _list.back();

    // Add the new point.
    segs.push_back(Point(v, _color));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::draw a portal frustum
//       Access: Public
//  Description: Given the BoundingHexahedron draw it using lines
//           
////////////////////////////////////////////////////////////////////
void PortalClipper::
draw_hexahedron(BoundingHexahedron *frustum) {
  /*
  pgraph_cat.debug() << "frustum points " << frustum->get_num_points() << endl;

  pgraph_cat.debug() << frustum->get_point(0) << endl;
  pgraph_cat.debug() << frustum->get_point(1) << endl;
  pgraph_cat.debug() << frustum->get_point(2) << endl;
  pgraph_cat.debug() << frustum->get_point(3) << endl;
  pgraph_cat.debug() << frustum->get_point(4) << endl;
  pgraph_cat.debug() << frustum->get_point(5) << endl;
  pgraph_cat.debug() << frustum->get_point(6) << endl;
  pgraph_cat.debug() << frustum->get_point(7) << endl;
  */

  // walk the view frustum as it should be drawn
  move_to(frustum->get_point(0));
  draw_to(frustum->get_point(1));
  draw_to(frustum->get_point(2));
  draw_to(frustum->get_point(3));

  move_to(frustum->get_point(4));
  draw_to(frustum->get_point(0));
  draw_to(frustum->get_point(3));
  draw_to(frustum->get_point(7));

  move_to(frustum->get_point(5));
  draw_to(frustum->get_point(4));
  draw_to(frustum->get_point(7));
  draw_to(frustum->get_point(6));

  move_to(frustum->get_point(1));
  draw_to(frustum->get_point(5));
  draw_to(frustum->get_point(6));
  draw_to(frustum->get_point(2));
}
////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::draw the lines
//       Access: Public
//  Description: Draw all the lines in the buffer
//           
////////////////////////////////////////////////////////////////////
void PortalClipper::
draw_lines()
{
  if (!_list.empty()) {
    _created_verts.clear();
    _created_colors.clear();
    
    // One array each for the indices into these arrays for points
    // and lines, and one for our line-segment lengths array.
    PTA_ushort point_index;
    PTA_ushort line_index;
    PTA_int lengths;
    
    // Now fill up the arrays.
    int v = 0;
    LineList::const_iterator ll;
    SegmentList::const_iterator sl;
    
    for (ll = _list.begin(); ll != _list.end(); ll++) {
      const SegmentList &segs = (*ll);
      
      if (segs.size() < 2) {
        point_index.push_back(v);
      } else {
        lengths.push_back(segs.size());
      }
      
      for (sl = segs.begin(); sl != segs.end(); sl++) {
        if (segs.size() >= 2) {
          line_index.push_back(v);
        }
        _created_verts.push_back((*sl)._point);
        _created_colors.push_back((*sl)._color);
        v++;
        //nassertr(v == (int)_created_verts.size(), previous);
      }
    }


    // Now create the lines.
    Geom *geom;
    if (line_index.size() > 0) {
      // Create a new Geom and add the line segments.
      if (line_index.size() <= 2) {
        // Here's a special case: just one line segment.
        _geom_line->set_num_prims(1);
        _geom_line->set_width(_thick);
        geom = _geom_line;

      } else {
        // The more normal case: multiple line segments, connected
        // end-to-end like a series of linestrips.
        _geom_linestrip->set_num_prims(lengths.size());
        _geom_linestrip->set_lengths(lengths);
        _geom_linestrip->set_width(_thick);
        geom = _geom_linestrip;
      }

      geom->set_colors(_created_colors, G_PER_VERTEX, line_index);
      geom->set_coords(_created_verts, line_index);

      //geom->write_verbose(cerr, 0);

      _previous->add_geom(geom);
      pgraph_cat.debug() << "added geometry" << endl;
    }
  }
}
////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::prepare the portal
//       Access: Public
//  Description: Given the portal draw the frustum with line segs
//               for now. More functionalities coming up
////////////////////////////////////////////////////////////////////
void PortalClipper::
prepare_portal(int idx)
{
  SegmentList segs;
  char portal_name[128];

  // print some messages to see if i am getting to this part
  pgraph_cat.debug() << "creating portal clipper " << idx << endl;

  // walk the portal
  _num_vert = 0;
  sprintf(portal_name, "**/portal%d", idx);
  NodePath portal_nodepath = _scene_setup->get_scene_root().find(portal_name);
  if (!portal_nodepath.is_empty()) {
    pgraph_cat.debug() << "portal nodepath " << portal_nodepath << endl;
    
    /*
    // Get the World transformation matrix
    CPT(TransformState) wtransform = portal_nodepath.get_transform(_scene_setup->get_scene_root());
    LMatrix4f wmat = wtransform->get_mat();
    pgraph_cat.debug() << wmat << endl;
    */
    
    // Get the camera transformation matrix
    CPT(TransformState) ctransform = portal_nodepath.get_transform(_scene_setup->get_cull_center());
    //CPT(TransformState) ctransform = portal_nodepath.get_transform(_scene_setup->get_camera_path());
    LMatrix4f cmat = ctransform->get_mat();
    pgraph_cat.debug() << cmat << endl;
    
    // Get the geometry from the portal    
    PandaNode *node = portal_nodepath.node();
    if (node->is_of_type(PortalNode::get_class_type())) {
      PortalNode *portal_node = DCAST(PortalNode, node);
      pgraph_cat.debug() << *portal_node << endl;
    
      _coords[0] = portal_node->get_vertex(0)*cmat;
      _coords[1] = portal_node->get_vertex(1)*cmat;
      _coords[2] = portal_node->get_vertex(2)*cmat;
      _coords[3] = portal_node->get_vertex(3)*cmat;
      
      pgraph_cat.debug() << "after transformation to camera space" << endl;
      pgraph_cat.debug() << _coords[0] << endl;
      pgraph_cat.debug() << _coords[1] << endl;
      pgraph_cat.debug() << _coords[2] << endl;
      pgraph_cat.debug() << _coords[3] << endl;
      
      // check if facing camera
      if (is_facing_camera()) {
        
        // ok, now lets add the near plane to this portal
        _color = Colorf(1,0,0,1);
        move_to(_coords[0]);
        draw_to(_coords[1]);
        draw_to(_coords[2]);
        draw_to(_coords[3]);
        draw_to(_coords[0]);
        
        pgraph_cat.debug() << "assembled portal" << idx << " frustum points" << endl;
        _num_vert = portal_node->get_num_vertices();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::clip the portal
//       Access: Public
//  Description: From the frustum clip the portal against the frustum
//               and form the new planes of the reduced view frustum
////////////////////////////////////////////////////////////////////
void PortalClipper::
clip_portal(int idx)
{
  int num_planes = _hex_frustum->get_num_planes();

  if (!_num_vert)
    return;

  /*
  pgraph_cat.debug() << "Number of planes " << num_planes << endl;

  // print out the planes. plane 0 should be far and plane 5 should be near
  // so we are only concerned with the 4 side planes.
  for (int i=0; i<num_planes; ++i) {
    Planef plane = _hex_frustum->get_plane(i);
    plane.output(pgraph_cat.debug());
    pgraph_cat.debug() << endl;
  }
  */

  for (int i=1; i<num_planes-1; ++i) {
    Planef plane = _hex_frustum->get_plane(i);
    for (int j=0; j<_num_vert; ++j) {
      float t;
      LPoint3f from_origin = _coords[j];
      LVector3f from_direction = _coords[(j+1)%_num_vert] - _coords[j];
      bool is_intersect = plane.intersects_line(t, from_origin, from_direction);
      if (is_intersect) {
        pgraph_cat.debug() << "plane " << i << " intersected segement " << j << "->" << (j+1)%_num_vert << " at t=" << t << endl;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::get_reduced_frustum
//       Access: Public
//  Description: After clipping the portal, form the new sides and 
//               fill in the new frustum. Return true if success
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) PortalClipper::
get_reduced_frustum(int idx)
{
  int num_planes = 6;
  LPoint3f intersect_points[4];
  
#if 0
  // calculate the new side planes
  for (int i=0; i<_num_vert; ++i) {
    // get the vectors, Vi+1 and Vi
    LVector3f front(_coords[(i+1)%_num_vert]);
    LVector3f back(_coords[i]);
    // get the cross product of these two vectors
    LVector3f normal = front.cross(back);
    normal.normalize();
    frustum_planes[i+1] = Planef(normal, LPoint3f(0,0,0));
    frustum_planes[i+1].output(pgraph_cat.debug());
    pgraph_cat.debug() << endl;
  }
#else
  // another approach to actually finding the points, so that 
  // I can reuse the current BoundingHexahedron object. Apparently,
  // it is better to construct this BH with bounding points, rather
  // than bounding planes (which I might have to implement soon)
  
  if (!_num_vert)
    return NULL;
  
  float t;
  bool visible = true;
  // find intersection of 7->0 with far
  LPoint3f from_origin = _hex_frustum->get_point(7);
  LVector3f from_direction = _coords[0] - from_origin;
  bool is_intersect = _hex_frustum->get_plane(0).intersects_line(t, from_origin, from_direction);
  if (is_intersect && t >= 0.0) { // has to be positive, else camera is not looking at the portal
    pgraph_cat.debug() << "far plane intersected 7->0 at t=" << t << endl;
    intersect_points[0] = from_origin + t*from_direction;
    pgraph_cat.debug() << intersect_points[0] << endl;
  }
  else
    visible = false;

  // find intersection of 4->1 with far
  from_origin = _hex_frustum->get_point(4);
  from_direction = _coords[1] - from_origin;
  is_intersect = _hex_frustum->get_plane(0).intersects_line(t, from_origin, from_direction);
  if (is_intersect && t >= 0.0) { // has to be positive, else camera is not looking at the portal
    pgraph_cat.debug() << "far plane intersected 4->1 at t=" << t << endl;
    intersect_points[1] = from_origin + t*from_direction;
    pgraph_cat.debug() << intersect_points[1] << endl;
  }
  else
    visible = false;

  // find intersection of 5->2 with far
  from_origin = _hex_frustum->get_point(5);
  from_direction = _coords[2] - from_origin;
  is_intersect = _hex_frustum->get_plane(0).intersects_line(t, from_origin, from_direction);
  if (is_intersect && t >= 0.0) { // has to be positive, else camera is not looking at the portal
    pgraph_cat.debug() << "far plane intersected 5->2 at t=" << t << endl;
    intersect_points[2] = from_origin + t*from_direction;
    pgraph_cat.debug() << intersect_points[2] << endl;
  }
  else
    visible = false;

  // find intersection of 6->3 with far
  from_origin = _hex_frustum->get_point(6);
  from_direction = _coords[3] - from_origin;
  is_intersect = _hex_frustum->get_plane(0).intersects_line(t, from_origin, from_direction);
  if (is_intersect && t >= 0.0) { // has to be positive, else camera is not looking at the portal
    pgraph_cat.debug() << "far plane intersected 6->3 at t=" << t << endl;
    intersect_points[3] = from_origin + t*from_direction;
    pgraph_cat.debug() << intersect_points[3] << endl;
  }
  else
    visible = false;

  if (!visible) {
    pgraph_cat.debug() << "portal" << idx << " is not visible from current camera look at" << endl;
    return NULL;
  }
  
  // With these intersect_points, construct the new reduced frustum
  PT(BoundingVolume) reduced_frustum = new
    BoundingHexahedron(intersect_points[1], intersect_points[2],
                       intersect_points[3], intersect_points[0],
                       _hex_frustum->get_point(4), _hex_frustum->get_point(5),
                       _hex_frustum->get_point(6), _hex_frustum->get_point(7));

  pgraph_cat.debug() << *reduced_frustum << endl;

  // draw this hexahedron
  _color = Colorf(0,0,1,1);
  draw_hexahedron(DCAST(BoundingHexahedron, reduced_frustum));

#endif
  return reduced_frustum;
}
