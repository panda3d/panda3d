// Filename: wrapperBuilder.h
// Created by:  drose (01Aug00)
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

////////////////////////////////////////////////////////////////////
// 	 Class : WrapperBuilder
// Description : Contains all the information necessary to synthesize
//               a wrapper around a particular function.  This
//               includes choosing appropriate parameter types to
//               remap from the C++ function's parameter types, as
//               well as actually generating wrapper code.
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

  void clear();
  bool set_function(CPPInstance *function, const string &description,
		    CPPStructType *struct_type, CPPScope *scope,
		    const string &function_signature, Type type,
		    const string &expression,
		    int num_default_parameters);

  bool is_valid() const;
  bool return_value_needs_management() const;
  FunctionIndex get_return_value_destructor() const;

  virtual void
  write_wrapper(ostream &out, const string &wrapper_name) const=0;

  virtual string
  get_wrapper_name(const string &library_hash_name) const=0;

  virtual bool supports_atomic_strings() const=0;

  enum CallingConvention {
    CC_c,
    CC_python,
  };

  virtual CallingConvention get_calling_convention() const=0;

  class Parameter {
  public:
    bool _has_name;
    string _name;
    ParameterRemap *_remap;
  };

  typedef vector<Parameter> Parameters;
  Parameters _parameters;
  ParameterRemap *_return_type;
  bool _void_return;
  bool _has_this;
  Type _type;

  CPPInstance *_function;
  string _description;
  CPPStructType *_struct_type;
  CPPScope *_scope;
  CPPFunctionType *_ftype;
  string _function_signature;
  string _expression;
  int _num_default_parameters;
  string _hash;
  int _wrapper_index;

protected:
  virtual ParameterRemap *make_remap(CPPType *orig_type);
  string manage_return_value(ostream &out, int indent_level,
			     const string &return_expr) const;
  void output_ref(ostream &out, int indent_level, const string &varname) const;

  string get_parameter_name(int n) const;
  string get_parameter_expr(int n, const vector_string &pexprs) const;

  string get_call_str(const vector_string &pexprs = vector_string()) const;
  string call_function(ostream &out, int indent_level, 
		       bool convert_result = true,
		       const vector_string &pexprs = vector_string()) const;

  void write_spam_message(ostream &out) const;
  void write_quoted_string(ostream &out, const string &str) const;

  bool _is_valid;
  bool _return_value_needs_management;
  FunctionIndex _return_value_destructor;
  bool _manage_reference_count;

public:
  static ostream &indent(ostream &out, int indent_level);
};

#endif
