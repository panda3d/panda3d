/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullPlanes.I
 * @author drose
 * @date 2005-08-23
 */

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullPlanes::
CullPlanes() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullPlanes::
CullPlanes(const CullPlanes &copy) :
  _planes(copy._planes),
  _occluders(copy._occluders)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::Copy Assignment Operator
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CullPlanes::
operator = (const CullPlanes &copy) {
  _planes = copy._planes;
  _occluders = copy._occluders;
}

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullPlanes::
~CullPlanes() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::is_empty
//       Access: Public
//  Description: Returns true if there are no planes listed in the
//               CullPlanes object.
////////////////////////////////////////////////////////////////////
INLINE bool CullPlanes::
is_empty() const {
  return _planes.empty() && _occluders.empty();

}
