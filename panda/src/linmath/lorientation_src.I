// Filename: lorientation.I
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LOrientation)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LOrientation)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    do_init_type(FLOATTYPE1);
    string name =
      "LOrientation<" + get_type_handle(FLOATTYPE1).get_name() + ">";
    register_type(_type_handle, name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LOrientation::Destructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

FLOATNAME(LOrientation)::
~FLOATNAME(LOrientation)() {
}
