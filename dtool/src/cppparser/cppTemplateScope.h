/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTemplateScope.h
 * @author drose
 * @date 1999-10-28
 */

#ifndef CPPTEMPLATESCOPE_H
#define CPPTEMPLATESCOPE_H

#include "dtoolbase.h"

#include "cppScope.h"
#include "cppTemplateParameterList.h"

/**
 * This is an implicit scope that is created following the appearance of a
 * "template<class x, class y>" or some such line in a C++ file.  It simply
 * defines the template parameters.
 */
class CPPTemplateScope : public CPPScope {
public:
  CPPTemplateScope(CPPScope *parent_scope);

  void add_template_parameter(CPPDeclaration *param);

  virtual void add_declaration(CPPDeclaration *decl, CPPScope *global_scope,
                               CPPPreprocessor *preprocessor,
                               const cppyyltype &pos);
  virtual void add_enum_value(CPPInstance *inst);
  virtual void define_typedef_type(CPPTypedefType *type,
                                   CPPPreprocessor *error_sink = nullptr);
  virtual void define_extension_type(CPPExtensionType *type,
                                     CPPPreprocessor *error_sink = nullptr);
  virtual void define_namespace(CPPNamespace *scope);
  virtual void add_using(CPPUsing *using_decl, CPPScope *global_scope,
                         CPPPreprocessor *error_sink = nullptr);

  virtual bool is_fully_specified() const;

  virtual std::string get_simple_name() const;
  virtual std::string get_local_name(CPPScope *scope = nullptr) const;
  virtual std::string get_fully_scoped_name() const;

  virtual void output(std::ostream &out, CPPScope *scope) const;

  virtual CPPTemplateScope *as_template_scope();

  CPPTemplateParameterList _parameters;
};

#endif
