// Filename: lpoint3.I
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
    string name =
      "LPoint3<" + get_type_handle(FLOATTYPE1).get_name() + ">";
    register_type(_type_handle, name, 
		  FLOATNAME(LVecBase3)::get_class_type());
  }
}

