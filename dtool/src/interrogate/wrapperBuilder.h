// Filename: wrapperBuilder.h
// Created by:  drose (01Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef WRAPPERBUILDER_H
#define WRAPPERBUILDER_H

#include <dtoolbase.h>

#include <interrogate_interface.h>
#include <vector_string.h>

#include <vector>

class CPPInstance;
class CPPScope;
class CPPStructType;
class CPPType;
class CPPFunctionType;
class ParameterRemap;
class FunctionWriters;

////////////////////////////////////////////////////////////////////
//       Class : WrapperBuilder
// Description : Contains all the information necessary to synthesize
//               a wrapper around a particular function.  This
//               includes choosing appropriate parameter types to
//               remap from the C++ function's parameter types, as
//               well as actually generating wrapper code.
//
//               This can wrap just one particular function signature,
//               or several different function signatures with the
//               same function name (e.g. function overloading).  In
//               the simple kinds of wrappers, like WrapperBuilderC
//               and WrapperBuilderPython, this is only intended to
//               wrap one function signature per wrapper, but wrappers
//               that can implement function overloading directly
//               (like WrapperBuilderPythonObj) will store multiple
//               function signatures and implement all of them within
//               the same wrapper.
//
//               This is an abstract class; it doesn't actually know
//               how to choose parameter types and synthesize
//               wrappers; see WrapperBuilderC and
//               WrapperBuilderPython.
////////////////////////////////////////////////////////////////////
class WrapperBuilder {
public:
  enum Type {
    T_normal,
    T_constructor,
    T_destructor,
    T_typecast_method,
    T_assignment_method,
    T_typecast,
    T_getter,
    T_setter
  };

  WrapperBuilder();
  virtual ~WrapperBuilder();

  int add_function(CPPInstance *function, const string &description,
                   CPPStructType *struct_type, CPPScope *scope,
                   const string &function_signature, Type type,
                   const string &expression,
                   int num_default_parameters);

  virtual void get_function_writers(FunctionWriters &writers);

  virtual void
  write_prototype(ostream &out, const string &wrapper_name) const=0;

  virtual void
  write_wrapper(ostream &out, const string &wrapper_name) const=0;

  virtual string
  get_wrapper_name(const string &library_hash_name) const=0;

  virtual bool supports_atomic_strings() const=0;
  virtual bool synthesize_this_parameter() const;

  enum CallingConvention {
    CC_c,
    CC_python,
    CC_python_obj,
  };

  virtual CallingConvention get_calling_convention() const=0;

  class Parameter {
  public:
    bool _has_name;
    string _name;
    ParameterRemap *_remap;
  };

  typedef vector<Parameter> Parameters;

  class FunctionDef {
  public:
    FunctionDef();
    ~FunctionDef();
  private:
    FunctionDef(const FunctionDef &copy);
    void operator = (const FunctionDef &copy);

  public:
    Parameters _parameters;
    ParameterRemap *_return_type;
    bool _void_return;
    bool _has_this;
    bool _is_method;
    Type _type;

    CPPInstance *_function;
    string _description;
    CPPStructType *_struct_type;
    CPPScope *_scope;
    CPPFunctionType *_ftype;
    string _function_signature;
    string _expression;
    int _num_default_parameters;

    bool _return_value_needs_management;
    FunctionIndex _return_value_destructor;
    bool _manage_reference_count;
  };

  typedef vector<FunctionDef *> Def;
  Def _def;

  string _hash;
  int _wrapper_index;
  bool _is_valid;

protected:
  virtual ParameterRemap *make_remap(int def_index, CPPType *orig_type);
  string manage_return_value(int def_index, 
                             ostream &out, int indent_level,
                             const string &return_expr) const;
  void output_ref(int def_index, ostream &out, int indent_level, const string &varname) const;

  string get_parameter_name(int n) const;
  string get_parameter_expr(int n, const vector_string &pexprs) const;

  string get_call_str(int def_index,
                      const string &container, 
                      const vector_string &pexprs) const;
  string call_function(int def_index, 
                       ostream &out, int indent_level,
                       bool convert_result, const string &container,
                       const vector_string &pexprs = vector_string()) const;

  void write_spam_message(int def_index, ostream &out) const;
  void write_quoted_string(ostream &out, const string &str) const;

public:
  static ostream &indent(ostream &out, int indent_level);
};

#endif
