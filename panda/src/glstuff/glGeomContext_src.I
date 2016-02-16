/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glGeomContext_src.I
 * @author drose
 * @date 2004-03-19
 */

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomContext)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLP(GeomContext)::
CLP(GeomContext)(Geom *geom) :
  GeomContext(geom)
{
#ifdef DO_PSTATS
  _num_verts = 0;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomContext)::DisplayList::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLP(GeomContext)::DisplayList::
DisplayList() :
  _index(0)
{
}
