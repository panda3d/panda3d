/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppDeclaration.cxx
 * @author drose
 * @date 1999-10-19
 */

#include "cppDeclaration.h"
#include "cppPreprocessor.h"

/**
 *
 */
CPPDeclaration::
CPPDeclaration(const CPPFile &file) :
  _file(file)
{
  _vis = V_unknown;
  _template_scope = nullptr;
  _leading_comment = (CPPCommentBlock *)nullptr;
}

/**
 *
 */
CPPDeclaration::
CPPDeclaration(const CPPDeclaration &copy) :
  _vis(copy._vis),
  _template_scope(copy._template_scope),
  _file(copy._file),
  _leading_comment(copy._leading_comment)
{
}

/**
 *
 */
CPPDeclaration &CPPDeclaration::
operator = (const CPPDeclaration &copy) {
  _vis = copy._vis;
  _template_scope = copy._template_scope;
  _file = copy._file;
  _leading_comment = copy._leading_comment;
  return *this;
}

/**
 *
 */
bool CPPDeclaration::
operator == (const CPPDeclaration &other) const {
  if (get_subtype() != other.get_subtype()) {
    return false;
  }
  return is_equal(&other);
}

/**
 *
 */
bool CPPDeclaration::
operator != (const CPPDeclaration &other) const {
  return !(*this == other);
}

/**
 *
 */
bool CPPDeclaration::
operator < (const CPPDeclaration &other) const {
  if (get_subtype() != other.get_subtype()) {
    return get_subtype() < other.get_subtype();
  }
  return is_less(&other);
}

/**
 * Returns true if this is a template declaration of some kind: a template
 * function or a template class, typically.
 */
bool CPPDeclaration::
is_template() const {
  return _template_scope != nullptr;
}

/**
 * If is_template(), above, returns true, this returns the CPPTemplateScope in
 * which this particular template declaration is defined.  This scope includes
 * the information about the template parameters.
 */
CPPTemplateScope *CPPDeclaration::
get_template_scope() const {
  return _template_scope;
}

/**
 * Returns true if this declaration is an actual, factual declaration, or
 * false if some part of the declaration depends on a template parameter which
 * has not yet been instantiated.
 */
bool CPPDeclaration::
is_fully_specified() const {
  return !is_template();
}

/**
 *
 */
CPPDeclaration *CPPDeclaration::
instantiate(const CPPTemplateParameterList *,
            CPPScope *, CPPScope *,
            CPPPreprocessor *error_sink) const {
  if (error_sink != nullptr) {
    error_sink->warning("Ignoring template parameters");
  }
  return (CPPDeclaration *)this;
}

/**
 *
 */
CPPDeclaration *CPPDeclaration::
substitute_decl(SubstDecl &subst, CPPScope *, CPPScope *) {
  SubstDecl::const_iterator si = subst.find(this);
  if (si != subst.end()) {
    assert((*si).second != nullptr);
    return (*si).second;
  }
  return this;
}

/**
 *
 */
CPPInstance *CPPDeclaration::
as_instance() {
  return (CPPInstance *)nullptr;
}

/**
 *
 */
CPPClassTemplateParameter *CPPDeclaration::
as_class_template_parameter() {
  return (CPPClassTemplateParameter *)nullptr;
}

/**
 *
 */
CPPTypedefType *CPPDeclaration::
as_typedef_type() {
  return (CPPTypedefType *)nullptr;
}

/**
 *
 */
CPPTypeDeclaration *CPPDeclaration::
as_type_declaration() {
  return (CPPTypeDeclaration *)nullptr;
}

/**
 *
 */
CPPExpression *CPPDeclaration::
as_expression() {
  return (CPPExpression *)nullptr;
}

/**
 *
 */
CPPType *CPPDeclaration::
as_type() {
  return (CPPType *)nullptr;
}

/**
 *
 */
CPPNamespace *CPPDeclaration::
as_namespace() {
  return (CPPNamespace *)nullptr;
}

/**
 *
 */
CPPUsing *CPPDeclaration::
as_using() {
  return (CPPUsing *)nullptr;
}

/**
 *
 */
CPPSimpleType *CPPDeclaration::
as_simple_type() {
  return (CPPSimpleType *)nullptr;
}

/**
 *
 */
CPPPointerType *CPPDeclaration::
as_pointer_type() {
  return (CPPPointerType *)nullptr;
}

/**
 *
 */
CPPReferenceType *CPPDeclaration::
as_reference_type() {
  return (CPPReferenceType *)nullptr;
}

/**
 *
 */
CPPArrayType *CPPDeclaration::
as_array_type() {
  return (CPPArrayType *)nullptr;
}

/**
 *
 */
CPPConstType *CPPDeclaration::
as_const_type() {
  return (CPPConstType *)nullptr;
}

/**
 *
 */
CPPFunctionType *CPPDeclaration::
as_function_type() {
  return (CPPFunctionType *)nullptr;
}

/**
 *
 */
CPPFunctionGroup *CPPDeclaration::
as_function_group() {
  return (CPPFunctionGroup *)nullptr;
}

/**
 *
 */
CPPExtensionType *CPPDeclaration::
as_extension_type() {
  return (CPPExtensionType *)nullptr;
}

/**
 *
 */
CPPStructType *CPPDeclaration::
as_struct_type() {
  return (CPPStructType *)nullptr;
}

/**
 *
 */
CPPEnumType *CPPDeclaration::
as_enum_type() {
  return (CPPEnumType *)nullptr;
}

/**
 *
 */
CPPTBDType *CPPDeclaration::
as_tbd_type() {
  return (CPPTBDType *)nullptr;
}

/**
 *
 */
CPPTypeProxy *CPPDeclaration::
as_type_proxy() {
  return (CPPTypeProxy *)nullptr;
}

/**
 *
 */
CPPMakeProperty *CPPDeclaration::
as_make_property() {
  return (CPPMakeProperty *)nullptr;
}

/**
 *
 */
CPPMakeSeq *CPPDeclaration::
as_make_seq() {
  return (CPPMakeSeq *)nullptr;
}

/**
 *
 */
CPPClosureType *CPPDeclaration::
as_closure_type() {
  return (CPPClosureType *)nullptr;
}

/**
 * Called by CPPDeclaration to determine whether this type is equivalent to
 * another type of the same type.
 */
bool CPPDeclaration::
is_equal(const CPPDeclaration *other) const {
  return this == other;
}

/**
 * Called by CPPDeclaration to determine whether this type should be ordered
 * before another type of the same type, in an arbitrary but fixed ordering.
 */
bool CPPDeclaration::
is_less(const CPPDeclaration *other) const {
  return this < other;
}


ostream &
operator << (ostream &out, const CPPDeclaration::SubstDecl &subst) {
  CPPDeclaration::SubstDecl::const_iterator it;
  for (it = subst.begin(); it != subst.end(); ++it) {
    out << "  ";
    if (it->first == nullptr) {
      out << "(null)";
    } else {
      out << *(it->first);
    }
    out << " -> ";
    if (it->second == nullptr) {
      out << "(null)";
    } else {
      out << *(it->second);
    }
    out << "\n";
  }
  return out;
}
