// Filename: scissorEffect.cxx
// Created by:  drose (30Jul08)
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

TypeHandle ScissorEffect::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::Constructor
//       Access: Private
//  Description: Use ScissorEffect::make() to construct a new
//               ScissorEffect object.
////////////////////////////////////////////////////////////////////
ScissorEffect::
ScissorEffect(bool screen, const LVecBase4f &frame,
              const LPoint3f *points, int num_points, bool clip) :
  _screen(screen), _frame(frame), _clip(clip) 
{
  _points.reserve(num_points);
  for (int i = 0; i < num_points; ++i) {
    _points.push_back(points[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::make_screen
//       Access: Published, Static
//  Description: Constructs a new screen-relative ScissorEffect.  The
//               frame defines a left, right, bottom, top region,
//               relative to the DisplayRegion.  See ScissorAttrib.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) ScissorEffect::
make_screen(const LVecBase4f &frame, bool clip) {
  ScissorEffect *effect = new ScissorEffect(true, frame, NULL, 0, clip);
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::make_node
//       Access: Published, Static
//  Description: Constructs a new node-relative ScissorEffect.  The
//               two points are understood to be relative to the
//               current node, and determine the diagonally opposite
//               corners of the scissor region.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) ScissorEffect::
make_node(const LPoint3f &a, const LPoint3f &b, bool clip) {
  LPoint3f points[2];
  points[0] = a;
  points[1] = b;
  ScissorEffect *effect = new ScissorEffect(false, LVecBase4f::zero(), points, 2, clip);
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::make_node
//       Access: Published, Static
//  Description: Constructs a new node-relative ScissorEffect.  The
//               four points are understood to be relative to the
//               current node, and determine four points surrounding
//               the scissor region.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) ScissorEffect::
make_node(const LPoint3f &a, const LPoint3f &b, const LPoint3f &c, const LPoint3f &d, bool clip) {
  LPoint3f points[4];
  points[0] = a;
  points[1] = b;
  points[2] = c;
  points[3] = d;
  ScissorEffect *effect = new ScissorEffect(false, LVecBase4f::zero(), points, 4, clip);
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of RenderEffect by calling the
//               xform() method, false otherwise.
////////////////////////////////////////////////////////////////////
bool ScissorEffect::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ScissorEffect::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_screen()) {
    out << "screen [" << _frame << "]";
  } else {
    out << "node";
    Points::const_iterator pi;
    for (pi = _points.begin(); pi != _points.end(); ++pi) {
      out << " [" << (*pi) << "]";
    }
  }
  if (!get_clip()) {
    out << " !clip";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this effect during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool ScissorEffect::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               At the time this function is called, the current
//               node's transform and state have not yet been applied
//               to the net_transform and net_state.  This callback
//               may modify the node_transform and node_state to apply
//               an effective change to the render state at this
//               level.
////////////////////////////////////////////////////////////////////
void ScissorEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &node_state) const {
  LVecBase4f frame;
  const Lens *lens = trav->get_scene()->get_lens();
  CPT(TransformState) modelview_transform = data.get_modelview_transform(trav)->compose(node_transform);
  if (modelview_transform->is_singular()) {
    // If we're under a singular transform, never mind.
    return;
  }

  if (is_screen()) {
    frame = _frame;
  } else {
    const LMatrix4f &proj_mat = lens->get_projection_mat();
    LMatrix4f net_mat = modelview_transform->get_mat() * proj_mat;

    bool any_points = false;

    Points::const_iterator pi;
    for (pi = _points.begin(); pi != _points.end(); ++pi) {
      const LPoint3f &p = (*pi);
      LVecBase4f pv(p[0], p[1], p[2], 1.0f);
      pv = pv * net_mat;
      if (pv[3] == 0) {
        continue;
      }
      LPoint3f pr(pv[0] / pv[3], pv[1] / pv[3], pv[2] / pv[3]);
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
  frame[0] = max(min(frame[0], 1.0f), 0.0f);
  frame[1] = max(min(frame[1], 1.0f), frame[0]);
  frame[2] = max(min(frame[2], 1.0f), 0.0f);
  frame[3] = max(min(frame[3], 1.0f), frame[2]);

  if (_clip) {
    CPT(RenderAttrib) scissor_attrib = ScissorAttrib::make(frame);
    CPT(RenderState) state = RenderState::make(scissor_attrib);
    node_state = node_state->compose(state); 
  }

  // Set up the culling.  We do this by extruding the four corners of
  // the frame into the eight corners of the bounding frustum.
  PT(GeometricBoundingVolume) frustum = make_frustum(lens, frame);
  if (frustum != (GeometricBoundingVolume *)NULL) {
    frustum->xform(modelview_transform->get_inverse()->get_mat());
    data._view_frustum = frustum;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ScissorEffect
//               types to return a unique number indicating whether
//               this ScissorEffect is equivalent to the other one.
//
//               This should return 0 if the two ScissorEffect objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two ScissorEffect
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
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
      compare = _points[i].compare_to(ta->_points[i]);
      if (compare != 0) {
        return compare;
      }
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ScissorEffect.
////////////////////////////////////////////////////////////////////
void ScissorEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
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
      (*pi).write_datagram(dg);
    }
  }
  dg.add_bool(_clip);
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ScissorEffect is encountered
//               in the Bam file.  It should create the ScissorEffect
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ScissorEffect::
make_from_bam(const FactoryParams &params) {
  ScissorEffect *effect = new ScissorEffect(true, LVecBase4f::zero(), NULL, 0, false);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ScissorEffect.
////////////////////////////////////////////////////////////////////
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
      LPoint3f p;
      p.read_datagram(scan);
      _points.push_back(p);
    }
  }
  _clip = scan.get_bool();
}

////////////////////////////////////////////////////////////////////
//     Function: ScissorEffect::make_frustum
//       Access: Private
//  Description: Constructs a new bounding frustum from the lens
//               properties, given the indicated scissor frame.
////////////////////////////////////////////////////////////////////
PT(GeometricBoundingVolume) ScissorEffect::
make_frustum(const Lens *lens, const LVecBase4f &frame) const{
  // Scale the frame from 0 .. 1 into -1 .. 1.
  LVecBase4f f2(frame[0] * 2.0f - 1.0f,
                frame[1] * 2.0f - 1.0f,
                frame[2] * 2.0f - 1.0f,
                frame[3] * 2.0f - 1.0f);

  LPoint3f fll, flr, ful, fur;
  LPoint3f nll, nlr, nul, nur;
  LPoint2f corner;

  corner[0] = f2[0]; corner[1] = f2[3];

  // Upper left.
  if (!lens->extrude(corner, nul, ful)) {
    return (GeometricBoundingVolume *)NULL;
  }

  corner[0] = f2[1]; corner[1] = f2[3];

  // Upper right.
  if (!lens->extrude(corner, nur, fur)) {
    return (GeometricBoundingVolume *)NULL;
  }

  corner[0] = f2[1]; corner[1] = f2[2];

  // Lower right.
  if (!lens->extrude(corner, nlr, flr)) {
    return (GeometricBoundingVolume *)NULL;
  }

  corner[0] = f2[0]; corner[1] = f2[2];

  // Lower left.
  if (!lens->extrude(corner, nll, fll)) {
    return (GeometricBoundingVolume *)NULL;
  }

  return new BoundingHexahedron(fll, flr, fur, ful, nll, nlr, nur, nul);
}
