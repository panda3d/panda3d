// Filename: interrogateBuilder.h
// Created by:  drose (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATEBUILDER_H
#define INTERROGATEBUILDER_H

#include <dtoolbase.h>

#include "interrogate_interface.h"
#include "interrogate_request.h"
#include "wrapperBuilder.h"

#include <map>
#include <set>
#include <vector>

class CPPFunctionGroup;
class CPPInstance;
class CPPType;
class CPPSimpleType;
class CPPPointerType;
class CPPConstType;
class CPPExtensionType;
class CPPStructType;
class CPPEnumType;
class CPPFunctionType;
class CPPScope;
class CPPIdentifier;
class CPPNameComponent;
class CPPManifest;
class InterrogateType;

////////////////////////////////////////////////////////////////////
// 	 Class : InterrogateBuilder
// Description : This class builds up the InterrogateDatabase based on
//               the data indicated by CPPParser after reading the
//               source code.
////////////////////////////////////////////////////////////////////
class InterrogateBuilder {
public:
  void add_source_file(const string &filename);
  void read_command_file(istream &in);
  void do_command(const string &command, const string &params);
  void build();
  void write_code(ostream &out, InterrogateModuleDef *def);
  InterrogateModuleDef *make_module_def(int file_identifier);

  static string clean_identifier(const string &name);
  static string descope(const string &name);
  FunctionIndex get_destructor_for(CPPType *type);

private:
  typedef set<string> Commands;
  void insert_param_list(InterrogateBuilder::Commands &commands, 
			 const string &params);

  bool in_forcetype(const string &name) const;
  bool in_ignoretype(const string &name) const;
  bool in_ignoreinvolved(const string &name) const;
  bool in_ignoreinvolved(CPPType *type) const;
  bool in_ignorefile(const string &name) const;
  bool in_ignoremember(const string &name) const;

  void remap_indices();
  void scan_function(CPPFunctionGroup *fgroup);
  void scan_function(CPPInstance *function);
  void scan_struct_type(CPPStructType *type);
  void scan_enum_type(CPPEnumType *type);
  void scan_manifest(CPPManifest *manifest);
  ElementIndex scan_element(CPPInstance *element, CPPStructType *struct_type, 
			    CPPScope *scope);

  FunctionIndex get_getter(CPPType *expr_type, string expression,
			   CPPStructType *struct_type, CPPScope *scope,
			   CPPInstance *element);
  FunctionIndex get_setter(CPPType *expr_type, string expression,
			   CPPStructType *struct_type, CPPScope *scope,
			   CPPInstance *element);
  FunctionIndex get_cast_function(CPPType *to_type, CPPType *from_type,
				  const string &prefix);
  FunctionIndex
  get_function(CPPInstance *function, string description,
	       CPPStructType *struct_type, CPPScope *scope,
	       bool global, WrapperBuilder::Type wtype,
	       const string &expression = string());

  void make_wrappers();
  FunctionWrapperIndex 
  get_wrapper(FunctionIndex function_index, 
	      WrapperBuilder *wbuilder, CPPInstance *function, 
	      string description, CPPStructType *struct_type, CPPScope *scope,
	      WrapperBuilder::Type wtype, const string &expression,
	      int num_default_parameters);

  TypeIndex get_atomic_string_type();
  TypeIndex get_type(CPPType *type, bool global);

  void define_atomic_type(InterrogateType &itype, CPPSimpleType *cpptype);
  void define_wrapped_type(InterrogateType &itype, CPPPointerType *cpptype);
  void define_wrapped_type(InterrogateType &itype, CPPConstType *cpptype);
  void define_struct_type(InterrogateType &itype, CPPStructType *cpptype,
			  bool forced);
  void update_method_comment(CPPInstance *function, CPPStructType *struct_type,
			     CPPScope *scope);
  void define_method(CPPFunctionGroup *fgroup, InterrogateType &itype,
		     CPPStructType *struct_type, CPPScope *scope);
  void define_method(CPPInstance *function, InterrogateType &itype, 
		     CPPStructType *struct_type, CPPScope *scope);
  void define_enum_type(InterrogateType &itype, CPPEnumType *cpptype);
  void define_extension_type(InterrogateType &itype, 
                             CPPExtensionType *cpptype);

  void hash_function_signature(WrapperBuilder *wbuilder);
  string hash_string(const string &name, 
		     int additional_number = 0,
		     int shift_offset = 5);

  static string trim_blanks(const string &str);

  class NewFunction {
  public:
    CPPInstance *_function;
    string _description;
    CPPFunctionType *_ftype;
    CPPStructType *_struct_type;
    CPPScope *_scope;
    WrapperBuilder::Type _wtype;
    string _expression;
    int _function_index;
  };
  typedef vector<NewFunction> NewFunctions;
  NewFunctions _new_functions;

  typedef map<string, TypeIndex> TypesByName;
  typedef map<string, FunctionIndex> FunctionsBySignature;
  typedef map<string, WrapperBuilder *> WrappersByHash;

  TypesByName _types_by_name;
  FunctionsBySignature _functions_by_signature;
  WrappersByHash _wrappers_by_hash;

  typedef set<string> IncludeFiles;
  IncludeFiles _include_files;

  Commands _forcetype;
  Commands _ignoretype;
  Commands _ignoreinvolved;
  Commands _ignorefile;
  Commands _ignoremember;

  string _library_hash_name;
};

extern InterrogateBuilder builder;

#endif


