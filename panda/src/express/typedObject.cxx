// Filename: typedObject.cxx
// Created by:  drose (11May01)
// 
////////////////////////////////////////////////////////////////////

#include "typedObject.h"
#include "config_express.h"


TypeHandle TypedObject::_type_handle;
 
////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TypedObject::
~TypedObject() { 
}

 
////////////////////////////////////////////////////////////////////
//     Function: TypedObject::get_type
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TypeHandle TypedObject::
get_type() const {
  // Normally, this function should never be called, because it is a
  // pure virtual function.  If it is called, you probably called
  // get_type() on a recently-destructed object.
  express_cat.warning()
    << "TypedObject::get_type() called!\n";
  return _type_handle;
}
