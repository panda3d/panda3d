// Filename: fog.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "fog.h"

#include "mathNumbers.h"
#include "renderRelation.h"
#include "transformTransition.h"
#include "wrt.h"

#include <stddef.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Fog::_type_handle;

ostream &
operator << (ostream &out, Fog::Mode mode) {
  switch (mode) {
  case Fog::M_linear:
    return out << "linear";

  case Fog::M_exponential:
    return out << "exponential";

  case Fog::M_exponential_squared:
    return out << "exponential-squared";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Fog::
Fog(const string &name) : NamedNode(name) {
  _mode = M_linear;
  _bits_per_color_channel = 8;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _linear_onset_point.set(0.0f, 0.0f, 0.0f);
  _linear_opaque_point.set(0.0, 100.0f, 0.0f);
  _exp_density = 0.5f;
  _linear_fallback_cosa = -1.0f;
  _linear_fallback_onset = 0.0f;
  _linear_fallback_opaque = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Fog::
Fog(const Fog &copy) :
  NamedNode(copy)
{
  _mode = copy._mode;
  _bits_per_color_channel = copy._bits_per_color_channel;
  _color = copy._color;
  _linear_onset_point = copy._linear_onset_point;
  _linear_opaque_point = copy._linear_opaque_point;
  _exp_density = copy._exp_density;
  _linear_fallback_cosa = copy._linear_fallback_cosa;
  _linear_fallback_onset = copy._linear_fallback_onset;
  _linear_fallback_opaque = copy._linear_fallback_opaque;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Fog::
operator = (const Fog &copy) {
  NamedNode::operator = (copy);
  _mode = copy._mode;
  _bits_per_color_channel = copy._bits_per_color_channel;
  _color = copy._color;
  _linear_onset_point = copy._linear_onset_point;
  _linear_opaque_point = copy._linear_opaque_point;
  _exp_density = copy._exp_density;
  _linear_fallback_cosa = copy._linear_fallback_cosa;
  _linear_fallback_onset = copy._linear_fallback_onset;
  _linear_fallback_opaque = copy._linear_fallback_opaque;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Fog::
~Fog() {
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *Fog::
make_copy() const {
  return new Fog(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
////////////////////////////////////////////////////////////////////
void Fog::
xform(const LMatrix4f &mat) {
  _linear_onset_point = _linear_onset_point * mat;
  _linear_opaque_point = _linear_opaque_point * mat;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Fog::
output(ostream &out) const {
  out << "fog: " << _mode;
  switch (_mode) {
  case M_linear:
    out << "(" << _linear_onset_point << ") -> ("
        << _linear_opaque_point << ")";
    break;

  case M_exponential:
  case M_exponential_squared:
    out << "(" << _bits_per_color_channel << ", " << _exp_density 
        << ")";
    break;
  };
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::compute_linear_range
//       Access: Public
//  Description: This function is intended to be called by GSG's to
//               compute the appropriate camera-relative onset and
//               opaque distances, based on the fog node's position
//               within the scene graph (if linear fog is in effect).
////////////////////////////////////////////////////////////////////
void Fog::
compute_linear_range(float &onset, float &opaque,
                     Node *camera_node, CoordinateSystem cs) {
  LVector3f forward = LVector3f::forward(cs);

  LPoint3f onset_point, opaque_point;
  if (get_num_parents(RenderRelation::get_class_type()) != 0) {
    // Linear fog is relative to the fog's net transform in the scene
    // graph.
    LMatrix4f mat;

    NodeTransitionWrapper ntw(TransformTransition::get_class_type());
    wrt(this, camera_node, ntw, RenderRelation::get_class_type());
    const TransformTransition *tt;
    if (!get_transition_into(tt, ntw)) {
      // No relative transform.
      mat = LMatrix4f::ident_mat();
    } else {
      mat = tt->get_matrix();
    }

    // How far out of whack are we?
    LVector3f fog_vector = (_linear_opaque_point - _linear_onset_point) * mat;
    fog_vector.normalize();
    float cosa = fog_vector.dot(forward);
    if (cabs(cosa) < _linear_fallback_cosa) {
      // The fog vector is too far from the eye vector; use the
      // fallback mode.
      onset = _linear_fallback_onset;
      opaque = _linear_fallback_opaque;
      //cerr << "fallback! " << cosa << " vs. " << _linear_fallback_cosa << "\n";
      return;
    }

    onset_point = _linear_onset_point * mat;
    opaque_point = _linear_opaque_point * mat;

  } else {
    // If the fog object has no parents, we assume the user meant
    // camera-relative fog.
    onset_point = _linear_onset_point;
    opaque_point = _linear_opaque_point;
  }
  
  onset = onset_point.dot(forward);
  opaque = opaque_point.dot(forward);
}

#if 0
// this fn tries to 'match' exponential to linear fog by computing an
// exponential density factor such that exponential fog matches linear
// fog at the linear fog's 'fog-end' distance.  usefulness of this is
// debatable since the exponential fog that matches will be very very
// close to observer, and it's harder to guess a good end fog distance
// than it is to manipulate the [0.0,1.0] density range directly, so
// taking this out

////////////////////////////////////////////////////////////////////
//     Function: Fog::compute_density
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void Fog::compute_density(void) {
  _density = 1.0f;
  float opaque_multiplier;
  switch (_mode) {

  case M_linear:
    break;

  case M_exponential:
    // Multiplier = ln(2^bits)
        // attempt to compute density based on full
    opaque_multiplier = MathNumbers::ln2 * _bits_per_color_channel;
    _density = opaque_multiplier / _opaque_distance;
    break;

  case M_exponential_squared:
    // Multiplier = ln(sqrt(2^bits))
    opaque_multiplier = 0.5f * MathNumbers::ln2 * _bits_per_color_channel;
    opaque_multiplier *= opaque_multiplier;
    _density = opaque_multiplier / _opaque_distance;
    break;
  }
}

#endif
