// Filename: ropeNode.I
// Created by:  drose (04Dec02)
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


////////////////////////////////////////////////////////////////////
//     Function: RopeNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RopeNode::CData::
CData() {
  _curve = new NurbsCurveEvaluator;
  _num_segs = 10;
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RopeNode::CData::
CData(const RopeNode::CData &copy) :
  _curve(copy._curve)
{
}

////////////////////////////////////////////////////////////////////
//     Function: set_curve
//       Access: Public
//  Description: Sets the particular curve represented by the
//               RopeNode.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_curve(NurbsCurveEvaluator *curve) {
  CDWriter cdata(_cycler);
  cdata->_curve = curve;
}

////////////////////////////////////////////////////////////////////
//     Function: get_curve
//       Access: Public
//  Description: Returns the curve represented by the RopeNode.
////////////////////////////////////////////////////////////////////
INLINE NurbsCurveEvaluator *RopeNode::
get_curve() const {
  CDReader cdata(_cycler);
  return cdata->_curve;
}

////////////////////////////////////////////////////////////////////
//     Function: set_num_segs
//       Access: Public
//  Description: Specifies the number of segments per cubic segment
//               (that is, per unique knot value) to draw in a fixed
//               uniform tesselation of the curve.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_num_segs(int num_segs) {
  CDWriter cdata(_cycler);
  cdata->_num_segs = num_segs;
}

////////////////////////////////////////////////////////////////////
//     Function: get_num_segs
//       Access: Public
//  Description: Returns the number of segments per cubic segment to
//               draw.  See set_num_segs().
////////////////////////////////////////////////////////////////////
INLINE int RopeNode::
get_num_segs() const {
  CDReader cdata(_cycler);
  return cdata->_num_segs;
}
