/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfLayer.cxx
 * @author drose
 * @date 2004-05-04
 */

#include "dxfLayer.h"


////////////////////////////////////////////////////////////////////
//     Function: DXFLayer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXFLayer::
DXFLayer(const string &name) : Namable(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: DXFLayer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DXFLayer::
~DXFLayer() {
}
