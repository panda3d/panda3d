// Filename: mayaBlendDesc.cxx
// Created by:  drose (10Feb04)
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

#include "mayaBlendDesc.h"

TypeHandle MayaBlendDesc::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MayaBlendDesc::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaBlendDesc::
MayaBlendDesc(MFnBlendShapeDeformer deformer, int weight_index) :
  _deformer(deformer),
  _weight_index(weight_index)
{
  ostringstream strm;
  strm << _deformer.name().asChar() << "." << _weight_index;
  set_name(strm.str());

  _anim = (EggSAnimData *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaBlendDesc::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaBlendDesc::
~MayaBlendDesc() {
}

////////////////////////////////////////////////////////////////////
//     Function: MayaBlendDesc::set_slider
//       Access: Public
//  Description: Moves the Maya slider associated with this blend
//               shape to the indicated value.  This will move all the
//               affected vertices.
////////////////////////////////////////////////////////////////////
void MayaBlendDesc::
set_slider(float value) {
  _deformer.setWeight(_weight_index, value);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaBlendDesc::get_slider
//       Access: Public
//  Description: Returns the current position of the Maya slider
//               associated with this blend shape.
////////////////////////////////////////////////////////////////////
float MayaBlendDesc::
get_slider() const {
  return _deformer.weight(_weight_index);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaBlendDesc::clear_egg
//       Access: Private
//  Description: Clears the egg pointers from this blend desc.
////////////////////////////////////////////////////////////////////
void MayaBlendDesc::
clear_egg() {
  _anim = (EggSAnimData *)NULL;
}
