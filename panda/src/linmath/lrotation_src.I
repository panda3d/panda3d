// Filename: lrotation.I
// Created by:  frang, charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LRotation)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LRotation::Destructor
//       Access: public
//  Description: 
////////////////////////////////////////////////////////////////////

FLOATNAME(LRotation)::
~FLOATNAME(LRotation)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LRotation::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////

void FLOATNAME(LRotation)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    do_init_type(FLOATTYPE1);
    string name =
      "LRotation<" + get_type_handle(FLOATTYPE1).get_name() + ">";
    register_type(_type_handle, name);
  }
}

