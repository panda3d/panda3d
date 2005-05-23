// Filename: functionRemap.h
// Created by:  drose (19Sep01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef FUNCTIONREMAP_H
#define FUNCTIONREMAP_H

#include "dtoolbase.h"

#include "interrogate_interface.h"
#include "vector_string.h"

#include <vector>

class InterrogateBuilder;
class InterrogateType;
class InterrogateFunction;
class ParameterRemap;
class CPPType;
class CPPInstance;
class CPPStructType;
class CPPScope;
class CPPFunctionType;
class InterfaceMaker;

////////////////////////////////////////////////////////////////////
//       Class : FunctionRemap
// Description : This class describes how to remap a C++ function (and
//               its list of parameters and return type) to a wrapped
//               function, for a particular scripting language.
//
//               The InterfaceMaker class will create one of these for
//               each function, including one for each instance of an
//               overloaded function.
////////////////////////////////////////////////////////////////////
class FunctionRemap {
public:
  FunctionRemap(const InterrogateType &itype,
                const InterrogateFunction &ifunc,
                CPPInstance *cppfunc, int num_default_parameters,
                InterfaceMaker *interface);
  ~FunctionRemap();

  string get_parameter_name(int n) const;
  string call_function(ostream &out, int indent_level, 
                       bool convert_result, const string &container,
                       const vector_string &pexprs = vector_string()) const;

  void write_orig_prototype(ostream &out, int indent_level) const;

  FunctionWrapperIndex make_wrapper_entry(FunctionIndex function_index);

  class Parameter {
  public:
    bool _has_name;
    string _name;
    ParameterRemap *_remap;
  };

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

  typedef vector<Parameter> Parameters;

  Parameters _parameters;
  ParameterRemap *_return_type;
  bool _void_return;
  bool _ForcedVoidReturn;
  bool _has_this;
  int _first_true_parameter;
  int _num_default_parameters;
  Type _type;
  string _expression;
  string _function_signature;
  string _hash;
  string _unique_name;
  string _reported_name;
  string _wrapper_name;
  FunctionWrapperIndex _wrapper_index;
  
  bool _return_value_needs_management;
  FunctionIndex _return_value_destructor;
  bool _manage_reference_count;

  CPPType *_cpptype;
  CPPScope *_cppscope;
  CPPInstance *_cppfunc;
  CPPFunctionType *_ftype;

  bool _is_valid;
private:
  string get_call_str(const string &container, const vector_string &pexprs) const;
  string get_parameter_expr(int n, const vector_string &pexprs) const;
  bool setup_properties(const InterrogateFunction &ifunc, InterfaceMaker *interface);
};

#endif
