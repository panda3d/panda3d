// Filename: cppDeclaration.h
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

#ifndef CPPDECLARATION_H
#define CPPDECLARATION_H

#include "dtoolbase.h"

#include "cppVisibility.h"
#include "cppFile.h"
#include "cppCommentBlock.h"

#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

class CPPInstance;
class CPPTemplateParameterList;
class CPPTypedefType;
class CPPTypeDeclaration;
class CPPExpression;
class CPPType;
class CPPNamespace;
class CPPUsing;
class CPPSimpleType;
class CPPPointerType;
class CPPReferenceType;
class CPPArrayType;
class CPPConstType;
class CPPFunctionType;
class CPPFunctionGroup;
class CPPExtensionType;
class CPPStructType;
class CPPEnumType;
class CPPTypeProxy;
class CPPMakeProperty;
class CPPMakeSeq;
class CPPClassTemplateParameter;
class CPPTBDType;
class CPPScope;
class CPPTemplateScope;
class CPPPreprocessor;

///////////////////////////////////////////////////////////////////
//       Class : CPPDeclaration
// Description :
////////////////////////////////////////////////////////////////////
class CPPDeclaration {
public:
  enum SubType {
    // Subtypes of CPPDeclaration
    ST_instance,
    ST_type_declaration,
    ST_expression,
    ST_type,
    ST_namespace,
    ST_using,
    ST_make_property,
    ST_make_seq,

    // Subtypes of CPPType
    ST_simple,
    ST_pointer,
    ST_reference,
    ST_array,
    ST_const,
    ST_function,
    ST_function_group,
    ST_extension,
    ST_struct,
    ST_enum,
    ST_class_template_parameter,
    ST_tbd,
    ST_type_proxy,
    ST_typedef,
  };

  CPPDeclaration(const CPPFile &file);
  CPPDeclaration(const CPPDeclaration &copy);
  virtual ~CPPDeclaration();

  bool operator == (const CPPDeclaration &other) const;
  bool operator != (const CPPDeclaration &other) const;
  bool operator < (const CPPDeclaration &other) const;

  bool is_template() const;
  CPPTemplateScope *get_template_scope() const;
  virtual bool is_fully_specified() const;
  virtual CPPDeclaration *
  instantiate(const CPPTemplateParameterList *actual_params,
              CPPScope *current_scope, CPPScope *global_scope,
              CPPPreprocessor *error_sink = NULL) const;

  typedef map<CPPDeclaration *, CPPDeclaration *> SubstDecl;
  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
                                          CPPScope *current_scope,
                                          CPPScope *global_scope);

  typedef set<CPPDeclaration *> Instantiations;
  Instantiations _instantiations;

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const=0;

  virtual SubType get_subtype() const=0;

  virtual CPPInstance *as_instance();
  virtual CPPClassTemplateParameter *as_class_template_parameter();
  virtual CPPTypedefType *as_typedef_type();
  virtual CPPTypeDeclaration *as_type_declaration();
  virtual CPPExpression *as_expression();
  virtual CPPType *as_type();
  virtual CPPNamespace *as_namespace();
  virtual CPPUsing *as_using();
  virtual CPPSimpleType *as_simple_type();
  virtual CPPPointerType *as_pointer_type();
  virtual CPPReferenceType *as_reference_type();
  virtual CPPArrayType *as_array_type();
  virtual CPPConstType *as_const_type();
  virtual CPPFunctionType *as_function_type();
  virtual CPPFunctionGroup *as_function_group();
  virtual CPPExtensionType *as_extension_type();
  virtual CPPStructType *as_struct_type();
  virtual CPPEnumType *as_enum_type();
  virtual CPPTBDType *as_tbd_type();
  virtual CPPTypeProxy *as_type_proxy();
  virtual CPPMakeProperty *as_make_property();
  virtual CPPMakeSeq *as_make_seq();

  CPPVisibility _vis;
  CPPTemplateScope *_template_scope;
  CPPFile _file;
  CPPCommentBlock *_leading_comment;

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

inline ostream &
operator << (ostream &out, const CPPDeclaration &decl) {
  decl.output(out, 0, (CPPScope *)NULL, false);
  return out;
}

ostream &
operator << (ostream &out, const CPPDeclaration::SubstDecl &decl);

#endif
