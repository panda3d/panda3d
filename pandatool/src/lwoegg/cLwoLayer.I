// Filename: cLwoLayer.I
// Created by:  drose (25Apr01)
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
//     Function: CLwoLayer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLwoLayer::
CLwoLayer(LwoToEggConverter *converter, const LwoLayer *layer) :
  _converter(converter),
  _layer(layer)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoLayer::get_number
//       Access: Public
//  Description: Returns the index number associated with this
//               particular layer.  This should be unique among all
//               Lightwave layers in a single file.
////////////////////////////////////////////////////////////////////
INLINE int CLwoLayer::
get_number() const {
  return _layer->_number;
}
