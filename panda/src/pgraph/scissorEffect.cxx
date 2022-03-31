/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file scissorEffect.cxx
 * @author drose
 * @date 2008-07-30
 */

#include "scissorEffect.h"
#include "scissorAttrib.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "nodePath.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "boundingHexahedron.h"
#include "lens.h"

using std::max;
using std::min;

TypeHandle ScissorEffect::_type_handle;

/**
 * Use ScissorEffect::make() to construct a new ScissorEffect object.
 */
ScissorEffect::
ScissorEffect(bool screen, const LVecBase4 &frame,
              const PointDef *points, int num_points, bool clip) :
  _screen(screen), _frame(frame), _clip(clip)
{
  _points.reserve(num_points);
  for (int i = 0; i < num_points; ++i) {
    _points.push_back(points[i]);
  }
}

/**
 * Use ScissorEffect::make() to construct a new ScissorEffect object.
 */
ScissorEffect::
ScissorEffect(const ScissorEffect &copy) :
  _screen(copy._screen),
  _frame(copy._frame),
  _points(copy._points),
  _clip(copy._clip)
{
}

/**
 * Constructs a new screen-relative ScissorEffect.  The frame defines a left,
 * right, bottom, top region, relative to the DisplayRegion.  See
 * ScissorAttrib.
 */
CPT(RenderEffect) ScissorEffect::
make_screen(const LVecBase4 &frame, bool clip) {
  ScissorEffect *effect = new ScissorEffect(true, frame, nullptr, 0, clip);
  return return_new(effect);
}

/**
 * Constructs a new node-relative ScissorEffect, with no points.  This empty
 * ScissorEffect does nothing.  You must then call add_point a number of times
 * to add the points you require.
 */
CPT(RenderEffect) ScissorEffect::
make_node(bool clip) {
  ScissorEffect *effect = new ScissorEffect(false, LVecBase4::zero(), nullptr, 0, clip);
  return return_new(effect);
}

/**
 * Constructs a new node-relative ScissorEffect.  The two points are
 * understood to be relative to the indicated node, or the current node if the
 * NodePath is empty, and determine the diagonally opposite corners of the
 * scissor region.
 */
CPT(RenderEffect) ScissorEffect::
make_node(const LPoint3 &a, const LPoint3 &b, const NodePath &node) {
  PointDef points[2];
  points[0]._p = a;
  points[0]._node = node;
  points[1]._p = b;
  points[1]._node = node;
  ScissorEffect *effect = new ScissorEffect(false, LVecBase4::zero(), points, 2, true);
  return return_new(effect);
}

/**
 * Constructs a new node-relative ScissorEffect.  The four points are
 * understood to be relative to the indicated node, or the current node if the
 * indicated NodePath is empty, and determine four points surrounding the
 * scissor region.
 */
CPT(RenderEffect) ScissorEffect::
make_node(const LPoint3 &a, const LPoint3 &b, const LPoint3 &c, const LPoint3 &d, const NodePath &node) {
  PointDef points[4];
  points[0]._p = a;
  points[0]._node = node;
  points[1]._p = b;
  points[1]._node = node;
  points[2]._p = c;
  points[2]._node = node;
  points[3]._p = d;
  points[3]._node = node;
  ScissorEffect *effect = new ScissorEffect(false, LVecBase4::zero(), points, 4, true);
  return return_new(effect);
}

/**
 * Returns a new ScissorEffect with the indicated point added.  It is only
 * valid to call this on a "node" type ScissorEffect.  The full set of points,
 * projected into screen space, defines the bounding volume of the rectangular
 * scissor region.
 *
 * Each point may be relative to a different node, if desired.
 */
CPT(RenderEffect) ScissorEffect::
add_point(const LPoint3 &p, const NodePath &node) const {
  nassertr(!is_screen(), this);
  ScissorEffect *effect = new ScissorEffect(*this);
  PointDef point;
  point._p = p;
  point._node = node;
  effect->_points.push_back(point);
  return return_new(effect);
}

/**
 * Returns a new RenderEffect transformed by the indicated matrix.
 */
CPT(RenderEffect) ScissorEffect::
xform(const LMatrix4 &mat) const {
  if (is_screen()) {
    return this;
  }
  ScissorEffect *effect = new ScissorEffect(*this);
  Points::iterator pi;
  for (pi = effect->_points.begin();
       pi != effect->_points.end();
       ++pi) {
    PointDef &point = (*pi);
    if (point._node.is_empty()) {
      point._p = point._p * mat;
    }
  }
  return return_new(effect);
}

/**
 *
 */
void ScissorEffect::
output(std::ostream &out) const {
  out << get_type() << ":";
  if (is_screen()) {
    out << "screen [" << _frame << "]";
  } else {
    out << "node";
    Points::const_iterator pi;
    for (pi = _points.begin(); pi != _points.end(); ++pi) {
      const PointDef &point = (*pi);
      if (point._node.is_empty()) {
        out << " (" << point._p << ")";
      } else {
        out << " (" << point._node << ":" << point._p << ")";
      }
    }
  }
  if (!get_clip()) {
    out << " !clip";
  }
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this effect during the cull traversal.
 */
bool ScissorEffect::
has_cull_callback() const {
  return true;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.  This may include additional manipulation of render
 * state or additional visible/invisible decisions, or any other arbitrary
 * operation.
 *
 * At the time this function is called, the current node's transform and state
 * have not yet been applied to the net_transform and net_state.  This
 * callback may modify the node_transform and node_state to apply an effective
 * change to the render state at this level.
 */
void ScissorEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &node_state) const {
  LVecBase4 frame;
  const Lens *lens = trav->get_scene()->get_lens();
  CPT(TransformState) modelview_transform = data.get_modelview_transform(trav);
  CPT(TransformState) net_transform = modelview_transform->compose(node_transform);
  if (net_transform->is_singular()) {
    // If we're under a singular transform, never mind.
    return;
  }

  if (is_screen()) {
    frame = _frame;
  } else {
    const LMatrix4 &proj_mat = lens->get_projection_mat();
    LMatrix4 net_mat = net_transform->get_mat() * proj_mat;

    bool any_points = false;

    Points::const_iterator pi;
    for (pi = _points.begin(); pi != _points.end(); ++pi) {
      const PointDef &point = (*pi);
      LVecBase4 pv(point._p[0], point._p[1], point._p[2], 1.0f);
      if (point._node.is_empty()) {
        // Relative to this node.
        pv = pv * net_mat;

      } else {
        // Relative to some other node.
        LMatrix4 other_mat = point._node.get_net_transform()->get_mat() * proj_mat;
        pv = pv * other_mat;
      }

      if (pv[3] == 0) {
        continue;
      }
      LPoint3 pr(pv[0] / pv[3], pv[1] / pv[3], pv[2] / pv[3]);
      if (!any_points) {
        frame[0] = pr[0];
        frame[1] = pr[0];
        frame[2] = pr[1];
        frame[3] = pr[1];
        any_points = true;
      } else {
        frame[0] = min(frame[0], pr[0]);
        frame[1] = max(frame[1], pr[0]);
        frame[2] = min(frame[2], pr[1]);
        frame[3] = max(frame[3], pr[1]);
      }
    }

    // Scale from -1..1 to 0..1.
    frame[0] = (frame[0] + 1.0f) * 0.5f;
    frame[1] = (frame[1] + 1.0f) * 0.5f;
    frame[2] = (frame[2] + 1.0f) * 0.5f;
    frame[3] = (frame[3] + 1.0f) * 0.5f;
  }

  // Impose bounding volumes.
  frame[0] = max(min(frame[0], (PN_stdfloat)1.0), (PN_stdfloat)0.0);
  frame[1] = max(min(frame[1], (PN_stdfloat)1.0), frame[0]);
  frame[2] = max(min(frame[2], (PN_stdfloat)1.0), (PN_stdfloat)0.0);
  frame[3] = max(min(frame[3], (PN_stdfloat)1.0), frame[2]);

  if (_clip) {
    CPT(RenderAttrib) scissor_attrib = ScissorAttrib::make(frame);
    CPT(RenderState) state = RenderState::make(scissor_attrib);
    node_state = node_state->compose(state);
  }

  // Set up the culling.  We do this by extruding the four corners of the
  // frame into the eight corners of the bounding frustum.
  PT(GeometricBoundingVolume) frustum = make_frustum(lens, frame);
  if (frustum != nullptr) {
    frustum->xform(modelview_transform->get_inverse()->get_mat());
    data._view_frustum = frustum;
  }
}

/**
 * Intended to be overridden by derived ScissorEffect types to return a unique
 * number indicating whether this ScissorEffect is equivalent to the other
 * one.
 *
 * This should return 0 if the two ScissorEffect objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two ScissorEffect objects whose get_type()
 * functions return the same.
 */
int ScissorEffect::
compare_to_impl(const RenderEffect *other) const {
  const ScissorEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_screen != ta->_screen) {
    return (int)_screen - (int)ta->_screen;
  }
  if (_clip != ta->_clip) {
    return (int)_clip - (int)ta->_clip;
  }
  if (_screen) {
    int compare = _frame.compare_to(ta->_frame);
    if (compare != 0) {
      return compare;
    }
  } else {
    int compare = (int)_points.size() - (int)ta->_points.size();
    if (compare != 0) {
      return compare;
    }
    for (size_t i = 0; i < _points.size(); ++i) {
      compare = _points[i]._p.compare_to(ta->_points[i]._p);
      if (compare != 0) {
        return compare;
      }
      compare = _points[i]._node.compare_to(ta->_points[i]._node);
      if (compare != 0) {
        return compare;
      }
    }
  }
  return 0;
}

/**
 * Tells the BamReader how to create objects of type ScissorEffect.
 */
void ScissorEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void ScissorEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);

  dg.add_bool(_screen);
  if (_screen) {
    _frame.write_datagram(dg);
  } else {
    dg.add_uint16(_points.size());
    Points::const_iterator pi;
    for (pi = _points.begin(); pi != _points.end(); ++pi) {
      (*pi)._p.write_datagram(dg);
    }
  }
  dg.add_bool(_clip);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type ScissorEffect is encountered in the Bam file.  It should create the
 * ScissorEffect and extract its information from the file.
 */
TypedWritable *ScissorEffect::
make_from_bam(const FactoryParams &params) {
  ScissorEffect *effect = new ScissorEffect(true, LVecBase4::zero(), nullptr, 0, false);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new ScissorEffect.
 */
void ScissorEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);

  _screen = scan.get_bool();
  if (_screen) {
    _frame.read_datagram(scan);
  } else {
    int num_points = scan.get_uint16();
    _points.reserve(num_points);
    for (int i = 0; i < num_points; ++i) {
      PointDef point;
      point._p.read_datagram(scan);
      _points.push_back(point);
    }
  }
  _clip = scan.get_bool();
}

/**
 * Constructs a new bounding frustum from the lens properties, given the
 * indicated scissor frame.
 */
PT(GeometricBoundingVolume) ScissorEffect::
make_frustum(const Lens *lens, const LVecBase4 &frame) const{
  // Scale the frame from 0 .. 1 into -1 .. 1.
  LVecBase4 f2(frame[0] * 2.0f - 1.0f,
               frame[1] * 2.0f - 1.0f,
               frame[2] * 2.0f - 1.0f,
               frame[3] * 2.0f - 1.0f);

  LPoint3 fll, flr, ful, fur;
  LPoint3 nll, nlr, nul, nur;
  LPoint2 corner;

  corner[0] = f2[0]; corner[1] = f2[3];

  // Upper left.
  if (!lens->extrude(corner, nul, ful)) {
    return nullptr;
  }

  corner[0] = f2[1]; corner[1] = f2[3];

  // Upper right.
  if (!lens->extrude(corner, nur, fur)) {
    return nullptr;
  }

  corner[0] = f2[1]; corner[1] = f2[2];

  // Lower right.
  if (!lens->extrude(corner, nlr, flr)) {
    return nullptr;
  }

  corner[0] = f2[0]; corner[1] = f2[2];

  // Lower left.
  if (!lens->extrude(corner, nll, fll)) {
    return nullptr;
  }

  return new BoundingHexahedron(fll, flr, fur, ful, nll, nlr, nur, nul);
}
