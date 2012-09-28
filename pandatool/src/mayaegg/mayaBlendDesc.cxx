// Filename: mayaBlendDesc.cxx
// Created by:  drose (10Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "mayaBlendDesc.h"
#include "config_mayaegg.h"

TypeHandle MayaBlendDesc::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MayaBlendDesc::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaBlendDesc::
MayaBlendDesc(MFnBlendShapeDeformer &deformer, int weight_index) :
  _deformer(deformer.object()),
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
set_slider(PN_stdfloat value) {
  MStatus status = _deformer.setWeight(_weight_index, value);
  if (!status) {
    mayaegg_cat.warning()
      << "Unable to set slider " << get_name() << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaBlendDesc::get_slider
//       Access: Public
//  Description: Returns the current position of the Maya slider
//               associated with this blend shape.
////////////////////////////////////////////////////////////////////
PN_stdfloat MayaBlendDesc::
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
