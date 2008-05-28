// Filename: cppClassTemplateParameter.cxx
// Created by:  drose (28Oct99)
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


#include "cppClassTemplateParameter.h"
#include "cppIdentifier.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPClassTemplateParameter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPClassTemplateParameter::
CPPClassTemplateParameter(CPPIdentifier *ident, CPPType *default_type) :
  CPPType(CPPFile()),
  _ident(ident),
  _default_type(default_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPClassTemplateParameter::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPClassTemplateParameter::
is_fully_specified() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPClassTemplateParameter::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CPPClassTemplateParameter::
output(ostream &out, int indent_level, CPPScope *scope, bool complete) const {
  if (complete) {
    out << "class ";
    _ident->output(out, scope);
    if (_default_type) {
      out << " = ";
      _default_type->output(out, indent_level, scope, false);
    }
  } else {
    _ident->output(out, scope);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CPPClassTemplateParameter::get_subtype
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::SubType CPPClassTemplateParameter::
get_subtype() const {
  return ST_class_template_parameter;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPClassTemplateParameter::as_classTemplateParameter
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPClassTemplateParameter *CPPClassTemplateParameter::
as_class_template_parameter() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPClassTemplateParameter::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type is
//               equivalent to another type of the same type.
////////////////////////////////////////////////////////////////////
bool CPPClassTemplateParameter::
is_equal(const CPPDeclaration *other) const {
  const CPPClassTemplateParameter *ot = ((CPPDeclaration *)other)->as_class_template_parameter();
  assert(ot != NULL);

  if (_default_type != ot->_default_type) {
    return false;
  }

  return *_ident == *ot->_ident;
}


////////////////////////////////////////////////////////////////////
//     Function: CPPClassTemplateParameter::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration() to determine whether this type
//               should be ordered before another type of the same
//               type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
bool CPPClassTemplateParameter::
is_less(const CPPDeclaration *other) const {
  const CPPClassTemplateParameter *ot = ((CPPDeclaration *)other)->as_class_template_parameter();
  assert(ot != NULL);

  if (_default_type != ot->_default_type) {
    return _default_type < ot->_default_type;
  }

  return *_ident < *ot->_ident;
}
