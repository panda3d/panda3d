// Filename: lvecBase2_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LVecBase2)::_type_handle;

const FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::_zero =
  FLOATNAME(LVecBase2)(0.0, 0.0);
const FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::_unit_x =
  FLOATNAME(LVecBase2)(1.0, 0.0);
const FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::_unit_y =
  FLOATNAME(LVecBase2)(0.0, 1.0);

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
void FLOATNAME(LVecBase2)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    string name = "LVecBase2";
    name += FLOATTOKEN; 
    register_type(_type_handle, name);
  }
}
