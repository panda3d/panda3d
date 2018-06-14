/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionVisualizer.cxx
 * @author drose
 * @date 2003-04-16
 */

#include "collisionVisualizer.h"
#include "collisionEntry.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "renderState.h"
#include "renderModeAttrib.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "geom.h"
#include "geomPoints.h"
#include "geomLines.h"
#include "omniBoundingVolume.h"
#include "depthOffsetAttrib.h"
#include "colorScaleAttrib.h"
#include "transparencyAttrib.h"
#include "clipPlaneAttrib.h"
#include "geomVertexWriter.h"


#ifdef DO_COLLISION_RECORDING

TypeHandle CollisionVisualizer::_type_handle;

/**
 *
 */
CollisionVisualizer::
CollisionVisualizer(const std::string &name) : PandaNode(name), _lock("CollisionVisualizer") {
  set_cull_callback();

  // We always want to render the CollisionVisualizer node itself (even if it
  // doesn't appear to have any geometry within it).
  set_internal_bounds(new OmniBoundingVolume());
  _point_scale = 1.0f;
  _normal_scale = 1.0f;
}

/**
 * Copy constructor.
 */
CollisionVisualizer::
CollisionVisualizer(const CollisionVisualizer &copy) :
  PandaNode(copy),
  _lock("CollisionVisualizer"),
  _point_scale(copy._point_scale),
  _normal_scale(copy._normal_scale) {

  set_cull_callback();

  // We always want to render the CollisionVisualizer node itself (even if it
  // doesn't appear to have any geometry within it).
  set_internal_bounds(new OmniBoundingVolume());
}

/**
 *
 */
CollisionVisualizer::
~CollisionVisualizer() {
}

/**
 * Removes all the visualization data from a previous traversal and resets the
 * visualizer to empty.
 */
void CollisionVisualizer::
clear() {
  LightMutexHolder holder(_lock);
  _data.clear();
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *CollisionVisualizer::
make_copy() const {
  return new CollisionVisualizer(*this);
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool CollisionVisualizer::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // Now we go through and actually draw our visualized collision solids.

  LightMutexHolder holder(_lock);

  Data::const_iterator di;
  for (di = _data.begin(); di != _data.end(); ++di) {
    const TransformState *net_transform = (*di).first;
    const VizInfo &viz_info = (*di).second;

    CullTraverserData xform_data(data);

    // We don't want to inherit the transform from above!  We ignore whatever
    // transforms were above the CollisionVisualizer node; it always renders
    // its objects according to their appropriate net transform.
    xform_data._net_transform = TransformState::make_identity();
    xform_data._view_frustum = trav->get_view_frustum();
    xform_data.apply_transform(net_transform);

    // Draw all the collision solids.
    Solids::const_iterator si;
    for (si = viz_info._solids.begin(); si != viz_info._solids.end(); ++si) {
      // Note that we don't preserve the clip plane attribute from the
      // collision solid.  We always draw the whole polygon (or whatever) in
      // the CollisionVisualizer.  This is a deliberate decision; clipping the
      // polygons may obscure many collision tests that are being made.
      const CollisionSolid *solid = (*si).first;
      const SolidInfo &solid_info = (*si).second;
      bool was_detected = (solid_info._detected_count > 0);
      PT(PandaNode) node = solid->get_viz(trav, xform_data, !was_detected);
      if (node != nullptr) {
        CullTraverserData next_data(xform_data, node);

        // We don't want to inherit the render state from above for these
        // guys.
        next_data._state = get_viz_state();
        trav->traverse(next_data);
      }
    }

    // Now draw all of the detected points.
    if (!viz_info._points.empty()) {
      CPT(RenderState) empty_state = RenderState::make_empty();
      CPT(RenderState) point_state = RenderState::make(RenderModeAttrib::make(RenderModeAttrib::M_unchanged, 1.0f, false));

      PT(GeomVertexArrayFormat) point_array_format =
        new GeomVertexArrayFormat(InternalName::get_vertex(), 3,
                                  Geom::NT_stdfloat, Geom::C_point,
                                  InternalName::get_color(), 1,
                                  Geom::NT_packed_dabc, Geom::C_color,
                                  InternalName::get_size(), 1,
                                  Geom::NT_stdfloat, Geom::C_other);
      CPT(GeomVertexFormat) point_format =
        GeomVertexFormat::register_format(point_array_format);

      Points::const_iterator pi;
      for (pi = viz_info._points.begin(); pi != viz_info._points.end(); ++pi) {
        const CollisionPoint &point = (*pi);

        // Draw a small red point at the surface point, and a smaller white
        // point at the interior point.
        {
          PT(GeomVertexData) point_vdata =
            new GeomVertexData("viz", point_format, Geom::UH_stream);

          PT(GeomPoints) points = new GeomPoints(Geom::UH_stream);

          GeomVertexWriter vertex(point_vdata, InternalName::get_vertex());
          GeomVertexWriter color(point_vdata, InternalName::get_color());
          GeomVertexWriter size(point_vdata, InternalName::get_size());

          vertex.add_data3(point._surface_point);
          color.add_data4(1.0f, 0.0f, 0.0f, 1.0f);
          size.add_data1f(16.0f * _point_scale);
          points->add_next_vertices(1);
          points->close_primitive();

          if (point._interior_point != point._surface_point) {
            vertex.add_data3(point._interior_point);
            color.add_data4(1.0f, 1.0f, 1.0f, 1.0f);
            size.add_data1f(8.0f * _point_scale);
            points->add_next_vertices(1);
            points->close_primitive();
          }

          PT(Geom) geom = new Geom(point_vdata);
          geom->add_primitive(points);

          CullableObject *object =
            new CullableObject(geom, point_state,
                               xform_data.get_internal_transform(trav));

          trav->get_cull_handler()->record_object(object, trav);
        }

        // Draw the normal vector at the surface point.
        if (!point._surface_normal.almost_equal(LVector3::zero())) {
          PT(GeomVertexData) line_vdata =
            new GeomVertexData("viz", GeomVertexFormat::get_v3cp(),
                               Geom::UH_stream);

          PT(GeomLines) lines = new GeomLines(Geom::UH_stream);

          GeomVertexWriter vertex(line_vdata, InternalName::get_vertex());
          GeomVertexWriter color(line_vdata, InternalName::get_color());

          vertex.add_data3(point._surface_point);
          vertex.add_data3(point._surface_point +
                            point._surface_normal * _normal_scale);
          color.add_data4(1.0f, 0.0f, 0.0f, 1.0f);
          color.add_data4(1.0f, 1.0f, 1.0f, 1.0f);
          lines->add_next_vertices(2);
          lines->close_primitive();

          PT(Geom) geom = new Geom(line_vdata);
          geom->add_primitive(lines);

          CullableObject *object =
            new CullableObject(geom, empty_state,
                               xform_data.get_internal_transform(trav));

          trav->get_cull_handler()->record_object(object, trav);
        }
      }
    }
  }

  // Now carry on to render our child nodes.
  return true;
}

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool CollisionVisualizer::
is_renderable() const {
  return true;
}


/**
 * Writes a brief description of the node to the indicated output stream.
 * This is invoked by the << operator.  It may be overridden in derived
 * classes to include some information relevant to the class.
 */
void CollisionVisualizer::
output(std::ostream &out) const {
  PandaNode::output(out);
  out << " ";
  CollisionRecorder::output(out);
}

/**
 * This method is called at the beginning of a CollisionTraverser::traverse()
 * call.  It is provided as a hook for the derived class to reset its state as
 * appropriate.
 */
void CollisionVisualizer::
begin_traversal() {
  CollisionRecorder::begin_traversal();
  LightMutexHolder holder(_lock);
  _data.clear();
}

/**
 * This method is called when a pair of collision solids have passed all
 * bounding-volume tests and have been tested for a collision.  The detected
 * value is set true if a collision was detected, false otherwise.
 */
void CollisionVisualizer::
collision_tested(const CollisionEntry &entry, bool detected) {
  CollisionRecorder::collision_tested(entry, detected);

  NodePath node_path = entry.get_into_node_path();
  CPT(TransformState) net_transform = node_path.get_net_transform();
  CPT(CollisionSolid) solid = entry.get_into();
  nassertv(!solid.is_null());

  LightMutexHolder holder(_lock);
  VizInfo &viz_info = _data[std::move(net_transform)];
  if (detected) {
    viz_info._solids[std::move(solid)]._detected_count++;

    if (entry.has_surface_point()) {
      CollisionPoint p;
      entry.get_all(entry.get_into_node_path(),
                    p._surface_point, p._surface_normal, p._interior_point);
      viz_info._points.push_back(p);
    }

  } else {
    viz_info._solids[std::move(solid)]._missed_count++;
  }
}


/**
 * Returns a RenderState suitable for rendering the collision solids with
 * which a collision was detected.
 */
CPT(RenderState) CollisionVisualizer::
get_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make
      (DepthOffsetAttrib::make());
  }

  return state;
}

#endif  // DO_COLLISION_RECORDING
