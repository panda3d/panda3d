// Filename: boxEmitter.I
// Created by:  charles (26Jun00)
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
//    Function : set_min_bound
//      Access : Public
// Description : boundary assignment
////////////////////////////////////////////////////////////////////
INLINE void BoxEmitter::
set_min_bound(const LPoint3f& vmin) {
  _vmin = vmin;
}

////////////////////////////////////////////////////////////////////
//    Function : set_max_bound
//      Access : Public
// Description : boundary assignment
////////////////////////////////////////////////////////////////////
INLINE void BoxEmitter::
set_max_bound(const LPoint3f& vmax) {
  _vmax = vmax;
}

////////////////////////////////////////////////////////////////////
//    Function : get_min_bound
//      Access : Public
// Description : boundary accessor
////////////////////////////////////////////////////////////////////
INLINE LPoint3f BoxEmitter::
get_min_bound() const {
  return _vmin;
}

////////////////////////////////////////////////////////////////////
//    Function : get_max_bound
//      Access : Public
// Description : boundary accessor
////////////////////////////////////////////////////////////////////
INLINE LPoint3f BoxEmitter::
get_max_bound() const {
  return _vmax;
}
