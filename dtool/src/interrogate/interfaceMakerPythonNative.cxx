// Filename: interfaceMakerPythonNative.cxx
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
#include "interfaceMakerPythonNative.h"
#include "interrogateBuilder.h"
#include "interrogate.h"
#include "functionRemap.h"
#include "parameterRemapUnchanged.h"
#include "typeManager.h"

#include "interrogateDatabase.h"
#include "interrogateType.h"
#include "interrogateFunction.h"
#include "cppFunctionType.h"
#include "cppPointerType.h"
#include "cppTypeDeclaration.h"
#include "cppStructType.h"
#include "vector"
#include "cppParameterList.h"
#include "algorithm"

#include <set>
#include <map>


extern bool     inside_python_native;
extern          InterrogateType dummy_type;
extern std::string EXPORT_IMPORT_PREFEX;

#define         CLASS_PREFEX  "Dtool_"
#define         INSTANCE_PREFEX "Dtool_"
#define         BASE_INSTANCE_NAME "Dtool_PyInstDef"


/////////////////////////////////////////////////////////
// Name Remaper...
//      Snagged from ffi py code....
/////////////////////////////////////////////////////////
struct RenameSet {
  const char *_from;
  const char *_to;
  int function_type;
};
struct FlagSet {
  const char *_to;
  int function_type;
};

///////////////////////////////////////////////////////////////////////////////////////
RenameSet methodRenameDictionary[] = {
    { "operator =="  , "eq",                   0 },
    { "operator !="  , "ne",                   0 },
    { "operator <<"  , "__lshift__",           0 },
    { "operator >>"  , "__rshift__",           0 },
    { "operator <"   , "lessThan",             0 },
    { "operator >"   , "greaterThan",          0 },
    { "operator <="  , "lessThanOrEqual",      0 },
    { "operator >="  , "greaterThanOrEqual",   0 },
    { "operator ="   , "assign",               0 },
    { "operator ()"  , "__call__",             0 },
    { "operator []"  , "__getitem__",          0 },
    { "operator ++unary"  , "increment",            0 },
    { "operator ++"  , "increment",            0 },
    { "operator --unary"  , "decrement",            0 },
    { "operator --"  , "decrement",            0 },
    { "operator ^"   , "__xor__",              0 },
    { "operator %"   , "__mod__",              0 },
    { "operator !"   , "logicalNot",           0 },
    { "operator ~unary"   , "__invert__",           0 },
    { "operator &"   , "__and__",              0 },
    { "operator &&"  , "logicalAnd",           0 },
    { "operator |"   , "__or__",               0 },
    { "operator ||"  , "logicalOr",            0 },
    { "operator +"   , "__add__",              0 },
    { "operator -"   , "__sub__",              0 },
    { "operator -unary", "__neg__",            0 },
    { "operator *"   , "__mul__",              0 },
    { "operator /"   , "__div__",              0 },
    { "operator +="  , "__iadd__",             1 },
    { "operator -="  , "__isub__",             1 },
    { "operator *="  , "__imul__",             1 },
    { "operator /="  , "__idiv__",             1 },
    { "operator ,"   , "concatenate",          0 },
    { "operator |="  , "__ior__",              1 },
    { "operator &="  , "__iand__",             1 },
    { "operator ^="  , "__ixor__",             1 },
    { "operator ~="  , "bitwiseNotEqual",      0 },
    { "operator ->"  , "dereference",          0 },
    { "operator <<=" , "__ilshift__",          1 },
    { "operator >>=" , "__irshift__",          1 },
    { "operator typecast bool" , "__nonzero__", 0 },
    { "print"       , "Cprint",               0 },
    { "CInterval.set_t" , "_priv__cSetT",      0 },
    { NULL, NULL, -1 }
    };

const char *  InPlaceSet[] = {
     "__iadd__",            
     "__isub__",            
     "__imul__",            
     "__idiv__",            
     "__ior__",             
     "__iand__",            
     "__ixor__",            
     "__ilshift__",         
     "__irshift__",         
    NULL,
    };



///////////////////////////////////////////////////////////////////////////////////////
RenameSet classRenameDictionary[] = {
    { "Loader"                    , "PandaLoader",0 },
    { "string"                    , "CString",0 },
    { "LMatrix4f"                 , "Mat4",0 },
    { "LMatrix3f"                 , "Mat3",0 },
    { "LVecBase4f"                , "VBase4",0 },
    { "LVector4f"                 , "Vec4",0 },
    { "LPoint4f"                  , "Point4",0 },
    { "LVecBase3f"                , "VBase3",0 },
    { "LVector3f"                 , "Vec3",0 },
    { "LPoint3f"                  , "Point3",0 },
    { "LVecBase2f"                , "VBase2",0 },
    { "LVector2f"                 , "Vec2",0 },
    { "LPoint2f"                  , "Point2",0 },
    { "LQuaternionf"              , "Quat",0 },
    { "LMatrix4d"                 , "Mat4D",0 },
    { "LMatrix3d"                 , "Mat3D",0 },
    { "LVecBase4d"                , "VBase4D",0 },
    { "LVector4d"                 , "Vec4D",0 },
    { "LPoint4d"                  , "Point4D",0 },
    { "LVecBase3d"                , "VBase3D",0 },
    { "LVector3d"                 , "Vec3D",0 },
    { "LPoint3d"                  , "Point3D",0 },
    { "LVecBase2d"                , "VBase2D",0 },
    { "LVector2d"                 , "Vec2D",0 },
    { "LPoint2d"                  , "Point2D",0 },
    { "LQuaterniond"              , "QuatD",0 },
    { "Plane"                     , "PlaneBase",0 },
    { "Planef"                    , "Plane",0 },
    { "Planed"                    , "PlaneD",0 },
    { "Frustum"                   , "FrustumBase",0 },
    { "Frustumf"                  , "Frustum",0 },
    { "Frustumd"                  , "FrustumD",0 },
    { NULL,NULL,-1 }
    };

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
const char * pythonKeywords[] = {
    "and",
        "del",
        "for",
        "is",
        "raise",
        "assert",
        "elif",
        "from",
        "lambda",
        "return",
        "break",
        "else",
        "global",
        "not",
        "try",
        "class",
        "except",
        "if",
        "or",
        "while",
        "continue",
        "exec",
        "import",
        "pass",
        "def",
        "finally",
        "in",
        "print",
        NULL};
///////////////////////////////////////////////////////////////////////////////////////
std::string  checkKeyword(std::string & cppName)
{
    for(int x = 0; pythonKeywords[x] != NULL; x++)
    {
        if(cppName == pythonKeywords[x])
        {
            return std::string("_")+cppName;
        }
    }
    return cppName;
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
std::string  classNameFromCppName(const std::string &cppName)
{
    //# initialize to empty string
    std::string className = "";
    //# These are the characters we want to strip out of the name
    const std::string  badChars("!@#$%^&*()<>,.-=+~{}?");
    int nextCap = 0;
    int firstChar = 1;
    for(std::string::const_iterator  chr = cppName.begin(); chr != cppName.end(); chr++)
    {
        if (badChars.find(*chr) != std::string::npos)
        {
        }
        else if (*chr == '_' || *chr == ' ')
        {
            nextCap = 1;
        }
        else if (nextCap || firstChar)
        {
            className += toupper(*chr);
            nextCap = 0;
            firstChar = 0;
        }
        else
        {
            className += * chr;
        }
    }
    for(int x = 0; classRenameDictionary[x]._from != NULL; x++)
    {
        if(cppName == classRenameDictionary[x]._from)
            className = classRenameDictionary[x]._to;
    }

    if (className.empty())
    {
        std::string text = "** ERROR ** Renaming class: " + cppName + " to empty string";
        printf("%s",text.c_str());
    }


    //# FFIConstants.notify.debug('Renaming class: ' + cppName + ' to: ' + className)
    //# Note we do not have to check for keywords because class name are capitalized
    return className;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
std::string nonClassNameFromCppName(const std::string &cppName_in)
{
    std::string className = classNameFromCppName(cppName_in);
    //# Make the first character lowercase
    std::string newName;
    int pass = 0;
    for(std::string::const_iterator  chr = className.begin(); chr != className.end(); chr++)
    {
        if(pass == 0)
            newName += toupper(*chr);
        else
            newName += tolower(*chr);
        pass++;
    }    
    //# Mangle names that happen to be python keywords so they are not anymore
    //newName = checkKeyword(newName)
    return newName;
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
std::string  methodNameFromCppName(const std::string &cppName, const std::string &className) {
    std::string methodName;
    const std::string  badChars("!@#$%^&*()<>,.-=+~{}?");
    int nextCap = 0;
    for(std::string::const_iterator  chr = cppName.begin(); chr != cppName.end(); chr++)
    {
        if (badChars.find(*chr) != std::string::npos)
        {
        }
        else if (*chr == '_' || *chr == ' ')
        {
            nextCap = 1;
        }
        else if (nextCap)
        {
            methodName += toupper(*chr);
            nextCap = 0;
        }
        else
        {
            methodName += *chr;
        }
    }

    for(int x = 0; methodRenameDictionary[x]._from != NULL; x++)
    {
        if(cppName == methodRenameDictionary[x]._from)
        {
            methodName = methodRenameDictionary[x]._to;
        }
    }

    if(className.size() > 0)
    {
        string LookUpName = className + '.' + cppName;
        for(int x = 0; classRenameDictionary[x]._from != NULL; x++)
        {
            if(LookUpName == methodRenameDictionary[x]._from)
                methodName = methodRenameDictionary[x]._to;
        }
    }
 

//    # Mangle names that happen to be python keywords so they are not anymore
    methodName = checkKeyword(methodName);
    return methodName;
}

std::string  methodNameFromCppName(InterfaceMaker::Function *func, const std::string &className)
{
    std::string cppName = func->_ifunc.get_name();
    if (func->_ifunc.is_unary_op()) {
      cppName += "unary";
    }
    return methodNameFromCppName(cppName, className);
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string make_safe_name(const std::string & name)
{
    return InterrogateBuilder::clean_identifier(name);

    /*
    static const char safe_chars2[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
        std::string result = name;

        size_t pos = result.find_first_not_of(safe_chars2);
        while (pos != std::string::npos)
        {
                result[pos] = '_';
                pos = result.find_first_not_of(safe_chars2);
        }

        return result;
        */
}

bool isInplaceFunction(InterfaceMaker::Function *func)
{
    std::string wname = methodNameFromCppName(func,"");

    for(int x = 0; InPlaceSet[x] != NULL; x++)
        if(InPlaceSet[x] == wname)
            return true;

    return false;
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::get_slotted_function_def
//       Access: Private, Static
//  Description: Determines whether this method should be mapped to
//               one of Python's special slotted functions, those
//               hard-coded functions that are assigned to particular
//               function pointers within the object structure, for
//               special functions like __getitem__ and __len__.
//
//               Returns true if it has such a mapping, false if it is
//               just a normal method.  If it returns true, the
//               SlottedFunctionDef structure is filled in with the
//               important details.
////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::
get_slotted_function_def(Object *obj, Function *func, SlottedFunctionDef &def) {
  def._answer_location = string();
  def._wrapper_type = WT_none;

  string method_name = func->_ifunc.get_name();
  bool is_unary_op = func->_ifunc.is_unary_op();

  if (method_name == "operator +") {
    def._answer_location = "tp_as_number->nb_add";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator -" && is_unary_op) {
    def._answer_location = "tp_as_number->nb_negative";
    def._wrapper_type = WT_no_params;
    return true;        
  }

  if (method_name == "operator -") {
    def._answer_location = "tp_as_number->nb_subtract";
    def._wrapper_type = WT_numeric_operator;
    return true;        
  }

  if (method_name == "operator *") {
    def._answer_location = "tp_as_number->nb_multiply";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator /") {
    def._answer_location = "tp_as_number->nb_divide";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator %") {
    def._answer_location = "tp_as_number->nb_remainder";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator <<") {
    def._answer_location = "tp_as_number->nb_lshift";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator >>") {
    def._answer_location = "tp_as_number->nb_rshift";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator ^") {
    def._answer_location = "tp_as_number->nb_xor";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator ~" && is_unary_op) {
    def._answer_location = "tp_as_number->nb_invert";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "operator &") {
    def._answer_location = "tp_as_number->nb_and";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator |") {
    def._answer_location = "tp_as_number->nb_or";
    def._wrapper_type = WT_numeric_operator;
    return true;
  }

  if (method_name == "operator +=") {
    def._answer_location = "tp_as_number->nb_inplace_add";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (method_name == "operator -=") {
    def._answer_location = "tp_as_number->nb_inplace_subtract";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (method_name == "operator *=") {
    def._answer_location = "tp_as_number->nb_inplace_multiply";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (method_name == "operator /=") {
    def._answer_location = "tp_as_number->nb_inplace_divide";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (method_name == "operator %=") {
    def._answer_location = ".tp_as_number->nb_inplace_remainder";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (method_name == "operator <<=") {
    def._answer_location = "tp_as_number->nb_inplace_lshift";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (method_name == "operator >>=") {
    def._answer_location = "tp_as_number->nb_inplace_rshift";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (method_name == "operator &=") {
    def._answer_location = "tp_as_number->nb_inplace_and";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (method_name == "operator ^=") {
    def._answer_location = "tp_as_number->nb_inplace_xor";
    def._wrapper_type = WT_one_param;
    return true;
  }

  if (obj->_protocol_types & Object::PT_sequence) {
    if (func->_flags & FunctionRemap::F_getitem_int) {
      def._answer_location = "tp_as_sequence->sq_item";
      def._wrapper_type = WT_sequence_getitem;
      return true;
    }
    if (func->_flags & FunctionRemap::F_setitem_int) {
      def._answer_location = "tp_as_sequence->sq_ass_item";
      def._wrapper_type = WT_sequence_setitem;
      return true;
    }
    if (func->_flags & FunctionRemap::F_size) {
      def._answer_location = "tp_as_sequence->sq_length";
      def._wrapper_type = WT_sequence_size;
      return true;
    }
  }
  
  if (obj->_protocol_types & Object::PT_mapping) {
    if (func->_flags & FunctionRemap::F_getitem) {
      def._answer_location = "tp_as_mapping->mp_subscript";
      def._wrapper_type = WT_one_param;
      return true;
    }
    if (func->_flags & FunctionRemap::F_setitem) {
      def._answer_location = "tp_as_mapping->mp_ass_subscript";
      def._wrapper_type = WT_mapping_setitem;
      return true;
    }
  }

  if (method_name == "operator ()") {
    def._answer_location = "tp_call";
    def._wrapper_type = WT_none;
    return true;
  }

  if (method_name == "__getattr__") {
    def._answer_location = "tp_getattro";
    def._wrapper_type = WT_getattr;
    return true;
  }

  if (method_name == "__setattr__") {
    def._answer_location = "tp_setattro";
    def._wrapper_type = WT_setattr;
    return true;
  }

  if (func->_ifunc.is_operator_typecast()) {
    // A typecast operator.  Check for a supported low-level typecast type.
    if (!func->_remaps.empty()) {
      if (TypeManager::is_bool(func->_remaps[0]->_return_type->get_orig_type())) {
        // If it's a bool type, then we wrap it with the __nonzero__
        // slot method.
        def._answer_location = "tp_as_number->nb_nonzero";
        def._wrapper_type = WT_inquiry;
        return true;

      } else if (TypeManager::is_integer(func->_remaps[0]->_return_type->get_orig_type())) {
        // An integer type.
        def._answer_location = "tp_as_number->nb_int";
        def._wrapper_type = WT_no_params;
        return true;

      } else if (TypeManager::is_float(func->_remaps[0]->_return_type->get_orig_type())) {
        // A floating-point (or double) type.
        def._answer_location = "tp_as_number->nb_float";
        def._wrapper_type = WT_no_params;
        return true;
      }
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void   InterfaceMakerPythonNative::GetValideChildClasses( std::map< std::string ,CastDetails > &answer, CPPStructType * inclass,  const std::string &up_cast_seed, bool downcastposible)
{
    if(inclass == NULL)
        return;

    CPPStructType::Derivation::const_iterator bi;
    for (bi = inclass->_derivation.begin();
        bi != inclass->_derivation.end();
        ++bi) 
    {

        const CPPStructType::Base &base = (*bi);
//        if (base._vis <= V_public) 
  //          downcastposible = false;
        CPPStructType *base_type = TypeManager::resolve_type(base._base)->as_struct_type();
        if(base_type != NULL)
        {
            std::string scoped_name = base_type->get_fully_scoped_name();

            if(answer.find(scoped_name) == answer.end())
            {    
                answer[scoped_name]._can_downcast = downcastposible;
                answer[scoped_name]._to_class_name = scoped_name;
                answer[scoped_name]._structType = base_type;

                if(base._is_virtual)
                     answer[scoped_name]._can_downcast = false;

                std::string local_up_cast("( ");
                local_up_cast += scoped_name + " *)"+ up_cast_seed +"";
                answer[scoped_name]._up_cast_string = local_up_cast;
                answer[scoped_name]._is_legal_py_class = isCppTypeLegal(base_type);
            }
            else
            {
                answer[scoped_name]._can_downcast = false;
            }

            GetValideChildClasses(answer,base_type, answer[scoped_name]._up_cast_string,answer[scoped_name]._can_downcast);
        }
    }  
}
///////////////////////////////////////////////////////////////////////////////
//  Function : WriteReturnInstance
//
///////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::WriteReturnInstance(ostream &out, int indent_level, std::string &return_expr, std::string &ows_memory_flag, const std::string &class_name, CPPType *ctype, bool inplace, const std::string &const_flag)
{
    if(inplace == true)
    {
        indent(out, indent_level)<<"Py_INCREF(self);\n";
        indent(out, indent_level)<<"return self;\n";
    }
    else
    {

        indent(out, indent_level)<<"if("<< return_expr<< " == NULL)\n";
        indent(out, indent_level)<<"{\n";
        indent(out, indent_level)<<"    Py_INCREF(Py_None);\n";
        indent(out, indent_level)<<"    return Py_None;\n";
        indent(out, indent_level)<<"}\n";

        if(IsPandaTypedObject(ctype->as_struct_type()))
        {
            std::string typestr = "(" + return_expr + ")->as_typed_object()->get_type_index()";
            indent(out, indent_level)<<"return DTool_CreatePyInstanceTyped((void *)" << return_expr <<"," << CLASS_PREFEX << make_safe_name(class_name) << ","<< ows_memory_flag << ", " << const_flag << ", " << typestr << ");\n";

        }
        else
        {
            //    indent(out, indent_level)<< "if(" << return_expr <<"!= NULL)\n";
            indent(out, indent_level)
              <<"return DTool_CreatePyInstance((void *)" << return_expr <<"," << CLASS_PREFEX << make_safe_name(class_name) << ","<<ows_memory_flag << ", " << const_flag <<");\n";
        }
    }
}
////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
InterfaceMakerPythonNative::
InterfaceMakerPythonNative(InterrogateModuleDef *def) :
  InterfaceMakerPython(def) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
InterfaceMakerPythonNative::
~InterfaceMakerPythonNative() {
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_prototypes
//       Access: Public, Virtual
//  Description: Generates the list of function prototypes
//               corresponding to the functions that will be output in
//               write_functions().
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_prototypes(ostream &out_code,ostream *out_h) 
{
  inside_python_native = true;

  Functions::iterator fi;

  if(out_h != NULL)
      *out_h << "#include \"py_panda.h\"\n\n";

  out_code << "//********************************************************************\n";
  out_code << "//*** prototypes for .. Glabal\n";
  out_code << "//********************************************************************\n";

  /*
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) 
  {
      Function *func = (*fi);
      if(!func->_itype.is_global() && isFunctionLegal(func))
        write_prototype_for(out_code, func);
  }
  */

  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) 
  {
      Object *object = (*oi).second;
      if(object->_itype.is_class() ||object->_itype.is_struct())
      {
          if(isCppTypeLegal(object->_itype._cpptype))
          {
              if(isExportThisRun(object->_itype._cpptype))
              {
                write_prototypes_class(out_code,out_h,object);
              }
              else
                //write_prototypes_class_external(out_code,object);
                _external_imports.insert(make_safe_name(object->_itype.get_scoped_name()));
          }
      }
  }

  out_code << "//********************************************************************\n";
  out_code << "//*** prototypes for .. Extrernal Objects \n";
  out_code << "//********************************************************************\n";

  for(std::set< std::string >::iterator ii = _external_imports.begin(); ii != _external_imports.end(); ii++)
         out_code << "IMPORT_THIS struct   Dtool_PyTypedObject Dtool_" <<*ii <<";\n";


  inside_python_native = false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
//  Function : write_prototypes_class_external
//    
//   Description :  Output enough enformation to a declartion of a externally 
//                 generated dtool type object
/////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_prototypes_class_external(ostream &out, Object * obj) 
{
    std::string class_name = make_safe_name(obj->_itype.get_scoped_name());
    std::string c_class_name =  obj->_itype.get_true_name();


    out << "//********************************************************************\n";
    out << "//*** prototypes for external.. " << class_name <<"\n";
    out << "//********************************************************************\n";

    out << "typedef  "<< c_class_name <<"  "<< class_name <<"_localtype;\n";
    out << "Define_Module_Class_Forward("<< _def->module_name << ", "<< class_name << "," << class_name <<"_localtype,"<< classNameFromCppName(c_class_name) << ");\n";

}
///////////////////////////////////////// ////////////////////////////////////////////////////
// Function : write_prototypes_class
//
/////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_prototypes_class(ostream &out_code,ostream *out_h, Object * obj) 
{
    std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
    Functions::iterator fi;
    
    out_code << "//********************************************************************\n";
    out_code << "//*** prototypes for .. " << ClassName <<"\n";
    out_code << "//********************************************************************\n";

    /*
    for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) 
    {
        Function *func = (*fi);
        write_prototype_for(out_code, func);
    }
    */

    /*
    for (fi = obj->_constructors.begin(); fi != obj->_constructors.end(); ++fi) 
    {
        Function *func = (*fi);
        std::string fname =     "int  Dtool_Init_"+ClassName+"(PyObject *self, PyObject *args, PyObject *kwds)";
        write_prototype_for_name(out_code, obj, func,fname);

    }
    */

    write_ClasseDeclarations(out_code,out_h,obj);
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_functions
//       Access: Public, Virtual
//  Description: Generates the list of functions that are appropriate
//               for this interface.  This function is called *before*
//               write_prototypes(), above.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_functions(ostream &out) 
{
  inside_python_native = true;

    out << "//********************************************************************\n";
    out << "//*** Functions for .. Global \n" ;
    out << "//********************************************************************\n";
        Functions::iterator fi;
        for (fi = _functions.begin(); fi != _functions.end(); ++fi) 
        {
            Function *func = (*fi);
            if(!func->_itype.is_global() && isFunctionLegal(func))
                write_function_for_top(out, NULL, func,"");
        }

        Objects::iterator oi;
        for (oi = _objects.begin(); oi != _objects.end(); ++oi) 
        {
            Object *object = (*oi).second;
            if(object->_itype.is_class() ||object->_itype.is_struct())
            {
                if(isCppTypeLegal(object->_itype._cpptype))
                  if(isExportThisRun(object->_itype._cpptype))
                        write_ClasseDetails(out,object);
            }
        }

//    Objects::iterator oi;
    for (oi = _objects.begin(); oi != _objects.end(); ++oi) 
    {
        Object *object = (*oi).second;
        if(!object->_itype.get_outer_class())
        {
            if(object->_itype.is_class() ||object->_itype.is_struct())
              if(isCppTypeLegal(object->_itype._cpptype))
                  if(isExportThisRun(object->_itype._cpptype))
                        write_module_class(out,object);
        }
    }
  inside_python_native = true;

}
////////////////////////////////////////////////////////////
//  Function : write_ClasseDetails
////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_ClasseDetails(ostream &out, Object * obj) 
{
    Functions::iterator fi;

    //std::string cClassName = obj->_itype.get_scoped_name();
    std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
    std::string cClassName =  obj->_itype.get_true_name();
    std::string export_class_name = classNameFromCppName(obj->_itype.get_name());

    out << "//********************************************************************\n";
    out << "//*** Functions for .. "<< cClassName <<" \n" ;
    out << "//********************************************************************\n";

    for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) 
    {
        Function *func = (*fi);
        if( (func))
        {
          SlottedFunctionDef def;
          get_slotted_function_def(obj, func, def);

          ostringstream GetThis;
          GetThis << "    "<<cClassName  << " * local_this = NULL;\n";
          GetThis << "    DTOOL_Call_ExtractThisPointerForType(self,&Dtool_"<<  ClassName<<",(void **)&local_this);\n";
          GetThis << "    if(local_this == NULL) {\n";
          if (def._wrapper_type == WT_numeric_operator) {
            // WT_numeric_operator means we must return NotImplemented, instead
            // of raising an exception, if the this pointer doesn't
            // match.  This is for things like __sub__, which Python
            // likes to call on the wrong-type objects.
            GetThis << "       Py_INCREF(Py_NotImplemented);\n";
            GetThis << "       return Py_NotImplemented;\n";

          } else {
            // Other functions should raise an exception if the this
            // pointer isn't set or is the wrong type.
            GetThis << "       PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
            GetThis << "       return NULL;\n";
          }
          GetThis << "    }\n";
          write_function_for_top(out, obj, func,GetThis.str());
        }
    }

//    bool AnyLeganConstructors;

    if(obj->_constructors.size() == 0)
    {
        std::string fname =     "int  Dtool_Init_"+ClassName+"(PyObject *self, PyObject *args, PyObject *kwds)";
        out << fname << "\n";
        out << "{\n";
        out << "       PyErr_SetString(PyExc_TypeError, \"Error Can Not Init Constant Class (" << cClassName << ")\");\n";
        out << "       return -1;\n" ;
        out << "}\n";

    }
    else
    {
        for (fi = obj->_constructors.begin(); fi != obj->_constructors.end(); ++fi) 
        {
            Function *func = (*fi);
            std::string fname =     "int  Dtool_Init_"+ClassName+"(PyObject *self, PyObject *args, PyObject *kwds) ";

            write_function_for_name(out, obj, func,fname,"",ClassName);
        }
    }

    MakeSeqs::iterator msi;
    for (msi = obj->_make_seqs.begin(); msi != obj->_make_seqs.end(); ++msi) {
      write_make_seq(out, obj, ClassName, *msi);
    }

    CPPType *cpptype = TypeManager::resolve_type(obj->_itype._cpptype);
    std::map< string ,CastDetails > details;
    std::map< string ,CastDetails >::iterator di;
    builder.get_type(TypeManager::unwrap(cpptype),false);
    GetValideChildClasses(details, cpptype->as_struct_type());
    for(di = details.begin(); di != details.end(); di++)
      {
        //InterrogateType ptype =idb->get_type(di->first);
        if(di->second._is_legal_py_class && !isExportThisRun(di->second._structType))
          _external_imports.insert(make_safe_name(di->second._to_class_name));
        //out << "IMPORT_THIS struct   Dtool_PyTypedObject Dtool_" << make_safe_name(di->second._to_class_name) <<";\n";
      }
    


    { // the Cast Converter

        out << "inline void  * Dtool_UpcastInterface_"<< ClassName << "(PyObject *self, Dtool_PyTypedObject *requested_type)\n";
        out << "{\n";
        out << "    Dtool_PyTypedObject *SelfType = ((Dtool_PyInstDef *)self)->_My_Type;\n";
        out << "    if(SelfType != &Dtool_" << ClassName <<")\n";
        out << "    {\n";
        out << "        printf(\""<<ClassName<<" ** Bad Source Type-- Requesting Conversion from %s to %s\\n\",((Dtool_PyInstDef *)self)->_My_Type->_name,requested_type->_name);fflush(NULL);\n";;
        out << "        return NULL;\n";
        out << "    }\n";
        out << " \n";
        out << "    "<<cClassName<<" * local_this = (" << cClassName<<" *)((Dtool_PyInstDef *)self)->_ptr_to_object;\n"; 
        out << "    if(requested_type == &Dtool_"<<ClassName<<")\n";
        out << "        return local_this;\n";

        for(di = details.begin(); di != details.end(); di++)
        {
            if(di->second._is_legal_py_class)
            {
                    out << "    if(requested_type == &Dtool_"<<make_safe_name(di->second._to_class_name)<<")\n";
                    out << "        return "<< di->second._up_cast_string << " local_this;\n";
            }
        }

        out << "    return NULL;\n";
        out << "}\n";

        out << "inline void  * Dtool_DowncastInterface_"<< ClassName << "(void *from_this, Dtool_PyTypedObject *from_type)\n";
        out << "{\n";
        out << "    if(from_this == NULL || from_type == NULL)\n";
        out << "        return NULL;\n";
        out << "    if(from_type == &Dtool_" << ClassName<<")\n";
        out << "        return from_this;\n";
        for(di = details.begin(); di != details.end(); di++)
        {
            if(di->second._can_downcast && di->second._is_legal_py_class)
            {
                out << "    if(from_type == &Dtool_"<<make_safe_name(di->second._to_class_name)<<")\n";
                out << "    {\n";
                out << "          "<< di->second._to_class_name << "* other_this = ("<< di->second._to_class_name <<  "*)from_this;\n" ;
                out << "          return ("<< cClassName << "*)other_this;\n";
                out << "    }\n";
            }
        }
        out << "    return (void *) NULL;\n";
        out << "}\n";

    }

}
////////////////////////////////////////////////////////////
/// Function : write_ClasseDeclarations
//
//
////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_ClasseDeclarations(ostream &out, ostream *out_h,Object * obj ) 
{

    const InterrogateType &itype = obj->_itype;
    std::string class_name = make_safe_name(obj->_itype.get_scoped_name());
    std::string c_class_name =  itype.get_true_name();
    std::string class_struct_name = std::string(CLASS_PREFEX) +class_name;

    out << "typedef  "<< c_class_name <<"  "<< class_name <<"_localtype;\n";
    if(obj->_itype.has_destructor() ||
       obj->_itype.destructor_is_inherited())
    {
        if(TypeManager::is_reference_count(obj->_itype._cpptype))
        {
            out << "Define_Module_ClassRef("<< _def->module_name<<"," << class_name << "," << class_name <<"_localtype,"<< classNameFromCppName(c_class_name) <<");\n";
        }
        else
        {
            out << "Define_Module_Class("<<_def->module_name << "," << class_name << "," <<class_name <<"_localtype,"<< classNameFromCppName(c_class_name) <<");\n";
        }
    }
    else
    {
        if(TypeManager::is_reference_count(obj->_itype._cpptype))
        {
            out << "Define_Module_ClassRef_Private("<<_def->module_name << "," << class_name << "," << class_name <<"_localtype,"<< classNameFromCppName(c_class_name) <<");\n";
        }
        else
        {
            out << "Define_Module_Class_Private("<<_def->module_name<< "," << class_name << "," << class_name <<"_localtype,"<< classNameFromCppName(c_class_name) << ");\n";
        }
    }


    if(out_h != NULL)
        *out_h  << "extern \"C\" " << EXPORT_IMPORT_PREFEX << " struct   Dtool_PyTypedObject Dtool_" << class_name <<";\n";
}
////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_module
//       Access: Public, Virtual
//  Description: Generates whatever additional code is required to
//               support a module file.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_sub_module(ostream &out, Object *obj) 
{
        //Object * obj =  _objects[_embeded_index] ;
        std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
    out << "//********************************************************************\n";
    out << "//*** Module Init Updcall .." << obj->_itype.get_scoped_name() << "\n";
    out << "//********************************************************************\n";
        out << "   Dtool_PyModuleClassInit_"<< ClassName <<"(module);\n";
}
/////////////////////////////////////////////////////////////////////////////
// Function : write_module_support
/////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_module_support(ostream &out,ostream *out_h,InterrogateModuleDef *moduledefdef)
{
    out << "//********************************************************************\n";
    out << "//*** Module Object Linker .. \n";
    out << "//********************************************************************\n";

    out << "static void BuildInstants(PyObject * module)\n";
    out << "{\n";
    Objects::iterator oi;
    for (oi = _objects.begin(); oi != _objects.end(); ++oi) 
    {
        Object *object = (*oi).second;
        if(!object->_itype.get_outer_class())
        {
            if(object->_itype.is_enum())
            {
                int enum_count = object->_itype.number_of_enum_values();
                if(enum_count > 0)
                {
                    out << "//********************************************************************\n";
                    out << "//*** Module Enums  .." << object->_itype.get_scoped_name() << "\n";
                    out << "//********************************************************************\n";
                }
                for(int xx = 0; xx< enum_count; xx++)
                    out << "   PyModule_AddIntConstant(module,\"" << classNameFromCppName(object->_itype.get_enum_value_name(xx)) <<"\","<<  object->_itype.get_enum_value(xx) << ");\n";
            }
        }
    }

     InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
    int num_manifests = idb->get_num_global_manifests();
    for (int mi = 0; mi < num_manifests; mi++) 
    {
        ManifestIndex manifest_index = idb->get_global_manifest(mi);
        const InterrogateManifest &iman = idb->get_manifest(manifest_index);
        if (iman.has_getter()) 
        {
            FunctionIndex func_index = iman.get_getter();
            record_function(dummy_type, func_index);
        }
        if(iman.has_int_value())
                    out << "   PyModule_AddIntConstant(module,\"" << classNameFromCppName(iman.get_name()) <<"\","<<  iman.get_int_value() << ");\n";
        else
                    out << "   PyModule_AddStringConstant(module,\"" << classNameFromCppName(iman.get_name()) <<"\",\""<<  iman.get_definition().c_str() << "\");\n";

    }    



    for (oi = _objects.begin(); oi != _objects.end(); ++oi) 
    {
        Object *object = (*oi).second;
        if(!object->_itype.get_outer_class())
        {
            if(object->_itype.is_class() ||object->_itype.is_struct())
              if(isCppTypeLegal(object->_itype._cpptype))
                 if(isExportThisRun(object->_itype._cpptype))
                    write_sub_module(out,object);
        }
    }
    out << "//********************************************************************\n";
    out << "//*** Module Init Updcall ..  Externally Defined Class\n";
    out << "//********************************************************************\n";

//    for(std::set< std::string >::iterator ii = _external_imports.begin(); ii != _external_imports.end(); ii++)
//                 out << "Dtool_" <<*ii <<"._Dtool_ClassInit(NULL);\n";



    out << "}\n";

    bool force_base_functions = true;

    out << "static PyMethodDef python_simple_funcs[] = {\n";
    Functions::iterator fi;
    for (fi = _functions.begin(); fi != _functions.end(); ++fi) 
    {
        Function *func = (*fi);
        if(!func->_itype.is_global() && isFunctionLegal(func))
        {
            {
                out << "  { \"" << methodNameFromCppName(func,"") << "\", (PyCFunction) &" 
                    << func->_name << ", METH_VARARGS| METH_KEYWORDS, (char *)" << func->_name << "_comment},\n";
            }
        }
    }  

    if(force_base_functions)
    {
        out << "  //Support Function For Dtool_types ... for now in each module ??\n";
        out << "  {\"Dtool_BorrowThisReference\", &Dtool_BorrowThisReference,METH_VARARGS,\"Used to borrow 'this' poiner ( to, from)\\n Assumes no ownership\"}, \n"; 
        out << "  {\"Dtool_AddToDictionary\", &Dtool_AddToDictionary,METH_VARARGS,\"Used to Items Into a types (tp_dict)\"}, \n"; 
    }

    out << "  { NULL, NULL ,0,NULL}\n" << "};\n\n";

    out << "struct LibrayDef " << moduledefdef->library_name <<"_moddef = {python_simple_funcs,BuildInstants};\n";
    if(out_h != NULL)
        *out_h << "extern struct LibrayDef " << moduledefdef->library_name <<"_moddef;\n";
}
/////////////////////////////////////////////////////////////////////////////
///// Function : write_module
/////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_module(ostream &out,ostream *out_h, InterrogateModuleDef *moduledefdef) 
{
    InterfaceMakerPython::write_module(out,out_h, moduledefdef);
    Objects::iterator oi;

    out << "//********************************************************************\n";
    out << "//*** Py Init Code For .. GlobalScope\n" ;
    out << "//********************************************************************\n";

    out << "#ifdef _WIN32\n"
        << "extern \"C\" __declspec(dllexport) void init" << moduledefdef->module_name << "();\n"
        << "#else\n"
        << "extern \"C\" void init" << moduledefdef->module_name << "();\n"
        << "#endif\n\n";

    out << "void init" << moduledefdef->module_name << "() {\n";
    out << "    LibrayDef * refs[] = {&" << moduledefdef->library_name << "_moddef,NULL};\n";
    out << "    Dtool_PyModuleInitHelper(refs,\"" << moduledefdef->module_name << "\");\n";
    out << "}\n\n";
}
/////////////////////////////////////////////////////////////////////////////////////////////
// Function :write_module_class
/////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_module_class(ostream &out,  Object *obj) {
  bool has_local_hash = false;
  bool has_local_repr = false;
  bool has_local_str = false;

  {
    int num_nested = obj->_itype.number_of_nested_types();
    for (int ni = 0; ni < num_nested; ni++)
      {
        TypeIndex nested_index = obj->_itype.get_nested_type(ni);
        Object * nested_obj =  _objects[nested_index];
        if(nested_obj->_itype.is_class() ||nested_obj->_itype.is_struct())
          {
            write_module_class(out,nested_obj);
          }
      }
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName =  obj->_itype.get_true_name();
  std::string export_calss_name = classNameFromCppName(obj->_itype.get_name());

  Functions::iterator fi;
  out << "//********************************************************************\n";
  out << "//*** Py Init Code For .. "<< ClassName <<" | " << export_calss_name <<"\n" ;
  out << "//********************************************************************\n";
  out << "PyMethodDef Dtool_Methods_"<< ClassName << "[]= {\n";



  std::map<int , Function * > static_functions;
  std::map<Function *, std::string >       normal_Operator_functions;
  std::map<Function *, SlottedFunctionDef> wraped_Operator_functions;
  // function Table
  int x;
  for (x = 0, fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi,x++) {
    Function *func = (*fi);
    SlottedFunctionDef slotted_def;
    if (!get_slotted_function_def(obj, func, slotted_def)) {
      out << "  { \"" << methodNameFromCppName(func,export_calss_name) << "\",(PyCFunction ) &" 
          << func->_name << ", METH_VARARGS| METH_KEYWORDS, (char *)" << func->_name << "_comment},\n";
      if(!isFunctionWithThis(func)) {
        static_functions[x] = func;
      }
        
    } else {
      if (slotted_def._wrapper_type != WT_none) {
        wraped_Operator_functions[func] = slotted_def;
        
        out << "  { \"" << methodNameFromCppName(func,export_calss_name) << "\",(PyCFunction ) &" 
            << func->_name << ", METH_VARARGS| METH_KEYWORDS, (char *)" << func->_name << "_comment},\n";
        if (!isFunctionWithThis(func)) {
          static_functions[x] = func;
        }
        
      } else {
        normal_Operator_functions[func] = slotted_def._answer_location;
        
        out << "  { \"" << methodNameFromCppName(func,export_calss_name) << "\",(PyCFunction ) &" 
            << func->_name << ", METH_VARARGS| METH_KEYWORDS, (char *)" << func->_name << "_comment},\n";
        if (!isFunctionWithThis(func)) {
          static_functions[x] = func;
        }
      }
    }
  }

  MakeSeqs::iterator msi;
  for (msi = obj->_make_seqs.begin(); msi != obj->_make_seqs.end(); ++msi) {
    out << "  { \""
        << methodNameFromCppName((*msi)->_seq_name, export_calss_name)
        << "\",(PyCFunction) &" << (*msi)->_name << ", METH_NOARGS, NULL},\n";
  }

  out << "  { NULL, NULL }\n"
      << "};\n\n";

  int num_derivations = obj->_itype.number_of_derivations();
  int di;
  for (di = 0; di < num_derivations; di++) 
    {
      TypeIndex d_type_Index = obj->_itype.get_derivation(di);
      if(!interrogate_type_is_unpublished(d_type_Index))
        {
          const InterrogateType &d_itype = idb->get_type(d_type_Index);    
          if(isCppTypeLegal(d_itype._cpptype))
            {
              if(!isExportThisRun(d_itype._cpptype))
                {
                  _external_imports.insert(make_safe_name(d_itype.get_scoped_name().c_str()));

                  //out << "IMPORT_THIS struct   Dtool_PyTypedObject Dtool_" << make_safe_name(d_itype.get_scoped_name().c_str()) <<";\n";
                }
            }
        }
    }

  std::vector< std::string >  bases;
  for (di = 0; di < num_derivations; di++) 
    {
      TypeIndex d_type_Index = obj->_itype.get_derivation(di);
      if(!interrogate_type_is_unpublished(d_type_Index))
        {

          const InterrogateType &d_itype = idb->get_type(d_type_Index);    
          if(isCppTypeLegal(d_itype._cpptype))
            {
              bases.push_back(make_safe_name(d_itype.get_scoped_name().c_str()));
            }
        }
    }

  if(bases.empty())
    bases.push_back("DTOOL_SUPPER_BASE");


  {
    std::map<Function *, SlottedFunctionDef>::iterator rfi; //          wraped_Operator_functions;
    for(rfi = wraped_Operator_functions.begin(); rfi != wraped_Operator_functions.end(); rfi++) {
      switch (rfi->second._wrapper_type) {
      case WT_no_params:
        // PyObject *func(PyObject *self)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static PyObject * " <<  func->_name << methodNameFromCppName(func,export_calss_name) << "( PyObject * self)\n";
          out << "{\n";
          out << "    PyObject *args = Py_BuildValue(\"()\");\n";
          out << "    PyObject *result = " << func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    return result;\n";
          out << "}\n\n";
        }
        break;
        
      case WT_one_param:
      case WT_numeric_operator:
        // PyObject *func(PyObject *self, PyObject *one)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func,export_calss_name) << "(PyObject *self, PyObject *one)\n";
          out << "{\n";
          out << "    PyObject *args = Py_BuildValue(\"(O)\", one);\n";
          out << "    PyObject *result = " << func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    return result;\n";
          out << "}\n\n";
        }
        break;

      case WT_setattr:
        // int func(PyObject *self, PyObject *one, PyObject *two = NULL)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static int " <<  func->_name << methodNameFromCppName(func,export_calss_name) << "( PyObject * self, PyObject * one, PyObject * two)\n";
          out << "{\n";
          out << "    PyObject *args;\n";
          out << "    if (two == NULL) {\n";
          out << "        args = Py_BuildValue(\"(O)\", one);\n";
          out << "    } else {\n";
          out << "        args = Py_BuildValue(\"(OO)\", one, two);\n";
          out << "    }\n";
          out << "    PyObject *py_result = " << func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    if (py_result == NULL) return -1;\n";
          out << "    int result = PyInt_AsLong(py_result);\n";
          out << "    Py_DECREF(py_result);\n";
          out << "    return result;\n";
          out << "}\n\n";
        }
        break;

      case WT_getattr:
        // PyObject *func(PyObject *self, PyObject *one)
        // Specifically to implement __getattr__.
        // With special handling to pass up to
        // PyObject_GenericGetAttr() if it returns NULL.
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static PyObject * " <<  func->_name << methodNameFromCppName(func,export_calss_name) << "( PyObject * self, PyObject * one)\n";
          out << "{\n";
          out << "    PyObject *args = Py_BuildValue(\"(O)\", one);\n";
          out << "    PyObject *result = " << func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    if (result == NULL) {\n";
          out << "        PyErr_Clear();\n";
          out << "        return PyObject_GenericGetAttr(self, one);\n";
          out << "    }\n";
          out << "    return result;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_getitem:
        // PyObject *func(PyObject *self, Py_ssize_t index)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static PyObject * " <<  func->_name << methodNameFromCppName(func,export_calss_name) << "( PyObject * self, Py_ssize_t index)\n";
          out << "{\n";
          out << "    PyObject *args = Py_BuildValue(\"(i)\", index);\n";
          out << "    PyObject *result = " << func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    return result;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_setitem:
        // int_t func(PyObject *self, Py_ssize_t index, PyObject *value)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static int " <<  func->_name << methodNameFromCppName(func,export_calss_name) << "( PyObject * self, Py_ssize_t index, PyObject *value)\n";
          out << "{\n";
          out << "    PyObject *args = Py_BuildValue(\"(iO)\", index, value);\n";
          out << "    PyObject *result = " << func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    if (result == NULL) {\n";
          out << "      return -1;\n";
          out << "    }\n";
          out << "    Py_DECREF(result);\n";
          out << "    return 0;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_size:
        // Py_ssize_t func(PyObject *self)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static Py_ssize_t " <<  func->_name << methodNameFromCppName(func,export_calss_name) << "(PyObject *self)\n";
          out << "{\n";
          out << "    PyObject *args = Py_BuildValue(\"()\");\n";
          out << "    PyObject *result = "<< func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    if (result == NULL) {\n";
          out << "      return -1;\n";
          out << "    }\n";
          out << "    Py_ssize_t num = PyInt_AsSsize_t(result);\n";
          out << "    Py_DECREF(result);\n";
          out << "    return num;\n";
          out << "}\n\n";
        }
        break;

      case WT_mapping_setitem:
        // int func(PyObject *self, PyObject *one, PyObject *two)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static int " <<  func->_name << methodNameFromCppName(func,export_calss_name) << "( PyObject * self, PyObject * one, PyObject * two)\n";
          out << "{\n";
          out << "    PyObject *args = Py_BuildValue(\"(OO)\", one, two);\n";
          out << "    PyObject *result = " << func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    if (result == NULL) {\n";
          out << "      return -1;\n";
          out << "    }\n";
          out << "    Py_DECREF(result);\n";
          out << "    return 0;\n";
          out << "}\n\n";
        }
        break;
        
      case WT_inquiry:
        // int func(PyObject *self)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions. \n";
          out << "//     " <<ClassName<< " ..." << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func,export_calss_name) << "(PyObject *self)\n";
          out << "{\n";
          out << "    PyObject *args = Py_BuildValue(\"()\");\n";
          out << "    PyObject *result = " << func->_name <<"(self, args, NULL);\n";
          out << "    Py_DECREF(args);\n";
          out << "    if (result == NULL) {\n";
          out << "      return -1;\n";
          out << "    }\n";
          out << "    int iresult = PyInt_AsLong(result);\n";
          out << "    Py_DECREF(result);\n";
          out << "    return iresult;\n";
          out << "}\n\n";
        }
        break;

      case WT_none:
        break;
      }
    }

    if(HasAGetKeyFunction(obj->_itype)) 
      {
        out << "//////////////////\n";
        out << "//  A LocalHash(getKey) Function for this type\n";
        out << "//     " <<ClassName << "\n";
        out << "//////////////////\n";
        out << "static long  DTool_HashKey_"<<ClassName << "(PyObject * self)\n";
        out << "{\n";
        out << "    "<<cClassName  << " * local_this = NULL;\n";
        out << "    DTOOL_Call_ExtractThisPointerForType(self,&Dtool_"<<  ClassName<<",(void **)&local_this);\n";
        out << "    if(local_this == NULL)\n";
        out << "    {\n";
        out << "       PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
        out << "       return -1;\n";
        out << "    };\n";
        out << "    return local_this->get_key();\n";
        out << "}\n\n";
        has_local_hash = true;
      }
    else
      {
        if(bases.size() == 0)
          {
            out << "//////////////////\n";
            out << "//  A LocalHash(This Pointer) Function for this type\n";
            out << "//     " <<ClassName << "\n";
            out << "//////////////////\n";
            out << "static long  DTool_HashKey_"<<ClassName << "(PyObject * self)\n";
            out << "{\n";
            out << "    "<<cClassName  << " * local_this = NULL;\n";
            out << "    DTOOL_Call_ExtractThisPointerForType(self,&Dtool_"<<  ClassName<<",(void **)&local_this);\n";
            out << "    if(local_this == NULL)\n";
            out << "    {\n";
            out << "       PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
            out << "       return -1;\n";
            out << "    };\n";
            out << "    return (long)local_this;\n";
            out << "}\n\n";
            has_local_hash = true;
          }
      }

    int need_repr = NeedsAReprFunction(obj->_itype);
    if(need_repr > 0)
      {
        out << "//////////////////\n";
        out << "//  A __repr__ Function\n";
        out << "//     " <<ClassName << "\n";
        out << "//////////////////\n";
        out << "static PyObject *  Dtool_Repr_"<<ClassName << "(PyObject * self)\n";
        out << "{\n";
        out << "    "<<cClassName  << " * local_this = NULL;\n";
        out << "    DTOOL_Call_ExtractThisPointerForType(self,&Dtool_"<<  ClassName<<",(void **)&local_this);\n";
        out << "    if(local_this == NULL)\n";
        out << "    {\n";
        out << "       PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
        out << "       return NULL;\n";
        out << "    };\n";
        out << "     ostringstream os;\n";
        if (need_repr == 2) {
          out << "     local_this->output(os);\n";
        } else {
          out << "     local_this->python_repr(os, \""
              << classNameFromCppName(ClassName) << "\");\n";
        }
        out << "     std::string ss = os.str();\n";
        out << "     return PyString_FromStringAndSize(ss.data(),ss.length());\n";
        out << "}\n";   
        has_local_repr = true;
      }

    int need_str = NeedsAStrFunction(obj->_itype);
    if(need_str > 0)
      {
        out << "//////////////////\n";
        out << "//  A __str__ Function\n";
        out << "//     " <<ClassName << "\n";
        out << "//////////////////\n";
        out << "static PyObject *  Dtool_Str_"<<ClassName << "(PyObject * self)\n";
        out << "{\n";
        out << "    "<<cClassName  << " * local_this = NULL;\n";
        out << "    DTOOL_Call_ExtractThisPointerForType(self,&Dtool_"<<  ClassName<<",(void **)&local_this);\n";
        out << "    if(local_this == NULL)\n";
        out << "    {\n";
        out << "       PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
        out << "       return NULL;\n";
        out << "    };\n";
        out << "     ostringstream os;\n";
        if(need_str == 2)
          out << "     local_this->write(os,0);\n";
        else
          out << "     local_this->write(os);\n";
        out << "     std::string ss = os.str();\n";
        out << "     return PyString_FromStringAndSize(ss.data(),ss.length());\n";
        out << "}\n";   
        has_local_str = true;
      }


  }




  out << "void   Dtool_PyModuleClassInit_" << ClassName << "(PyObject *module)\n";
  out << "{\n";
  out << "    static bool initdone = false;\n";
  out << "    if(!initdone)\n";
  out << "    {\n";
  out << "        initdone = true;\n";
  //        out << "        memset(Dtool_"<< ClassName << ".As_PyTypeObject().tp_as_number,0,sizeof(PyNumberMethods));\n";
  //        out << "        memset(Dtool_"<< ClassName << ".As_PyTypeObject().tp_as_mapping,0,sizeof(PyMappingMethods));\n";
  //        out << "        static Dtool_PyTypedObject  *InheritsFrom[] = {";

  // add doc string 
  if (obj->_itype.has_comment()) {
    out << "#ifndef NDEBUG\n";
    out << "        // Class documentation string\n";
    out << "        Dtool_" << ClassName 
        << ".As_PyTypeObject().tp_doc =\n";
    output_quoted(out, 10, obj->_itype.get_comment());
    out << ";\n"
        << "#endif\n";
  }

  // add bases///
  if(bases.size() > 0)
    {
      out << "        // Dependent Objects   \n";
      std::string format1= "";
      std::string format2= "";
      for(std::vector< std::string >::iterator bi = bases.begin(); bi != bases.end(); bi++)
        {
          format1 += "O";
          format2 += ",&Dtool_" + *bi + ".As_PyTypeObject()";
          out << "        Dtool_"<< make_safe_name(*bi) << "._Dtool_ClassInit(NULL);\n";
        }

      out << "        Dtool_"<<ClassName<<".As_PyTypeObject().tp_bases = Py_BuildValue(\"(" << format1 << ")\""<< format2 << ");\n";           
    }

  // get dictionary
  out << "        Dtool_" << ClassName << ".As_PyTypeObject().tp_dict = PyDict_New();\n";
  out << "        PyDict_SetItemString(Dtool_"<<ClassName <<".As_PyTypeObject().tp_dict,\"DtoolClassDict\",Dtool_"<<ClassName <<".As_PyTypeObject().tp_dict);\n";
        


  // the standard call functions
  std::map<Function *, std::string >::iterator ofi;
  for(ofi = normal_Operator_functions.begin(); ofi != normal_Operator_functions.end(); ofi++)
    {
      Function *func = ofi->first;
      out << "        // " << ofi->second <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
      out << "        Dtool_" << ClassName <<".As_PyTypeObject()." << ofi->second <<" = &" << func->_name <<";\n";
    }


  // wraped functions...
  {
    std::map<Function *, SlottedFunctionDef>::iterator rfi; //          wraped_Operator_functions;
    for(rfi = wraped_Operator_functions.begin(); rfi != wraped_Operator_functions.end(); rfi++)
      {
        Function *func = rfi->first;
        out << "        // " << rfi->second._answer_location <<" = "<< methodNameFromCppName(func,export_calss_name) <<"\n";
        out << "        Dtool_" << ClassName <<".As_PyTypeObject()." << rfi->second._answer_location <<" = &" << func->_name << methodNameFromCppName(func,export_calss_name)<<";\n";
      }
  }

  // compare and hash work together in PY inherit behavior hmm grrr
  // __hash__
  if(has_local_hash == true)
    {
      out << "        // __hash__\n";
      out << "        Dtool_" << ClassName <<".As_PyTypeObject().tp_hash = &DTool_HashKey_"<<ClassName <<";\n";
      out << "        Dtool_" << ClassName <<".As_PyTypeObject().tp_compare = &DTOOL_PyObject_Compare;\n";
            

    }

  if(has_local_repr == true)
    {
      out << "        // __repr__\n";
      out << "        Dtool_" << ClassName <<".As_PyTypeObject().tp_repr = & Dtool_Repr_"<<ClassName <<";\n";
    }

  if(has_local_str == true)
    {
      out << "        // __str__\n";
      out << "        Dtool_" << ClassName <<".As_PyTypeObject().tp_str = & Dtool_Str_"<<ClassName <<";\n";

    }
  else if(has_local_repr == true)
    {
      out << "        // __str__ Repr Proxy\n";
      out << "        Dtool_" << ClassName <<".As_PyTypeObject().tp_str = & Dtool_Repr_"<<ClassName <<";\n";
    }



  int num_nested = obj->_itype.number_of_nested_types();
  for (int ni = 0; ni < num_nested; ni++)
    {
      TypeIndex nested_index = obj->_itype.get_nested_type(ni);
      Object * nested_obj =  _objects[nested_index];
      if(nested_obj->_itype.is_class() ||nested_obj->_itype.is_struct())
        {
          std::string ClassName1 = make_safe_name(nested_obj->_itype.get_scoped_name());
          std::string ClassName2 = make_safe_name(nested_obj->_itype.get_name());
          out << "        // Nested Object   "<< ClassName1 << ";\n";
          out << "        Dtool_" << ClassName1 << "._Dtool_ClassInit(NULL);\n";
          out << "        PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict,\"" << classNameFromCppName(ClassName2) <<"\",(PyObject *)&Dtool_" << ClassName1 << ".As_PyTypeObject());\n";
        }  
      else
        {
          if(nested_obj->_itype.is_enum())
            {
              out << "        // Enum  "<< nested_obj->_itype.get_scoped_name() << ";\n";
              int enum_count = nested_obj->_itype.number_of_enum_values();
              for(int xx = 0; xx< enum_count; xx++)
                out << "        PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict,\"" << classNameFromCppName(nested_obj->_itype.get_enum_value_name(xx)) <<"\",PyInt_FromLong("<<  nested_obj->_itype.get_enum_value(xx) << "));\n";

            }
        }
    }

  out << "        if(PyType_Ready(&Dtool_"<< ClassName << ".As_PyTypeObject()) < 0)\n";
  out << "        {\n";
  out << "             PyErr_SetString(PyExc_TypeError, \"PyType_Ready("<< ClassName << ")\");\n"; 
  out << "             printf(\" Error In PyType_Ready" << ClassName << "\");\n";
  out << "             return;\n";
  out << "        }\n";

  out << "        Py_INCREF(&Dtool_"<< ClassName << ".As_PyTypeObject());\n";
  out << "        PyDict_SetItemString(Dtool_"<<ClassName <<".As_PyTypeObject().tp_dict,\""<<export_calss_name<< "\",&Dtool_"<<ClassName <<".As_PyObject());\n";

  // static function into dictionary with bogus self..
  //
  std::map<int , Function * >::iterator sfi;
  for(sfi= static_functions.begin(); sfi != static_functions.end(); sfi++)
    {
      out << "        //  Static Method " << methodNameFromCppName(sfi->second,export_calss_name) << "\n";
      out << "        PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict,\"" ;
      out << methodNameFromCppName(sfi->second,export_calss_name) ;
      out << "\",PyCFunction_New(&Dtool_Methods_"<< ClassName <<"[" << sfi->first << "],&Dtool_"<< ClassName<< ".As_PyObject()));\n";
    }


  bool is_runtime_typed = IsPandaTypedObject(obj->_itype._cpptype->as_struct_type());
  if (HasAGetClassTypeFunction(obj->_itype)) {
    is_runtime_typed = true;
  }

  if(is_runtime_typed)
    out << "        RegisterRuntimeClass(&Dtool_"<<ClassName<<","<< cClassName <<"::get_class_type().get_index());\n";
  else
    out << "        RegisterRuntimeClass(&Dtool_"<<ClassName<<",-1);\n";

  out << "    }\n";

  out << "    if(module != NULL)\n";
  out << "    {\n";
  out << "        Py_INCREF(&Dtool_"<< ClassName << ".As_PyTypeObject());\n";
  out << "        PyModule_AddObject(module, \""<<export_calss_name<<"\",(PyObject *)&Dtool_"<< ClassName << ".As_PyTypeObject());\n";
  out << "    }\n";
  out << "}\n";

}
////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::synthesize_this_parameter
//       Access: Public, Virtual
//  Description: This method should be overridden and redefined to
//               return true for interfaces that require the implicit
//               "this" parameter, if present, to be passed as the
//               first parameter to any wrapper functions.
////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::
synthesize_this_parameter() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::get_wrapper_prefix
//       Access: Protected, Virtual
//  Description: Returns the prefix string used to generate wrapper
//               function names.
////////////////////////////////////////////////////////////////////
string InterfaceMakerPythonNative::
get_wrapper_prefix() {
  return "Dtool_";
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::get_unique_prefix
//       Access: Protected, Virtual
//  Description: Returns the prefix string used to generate unique
//               symbolic names, which are not necessarily C-callable
//               function names.
////////////////////////////////////////////////////////////////////
string InterfaceMakerPythonNative::get_unique_prefix() 
{
  return "Dtool_";
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::record_function_wrapper
//       Access: Protected, Virtual
//  Description: Associates the function wrapper with its function in
//               the appropriate structures in the database.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::record_function_wrapper(InterrogateFunction &ifunc, FunctionWrapperIndex wrapper_index) 
{
  ifunc._python_wrappers.push_back(wrapper_index);
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_prototype_for
//       Access: Private
//  Description: Writes the prototype for the indicated function.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_prototype_for(ostream &out, InterfaceMaker::Function *func) 
{
    std::string fname =     "PyObject *"+func->_name+"(PyObject *self, PyObject *args)";


    write_prototype_for_name(out,func,fname);
}
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_prototype_for_name(ostream &out, InterfaceMaker::Function *func, const std::string &function_namename) 
{
  Function::Remaps::const_iterator ri;

//  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
//    FunctionRemap *remap = (*ri);
    if (!output_function_names) {
      // If we're not saving the function names, don't export it from
      // the library.
      out << "static ";
    } else {
      out << "extern \"C\" ";
    }
    out << function_namename <<";\n";
//  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_function_for
//       Access: Private
//  Description: Writes the definition for a function that will call
//               the indicated C++ function or method.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::write_function_for_top(ostream &out, InterfaceMaker::Object *obj, InterfaceMaker::Function *func, const std::string &PreProcess)
{
    std::string fname =     "static PyObject *"+func->_name+"(PyObject *self, PyObject *args,PyObject *kwds)";

    write_function_for_name(out,obj,func,fname,PreProcess,"");
}

////////////////////////////////////////////////////////////////////
/// Function  : write_function_for_name
//
//   Wrap a complete name override function for Py.....
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_function_for_name(ostream &out1, InterfaceMaker::Object *obj, InterfaceMaker::Function *func, 
                        const std::string &function_name, 
                        const std::string &PreProcess,
                        const std::string &ClassName) {
  ostringstream forward_decl;
  ostringstream out;

  std::map<int , std::set<FunctionRemap *>  >    MapSets;
  std::map<int , std::set<FunctionRemap *>  >::iterator    mii;
  std::set<FunctionRemap *>::iterator                      sii;


  Function::Remaps::const_iterator ri;
  out1 << "/******************************************************************\n" << " * Python type method wrapper for\n";
  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    if (isRemapLegal(*remap)) {
      int parameter_size = remap->_parameters.size();
      if(remap->_has_this && remap->_type != FunctionRemap::T_constructor)
        parameter_size --;
      
      MapSets[parameter_size].insert(remap);
      out1 << " * ";
      remap->write_orig_prototype(out1, 0);
      out1 << "\n";
    } else {
      out1 << " * Rejected Remap [";
      remap->write_orig_prototype(out1, 0);
      out1 << "]\n";
    }
  }

  out1 << " *******************************************************************/\n";

  out << function_name << " {\n";
  if (isFunctionWithThis(func)) {
    out << PreProcess;
  }

  bool is_inplace = isInplaceFunction(func);

  if (MapSets.empty()) {
    return;
  }

  std::string FunctionComment = func->_ifunc._comment;
  std::string FunctionComment1;
  if (FunctionComment.size() > 2) {
    FunctionComment += "\n";
  }

  bool constructor = false;

  if (MapSets.size() > 1) {
    string expected_params;
      
    indent(out,4) << "int parameter_count = 1;\n";
    indent(out,4) << "if(PyTuple_Check(args))\n";
    indent(out,4) << "{\n";
    indent(out,4) << "    parameter_count = PyTuple_Size(args);\n" ;
    indent(out,4) << "    if(kwds != NULL && PyDict_Check(kwds))\n";
    indent(out,4) << "          parameter_count += PyDict_Size(kwds);\n" ;
    indent(out,4) << "}\n";
      
    indent(out,4) << "switch(parameter_count)\n";
    indent(out,4) << "{\n";
    for (mii = MapSets.begin(); mii != MapSets.end(); mii ++) {
      indent(out,4) << "case(" << mii->first << "):\n";
      indent(out,8) << "{\n";
        
      write_function_forset(out,obj,func,mii->second,expected_params,8,forward_decl,ClassName + function_name, is_inplace);
      if ((*mii->second.begin())->_type == FunctionRemap::T_constructor) {
        constructor = true;
      }
        
        
      indent(out,8)<< "}\n";       
      indent(out,8)<< "break;\n";       
    } 
      
    indent(out,4)<< "default:\n";
    indent(out,8)<< "{\n";
    indent(out,12)
      << "PyErr_Format(PyExc_TypeError, \""
      << methodNameFromCppName(func, "")
      << "() takes ";

    // We add one to the parameter count for "self", following the
    // Python convention.
    int add_self = func->_has_this ? 1 : 0;
    size_t mic;
    for (mic = 0, mii = MapSets.begin(); 
         mii != MapSets.end(); 
         ++mii, ++mic) {
      if (mic == MapSets.size() - 1) {
        if (mic == 1) {
          out << " or ";
        } else {
          out << ", or ";
        }
      } else if (mic != 0) {
        out << ", ";
      }

      out << mii->first + add_self;
    }

    out << " arguments (%d given)\", parameter_count + " << add_self << ");\n";
      
    if (constructor)
      indent(out,12) << "return -1;\n";
    else
      indent(out,12) << "return (PyObject *) NULL; \n";
      
    indent(out,8)<< "}\n";
    indent(out,8)<< "break;\n";
    indent(out,4)<< "}\n";
      
    out << "    if(!PyErr_Occurred()) // let error pass on \n";
    out << "        PyErr_SetString(PyExc_TypeError, \n";
    out << "          \"Arguments must match one of:\\n\"\n";
    output_quoted(out, 10, expected_params);
    out << ");\n";
    if (constructor)
      indent(out,4) << "return -1;\n";
    else
      indent(out,4) << "return (PyObject *) NULL; \n";
      
    if(!expected_params.empty() && FunctionComment1.empty())
      FunctionComment1 += "C++ Interface:\n";
      
    FunctionComment1 += expected_params;

  } else {
    string expected_params = "";
    for (mii = MapSets.begin(); mii != MapSets.end(); mii ++) {
      write_function_forset(out,obj,func,mii->second,expected_params,4,forward_decl,ClassName + function_name,is_inplace);
      if((*mii->second.begin())->_type == FunctionRemap::T_constructor) {
        constructor = true;
      }
    }

    out << "    if(!PyErr_Occurred())\n";
    out << "        PyErr_SetString(PyExc_TypeError,\n";
    out << "          \"Must Match :\\n\"\n";
    output_quoted(out, 10, expected_params);
    out << ");\n";
    if (constructor)
      indent(out,4) << "return -1;\n";
    else
      indent(out,4) << "return (PyObject *) NULL; \n";
      
    if (!expected_params.empty() && FunctionComment1.empty()) {
      FunctionComment1 += "C++ Interface:\n";
    }
      
    FunctionComment1 += expected_params;
  }

  out << "}\n\n";

  if (!FunctionComment1.empty()) {
    FunctionComment = FunctionComment1 + "\n" + FunctionComment;
  }

  if (!constructor) {
    // Write out the function doc string.  We only do this if it is
    // not a constructor, since we don't have a place to put the
    // constructor doc string.
    out << "#ifndef NDEBUG\n";
    out << "static const char * " << func->_name << "_comment =\n";
    output_quoted(out, 4, FunctionComment);
    out << ";\n";
    out << "#else\n";
    out << "static const char * " << func->_name << "_comment = NULL;\n";
    out << "#endif\n";
  }

  out << "\n";


  out1 << forward_decl.str();
  out1 << out.str();
}

////////////////////////////////////////////////////////
// Function : GetParnetDepth
//
// Support Function used to Sort the name based overrides.. For know must be complex to simple
////////////////////////////////////////////////////////
int     GetParnetDepth(CPPType  *type)
{
    int answer = 0;
//    printf("    %s\n",type->get_local_name().c_str());

    if (TypeManager::is_basic_string_char(type)) {
    } else if (TypeManager::is_basic_string_wchar(type)) {
    } else if (TypeManager::is_bool(type)) {
    } else if (TypeManager::is_unsigned_longlong(type)) {
    } else if (TypeManager::is_longlong(type)) {
    } else if (TypeManager::is_integer(type)) {
    } else if (TypeManager::is_float(type)) {
    } else if (TypeManager::is_char_pointer(type)) {
    } else if (TypeManager::is_pointer_to_PyObject(type)) {
    } else if (TypeManager::is_pointer(type) ||TypeManager::is_reference(type) || TypeManager::is_struct(type) ) 
    {
        answer ++;
        int deepest = 0;
        TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)),false);
        InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
        const InterrogateType &itype = idb->get_type(type_index);    

        if(itype.is_class() ||itype.is_struct())
        {

            int num_derivations = itype.number_of_derivations();
            for (int di = 0; di < num_derivations; di++) 
            {
                TypeIndex d_type_Index = itype.get_derivation(di);
                const InterrogateType &d_itype = idb->get_type(d_type_Index);    
                int this_one = GetParnetDepth(d_itype._cpptype);
                if(this_one > deepest)
                    deepest = this_one;
            }
        }
        answer += deepest;
//        printf(" Class Name %s  %d\n",itype.get_name().c_str(),answer);
    }


//    printf(" Class Name %s  %d\n",itype.get_name().c_str(),answer);
    return answer;
}
////////////////////////////////////////////////////////
//  The Core sort function for remap calling orders..
//////////////////////////////////////////////////////////
int RemapCompareLesss(FunctionRemap * in1 , FunctionRemap * in2)
{
    if(in1->_parameters.size() != in2->_parameters.size())
        return (in1->_parameters.size() > in2->_parameters.size());
    
    int pcount = in1->_parameters.size();
    for(int x = 0; x< pcount; x++)
    {
        CPPType *orig_type1 = in1->_parameters[x]._remap->get_orig_type();
        CPPType *orig_type2 = in2->_parameters[x]._remap->get_orig_type();

        int pd1 = GetParnetDepth(orig_type1);
        int pd2 = GetParnetDepth(orig_type2);
        if(pd1 != pd2)
            return pd1> pd2;        
    }

    // ok maybe something to do with return strength..

    return false;
}
//////////////////////////////////////////////////////////
//  Convience for the sort behavior..
///////////////////////////////////////////////////////////
std::vector< FunctionRemap * >  SortFunctionSet(std::set< FunctionRemap *> &remaps)
{
    std::vector< FunctionRemap * >  out;
    for(std::set< FunctionRemap *>::iterator ii = remaps.begin(); ii!= remaps.end(); ii++)
        out.push_back(*ii);


    std::sort(out.begin(), out.end(), RemapCompareLesss);

    return out;
}
///////////////////////////////////////////////////////////
//  Function  : write_function_forset
//
//  A set is defined as all remaps that have the same number of paramaters..
///////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_function_forset(ostream &out, InterfaceMaker::Object *obj, 
                      InterfaceMaker::Function *func, 
                      std::set< FunctionRemap *> &remapsin, 
                      string &expected_params, int indent_level, 
                      ostream &forward_decl, const std::string &functionname, 
                      bool is_inplace) {
  // Do we accept any parameters that are class objects?  If so, we
  // might need to check for parameter coercion.
  bool coercion_possible = false;
  std::set<FunctionRemap *>::const_iterator sii;
  for (sii = remapsin.begin(); sii != remapsin.end() && !coercion_possible; ++sii) {
    FunctionRemap *remap = (*sii);
    if (isRemapLegal(*remap)) {
      int pn = 0;
      if (remap->_has_this) {
        // Skip the "this" parameter.  It's never coerceable.
        ++pn;
      }
      while (pn < (int)remap->_parameters.size()) {
        CPPType *type = remap->_parameters[pn]._remap->get_new_type();

        if (TypeManager::is_char_pointer(type)) {
        } else if (TypeManager::is_pointer_to_PyObject(type)) {
        } else if (TypeManager::is_pointer(type)) {
          // This is a pointer to an object, so we
          // might be able to coerce a parameter to it.
          coercion_possible = true;
          break;
        }
        ++pn;
      }
    }
  }

  if (coercion_possible) {
    // These objects keep track of whether we have attempted automatic
    // parameter coercion.
    indent(out, indent_level)
      << "{\n";
    indent_level += 2;
    indent(out,indent_level) 
      << "PyObject *coerced = NULL;\n";
    indent(out,indent_level) 
      << "PyObject **coerced_ptr = NULL;\n";
    indent(out,indent_level) 
      << "bool report_errors = false;\n";
    indent(out,indent_level) 
      << "while (true) {\n";
    indent_level += 2;
  }
    
  if (remapsin.size() > 1) {
    // There are multiple different overloads for this number of
    // parameters.  Sort them all into order from most-specific to
    // least-specific, then try them one at a time.
    std::vector<FunctionRemap *> remaps = SortFunctionSet(remapsin);

    std::vector<FunctionRemap *>::iterator sii;
    for (sii = remaps.begin(); sii != remaps.end(); sii ++) {
      FunctionRemap *remap = (*sii);
      if(isRemapLegal(*remap)) {
        if (remap->_has_this && !remap->_const_method) {
          // If it's a non-const method, we only allow a
          // non-const this.
          indent(out,indent_level) 
            << "if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
        } else {
          indent(out, indent_level)
            << "{\n";
        }

        indent(out,indent_level) << "  // -2 " ;
        remap->write_orig_prototype(out, 0); out << "\n" ;

        write_function_instance(out, obj, func, remap,expected_params,indent_level+4,false,forward_decl, func->_name, is_inplace, coercion_possible);

        indent(out,indent_level+4)<< "PyErr_Clear(); \n";
        indent(out,indent_level)<< "}\n\n";            
      }
    }
  } else {
    // There is only one possible overload with this number of
    // parameters.  Just call it.
    std::set<FunctionRemap *>::iterator sii;
    for(sii = remapsin.begin(); sii != remapsin.end(); sii ++) {
      FunctionRemap *remap = (*sii);
      if (isRemapLegal(*remap)) {
        if (remap->_has_this && !remap->_const_method) {
          // If it's a non-const method, we only allow a
          // non-const this.
          indent(out, indent_level) 
            << "if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
        } else {
          indent(out, indent_level)
            << "{\n";
        }

        indent(out,indent_level + 2) 
          << "// 1-" ;
        remap->write_orig_prototype(out, 0); 
        out << "\n" ;
        write_function_instance(out, obj, func, remap,expected_params,indent_level+4,true,forward_decl, func->_name, is_inplace, coercion_possible);

        if (remap->_has_this && !remap->_const_method) {
          indent(out, indent_level)
            << "} else {\n";
          indent(out, indent_level + 2)
            << "PyErr_SetString(PyExc_TypeError,\n";
          string class_name = remap->_cpptype->get_simple_name();
          indent(out, indent_level + 2)
            << "                \"Cannot call "
            << classNameFromCppName(class_name)
            << "." << methodNameFromCppName(func, class_name)
            << "() on a const object.\");\n";
          indent(out, indent_level + 2)
            << "return (PyObject *) NULL;\n";
          indent(out,indent_level)
            << "}\n\n";
        } else {
          indent(out,indent_level)
            << "}\n\n";            
        }
      }
    }
  }

  // Now we've tried all of the possible overloads, and had no luck.

  if (coercion_possible) {
    // Try again, this time with coercion enabled.
    indent(out,indent_level) 
      << "if (coerced_ptr == NULL && !report_errors) {\n";
    indent(out,indent_level + 2) 
      << "coerced_ptr = &coerced;\n";
    indent(out,indent_level + 2) 
      << "continue;\n";
    indent(out,indent_level) 
      << "}\n";

    // No dice.  Go back one more time, and this time get the error
    // message.
    indent(out,indent_level) 
      << "if (!report_errors) {\n";
    indent(out,indent_level + 2) 
      << "report_errors = true;\n";
    indent(out,indent_level + 2) 
      << "continue;\n";
    indent(out,indent_level) 
      << "}\n";

    // We've been through three times.  We're done.
    indent(out,indent_level) 
      << "break;\n";
    
    indent_level -= 2;
    indent(out,indent_level) 
      << "}\n";
    
    indent(out,indent_level) 
      << "Py_XDECREF(coerced);\n";
    indent_level -= 2;
    indent(out,indent_level) 
      << "}\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_function_instance
//       Access: Private
//  Description: Writes out the particular function that handles a
//               single instance of an overloaded function.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_function_instance(ostream &out, InterfaceMaker::Object *obj, 
                        InterfaceMaker::Function *func1,
                        FunctionRemap *remap, string &expected_params, 
                        int indent_level, bool errors_fatal, 
                        ostream &ForwardDeclrs, 
                        const std::string &functionnamestr, bool is_inplace,
                        bool coercion_possible) {
  string format_specifiers;
  std::string keyword_list;
  string parameter_list;
  string container;
  vector_string pexprs;
  string extra_convert;
  string extra_param_check;
  string extra_cleanup;
  string pname_for_pyobject;

  bool is_constructor = false;

  if (remap->_type == FunctionRemap::T_constructor) {
    is_constructor = true;
  }

  // Make one pass through the parameter list.  We will output a
  // one-line temporary variable definition for each parameter, while
  // simultaneously building the ParseTuple() function call and also
  // the parameter expression list for call_function().

  expected_params += methodNameFromCppName(func1, "");
  expected_params += "(";

  int pn;
  for (pn = 0; pn < (int)remap->_parameters.size(); pn++) {
    if(pn > 0) {
      expected_params += ", ";
    }

    CPPType *orig_type = remap->_parameters[pn]._remap->get_orig_type();
    CPPType *type = remap->_parameters[pn]._remap->get_new_type();
    string param_name = remap->get_parameter_name(pn);
    
    // This is the string to convert our local variable to the
    // appropriate C++ type.  Normally this is just a cast.
    string pexpr_string =
      "(" + type->get_local_name(&parser) + ")" + param_name;

    if (!remap->_has_this || pn != 0) {
      keyword_list += "(char *)\""+remap->_parameters[pn]._name + "\", ";
    }

    if (remap->_parameters[pn]._remap->new_type_is_atomic_string()) {
      if (TypeManager::is_char_pointer(orig_type)) {
        indent(out,indent_level)<< "char *" << param_name;
        format_specifiers += "s";
        parameter_list += ", &" + param_name;
        
      } else if (TypeManager::is_wstring(orig_type)) {
        indent(out,indent_level) << "PyUnicodeObject *" << param_name << "\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        extra_convert += " int " + param_name + "_len = PyUnicode_GetSize((PyObject *)" + param_name + "); wchar_t *" + param_name + "_str = new wchar_t[" + param_name + "_len]; PyUnicode_AsWideChar(" + param_name + ", " + param_name + "_str, " + param_name + "_len);";

        pexpr_string = "basic_string<wchar_t>((wchar_t *)" +
          param_name + "_str, " +
          param_name + "_len)";

        extra_cleanup += " delete[] " + param_name + "_str;";

      } else {
        indent(out,indent_level) << "char *" << param_name
            << "_str; int " << param_name << "_len";
        format_specifiers += "s#";
        parameter_list += ", &" + param_name
          + "_str, &" + param_name + "_len";
        pexpr_string = "basic_string<char>(" +
          param_name + "_str, " +
          param_name + "_len)";
      }
      expected_params += "string";
      
    } else if (TypeManager::is_bool(type)) {
      indent(out,indent_level) << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      pexpr_string = "(PyObject_IsTrue(" + param_name + ")!=0)";
      expected_params += "bool";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_unsigned_longlong(type)) {
      indent(out,indent_level) << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += " PyObject *" + param_name + "_long = PyNumber_Long(" + param_name + ");";
      extra_param_check += "|| (" + param_name + "_long == NULL)";
      pexpr_string = "PyLong_AsUnsignedLongLong(" + param_name + "_long)";
      extra_cleanup += " Py_XDECREF(" + param_name + "_long);";
      expected_params += "unsigned long long";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_longlong(type)) {
      indent(out,indent_level) << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += " PyObject *" + param_name + "_long = PyNumber_Long(" + param_name + ");";
      extra_param_check += "|| (" + param_name + "_long == NULL)";
      pexpr_string = "PyLong_AsLongLong(" + param_name + "_long)";
      extra_cleanup += " Py_XDECREF(" + param_name + "_long);";
      expected_params += "long long";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_unsigned_integer(type)) {
      indent(out,indent_level) << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += " PyObject *" + param_name + "_uint = PyNumber_Long(" + param_name + ");";
      extra_param_check += "|| (" + param_name + "_uint == NULL)";
      pexpr_string = "PyLong_AsUnsignedLong(" + param_name + "_uint)";
      extra_cleanup += " Py_XDECREF(" + param_name + "_uint);";
      expected_params += "unsigned int";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_integer(type)) {
      indent(out,indent_level) << "int " << param_name;
      format_specifiers += "i";
      parameter_list += ", &" + param_name;
      expected_params += "int";

    } else if (TypeManager::is_float(type)) {
      indent(out,indent_level) << "double " << param_name;
      format_specifiers += "d";
      parameter_list += ", &" + param_name;
      expected_params += "float";

    } else if (TypeManager::is_char_pointer(type)) {
      indent(out,indent_level) << "char *" << param_name;
      format_specifiers += "s";
      parameter_list += ", &" + param_name;
      expected_params += "string";

    } else if (TypeManager::is_pointer_to_PyObject(type)) {
      indent(out,indent_level) << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      pexpr_string = param_name;
      pname_for_pyobject += param_name;
      expected_params += "any";

    } else if (TypeManager::is_pointer(type)) {
      CPPType *obj_type = TypeManager::unwrap(TypeManager::resolve_type(type));
      bool const_ok = !TypeManager::is_non_const_pointer_or_ref(orig_type);

      if (const_ok) {
        expected_params += "const ";
      } else {
        expected_params += "non-const ";
      }
      expected_params += classNameFromCppName(obj_type->get_simple_name());

      if (!remap->_has_this || pn != 0) {
        indent(out, indent_level) 
          << "PyObject *" << param_name;
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
        pname_for_pyobject += param_name;

        TypeIndex p_type_index = builder.get_type(obj_type,false);
        InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
        const InterrogateType &p_itype = idb->get_type(p_type_index);    

        bool is_copy_constructor = false;
        if (is_constructor && remap->_parameters.size() == 1 && pn == 0) {
          if (&p_itype == &obj->_itype) {
            // If this is the only one parameter, and it's the same as
            // the "this" type, this is a copy constructor.
            is_copy_constructor = true;
          }
        }

        //make_safe_name(itype.get_scoped_name())
        extra_convert += p_itype.get_scoped_name()+" *" + param_name + "_this = ("+p_itype.get_scoped_name()+" *)";
        // need to a forward scope for this class..
        if(!isExportThisRun(p_itype._cpptype)) {
          _external_imports.insert(make_safe_name(p_itype.get_scoped_name()));
          //ForwardDeclrs << "IMPORT_THIS struct   Dtool_PyTypedObject Dtool_" << make_safe_name(p_itype.get_scoped_name()) << ";\n";
        }

        string class_name;
        string method_prefix;
        if (remap->_cpptype) {
          class_name = remap->_cpptype->get_simple_name();
          method_prefix = classNameFromCppName(class_name) + string(".");
        }

        ostringstream str;
        str << "DTOOL_Call_GetPointerThisClass(" << param_name 
            << ", &Dtool_" << make_safe_name(p_itype.get_scoped_name()) 
            << ", " << pn << ", \"" 
            << method_prefix << methodNameFromCppName(func1, class_name)
            << "\", " << const_ok;
        if (coercion_possible && !is_copy_constructor) {
          // We never attempt to coerce a copy constructor parameter.
          // That would lead to infinite recursion.
          str << ", coerced_ptr, report_errors";
        } else {
          str << ", NULL, true";
        }
        str << ");\n";

        extra_convert += str.str();
        extra_param_check += "|| (" + param_name + "_this == NULL)";
        pexpr_string =  param_name + "_this";
      }

    } else {
      // Ignore a parameter.
      indent(out,indent_level) << "PyObject *" << param_name;
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      expected_params += "any";
      pname_for_pyobject += param_name;
    }

    if (remap->_parameters[pn]._has_name) {
      expected_params += " " + remap->_parameters[pn]._name;
    }

    if (remap->_has_this && pn == 0) {
      container = "local_this";
      if (remap->_const_method) {
        string class_name = remap->_cpptype->get_local_name(&parser);
        container = "(const " + class_name + "*)local_this";
      }
    } else {
      out << ";\n";
    }

    pexprs.push_back(pexpr_string);
  }
  expected_params += ")\n";
    
  // If we got what claimed to be a unary operator, don't check for
  // parameters, since we won't be getting any anyway.
  if (!func1->_ifunc.is_unary_op()) {
    std::string format_specifiers1 = format_specifiers + ":" + 
      methodNameFromCppName(func1, "");
    indent(out,indent_level)
      << "static char * key_word_list[] = {" << keyword_list << "NULL};\n";

    if (remap->_parameters.size() == 1 || 
        (remap->_has_this && remap->_parameters.size() == 2)) {
      indent(out,indent_level)
        << "// Special Case to Make operator work \n";
      indent(out,indent_level)
        << "if(PyTuple_Check(args) || (kwds != NULL && PyDict_Check(kwds)))\n";
      indent(out,indent_level)
        << "    (PyArg_ParseTupleAndKeywords(args,kwds, \"" 
        << format_specifiers1 << "\", key_word_list" << parameter_list
        << "));\n";
      indent(out,indent_level)
        << "else\n";
      indent(out,indent_level)
        << "    (PyArg_Parse(args, \"" << format_specifiers1 << "\"" 
        << parameter_list << "));\n";
      indent(out,indent_level)
        << "if(!PyErr_Occurred())\n";

    } else {
      indent(out,indent_level)
        << "if (PyArg_ParseTupleAndKeywords(args,kwds, \"" 
        << format_specifiers1 << "\", key_word_list"
        << parameter_list << "))\n";
    }
  }
  indent(out,indent_level) << "{\n";

  if (!extra_convert.empty()) {
    indent(out,indent_level+4)
      << extra_convert << "\n";
  }

  int extra_indent_level = indent_level+4;

  if (!extra_param_check.empty()) {
    indent(out,extra_indent_level)
      << "if (!(" << extra_param_check.substr(3) << "))\n";
    indent(out,extra_indent_level)
      <<"{\n";
    extra_indent_level+=4;
  }

  if ((remap->_flags & (FunctionRemap::F_getitem_int | FunctionRemap::F_setitem_int)) &&
      (obj != (Object *)NULL && (obj->_protocol_types & Object::PT_sequence))) {
    // This is a getitem or setitem of a sequence type.  This means we
    // *need* to raise IndexError if we're out of bounds.  We have to
    // assume the bounds are 0 .. this->size() (this is the same
    // assumption that Python makes).
    string pexpr_string = pexprs[1];
    indent(out, extra_indent_level)
      << "if ((" << pexpr_string << ") < 0 || (" << pexpr_string << ") >= ("
      << container << ")->size()) {\n";
    if (coercion_possible) {
      indent(out, extra_indent_level + 2)
        << "Py_XDECREF(coerced);\n";
    }
    indent(out, extra_indent_level + 2)
      << "PyErr_SetString(PyExc_IndexError, \"Out of bounds.\");\n";
    indent(out, extra_indent_level + 2)
      << "return NULL;\n";
    indent(out, extra_indent_level)
      << "}\n";
  }
  
  if (!remap->_void_return && 
      remap->_return_type->new_type_is_atomic_string()) {
    // Treat strings as a special case.  We don't want to format the
    // return expression. 
    if (remap->_blocking) {
      // With SIMPLE_THREADS, it's important that we never release the
      // interpreter lock.
      out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
      indent(out, extra_indent_level)
        << "PyThreadState *_save;\n";
      indent(out, extra_indent_level)
        << "Py_UNBLOCK_THREADS\n";
      out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
    }
    if (track_interpreter) {
      indent(out,extra_indent_level) << "in_interpreter = 0;\n";
    }
    string tt;
    string return_expr = remap->call_function(out, extra_indent_level, false, container, pexprs);
    CPPType *type = remap->_return_type->get_orig_type();
    indent(out,extra_indent_level);
    type->output_instance(out, "return_value", &parser);
    //    type->output_instance(tt, "return_value", &parser);
    out << " = " << return_expr << ";\n";

    if (track_interpreter) {
      indent(out,extra_indent_level) << "in_interpreter = 1;\n";
    }
    if (remap->_blocking) {
      out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
      indent(out, extra_indent_level)
        << "Py_BLOCK_THREADS\n";
      out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
    }
    if (!extra_cleanup.empty()) {
      indent(out,extra_indent_level) <<  extra_cleanup << "\n";
    }

    return_expr = manage_return_value(out, 4, remap, "return_value");
    do_assert_init(out, extra_indent_level,is_constructor);
    pack_return_value(out, extra_indent_level, remap, return_expr,ForwardDeclrs,is_inplace);

  } else {
    if (remap->_blocking) {
      out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
      indent(out, extra_indent_level)
        << "PyThreadState *_save;\n";
      indent(out, extra_indent_level)
        << "Py_UNBLOCK_THREADS\n";
      out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
    }
    if (track_interpreter) {
      indent(out,extra_indent_level) << "in_interpreter = 0;\n";
    }

    string return_expr = remap->call_function(out, extra_indent_level, true, container, pexprs);
    if (coercion_possible) {
      indent(out, extra_indent_level)
        << "Py_XDECREF(coerced);\n";
    }
    if (return_expr.empty()) {
      if (track_interpreter) {
        indent(out,extra_indent_level) << "in_interpreter = 1;\n";
      }
      if (remap->_blocking) {
        out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
        indent(out, extra_indent_level)
          << "Py_BLOCK_THREADS\n";
        out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
      }
      if (!extra_cleanup.empty()) {
        indent(out,extra_indent_level) << extra_cleanup << "\n";
      }
      do_assert_init(out, extra_indent_level,is_constructor);
      indent(out,extra_indent_level) << "return Py_BuildValue(\"\");\n";

    } else {
      CPPType *type = remap->_return_type->get_temporary_type();
      if(!is_inplace) {
        indent(out,extra_indent_level);
        type->output_instance(out, "return_value", &parser);
        out << " = " << return_expr << ";\n";
      }
      if (track_interpreter) {
        indent(out,extra_indent_level) << "in_interpreter = 1;\n";
      }
      if (remap->_blocking) {
        out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
        indent(out, extra_indent_level)
          << "Py_BLOCK_THREADS\n";
        out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
      }
      if (!extra_cleanup.empty()) {
        indent(out,extra_indent_level) << extra_cleanup << "\n";
      }

      if (!is_inplace) {
        return_expr = manage_return_value(out, extra_indent_level, remap, "return_value");
      }
      do_assert_init(out, extra_indent_level,is_constructor);
      pack_return_value(out, extra_indent_level, remap, remap->_return_type->temporary_to_return(return_expr),ForwardDeclrs,is_inplace);
    }
  }

  if (!extra_param_check.empty()) {
    extra_indent_level-=4;
    indent(out,extra_indent_level)<< "}\n";
  }
  
  indent(out,indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::pack_return_value
//       Access: Private
//  Description: Outputs a command to pack the indicated expression,
//               of the return_type type, as a Python return value.
////////////////////////////////////////////////////////////////////

void InterfaceMakerPythonNative::pack_return_value(ostream &out, int indent_level,
                                                   FunctionRemap *remap, string return_expr, ostream &ForwardDeclrs, bool is_inplace) 
{
  CPPType *orig_type = remap->_return_type->get_orig_type();
  CPPType *type = remap->_return_type->get_new_type();

  if (remap->_return_type->new_type_is_atomic_string()) {
    if (TypeManager::is_char_pointer(orig_type)) {
      indent(out, indent_level)
        << "return PyString_FromString(" << return_expr << ");\n";

    } else if (TypeManager::is_wstring(orig_type)) {
      indent(out, indent_level)
        << "return PyUnicode_FromWideChar("
        << return_expr << ".data(), (int)" << return_expr << ".length());\n";

    } else {
      indent(out, indent_level)
        << "return PyString_FromStringAndSize("
        << return_expr << ".data(), (int)" << return_expr << ".length());\n";
    }

  } else if (TypeManager::is_unsigned_longlong(type)) {
    indent(out, indent_level)
      << "return PyLong_FromUnsignedLongLong(" << return_expr << ");\n";

  } else if (TypeManager::is_longlong(type)) {
      indent(out, indent_level)
          << "return PyLong_FromLongLong(" << return_expr << ");\n";

  }  else if(TypeManager::is_unsigned_integer(type)){
      indent(out, indent_level)
          << "return PyLongOrInt_FromUnsignedLong(" << return_expr << ");\n";

  }else if (TypeManager::is_integer(type)) {
      indent(out, indent_level)
          << "return PyInt_FromLong(" << return_expr << ");\n";
  } else if (TypeManager::is_float(type)) {
      indent(out, indent_level)
          << "return PyFloat_FromDouble(" << return_expr << ");\n";

  } else if (TypeManager::is_char_pointer(type)) {
      indent(out, indent_level)
      << "return PyString_FromString(" << return_expr << ");\n";

  }
  else if (TypeManager::is_pointer_to_PyObject(type)) 
  {
    indent(out, indent_level)
      << "return  "<< return_expr << ";\n";
    
  }
  else if (TypeManager::is_pointer(type)) 
  {
      string const_flag;
      if (TypeManager::is_const_pointer_to_anything(type)) {
        const_flag = "true";
      } else {
        const_flag = "false";
      }

      if (TypeManager::is_struct(orig_type) || TypeManager::is_ref_to_anything(orig_type)) 
      {
        if( TypeManager::is_ref_to_anything(orig_type))
        {
            TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)),false);
            InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
            const InterrogateType &itype = idb->get_type(type_index);    
            std::string ows_memory_flag("true");

            if(remap->_return_value_needs_management)
              ows_memory_flag = "true";
            else
              ows_memory_flag = "false";

           if(!isExportThisRun(itype._cpptype))
           {
               _external_imports.insert(make_safe_name(itype.get_scoped_name()));
               //ForwardDeclrs << "IMPORT_THIS struct   Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
           }

           WriteReturnInstance(out,indent_level,return_expr,ows_memory_flag,itype.get_scoped_name(),itype._cpptype,is_inplace, const_flag);

        }
        else
        {   
            if (remap->_type == FunctionRemap::T_constructor )
            {
                // should only reach this in the INIT function a a Class .. IE the PY exists before the CPP object
                // this is were we type to returned a class/struct.. ie CPP TYpe            


                TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
                InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
                const InterrogateType &itype = idb->get_type(type_index);    
                indent(out, indent_level)
                    <<"return DTool_PyInit_Finalize(self, " << return_expr <<",&"<<CLASS_PREFEX  << make_safe_name(itype.get_scoped_name()) << ",true,false);\n";
            }
            else
            {

                std::string ows_memory_flag("true");
                if(remap->_return_value_needs_management)
                    ows_memory_flag = "true";
                else
                    ows_memory_flag = "false";

                if(remap->_manage_reference_count)
                {
                    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)),false);
                    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
                    const InterrogateType &itype = idb->get_type(type_index);    

                    if(!isExportThisRun(itype._cpptype))
                    {
                        _external_imports.insert(make_safe_name(itype.get_scoped_name()));
                        //ForwardDeclrs << "IMPORT_THIS struct   Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
                    }

                    //                    ForwardDeclrs << "extern  \"C\" struct   Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
                    WriteReturnInstance(out,indent_level,return_expr,ows_memory_flag,itype.get_scoped_name(),itype._cpptype,is_inplace, const_flag);
                }
                else
                {
                    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
                    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
                    const InterrogateType &itype = idb->get_type(type_index);    

                    if(!isExportThisRun(itype._cpptype))
                    {
                        _external_imports.insert(make_safe_name(itype.get_scoped_name()));
                        //ForwardDeclrs << "IMPORT_THIS struct   Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
                    }

                    //                    ForwardDeclrs << "extern  \"C\" struct   Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
                    WriteReturnInstance(out,indent_level,return_expr,ows_memory_flag,itype.get_scoped_name(),itype._cpptype,is_inplace, const_flag);

                }
            }
        }
      } 
      else if( TypeManager::is_struct(orig_type->as_pointer_type()->_pointing_at) )
      {
          TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
          InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
          const InterrogateType &itype = idb->get_type(type_index);    

          std::string ows_memory_flag("true");
          if(remap->_return_value_needs_management)
              ows_memory_flag ="true";
          else
              ows_memory_flag = "false";

          if(!isExportThisRun(itype._cpptype))
          {
               _external_imports.insert(make_safe_name(itype.get_scoped_name()));
              //ForwardDeclrs << "IMPORT_THIS struct   Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
          }

          //        ForwardDeclrs << "extern  \"C\" struct   Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
          WriteReturnInstance(out,indent_level,return_expr,ows_memory_flag,itype.get_scoped_name(),itype._cpptype,is_inplace, const_flag);

      }
      else
      {
        indent(out, indent_level)<<"  Shouln Never Reach This InterfaceMakerPythonNative::pack_return_value";
            //<< "return PyInt_FromLong((int)" << return_expr << ");\n";
      }

  } else {
    // Return None.
    indent(out, indent_level)
      << "return Py_BuildValue(\"\");\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonName::write_make_seq
//       Access: Public
//  Description: Generates the synthetic method described by the
//               MAKE_SEQ() macro.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_make_seq(ostream &out, Object *obj, const std::string &ClassName,
               MakeSeq *make_seq) {
  out << "/******************************************************************\n" << " * Python make_seq wrapper\n";
  out << " *******************************************************************/\n";

  out << "static PyObject *" << make_seq->_name + "(PyObject *self, PyObject *) {\n";
  string num_name = methodNameFromCppName(make_seq->_num_name, ClassName);
  string element_name = methodNameFromCppName(make_seq->_element_name, ClassName);

  out << "  return make_list_for_item(self, \"" << num_name 
      << "\", \"" << element_name << "\");\n";
  out << "}\n";
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::record_object
//       Access: Protected
//  Description: Records the indicated type, which may be a struct
//               type, along with all of its associated methods, if
//               any.
////////////////////////////////////////////////////////////////////
InterfaceMaker::Object *InterfaceMakerPythonNative::record_object(TypeIndex type_index) 
{

  if (type_index == 0) {
    return (Object *)NULL;
  }

  Objects::iterator oi = _objects.find(type_index);
  if (oi != _objects.end()) {
    return (*oi).second;
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  const InterrogateType &itype = idb->get_type(type_index);    

  if(!isCppTypeLegal(itype._cpptype))
  {
       return (Object *)NULL;
  }

  Object *object = new Object(itype);
  bool inserted = _objects.insert(Objects::value_type(type_index, object)).second;
  assert(inserted);

  Function *function;

  int num_constructors = itype.number_of_constructors();
  for (int ci = 0; ci < num_constructors; ci++) 
  {
      function = record_function(itype, itype.get_constructor(ci));
      if(isFunctionLegal(function))
        object->_constructors.push_back(function);
  }
  
  int num_methods = itype.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; mi++) 
  {
    function = record_function(itype, itype.get_method(mi));
    if(isFunctionLegal(function))
        object->_methods.push_back(function);

  }
  
  int num_casts = itype.number_of_casts();
  for (mi = 0; mi < num_casts; mi++) 
  {
    function = record_function(itype, itype.get_cast(mi));
    if(isFunctionLegal(function))
        object->_methods.push_back(function);
  }
  
  int num_derivations = itype.number_of_derivations();
  for (int di = 0; di < num_derivations; di++) 
  {
    TypeIndex d_type_Index = itype.get_derivation(di);
    idb->get_type(d_type_Index);    

    if(!interrogate_type_is_unpublished(d_type_Index))
    {
        if (itype.derivation_has_upcast(di)) 
        {
            function = record_function(itype, itype.derivation_get_upcast(di));
            if(isFunctionLegal(function))
                object->_methods.push_back(function);
        }
        if (itype.derivation_has_downcast(di)) 
        {
            // Downcasts are methods of the base class, not the child class.
            TypeIndex base_type_index = itype.get_derivation(di);

            const InterrogateType &base_type = idb->get_type(base_type_index);
            function = record_function(base_type, itype.derivation_get_downcast(di));

            if(isFunctionLegal(function))
            {
                Object * pobject = record_object(base_type_index);
                if(pobject != NULL)
                    pobject->_methods.push_back(function);
            }
        }
    }
  }

  int num_elements = itype.number_of_elements();
  for (int ei = 0; ei < num_elements; ei++)
  {
    ElementIndex element_index = itype.get_element(ei);
    const InterrogateElement &ielement = idb->get_element(element_index);
    if (ielement.has_getter()) 
    {
      FunctionIndex func_index = ielement.get_getter();
      record_function(itype, func_index);
    }
    if (ielement.has_setter()) 
    {
      FunctionIndex func_index = ielement.get_setter();
      record_function(itype, func_index);
    }
  }    

  object->check_protocols();

  int num_nested = itype.number_of_nested_types();
  for (int ni = 0; ni < num_nested; ni++)
  {
        TypeIndex nested_index = itype.get_nested_type(ni);
          record_object(nested_index);
  }
  return object;
}
////////////////////////////////////////////////////////////////////
//     Function: InterfaceMaker::generate_wrappers
//       Access: Public, Virtual
//  Description: Walks through the set of functions in the database
//               and generates wrappers for each function, storing
//               these in the database.  No actual code should be
//               output yet; this just updates the database with the
//               wrapper information.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::generate_wrappers() 
{
  inside_python_native = true;
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  // We use a while loop rather than a simple for loop, because we
  // might increase the number of types recursively during the
  // traversal.

  int ti = 0;
  while (ti < idb->get_num_all_types())
  {
        TypeIndex type_index = idb->get_all_type(ti);
        record_object(type_index);
        ++ti;
  }

  int num_global_elements = idb->get_num_global_elements();
  for (int gi = 0; gi < num_global_elements; ++gi) {
    printf(" Global Type = %d",gi);
    TypeIndex type_index = idb->get_global_element(gi);
    record_object(type_index);

  }

  int num_functions = idb->get_num_global_functions();
  for (int fi = 0; fi < num_functions; fi++) 
  {
    FunctionIndex func_index = idb->get_global_function(fi);
    record_function(dummy_type, func_index);
  }    



  int num_manifests = idb->get_num_global_manifests();
  for (int mi = 0; mi < num_manifests; mi++) 
  {
    ManifestIndex manifest_index = idb->get_global_manifest(mi);
    const InterrogateManifest &iman = idb->get_manifest(manifest_index);
    if (iman.has_getter()) 
    {
      FunctionIndex func_index = iman.get_getter();
      record_function(dummy_type, func_index);
    }
  }    

  int num_elements = idb->get_num_global_elements();
  for (int ei = 0; ei < num_elements; ei++) 
  {
    printf(" Element %d\n",ei);

    ElementIndex element_index = idb->get_global_element(ei);
    const InterrogateElement &ielement = idb->get_element(element_index);
    if (ielement.has_getter()) 
    {
      FunctionIndex func_index = ielement.get_getter();
      record_function(dummy_type, func_index);
    }
    if (ielement.has_setter()) 
    {
      FunctionIndex func_index = ielement.get_setter();
      record_function(dummy_type, func_index);
    }
  }    
  inside_python_native = false;
} 
////////////////////////////////////////////// 
//   Function :isCppTypeLegal
//
// is the cpp object  supported by by the dtool_py interface..
//////////////////////////////////////////////
bool InterfaceMakerPythonNative::isCppTypeLegal(CPPType *in_ctype)
{
    if(in_ctype == NULL)
        return false;

    if(builder.in_ignoretype(in_ctype->get_local_name(&parser)))
    {
        return false;
    }

    //bool answer = false;
    CPPType *type = TypeManager::unwrap(TypeManager::resolve_type(in_ctype));
    type = TypeManager::unwrap(type);
    //CPPType *type =  ctype;

    if(TypeManager::is_basic_string_char(type))
    {
        return true;
    }
    else if(TypeManager::is_basic_string_wchar(type))
    {
        return true;
    }
    else if(TypeManager::is_simple(type))
    {
        return true;
    }
    else  if(builder.in_forcetype(in_ctype->get_local_name(&parser)))
    {
        return true;
    }
    else if(TypeManager::IsExported(type) == true)
    {
         return  true;
    }
    else if(TypeManager::is_pointer_to_PyObject(in_ctype) == true)
    {
      return true;
    }

    //if(answer == false)
//        printf(" -------------------- Bad Type ?? %s \n",type->get_local_name().c_str());

    return false;
}
////////////////////////////////////////////// 
//   Function :isExportThisRun
//
////////////////////////////////////////////// 
bool InterfaceMakerPythonNative::isExportThisRun(CPPType *ctype)
{
    CPPType *type = TypeManager::unwrap(ctype);
    if(TypeManager::IsLocal(type))
        return true;

    if(builder.in_forcetype(type->get_local_name(&parser)))
        return true;

    return false;
}

////////////////////////////////////////////// 
// Function : isExportThisRun
/////////////////////////////////////////////
bool InterfaceMakerPythonNative::isExportThisRun(Function *func)
{
    if(func == NULL || !isFunctionLegal(func))
        return false;

    Function::Remaps::const_iterator ri;
    for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) 
    {
        FunctionRemap *remap = (*ri);
        return isExportThisRun(remap->_cpptype);
    }

    return false;
}
////////////////////////////////////////////// 
// Function : isRemapLegal
//////////////////////////////////////////////
bool InterfaceMakerPythonNative::isRemapLegal( FunctionRemap &remap)
{
    // return must be legal and managable..
    if(!isCppTypeLegal(remap._return_type->get_orig_type()))
    {
//        printf("  isRemapLegal Return Is Bad %s\n",remap._return_type->get_orig_type()->get_fully_scoped_name().c_str());
        return false;
    }

    // ouch .. bad things will happen here ..  do not even try..
    if(remap._ForcedVoidReturn)
       return false;

    // all params must be legal
    for (int pn = 0; pn < (int)remap._parameters.size(); pn++)
    {
        CPPType *orig_type = remap._parameters[pn]._remap->get_orig_type();
        if(!isCppTypeLegal(orig_type))
            return false;
    }

    // ok all looks ok.
    return true;
}
////////////////////////////////////////////////////////////////////////
// Function  : isFunctionLegal
////////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::isFunctionLegal( Function *func)
{
    Function::Remaps::const_iterator ri;
    for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) 
    {
        FunctionRemap *remap = (*ri);
        if(isRemapLegal(*remap))
        {
//    printf("  Function Is Marked Legal %s\n",func->_name.c_str());

            return true;
        }
    }   

//    printf("  Function Is Marked Illegal %s\n",func->_name.c_str());
    return false;
}
////////////////////////////////////////////////////////////////////////
// Function  : isFunctionWithThis
//
// If any rempas have a this .. this function has a this..( of self) to python..
////////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::isFunctionWithThis( Function *func)
{
    Function::Remaps::const_iterator ri;
    for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) 
    {
        FunctionRemap *remap = (*ri);
        if(remap->_has_this)
            return true;
    }   
    return false;
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPython::test_assert
//       Access: Protected
//  Description: Outputs code to check to see if an assertion has
//               failed while the C++ code was executing, and report
//               this failure back to Python.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::do_assert_init(ostream &out, int indent_level, bool constructor) const {
  if (watch_asserts) {
    out << "#ifndef NDEBUG\n";
    indent(out, indent_level)
      << "Notify *notify = Notify::ptr();\n";
    indent(out, indent_level)
      << "if (notify->has_assert_failed()) {\n";
    indent(out, indent_level + 2)
      << "PyErr_SetString(PyExc_AssertionError, notify->get_assert_error_message().c_str());\n";
    indent(out, indent_level + 2)
      << "notify->clear_assert_failed();\n";
    if(constructor)
        indent(out, indent_level + 2) << "return -1;\n";
    else
        indent(out, indent_level + 2) << "return (PyObject *)NULL;\n";
    indent(out, indent_level)
      << "}\n";
    out << "#endif\n";
    indent(out, indent_level)
      << "if (PyErr_Occurred()) {\n";
    if(constructor)
        indent(out, indent_level + 2) << "return -1;\n";
    else
        indent(out, indent_level + 2) << "return (PyObject *)NULL;\n";
    indent(out, indent_level)
      << "}\n";
  }
}
////////////////////////////////////////////////////////
// Function :  IsRunTimeTyped
///////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::IsRunTimeTyped(const InterrogateType &itype)
{
    TypeIndex ptype_id = itype.get_outer_class();
    if(ptype_id > 0)
    {
        InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

        InterrogateType ptype = idb->get_type(ptype_id);
        return IsRunTimeTyped(ptype);
    }


   if(itype.get_name() == "TypedObject")
       return true;

    return false;
};

//////////////////////////////////////////////////////////
// Function : DoesInheritFromIsClass
//
// Helper function to check cpp class inharatience..
///////////////////////////////////////////////////////////
bool   InterfaceMakerPythonNative::DoesInheritFromIsClass( const CPPStructType * inclass, const std::string &name)
{
    if(inclass == NULL)
        return false;

    std::string scoped_name = inclass->get_fully_scoped_name();
    if(scoped_name == name)
      return true;

    CPPStructType::Derivation::const_iterator bi;
    for (bi = inclass->_derivation.begin();
        bi != inclass->_derivation.end();
        ++bi) 
    {

        const CPPStructType::Base &base = (*bi);

        CPPStructType *base_type = TypeManager::resolve_type(base._base)->as_struct_type();
        if(base_type != NULL)
        {
            if(DoesInheritFromIsClass(base_type,name) == true)
                return true;
        }
    }  
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
//  Function : HasAGetKeyFunction
//
// does the class have a supportable GetKey for hash usage..
//////////////////////////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::HasAGetKeyFunction(const InterrogateType &itype_class)
{
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  int num_methods = itype_class.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; mi++) 
  {
      FunctionIndex func_index = itype_class.get_method(mi);
      const InterrogateFunction &ifunc = idb->get_function(func_index);
      if(ifunc.get_name() == "get_key") 
      {
          if (ifunc._instances != (InterrogateFunction::Instances *)NULL) 
          {
              InterrogateFunction::Instances::const_iterator ii;
              for (ii = ifunc._instances->begin();ii != ifunc._instances->end();++ii) 
              {
                  CPPInstance *cppinst = (*ii).second;
                  CPPFunctionType *cppfunc = cppinst->_type->as_function_type();

                  if(cppfunc != NULL)
                  {
                      if(cppfunc->_parameters != NULL && cppfunc->_return_type != NULL && TypeManager::is_integer(cppfunc->_return_type))
                      {
                          if(cppfunc->_parameters->_parameters.size() == 0)
                          {
                              return true;
                          }
                      }
                  }

              }   
          }
      }
  }
      return false;
};

////////////////////////////////////////////////////////////////////////////////////////////
//  Function : HasAGetClassTypeFunction
//
// does the class have a supportable GetClassType which returns a TypeHandle.
//////////////////////////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::
HasAGetClassTypeFunction(const InterrogateType &itype_class) {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  
  int num_methods = itype_class.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; mi++) {
    FunctionIndex func_index = itype_class.get_method(mi);
    const InterrogateFunction &ifunc = idb->get_function(func_index);
    if (ifunc.get_name() == "get_class_type") {
      if (ifunc._instances != (InterrogateFunction::Instances *)NULL) {
        InterrogateFunction::Instances::const_iterator ii;
        for (ii = ifunc._instances->begin();ii != ifunc._instances->end();++ii) {
          CPPInstance *cppinst = (*ii).second;
          CPPFunctionType *cppfunc = cppinst->_type->as_function_type();
          
          if (cppfunc != NULL && cppfunc->_return_type != NULL &&
              cppfunc->_parameters != NULL) {
            CPPType *ret_type = TypeManager::unwrap(cppfunc->_return_type);
            if (TypeManager::is_struct(ret_type) && 
                ret_type->get_simple_name() == "TypeHandle") {
              if(cppfunc->_parameters->_parameters.size() == 0) {
                return true;
              }
            }
          }
          
        }   
      }
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::NeedsAStrFunction
//       Access: Private
//  Description: Returns -1 if the class does not define write() (and
//               therefore cannot support a __str__ function).
//
//               Returns 1 if the class defines write(ostream).
//
//               Returns 2 if the class defines write(ostream, int).
////////////////////////////////////////////////////////////////////
int InterfaceMakerPythonNative::
NeedsAStrFunction(const InterrogateType &itype_class) {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  int num_methods = itype_class.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; ++mi) {
    FunctionIndex func_index = itype_class.get_method(mi);
    const InterrogateFunction &ifunc = idb->get_function(func_index);
    if (ifunc.get_name() == "write") {
      if (ifunc._instances != (InterrogateFunction::Instances *)NULL) {
        InterrogateFunction::Instances::const_iterator ii;
        for (ii = ifunc._instances->begin();
             ii != ifunc._instances->end();
             ++ii) {
          CPPInstance *cppinst = (*ii).second;
          CPPFunctionType *cppfunc = cppinst->_type->as_function_type();

          if (cppfunc != NULL) {
            if (cppfunc->_parameters != NULL && 
                cppfunc->_return_type != NULL && 
                TypeManager::is_void(cppfunc->_return_type)) {
              if (cppfunc->_parameters->_parameters.size() == 1) {
                CPPInstance *inst1 = cppfunc->_parameters->_parameters[0];
                if (TypeManager::is_pointer_to_ostream(inst1->_type)) {
                  // write(ostream)
                  return 1;
                }
              }

              if (cppfunc->_parameters->_parameters.size() == 2) {
                CPPInstance *inst1 = cppfunc->_parameters->_parameters[0];
                if (TypeManager::is_pointer_to_ostream(inst1->_type)) {
                  inst1 = cppfunc->_parameters->_parameters[1];
                  if (inst1->_initializer != NULL) {
                    // write(ostream, int = 0)
                    return 1;
                  }

                  if (TypeManager::is_integer(inst1->_type)) {
                    // write(ostream, int)
                    return 2;
                  }
                }
              }
            }
          }
        }   
      }
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::NeedsAReprFunction
//       Access: Private
//  Description: Returns -1 if the class does not define output() or
//               python_repr() (and therefore cannot support a
//               __repr__ function).
//
//               Returns 1 if the class defines python_repr(ostream, string).
//
//               Returns 2 if the class defines output(ostream).
////////////////////////////////////////////////////////////////////
int InterfaceMakerPythonNative::
NeedsAReprFunction(const InterrogateType &itype_class) {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  int num_methods = itype_class.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; ++mi) {
    FunctionIndex func_index = itype_class.get_method(mi);
    const InterrogateFunction &ifunc = idb->get_function(func_index);
    if (ifunc.get_name() == "python_repr") {
      if (ifunc._instances != (InterrogateFunction::Instances *)NULL) {
        InterrogateFunction::Instances::const_iterator ii;
        for (ii = ifunc._instances->begin();
             ii != ifunc._instances->end();
             ++ii) {
          CPPInstance *cppinst = (*ii).second;
          CPPFunctionType *cppfunc = cppinst->_type->as_function_type();

          if (cppfunc != NULL) {
            if (cppfunc->_parameters != NULL && 
                cppfunc->_return_type != NULL && 
                TypeManager::is_void(cppfunc->_return_type)) {
              if (cppfunc->_parameters->_parameters.size() == 2) {
                CPPInstance *inst1 = cppfunc->_parameters->_parameters[0];
                if (TypeManager::is_pointer_to_ostream(inst1->_type)) {
                  inst1 = cppfunc->_parameters->_parameters[1];
                  if (TypeManager::is_string(inst1->_type) ||
                      TypeManager::is_char_pointer(inst1->_type)) {
                    // python_repr(ostream, string)
                    return 1;
                  }
                }
              }
            }
          }
        }   
      }
    }
  }

  for (mi = 0; mi < num_methods; ++mi) {
    FunctionIndex func_index = itype_class.get_method(mi);
    const InterrogateFunction &ifunc = idb->get_function(func_index);
    if (ifunc.get_name() == "output") {
      if (ifunc._instances != (InterrogateFunction::Instances *)NULL) {
        InterrogateFunction::Instances::const_iterator ii;
        for (ii = ifunc._instances->begin();
             ii != ifunc._instances->end();
             ++ii) {
          CPPInstance *cppinst = (*ii).second;
          CPPFunctionType *cppfunc = cppinst->_type->as_function_type();

          if (cppfunc != NULL) {
            if (cppfunc->_parameters != NULL && 
                cppfunc->_return_type != NULL && 
                TypeManager::is_void(cppfunc->_return_type)) {
              if (cppfunc->_parameters->_parameters.size() == 1) {
                CPPInstance *inst1 = cppfunc->_parameters->_parameters[0];
                if (TypeManager::is_pointer_to_ostream(inst1->_type)) {
                  // output(ostream)
                  return 2;
                }
              }

              if (cppfunc->_parameters->_parameters.size() >= 2) {
                CPPInstance *inst1 = cppfunc->_parameters->_parameters[0];
                if (TypeManager::is_pointer_to_ostream(inst1->_type)) {
                  inst1 = cppfunc->_parameters->_parameters[1];
                  if (inst1->_initializer != NULL) {
                    // output(ostream, foo = bar, ...)
                    return 2;
                  }
                }
              }
            }
          }
        }   
      }
    }
  }

  return -1;
}


////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::output_quoted
//       Access: Private
//  Description: Outputs the indicated string as a single quoted,
//               multi-line string to the generated C++ source code.
//               The output point is left on the last line of the
//               string, following the trailing quotation mark.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
output_quoted(ostream &out, int indent_level, const std::string &str) {
  indent(out, indent_level)
    << '"';
  std::string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    switch (*si) {
    case '"':
    case '\\':
      out << '\\' << *si;
      break;

    case '\n':
      out << "\\n\"\n";
      indent(out, indent_level)
        << '"';
      break;

    default:
      if (!isprint(*si)) {
        out << "\\" << oct << setw(3) << setfill('0') << (unsigned int)(*si)
            << dec;
      } else {
        out << *si;
      }
    }
  }
  out << '"';
}
