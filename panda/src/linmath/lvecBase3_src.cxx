// Filename: lvecBase3_src.h
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LVecBase3)::_type_handle;

const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_zero =
  FLOATNAME(LVecBase3)(0.0, 0.0, 0.0);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_x =
  FLOATNAME(LVecBase3)(1.0, 0.0, 0.0);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_y =
  FLOATNAME(LVecBase3)(0.0, 1.0, 0.0);
const FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::_unit_z =
  FLOATNAME(LVecBase3)(0.0, 0.0, 1.0);

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LVecBase3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    string name = "LVecBase3";
    name += FLOATTOKEN; 
    register_type(_type_handle, name);
  }
}
