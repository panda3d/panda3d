// Filename: lvecBase2.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LVecBase2)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LVecBase2)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    do_init_type(FLOATTYPE1);
    string name =
      "LVecBase2<" + get_type_handle(FLOATTYPE1).get_name() + ">";
    register_type(_type_handle, name);
  }
}
