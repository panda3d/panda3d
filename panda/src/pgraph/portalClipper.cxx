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

  _view_frustum = _reduced_frustum = DCAST(BoundingHexahedron, frustum);

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
//     Function: PortalClipper::draw the current visible portal
//       Access: Public
//  Description: _portal_node is the current portal, draw it.
//           
////////////////////////////////////////////////////////////////////
void PortalClipper::
draw_current_portal()
{
  move_to(_portal_node->get_vertex(0));
  draw_to(_portal_node->get_vertex(1));
  draw_to(_portal_node->get_vertex(2));
  draw_to(_portal_node->get_vertex(3));
}
////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::draw the lines
//       Access: Public
//  Description: Draw all the lines in the buffer
//               Yellow portal is the original geometry of the portal
//               Cyan portal is the minmax adjusted portal
//               Red portal is the clipped against frustum portal
//               Blue frustum is the frustum through portal
//               White frustum is the camera frustum
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
        //_geom_line->set_width(_thick);
        geom = _geom_line;

      } else {
        // The more normal case: multiple line segments, connected
        // end-to-end like a series of linestrips.
        _geom_linestrip->set_num_prims(lengths.size());
        _geom_linestrip->set_lengths(lengths);
        //_geom_linestrip->set_width(_thick);
        geom = _geom_linestrip;
      }

      geom->set_colors(_created_colors, G_PER_VERTEX, line_index);
      geom->set_coords(_created_verts, line_index);

      //geom->write_verbose(cerr, 0);

      _previous->add_geom(geom);
      pgraph_cat.spam() << "added geometry" << endl;
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
prepare_portal(const NodePath &node_path)
{
  SegmentList segs;

  // Get the Portal Node from this node_path
  PandaNode *node = node_path.node();
  _portal_node = NULL;
  if (node->is_of_type(PortalNode::get_class_type())) {  
    _portal_node = DCAST(PortalNode, node);
    // lets draw the portal anyway
    //draw_current_portal();
  }

  // walk the portal
  _num_vert = 0;

  // Get the geometry from the portal    
  pgraph_cat.spam() << *_portal_node << endl;

  /*
  // Get the World transformation matrix
  CPT(TransformState) wtransform = portal_nodepath.get_transform(_scene_setup->get_scene_root());
  LMatrix4f wmat = wtransform->get_mat();
  pgraph_cat.spam() << wmat << endl;
  */
  
  // Get the camera transformation matrix
  CPT(TransformState) ctransform = node_path.get_transform(_scene_setup->get_cull_center());
  //CPT(TransformState) ctransform = node_path.get_transform(_scene_setup->get_camera_path());
  LMatrix4f cmat = ctransform->get_mat();
  pgraph_cat.spam() << cmat << endl;
 
  Vertexf temp[4];
  temp[0] = _portal_node->get_vertex(0);
  temp[1] = _portal_node->get_vertex(1);
  temp[2] = _portal_node->get_vertex(2);
  temp[3] = _portal_node->get_vertex(3);

  pgraph_cat.spam() << "before transformation to camera space" << endl;
  pgraph_cat.spam() << temp[0] << endl;
  pgraph_cat.spam() << temp[1] << endl;
  pgraph_cat.spam() << temp[2] << endl;
  pgraph_cat.spam() << temp[3] << endl;
  
  temp[0] = temp[0]*cmat;
  temp[1] = temp[1]*cmat;
  temp[2] = temp[2]*cmat;
  temp[3] = temp[3]*cmat;

  Planef portal_plane(temp[0], temp[1], temp[2]);
  if (!is_facing_view(portal_plane)) {
    pgraph_cat.debug() << "portal failed 1st level test \n";
    return;
  }

  pgraph_cat.spam() << "after transformation to camera space" << endl;
  pgraph_cat.spam() << temp[0] << endl;
  pgraph_cat.spam() << temp[1] << endl;
  pgraph_cat.spam() << temp[2] << endl;
  pgraph_cat.spam() << temp[3] << endl;
  
  float min_x, max_x, min_z, max_z;

  min_x = min(min(min(temp[0][0], temp[1][0]), temp[2][0]), temp[3][0]);
  max_x = max(max(max(temp[0][0], temp[1][0]), temp[2][0]), temp[3][0]);
  min_z = min(min(min(temp[0][2], temp[1][2]), temp[2][2]), temp[3][2]);
  max_z = max(max(max(temp[0][2], temp[1][2]), temp[2][2]), temp[3][2]);

  pgraph_cat.spam() << "min_x " << min_x << ";max_x " << max_x << ";min_z " << min_z << ";max_z " << max_z << endl;

  float y;

  y = get_plane_depth(min_x, min_z, &portal_plane);
  pgraph_cat.spam() << "plane's depth is " << y << endl;
  _coords[0].set(min_x, y, min_z);

  y = get_plane_depth(max_x, min_z, &portal_plane);
  pgraph_cat.spam() << "plane's depth is " << y << endl;
  _coords[1].set(max_x, y, min_z);

  y = get_plane_depth(max_x, max_z, &portal_plane);
  pgraph_cat.spam() << "plane's depth is " << y << endl;
  _coords[2].set(max_x, y, max_z);

  y = get_plane_depth(min_x, max_z, &portal_plane);
  pgraph_cat.spam() << "plane's depth is " << y << endl;
  _coords[3].set(min_x, y, max_z);
    
  pgraph_cat.spam() << "after min max calculation" << endl;
  pgraph_cat.spam() << _coords[0] << endl;
  pgraph_cat.spam() << _coords[1] << endl;
  pgraph_cat.spam() << _coords[2] << endl;
  pgraph_cat.spam() << _coords[3] << endl;

  // check if portal is in view
  if (is_whole_portal_in_view(cmat)) {
    // ok, now lets add the original portal
    _color = Colorf(0,1,1,1);
    move_to(temp[0]);
    draw_to(temp[1]);
    draw_to(temp[2]);
    draw_to(temp[3]);
    draw_to(temp[0]);

    // ok, now lets add the min_max portal
    _color = Colorf(1,1,0,1);
    move_to(_coords[0]);
    draw_to(_coords[1]);
    draw_to(_coords[2]);
    draw_to(_coords[3]);
    draw_to(_coords[0]);
    
    pgraph_cat.spam() << "assembled " << _portal_node->get_name() << ": frustum points" << endl;
    _num_vert = _portal_node->get_num_vertices();
  }
  else
    pgraph_cat.debug() << "portal failed 2nd level test \n";
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::clip the portal
//       Access: Public
//  Description: From the frustum clip the portal against the frustum
//               and form the new planes of the reduced view frustum
////////////////////////////////////////////////////////////////////
void PortalClipper::
clip_portal(const NodePath &node_path)
{
  if (!_num_vert)
    return;

  // ViewFrustum -> Logical Planes -> Portal Edge
  // Plane0      -> far plane      -> None
  // plane5      -> near plane     -> None
  // Plane1      -> bottom plane   -> 0-1
  // Plane3      -> top plane      -> 1-2
  // Plane2      -> right plane    -> 2-3
  // Plane4      -> left plane     -> 3-0

  int j;
  float t;
  Planef plane;
  bool is_intersect;
  unsigned int xect=0;
  LPoint3f from_origin;
  LPoint3f cut_point;
  LVector3f from_direction;

  // Look for intersection with the view frustum's bottom_plane and portal edges
  plane = _reduced_frustum->get_plane(1);
  for (j=0; j<_num_vert; ++j) {
    from_origin = _coords[j];
    from_direction = _coords[(j+1)%_num_vert] - _coords[j];
    is_intersect = plane.intersects_line(t, from_origin, from_direction);
    if (is_intersect && (t >= 0.0 && t <= 1.0)) {
      xect |= 1 << 0;
      pgraph_cat.debug() << "bottom plane intersected segment " << j << "->" 
                         << (j+1)%_num_vert << " at t=" << t << endl;
      cut_point = from_origin + t*from_direction;
      pgraph_cat.spam() << "cut_point: " << cut_point << endl;
      if (j == 1) {
        // means bottom should cut 1->2 by moving 1 to the intersection point
        _coords[1] = cut_point;
      }
      else if (j == 3) {
        // means bottom should cut 3->0 by moving 0 to the intersection point
        _coords[0] = cut_point;
      }
      else
        pgraph_cat.debug() << "ignored for now for simplicity \n";
    }
    else
      pgraph_cat.spam() << "is_intersect: " << is_intersect << " at t = " << t << endl;
  }

  // Look for intersection with the view frustum's top_plane and portal edges
  plane = _reduced_frustum->get_plane(3);
  for (j=0; j<_num_vert; ++j) {
    from_origin = _coords[j];
    from_direction = _coords[(j+1)%_num_vert] - _coords[j];
    is_intersect = plane.intersects_line(t, from_origin, from_direction);
    if (is_intersect && (t >= 0.0 && t <= 1.0)) {
      xect |= 1 << 1;
      pgraph_cat.debug() << "top plane intersected segment " << j << "->" 
                         << (j+1)%_num_vert << " at t=" << t << endl;
      cut_point = from_origin + t*from_direction;
      pgraph_cat.spam() << "cut_point: " << cut_point << endl;
      if (j == 1) {
        // means top should cut 1->2 by moving 2 to the intersection point
        _coords[2] = cut_point;
      }
      else if (j == 3) {
        // means top should cut 3->0 by moving 3 to the intersection point
        _coords[3] = cut_point;
      }
      else
        pgraph_cat.debug() << "ignored for now for simplicity \n";
    }
    else
      pgraph_cat.spam() << "is_intersect: " << is_intersect << " at t = " << t << endl;
  }

  // Look for intersection with the view frustum's right_plane and portal edges
  plane = _reduced_frustum->get_plane(2);
  for (j=0; j<_num_vert; ++j) {
    from_origin = _coords[j];
    from_direction = _coords[(j+1)%_num_vert] - _coords[j];
    is_intersect = plane.intersects_line(t, from_origin, from_direction);
    if (is_intersect && (t >= 0.0 && t <= 1.0)) {
      xect |= 1 << 2;
      pgraph_cat.debug() << "right plane intersected segment " << j << "->" 
                         << (j+1)%_num_vert << " at t=" << t << endl;
      cut_point = from_origin + t*from_direction;
      pgraph_cat.spam() << "cut_point: " << cut_point << endl;
      if (j == 0) {
        // means right should cut 0->1 by moving 1 to the intersection point
        _coords[1] = cut_point;
      }
      else if (j == 2) {
        // means right should cut 2->3 by moving 2 to the intersection point
        _coords[2] = cut_point;
      }
      else
        pgraph_cat.debug() << "ignored for now for simplicity \n";
    }
    else
      pgraph_cat.spam() << "is_intersect: " << is_intersect << " at t = " << t << endl;
  }

  // Look for intersection with the view frustum's left_plane and portal edges
  plane = _reduced_frustum->get_plane(4);
  for (j=0; j<_num_vert; ++j) {
    from_origin = _coords[j];
    from_direction = _coords[(j+1)%_num_vert] - _coords[j];
    is_intersect = plane.intersects_line(t, from_origin, from_direction);
    if (is_intersect && (t >= 0.0 && t <= 1.0)) {
      xect |= 1 << 3;
      pgraph_cat.debug() << "left plane intersected segment " << j << "->" 
                         << (j+1)%_num_vert << " at t=" << t << endl;
      cut_point = from_origin + t*from_direction;
      pgraph_cat.spam() << "cut_point: " << cut_point << endl;
      if (j == 0) {
        // means left should cut 0->1 by moving 0 to the intersection point
        _coords[0] = cut_point;
      }
      else if (j == 2) {
        // means left should cut 2->3 by moving 3 to the intersection point
        _coords[3] = cut_point;
      }
      else
        pgraph_cat.debug() << "ignored for now for simplicity \n";
    }
    else
      pgraph_cat.spam() << "is_intersect: " << is_intersect << " at t = " << t << endl;
  }
  // ok, now lets add the clipped portal
  _color = Colorf(1,0,0,1);
  move_to(_coords[0]);
  draw_to(_coords[1]);
  draw_to(_coords[2]);
  draw_to(_coords[3]);
  draw_to(_coords[0]);

  // 3rd level test, more accurate to determine if the portal is worth visiting
  pgraph_cat.debug() << "portal clipper flag: " << xect << endl;
  if (xect == 0xf) {  //if all four planes intersected the portal, it is visible
    return;
  }
  if (!is_partial_portal_in_view()) {
    pgraph_cat.debug() << "portal failed 3rd level test \n";
    _num_vert = 0;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::get_reduced_frustum
//       Access: Public
//  Description: After clipping the portal, form the new sides and 
//               fill in the new frustum. Return true if success
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) PortalClipper::
get_reduced_frustum(const NodePath &node_path)
{
  //  int num_planes = 6;
  LPoint3f intersect_points[4];
  
  // another approach to actually finding the points, so that 
  // I can reuse the current BoundingHexahedron object. Apparently,
  // it is better to construct this BH with bounding points, rather
  // than bounding planes (which I might have to implement soon)
  
  if (!_num_vert)
    return NULL;
  
  float t;
  bool visible = true;

  // find intersection of 7->3 with far
  LPoint3f from_origin = _reduced_frustum->get_point(7);
  LVector3f from_direction = _coords[3] - from_origin;
  bool is_intersect = _reduced_frustum->get_plane(0).intersects_line(t, from_origin, from_direction);
  if (is_intersect && t >= 0.0) { // has to be positive, else camera is not looking at the portal
    pgraph_cat.spam() << "far plane intersected 7->3 at t=" << t << endl;
    intersect_points[0] = from_origin + t*from_direction;
    pgraph_cat.spam() << intersect_points[0] << endl;
  }
  else
    visible = false;

  // find intersection of 4->0 with far
  from_origin = _reduced_frustum->get_point(4);
  from_direction = _coords[0] - from_origin;
  is_intersect = _reduced_frustum->get_plane(0).intersects_line(t, from_origin, from_direction);
  if (is_intersect && t >= 0.0) { // has to be positive, else camera is not looking at the portal
    pgraph_cat.spam() << "far plane intersected 4->0 at t=" << t << endl;
    intersect_points[1] = from_origin + t*from_direction;
    pgraph_cat.spam() << intersect_points[1] << endl;
  }
  else
    visible = false;

  // find intersection of 5->1 with far
  from_origin = _reduced_frustum->get_point(5);
  from_direction = _coords[1] - from_origin;
  is_intersect = _reduced_frustum->get_plane(0).intersects_line(t, from_origin, from_direction);
  if (is_intersect && t >= 0.0) { // has to be positive, else camera is not looking at the portal
    pgraph_cat.spam() << "far plane intersected 5->1 at t=" << t << endl;
    intersect_points[2] = from_origin + t*from_direction;
    pgraph_cat.spam() << intersect_points[2] << endl;
  }
  else
    visible = false;

  // find intersection of 6->2 with far
  from_origin = _reduced_frustum->get_point(6);
  from_direction = _coords[2] - from_origin;
  is_intersect = _reduced_frustum->get_plane(0).intersects_line(t, from_origin, from_direction);
  if (is_intersect && t >= 0.0) { // has to be positive, else camera is not looking at the portal
    pgraph_cat.spam() << "far plane intersected 6->2 at t=" << t << endl;
    intersect_points[3] = from_origin + t*from_direction;
    pgraph_cat.spam() << intersect_points[3] << endl;
  }
  else
    visible = false;

  if (!visible) {
    pgraph_cat.spam() << "portal is not visible from current camera look at" << endl;
    return NULL;
  }
  
  // With these intersect_points, construct the new reduced frustum
  PT(BoundingVolume) reduced_frustum = new
    BoundingHexahedron(intersect_points[1], intersect_points[2],
                       intersect_points[3], intersect_points[0],
                       _reduced_frustum->get_point(4), _reduced_frustum->get_point(5),
                       _reduced_frustum->get_point(6), _reduced_frustum->get_point(7));

  pgraph_cat.debug() << *reduced_frustum << endl;

  // draw this hexahedron
  _color = Colorf(0,0,1,1);
  draw_hexahedron(DCAST(BoundingHexahedron, reduced_frustum));

  return reduced_frustum;
}
