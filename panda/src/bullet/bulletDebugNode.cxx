// Filename: bulletDebugNode.cxx
// Created by:  enn0x (23Jan10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "bulletDebugNode.h"

#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "omniBoundingVolume.h"

TypeHandle BulletDebugNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletDebugNode::
BulletDebugNode(const char *name) : GeomNode(name) {

  _wireframe = true;
  _constraints = true;
  _bounds = false;
  _drawer._normals = true;

  _vdata = new GeomVertexData("", GeomVertexFormat::get_v3c4(), Geom::UH_stream);

  // Lines
  _prim_lines = new GeomLines(Geom::UH_stream);
  _prim_lines->set_shade_model(Geom::SM_uniform);

  _geom_lines = new Geom(_vdata);
  _geom_lines->add_primitive(_prim_lines);

  add_geom(_geom_lines);

  // Triangles
  _prim_triangles = new GeomTriangles(Geom::UH_stream);
  _prim_triangles->set_shade_model(Geom::SM_uniform);

  _geom_triangles = new Geom(_vdata);
  _geom_triangles->add_primitive(_prim_triangles);

  add_geom(_geom_triangles);

  // Draw something in oder to prevent getting optimized away
  GeomVertexWriter vwriter(_vdata, InternalName::get_vertex());
  vwriter.add_data3(0.0, 0.0, 0.0);
  vwriter.add_data3(0.0, 0.0, 0.0);
  vwriter.add_data3(0.0, 0.0, 0.0);
  _prim_lines->add_next_vertices(2);
  _prim_lines->close_primitive();
  _prim_triangles->add_next_vertices(3);
  _prim_triangles->close_primitive();

  CPT (BoundingVolume) bounds = new OmniBoundingVolume();
  set_bounds(bounds);
  set_final(true);
  set_overall_hidden(true);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool BulletDebugNode::
safe_to_flatten() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool BulletDebugNode::
safe_to_transform() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::safe_to_modify_transform
//       Access: Public, Virtual
//  Description: Returns true if it is safe to automatically adjust
//               the transform on this kind of node.  Usually, this is
//               only a bad idea if the user expects to find a
//               particular transform on the node.
//
//               ModelNodes with the preserve_transform flag set are
//               presently the only kinds of nodes that should not
//               have their transform even adjusted.
////////////////////////////////////////////////////////////////////
bool BulletDebugNode::
safe_to_modify_transform() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes of compatible type, adding children or
//               whatever.  For instance, an LODNode should not be
//               combined with any other PandaNode, because its set of
//               children is meaningful.
////////////////////////////////////////////////////////////////////
bool BulletDebugNode::
safe_to_combine() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::safe_to_combine_children
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine the
//               children of this PandaNode with each other.  For
//               instance, an LODNode's children should not be
//               combined with each other, because the set of children
//               is meaningful.
////////////////////////////////////////////////////////////////////
bool BulletDebugNode::
safe_to_combine_children() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::safe_to_flatten_below
//       Access: Public, Virtual
//  Description: Returns true if a flatten operation may safely
//               continue past this node, or false if nodes below this
//               node may not be molested.
////////////////////////////////////////////////////////////////////
bool BulletDebugNode::
safe_to_flatten_below() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::draw_mask_changed
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::
draw_mask_changed() {

  if (is_overall_hidden()) {
    _drawer.setDebugMode(DebugDraw::DBG_NoDebug);
  }
  else {
    int mode = DebugDraw::DBG_DrawText |
               DebugDraw::DBG_DrawFeaturesText |
               DebugDraw::DBG_DrawContactPoints;

    if (_wireframe) {
      mode |= DebugDraw::DBG_DrawWireframe;
    }

    if (_constraints) {
      mode |= DebugDraw::DBG_DrawConstraints;
      mode |= DebugDraw::DBG_DrawConstraintLimits;
    }

    if (_bounds) {
      mode |= DebugDraw::DBG_DrawAabb;
    }

    _drawer.setDebugMode(mode);
  } 
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::sync_b2p
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::
sync_b2p(btDynamicsWorld *world) {

  if (is_overall_hidden()) return;

  // Collect debug geometry data
  world->debugDrawWorld();

  // Get inverse of this node's net transform
  NodePath np = NodePath::any_path((PandaNode *)this);
  LMatrix4 m = np.get_net_transform()->get_mat();
  m.invert_in_place();

  // Render collected data
  _prim_lines->clear_vertices();
  _prim_triangles->clear_vertices();

  GeomVertexWriter vwriter(_vdata, InternalName::get_vertex());
  GeomVertexWriter cwriter(_vdata, InternalName::get_color());

  int v = 0;

  pvector<Line>::const_iterator lit;
  pvector<Triangle>::const_iterator tit;

  for (lit = _drawer._lines.begin(); lit != _drawer._lines.end(); lit++) {
    Line line = *lit;

    vwriter.add_data3(m.xform_point(line._p0));
    vwriter.add_data3(m.xform_point(line._p1));
    cwriter.add_data4(line._color);
    cwriter.add_data4(line._color);

    _prim_lines->add_vertex(v++);
    _prim_lines->add_vertex(v++);
    _prim_lines->close_primitive();
  }

  for (tit = _drawer._triangles.begin(); tit != _drawer._triangles.end(); tit++) {
    Triangle tri = *tit;

    vwriter.add_data3(m.xform_point(tri._p0));
    vwriter.add_data3(m.xform_point(tri._p1));
    vwriter.add_data3(m.xform_point(tri._p2));
    cwriter.add_data4(tri._color);
    cwriter.add_data4(tri._color);
    cwriter.add_data4(tri._color);

    _prim_triangles->add_vertex(v++);
    _prim_triangles->add_vertex(v++);
    _prim_triangles->add_vertex(v++);
    _prim_triangles->close_primitive();
  }

  // Clear the collected data again
  _drawer._lines.clear();
  _drawer._triangles.clear();

  // Force recompute of bounds
  np.force_recompute_bounds();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::setDebugMode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::DebugDraw::
setDebugMode(int mode) {

  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::getDebugMode
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int BulletDebugNode::DebugDraw::
getDebugMode() const {

  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::reportErrorWarning
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::DebugDraw::
reportErrorWarning(const char *warning) {

  bullet_cat.error() << warning << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::drawLine
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::DebugDraw::
drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {

  PN_stdfloat r = color.getX();
  PN_stdfloat g = color.getY();
  PN_stdfloat b = color.getZ();

  // Hack to get rid of triangle normals. The hack is based on the
  // assumption that only normals are drawn in yellow.
  if (_normals==false && r==1.0f && g==1.0f && b==0.0f) return;

  Line line;

  line._p0 = LVecBase3(from.getX(), from.getY(), from.getZ());
  line._p1 = LVecBase3(to.getX(), to.getY(), to.getZ());
  line._color = LColor(r, g, b, 1.0f);

  _lines.push_back(line);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::drawTriangle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::DebugDraw::
drawTriangle(const btVector3 &v0, const btVector3 &v1, const btVector3 &v2, const btVector3 &color, btScalar) {

  btScalar r = color.getX();
  btScalar g = color.getY();
  btScalar b = color.getZ();

  Triangle tri;

  tri._p0 = LVecBase3((PN_stdfloat)v0.getX(),
                      (PN_stdfloat)v0.getY(),
                      (PN_stdfloat)v0.getZ());

  tri._p1 = LVecBase3((PN_stdfloat)v1.getX(),
                      (PN_stdfloat)v1.getY(),
                      (PN_stdfloat)v1.getZ());

  tri._p2 = LVecBase3((PN_stdfloat)v2.getX(),
                      (PN_stdfloat)v2.getY(),
                      (PN_stdfloat)v2.getZ());

  tri._color = LColor((PN_stdfloat)r, 
                      (PN_stdfloat)g, 
                      (PN_stdfloat)b, 1.0f);

  _triangles.push_back(tri);


  // Draw the triangle's normal
/*
  btVector3 x1 = v1 - v0;
  btVector3 x2 = v2 - v0;
  btVector3 normal = v1.cross(v2).normalize();

  btVector3 from = (v0 + v1 + v2) * 0.3333;
  btVector3 to = from + normal;
  drawLine(from, to, color);
*/
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::drawTriangle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::DebugDraw::
drawTriangle(const btVector3 &v0, const btVector3 &v1, const btVector3 &v2, const btVector3 &n0, const btVector3 &n1, const btVector3 &n2, const btVector3 &color, btScalar alpha) {

  bullet_cat.debug() << "drawTriangle(2) - not yet implemented!" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::drawContactPoint
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::DebugDraw::
drawContactPoint(const btVector3 &point, const btVector3 &normal, btScalar distance, int lifetime, const btVector3 &color) {

  const btVector3 to = point + normal * distance;
  const btVector3 &from = point;

  drawLine(from, to, color);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::draw3dText
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::DebugDraw::
draw3dText(const btVector3 &location, const char *text) {

  bullet_cat.debug() << "draw3dText - not yet implemented!" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletDebugNode::DebugDraw::drawSphere
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletDebugNode::DebugDraw::
drawSphere(btScalar radius, const btTransform &transform, const btVector3 &color) {

  btVector3 center = transform.getOrigin();

  const btVector3 xoffs = transform.getBasis() * btVector3(1, 0, 0);
  const btVector3 yoffs = transform.getBasis() * btVector3(0, 1, 0);
  const btVector3 zoffs = transform.getBasis() * btVector3(0, 0, 1);

  drawArc(center, xoffs, yoffs, radius, radius, 0, SIMD_2_PI, color, false, 10.0);
  drawArc(center, yoffs, zoffs, radius, radius, 0, SIMD_2_PI, color, false, 10.0);
  drawArc(center, zoffs, xoffs, radius, radius, 0, SIMD_2_PI, color, false, 10.0);
}

