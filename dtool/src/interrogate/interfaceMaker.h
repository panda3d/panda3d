// Filename: interfaceMaker.h
// Created by:  drose (19Sep01)
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

#ifndef INTERFACEMAKER_H
#define INTERFACEMAKER_H

#include "dtoolbase.h"

#include "interrogate_interface.h"
#include "interrogate_request.h"
#include "functionWriters.h"

#include <vector>
#include <map>

class FunctionRemap;
class ParameterRemap;
class CPPType;
class CPPInstance;
class InterrogateBuilder;
class InterrogateType;
class InterrogateFunction;

////////////////////////////////////////////////////////////////////
//       Class : InterfaceMaker
// Description : This is an abstract base class that defines how to
//               generate code that can be called from an external
//               language (like Python or Squeak) and that can call
//               into Panda.
//
//               The specializations of this class like
//               InterfaceMakerPython and InterfaceMakerC will
//               generate the actual wrappers for the various language
//               calling conventions.
////////////////////////////////////////////////////////////////////
class InterfaceMaker {
public:
  InterfaceMaker(InterrogateModuleDef *def);
  virtual ~InterfaceMaker();

  virtual void generate_wrappers();

  virtual void write_includes(ostream &out);
  virtual void write_prototypes(ostream &out, ostream *out_h);
  virtual void write_functions(ostream &out);
  virtual void write_module_support(ostream &out, ostream *out_h, InterrogateModuleDef *def) {};

  virtual void write_module(ostream &out, ostream *out_h, InterrogateModuleDef *def);

  virtual ParameterRemap *remap_parameter(CPPType *struct_type, CPPType *param_type);

  virtual bool synthesize_this_parameter();
  virtual bool separate_overloading();
  virtual bool wrap_global_functions();

  void get_function_remaps(vector<FunctionRemap *> &remaps);

  static ostream &indent(ostream &out, int indent_level);

public:
  class Function {
  public:
    Function(const string &name,
             const InterrogateType &itype,
             const InterrogateFunction &ifunc);
    ~Function();

    string _name;
    const InterrogateType &_itype;
    const InterrogateFunction &_ifunc;
    typedef vector<FunctionRemap *> Remaps;
    Remaps _remaps;
    bool _has_this;
    int _flags;
  };
  typedef vector<Function *> Functions;
  Functions _functions;

  class MakeSeq {
  public:
    MakeSeq(const string &name, CPPMakeSeq *cpp_make_seq);

    string _name;
    string _seq_name;
    string _num_name;
    string _element_name;
  };
  typedef vector<MakeSeq *> MakeSeqs;

  class Object {
  public:
    Object(const InterrogateType &itype);
    ~Object();

    void check_protocols();
    bool is_static_method(const string &name);

    const InterrogateType &_itype;
    Functions _constructors;
    Functions _methods;
    MakeSeqs _make_seqs;

    enum ProtocolTypes {
      PT_sequence         = 0x0001,
      PT_mapping          = 0x0002,
      PT_make_copy        = 0x0004,
      PT_copy_constructor = 0x0008,
      PT_iter             = 0x0010,
    };
    int _protocol_types;
  };
  typedef map<TypeIndex, Object *> Objects;
  Objects _objects;

  typedef map<string, FunctionRemap *> WrappersByHash;
  WrappersByHash _wrappers_by_hash;

  virtual FunctionRemap *
  make_function_remap(const InterrogateType &itype,
                      const InterrogateFunction &ifunc,
                      CPPInstance *cppfunc, int num_default_parameters);

  virtual string
  get_wrapper_name(const InterrogateType &itype, 
                   const InterrogateFunction &ifunc,
                   FunctionIndex func_index);
  virtual string get_wrapper_prefix();
  virtual string get_unique_prefix();
  
  Function *
  record_function(const InterrogateType &itype, FunctionIndex func_index);

  virtual void
  record_function_wrapper(InterrogateFunction &ifunc, 
                          FunctionWrapperIndex wrapper_index);

  virtual Object *record_object(TypeIndex type_index);

  void hash_function_signature(FunctionRemap *remap);
  

  string
  manage_return_value(ostream &out, int indent_level,
                      FunctionRemap *remap, const string &return_expr) const;

  void output_ref(ostream &out, int indent_level, FunctionRemap *remap, 
                  const string &varname) const;
  void write_spam_message(ostream &out, FunctionRemap *remap) const;

protected:
  InterrogateModuleDef *_def;

  FunctionWriters _function_writers;
};

#endif
