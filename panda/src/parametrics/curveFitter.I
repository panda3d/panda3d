// Filename: curveFitter.I
// Created by:  drose (04Mar01)
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


////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::DataPoint::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CurveFitter::DataPoint::
DataPoint() :
  _t(0.0f),
  _xyz(0.0f, 0.0f, 0.0f),
  _hpr(0.0f, 0.0f, 0.0f),
  _tangent(0.0f, 0.0f, 0.0f),
  _hpr_tangent(0.0f, 0.0f, 0.0f)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::DataPoint::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CurveFitter::DataPoint::
output(ostream &out) const {
  out << "Time " << _t << " xyz " << _xyz << " hpr " << _hpr
      << " tan " << _tangent;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::DataPoint::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CurveFitter::DataPoint::
operator < (const DataPoint &other) const {
  return _t < other._t;
}

