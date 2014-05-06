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
extern std::string EXPORT_IMPORT_PREFIX;

#define CLASS_PREFIX "Dtool_"
#define INSTANCE_PREFIX "Dtool_"
#define BASE_INSTANCE_NAME "Dtool_PyInstDef"

/////////////////////////////////////////////////////////
// Name Remapper...
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
  { "operator =="   , "__eq__",                 0 },
  { "operator !="   , "__ne__",                 0 },
  { "operator << "  , "__lshift__",             0 },
  { "operator >>"   , "__rshift__",             0 },
  { "operator <"    , "__lt__",                 0 },
  { "operator >"    , "__gt__",                 0 },
  { "operator <="   , "__le__",                 0 },
  { "operator >="   , "__ge__",                 0 },
  { "operator ="    , "assign",                 0 },
  { "operator ()"   , "__call__",               0 },
  { "operator []"   , "__getitem__",            0 },
  { "operator ++unary", "increment",            0 },
  { "operator ++"   , "increment",              0 },
  { "operator --unary", "decrement",            0 },
  { "operator --"   , "decrement",              0 },
  { "operator ^"    , "__xor__",                0 },
  { "operator %"    , "__mod__",                0 },
  { "operator !"    , "logicalNot",             0 },
  { "operator ~unary", "__invert__",            0 },
  { "operator &"    , "__and__",                0 },
  { "operator &&"   , "logicalAnd",             0 },
  { "operator |"    , "__or__",                 0 },
  { "operator ||"   , "logicalOr",              0 },
  { "operator +"    , "__add__",                0 },
  { "operator -"    , "__sub__",                0 },
  { "operator -unary", "__neg__",               0 },
  { "operator *"    , "__mul__",                0 },
  { "operator /"    , "__div__",                0 },
  { "operator +="   , "__iadd__",               1 },
  { "operator -="   , "__isub__",               1 },
  { "operator *="   , "__imul__",               1 },
  { "operator /="   , "__idiv__",               1 },
  { "operator ,"    , "concatenate",            0 },
  { "operator |="   , "__ior__",                1 },
  { "operator &="   , "__iand__",               1 },
  { "operator ^="   , "__ixor__",               1 },
  { "operator ~="   , "bitwiseNotEqual",        0 },
  { "operator ->"   , "dereference",            0 },
  { "operator <<="  , "__ilshift__",            1 },
  { "operator >>="  , "__irshift__",            1 },
  { "operator typecast bool", "__nonzero__",    0 },
  { "__nonzero__"   , "__nonzero__",            0 },
  { "__reduce__"    , "__reduce__",             0 },
  { "__reduce_persist__", "__reduce_persist__", 0 },
  { "__copy__"      , "__copy__",               0 },
  { "__deepcopy__"  , "__deepcopy__",           0 },
  { "print"         , "Cprint",                 0 },
  { "CInterval.set_t", "_priv__cSetT",          0 },
  { "__bool__"      , "__bool__",               0 },
  { "__bytes__"     , "__bytes__",              0 },
  { "__iter__"      , "__iter__",               0 },
  { "__getbuffer__" , "__getbuffer__",          0 },
  { "__releasebuffer__", "__releasebuffer__",   0 },
  { NULL, NULL, -1 }
};

const char *InPlaceSet[] = {
  "__iadd__",
  "__isub__",
  "__imul__",
  "__idiv__",
  "__ior__",
  "__iand__",
  "__ixor__",
  "__ilshift__",
  "__irshift__",
  "__itruediv__",
  "__ifloordiv__",
  "__imod__",
  "__ipow__",
  NULL,
};

///////////////////////////////////////////////////////////////////////////////////////
RenameSet classRenameDictionary[] = {
  // No longer used, now empty.
  { NULL, NULL, -1 }
};

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
const char *pythonKeywords[] = {
  "and",
  "as",
  "assert",
  "break",
  "class",
  "continue",
  "def",
  "del",
  "elif",
  "else",
  "except",
  "exec",
  "finally",
  "for",
  "from",
  "global",
  "if",
  "import",
  "in",
  "is",
  "lambda",
  "nonlocal",
  "not",
  "or",
  "pass",
  "print",
  "raise",
  "return",
  "try",
  "while",
  "with",
  "yield",
  NULL
};

///////////////////////////////////////////////////////////////////////////////////////
std::string
checkKeyword(std::string &cppName) {
  for (int x = 0; pythonKeywords[x] != NULL; x++) {
    if (cppName == pythonKeywords[x]) {
      return std::string("_") + cppName;
    }
  }
  return cppName;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
std::string
classNameFromCppName(const std::string &cppName, bool mangle) {
  //# initialize to empty string
  std::string className = "";

  //# These are the characters we want to strip out of the name
  const std::string badChars("!@#$%^&*()<>,.-=+~{}? ");

  bool nextCap = false;
  bool firstChar = true && mangle;

  for (std::string::const_iterator chr = cppName.begin();
       chr != cppName.end();
       chr++) {
    if ((*chr == '_' || *chr == ' ') && mangle) {
      nextCap = true;

    } else if (badChars.find(*chr) != std::string::npos) {
      if (!mangle) {
        className += '_';
      }

    } else if (nextCap || firstChar) {
      className += toupper(*chr);
      nextCap = false;
      firstChar = false;

    } else {
      className += * chr;
    }
  }

  for (int x = 0; classRenameDictionary[x]._from != NULL; x++) {
    if (cppName == classRenameDictionary[x]._from) {
      className = classRenameDictionary[x]._to;
    }
  }

  if (className.empty()) {
    std::string text = "** ERROR ** Renaming class: " + cppName + " to empty string";
    printf("%s", text.c_str());
  }

  className = checkKeyword(className);
  //# FFIConstants.notify.debug('Renaming class: ' + cppName + ' to: ' + className)
  return className;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
std::string
methodNameFromCppName(const std::string &cppName, const std::string &className, bool mangle) {
  std::string origName = cppName;

  if (origName.substr(0, 6) == "__py__") {
    // By convention, a leading prefix of "__py__" is stripped.  This
    // indicates a Python-specific variant of a particular method.
    origName = origName.substr(6);
  }

  std::string methodName;
  const std::string badChars("!@#$%^&*()<>,.-=+~{}? ");
  bool nextCap = false;
  for (std::string::const_iterator chr = origName.begin();
       chr != origName.end();
       chr++) {
    if ((*chr == '_' || *chr == ' ') && mangle) {
      nextCap = true;

    } else if (badChars.find(*chr) != std::string::npos) {
      if (!mangle) {
        methodName += '_';
      }

    } else if (nextCap) {
      methodName += toupper(*chr);
      nextCap = false;

    } else {
      methodName += *chr;
    }
  }

  for (int x = 0; methodRenameDictionary[x]._from != NULL; x++) {
    if (origName == methodRenameDictionary[x]._from) {
      methodName = methodRenameDictionary[x]._to;
    }
  }

  if (className.size() > 0) {
    string lookup_name = className + '.' + cppName;
    for (int x = 0; classRenameDictionary[x]._from != NULL; x++) {
      if (lookup_name == methodRenameDictionary[x]._from) {
        methodName = methodRenameDictionary[x]._to;
      }
    }
  }

  //    # Mangle names that happen to be python keywords so they are not anymore
  methodName = checkKeyword(methodName);
  return methodName;
}

std::string methodNameFromCppName(InterfaceMaker::Function *func, const std::string &className, bool mangle) {
  std::string cppName = func->_ifunc.get_name();
  if (func->_ifunc.is_unary_op()) {
    cppName += "unary";
  }
  return methodNameFromCppName(cppName, className, mangle);
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

bool isInplaceFunction(InterfaceMaker::Function *func) {
  std::string wname = methodNameFromCppName(func, "", false);

  for (int x = 0; InPlaceSet[x] != NULL; x++) {
    if (InPlaceSet[x] == wname) {
      return true;
    }
  }

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
  def._min_version = 0;

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

  if (method_name == "operator << ") {
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
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator -=") {
    def._answer_location = "tp_as_number->nb_inplace_subtract";
    def._wrapper_type = WT_one_param;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator *=") {
    def._answer_location = "tp_as_number->nb_inplace_multiply";
    def._wrapper_type = WT_one_param;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator /=") {
    def._answer_location = "tp_as_number->nb_inplace_divide";
    def._wrapper_type = WT_one_param;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator %=") {
    def._answer_location = ".tp_as_number->nb_inplace_remainder";
    def._wrapper_type = WT_one_param;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator <<=") {
    def._answer_location = "tp_as_number->nb_inplace_lshift";
    def._wrapper_type = WT_one_param;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator >>=") {
    def._answer_location = "tp_as_number->nb_inplace_rshift";
    def._wrapper_type = WT_one_param;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator &=") {
    def._answer_location = "tp_as_number->nb_inplace_and";
    def._wrapper_type = WT_one_param;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator ^=") {
    def._answer_location = "tp_as_number->nb_inplace_xor";
    def._wrapper_type = WT_one_param;
    def._min_version = 0x02000000;
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

  if (obj->_protocol_types & Object::PT_iter) {
    if (method_name == "__iter__") {
      def._answer_location = "tp_iter";
      def._wrapper_type = WT_no_params;
      return true;
    }

    if (method_name == "next" || method_name == "__next__") {
      def._answer_location = "tp_iternext";
      def._wrapper_type = WT_iter_next;
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

  if (method_name == "__nonzero__") {
    // Python 2 style.
    def._answer_location = "tp_as_number->nb_nonzero";
    def._wrapper_type = WT_inquiry;
    return true;
  }

  if (method_name == "__bool__") {
    // Python 3 style.
    def._answer_location = "tp_as_number->nb_nonzero";
    def._wrapper_type = WT_inquiry;
    return true;
  }

  if (method_name == "__getbuffer__") {
    def._answer_location = "tp_as_buffer->bf_getbuffer";
    def._wrapper_type = WT_getbuffer;
    def._min_version = 0x02060000;
    return true;
  }

  if (method_name == "__releasebuffer__") {
    def._answer_location = "tp_as_buffer->bf_releasebuffer";
    def._wrapper_type = WT_releasebuffer;
    def._min_version = 0x02060000;
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
void InterfaceMakerPythonNative::
get_valid_child_classes(std::map<std::string, CastDetails> &answer, CPPStructType *inclass, const std::string &upcast_seed, bool can_downcast) {
  if (inclass == NULL) {
    return;
  }

  CPPStructType::Derivation::const_iterator bi;
  for (bi = inclass->_derivation.begin();
      bi != inclass->_derivation.end();
      ++bi) {

    const CPPStructType::Base &base = (*bi);
//        if (base._vis <= V_public)
//          can_downcast = false;
    CPPStructType *base_type = TypeManager::resolve_type(base._base)->as_struct_type();
    if (base_type != NULL) {
      std::string scoped_name = base_type->get_local_name(&parser);

      if (answer.find(scoped_name) == answer.end()) {
        answer[scoped_name]._can_downcast = can_downcast;
        answer[scoped_name]._to_class_name = scoped_name;
        answer[scoped_name]._structType = base_type;

        if (base._is_virtual) {
          answer[scoped_name]._can_downcast = false;
        }

        std::string local_upcast("(");
        local_upcast += scoped_name + " *)"+ upcast_seed +"";
        answer[scoped_name]._up_cast_string = local_upcast;
        answer[scoped_name]._is_legal_py_class = is_cpp_type_legal(base_type);
      } else {
        answer[scoped_name]._can_downcast = false;
      }

      get_valid_child_classes(answer, base_type, answer[scoped_name]._up_cast_string, answer[scoped_name]._can_downcast);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//  Function : write_python_instance
//
///////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_python_instance(ostream &out, int indent_level, const std::string &return_expr, const std::string &assign_to, std::string &owns_memory_flag, const std::string &class_name, CPPType *ctype, bool inplace, const std::string &const_flag) {
  string assign_stmt("return ");
  if (!assign_to.empty()) {
    assign_stmt = assign_to + " = ";
  }

  if (inplace) {
    indent(out, indent_level) << "Py_INCREF(self);\n";
    indent(out, indent_level) << assign_stmt << "self;\n";
  } else {
    indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
    indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
    indent(out, indent_level+2) << assign_stmt << "Py_None;\n";
    indent(out, indent_level) << "} else {\n";

    if (IsPandaTypedObject(ctype->as_struct_type())) {
      std::string typestr = "(" + return_expr + ")->as_typed_object()->get_type_index()";
      indent(out, indent_level+2) << assign_stmt
        << "DTool_CreatePyInstanceTyped((void *)" << return_expr << ", " << CLASS_PREFIX << make_safe_name(class_name) << ", " << owns_memory_flag << ", " << const_flag << ", " << typestr << ");\n";
    } else {
      //    indent(out, indent_level) << "if (" << return_expr << "!= NULL)\n";
      indent(out, indent_level+2) << assign_stmt
        << "DTool_CreatePyInstance((void *)" << return_expr << ", " << CLASS_PREFIX << make_safe_name(class_name) << ", " << owns_memory_flag << ", " << const_flag << ");\n";
    }
    indent(out, indent_level) << "}\n";
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
void InterfaceMakerPythonNative::
write_prototypes(ostream &out_code, ostream *out_h) {
  inside_python_native = true;

  Functions::iterator fi;

  if (out_h != NULL) {
    *out_h << "#include \"py_panda.h\"\n\n";
  }

  out_code << "//********************************************************************\n";
  out_code << "//*** prototypes for .. Global\n";
  out_code << "//********************************************************************\n";

  /*
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) 
  {
      Function *func = (*fi);
      if (!func->_itype.is_global() && is_function_legal(func))
        write_prototype_for (out_code, func);
  }
  */

  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (object->_itype.is_class() || object->_itype.is_struct()) {
      if (is_cpp_type_legal(object->_itype._cpptype)) {
        if (isExportThisRun(object->_itype._cpptype)) {
          write_prototypes_class(out_code, out_h, object);
        } else {
          //write_prototypes_class_external(out_code, object);
          _external_imports.insert(make_safe_name(object->_itype.get_scoped_name()));
        }
      }
    }
  }

  out_code << "//********************************************************************\n";
  out_code << "//*** prototypes for .. External Objects\n";
  out_code << "//********************************************************************\n";

  for (std::set<std::string>::iterator ii = _external_imports.begin(); ii != _external_imports.end(); ii++) {
    out_code << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << *ii << ";\n";
  }

  inside_python_native = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//  Function : write_prototypes_class_external
//    
//   Description :  Output enough enformation to a declartion of a externally 
//                 generated dtool type object
/////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_prototypes_class_external(ostream &out, Object *obj) {
  std::string class_name = make_safe_name(obj->_itype.get_scoped_name());
  std::string c_class_name =  obj->_itype.get_true_name();
  std::string preferred_name =  obj->_itype.get_name();


  out << "//********************************************************************\n";
  out << "//*** prototypes for external.. " << class_name << "\n";
  out << "//********************************************************************\n";

  out << "typedef  " << c_class_name << "  " << class_name << "_localtype;\n";
  out << "Define_Module_Class_Forward(" << _def->module_name << ", " << class_name << ", " << class_name << "_localtype, " << classNameFromCppName(preferred_name, false) << ");\n";
}

///////////////////////////////////////// ////////////////////////////////////////////////////
// Function : write_prototypes_class
//
/////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_prototypes_class(ostream &out_code, ostream *out_h, Object *obj) {
  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  Functions::iterator fi;
  
  out_code << "//********************************************************************\n";
  out_code << "//*** prototypes for .. " << ClassName << "\n";
  out_code << "//********************************************************************\n";

  /*
  for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) {
    Function *func = (*fi);
    write_prototype_for(out_code, func);
  }
  */

  /*
  for (fi = obj->_constructors.begin(); fi != obj->_constructors.end(); ++fi) {
    Function *func = (*fi);
    std::string fname = "int Dtool_Init_" + ClassName + "(PyObject *self, PyObject *args, PyObject *kwds)";
    write_prototype_for_name(out_code, obj, func, fname);
  }
  */

  write_class_declarations(out_code, out_h, obj);
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_functions
//       Access: Public, Virtual
//  Description: Generates the list of functions that are appropriate
//               for this interface.  This function is called *before*
//               write_prototypes(), above.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_functions(ostream &out) {
  inside_python_native = true;
  
  out << "//********************************************************************\n";
  out << "//*** Functions for .. Global\n" ;
  out << "//********************************************************************\n";
  Functions::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi);
    if (!func->_itype.is_global() && is_function_legal(func)) {
      write_function_for_top(out, NULL, func, "");
    }
  }

  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (object->_itype.is_class() || object->_itype.is_struct()) {
      if (is_cpp_type_legal(object->_itype._cpptype)) {
        if (isExportThisRun(object->_itype._cpptype)) {
          write_class_details(out, object);
        }
      }
    }
  }
  
  //Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (!object->_itype.get_outer_class()) {
      if (object->_itype.is_class() || object->_itype.is_struct()) {
        if (is_cpp_type_legal(object->_itype._cpptype)) {
          if (isExportThisRun(object->_itype._cpptype)) {
            write_module_class(out, object);
          }
        }
      }
    }
  }

  inside_python_native = true;
}

////////////////////////////////////////////////////////////
//  Function : write_class_details
////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_class_details(ostream &out, Object *obj) {
  Functions::iterator fi;
  
  //std::string cClassName = obj->_itype.get_scoped_name();
  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName = obj->_itype.get_true_name();
  
  out << "//********************************************************************\n";
  out << "//*** Functions for .. " << cClassName << "\n" ;
  out << "//********************************************************************\n";

  for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) {
    Function *func = (*fi);
    if (func) {
      SlottedFunctionDef def;
      get_slotted_function_def(obj, func, def);

      ostringstream GetThis;
      GetThis << "  " << cClassName << " *local_this = NULL;\n";
      GetThis << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
      GetThis << "  if (local_this == NULL) {\n";
      if (def._wrapper_type == WT_numeric_operator) {
        // WT_numeric_operator means we must return NotImplemented, instead
        // of raising an exception, if the this pointer doesn't
        // match.  This is for things like __sub__, which Python
        // likes to call on the wrong-type objects.
        GetThis << "    Py_INCREF(Py_NotImplemented);\n";
        GetThis << "    return Py_NotImplemented;\n";

      } else {
        // Other functions should raise an exception if the this
        // pointer isn't set or is the wrong type.
        GetThis << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
        GetThis << "    return NULL;\n";
      }
      GetThis << "  }\n";
      write_function_for_top(out, obj, func, GetThis.str());
    }
  }

  if (obj->_constructors.size() == 0) {
    std::string fname = "int Dtool_Init_" + ClassName + "(PyObject *self, PyObject *args, PyObject *kwds)";
    out << fname << " {\n";
    out << "  PyErr_SetString(PyExc_TypeError, \"cannot init constant class (" << cClassName << ")\");\n";
    out << "  return -1;\n" ;
    out << "}\n\n";

    out
      << "int Dtool_InitNoCoerce_" << ClassName << "(PyObject *self, PyObject *args, PyObject *kwds) {\n"
      << "  return Dtool_Init_" << ClassName << "(self, args, kwds);\n"
      << "}\n\n";

  } else {
    bool coercion_attempted = false;
    for (fi = obj->_constructors.begin(); fi != obj->_constructors.end(); ++fi) {
      Function *func = (*fi);
      std::string fname = "int Dtool_Init_" + ClassName + "(PyObject *self, PyObject *args, PyObject *kwds)";

      write_function_for_name(out, obj, func, fname, "", ClassName, true, coercion_attempted);
    }
    if (coercion_attempted) {
      // If a coercion attempt was written into the above constructor,
      // then write a secondary constructor, that won't attempt any
      // coercion.  We'll need this for nested coercion calls.
      for (fi = obj->_constructors.begin(); fi != obj->_constructors.end(); ++fi) {
        Function *func = (*fi);
        std::string fname = "int Dtool_InitNoCoerce_" + ClassName + "(PyObject *self, PyObject *args, PyObject *kwds)";

        write_function_for_name(out, obj, func, fname, "", ClassName, false, coercion_attempted);
      }
    } else {
      // Otherwise, since the above constructor didn't involve any
      // coercion anyway, we can use the same function for both
      // purposes.  Construct a trivial wrapper.
      out
        << "int Dtool_InitNoCoerce_" << ClassName << "(PyObject *self, PyObject *args, PyObject *kwds) {\n"
        << "  return Dtool_Init_" << ClassName << "(self, args, kwds);\n"
        << "}\n\n";
    }
  }

  MakeSeqs::iterator msi;
  for (msi = obj->_make_seqs.begin(); msi != obj->_make_seqs.end(); ++msi) {
    write_make_seq(out, obj, ClassName, *msi);
  }

  CPPType *cpptype = TypeManager::resolve_type(obj->_itype._cpptype);
  std::map<string, CastDetails> details;
  std::map<string, CastDetails>::iterator di;
  builder.get_type(TypeManager::unwrap(cpptype), false);
  get_valid_child_classes(details, cpptype->as_struct_type());
  for (di = details.begin(); di != details.end(); di++) {
    //InterrogateType ptype =idb->get_type(di->first);
    if (di->second._is_legal_py_class && !isExportThisRun(di->second._structType))
      _external_imports.insert(make_safe_name(di->second._to_class_name));
    //out << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << make_safe_name(di->second._to_class_name) << ";\n";
  }

  { // the Cast Converter

    out << "inline void *Dtool_UpcastInterface_" << ClassName << "(PyObject *self, Dtool_PyTypedObject *requested_type) {\n";
    out << "  Dtool_PyTypedObject *SelfType = ((Dtool_PyInstDef *)self)->_My_Type;\n";
    out << "  if (SelfType != &Dtool_" << ClassName << ") {\n";
    out << "    printf(\"" << ClassName << " ** Bad Source Type-- Requesting Conversion from %s to %s\\n\", ((Dtool_PyInstDef *)self)->_My_Type->_name, requested_type->_name); fflush(NULL);\n";;
    out << "    return NULL;\n";
    out << "  }\n";
    out << "\n";
    out << "  " << cClassName << " *local_this = (" << cClassName << " *)((Dtool_PyInstDef *)self)->_ptr_to_object;\n"; 
    out << "  if (requested_type == &Dtool_" << ClassName << ") {\n";
    out << "    return local_this;\n";
    out << "  }\n";

    for (di = details.begin(); di != details.end(); di++) {
      if (di->second._is_legal_py_class) {
        out << "  if (requested_type == &Dtool_" << make_safe_name(di->second._to_class_name) << ") {\n";
        out << "    return " << di->second._up_cast_string << " local_this;\n";
        out << "  }\n";
      }
    }

    out << "  return NULL;\n";
    out << "}\n\n";

    out << "inline void *Dtool_DowncastInterface_" << ClassName << "(void *from_this, Dtool_PyTypedObject *from_type) {\n";
    out << "  if (from_this == NULL || from_type == NULL) {\n";
    out << "    return NULL;\n";
    out << "  }\n";
    out << "  if (from_type == &Dtool_" << ClassName << ") {\n";
    out << "    return from_this;\n";
    out << "  }\n";
    for (di = details.begin(); di != details.end(); di++) {
      if (di->second._can_downcast && di->second._is_legal_py_class) {
        out << "  if (from_type == &Dtool_" << make_safe_name(di->second._to_class_name) << ") {\n";
        out << "    " << di->second._to_class_name << "* other_this = (" << di->second._to_class_name << "*)from_this;\n" ;
        out << "    return (" << cClassName << "*)other_this;\n";
        out << "  }\n";
      }
    }
    out << "  return (void *) NULL;\n";
    out << "}\n\n";
  }
}

////////////////////////////////////////////////////////////
/// Function : write_class_declarations
//
//
////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_class_declarations(ostream &out, ostream *out_h, Object *obj) {
  const InterrogateType &itype = obj->_itype;
  std::string class_name = make_safe_name(obj->_itype.get_scoped_name());
  std::string c_class_name =  obj->_itype.get_true_name();
  std::string preferred_name =  itype.get_name();
  std::string class_struct_name = std::string(CLASS_PREFIX) + class_name;

  out << "typedef " << c_class_name << " " << class_name << "_localtype;\n";
  if (obj->_itype.has_destructor() ||
      obj->_itype.destructor_is_inherited()) {

    if (TypeManager::is_reference_count(obj->_itype._cpptype)) {
      out << "Define_Module_ClassRef(" << _def->module_name << ", " << class_name << ", " << class_name << "_localtype, " << classNameFromCppName(preferred_name, false) << ");\n";
    } else {
      out << "Define_Module_Class(" << _def->module_name << ", " << class_name << ", " << class_name << "_localtype, " << classNameFromCppName(preferred_name, false) << ");\n";
    }
  } else {
    if (TypeManager::is_reference_count(obj->_itype._cpptype)) {
      out << "Define_Module_ClassRef_Private(" << _def->module_name << ", " << class_name << ", " << class_name << "_localtype, " << classNameFromCppName(preferred_name, false) << ");\n";
    } else {
      out << "Define_Module_Class_Private(" << _def->module_name << ", " << class_name << ", " << class_name << "_localtype, " << classNameFromCppName(preferred_name, false) << ");\n";
    }
  }
  out << "\n";

  if (out_h != NULL) {
    *out_h << "extern \"C\" " << EXPORT_IMPORT_PREFIX << " struct Dtool_PyTypedObject Dtool_" << class_name << ";\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_sub_module
//       Access: Public, Virtual
//  Description: Generates whatever additional code is required to
//               support a module file.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_sub_module(ostream &out, Object *obj) {
  //Object * obj = _objects[_embeded_index] ;
  std::string class_name = make_safe_name(obj->_itype.get_scoped_name());
  out << "  // Module init upcall for " << obj->_itype.get_scoped_name() << "\n";
  out << "  Dtool_PyModuleClassInit_" << class_name << "(module);\n";
}

/////////////////////////////////////////////////////////////////////////////
// Function : write_module_support
/////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_module_support(ostream &out, ostream *out_h, InterrogateModuleDef *def) {
  out << "//********************************************************************\n";
  out << "//*** Module Object Linker ..\n";
  out << "//********************************************************************\n";

  out << "static void BuildInstants(PyObject * module) {\n";
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (!object->_itype.get_outer_class()) {
      if (object->_itype.is_enum()) {
        int enum_count = object->_itype.number_of_enum_values();
        if (enum_count > 0) {
            out << "//********************************************************************\n";
            out << "//*** Module Enums  .." << object->_itype.get_scoped_name() << "\n";
            out << "//********************************************************************\n";
        }
        for (int xx = 0; xx< enum_count; xx++) {
          string name1 = classNameFromCppName(object->_itype.get_enum_value_name(xx), false);
          string name2 = classNameFromCppName(object->_itype.get_enum_value_name(xx), true);
          int enum_value = object->_itype.get_enum_value(xx);
          out << "   PyModule_AddIntConstant(module, \"" << name1 << "\", " << enum_value << ");\n";
          if (name1 != name2) {
            // Also write the mangled name, for historical purposes.
            out << "   PyModule_AddIntConstant(module, \"" << name2 << "\", " << enum_value << ");\n";
          }
        }
      }
    }
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  int num_manifests = idb->get_num_global_manifests();
  for (int mi = 0; mi < num_manifests; mi++) {
    ManifestIndex manifest_index = idb->get_global_manifest(mi);
    const InterrogateManifest &iman = idb->get_manifest(manifest_index);
    if (iman.has_getter()) {
      FunctionIndex func_index = iman.get_getter();
      record_function(dummy_type, func_index);
    }

    string name1 = classNameFromCppName(iman.get_name(), false);
    string name2 = classNameFromCppName(iman.get_name(), true);
    if (iman.has_int_value()) {
      int value = iman.get_int_value();
      out << "   PyModule_AddIntConstant(module, \"" << name1 << "\", " << value << ");\n";
      if (name1 != name2) {
        // Also write the mangled name, for historical purposes.
        out << "   PyModule_AddIntConstant(module, \"" << name2 << "\", " << value << ");\n";
      }
    } else {
      string value = iman.get_definition();
      out << "   PyModule_AddStringConstant(module, \"" << name1 << "\", \"" << value << "\");\n";
      if (name1 != name2) {
        out << "   PyModule_AddStringConstant(module, \"" << name2 << "\", \"" << value << "\");\n";
      }
    }
  }

  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (!object->_itype.get_outer_class()) {
      if (object->_itype.is_class() ||object->_itype.is_struct()) {
        if (is_cpp_type_legal(object->_itype._cpptype)) {
          if (isExportThisRun(object->_itype._cpptype)) {
            write_sub_module(out, object);
          }
        }
      }
    }
  }
  out << "//********************************************************************\n";
  out << "//*** Module Init Upcall ..  Externally Defined Class\n";
  out << "//********************************************************************\n";

//    for (std::set< std::string >::iterator ii = _external_imports.begin(); ii != _external_imports.end(); ii++)
//                 out << "Dtool_" <<*ii << "._Dtool_ClassInit(NULL);\n";

  out << "}\n\n";

  bool force_base_functions = true;

  out << "static PyMethodDef python_simple_funcs[] = {\n";
  Functions::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi);
    if (!func->_itype.is_global() && is_function_legal(func)) {
      string name1 = methodNameFromCppName(func, "", false);
      string name2 = methodNameFromCppName(func, "", true);
      out << "  { \"" << name1 << "\", (PyCFunction) &"
          << func->_name << ", METH_VARARGS | METH_KEYWORDS, (char *)" << func->_name << "_comment},\n";
      if (name1 != name2) {
        out << "  { \"" << name2 << "\", (PyCFunction) &"
            << func->_name << ", METH_VARARGS | METH_KEYWORDS, (char *)" << func->_name << "_comment},\n";
      }
    }
  }

  if (force_base_functions) {
    out << "  // Support Function For Dtool_types ... for now in each module ??\n";
    out << "  {\"Dtool_BorrowThisReference\", &Dtool_BorrowThisReference, METH_VARARGS, \"Used to borrow 'this' pointer (to, from)\\nAssumes no ownership.\"},\n"; 
    out << "  {\"Dtool_AddToDictionary\", &Dtool_AddToDictionary, METH_VARARGS, \"Used to add items into a tp_dict\"},\n"; 
  }

  out << "  {NULL, NULL, 0, NULL}\n" << "};\n\n";

  out << "EXPORT_THIS struct LibraryDef " << def->library_name << "_moddef = {python_simple_funcs, BuildInstants};\n";
  if (out_h != NULL) {
    *out_h << "extern struct LibraryDef " << def->library_name << "_moddef;\n";
  }
}

/////////////////////////////////////////////////////////////////////////////
///// Function : write_module
/////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_module(ostream &out, ostream *out_h, InterrogateModuleDef *def) {
  InterfaceMakerPython::write_module(out, out_h, def);
  Objects::iterator oi;

  out << "//********************************************************************\n";
  out << "//*** Py Init Code For .. GlobalScope\n" ;
  out << "//********************************************************************\n";

  out << "#if PY_MAJOR_VERSION >= 3\n"
      << "static struct PyModuleDef python_native_module = {\n"
      << "  PyModuleDef_HEAD_INIT,\n"
      << "  \"" << def->module_name << "\",\n"
      << "  NULL,\n"
      << "  -1,\n"
      << "  NULL,\n"
      << "  NULL, NULL, NULL, NULL\n"
      << "};\n"
      << "\n"
      << "#ifdef _WIN32\n"
      << "extern \"C\" __declspec(dllexport) PyObject *PyInit_" << def->module_name << "();\n"
      << "#else\n"
      << "extern \"C\" PyObject *PyInit_" << def->module_name << "();\n"
      << "#endif\n"
      << "\n"
      << "PyObject *PyInit_" << def->module_name << "() {\n"
      << "  LibraryDef *refs[] = {&" << def->library_name << "_moddef, NULL};\n"
      << "  return Dtool_PyModuleInitHelper(refs, &python_native_module);\n"
      << "}\n"
      << "\n"
      << "#else  // Python 2 case\n"
      << "\n"
      << "#ifdef _WIN32\n"
      << "extern \"C\" __declspec(dllexport) void init" << def->module_name << "();\n"
      << "#else\n"
      << "extern \"C\" void init" << def->module_name << "();\n"
      << "#endif\n"
      << "\n"
      << "void init" << def->module_name << "() {\n"
      << "  LibraryDef *refs[] = {&" << def->library_name << "_moddef, NULL};\n"
      << "  Dtool_PyModuleInitHelper(refs, \"" << def->module_name << "\");\n"
      << "}\n"
      << "\n"
      << "#endif\n"
      << "\n";
}
/////////////////////////////////////////////////////////////////////////////////////////////
// Function :write_module_class
/////////////////////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_module_class(ostream &out, Object *obj) {
  bool has_local_hash = false;
  bool has_local_repr = false;
  bool has_local_str = false;
  bool has_local_richcompare = false;
  bool has_local_getbuffer = false;

  {
    int num_nested = obj->_itype.number_of_nested_types();
    for (int ni = 0; ni < num_nested; ni++) {
      TypeIndex nested_index = obj->_itype.get_nested_type(ni);
      Object * nested_obj =  _objects[nested_index];
      if (nested_obj->_itype.is_class() || nested_obj->_itype.is_struct()) {
        write_module_class(out, nested_obj);
      }
    }
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName =  obj->_itype.get_true_name();
  std::string export_class_name = classNameFromCppName(obj->_itype.get_name(), false);
  std::string export_class_name2 = classNameFromCppName(obj->_itype.get_name(), true);

  Functions::iterator fi;
  out << "//********************************************************************\n";
  out << "//*** Py Init Code For .. " << ClassName << " | " << export_class_name << "\n" ;
  out << "//********************************************************************\n";
  out << "PyMethodDef Dtool_Methods_" << ClassName << "[] = {\n";

  std::map<int, Function *> static_functions;
  std::map<Function *, SlottedFunctionDef> slotted_functions;
  // function Table
  bool got_copy = false;
  bool got_deepcopy = false;

  int x = 0;
  for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) {
    Function *func = (*fi);
    if (func->_name == "__copy__") {
      got_copy = true;
    } else if (func->_name == "__deepcopy__") {
      got_deepcopy = true;
    }

    if (!isFunctionWithThis(func)) {
      // Save a reference to this static method, so we can add it
      // directly to the class.
      static_functions[x] = func;
    }

    string name1 = methodNameFromCppName(func, export_class_name, false);
    string name2 = methodNameFromCppName(func, export_class_name, true);
    out << "  { \"" << name1 << "\", (PyCFunction) &"
        << func->_name << ", METH_VARARGS | METH_KEYWORDS, (char *) " << func->_name << "_comment},\n";
    ++x;
    if (name1 != name2) {
      out << "  { \"" << name2 << "\", (PyCFunction) &"
          << func->_name << ", METH_VARARGS | METH_KEYWORDS, (char *) " << func->_name << "_comment},\n";
      ++x;
    }

    SlottedFunctionDef slotted_def;
    if (get_slotted_function_def(obj, func, slotted_def)) {
      slotted_functions[func] = slotted_def;
    }
  }

  if (obj->_protocol_types & Object::PT_make_copy) {
    if (!got_copy) {
      out << "  { \"__copy__\", (PyCFunction) &copy_from_make_copy, METH_NOARGS, NULL},\n";
      got_copy = true;
    }
  } else if (obj->_protocol_types & Object::PT_copy_constructor) {
    if (!got_copy) {
      out << "  { \"__copy__\", (PyCFunction) &copy_from_copy_constructor, METH_NOARGS, NULL},\n";
      got_copy = true;
    }
  }

  if (got_copy && !got_deepcopy) {
    out << "  { \"__deepcopy__\", (PyCFunction) &map_deepcopy_to_copy, METH_VARARGS, NULL},\n";
  }

  MakeSeqs::iterator msi;
  for (msi = obj->_make_seqs.begin(); msi != obj->_make_seqs.end(); ++msi) {
    string flags = "METH_NOARGS";
    if (obj->is_static_method((*msi)->_element_name)) {
      flags += " | METH_CLASS";
    }
    string name1 = methodNameFromCppName((*msi)->_seq_name, export_class_name, false);
    string name2 = methodNameFromCppName((*msi)->_seq_name, export_class_name, true);
    out << "  { \"" << name1
        << "\", (PyCFunction) &" << (*msi)->_name << ", " << flags << ", NULL},\n";
    if (name1 != name2) {
      out << "  { \"" << name2
          << "\", (PyCFunction) &" << (*msi)->_name << ", " << flags << ", NULL},\n";
    }
  }

  out << "  { NULL, NULL }\n"
      << "};\n\n";

  int num_derivations = obj->_itype.number_of_derivations();
  int di;
  for (di = 0; di < num_derivations; di++) {
    TypeIndex d_type_Index = obj->_itype.get_derivation(di);
    if (!interrogate_type_is_unpublished(d_type_Index)) {
      const InterrogateType &d_itype = idb->get_type(d_type_Index);
      if (is_cpp_type_legal(d_itype._cpptype)) {
        if (!isExportThisRun(d_itype._cpptype)) {
          _external_imports.insert(make_safe_name(d_itype.get_scoped_name().c_str()));

          //out << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << make_safe_name(d_itype.get_scoped_name().c_str()) << ";\n";
        }
      }
    }
  }

  std::vector<std::string> bases;
  for (di = 0; di < num_derivations; di++) {
    TypeIndex d_type_Index = obj->_itype.get_derivation(di);
    if (!interrogate_type_is_unpublished(d_type_Index)) {
      const InterrogateType &d_itype = idb->get_type(d_type_Index);
      if (is_cpp_type_legal(d_itype._cpptype)) {
        bases.push_back(make_safe_name(d_itype.get_scoped_name().c_str()));
      }
    }
  }

  if (bases.empty()) {
    bases.push_back("DTOOL_SUPER_BASE");
  }

  {
    std::map<Function *, SlottedFunctionDef>::iterator rfi; //          slotted_functions;
    for (rfi = slotted_functions.begin(); rfi != slotted_functions.end(); rfi++) {
      switch (rfi->second._wrapper_type) {
      case WT_no_params:
        // PyObject *func(PyObject *self)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self) {\n";
          out << "  PyObject *args = Py_BuildValue(\"()\");\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  return result;\n";
          out << "}\n\n";
        }
        break;

      case WT_one_param:
      case WT_numeric_operator:
        // PyObject *func(PyObject *self, PyObject *one)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *one) {\n";
          out << "  PyObject *args = Py_BuildValue(\"(O)\", one);\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  return result;\n";
          out << "}\n\n";
        }
        break;

      case WT_setattr:
        // int func(PyObject *self, PyObject *one, PyObject *two = NULL)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *one, PyObject *two) {\n";
          out << "  PyObject *args;\n";
          out << "  if (two == NULL) {\n";
          out << "      args = Py_BuildValue(\"(O)\", one);\n";
          out << "  } else {\n";
          out << "      args = Py_BuildValue(\"(OO)\", one, two);\n";
          out << "  }\n";
          out << "  PyObject *py_result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  if (py_result == NULL) return -1;\n";
          out << "#if PY_MAJOR_VERSION >= 3\n";
          out << "  int result = PyLong_AsLong(py_result);\n";
          out << "#else\n";
          out << "  int result = PyInt_AsLong(py_result);\n";
          out << "#endif\n";
          out << "  Py_DECREF(py_result);\n";
          out << "  return result;\n";
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
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *one) {\n";
          out << "  PyObject *args = Py_BuildValue(\"(O)\", one);\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  if (result == NULL) {\n";
          out << "      PyErr_Clear();\n";
          out << "      return PyObject_GenericGetAttr(self, one);\n";
          out << "  }\n";
          out << "  return result;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_getitem:
        // PyObject *func(PyObject *self, Py_ssize_t index)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, Py_ssize_t index) {\n";
          out << "  PyObject *args = Py_BuildValue(\"(i)\", index);\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  return result;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_setitem:
        // int_t func(PyObject *self, Py_ssize_t index, PyObject *value)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, Py_ssize_t index, PyObject *value) {\n";
          out << "  PyObject *args = Py_BuildValue(\"(iO)\", index, value);\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  if (result == NULL) {\n";
          out << "    return -1;\n";
          out << "  }\n";
          out << "  Py_DECREF(result);\n";
          out << "  return 0;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_size:
        // Py_ssize_t func(PyObject *self)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static Py_ssize_t " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self) {\n";
          out << "  PyObject *args = Py_BuildValue(\"()\");\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  if (result == NULL) {\n";
          out << "    return -1;\n";
          out << "  }\n";
          out << "#if PY_MAJOR_VERSION >= 3\n";
          out << "  Py_ssize_t num = PyLong_AsSsize_t(result);\n";
          out << "#else\n";
          out << "  Py_ssize_t num = PyInt_AsSsize_t(result);\n";
          out << "#endif\n";
          out << "  Py_DECREF(result);\n";
          out << "  return num;\n";
          out << "}\n\n";
        }
        break;

      case WT_mapping_setitem:
        // int func(PyObject *self, PyObject *one, PyObject *two)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *one, PyObject *two) {\n";
          out << "  PyObject *args = Py_BuildValue(\"(OO)\", one, two);\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  if (result == NULL) {\n";
          out << "    return -1;\n";
          out << "  }\n";
          out << "  Py_DECREF(result);\n";
          out << "  return 0;\n";
          out << "}\n\n";
        }
        break;

      case WT_inquiry:
        // int func(PyObject *self)
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self) {\n";
          out << "  PyObject *args = Py_BuildValue(\"()\");\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  if (result == NULL) {\n";
          out << "    return -1;\n";
          out << "  }\n";
          out << "#if PY_MAJOR_VERSION >= 3\n";
          out << "  int iresult = PyLong_AsLong(result);\n";
          out << "#else\n";
          out << "  int iresult = PyInt_AsLong(result);\n";
          out << "#endif\n";
          out << "  Py_DECREF(result);\n";
          out << "  return iresult;\n";
          out << "}\n\n";
        }
        break;

      case WT_getbuffer:
        // int __getbuffer__(PyObject *self, Py_buffer *buffer, int flags)
        // We map this directly, and assume that the arguments match.  The whole point
        // of this is to be fast, and we don't want to negate that by first wrapping
        // and then unwrapping the arguments again. We also want to guarantee const
        // correctness, since that will determine whether a read-only buffer is given.
        {
          has_local_getbuffer = true;

          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, Py_buffer *buffer, int flags) {\n";
          out << "  " << cClassName << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **) &local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          vector_string params_const(1);
          vector_string params_nonconst(1);
          FunctionRemap *remap_const = NULL;
          FunctionRemap *remap_nonconst = NULL;

          // Iterate through the remaps to find the one that matches our parameters.
          Function::Remaps::const_iterator ri;
          for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
            FunctionRemap *remap = (*ri);
            if (remap->_flags & FunctionRemap::F_getbuffer) {
              if (remap->_const_method) {
                if ((remap->_flags & FunctionRemap::F_explicit_self) == 0) {
                  params_const.push_back("self");
                }
                remap_const = remap;
              } else {
                if ((remap->_flags & FunctionRemap::F_explicit_self) == 0) {
                  params_nonconst.push_back("self");
                }
                remap_nonconst = remap;
              }
            }
          }
          params_const.push_back("buffer");
          params_const.push_back("flags");
          params_nonconst.push_back("buffer");
          params_nonconst.push_back("flags");

          // We have to distinguish properly between const and nonconst, because the function
          // may depend on it to decide whether to provide a writable buffer or a readonly buffer.
          const string const_this = "(const " + cClassName + " *)local_this";
          if (remap_const != NULL && remap_nonconst != NULL) {
            out << "  if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
            out << "    return " << remap_nonconst->call_function(out, 4, false, "local_this", params_nonconst) << ";\n";
            out << "  } else {\n";
            out << "    return " << remap_const->call_function(out, 4, false, const_this, params_const) << ";\n";
            out << "  }\n";
          } else if (remap_nonconst != NULL) {
            out << "  if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
            out << "    return " << remap_nonconst->call_function(out, 4, false, "local_this", params_nonconst) << ";\n";
            out << "  } else {\n";
            out << "    PyErr_SetString(PyExc_TypeError,\n";
            out << "                    \"Cannot call " << ClassName << ".__getbuffer__() on a const object.\");\n";
            out << "    return -1;\n";
            out << "  }\n";
          } else if (remap_const != NULL) {
            out << "  return " << remap_const->call_function(out, 4, false, const_this, params_const) << ";\n";
          } else {
            nout << ClassName << "::__getbuffer__ does not match the required signature.\n";
            out << "  return -1;\n";
          }

          out << "}\n\n";
        }
        break;

      case WT_releasebuffer:
        // void __releasebuffer__(PyObject *self, Py_buffer *buffer)
        // Same story as __getbuffer__ above.
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static void " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, Py_buffer *buffer) {\n";
          out << "  " << cClassName << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **) &local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return;\n";
          out << "  }\n\n";

          vector_string params_const(1);
          vector_string params_nonconst(1);
          FunctionRemap *remap_const = NULL;
          FunctionRemap *remap_nonconst = NULL;

          // Iterate through the remaps to find the one that matches our parameters.
          Function::Remaps::const_iterator ri;
          for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
            FunctionRemap *remap = (*ri);
            if (remap->_flags & FunctionRemap::F_releasebuffer) {
              if (remap->_const_method) {
                if ((remap->_flags & FunctionRemap::F_explicit_self) == 0) {
                  params_const.push_back("self");
                }
                remap_const = remap;
              } else {
                if ((remap->_flags & FunctionRemap::F_explicit_self) == 0) {
                  params_nonconst.push_back("self");
                }
                remap_nonconst = remap;
              }
            }
          }
          params_const.push_back("buffer");
          params_nonconst.push_back("buffer");

          string return_expr;
          const string const_this = "(const " + cClassName + " *)local_this";
          if (remap_const != NULL && remap_nonconst != NULL) {
            out << "  if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
            return_expr = remap_nonconst->call_function(out, 4, false, "local_this", params_nonconst);
            if (!return_expr.empty()) {
              out << "    " << return_expr << ";\n";
            }
            out << "  } else {\n";
            return_expr = remap_const->call_function(out, 4, false, const_this, params_const);
            if (!return_expr.empty()) {
              out << "    " << return_expr << ";\n";
            }
            out << "  }\n";
          } else if (remap_nonconst != NULL) {
            // Doesn't matter if there's no const version.  We *have* to call it or else we could leak memory.
            return_expr = remap_nonconst->call_function(out, 2, false, "local_this", params_nonconst);
            if (!return_expr.empty()) {
              out << "  " << return_expr << ";\n";
            }
          } else if (remap_const != NULL) {
            return_expr = remap_const->call_function(out, 2, false, const_this, params_const);
            if (!return_expr.empty()) {
              out << "  " << return_expr << ";\n";
            }
          } else {
            nout << ClassName << "::__releasebuffer__ does not match the required signature.\n";
            out << "  return;\n";
          }

          out << "}\n\n";
        }
        break;

      case WT_iter_next:
        // PyObject *func(PyObject *self)
        // However, returns NULL instead of None
        {
          Function *func = rfi->first;
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self) {\n";
          out << "  PyObject *args = Py_BuildValue(\"()\");\n";
          out << "  PyObject *result = " << func->_name << "(self, args, NULL);\n";
          out << "  Py_DECREF(args);\n";
          out << "  if (result == Py_None) {\n";
          out << "    Py_DECREF(Py_None);\n";
          out << "    return NULL;\n";
          out << "  } else {\n";
          out << "    return result;\n";
          out << "  }\n";
          out << "}\n\n";
        }
        break;

      case WT_none:
        break;
      }
    }

    string get_key = HasAGetKeyFunction(obj->_itype);
    if (!get_key.empty()) {
      out << "//////////////////\n";
      out << "//  A LocalHash(getKey) Function for this type\n";
      out << "//     " << ClassName << "\n";
      out << "//////////////////\n";
      out << "static Py_hash_t Dtool_HashKey_" << ClassName << "(PyObject *self) {\n";
      out << "  " << cClassName << " *local_this = NULL;\n";
      out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **) &local_this);\n";
      out << "  if (local_this == NULL) {\n";
      out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
      out << "    return -1;\n";
      out << "  }\n";
      out << "  return local_this->" << get_key << "();\n";
      out << "}\n\n";
      has_local_hash = true;
    } else {
      if (bases.size() == 0) {
        out << "//////////////////\n";
        out << "//  A LocalHash(This Pointer) Function for this type\n";
        out << "//     " << ClassName << "\n";
        out << "//////////////////\n";
        out << "static Py_hash_t Dtool_HashKey_" << ClassName << "(PyObject *self) {\n";
        out << "  " << cClassName << " *local_this = NULL;\n";
        out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **) &local_this);\n";
        out << "  if (local_this == NULL) {\n";
        out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
        out << "    return -1;\n";
        out << "  }\n";
        out << "  return (Py_hash_t) local_this;\n";
        out << "}\n\n";
        has_local_hash = true;
      }
    }

    int need_repr = NeedsAReprFunction(obj->_itype);
    if (need_repr > 0) {
      out << "//////////////////\n";
      out << "//  A __repr__ function\n";
      out << "//     " << ClassName << "\n";
      out << "//////////////////\n";
      out << "static PyObject *Dtool_Repr_" << ClassName << "(PyObject *self) {\n";
      out << "  " << cClassName << " *local_this = NULL;\n";
      out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **) &local_this);\n";
      out << "  if (local_this == NULL) {\n";
      out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
      out << "    return NULL;\n";
      out << "  }\n";
      out << "  ostringstream os;\n";
      if (need_repr == 3) {
        out << "  invoke_extension(local_this).python_repr(os, \""
            << classNameFromCppName(ClassName, false) << "\");\n";
      } else if (need_repr == 2) {
        out << "  local_this->output(os);\n";
      } else {
        out << "  local_this->python_repr(os, \""
            << classNameFromCppName(ClassName, false) << "\");\n";
      }
      out << "  std::string ss = os.str();\n";
      out << "#if PY_MAJOR_VERSION >= 3\n";
      out << "  return PyUnicode_FromStringAndSize(ss.data(), ss.length());\n";
      out << "#else\n";
      out << "  return PyString_FromStringAndSize(ss.data(), ss.length());\n";
      out << "#endif\n";
      out << "}\n\n";
      has_local_repr = true;
    }

    int need_str = NeedsAStrFunction(obj->_itype);
    if (need_str > 0) {
      out << "//////////////////\n";
      out << "//  A __str__ function\n";
      out << "//     " << ClassName << "\n";
      out << "//////////////////\n";
      out << "static PyObject *Dtool_Str_" << ClassName << "(PyObject *self) {\n";
      out << "  " << cClassName  << " *local_this = NULL;\n";
      out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
      out << "  if (local_this == NULL) {\n";
      out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
      out << "    return NULL;\n";
      out << "  }\n";
      out << "  ostringstream os;\n";
      if (need_str == 2) {
        out << "  local_this->write(os, 0);\n";
      } else {
        out << "  local_this->write(os);\n";
      }
      out << "  std::string ss = os.str();\n";
      out << "#if PY_MAJOR_VERSION >= 3\n";
      out << "  return PyUnicode_FromStringAndSize(ss.data(), ss.length());\n";
      out << "#else\n";
      out << "  return PyString_FromStringAndSize(ss.data(), ss.length());\n";
      out << "#endif\n";
      out << "}\n\n";
      has_local_str = true;
    }
  }

  if (NeedsARichCompareFunction(obj->_itype)) {
    // NB. It's a bit inefficient to first pack the 'other' arg into a
    // tuple only to be parsed by PyArg_Parse again later, but there's
    // no obvious other way to do it.
    out << "//////////////////\n";
    out << "//  A rich comparison function\n";
    out << "//     " << ClassName << "\n";
    out << "//////////////////\n";
    out << "static PyObject *Dtool_RichCompare_" << ClassName << "(PyObject *self, PyObject *other, int op) {\n";
    out << "  PyObject *kwds = NULL;\n";
    out << "  " << cClassName  << " *local_this = NULL;\n";
    out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
    out << "  if (local_this == NULL) {\n";
    out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
    out << "    return NULL;\n";
    out << "  }\n";
    out << "  PyObject *args = PyTuple_Pack(1, other);\n";
    string args_cleanup = "Py_DECREF(args);";
    out << "\n";

    out << "  switch (op) {\n";
    Function *compare_to_func = NULL;
    for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) {
      std::set<FunctionRemap*> remaps;
      Function *func = (*fi);
      if (!func) {
        continue;
      }
      // We only accept comparison operators that take one parameter (besides 'this').
      Function::Remaps::const_iterator ri;
      for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
        FunctionRemap *remap = (*ri);
        if (is_remap_legal(*remap) && remap->_has_this && remap->_parameters.size() == 2) {
          remaps.insert(remap);
        }
      }
      const string &fname = func->_ifunc.get_name();
      if (fname == "operator <") {
        out << "  case Py_LT:\n";
      } else if (fname == "operator <=") {
        out << "  case Py_LE:\n";
      } else if (fname == "operator ==") {
        out << "  case Py_EQ:\n";
      } else if (fname == "operator !=") {
        out << "  case Py_NE:\n";
      } else if (fname == "operator >") {
        out << "  case Py_GT:\n";
      } else if (fname == "operator >=") {
        out << "  case Py_GE:\n";
      } else if (fname == "compare_to") {
        compare_to_func = func;
        continue;
      } else {
        continue;
      }

      ostringstream forward_decl;
      string expected_params;
      bool coercion_attempted = false;
      write_function_forset(out, obj, func, remaps, expected_params, 2, forward_decl, false, true, coercion_attempted, args_cleanup);

      out << "  if (PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_TypeError)) {\n";
      out << "    PyErr_Clear();\n";
      out << "  }\n";
      out << "  break;\n";
      has_local_richcompare = true;
    }

    out << "  }\n\n";

    out << "  " << args_cleanup << "\n";
    out << "  if (PyErr_Occurred()) {\n";
    out << "    return (PyObject *)NULL;\n";
    out << "  }\n\n";

    if (compare_to_func != NULL) {
      out << "#if PY_MAJOR_VERSION >= 3\n";
      out << "  // All is not lost; we still have the compare_to function to fall back onto.\n";
      out << "  PyObject *result = " << compare_to_func->_name << "(self, args, kwds);\n";
      out << "  if (result != NULL) {\n";
      out << "    if (PyLong_Check(result)) {;\n";
      out << "      long cmpval = PyLong_AsLong(result);\n";
      out << "      switch (op) {\n";
      out << "      case Py_LT:\n";
      out << "        return PyBool_FromLong(cmpval < 0);\n";
      out << "      case Py_LE:\n";
      out << "        return PyBool_FromLong(cmpval <= 0);\n";
      out << "      case Py_EQ:\n";
      out << "        return PyBool_FromLong(cmpval == 0);\n";
      out << "      case Py_NE:\n";
      out << "        return PyBool_FromLong(cmpval != 0);\n";
      out << "      case Py_GT:\n";
      out << "        return PyBool_FromLong(cmpval > 0);\n";
      out << "      case Py_GE:\n";
      out << "        return PyBool_FromLong(cmpval >= 0);\n";
      out << "      }\n";
      out << "    }\n";
      out << "    Py_DECREF(result);\n";
      out << "  }\n\n";

      out << "  if (PyErr_Occurred()) {\n";
      out << "    if (PyErr_ExceptionMatches(PyExc_TypeError)) {\n";
      out << "      PyErr_Clear();\n";
      out << "    } else {\n";
      out << "      return (PyObject *)NULL;\n";
      out << "    }\n";
      out << "  }\n";
      out << "#endif\n\n";
    }

    out << "  Py_INCREF(Py_NotImplemented);\n";
    out << "  return Py_NotImplemented;\n";
    out << "}\n\n";
  }

  out << "void Dtool_PyModuleClassInit_" << ClassName << "(PyObject *module) {\n";
  out << "  static bool initdone = false;\n";
  out << "  if (!initdone) {\n";
  out << "    initdone = true;\n";
  //        out << "        memset(Dtool_" << ClassName << ".As_PyTypeObject().tp_as_number,0,sizeof(PyNumberMethods));\n";
  //        out << "        memset(Dtool_" << ClassName << ".As_PyTypeObject().tp_as_mapping,0,sizeof(PyMappingMethods));\n";
  //        out << "        static Dtool_PyTypedObject  *InheritsFrom[] = {";

  // add doc string
  if (obj->_itype.has_comment()) {
    out << "#ifndef NDEBUG\n";
    out << "    // Class documentation string\n";
    out << "    Dtool_" << ClassName
        << ".As_PyTypeObject().tp_doc =\n";
    output_quoted(out, 6, obj->_itype.get_comment());
    out << ";\n"
        << "#endif\n";
  }

  // Add flags.
  if (obj->_protocol_types & Object::PT_iter) {
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_flags |= Py_TPFLAGS_HAVE_ITER;\n";
  }
  if (has_local_getbuffer) {
    out << "#if PY_VERSION_HEX >= 0x02060000 && PY_VERSION_HEX < 0x03000000\n";
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_flags |= Py_TPFLAGS_HAVE_NEWBUFFER;\n";
    out << "#endif";
  }

  // add bases///
  if (bases.size() > 0) {
    out << "    // Dependent objects\n";
    std::string format1 =  "";
    std::string format2 =  "";
    for (std::vector<std::string>::iterator bi = bases.begin(); bi != bases.end(); bi++) {
      format1 += "O";
      format2 += ", &Dtool_" + *bi + ".As_PyTypeObject()";
      out << "    Dtool_" << make_safe_name(*bi) << "._Dtool_ClassInit(NULL);\n";
    }

    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_bases = Py_BuildValue(\"(" << format1 << ")\"" << format2 << ");\n";           
  }

  // get dictionary
  out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_dict = PyDict_New();\n";
  out << "    PyDict_SetItemString(Dtool_" <<ClassName << ".As_PyTypeObject().tp_dict, \"DtoolClassDict\", Dtool_" <<ClassName << ".As_PyTypeObject().tp_dict);\n";

  // Now assign the slotted function definitions.
  std::map<Function *, SlottedFunctionDef>::const_iterator rfi;
  int prev_min_version = 0;

  for (rfi = slotted_functions.begin(); rfi != slotted_functions.end(); rfi++) {
    Function *func = rfi->first;
    const SlottedFunctionDef &def = rfi->second;

    // Add an #ifdef if there is a specific version requirement on this function.
    if (def._min_version != prev_min_version) {
      if (prev_min_version > 0) {
        out << "#endif\n";
      }
      prev_min_version = def._min_version;
      if (def._min_version > 0) {
        out << "#if PY_VERSION_HEX >= 0x" << hex << def._min_version << dec << "\n";
      }
    }

    out << "    // " << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";

    if (def._wrapper_type == WT_none) {
      // Bound directly, without wrapper.
      out << "    Dtool_" << ClassName << ".As_PyTypeObject()." << def._answer_location << " = &" << func->_name << ";\n";
    } else {
      // Assign to the wrapper method that was generated earlier.
      out << "    Dtool_" << ClassName << ".As_PyTypeObject()." << def._answer_location << " = &" << func->_name << methodNameFromCppName(func, export_class_name, false) << ";\n";
    }
  }
  if (prev_min_version > 0) {
    out << "#endif\n";
  }

  // compare and hash work together in PY inherit behavior hmm grrr
  // __hash__
  if (has_local_hash) {
    out << "    // __hash__\n";
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_hash = &Dtool_HashKey_" << ClassName << ";\n";
    out << "#if PY_MAJOR_VERSION >= 3\n";
    if (!has_local_richcompare) {
      out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_richcompare = &DTOOL_PyObject_RichCompare;\n";
    }
    out << "#else\n";
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_compare = &DTOOL_PyObject_Compare;\n";
    out << "#endif\n";
  }

  if (has_local_richcompare) {
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_richcompare = &Dtool_RichCompare_" << ClassName << ";\n";
  }

  if (has_local_repr) {
    out << "    // __repr__\n";
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_repr = &Dtool_Repr_" << ClassName << ";\n";
  }

  if (has_local_str) {
    out << "    // __str__\n";
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_str = &Dtool_Str_" << ClassName << ";\n";

  } else if (has_local_repr) {
    out << "    // __str__ Repr Proxy\n";
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_str = &Dtool_Repr_" << ClassName << ";\n";
  }

  int num_nested = obj->_itype.number_of_nested_types();
  for (int ni = 0; ni < num_nested; ni++) {
    TypeIndex nested_index = obj->_itype.get_nested_type(ni);
    Object * nested_obj =  _objects[nested_index];
    if (nested_obj->_itype.is_class() || nested_obj->_itype.is_struct()) {
      std::string ClassName1 = make_safe_name(nested_obj->_itype.get_scoped_name());
      std::string ClassName2 = make_safe_name(nested_obj->_itype.get_name());
      out << "    // Nested Object   " << ClassName1 << ";\n";
      out << "    Dtool_" << ClassName1 << "._Dtool_ClassInit(NULL);\n";
      string name1 = classNameFromCppName(ClassName2, false);
      string name2 = classNameFromCppName(ClassName2, true);
      out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name1 << "\", (PyObject *)&Dtool_" << ClassName1 << ".As_PyTypeObject());\n";
      if (name1 != name2) {
        out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name2 << "\", (PyObject *)&Dtool_" << ClassName1 << ".As_PyTypeObject());\n";
      }
    } else {
      if (nested_obj->_itype.is_enum()) {
        out << "    // Enum  " << nested_obj->_itype.get_scoped_name() << ";\n";
        int enum_count = nested_obj->_itype.number_of_enum_values();
        for (int xx = 0; xx < enum_count; xx++) {
          string name1 = classNameFromCppName(nested_obj->_itype.get_enum_value_name(xx), false);
          string name2 = classNameFromCppName(nested_obj->_itype.get_enum_value_name(xx), true);
          int enum_value = nested_obj->_itype.get_enum_value(xx);
          out << "#if PY_MAJOR_VERSION >= 3\n";
          out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name1 << "\", PyLong_FromLong(" << enum_value << "));\n";
          if (name1 != name2) {
            out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name2 << "\", PyLong_FromLong(" << enum_value << "));\n";
          }
          out << "#else\n";
          out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name1 << "\", PyInt_FromLong(" << enum_value << "));\n";
          if (name1 != name2) {
            out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name2 << "\", PyInt_FromLong(" << enum_value << "));\n";
          }
          out << "#endif\n";
        }
      }
    }
  }

  out << "    if (PyType_Ready(&Dtool_" << ClassName << ".As_PyTypeObject()) < 0) {\n";
  out << "      PyErr_SetString(PyExc_TypeError, \"PyType_Ready(" << ClassName << ")\");\n"; 
  out << "      printf(\"Error in PyType_Ready(" << ClassName << ")\");\n";
  out << "      return;\n";
  out << "    }\n";

  out << "    Py_INCREF(&Dtool_" << ClassName << ".As_PyTypeObject());\n";

  // Why make the class a member of itself?
  //out << "        PyDict_SetItemString(Dtool_" <<ClassName << ".As_PyTypeObject().tp_dict,\"" <<export_class_name<< "\",&Dtool_" <<ClassName << ".As_PyObject());\n";

  // static function into dictionary with bogus self..
  //
  std::map<int , Function * >::iterator sfi;
  for (sfi = static_functions.begin(); sfi != static_functions.end(); sfi++) {
    out << "    // Static Method " << methodNameFromCppName(sfi->second, export_class_name, false) << "\n";
    string name1 = methodNameFromCppName(sfi->second, export_class_name, false);
    string name2 = methodNameFromCppName(sfi->second, export_class_name, true);
    out << "    PyDict_SetItemString(Dtool_" << ClassName
        << ".As_PyTypeObject().tp_dict, \"" << name1
        << "\", PyCFunction_New(&Dtool_Methods_" << ClassName << "[" << sfi->first
        << "], &Dtool_" << ClassName << ".As_PyObject()));\n";
    if (name1 != name2) {
      out << "    PyDict_SetItemString(Dtool_" << ClassName
          << ".As_PyTypeObject().tp_dict, \"" << name2
          << "\", PyCFunction_New(&Dtool_Methods_" << ClassName << "[" << sfi->first
          << "], &Dtool_" << ClassName << ".As_PyObject()));\n";
    }
  }

  bool is_runtime_typed = IsPandaTypedObject(obj->_itype._cpptype->as_struct_type());
  if (HasAGetClassTypeFunction(obj->_itype)) {
    is_runtime_typed = true;
  }

  if (is_runtime_typed) {
    out << "    RegisterRuntimeClass(&Dtool_" << ClassName << ", " << cClassName << "::get_class_type().get_index());\n";
  } else {
    out << "    RegisterRuntimeClass(&Dtool_" << ClassName << ", -1);\n";
  }

  out << "  }\n";

  out << "  if (module != NULL) {\n";
  out << "    Py_INCREF(&Dtool_" << ClassName << ".As_PyTypeObject());\n";
  out << "    PyModule_AddObject(module, \"" << export_class_name << "\", (PyObject *)&Dtool_" << ClassName << ".As_PyTypeObject());\n";
  if (export_class_name != export_class_name2) {
    out << "    PyModule_AddObject(module, \"" << export_class_name2 << "\", (PyObject *)&Dtool_" << ClassName << ".As_PyTypeObject());\n";
  }

  // Also write out the explicit alternate names.
  int num_alt_names = obj->_itype.get_num_alt_names();
  for (int i = 0; i < num_alt_names; ++i) {
    string alt_name = make_safe_name(obj->_itype.get_alt_name(i));
    if (export_class_name != alt_name) {
      out << "    PyModule_AddObject(module, \"" << alt_name << "\", (PyObject *)&Dtool_" << ClassName << ".As_PyTypeObject());\n";
    }
  }

  out << "  }\n";
  out << "}\n\n";
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
string InterfaceMakerPythonNative::
get_unique_prefix() {
  return "Dtool_";
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::record_function_wrapper
//       Access: Protected, Virtual
//  Description: Associates the function wrapper with its function in
//               the appropriate structures in the database.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
record_function_wrapper(InterrogateFunction &ifunc, FunctionWrapperIndex wrapper_index) {
  ifunc._python_wrappers.push_back(wrapper_index);
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_prototype_for
//       Access: Private
//  Description: Writes the prototype for the indicated function.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_prototype_for(ostream &out, InterfaceMaker::Function *func) {
  std::string fname = "PyObject *" + func->_name + "(PyObject *self, PyObject *args)";
  write_prototype_for_name(out, func, fname);
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_prototype_for_name(ostream &out, InterfaceMaker::Function *func, const std::string &function_namename) {
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
    out << function_namename << ";\n";
//  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_function_for
//       Access: Private
//  Description: Writes the definition for a function that will call
//               the indicated C++ function or method.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_function_for_top(ostream &out, InterfaceMaker::Object *obj, InterfaceMaker::Function *func, const std::string &PreProcess) {
  std::string fname = "static PyObject *" + func->_name + "(PyObject *self, PyObject *args, PyObject *kwds)";

  bool coercion_attempted = false;
  write_function_for_name(out, obj, func, fname, PreProcess, "", true, coercion_attempted);
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
                        const std::string &ClassName, bool coercion_allowed, bool &coercion_attempted) {
  ostringstream forward_decl;
  ostringstream out;

  std::map<int, std::set<FunctionRemap *> > MapSets;
  std::map<int, std::set<FunctionRemap *> >::iterator mii;
  std::set<FunctionRemap *>::iterator sii;

  Function::Remaps::const_iterator ri;
  out1 << "/******************************************************************\n" << " * Python type method wrapper for\n";
  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    if (is_remap_legal(*remap)) {
      int parameter_size = remap->_parameters.size();
      if (remap->_has_this && remap->_type != FunctionRemap::T_constructor)
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

    indent(out, 2) << "int parameter_count = 1;\n";
    indent(out, 2) << "if (PyTuple_Check(args)) {\n";
    indent(out, 2) << "  parameter_count = PyTuple_Size(args);\n";
    indent(out, 2) << "  if (kwds != NULL && PyDict_Check(kwds)) {\n";
    indent(out, 2) << "    parameter_count += PyDict_Size(kwds);\n";
    indent(out, 2) << "  }\n";
    indent(out, 2) << "}\n";

    indent(out, 2) << "switch (parameter_count) {\n";
    for (mii = MapSets.begin(); mii != MapSets.end(); mii ++) {
      indent(out, 2) << "case " << mii->first << ": {\n";

      write_function_forset(out, obj, func, mii->second, expected_params, 4, forward_decl, is_inplace, coercion_allowed, coercion_attempted, "");
      if ((*mii->second.begin())->_type == FunctionRemap::T_constructor) {
        constructor = true;
      }

      indent(out, 4) << "}\n";
      indent(out, 4) << "break;\n";
    }

    indent(out, 2) << "default:\n";
    indent(out, 4) << "{\n";
    indent(out, 6)
      << "PyErr_Format(PyExc_TypeError, \""
      << methodNameFromCppName(func, "", false)
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
      indent(out, 6) << "return -1;\n";
    else
      indent(out, 6) << "return (PyObject *) NULL;\n";

    indent(out, 4) << "}\n";
    indent(out, 4) << "break;\n";
    indent(out, 2) << "}\n";

    out << "  if (!PyErr_Occurred()) { // Let error pass on\n";
    out << "    PyErr_SetString(PyExc_TypeError,\n";
    out << "      \"Arguments must match one of:\\n\"\n";
    output_quoted(out, 6, expected_params);
    out << ");\n";
    out << " }\n";

    if (constructor) {
      indent(out, 2) << "return -1;\n";
    } else {
      indent(out, 2) << "return (PyObject *) NULL;\n";
    }

    if (!expected_params.empty() && FunctionComment1.empty()) {
      FunctionComment1 += "C++ Interface:\n";
    }

    FunctionComment1 += expected_params;

  } else {
    string expected_params = "";
    for (mii = MapSets.begin(); mii != MapSets.end(); mii++) {
      write_function_forset(out, obj, func, mii->second, expected_params, 2, forward_decl, is_inplace, coercion_allowed, coercion_attempted, "");
      if ((*mii->second.begin())->_type == FunctionRemap::T_constructor) {
        constructor = true;
      }
    }

    out << "  if (!PyErr_Occurred()) {\n";
    out << "    PyErr_SetString(PyExc_TypeError,\n";
    out << "      \"Arguments must match:\\n\"\n";
    output_quoted(out, 6, expected_params);
    out << ");\n";
    out << "  }\n";
    if (constructor) {
      indent(out, 2) << "return -1;\n";
    } else {
      indent(out, 2) << "return (PyObject *) NULL;\n";
    }

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
    out << "static const char *" << func->_name << "_comment =\n";
    output_quoted(out, 2, FunctionComment);
    out << ";\n";
    out << "#else\n";
    out << "static const char *" << func->_name << "_comment = NULL;\n";
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
int GetParnetDepth(CPPType *type) {
  int answer = 0;
//  printf("    %s\n",type->get_local_name().c_str());

  if (TypeManager::is_basic_string_char(type)) {
  } else if (TypeManager::is_basic_string_wchar(type)) {
  } else if (TypeManager::is_bool(type)) {
  } else if (TypeManager::is_unsigned_longlong(type)) {
  } else if (TypeManager::is_longlong(type)) {
  } else if (TypeManager::is_integer(type)) {
  } else if (TypeManager::is_float(type)) {
  } else if (TypeManager::is_char_pointer(type)) {
  } else if (TypeManager::is_wchar_pointer(type)) {
  } else if (TypeManager::is_pointer_to_PyObject(type)) {
  } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
  } else if (TypeManager::is_pointer(type) || TypeManager::is_reference(type) || TypeManager::is_struct(type)) {
    ++answer;
    int deepest = 0;
    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)), false);
    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
    const InterrogateType &itype = idb->get_type(type_index);

    if (itype.is_class() || itype.is_struct()) {
      int num_derivations = itype.number_of_derivations();
      for (int di = 0; di < num_derivations; di++) {
        TypeIndex d_type_Index = itype.get_derivation(di);
        const InterrogateType &d_itype = idb->get_type(d_type_Index);
        int this_one = GetParnetDepth(d_itype._cpptype);
        if (this_one > deepest) {
          deepest = this_one;
        }
      }
    }
    answer += deepest;
//    printf(" Class Name %s  %d\n",itype.get_name().c_str(),answer);
  }

//  printf(" Class Name %s  %d\n",itype.get_name().c_str(),answer);
  return answer;
}

////////////////////////////////////////////////////////
//  The Core sort function for remap calling orders..
//////////////////////////////////////////////////////////
int RemapCompareLess(FunctionRemap *in1, FunctionRemap *in2) {
  if (in1->_parameters.size() != in2->_parameters.size()) {
     return (in1->_parameters.size() > in2->_parameters.size());
  }

  int pcount = in1->_parameters.size();
  for (int x = 0; x< pcount; x++) {
    CPPType *orig_type1 = in1->_parameters[x]._remap->get_orig_type();
    CPPType *orig_type2 = in2->_parameters[x]._remap->get_orig_type();

    int pd1 = GetParnetDepth(orig_type1);
    int pd2 = GetParnetDepth(orig_type2);
    if (pd1 != pd2) {
      return pd1 > pd2;
    }
  }

  // ok maybe something to do with return strength..

  return false;
}

//////////////////////////////////////////////////////////
//  Convience for the sort behavior..
///////////////////////////////////////////////////////////
std::vector<FunctionRemap *>
SortFunctionSet(std::set<FunctionRemap *> &remaps) {
  std::vector<FunctionRemap *> out;
  for (std::set<FunctionRemap *>::iterator ii = remaps.begin(); ii!= remaps.end(); ii++) {
    out.push_back(*ii);
  }

  std::sort(out.begin(), out.end(), RemapCompareLess);
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
                      std::set<FunctionRemap *> &remapsin,
                      string &expected_params, int indent_level,
                      ostream &forward_decl, bool is_inplace,
                      bool coercion_allowed,
                      bool &coercion_attempted,
                      const string &args_cleanup) {
  // Do we accept any parameters that are class objects?  If so, we
  // might need to check for parameter coercion.
  bool coercion_possible = false;
  if (coercion_allowed) {
    std::set<FunctionRemap *>::const_iterator sii;
    for (sii = remapsin.begin(); sii != remapsin.end() && !coercion_possible; ++sii) {
      FunctionRemap *remap = (*sii);
      if (is_remap_legal(*remap)) {
        int pn = 0;
        if (remap->_has_this) {
          // Skip the "this" parameter.  It's never coercible.
          ++pn;
        }
        while (pn < (int)remap->_parameters.size()) {
          CPPType *type = remap->_parameters[pn]._remap->get_new_type();

          if (TypeManager::is_char_pointer(type)) {
          } else if (TypeManager::is_wchar_pointer(type)) {
          } else if (TypeManager::is_pointer_to_PyObject(type)) {
          } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
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
  }

  if (coercion_possible) {
    // These objects keep track of whether we have attempted automatic
    // parameter coercion.
    indent(out, indent_level)
      << "{\n";
    indent_level += 2;
    indent(out, indent_level)
      << "PyObject *coerced = NULL;\n";
    indent(out, indent_level)
      << "PyObject **coerced_ptr = NULL;\n";
    indent(out, indent_level)
      << "bool report_errors = false;\n";
    indent(out, indent_level)
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
      if (is_remap_legal(*remap)) {
        if (remap->_has_this && !remap->_const_method) {
          // If it's a non-const method, we only allow a
          // non-const this.
          indent(out, indent_level)
            << "if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
        } else {
          indent(out, indent_level)
            << "{\n";
        }

        indent(out, indent_level) << "// -2 ";
        remap->write_orig_prototype(out, 0); out << "\n";

        write_function_instance(out, obj, func, remap, expected_params, indent_level + 2, false, forward_decl, func->_name, is_inplace, coercion_possible, coercion_attempted, args_cleanup);

        indent(out, indent_level + 2) << "PyErr_Clear();\n";
        indent(out, indent_level) << "}\n\n";
      }
    }
  } else {
    // There is only one possible overload with this number of
    // parameters.  Just call it.
    std::set<FunctionRemap *>::iterator sii;
    for (sii = remapsin.begin(); sii != remapsin.end(); sii ++) {
      FunctionRemap *remap = (*sii);
      if (is_remap_legal(*remap)) {
        if (remap->_has_this && !remap->_const_method) {
          // If it's a non-const method, we only allow a
          // non-const this.
          indent(out, indent_level)
            << "if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
        } else {
          indent(out, indent_level)
            << "{\n";
        }

        indent(out, indent_level + 2)
          << "// 1-" ;
        remap->write_orig_prototype(out, 0);
        out << "\n" ;
        write_function_instance(out, obj, func, remap, expected_params, indent_level + 2, true, forward_decl, func->_name, is_inplace, coercion_possible, coercion_attempted, args_cleanup);

        if (remap->_has_this && !remap->_const_method) {
          indent(out, indent_level)
            << "} else {\n";
          indent(out, indent_level + 2)
            << "PyErr_SetString(PyExc_TypeError,\n";
          string class_name = remap->_cpptype->get_simple_name();
          indent(out, indent_level + 2)
            << "                \"Cannot call "
            << classNameFromCppName(class_name, false)
            << "." << methodNameFromCppName(func, class_name, false)
            << "() on a const object.\");\n";
          if (!args_cleanup.empty()) {
            indent(out, indent_level + 2)
              << args_cleanup << "\n";
          }
          indent(out, indent_level + 2)
            << "return (PyObject *) NULL;\n";
          indent(out, indent_level)
            << "}\n\n";
        } else {
          indent(out, indent_level)
            << "}\n\n";
        }
      }
    }
  }

  // Now we've tried all of the possible overloads, and had no luck.

  if (coercion_possible) {
    // Try again, this time with coercion enabled.
    indent(out, indent_level)
      << "if (coerced_ptr == NULL && !report_errors) {\n";
    indent(out, indent_level + 2)
      << "coerced_ptr = &coerced;\n";
    indent(out, indent_level + 2)
      << "continue;\n";
    indent(out, indent_level)
      << "}\n";

    // No dice.  Go back one more time, and this time get the error
    // message.
    indent(out, indent_level)
      << "if (!report_errors) {\n";
    indent(out, indent_level + 2)
      << "report_errors = true;\n";
    indent(out, indent_level + 2)
      << "continue;\n";
    indent(out, indent_level)
      << "}\n";

    // We've been through three times.  We're done.
    indent(out, indent_level)
      << "break;\n";

    indent_level -= 2;
    indent(out, indent_level)
      << "}\n";

    indent(out, indent_level)
      << "Py_XDECREF(coerced);\n";
    indent_level -= 2;
    indent(out, indent_level)
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
                        bool coercion_possible, bool &coercion_attempted,
                        const string &args_cleanup) {
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

  if (is_constructor && (remap->_flags & FunctionRemap::F_explicit_self) != 0) {
    // If we'll be passing "self" to the constructor, we need to
    // pre-initialize it here.  Unfortunately, we can't pre-load the
    // "this" pointer, but the constructor itself can do this.

    indent(out, indent_level)
      << "// Pre-initialize self for the constructor\n";

    CPPType *orig_type = remap->_return_type->get_orig_type();
    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)), false);
    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
    const InterrogateType &itype = idb->get_type(type_index);
    indent(out, indent_level)
      << "DTool_PyInit_Finalize(self, NULL, &"
      << CLASS_PREFIX << make_safe_name(itype.get_scoped_name())
      << ", false, false);\n";
  }

  // Make one pass through the parameter list.  We will output a
  // one-line temporary variable definition for each parameter, while
  // simultaneously building the ParseTuple() function call and also
  // the parameter expression list for call_function().

  expected_params += methodNameFromCppName(func1, "", false);
  expected_params += "(";

  int pn;
  for (pn = 0; pn < (int)remap->_parameters.size(); pn++) {
    if (pn > 0) {
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
      keyword_list += "(char *)\"" + remap->_parameters[pn]._name + "\", ";
    }

    if (remap->_parameters[pn]._remap->new_type_is_atomic_string()) {
      if (TypeManager::is_char_pointer(orig_type)) {
        indent(out, indent_level) << "char *" << param_name << ";\n";
        format_specifiers += "s";
        parameter_list += ", &" + param_name;

      } else if (TypeManager::is_wchar_pointer(orig_type)) {
        out << "#if PY_MAJOR_VERSION >= 3\n";
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        out << "#else\n";
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
        out << "#endif\n";

        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        extra_convert += " int " + param_name + "_len = PyUnicode_GetSize((PyObject *)" + param_name + ");"
                         " wchar_t *" + param_name + "_str = new wchar_t[" + param_name + "_len + 1];"
                         " PyUnicode_AsWideChar(" + param_name + ", " + param_name + "_str, " + param_name + "_len);"
                         " " + param_name + "_str[" + param_name + "_len] = 0;";

        pexpr_string = param_name + "_str";
        extra_cleanup += " delete[] " + param_name + "_str;";

      } else if (TypeManager::is_wstring(orig_type)) {
        out << "#if PY_MAJOR_VERSION >= 3\n";
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        out << "#else\n";
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
        out << "#endif\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        extra_convert += " int " + param_name + "_len = PyUnicode_GetSize((PyObject *)" + param_name + ");"
                         " wchar_t *" + param_name + "_str = new wchar_t[" + param_name + "_len];"
                         " PyUnicode_AsWideChar(" + param_name + ", " + param_name + "_str, " + param_name + "_len);";

        pexpr_string = "basic_string<wchar_t>((wchar_t *)" +
          param_name + "_str, " +
          param_name + "_len)";

        extra_cleanup += " delete[] " + param_name + "_str;";

      } else if (TypeManager::is_const_ptr_to_basic_string_wchar(orig_type)) {
        out << "#if PY_MAJOR_VERSION >= 3\n";
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        out << "#else\n";
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
        out << "#endif\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        extra_convert += " int " + param_name + "_len = PyUnicode_GetSize((PyObject *)" + param_name + ");"
                         " wchar_t *" + param_name + "_str = new wchar_t[" + param_name + "_len];"
                         " PyUnicode_AsWideChar(" + param_name + ", " + param_name + "_str, " + param_name + "_len);";

        pexpr_string = "&basic_string<wchar_t>((wchar_t *)" +
          param_name + "_str, " +
          param_name + "_len)";

        extra_cleanup += " delete[] " + param_name + "_str;";

      } else if (TypeManager::is_const_ptr_to_basic_string_char(orig_type)) {
        indent(out, indent_level) << "char *" << param_name << "_str;\n";
        indent(out, indent_level) << "int " << param_name << "_len;\n";
        format_specifiers += "s#";
        parameter_list += ", &" + param_name
          + "_str, &" + param_name + "_len";
        pexpr_string = "&basic_string<char>(" +
          param_name + "_str, " +
          param_name + "_len)";

      } else {
        indent(out, indent_level) << "char *" << param_name << "_str;\n";
        indent(out, indent_level) << "int " << param_name << "_len;\n";
        format_specifiers += "s#";
        parameter_list += ", &" + param_name
          + "_str, &" + param_name + "_len";
        pexpr_string = "basic_string<char>(" +
          param_name + "_str, " +
          param_name + "_len)";
      }
      expected_params += "string";

    } else if (TypeManager::is_bool(type)) {
      indent(out, indent_level) << "PyObject *" << param_name << ";\n";
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      pexpr_string = "(PyObject_IsTrue(" + param_name + ") != 0)";
      expected_params += "bool";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_unsigned_longlong(type)) {
      indent(out, indent_level) << "PyObject *" << param_name << ";\n";
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += "PyObject *" + param_name + "_long = PyNumber_Long(" + param_name + ");";
      extra_param_check += " || (" + param_name + "_long == NULL)";
      pexpr_string = "PyLong_AsUnsignedLongLong(" + param_name + "_long)";
      extra_cleanup += "Py_XDECREF(" + param_name + "_long);";
      expected_params += "unsigned long long";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_longlong(type)) {
      indent(out, indent_level) << "PyObject *" << param_name << ";\n";
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += "PyObject *" + param_name + "_long = PyNumber_Long(" + param_name + ");";
      extra_param_check += " || (" + param_name + "_long == NULL)";
      pexpr_string = "PyLong_AsLongLong(" + param_name + "_long)";
      extra_cleanup += "Py_XDECREF(" + param_name + "_long);";
      expected_params += "long long";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_unsigned_integer(type)) {
      indent(out, indent_level) << "PyObject *" << param_name << ";\n";
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += "PyObject *" + param_name + "_uint = PyNumber_Long(" + param_name + ");";
      extra_param_check += " || (" + param_name + "_uint == NULL)";
      pexpr_string = "PyLong_AsUnsignedLong(" + param_name + "_uint)";
      extra_cleanup += "Py_XDECREF(" + param_name + "_uint);";
      expected_params += "unsigned int";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_integer(type)) {
      indent(out, indent_level) << "int " << param_name << ";\n";
      format_specifiers += "i";
      parameter_list += ", &" + param_name;
      expected_params += "int";

    } else if (TypeManager::is_float(type)) {
      indent(out, indent_level) << "double " << param_name << ";\n";
      format_specifiers += "d";
      parameter_list += ", &" + param_name;
      expected_params += "float";

    } else if (TypeManager::is_char_pointer(type)) {
      indent(out, indent_level) << "char *" << param_name << ";\n";
      format_specifiers += "s";
      parameter_list += ", &" + param_name;
      expected_params += "string";

    } else if (TypeManager::is_pointer_to_PyObject(type)) {
      indent(out, indent_level) << "PyObject *" << param_name << ";\n";
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      pexpr_string = param_name;
      pname_for_pyobject += param_name;
      expected_params += "any";

    } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
      indent(out, indent_level) << "PyObject *" << param_name << ";\n";
      format_specifiers += "O";
      parameter_list += ", &" + param_name;
      extra_convert += "PyObject *" + param_name + "_buffer = PyMemoryView_FromObject(" + param_name + ");";
      extra_param_check += " || (" + param_name + "_buffer == NULL)";
      pexpr_string = "PyMemoryView_GET_BUFFER(" + param_name + "_buffer)";
      extra_cleanup += "Py_XDECREF(" + param_name + "_buffer);";
      expected_params += "memoryview";
      pname_for_pyobject += param_name;

    } else if (TypeManager::is_pointer(type)) {
      CPPType *obj_type = TypeManager::unwrap(TypeManager::resolve_type(type));
      bool const_ok = !TypeManager::is_non_const_pointer_or_ref(orig_type);

      if (const_ok) {
        expected_params += "const ";
      } else {
        expected_params += "non-const ";
      }
      expected_params += classNameFromCppName(obj_type->get_simple_name(), false);

      if (!remap->_has_this || pn != 0) {
        indent(out, indent_level)
          << "PyObject *" << param_name << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
        pname_for_pyobject += param_name;

        TypeIndex p_type_index = builder.get_type(obj_type, false);
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
        extra_convert += p_itype.get_scoped_name() + " *" + param_name + "_this = (" + p_itype.get_scoped_name()+" *)";
        // need to a forward scope for this class..
        if (!isExportThisRun(p_itype._cpptype)) {
          _external_imports.insert(make_safe_name(p_itype.get_scoped_name()));
          //ForwardDeclrs << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << make_safe_name(p_itype.get_scoped_name()) << ";\n";
        }

        string class_name;
        string method_prefix;
        if (remap->_cpptype) {
          class_name = remap->_cpptype->get_simple_name();
          method_prefix = classNameFromCppName(class_name, false) + string(".");
        }

        ostringstream str;
        str << "DTOOL_Call_GetPointerThisClass(" << param_name
            << ", &Dtool_" << make_safe_name(p_itype.get_scoped_name())
            << ", " << pn << ", \""
            << method_prefix << methodNameFromCppName(func1, class_name, false)
            << "\", " << const_ok;
        if (coercion_possible && !is_copy_constructor) {
          // We never attempt to coerce a copy constructor parameter.
          // That would lead to infinite recursion.
          str << ", coerced_ptr, report_errors";
          coercion_attempted = true;
        } else {
          str << ", NULL, true";
        }
        str << ");\n";

        extra_convert += str.str();
        extra_param_check += " || (" + param_name + "_this == NULL)";
        pexpr_string =  param_name + "_this";
      }

    } else {
      // Ignore a parameter.
      indent(out, indent_level) << "PyObject *" << param_name << ";\n";
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
    }

    pexprs.push_back(pexpr_string);
  }
  expected_params += ")\n";

  // If we got what claimed to be a unary operator, don't check for
  // parameters, since we won't be getting any anyway.
  if (!func1->_ifunc.is_unary_op()) {
    std::string format_specifiers1 = format_specifiers + ":" +
      methodNameFromCppName(func1, "", false);
    indent(out, indent_level)
      << "static char *keyword_list[] = {" << keyword_list << "NULL};\n";

    if (remap->_parameters.size() == 1 ||
        (remap->_has_this && remap->_parameters.size() == 2)) {
      indent(out, indent_level)
        << "// Special case to make operators work\n";
      indent(out, indent_level)
        << "if (PyTuple_Check(args) || (kwds != NULL && PyDict_Check(kwds))) {\n";
      indent(out, indent_level)
        << "  PyArg_ParseTupleAndKeywords(args, kwds, \""
        << format_specifiers1 << "\", keyword_list" << parameter_list
        << ");\n";
      indent(out, indent_level)
        << "} else {\n";
      indent(out, indent_level)
        << "  PyArg_Parse(args, \"" << format_specifiers1 << "\""
        << parameter_list << ");\n";
      indent(out, indent_level)
        << "}\n";
      indent(out, indent_level)
        << "if (!PyErr_Occurred()) {\n";

    } else {
      indent(out, indent_level)
        << "if (PyArg_ParseTupleAndKeywords(args, kwds, \""
        << format_specifiers1 << "\", keyword_list"
        << parameter_list << ")) {\n";
    }
  } else {
    indent(out, indent_level) << "{\n";
  }

  if (!extra_convert.empty()) {
    indent(out, indent_level + 2)
      << extra_convert << "\n";
  }

  int extra_indent_level = indent_level + 2;

  if (!extra_param_check.empty()) {
    indent(out, extra_indent_level)
      << "if (!(" << extra_param_check.substr(3) << ")) {\n";
    extra_indent_level += 2;
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
    if (!args_cleanup.empty()) {
      indent(out, extra_indent_level + 2)
        << args_cleanup << "\n";
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
      indent(out, extra_indent_level) << "in_interpreter = 0;\n";
    }
    string tt;
    string return_expr = remap->call_function(out, extra_indent_level, false, container, pexprs);
    CPPType *type = remap->_return_type->get_orig_type();
    indent(out, extra_indent_level);
    type->output_instance(out, "return_value", &parser);
    //    type->output_instance(tt, "return_value", &parser);
    out << " = " << return_expr << ";\n";

    if (track_interpreter) {
      indent(out, extra_indent_level) << "in_interpreter = 1;\n";
    }
    if (remap->_blocking) {
      out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
      indent(out, extra_indent_level)
        << "Py_BLOCK_THREADS\n";
      out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
    }
    if (!extra_cleanup.empty()) {
      indent(out, extra_indent_level) << extra_cleanup << "\n";
    }

    return_expr = manage_return_value(out, 4, remap, "return_value");
    do_assert_init(out, extra_indent_level, is_constructor, args_cleanup);
    pack_return_value(out, extra_indent_level, remap, return_expr, is_inplace);

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
      indent(out, extra_indent_level) << "in_interpreter = 0;\n";
    }

    string return_expr = remap->call_function(out, extra_indent_level, true, container, pexprs);
    if (return_expr.empty()) {
      if (track_interpreter) {
        indent(out, extra_indent_level) << "in_interpreter = 1;\n";
      }
      if (remap->_blocking) {
        out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
        indent(out, extra_indent_level)
          << "Py_BLOCK_THREADS\n";
        out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
      }
      if (!extra_cleanup.empty()) {
        indent(out, extra_indent_level) << extra_cleanup << "\n";
      }
      if (coercion_possible) {
        indent(out, extra_indent_level)
          << "Py_XDECREF(coerced);\n";
      }
      do_assert_init(out, extra_indent_level, is_constructor, args_cleanup);
      indent(out, extra_indent_level) << "return Py_BuildValue(\"\");\n";

    } else {
      CPPType *type = remap->_return_type->get_temporary_type();
      if (!is_inplace) {
        indent(out, extra_indent_level);
        type->output_instance(out, "return_value", &parser);
        out << " = " << return_expr << ";\n";
      }
      if (track_interpreter) {
        indent(out, extra_indent_level) << "in_interpreter = 1;\n";
      }
      if (remap->_blocking) {
        out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
        indent(out, extra_indent_level)
          << "Py_BLOCK_THREADS\n";
        out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
      }
      if (!extra_cleanup.empty()) {
        indent(out, extra_indent_level) << extra_cleanup << "\n";
      }

      if (!is_inplace) {
        return_expr = manage_return_value(out, extra_indent_level, remap, "return_value");
      }
      if (coercion_possible) {
        indent(out, extra_indent_level)
          << "Py_XDECREF(coerced);\n";
      }
      do_assert_init(out, extra_indent_level, is_constructor, args_cleanup);
      pack_return_value(out, extra_indent_level, remap, remap->_return_type->temporary_to_return(return_expr), is_inplace);
    }
  }

  // Close the extra brace opened within do_assert_init().
  extra_indent_level -= 2;
  indent(out, extra_indent_level)
    << "}\n";

  if (!extra_param_check.empty()) {
    extra_indent_level -= 2;
    indent(out, extra_indent_level) << "}\n";
  }

  indent(out, indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::pack_return_value
//       Access: Private
//  Description: Outputs a command to pack the indicated expression,
//               of the return_type type, as a Python return value.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
pack_return_value(ostream &out, int indent_level, FunctionRemap *remap,
                  const string &return_expr, bool is_inplace) {

  if (remap->_type == FunctionRemap::T_constructor) {
    // should only reach this in the INIT function a a Class .. IE the PY exists before the CPP object
    // this is were we type to returned a class/struct.. ie CPP Type
    CPPType *orig_type = remap->_return_type->get_orig_type();

    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)), false);
    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
    const InterrogateType &itype = idb->get_type(type_index);
    indent(out, indent_level)
      << "return DTool_PyInit_Finalize(self, " << return_expr << ", &" << CLASS_PREFIX  << make_safe_name(itype.get_scoped_name()) << ", true, false);\n";

  } else {
    ParameterRemap *return_type = remap->_return_type;
    pack_python_value(out, indent_level, remap, return_type, return_expr, "", is_inplace);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::pack_python_value
//       Access: Private
//  Description: Outputs a command to pack the indicated expression,
//               of the return_type type, as a Python value.
//               If assign_to is empty, the Python object is
//               returned.  Otherwise, it is assigned to a variable
//               of that name (expected to already be declared).
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
pack_python_value(ostream &out, int indent_level, FunctionRemap *remap,
                  ParameterRemap *return_type, const string &return_expr, const string &assign_to, bool is_inplace) {
  CPPType *orig_type = return_type->get_orig_type();
  CPPType *type = return_type->get_new_type();

  string assign_stmt("return ");
  if (!assign_to.empty()) {
    assign_stmt = assign_to + " = ";
  }

  if (return_type->new_type_is_atomic_string()) {
    if (TypeManager::is_char_pointer(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
      indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
      indent(out, indent_level+2) << assign_stmt << "Py_None;\n";
      indent(out, indent_level) << "} else {\n";

      out << "#if PY_MAJOR_VERSION >= 3\n";
      indent(out, indent_level+2) << assign_stmt
        << "PyUnicode_FromString(" << return_expr << ");\n";
      out << "#else\n";
      indent(out, indent_level+2) << assign_stmt
        << "PyString_FromString(" << return_expr << ");\n";
      out << "#endif\n";

      indent(out, indent_level) << "}\n";

    } else if (TypeManager::is_wchar_pointer(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
      indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
      indent(out, indent_level+2) << assign_stmt << "Py_None;\n";
      indent(out, indent_level) << "} else {\n";
      indent(out, indent_level+2)
        << assign_stmt << "PyUnicode_FromWideChar("
        << return_expr << ", wcslen(" << return_expr << "));\n";
      indent(out, indent_level) << "}\n";

    } else if (TypeManager::is_wstring(orig_type)) {
      indent(out, indent_level) << assign_stmt
        << "PyUnicode_FromWideChar("
        << return_expr << ".data(), (int) " << return_expr << ".length());\n";

    } else if (TypeManager::is_const_ptr_to_basic_string_wchar(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
      indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
      indent(out, indent_level+2) << assign_stmt << "Py_None;\n";
      indent(out, indent_level) << "} else {\n";

      indent(out, indent_level+2) << assign_stmt
        << "PyUnicode_FromWideChar("
        << return_expr << "->data(), (int) " << return_expr << "->length());\n";

      indent(out, indent_level) << "}\n";

    } else if (TypeManager::is_const_ptr_to_basic_string_char(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr<< " == NULL) {\n";
      indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
      indent(out, indent_level+2) << assign_stmt << "Py_None;\n";
      indent(out, indent_level) << "} else {\n";

      out << "#if PY_MAJOR_VERSION >= 3\n";
      indent(out, indent_level+2) << assign_stmt
        << "PyUnicode_FromStringAndSize("
        << return_expr << "->data(), (Py_ssize_t)" << return_expr << "->length());\n";
      out << "#else\n";
      indent(out, indent_level+2) << assign_stmt
        << "PyString_FromStringAndSize("
        << return_expr << "->data(), (Py_ssize_t)" << return_expr << "->length());\n";
      out << "#endif\n";

      indent(out, indent_level) << "}\n";

    } else {
      out << "#if PY_MAJOR_VERSION >= 3\n";
      indent(out, indent_level) << assign_stmt
        << "PyUnicode_FromStringAndSize("
        << return_expr << ".data(), (Py_ssize_t)" << return_expr << ".length());\n";
      out << "#else\n";
      indent(out, indent_level) << assign_stmt
        << "PyString_FromStringAndSize("
        << return_expr << ".data(), (Py_ssize_t)" << return_expr << ".length());\n";
      out << "#endif\n";
    }

  } else if (TypeManager::is_bool(type)) {
    indent(out, indent_level) << assign_stmt
      << "PyBool_FromLong(" << return_expr << ");\n";

  } else if (TypeManager::is_unsigned_longlong(type)) {
    indent(out, indent_level) << assign_stmt
      << "PyLong_FromUnsignedLongLong(" << return_expr << ");\n";

  } else if (TypeManager::is_longlong(type)) {
    indent(out, indent_level) << assign_stmt
      << "PyLong_FromLongLong(" << return_expr << ");\n";

  } else if (TypeManager::is_unsigned_integer(type)){
    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level) << assign_stmt
      << "PyLong_FromUnsignedLong(" << return_expr << ");\n";
    out << "#else\n";
    indent(out, indent_level) << assign_stmt
      << "PyLongOrInt_FromUnsignedLong(" << return_expr << ");\n";
    out << "#endif\n";

  } else if (TypeManager::is_integer(type)) {
    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level) << assign_stmt
      << "PyLong_FromLong(" << return_expr << ");\n";
    out << "#else\n";
    indent(out, indent_level) << assign_stmt
      << "PyInt_FromLong(" << return_expr << ");\n";
    out << "#endif\n";

  } else if (TypeManager::is_float(type)) {
    indent(out, indent_level) << assign_stmt
      << "PyFloat_FromDouble(" << return_expr << ");\n";

  } else if (TypeManager::is_char_pointer(type)) {
    indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
    indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
    indent(out, indent_level+2) << assign_stmt << "Py_None;\n";
    indent(out, indent_level) << "} else {\n";

    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level+2) << assign_stmt
      << "PyUnicode_FromString(" << return_expr << ");\n";
    out << "#else\n";
    indent(out, indent_level+2) << assign_stmt
      << "PyString_FromString(" << return_expr << ");\n";
    out << "#endif\n";

    indent(out, indent_level) << "}\n";

  } else if (TypeManager::is_wchar_pointer(type)) {
    indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
    indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
    indent(out, indent_level+2) << assign_stmt << "Py_None;\n";
    indent(out, indent_level) << "} else {\n";
    indent(out, indent_level+2) << assign_stmt
      << "PyUnicode_FromWideChar("
      << return_expr << ", wcslen(" << return_expr << "));\n";

    indent(out, indent_level) << "}\n";

  } else if (TypeManager::is_pointer_to_PyObject(type)) {
    indent(out, indent_level)
      << assign_stmt << return_expr << ";\n";

  } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
    indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
    indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
    indent(out, indent_level+2) << assign_stmt << "Py_None;\n";
    indent(out, indent_level) << "} else {\n";
    indent(out, indent_level+2) << assign_stmt
      << "PyMemoryView_FromBuffer(" << return_expr << ");\n";
    indent(out, indent_level) << "}\n";

  } else if (TypeManager::is_pointer(type)) {
    string const_flag;
    if (TypeManager::is_const_pointer_to_anything(type)) {
      const_flag = "true";
    } else {
      const_flag = "false";
    }

    if (TypeManager::is_struct(orig_type) || TypeManager::is_ref_to_anything(orig_type)) {
      if (TypeManager::is_ref_to_anything(orig_type)) {
        TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)),false);
        InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
        const InterrogateType &itype = idb->get_type(type_index);
        std::string owns_memory_flag("true");

        if (remap->_return_value_needs_management) {
          owns_memory_flag = "true";
        } else {
          owns_memory_flag = "false";
        }

        if (!isExportThisRun(itype._cpptype)) {
          _external_imports.insert(make_safe_name(itype.get_scoped_name()));
          //ForwardDeclrs << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
        }

        write_python_instance(out, indent_level, return_expr, assign_to, owns_memory_flag, itype.get_scoped_name(), itype._cpptype, is_inplace, const_flag);

      } else {
        std::string owns_memory_flag("true");
        if (remap->_return_value_needs_management) {
          owns_memory_flag = "true";
        } else {
          owns_memory_flag = "false";
        }

        if (remap->_manage_reference_count) {
          TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)),false);
          InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
          const InterrogateType &itype = idb->get_type(type_index);

          if (!isExportThisRun(itype._cpptype)) {
            _external_imports.insert(make_safe_name(itype.get_scoped_name()));
            //ForwardDeclrs << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
          }

          //                    ForwardDeclrs << "extern  \"C\" struct Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
          write_python_instance(out, indent_level, return_expr, assign_to, owns_memory_flag, itype.get_scoped_name(), itype._cpptype, is_inplace, const_flag);
        } else {
          TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
          InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
          const InterrogateType &itype = idb->get_type(type_index);

          if (!isExportThisRun(itype._cpptype)) {
            _external_imports.insert(make_safe_name(itype.get_scoped_name()));
            //ForwardDeclrs << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
          }

          //                    ForwardDeclrs << "extern  \"C\" struct Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
          write_python_instance(out, indent_level, return_expr, assign_to, owns_memory_flag, itype.get_scoped_name(), itype._cpptype, is_inplace, const_flag);
        }
      }
    } else if (TypeManager::is_struct(orig_type->as_pointer_type()->_pointing_at)) {
      TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
      InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
      const InterrogateType &itype = idb->get_type(type_index);

      std::string owns_memory_flag("true");
      if (remap->_return_value_needs_management) {
        owns_memory_flag = "true";
      } else {
        owns_memory_flag = "false";
      }

      if (!isExportThisRun(itype._cpptype)) {
        _external_imports.insert(make_safe_name(itype.get_scoped_name()));
        //ForwardDeclrs << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
      }

      //        ForwardDeclrs << "extern  \"C\" struct Dtool_PyTypedObject Dtool_" << make_safe_name(itype.get_scoped_name()) << ";\n";
      write_python_instance(out, indent_level, return_expr, assign_to, owns_memory_flag, itype.get_scoped_name(), itype._cpptype, is_inplace, const_flag);

    } else {
      indent(out, indent_level) << "  Should Never Reach This InterfaceMakerPythonNative::pack_python_value";
          //<< "return PyInt_FromLong((int) " << return_expr << ");\n";
    }
  } else {
    // Return None.
    indent(out, indent_level)
      << assign_stmt << "Py_BuildValue(\"\");\n";
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
  string num_name = methodNameFromCppName(make_seq->_num_name, ClassName, false);
  string element_name = methodNameFromCppName(make_seq->_element_name, ClassName, false);

  out << "  return make_list_for_item(self, \"" << num_name
      << "\", \"" << element_name << "\");\n";
  out << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::record_object
//       Access: Protected
//  Description: Records the indicated type, which may be a struct
//               type, along with all of its associated methods, if
//               any.
////////////////////////////////////////////////////////////////////
InterfaceMaker::Object *InterfaceMakerPythonNative::
record_object(TypeIndex type_index) {
  if (type_index == 0) {
    return (Object *)NULL;
  }

  Objects::iterator oi = _objects.find(type_index);
  if (oi != _objects.end()) {
    return (*oi).second;
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  const InterrogateType &itype = idb->get_type(type_index);

  if (!is_cpp_type_legal(itype._cpptype)) {
    return (Object *)NULL;
  }

  Object *object = new Object(itype);
  bool inserted = _objects.insert(Objects::value_type(type_index, object)).second;
  assert(inserted);

  Function *function;

  int num_constructors = itype.number_of_constructors();
  for (int ci = 0; ci < num_constructors; ci++) {
    function = record_function(itype, itype.get_constructor(ci));
    if (is_function_legal(function)) {
      object->_constructors.push_back(function);
    }
  }

  int num_methods = itype.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; mi++) {
    function = record_function(itype, itype.get_method(mi));
    if (is_function_legal(function)) {
      object->_methods.push_back(function);
    }
  }

  int num_casts = itype.number_of_casts();
  for (mi = 0; mi < num_casts; mi++) {
    function = record_function(itype, itype.get_cast(mi));
    if (is_function_legal(function)) {
      object->_methods.push_back(function);
    }
  }

  int num_derivations = itype.number_of_derivations();
  for (int di = 0; di < num_derivations; di++) {
    TypeIndex d_type_Index = itype.get_derivation(di);
    idb->get_type(d_type_Index);

    if (!interrogate_type_is_unpublished(d_type_Index)) {
      if (itype.derivation_has_upcast(di)) {
        function = record_function(itype, itype.derivation_get_upcast(di));
        if (is_function_legal(function)) {
          object->_methods.push_back(function);
        }
      }
      if (itype.derivation_has_downcast(di)) {
        // Downcasts are methods of the base class, not the child class.
        TypeIndex base_type_index = itype.get_derivation(di);

        const InterrogateType &base_type = idb->get_type(base_type_index);
        function = record_function(base_type, itype.derivation_get_downcast(di));

        if (is_function_legal(function)) {
          Object * pobject = record_object(base_type_index);
          if (pobject != NULL) {
            pobject->_methods.push_back(function);
          }
        }
      }
    }
  }

  int num_elements = itype.number_of_elements();
  for (int ei = 0; ei < num_elements; ei++) {
    ElementIndex element_index = itype.get_element(ei);
    const InterrogateElement &ielement = idb->get_element(element_index);
    if (ielement.has_getter()) {
      FunctionIndex func_index = ielement.get_getter();
      record_function(itype, func_index);
    }
    if (ielement.has_setter()) {
      FunctionIndex func_index = ielement.get_setter();
      record_function(itype, func_index);
    }
  }

  object->check_protocols();

  int num_nested = itype.number_of_nested_types();
  for (int ni = 0; ni < num_nested; ni++) {
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
void InterfaceMakerPythonNative::
generate_wrappers() {
  inside_python_native = true;
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  // We use a while loop rather than a simple for loop, because we
  // might increase the number of types recursively during the
  // traversal.

  int ti = 0;
  while (ti < idb->get_num_all_types()) {
    TypeIndex type_index = idb->get_all_type(ti);
    record_object(type_index);
    ++ti;
  }

  int num_global_elements = idb->get_num_global_elements();
  for (int gi = 0; gi < num_global_elements; ++gi) {
    TypeIndex type_index = idb->get_global_element(gi);
    record_object(type_index);
  }

  int num_functions = idb->get_num_global_functions();
  for (int fi = 0; fi < num_functions; fi++) {
    FunctionIndex func_index = idb->get_global_function(fi);
    record_function(dummy_type, func_index);
  }

  int num_manifests = idb->get_num_global_manifests();
  for (int mi = 0; mi < num_manifests; mi++) {
    ManifestIndex manifest_index = idb->get_global_manifest(mi);
    const InterrogateManifest &iman = idb->get_manifest(manifest_index);
    if (iman.has_getter()) {
      FunctionIndex func_index = iman.get_getter();
      record_function(dummy_type, func_index);
    }
  }

  int num_elements = idb->get_num_global_elements();
  for (int ei = 0; ei < num_elements; ei++) {
    ElementIndex element_index = idb->get_global_element(ei);
    const InterrogateElement &ielement = idb->get_element(element_index);
    if (ielement.has_getter()) {
      FunctionIndex func_index = ielement.get_getter();
      record_function(dummy_type, func_index);
    }
    if (ielement.has_setter()) {
      FunctionIndex func_index = ielement.get_setter();
      record_function(dummy_type, func_index);
    }
  }
  inside_python_native = false;
}

//////////////////////////////////////////////
//   Function :is_cpp_type_legal
//
// is the cpp object  supported by by the dtool_py interface..
//////////////////////////////////////////////
bool InterfaceMakerPythonNative::
is_cpp_type_legal(CPPType *in_ctype) {
  if (in_ctype == NULL) {
    return false;
  }

  if (builder.in_ignoretype(in_ctype->get_local_name(&parser))) {
    return false;
  }

  //bool answer = false;
  CPPType *type = TypeManager::unwrap(TypeManager::resolve_type(in_ctype));
  type = TypeManager::unwrap(type);
  //CPPType *type =  ctype;

  if (TypeManager::is_basic_string_char(type)) {
    return true;
  } else if (TypeManager::is_basic_string_wchar(type)) {
    return true;
  } else if (TypeManager::is_simple(type)) {
    return true;
  } else if (builder.in_forcetype(type->get_local_name(&parser))) {
    return true;
  } else if (TypeManager::IsExported(type)) {
    return  true;
  } else if (TypeManager::is_pointer_to_PyObject(in_ctype)) {
    return true;
  } else if (TypeManager::is_pointer_to_Py_buffer(in_ctype)) {
    return true;
  }

  //if (answer == false)
//        printf(" -------------------- Bad Type ?? %s\n",type->get_local_name().c_str());

  return false;
}
////////////////////////////////////////////// 
//   Function :isExportThisRun
//
////////////////////////////////////////////// 
bool InterfaceMakerPythonNative::
isExportThisRun(CPPType *ctype) {
  CPPType *type = TypeManager::unwrap(ctype);
  if (TypeManager::IsLocal(type)) {
    return true;
  }

  if (builder.in_forcetype(type->get_local_name(&parser))) {
    return true;
  }

  return false;
}

////////////////////////////////////////////// 
// Function : isExportThisRun
/////////////////////////////////////////////
bool InterfaceMakerPythonNative::
isExportThisRun(Function *func) {
  if (func == NULL || !is_function_legal(func)) {
    return false;
  }

  Function::Remaps::const_iterator ri;
  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    return isExportThisRun(remap->_cpptype);
  }

  return false;
}

////////////////////////////////////////////// 
// Function : is_remap_legal
//////////////////////////////////////////////
bool InterfaceMakerPythonNative::
is_remap_legal(FunctionRemap &remap) {
  // return must be legal and managable..
  if (!is_cpp_type_legal(remap._return_type->get_orig_type())) {
//        printf("  is_remap_legal Return Is Bad %s\n",remap._return_type->get_orig_type()->get_fully_scoped_name().c_str());
    return false;
  }

  // ouch .. bad things will happen here ..  do not even try..
  if (remap._ForcedVoidReturn) {
    return false;
  }

  // all params must be legal
  for (int pn = 0; pn < (int)remap._parameters.size(); pn++) {
    CPPType *orig_type = remap._parameters[pn]._remap->get_orig_type();
    if (!is_cpp_type_legal(orig_type)) {
      return false;
    }
  }

  // ok all looks ok.
  return true;
}

////////////////////////////////////////////////////////////////////////
// Function  : is_function_legal
////////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::
is_function_legal(Function *func) {
  Function::Remaps::const_iterator ri;
  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    if (is_remap_legal(*remap)) {
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
bool InterfaceMakerPythonNative::
isFunctionWithThis(Function *func) {
  Function::Remaps::const_iterator ri;
  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    if (remap->_has_this) {
      return true;
    }
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
void InterfaceMakerPythonNative::do_assert_init(ostream &out, int &indent_level, bool constructor, const string &args_cleanup) const {
  // If a method raises TypeError, continue.
  indent(out, indent_level)
    << "if (PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_TypeError)) {\n";
  indent(out, indent_level + 2)
    << "// TypeError raised; continue to next overload type.\n";
  if (constructor) {
    indent(out, indent_level + 2)
      << "delete return_value;\n";
  }
  indent(out, indent_level)
    << "} else {\n";
  indent_level += 2;

  if (!args_cleanup.empty()) {
    indent(out, indent_level)
      << args_cleanup << "\n";
  }

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
    if (constructor) {
      indent(out, indent_level + 2) << "delete return_value;\n";
      indent(out, indent_level + 2) << "return -1;\n";
    } else {
      indent(out, indent_level + 2) << "return (PyObject *)NULL;\n";
    }

    indent(out, indent_level)
      << "}\n";
    out << "#endif\n";
    indent(out, indent_level)
      << "if (PyErr_Occurred()) {\n";
    if (constructor) {
      indent(out, indent_level + 2) << "delete return_value;\n";
      indent(out, indent_level + 2) << "return -1;\n";
    } else {
      indent(out, indent_level + 2) << "return (PyObject *)NULL;\n";
    }

    indent(out, indent_level)
      << "}\n";
  }
}
////////////////////////////////////////////////////////
// Function :  IsRunTimeTyped
///////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::
IsRunTimeTyped(const InterrogateType &itype) {
  TypeIndex ptype_id = itype.get_outer_class();
  if (ptype_id > 0) {
    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

    InterrogateType ptype = idb->get_type(ptype_id);
    return IsRunTimeTyped(ptype);
  }

  if (itype.get_name() == "TypedObject") {
    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////
// Function : DoesInheritFromIsClass
//
// Helper function to check cpp class inharatience..
///////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::
DoesInheritFromIsClass(const CPPStructType *inclass, const std::string &name) {
  if (inclass == NULL) {
    return false;
  }

  std::string scoped_name = inclass->get_fully_scoped_name();
  if (scoped_name == name) {
    return true;
  }

  CPPStructType::Derivation::const_iterator bi;
  for (bi = inclass->_derivation.begin();
      bi != inclass->_derivation.end();
      ++bi) {

    const CPPStructType::Base &base = (*bi);

    CPPStructType *base_type = TypeManager::resolve_type(base._base)->as_struct_type();
    if (base_type != NULL) {
      if (DoesInheritFromIsClass(base_type, name)) {
        return true;
      }
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
//  Function : HasAGetKeyFunction
//
// does the class have a supportable get_key() or get_hash() to return
// a usable Python hash?  Returns the name of the method, or empty
// string if there is no suitable method.
//////////////////////////////////////////////////////////////////////////////////////////
string InterfaceMakerPythonNative::
HasAGetKeyFunction(const InterrogateType &itype_class) {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  int num_methods = itype_class.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; mi++) {
    FunctionIndex func_index = itype_class.get_method(mi);
    const InterrogateFunction &ifunc = idb->get_function(func_index);
    if (ifunc.get_name() == "get_key" || ifunc.get_name() == "get_hash") {
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
                TypeManager::is_integer(cppfunc->_return_type)) {
              if (cppfunc->_parameters->_parameters.size() == 0) {
                return ifunc.get_name();
              }
            }
          }
        }
      }
    }
  }
  return string();
}

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
              if (cppfunc->_parameters->_parameters.size() == 0) {
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
//
//               Returns 3 if the class defines an extension
//               function for python_repr(ostream, string).
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
                    if ((cppinst->_storage_class & CPPInstance::SC_extension) != 0) {
                      return 3;
                    } else {
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
//     Function: InterfaceMakerPythonNative::NeedsARichCompareFunction
//       Access: Private
//  Description: Returns true if the class defines a rich comparison
//               operator.
////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::
NeedsARichCompareFunction(const InterrogateType &itype_class) {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  int num_methods = itype_class.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; ++mi) {
    FunctionIndex func_index = itype_class.get_method(mi);
    const InterrogateFunction &ifunc = idb->get_function(func_index);
    if (ifunc.get_name() == "operator <") {
      return true;
    }
    if (ifunc.get_name() == "operator <=") {
      return true;
    }
    if (ifunc.get_name() == "operator ==") {
      return true;
    }
    if (ifunc.get_name() == "operator !=") {
      return true;
    }
    if (ifunc.get_name() == "operator >") {
      return true;
    }
    if (ifunc.get_name() == "operator >=") {
      return true;
    }
  }

  return false;
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
