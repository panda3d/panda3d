/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletDebugNode.cxx
 * @author enn0x
 * @date 2010-01-23
 */

#include "bulletDebugNode.h"

#include "config_bullet.h"

#include "bulletWorld.h"

#include "cullHandler.h"
#include "cullTraverser.h"
#include "cullableObject.h"
#include "geomLines.h"
#include "geomVertexData.h"
#include "geomTriangles.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "omniBoundingVolume.h"
#include "pStatTimer.h"

TypeHandle BulletDebugNode::_type_handle;
PStatCollector BulletDebugNode::_pstat_debug("App:Bullet:DoPhysics:Debug");

/**
 *
 */
BulletDebugNode::
BulletDebugNode(const char *name) : PandaNode(name) {

  _debug_stale = false;
  _debug_world = nullptr;
  _wireframe = true;
  _constraints = true;
  _bounds = false;
  _drawer._normals = true;

  CPT (BoundingVolume) bounds = new OmniBoundingVolume();
  set_bounds(bounds);
  set_final(true);
  set_overall_hidden(true);
}

/**
 * Returns true if it is generally safe to flatten out this particular kind of
 * Node by duplicating instances, false otherwise (for instance, a Camera
 * cannot be safely flattened, because the Camera pointer itself is
 * meaningful).
 */
bool BulletDebugNode::
safe_to_flatten() const {

  return false;
}

/**
 * Returns true if it is generally safe to transform this particular kind of
 * Node by calling the xform() method, false otherwise.  For instance, it's
 * usually a bad idea to attempt to xform a Character.
 */
bool BulletDebugNode::
safe_to_transform() const {

  return false;
}

/**
 * Returns true if it is safe to automatically adjust the transform on this
 * kind of node.  Usually, this is only a bad idea if the user expects to find
 * a particular transform on the node.
 *
 * ModelNodes with the preserve_transform flag set are presently the only
 * kinds of nodes that should not have their transform even adjusted.
 */
bool BulletDebugNode::
safe_to_modify_transform() const {

  return false;
}

/**
 * Returns true if it is generally safe to combine this particular kind of
 * PandaNode with other kinds of PandaNodes of compatible type, adding
 * children or whatever.  For instance, an LODNode should not be combined with
 * any other PandaNode, because its set of children is meaningful.
 */
bool BulletDebugNode::
safe_to_combine() const {

  return false;
}

/**
 * Returns true if it is generally safe to combine the children of this
 * PandaNode with each other.  For instance, an LODNode's children should not
 * be combined with each other, because the set of children is meaningful.
 */
bool BulletDebugNode::
safe_to_combine_children() const {

  return false;
}

/**
 * Returns true if a flatten operation may safely continue past this node, or
 * false if nodes below this node may not be molested.
 */
bool BulletDebugNode::
safe_to_flatten_below() const {

  return false;
}

/**
 *
 */
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

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool BulletDebugNode::
is_renderable() const {
  return true;
}

/**
 * Adds the node's contents to the CullResult we are building up during the
 * cull traversal, so that it will be drawn at render time.  For most nodes
 * other than GeomNodes, this is a do-nothing operation.
 */
void BulletDebugNode::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  PT(Geom) debug_lines;
  PT(Geom) debug_triangles;

  {
    LightMutexHolder holder(BulletWorld::get_global_lock());
    if (_debug_world == nullptr) {
      return;
    }
    if (_debug_stale) {
      nassertv(_debug_world != nullptr);
      PStatTimer timer(_pstat_debug);

      // Collect debug geometry data
      _drawer._lines.clear();
      _drawer._triangles.clear();

      _debug_world->debugDrawWorld();

      // Render lines
      {
        PT(GeomVertexData) vdata =
          new GeomVertexData("", GeomVertexFormat::get_v3c4(), Geom::UH_stream);
        vdata->unclean_set_num_rows(_drawer._lines.size() * 2);

        GeomVertexWriter vwriter(vdata, InternalName::get_vertex());
        GeomVertexWriter cwriter(vdata, InternalName::get_color());

        pvector<Line>::const_iterator lit;
        for (lit = _drawer._lines.begin(); lit != _drawer._lines.end(); lit++) {
          const Line &line = *lit;

          vwriter.set_data3(line._p0);
          vwriter.set_data3(line._p1);
          cwriter.set_data4(LVecBase4(line._color));
          cwriter.set_data4(LVecBase4(line._color));
        }

        PT(GeomPrimitive) prim = new GeomLines(Geom::UH_stream);
        prim->set_shade_model(Geom::SM_uniform);
        prim->add_next_vertices(_drawer._lines.size() * 2);

        debug_lines = new Geom(vdata);
        debug_lines->add_primitive(prim);
        _debug_lines = debug_lines;
      }

      // Render triangles
      {
        PT(GeomVertexData) vdata =
          new GeomVertexData("", GeomVertexFormat::get_v3c4(), Geom::UH_stream);
        vdata->unclean_set_num_rows(_drawer._triangles.size() * 3);

        GeomVertexWriter vwriter(vdata, InternalName::get_vertex());
        GeomVertexWriter cwriter(vdata, InternalName::get_color());

        pvector<Triangle>::const_iterator tit;
        for (tit = _drawer._triangles.begin(); tit != _drawer._triangles.end(); tit++) {
          const Triangle &tri = *tit;

          vwriter.set_data3(tri._p0);
          vwriter.set_data3(tri._p1);
          vwriter.set_data3(tri._p2);
          cwriter.set_data4(LVecBase4(tri._color));
          cwriter.set_data4(LVecBase4(tri._color));
          cwriter.set_data4(LVecBase4(tri._color));
        }

        PT(GeomPrimitive) prim = new GeomTriangles(Geom::UH_stream);
        prim->set_shade_model(Geom::SM_uniform);
        prim->add_next_vertices(_drawer._triangles.size() * 3);

        debug_triangles = new Geom(vdata);
        debug_triangles->add_primitive(prim);
        _debug_triangles = debug_triangles;
      }

      // Clear collected data.
      _drawer._lines.clear();
      _drawer._triangles.clear();

      _debug_stale = false;
    } else {
      debug_lines = _debug_lines;
      debug_triangles = _debug_triangles;
    }
  }

  // Record them without any state or transform.
  trav->_geoms_pcollector.add_level(2);
  {
    CullableObject *object =
      new CullableObject(std::move(debug_lines), RenderState::make_empty(), trav->get_scene()->get_cs_world_transform());
    trav->get_cull_handler()->record_object(object, trav);
  }
  {
    CullableObject *object =
      new CullableObject(std::move(debug_triangles), RenderState::make_empty(), trav->get_scene()->get_cs_world_transform());
    trav->get_cull_handler()->record_object(object, trav);
  }
}

/**
 *
 */
void BulletDebugNode::
do_sync_b2p(btDynamicsWorld *world) {

  _debug_world = world;
  _debug_stale = true;
}

/**
 *
 */
void BulletDebugNode::DebugDraw::
setDebugMode(int mode) {

  _mode = mode;
}

/**
 *
 */
int BulletDebugNode::DebugDraw::
getDebugMode() const {

  return _mode;
}

/**
 *
 */
void BulletDebugNode::DebugDraw::
reportErrorWarning(const char *warning) {

  bullet_cat.error() << warning << std::endl;
}

/**
 *
 */
void BulletDebugNode::DebugDraw::
drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {

  PN_stdfloat r = color.getX();
  PN_stdfloat g = color.getY();
  PN_stdfloat b = color.getZ();

  // Hack to get rid of triangle normals.  The hack is based on the assumption
  // that only normals are drawn in yellow.
  if (_normals==false && r==1.0f && g==1.0f && b==0.0f) return;

  Line line;

  line._p0 = LVecBase3((PN_stdfloat)from.getX(),
                       (PN_stdfloat)from.getY(),
                       (PN_stdfloat)from.getZ());
  line._p1 = LVecBase3((PN_stdfloat)to.getX(),
                       (PN_stdfloat)to.getY(),
                       (PN_stdfloat)to.getZ());
  line._color = UnalignedLVecBase4((PN_stdfloat)r,
                                   (PN_stdfloat)g,
                                   (PN_stdfloat)b, 1.0f);

  _lines.push_back(line);
}

/**
 *
 */
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

  tri._color = UnalignedLVecBase4((PN_stdfloat)r,
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

/**
 *
 */
void BulletDebugNode::DebugDraw::
drawTriangle(const btVector3 &v0, const btVector3 &v1, const btVector3 &v2, const btVector3 &n0, const btVector3 &n1, const btVector3 &n2, const btVector3 &color, btScalar alpha) {
  if (bullet_cat.is_debug()) {
    bullet_cat.debug() << "drawTriangle(2) - not yet implemented!" << std::endl;
  }
}

/**
 *
 */
void BulletDebugNode::DebugDraw::
drawContactPoint(const btVector3 &point, const btVector3 &normal, btScalar distance, int lifetime, const btVector3 &color) {

  const btVector3 to = point + normal * distance;
  const btVector3 &from = point;

  drawLine(from, to, color);
}

/**
 *
 */
void BulletDebugNode::DebugDraw::
draw3dText(const btVector3 &location, const char *text) {
  if (bullet_cat.is_debug()) {
    bullet_cat.debug() << "draw3dText - not yet implemented!" << std::endl;
  }
}

/**
 *
 */
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

/**
 * Tells the BamReader how to create objects of type BulletDebugNode.
 */
void BulletDebugNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletDebugNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);

  dg.add_bool(_wireframe);
  dg.add_bool(_constraints);
  dg.add_bool(_bounds);
  dg.add_bool(_drawer._normals);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * this type is encountered in the Bam file.  It should create the rigid body
 * and extract its information from the file.
 */
TypedWritable *BulletDebugNode::
make_from_bam(const FactoryParams &params) {
  BulletDebugNode *param = new BulletDebugNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletDebugNode.
 */
void BulletDebugNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  _wireframe = scan.get_bool();
  _constraints = scan.get_bool();
  _bounds = scan.get_bool();
  _drawer._normals = scan.get_bool();
}
