// Filename: InterfaceMakerPythonNative.h
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

#ifndef INTERFACEMAKERPYTHONNATIVE_H
#define INTERFACEMAKERPYTHONNATIVE_H
#include "map"
#include "set"
#include "dtoolbase.h"

#include "interfaceMakerPython.h"
#include "interrogate_interface.h"
#include "cppStructType.h"

class FunctionRemap;

////////////////////////////////////////////////////////////////////
//       Class : InterfaceMakerPythonNative
// Description : An InterfaceMaker for generating complex Python
//               function wrappers around C++ code.
////////////////////////////////////////////////////////////////////
class InterfaceMakerPythonNative : public InterfaceMakerPython 
{
public:
    InterfaceMakerPythonNative(InterrogateModuleDef *def);
    virtual ~InterfaceMakerPythonNative();


    virtual void write_prototypes(ostream &out,ostream *out_h);
    void write_prototypes_class(ostream &out,ostream *out_h, Object * obj) ;
    void write_prototypes_class_external(ostream &out, Object * obj);

    virtual void write_functions(ostream &out);

    virtual void write_module(ostream &out,ostream *out_h, InterrogateModuleDef *def);
    virtual void write_module_support(ostream &out, ostream *out_h,InterrogateModuleDef *moduledefdef);

    void write_module_class(ostream &out, Object *cls); 
    virtual void write_sub_module(ostream &out,  Object *obj); 

    virtual bool synthesize_this_parameter();

    virtual Object *record_object(TypeIndex type_index);

protected:
    virtual string get_wrapper_prefix();
    virtual string get_unique_prefix();
    virtual void record_function_wrapper(InterrogateFunction &ifunc, 
        FunctionWrapperIndex wrapper_index);

    virtual void generate_wrappers();

private:
    void write_prototype_for_name(ostream &out, Function *func, const std::string &name);
    void write_prototype_for(ostream &out, Function *func);
    void write_function_for_name(ostream &out, Function *func, const std::string &name, const std::string &PreProcess, const std::string &ClassName);
    void write_function_for_top(ostream &out, Function *func, const std::string &PreProcess);
    void write_function_instance(ostream &out, Function *func,
        FunctionRemap *remap , string &expected_params, int indent_level, bool errors_fatal, ostream &forwarddecl, const std::string &functionnamestr, bool inplace ) ;

    void write_function_forset(ostream &out, InterfaceMaker::Function *func,
        std::set< FunctionRemap *> &remaps, string &expected_params, int indent_level , ostream &forwarddecl,const std::string &functionname, bool inplace) ;

    void pack_return_value(ostream &out, int indent_level,
        FunctionRemap *remap, std::string return_expr ,  ostream &forwarddecl, bool in_place);

    void pack_return_value_tnit(ostream &out, int indent_level,
        FunctionRemap *remap, string return_expr) ;

    void write_class_prototypes(ostream &out) ;
    void write_ClasseDeclarations(ostream &out ,  ostream *out_h,Object * obj);
    void write_ClasseDetails(ostream &out, Object * obj);

    void do_assert_init(ostream &out, int indent_level, bool constructor) const;
public:
    bool isRemapLegal( FunctionRemap &remap);
    bool isFunctionLegal( Function *func);
    bool isCppTypeLegal(CPPType *ctype);
    bool isExportThisRun(CPPType *ctype);
    bool isExportThisRun(Function *func);
    bool isFunctionWithThis( Function *func);
    bool IsRunTimeTyped(const InterrogateType &itype);

    // comunicates the cast capabilites among methods..
    struct CastDetails
    {
        CPPStructType   *_structType;
        std::string     _to_class_name;
        std::string     _up_cast_string;
        bool            _can_downcast;
        bool            _is_legal_py_class;
    };

    void   GetValideChildClasses( std::map< std::string ,CastDetails > &answer, CPPStructType * inclass,  const std::string &up_cast_seed = "", bool downcastposible = true);
    bool   DoesInheritFromIsClass( const CPPStructType * inclass, const std::string &name);
    bool   IsPandaTypedObject(CPPStructType * inclass) { return DoesInheritFromIsClass(inclass,"TypedObject"); };
    void WriteReturnInstance(ostream &out, int indent_level, std::string &return_expr, std::string &ows_memory_flag,const std::string &class_name, CPPType *ctype, bool inplace);
    bool HasAGetKeyFunction(const InterrogateType &itype_class);
    int NeedsAStrFunction(const InterrogateType &itype_class);
    bool NeedsAReprFunction(const InterrogateType &itype_class);

    void output_quoted(ostream &out, int indent_level, const std::string &str);

    // stash the forwad declarations for this compile pass..
    std::set< std::string >     _external_imports;    

};

#endif
