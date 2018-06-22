/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaBlendDesc.cxx
 * @author drose
 * @date 2004-02-10
 */

#include "mayaBlendDesc.h"
#include "config_mayaegg.h"

TypeHandle MayaBlendDesc::_type_handle;

/**
 *
 */
MayaBlendDesc::
MayaBlendDesc(MFnBlendShapeDeformer &deformer, int weight_index) :
  _deformer(deformer.object()),
  _weight_index(weight_index)
{
  std::ostringstream strm;
  strm << _deformer.name().asChar() << "." << _weight_index;
  set_name(strm.str());

  _anim = nullptr;
}

/**
 *
 */
MayaBlendDesc::
~MayaBlendDesc() {
}

/**
 * Moves the Maya slider associated with this blend shape to the indicated
 * value.  This will move all the affected vertices.
 */
void MayaBlendDesc::
set_slider(PN_stdfloat value) {
  MStatus status = _deformer.setWeight(_weight_index, value);
  if (!status) {
    mayaegg_cat.warning()
      << "Unable to set slider " << get_name() << "\n";
  }
}

/**
 * Returns the current position of the Maya slider associated with this blend
 * shape.
 */
PN_stdfloat MayaBlendDesc::
get_slider() const {
  return _deformer.weight(_weight_index);
}

/**
 * Clears the egg pointers from this blend desc.
 */
void MayaBlendDesc::
clear_egg() {
  _anim = nullptr;
}
