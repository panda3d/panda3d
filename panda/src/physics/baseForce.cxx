// Filename: baseForce.cxx
// Created by:  charles (08Aug00)
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

#include "baseForce.h"
#include "indent.h"

TypeHandle BaseForce::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : BaseForce
//      Access : protected
// Description : constructor
////////////////////////////////////////////////////////////////////
BaseForce::
BaseForce(bool active) :
  _force_node(NULL), 
  _active(active) 
{
}

////////////////////////////////////////////////////////////////////
//    Function : BaseForce
//      Access : protected
// Description : copy constructor
////////////////////////////////////////////////////////////////////
BaseForce::
BaseForce(const BaseForce &copy) :
  TypedReferenceCount(copy) 
{
  _active = copy._active;
  _force_node = (ForceNode *) NULL;
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseForce
//      Access : public, virtual
// Description : destructor
////////////////////////////////////////////////////////////////////
BaseForce::
~BaseForce() {
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseForce::
output(ostream &out) const {
  out << "BaseForce (id " << this << ")";
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void BaseForce::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "BaseForce (id " << this << "):\n";
  
  indent(out, indent_level + 2)
    << "_force_node ";
  if (_force_node) {
    out << _force_node_path << "\n";
  } else {
    out << "null\n";
  }
  
  indent(out, indent_level + 2)
    << "_active " << _active << "\n";
}
