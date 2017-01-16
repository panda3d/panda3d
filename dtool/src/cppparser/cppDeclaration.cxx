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
  _template_scope = NULL;
  _leading_comment = (CPPCommentBlock *)NULL;
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
  return _template_scope != NULL;
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
  if (error_sink != NULL) {
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
    assert((*si).second != NULL);
    return (*si).second;
  }
  return this;
}

/**
 *
 */
CPPInstance *CPPDeclaration::
as_instance() {
  return (CPPInstance *)NULL;
}

/**
 *
 */
CPPClassTemplateParameter *CPPDeclaration::
as_class_template_parameter() {
  return (CPPClassTemplateParameter *)NULL;
}

/**
 *
 */
CPPTypedefType *CPPDeclaration::
as_typedef_type() {
  return (CPPTypedefType *)NULL;
}

/**
 *
 */
CPPTypeDeclaration *CPPDeclaration::
as_type_declaration() {
  return (CPPTypeDeclaration *)NULL;
}

/**
 *
 */
CPPExpression *CPPDeclaration::
as_expression() {
  return (CPPExpression *)NULL;
}

/**
 *
 */
CPPType *CPPDeclaration::
as_type() {
  return (CPPType *)NULL;
}

/**
 *
 */
CPPNamespace *CPPDeclaration::
as_namespace() {
  return (CPPNamespace *)NULL;
}

/**
 *
 */
CPPUsing *CPPDeclaration::
as_using() {
  return (CPPUsing *)NULL;
}

/**
 *
 */
CPPSimpleType *CPPDeclaration::
as_simple_type() {
  return (CPPSimpleType *)NULL;
}

/**
 *
 */
CPPPointerType *CPPDeclaration::
as_pointer_type() {
  return (CPPPointerType *)NULL;
}

/**
 *
 */
CPPReferenceType *CPPDeclaration::
as_reference_type() {
  return (CPPReferenceType *)NULL;
}

/**
 *
 */
CPPArrayType *CPPDeclaration::
as_array_type() {
  return (CPPArrayType *)NULL;
}

/**
 *
 */
CPPConstType *CPPDeclaration::
as_const_type() {
  return (CPPConstType *)NULL;
}

/**
 *
 */
CPPFunctionType *CPPDeclaration::
as_function_type() {
  return (CPPFunctionType *)NULL;
}

/**
 *
 */
CPPFunctionGroup *CPPDeclaration::
as_function_group() {
  return (CPPFunctionGroup *)NULL;
}

/**
 *
 */
CPPExtensionType *CPPDeclaration::
as_extension_type() {
  return (CPPExtensionType *)NULL;
}

/**
 *
 */
CPPStructType *CPPDeclaration::
as_struct_type() {
  return (CPPStructType *)NULL;
}

/**
 *
 */
CPPEnumType *CPPDeclaration::
as_enum_type() {
  return (CPPEnumType *)NULL;
}

/**
 *
 */
CPPTBDType *CPPDeclaration::
as_tbd_type() {
  return (CPPTBDType *)NULL;
}

/**
 *
 */
CPPTypeProxy *CPPDeclaration::
as_type_proxy() {
  return (CPPTypeProxy *)NULL;
}

/**
 *
 */
CPPMakeProperty *CPPDeclaration::
as_make_property() {
  return (CPPMakeProperty *)NULL;
}

/**
 *
 */
CPPMakeSeq *CPPDeclaration::
as_make_seq() {
  return (CPPMakeSeq *)NULL;
}

/**
 *
 */
CPPClosureType *CPPDeclaration::
as_closure_type() {
  return (CPPClosureType *)NULL;
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
