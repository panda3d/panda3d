// Filename: cppDeclaration.cxx
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


#include "cppDeclaration.h"
#include "cppPreprocessor.h"

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::
CPPDeclaration(const CPPFile &file) :
  _file(file)
{
  _vis = V_unknown;
  _template_scope = NULL;
  _leading_comment = (CPPCommentBlock *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::
CPPDeclaration(const CPPDeclaration &copy) :
  _vis(copy._vis),
  _template_scope(copy._template_scope),
  _file(copy._file),
  _leading_comment(copy._leading_comment)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration::
~CPPDeclaration() {
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::Equivalence Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPDeclaration::
operator == (const CPPDeclaration &other) const {
  if (get_subtype() != other.get_subtype()) {
    return false;
  }
  return is_equal(&other);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::Nonequivalence Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPDeclaration::
operator != (const CPPDeclaration &other) const {
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPDeclaration::
operator < (const CPPDeclaration &other) const {
  if (get_subtype() != other.get_subtype()) {
    return get_subtype() < other.get_subtype();
  }
  return is_less(&other);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::is_template
//       Access: Public
//  Description: Returns true if this is a template declaration of
//               some kind: a template function or a template class,
//               typically.
////////////////////////////////////////////////////////////////////
bool CPPDeclaration::
is_template() const {
  return _template_scope != NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::get_template_scope
//       Access: Public
//  Description: If is_template(), above, returns true, this returns
//               the CPPTemplateScope in which this particular
//               template declaration is defined.  This scope includes
//               the information about the template parameters.
////////////////////////////////////////////////////////////////////
CPPTemplateScope *CPPDeclaration::
get_template_scope() const {
  return _template_scope;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::is_fully_specified
//       Access: Public, Virtual
//  Description: Returns true if this declaration is an actual,
//               factual declaration, or false if some part of the
//               declaration depends on a template parameter which has
//               not yet been instantiated.
////////////////////////////////////////////////////////////////////
bool CPPDeclaration::
is_fully_specified() const {
  return !is_template();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::instantiate
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPDeclaration::
instantiate(const CPPTemplateParameterList *,
            CPPScope *, CPPScope *,
            CPPPreprocessor *error_sink) const {
  if (error_sink != NULL) {
    error_sink->warning("Ignoring template parameters");
  }
  return (CPPDeclaration *)this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::substitute_decl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPDeclaration *CPPDeclaration::
substitute_decl(SubstDecl &subst, CPPScope *, CPPScope *) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    return (*si).second;
  }
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_instance
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPInstance *CPPDeclaration::
as_instance() {
  return (CPPInstance *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_class_template_parameter
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPClassTemplateParameter *CPPDeclaration::
as_class_template_parameter() {
  return (CPPClassTemplateParameter *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_typedef_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypedefType *CPPDeclaration::
as_typedef_type() {
  return (CPPTypedefType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_type_declaration
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypeDeclaration *CPPDeclaration::
as_type_declaration() {
  return (CPPTypeDeclaration *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_expression
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPExpression *CPPDeclaration::
as_expression() {
  return (CPPExpression *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPType *CPPDeclaration::
as_type() {
  return (CPPType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_namespace
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPNamespace *CPPDeclaration::
as_namespace() {
  return (CPPNamespace *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_using
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPUsing *CPPDeclaration::
as_using() {
  return (CPPUsing *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_simple_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPSimpleType *CPPDeclaration::
as_simple_type() {
  return (CPPSimpleType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_pointer_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPPointerType *CPPDeclaration::
as_pointer_type() {
  return (CPPPointerType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_reference_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPReferenceType *CPPDeclaration::
as_reference_type() {
  return (CPPReferenceType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_array_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPArrayType *CPPDeclaration::
as_array_type() {
  return (CPPArrayType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_const_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPConstType *CPPDeclaration::
as_const_type() {
  return (CPPConstType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_function_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionType *CPPDeclaration::
as_function_type() {
  return (CPPFunctionType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_function_group
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPFunctionGroup *CPPDeclaration::
as_function_group() {
  return (CPPFunctionGroup *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_extension_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPExtensionType *CPPDeclaration::
as_extension_type() {
  return (CPPExtensionType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_struct_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPStructType *CPPDeclaration::
as_struct_type() {
  return (CPPStructType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_enum_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPEnumType *CPPDeclaration::
as_enum_type() {
  return (CPPEnumType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_tbd_type
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTBDType *CPPDeclaration::
as_tbd_type() {
  return (CPPTBDType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_type_proxy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPTypeProxy *CPPDeclaration::
as_type_proxy() {
  return (CPPTypeProxy *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_make_property
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPMakeProperty *CPPDeclaration::
as_make_property() {
  return (CPPMakeProperty *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::as_make_seq
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPPMakeSeq *CPPDeclaration::
as_make_seq() {
  return (CPPMakeSeq *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::is_equal
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration to determine whether this
//               type is equivalent to another type of the same type.
////////////////////////////////////////////////////////////////////
bool CPPDeclaration::
is_equal(const CPPDeclaration *other) const {
  return this == other;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPDeclaration::is_less
//       Access: Protected, Virtual
//  Description: Called by CPPDeclaration to determine whether this
//               type should be ordered before another type of the
//               same type, in an arbitrary but fixed ordering.
////////////////////////////////////////////////////////////////////
bool CPPDeclaration::
is_less(const CPPDeclaration *other) const {
  return this < other;
}


ostream &
operator << (ostream &out, const CPPDeclaration::SubstDecl &subst) {
  CPPDeclaration::SubstDecl::const_iterator it;
  for (it = subst.begin(); it != subst.end(); ++it) {
    out << "  ";
    if (it->first == NULL) {
      out << "(null)";
    } else {
      out << *(it->first);
    }
    out << " -> ";
    if (it->second == NULL) {
      out << "(null)";
    } else {
      out << *(it->second);
    }
    out << "\n";
  }
  return out;
}
