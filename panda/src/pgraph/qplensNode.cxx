// Filename: qplensNode.cxx
// Created by:  drose (26Feb02)
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

#include "qplensNode.h"
#include "geometricBoundingVolume.h"

TypeHandle qpLensNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpLensNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *qpLensNode::
make_copy() const {
  return new qpLensNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpLensNode::is_in_view
//       Access: Public
//  Description: Returns true if the given point is within the bounds
//               of the lens of the qpLensNode (i.e. if the camera can
//               see the point).
////////////////////////////////////////////////////////////////////
bool qpLensNode::
is_in_view(const LPoint3f &pos) {
  PT(BoundingVolume) bv = _lens->make_bounds();
  if (bv == NULL) {
    return false;
  }
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bv);
  int ret = gbv->contains(pos);
  return (ret != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpLensNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpLensNode::
output(ostream &out) const {
  PandaNode::output(out);
  if (_lens != (Lens *)NULL) {
    out << " (";
    _lens->output(out);
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpLensNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpLensNode::
write(ostream &out, int indent_level) const {
  PandaNode::write(out, indent_level);
  if (_lens != (Lens *)NULL) {
    _lens->write(out, indent_level + 2);
  }
}

