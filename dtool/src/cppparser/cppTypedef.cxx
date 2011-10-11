// Filename: cppTypedef.cxx
// Created by:  drose (19Oct99)
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


#include "cppTypedef.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedef::Constructor
//       Access: Public
//  Description: Constructs a new CPPTypedef object based on the
//               indicated CPPInstance object.  The CPPInstance is
//               deallocated.
//
//               If global is true, the typedef is defined at the
//               global scope, and hence it's worth telling the type
//               itself about.  Otherwise, it's just a locally-scoped
//               typedef.
////////////////////////////////////////////////////////////////////
CPPTypedef::
CPPTypedef(CPPInstance *inst, bool global) : CPPInstance(*inst)
{
  // Actually, we'll avoid deleting this for now.  It causes problems
  // for some reason to be determined later.
  //  delete inst;

  assert(_type != NULL);
  if (global) {
    _type->_typedefs.push_back(this);
    CPPType::record_alt_name_for(_type, inst->get_local_name());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedef::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPTypedef::
substitute_decl(CPPDeclaration::SubstDecl &subst,
                CPPScope *current_scope, CPPScope *global_scope) {
  CPPDeclaration *decl =
    CPPInstance::substitute_decl(subst, current_scope, global_scope);
  assert(decl != NULL);
  if (decl->as_typedef()) {
    return decl;
  }
  assert(decl->as_instance() != NULL);
  return new CPPTypedef(new CPPInstance(*decl->as_instance()), false);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedef::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPTypedef::
output(ostream &out, int indent_level, CPPScope *scope, bool) const {
  out << "typedef ";
  CPPInstance::output(out, indent_level, scope, false);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedef::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPTypedef::
get_subtype() const {
  return ST_typedef;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPTypedef::as_typedef
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypedef *CPPTypedef::
as_typedef() {
  return this;
}
