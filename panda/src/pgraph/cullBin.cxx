// Filename: cullBin.cxx
// Created by:  drose (28Feb02)
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

#include "cullBin.h"


#ifndef CPPPARSER
PStatCollector CullBin::_cull_bin_pcollector("Cull:Bins");
PStatCollector CullBin::_draw_bin_pcollector("Draw:Bins");
#endif

TypeHandle CullBin::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBin::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullBin::
~CullBin() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::make_next
//       Access: Public, Virtual
//  Description: Returns a newly-allocated CullBin object that
//               contains a copy of just the subset of the data from
//               this CullBin object that is worth keeping around
//               for next frame.
//
//               If a particular CullBin object has no data worth
//               preserving till next frame, it is acceptable to
//               return NULL (which is the default behavior of this
//               method).
////////////////////////////////////////////////////////////////////
PT(CullBin) CullBin::
make_next() const {
  return (CullBin *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::add_object
//       Access: Public, Virtual
//  Description: Adds a geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBin::
add_object(CullableObject *) {
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::finish_cull
//       Access: Public
//  Description: Called after all the geoms have been added, this
//               indicates that the cull process is finished for this
//               frame and gives the bins a chance to do any
//               post-processing (like sorting) before moving on to
//               draw.
////////////////////////////////////////////////////////////////////
void CullBin::
finish_cull() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::draw
//       Access: Public
//  Description: Draws all the geoms in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBin::
draw() {
}

