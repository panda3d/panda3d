/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppTypeProxy.h
 * @author drose
 * @date 1999-12-07
 */

#ifndef CPPTYPEPROXY_H
#define CPPTYPEPROXY_H

#include "dtoolbase.h"

#include "cppType.h"

/**
 * This is a special kind of type that is a placeholder for some type,
 * currently unknown, that will be filled in later.  It's used when a type
 * that references itself must instantiate.
 */
class CPPTypeProxy : public CPPType {
public:
  CPPTypeProxy();

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_tbd() const;

  bool has_typedef_name() const;
  std::string get_typedef_name(CPPScope *scope = nullptr) const;

  virtual std::string get_simple_name() const;
  virtual std::string get_local_name(CPPScope *scope = nullptr) const;
  virtual std::string get_fully_scoped_name() const;
  virtual std::string get_preferred_name() const;

  virtual bool is_incomplete() const;

  virtual void output_instance(std::ostream &out, int indent_level,
                               CPPScope *scope,
                               bool complete, const std::string &prename,
                               const std::string &name) const;
  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;

  virtual SubType get_subtype() const;

  virtual CPPType *as_type();
  virtual CPPSimpleType *as_simple_type();
  virtual CPPPointerType *as_pointer_type();
  virtual CPPReferenceType *as_reference_type();
  virtual CPPArrayType *as_array_type();
  virtual CPPConstType *as_const_type();
  virtual CPPFunctionType *as_function_type();
  virtual CPPExtensionType *as_extension_type();
  virtual CPPStructType *as_struct_type();
  virtual CPPEnumType *as_enum_type();
  virtual CPPTBDType *as_tbd_type();
  virtual CPPTypedefType *as_typedef_type();
  virtual CPPTypeProxy *as_type_proxy();

  CPPType *_actual_type;
};

#endif
