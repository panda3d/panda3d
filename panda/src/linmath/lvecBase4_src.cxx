// Filename: lvecBase4_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LVecBase4)::_type_handle;

const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_zero =
  FLOATNAME(LVecBase4)(0.0, 0.0, 0.0, 0.0);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_x =
  FLOATNAME(LVecBase4)(1.0, 0.0, 0.0, 0.0);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_y =
  FLOATNAME(LVecBase4)(0.0, 1.0, 0.0, 0.0);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_z =
  FLOATNAME(LVecBase4)(0.0, 0.0, 1.0, 0.0);
const FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::_unit_w =
  FLOATNAME(LVecBase4)(0.0, 0.0, 0.0, 1.0);

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LVecBase4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    string name = "LVecBase4";
    name += FLOATTOKEN; 
    register_type(_type_handle, name);
  }
}
