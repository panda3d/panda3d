// Filename: lpoint3_src.h
// Created by:  drose (25Sep99)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LPoint3)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LPoint3::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LPoint3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    FLOATNAME(LVecBase3)::init_type();
    string name = "LPoint3";
    name += FLOATTOKEN; 
    register_type(_type_handle, name, 
		  FLOATNAME(LVecBase3)::get_class_type());
  }
}

