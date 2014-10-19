// Filename: geomDrawCallbackData.cxx
// Created by:  drose (13Mar09)
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

#include "geomDrawCallbackData.h"
#include "cullableObject.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomDrawCallbackData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomDrawCallbackData::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomDrawCallbackData::
output(ostream &out) const {
  out << get_type() << "(" << (void *)_obj << ", " << (void *)_gsg 
      << ", " << _force << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomDrawCallbackData::upcall
//       Access: Published, Virtual
//  Description: You should make this call during the callback if you
//               want to continue the normal rendering function that
//               would have been done in the absence of a callback.
//
//               Specifically, this method will add the Geoms in this
//               node to the list of renderable objects for drawing.
//               If this callback was made on a CallbackNode, it
//               doesn't actually do anything, since only a GeomNode
//               holds geoms.
////////////////////////////////////////////////////////////////////
void GeomDrawCallbackData::
upcall() {
  // Go ahead and draw the object, if we have one.
  if (_obj->_geom != (Geom *)NULL) {
    if (_lost_state) {
      // Tell the GSG to forget its state.
      _gsg->clear_state_and_transform();
    }

    _obj->_geom->draw(_gsg, _obj->_munger, _obj->_munged_data, _force, 
                      Thread::get_current_thread());
  }
}
