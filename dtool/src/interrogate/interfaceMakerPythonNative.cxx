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

#include "pnotify.h" // For nout
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
#include "lineStream.h"

#include <set>
#include <map>

extern bool     inside_python_native;
extern          InterrogateType dummy_type;
extern std::string EXPORT_IMPORT_PREFIX;

#define CLASS_PREFIX "Dtool_"

/////////////////////////////////////////////////////////
// Name Remapper...
//      Snagged from ffi py code....
/////////////////////////////////////////////////////////
struct RenameSet {
  const char *_from;
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
  if (!mangle_names) {
    mangle = false;
  }

  //# initialize to empty string
  std::string className = "";

  //# These are the characters we want to strip out of the name
  const std::string badChars("!@#$%^&*()<>,.-=+~{}? ");

  bool nextCap = false;
  bool nextUscore = false;
  bool firstChar = true && mangle;

  for (std::string::const_iterator chr = cppName.begin();
       chr != cppName.end(); ++chr) {
    if ((*chr == '_' || *chr == ' ') && mangle) {
      nextCap = true;

    } else if (badChars.find(*chr) != std::string::npos) {
      nextUscore = !mangle;

    } else if (nextCap || firstChar) {
      className += toupper(*chr);
      nextCap = false;
      firstChar = false;

    } else if (nextUscore) {
      className += '_';
      nextUscore = false;
      className += *chr;

    } else {
      className += *chr;
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
  if (!mangle_names) {
    mangle = false;
  }

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

std::string methodNameFromCppName(FunctionRemap *remap, const std::string &className, bool mangle) {
  std::string cppName = remap->_cppfunc->get_local_name();
  if (remap->_ftype->_flags & CPPFunctionType::F_unary_op) {
    cppName += "unary";
  }
  return methodNameFromCppName(cppName, className, mangle);
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
get_slotted_function_def(Object *obj, Function *func, FunctionRemap *remap, SlottedFunctionDef &def) {
  if (obj == NULL) {
    // Only methods may be slotted.
    return false;
  }

  def._func = func;
  def._answer_location = string();
  def._wrapper_type = WT_none;
  def._min_version = 0;

  string method_name = func->_ifunc.get_name();
  bool is_unary_op = func->_ifunc.is_unary_op();

  if (method_name == "operator +") {
    def._answer_location = "tp_as_number->nb_add";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator -" && is_unary_op) {
    def._answer_location = "tp_as_number->nb_negative";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "operator -") {
    def._answer_location = "tp_as_number->nb_subtract";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator *") {
    def._answer_location = "tp_as_number->nb_multiply";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator /") {
    def._answer_location = "tp_as_number->nb_divide";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator %") {
    def._answer_location = "tp_as_number->nb_remainder";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator <<") {
    def._answer_location = "tp_as_number->nb_lshift";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator >>") {
    def._answer_location = "tp_as_number->nb_rshift";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator ^") {
    def._answer_location = "tp_as_number->nb_xor";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator ~" && is_unary_op) {
    def._answer_location = "tp_as_number->nb_invert";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "operator &") {
    def._answer_location = "tp_as_number->nb_and";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator |") {
    def._answer_location = "tp_as_number->nb_or";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "__pow__") {
    def._answer_location = "tp_as_number->nb_power";
    def._wrapper_type = WT_ternary_operator;
    return true;
  }

  if (method_name == "operator +=") {
    def._answer_location = "tp_as_number->nb_inplace_add";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator -=") {
    def._answer_location = "tp_as_number->nb_inplace_subtract";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator *=") {
    def._answer_location = "tp_as_number->nb_inplace_multiply";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator /=") {
    def._answer_location = "tp_as_number->nb_inplace_divide";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator %=") {
    def._answer_location = ".tp_as_number->nb_inplace_remainder";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator <<=") {
    def._answer_location = "tp_as_number->nb_inplace_lshift";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator >>=") {
    def._answer_location = "tp_as_number->nb_inplace_rshift";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator &=") {
    def._answer_location = "tp_as_number->nb_inplace_and";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "operator ^=") {
    def._answer_location = "tp_as_number->nb_inplace_xor";
    def._wrapper_type = WT_inplace_binary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (method_name == "__ipow__") {
    def._answer_location = "tp_as_number->nb_inplace_power";
    def._wrapper_type = WT_inplace_ternary_operator;
    def._min_version = 0x02000000;
    return true;
  }

  if (obj->_protocol_types & Object::PT_sequence) {
    if (remap->_flags & FunctionRemap::F_getitem_int) {
      def._answer_location = "tp_as_sequence->sq_item";
      def._wrapper_type = WT_sequence_getitem;
      return true;
    }
    if (remap->_flags & FunctionRemap::F_setitem_int ||
        remap->_flags & FunctionRemap::F_delitem_int) {
      def._answer_location = "tp_as_sequence->sq_ass_item";
      def._wrapper_type = WT_sequence_setitem;
      return true;
    }
    if (remap->_flags & FunctionRemap::F_size) {
      def._answer_location = "tp_as_sequence->sq_length";
      def._wrapper_type = WT_sequence_size;
      return true;
    }
  }

  if (obj->_protocol_types & Object::PT_mapping) {
    if (remap->_flags & FunctionRemap::F_getitem) {
      def._answer_location = "tp_as_mapping->mp_subscript";
      def._wrapper_type = WT_one_param;
      return true;
    }
    if (remap->_flags & FunctionRemap::F_setitem ||
        remap->_flags & FunctionRemap::F_delitem) {
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

  if (method_name == "__getattribute__") {
    // Like __getattr__, but is called unconditionally, ie.
    // does not try PyObject_GenericGetAttr first.
    def._answer_location = "tp_getattro";
    def._wrapper_type = WT_one_param;
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

  if (method_name == "__delattr__") {
    // __delattr__ shares the slot with __setattr__, except
    // that it takes only one argument.
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

  if (remap->_type == FunctionRemap::T_typecast_method) {
    // A typecast operator.  Check for a supported low-level typecast type.
    if (TypeManager::is_bool(remap->_return_type->get_orig_type())) {
      // If it's a bool type, then we wrap it with the __nonzero__
      // slot method.
      def._answer_location = "tp_as_number->nb_nonzero";
      def._wrapper_type = WT_inquiry;
      return true;

    } else if (TypeManager::is_integer(remap->_return_type->get_orig_type())) {
      // An integer type.
      def._answer_location = "tp_as_number->nb_int";
      def._wrapper_type = WT_no_params;
      return true;

    } else if (TypeManager::is_float(remap->_return_type->get_orig_type())) {
      // A floating-point (or double) type.
      def._answer_location = "tp_as_number->nb_float";
      def._wrapper_type = WT_no_params;
      return true;
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
write_python_instance(ostream &out, int indent_level, const std::string &return_expr, bool owns_memory, const std::string &class_name, CPPType *ctype, bool is_const) {
  indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
  indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
  indent(out, indent_level) << "  return Py_None;\n";
  indent(out, indent_level) << "} else {\n";

  out << boolalpha;

  if (IsPandaTypedObject(ctype->as_struct_type())) {
    std::string typestr = "(" + return_expr + ")->as_typed_object()->get_type_index()";
    indent(out, indent_level) << "  return "
      << "DTool_CreatePyInstanceTyped((void *)" << return_expr << ", " << CLASS_PREFIX << make_safe_name(class_name) << ", " << owns_memory << ", " << is_const << ", " << typestr << ");\n";
  } else {
    //    indent(out, indent_level) << "if (" << return_expr << "!= NULL)\n";
    indent(out, indent_level) << "  return "
      << "DTool_CreatePyInstance((void *)" << return_expr << ", " << CLASS_PREFIX << make_safe_name(class_name) << ", " << owns_memory << ", " << is_const << ");\n";
  }
  indent(out, indent_level) << "}\n";
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
          _external_imports.insert(object->_itype._cpptype);
        }
      }
    }
  }

  out_code << "//********************************************************************\n";
  out_code << "//*** prototypes for .. External Objects\n";
  out_code << "//********************************************************************\n";

  for (std::set<CPPType *>::iterator ii = _external_imports.begin(); ii != _external_imports.end(); ii++) {
    CPPType *type = (*ii);
    string class_name = type->get_local_name(&parser);
    string safe_name = make_safe_name(class_name);

    out_code << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << safe_name << ";\n";
    out_code << "IMPORT_THIS void Dtool_PyModuleClassInit_" << safe_name << "(PyObject *module);\n";

    int has_coerce = has_coerce_constructor(type->as_struct_type());
    if (has_coerce > 0) {
      out_code << "IMPORT_THIS bool Dtool_Coerce_" << safe_name << "(PyObject *args, " << class_name << " const *&coerced);\n";
      if (has_coerce > 1) {
        out_code << "IMPORT_THIS bool Dtool_Coerce_" << safe_name << "(PyObject *args, " << class_name << " *&coerced);\n";
      }
    }
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
  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi).second;
    if (!func->_itype.is_global() && is_function_legal(func)) {
      write_function_for_top(out, NULL, func);
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
  Function::Remaps::const_iterator ri;

  //std::string cClassName = obj->_itype.get_scoped_name();
  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName = obj->_itype.get_true_name();

  out << "//********************************************************************\n";
  out << "//*** Functions for .. " << cClassName << "\n" ;
  out << "//********************************************************************\n";

  for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) {
    Function *func = (*fi);
    if (func) {
      write_function_for_top(out, obj, func);
    }
  }

  Properties::const_iterator pit;
  for (pit = obj->_properties.begin(); pit != obj->_properties.end(); ++pit) {
    Property *property = (*pit);
    const InterrogateElement &ielem = property->_ielement;
    string expected_params;

    if (property->_getter != NULL) {
      std::string fname = "PyObject *Dtool_" + ClassName + "_" + ielem.get_name() + "_Getter(PyObject *self, void *)";
      write_function_for_name(out, obj, property->_getter->_remaps,
                              fname, expected_params, true,
                              AT_no_args, RF_pyobject | RF_err_null);
    }

    if (property->_setter != NULL) {
      std::string fname = "int Dtool_" + ClassName + "_" + ielem.get_name() + "_Setter(PyObject *self, PyObject *arg, void *)";
      write_function_for_name(out, obj, property->_setter->_remaps,
                              fname, expected_params, true,
                              AT_single_arg, RF_int);
    }
  }

  if (obj->_constructors.size() == 0) {
    out << "int Dtool_Init_" + ClassName + "(PyObject *, PyObject *, PyObject *) {\n"
        << "  PyErr_SetString(PyExc_TypeError, \"cannot init constant class (" << cClassName << ")\");\n"
        << "  return -1;\n"
        << "}\n\n";

  } else {
    for (fi = obj->_constructors.begin(); fi != obj->_constructors.end(); ++fi) {
      Function *func = (*fi);
      std::string fname = "int Dtool_Init_" + ClassName + "(PyObject *self, PyObject *args, PyObject *kwds)";

      string expected_params;
      write_function_for_name(out, obj, func->_remaps, fname, expected_params, true, AT_keyword_args, RF_int);
    }
  }

  CPPType *cpptype = TypeManager::resolve_type(obj->_itype._cpptype);

  // If we have "coercion constructors", write a single wrapper to consolidate those.
  int has_coerce = has_coerce_constructor(cpptype->as_struct_type());
  if (has_coerce > 0) {
    write_coerce_constructor(out, obj, true);
    if (has_coerce > 1) {
      write_coerce_constructor(out, obj, false);
    }
  }

  MakeSeqs::iterator msi;
  for (msi = obj->_make_seqs.begin(); msi != obj->_make_seqs.end(); ++msi) {
    write_make_seq(out, obj, ClassName, *msi);
  }

  std::map<string, CastDetails> details;
  std::map<string, CastDetails>::iterator di;
  builder.get_type(TypeManager::unwrap(cpptype), false);
  get_valid_child_classes(details, cpptype->as_struct_type());
  for (di = details.begin(); di != details.end(); di++) {
    //InterrogateType ptype =idb->get_type(di->first);
    if (di->second._is_legal_py_class && !isExportThisRun(di->second._structType)) {
      _external_imports.insert(di->second._structType);
    }
    //out << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" << make_safe_name(di->second._to_class_name) << ";\n";
  }

  { // the Cast Converter

    out << "inline void *Dtool_UpcastInterface_" << ClassName << "(PyObject *self, Dtool_PyTypedObject *requested_type) {\n";
    out << "  Dtool_PyTypedObject *SelfType = ((Dtool_PyInstDef *)self)->_My_Type;\n";
    out << "  if (SelfType != &Dtool_" << ClassName << ") {\n";
    out << "    printf(\"" << ClassName << " ** Bad Source Type-- Requesting Conversion from %s to %s\\n\", Py_TYPE(self)->tp_name, requested_type->_PyType.tp_name); fflush(NULL);\n";;
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

  out << "EXPORT_THIS void Dtool_PyModuleClassInit_" << class_name << "(PyObject *module);\n";

  int has_coerce = has_coerce_constructor(obj->_itype._cpptype->as_struct_type());
  if (has_coerce > 0) {
    out << "EXPORT_THIS bool Dtool_Coerce_" << class_name << "(PyObject *args, " << c_class_name << " const *&coerced);\n";
    if (has_coerce > 1) {
      out << "EXPORT_THIS bool Dtool_Coerce_" << class_name << "(PyObject *args, " << c_class_name << " *&coerced);\n";
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
  string class_name = make_safe_name(obj->_itype.get_scoped_name());
  out << "  // Module init upcall for " << obj->_itype.get_scoped_name() << "\n";

  if (!obj->_itype.is_typedef()) {
    out << "  // " << *(obj->_itype._cpptype) << "\n";
    out << "  Dtool_PyModuleClassInit_" << class_name << "(module);\n";

  } else {
    // Unwrap typedefs.
    TypeIndex wrapped = obj->_itype._wrapped_type;
    while (interrogate_type_is_typedef(wrapped)) {
      wrapped = interrogate_type_wrapped_type(wrapped);
    }

    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
    const InterrogateType &wrapped_itype = idb->get_type(wrapped);

    class_name = make_safe_name(wrapped_itype.get_scoped_name());

    out << "  // typedef " << wrapped_itype.get_scoped_name()
        << " " << *(obj->_itype._cpptype) << "\n";

    if (!isExportThisRun(wrapped_itype._cpptype)) {
      _external_imports.insert(wrapped_itype._cpptype);
    }
  }

  std::string export_class_name = classNameFromCppName(obj->_itype.get_name(), false);
  std::string export_class_name2 = classNameFromCppName(obj->_itype.get_name(), true);

  out << "  Py_INCREF(&Dtool_" << class_name << ".As_PyTypeObject());\n";
  out << "  PyModule_AddObject(module, \"" << export_class_name << "\", (PyObject *)&Dtool_" << class_name << ".As_PyTypeObject());\n";
  if (export_class_name != export_class_name2) {
    out << "  PyModule_AddObject(module, \"" << export_class_name2 << "\", (PyObject *)&Dtool_" << class_name << ".As_PyTypeObject());\n";
  }
}

/////////////////////////////////////////////////////////////////////////////
// Function : write_module_support
/////////////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_module_support(ostream &out, ostream *out_h, InterrogateModuleDef *def) {
  out << "//********************************************************************\n";
  out << "//*** Module Object Linker ..\n";
  out << "//********************************************************************\n";

  out << "static void BuildInstants(PyObject *module) {\n";
  out << "  (void) module; // Unused\n";

  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (object->_itype.is_enum() && !object->_itype.is_nested()) {
      int enum_count = object->_itype.number_of_enum_values();
      if (enum_count > 0) {
          out << "//********************************************************************\n";
          out << "//*** Module Enums  .." << object->_itype.get_scoped_name() << "\n";
          out << "//********************************************************************\n";
      }
      for (int xx = 0; xx < enum_count; xx++) {
        string name1 = classNameFromCppName(object->_itype.get_enum_value_name(xx), false);
        string name2 = classNameFromCppName(object->_itype.get_enum_value_name(xx), true);
        string enum_value = "::" + object->_itype.get_enum_value_name(xx);
        out << "  PyModule_AddIntConstant(module, \"" << name1 << "\", " << enum_value << ");\n";
        if (name1 != name2) {
          // Also write the mangled name, for historical purposes.
          out << "  PyModule_AddIntConstant(module, \"" << name2 << "\", " << enum_value << ");\n";
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
      out << "  PyModule_AddIntConstant(module, \"" << name1 << "\", " << value << ");\n";
      if (name1 != name2) {
        // Also write the mangled name, for historical purposes.
        out << "  PyModule_AddIntConstant(module, \"" << name2 << "\", " << value << ");\n";
      }
    } else {
      string value = iman.get_definition();
      out << "  PyModule_AddStringConstant(module, \"" << name1 << "\", \"" << value << "\");\n";
      if (name1 != name2) {
        out << "  PyModule_AddStringConstant(module, \"" << name2 << "\", \"" << value << "\");\n";
      }
    }
  }

  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (!object->_itype.get_outer_class()) {
      if (object->_itype.is_class() ||
          object->_itype.is_struct() ||
          object->_itype.is_typedef()) {
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
  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi).second;
    if (!func->_itype.is_global() && is_function_legal(func)) {
      string name1 = methodNameFromCppName(func, "", false);
      string name2 = methodNameFromCppName(func, "", true);

      string flags;
      switch (func->_args_type) {
      case AT_keyword_args:
        flags = "METH_VARARGS | METH_KEYWORDS";
        break;

      case AT_varargs:
        flags = "METH_VARARGS";
        break;

      case AT_single_arg:
        flags = "METH_O";
        break;

      default:
        flags = "METH_NOARGS";
        break;
      }

      // Note: we shouldn't add METH_STATIC here, since both METH_STATIC
      // and METH_CLASS are illegal for module-level functions.

      out << "  { \"" << name1 << "\", (PyCFunction) &"
          << func->_name << ", " << flags << ", (const char *)" << func->_name << "_comment},\n";
      if (name1 != name2) {
        out << "  { \"" << name2 << "\", (PyCFunction) &"
            << func->_name << ", " << flags << ", (const char *)" << func->_name << "_comment},\n";
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
      if (_objects.count(nested_index) == 0) {
        // Illegal type.
        continue;
      }

      Object *nested_obj = _objects[nested_index];
      assert(nested_obj != (Object *)NULL);

      if (nested_obj->_itype.is_class() || nested_obj->_itype.is_struct()) {
        write_module_class(out, nested_obj);
      }
    }
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName =  obj->_itype.get_true_name();
  std::string export_class_name = classNameFromCppName(obj->_itype.get_name(), false);

  Functions::iterator fi;
  out << "//********************************************************************\n";
  out << "//*** Py Init Code For .. " << ClassName << " | " << export_class_name << "\n" ;
  out << "//********************************************************************\n";
  out << "PyMethodDef Dtool_Methods_" << ClassName << "[] = {\n";

  std::map<string, SlottedFunctionDef> slotted_functions;
  // function Table
  bool got_copy = false;
  bool got_deepcopy = false;

  for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) {
    Function *func = (*fi);
    if (func->_name == "__copy__") {
      got_copy = true;
    } else if (func->_name == "__deepcopy__") {
      got_deepcopy = true;
    }

    string name1 = methodNameFromCppName(func, export_class_name, false);
    string name2 = methodNameFromCppName(func, export_class_name, true);

    string flags;
    switch (func->_args_type) {
    case AT_keyword_args:
      flags = "METH_VARARGS | METH_KEYWORDS";
      break;

    case AT_varargs:
      flags = "METH_VARARGS";
      break;

    case AT_single_arg:
      flags = "METH_O";
      break;

    default:
      flags = "METH_NOARGS";
      break;
    }

    if (!func->_has_this) {
      flags += " | METH_STATIC";
    }

    bool has_nonslotted = false;

    Function::Remaps::const_iterator ri;
    for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
      FunctionRemap *remap = (*ri);
      if (!is_remap_legal(remap)) {
        continue;
      }

      SlottedFunctionDef slotted_def;

      if (get_slotted_function_def(obj, func, remap, slotted_def)) {
        const string &key = slotted_def._answer_location;
        if (slotted_functions.count(key)) {
          slotted_functions[key]._remaps.insert(remap);
        } else {
          slotted_functions[key] = slotted_def;
          slotted_functions[key]._remaps.insert(remap);
        }
      } else {
        has_nonslotted = true;
      }
    }

    if (has_nonslotted) {
      // This function has non-slotted remaps, so write it out into the function table.

      out << "  { \"" << name1 << "\", (PyCFunction) &"
          << func->_name << ", " << flags << ", (char *) " << func->_name << "_comment},\n";
      if (name1 != name2) {
        out << "  { \"" << name2 << "\", (PyCFunction) &"
            << func->_name << ", " << flags << ", (char *) " << func->_name << "_comment},\n";
      }
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

  out << "  { NULL, NULL, 0, NULL }\n"
      << "};\n\n";

  int num_derivations = obj->_itype.number_of_derivations();
  int di;
  for (di = 0; di < num_derivations; di++) {
    TypeIndex d_type_Index = obj->_itype.get_derivation(di);
    if (!interrogate_type_is_unpublished(d_type_Index)) {
      const InterrogateType &d_itype = idb->get_type(d_type_Index);
      if (is_cpp_type_legal(d_itype._cpptype)) {
        if (!isExportThisRun(d_itype._cpptype)) {
          _external_imports.insert(d_itype._cpptype);

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
    Function *getitem_func;

    std::map<string, SlottedFunctionDef>::iterator rfi; //          slotted_functions;
    for (rfi = slotted_functions.begin(); rfi != slotted_functions.end(); rfi++) {
      const SlottedFunctionDef &def = rfi->second;
      Function *func = def._func;

      switch (rfi->second._wrapper_type) {
      case WT_no_params:
      case WT_iter_next: // TODO: fix iter_next to return NULL instead of None
        // PyObject *func(PyObject *self)
        {
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self) {\n";
          out << "  " << cClassName  << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return NULL;\n";
          out << "  }\n\n";

          string expected_params;
          write_function_forset(out, def._remaps, expected_params, 2, true, true,
                                AT_no_args, RF_pyobject | RF_err_null, false, "index");

          out << "  if (!PyErr_Occurred()) {\n";
          out << "    PyErr_SetString(PyExc_TypeError,\n";
          out << "      \"Arguments must match:\\n\"\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return NULL;\n";
          out << "}\n\n";
        }
        break;

      case WT_one_param:
      case WT_binary_operator:
      case WT_inplace_binary_operator:
        // PyObject *func(PyObject *self, PyObject *one)
        {
          int return_flags = RF_err_null;
          if (rfi->second._wrapper_type == WT_inplace_binary_operator) {
            return_flags |= RF_self;
          } else {
            return_flags |= RF_pyobject;
          }
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *arg) {\n";
          out << "  " << cClassName << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          if (rfi->second._wrapper_type != WT_one_param) {
            // WT_binary_operator means we must return NotImplemented, instead
            // of raising an exception, if the this pointer doesn't
            // match.  This is for things like __sub__, which Python
            // likes to call on the wrong-type objects.
            out << "    Py_INCREF(Py_NotImplemented);\n";
            out << "    return Py_NotImplemented;\n";
          } else {
            out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
            out << "    return NULL;\n";
          }
          out << "  }\n";

          string expected_params;
          write_function_forset(out, def._remaps, expected_params, 2, true, true,
                                AT_single_arg, return_flags, false);

          if (rfi->second._wrapper_type != WT_one_param) {
            out << "    Py_INCREF(Py_NotImplemented);\n";
            out << "    return Py_NotImplemented;\n";
          } else {
            out << "  if (!PyErr_Occurred()) {\n";
            out << "    PyErr_SetString(PyExc_TypeError,\n";
            out << "      \"Arguments must match:\\n\"\n";
            output_quoted(out, 6, expected_params);
            out << ");\n";
            out << "  }\n";
            out << "  return NULL;\n";
          }
          out << "}\n\n";
        }
        break;

      case WT_setattr:
        // int func(PyObject *self, PyObject *one, PyObject *two = NULL)
        {
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *arg, PyObject *arg2) {\n";
          out << "  " << cClassName  << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          set<FunctionRemap*> setattr_remaps;
          set<FunctionRemap*> delattr_remaps;

          // This function handles both delattr and setattr.  Fish out
          // the remaps for both types.
          set<FunctionRemap*>::const_iterator ri;
          for (ri = def._remaps.begin(); ri != def._remaps.end(); ++ri) {
            FunctionRemap *remap = (*ri);

            if (remap->_cppfunc->get_simple_name() == "__delattr__" && remap->_parameters.size() == 2) {
              delattr_remaps.insert(remap);

            } else if (remap->_cppfunc->get_simple_name() == "__setattr__" && remap->_parameters.size() == 3) {
              setattr_remaps.insert(remap);
            }
          }

          out << "  // Determine whether to call __setattr__ or __delattr__.\n";
          out << "  if (arg2 != (PyObject *)NULL) { // __setattr__\n";

          if (!setattr_remaps.empty()) {
            out << "    PyObject *args = PyTuple_Pack(2, arg, arg2);\n";
            string expected_params;
            write_function_forset(out, setattr_remaps, expected_params, 4,
                                  true, true, AT_varargs, RF_int | RF_decref_args, true);

            out << "    if (!PyErr_Occurred()) {\n";
            out << "      PyErr_SetString(PyExc_TypeError,\n";
            out << "        \"Arguments must match:\\n\"\n";
            output_quoted(out, 8, expected_params);
            out << ");\n";
            out << "    }\n";
            out << "    Py_DECREF(args);\n";
          } else {
            out << "    PyErr_Format(PyExc_TypeError,\n";
            out << "      \"can't set attributes of built-in/extension type '%s'\",\n";
            out << "      Py_TYPE(self)->tp_name);\n";
          }
          out << "    return -1;\n\n";

          out << "  } else { // __delattr__\n";

          if (!delattr_remaps.empty()) {
            string expected_params;
            write_function_forset(out, delattr_remaps, expected_params, 4,
                                  true, true, AT_single_arg, RF_int, true);

            out << "    if (!PyErr_Occurred()) {\n";
            out << "      PyErr_SetString(PyExc_TypeError,\n";
            out << "        \"Arguments must match:\\n\"\n";
            output_quoted(out, 8, expected_params);
            out << ");\n";
            out << "    }\n";
          } else {
            out << "    PyErr_Format(PyExc_TypeError,\n";
            out << "      \"can't delete attributes of built-in/extension type '%s'\",\n";
            out << "      Py_TYPE(self)->tp_name);\n";
          }
          out << "    return -1;\n";
          out << "  }\n";

          out << "}\n\n";
        }
        break;

      case WT_getattr:
        // PyObject *func(PyObject *self, PyObject *one)
        // Specifically to implement __getattr__.
        // First calls PyObject_GenericGetAttr(), and only calls the wrapper if it returns NULL.
        // If one wants to override this completely, one should define __getattribute__ instead.
        {
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *arg) {\n";
          out << "  PyObject *res = PyObject_GenericGetAttr(self, arg);\n";
          out << "  if (res != NULL) {\n";
          out << "    return res;\n";
          out << "  }\n";
          out << "  if (!PyErr_ExceptionMatches(PyExc_AttributeError)) {\n";
          out << "    return NULL;\n";
          out << "  }\n";
          out << "  PyErr_Clear();\n\n";

          out << "  " << cClassName  << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return NULL;\n";
          out << "  }\n\n";

          string expected_params;
          write_function_forset(out, def._remaps, expected_params, 2,
                                true, true, AT_single_arg,
                                RF_pyobject | RF_err_null, true);

          //out << "  PyErr_Clear();\n";
          out << "  return NULL;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_getitem:
        // PyObject *func(PyObject *self, Py_ssize_t index)
        {
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, Py_ssize_t index) {\n";
          out << "  " << cClassName  << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return NULL;\n";
          out << "  }\n\n";

          // This is a getitem or setitem of a sequence type.  This means we
          // *need* to raise IndexError if we're out of bounds.  We have to
          // assume the bounds are 0 .. this->size() (this is the same
          // assumption that Python makes).
          out << "  if (index < 0 || index >= (Py_ssize_t) local_this->size()) {\n";
          out << "    PyErr_SetString(PyExc_IndexError, \"" << ClassName << " index out of range\");\n";
          out << "    return NULL;\n";
          out << "  }\n";

          string expected_params;
          write_function_forset(out, def._remaps, expected_params, 2, true, true,
                                AT_no_args, RF_pyobject | RF_err_null, false, "index");

          out << "  if (!PyErr_Occurred()) {\n";
          out << "    PyErr_SetString(PyExc_TypeError,\n";
          out << "      \"Arguments must match:\\n\"\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return NULL;\n";
          out << "}\n\n";

          getitem_func = func;
        }
        break;

      case WT_sequence_setitem:
        // int_t func(PyObject *self, Py_ssize_t index, PyObject *value)
        {
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, Py_ssize_t index, PyObject *arg) {\n";
          out << "  " << cClassName  << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          out << "  if (index < 0 || index >= (Py_ssize_t) local_this->size()) {\n";
          out << "    PyErr_SetString(PyExc_IndexError, \"" << ClassName << " index out of range\");\n";
          out << "    return -1;\n";
          out << "  }\n";

          set<FunctionRemap*> setitem_remaps;
          set<FunctionRemap*> delitem_remaps;

          // This function handles both delitem and setitem.  Fish out
          // the remaps for either one.
          set<FunctionRemap*>::const_iterator ri;
          for (ri = def._remaps.begin(); ri != def._remaps.end(); ++ri) {
            FunctionRemap *remap = (*ri);

            if (remap->_flags & FunctionRemap::F_setitem_int) {
              setitem_remaps.insert(remap);

            } else if (remap->_flags & FunctionRemap::F_delitem_int) {
              delitem_remaps.insert(remap);
            }
          }

          string expected_params;
          out << "  if (arg != (PyObject *)NULL) { // __setitem__\n";
          write_function_forset(out, setitem_remaps, expected_params, 4,
                                true, true, AT_single_arg, RF_int, false, "index");
          out << "  } else { // __delitem__\n";
          write_function_forset(out, delitem_remaps, expected_params, 4,
                                true, true, AT_single_arg, RF_int, false, "index");
          out << "  }\n\n";

          out << "  if (!PyErr_Occurred()) {\n";
          out << "    PyErr_SetString(PyExc_TypeError,\n";
          out << "      \"Arguments must match:\\n\"\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return -1;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_size:
        // Py_ssize_t func(PyObject *self)
        {
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static Py_ssize_t " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self) {\n";
          out << "  " << cClassName  << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          // This is a cheap cheat around all of the overhead of calling the wrapper function.
          out << "  return (Py_ssize_t) local_this->" << func->_ifunc.get_name() << "();\n";
          out << "}\n\n";
        }
        break;

      case WT_mapping_setitem:
        // int func(PyObject *self, PyObject *one, PyObject *two)
        {
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *arg, PyObject *arg2) {\n";
          out << "  " << cClassName  << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          set<FunctionRemap*> setitem_remaps;
          set<FunctionRemap*> delitem_remaps;

          // This function handles both delitem and setitem.  Fish out
          // the remaps for either one.
          set<FunctionRemap*>::const_iterator ri;
          for (ri = def._remaps.begin(); ri != def._remaps.end(); ++ri) {
            FunctionRemap *remap = (*ri);

            if (remap->_flags & FunctionRemap::F_setitem_int) {
              setitem_remaps.insert(remap);

            } else if (remap->_flags & FunctionRemap::F_delitem_int) {
              delitem_remaps.insert(remap);
            }
          }

          string expected_params;
          out << "  if (arg2 != (PyObject *)NULL) { // __setitem__\n";
          out << "    PyObject *args = PyTuple_Pack(2, arg, arg2);\n";
          write_function_forset(out, setitem_remaps, expected_params, 4,
                                true, true, AT_varargs, RF_int | RF_decref_args, false);
          out << "    Py_DECREF(args);\n";
          out << "  } else { // __delitem__\n";
          write_function_forset(out, delitem_remaps, expected_params, 4,
                                true, true, AT_single_arg, RF_int, false);
          out << "  }\n\n";

          out << "  if (!PyErr_Occurred()) {\n";
          out << "    PyErr_SetString(PyExc_TypeError,\n";
          out << "      \"Arguments must match:\\n\"\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return -1;\n";
          out << "}\n\n";
        }
        break;

      case WT_inquiry:
        // int func(PyObject *self)
        {
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static int " << func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self) {\n";
          out << "  " << cClassName  << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          FunctionRemap *remap = *def._remaps.begin();
          vector_string params;
          out << "  return (int) " << remap->call_function(out, 4, false, "local_this", params) << ";\n";
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
          set<FunctionRemap*>::const_iterator ri;
          for (ri = def._remaps.begin(); ri != def._remaps.end(); ++ri) {
            FunctionRemap *remap = (*ri);
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
          set<FunctionRemap*>::const_iterator ri;
          for (ri = def._remaps.begin(); ri != def._remaps.end(); ++ri) {
            FunctionRemap *remap = (*ri);
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

      case WT_ternary_operator:
      case WT_inplace_ternary_operator:
        // PyObject *func(PyObject *self, PyObject *one, PyObject *two)
        {
          int return_flags = RF_err_null;
          if (rfi->second._wrapper_type == WT_inplace_ternary_operator) {
            return_flags |= RF_self;
          } else {
            return_flags |= RF_pyobject;
          }
          out << "//////////////////\n";
          out << "//  A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "//     " << ClassName << " ..." << rfi->second._answer_location << " = " << methodNameFromCppName(func, export_class_name, false) << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" <<  func->_name << methodNameFromCppName(func, export_class_name, false) << "(PyObject *self, PyObject *arg, PyObject *arg2) {\n";
          out << "  " << cClassName << " *local_this = NULL;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == NULL) {\n";
          // WT_ternary_operator means we must return NotImplemented, instead
          // of raising an exception, if the this pointer doesn't
          // match.  This is for things like __pow__, which Python
          // likes to call on the wrong-type objects.
          out << "    Py_INCREF(Py_NotImplemented);\n";
          out << "    return Py_NotImplemented;\n";
          out << "  }\n";

          set<FunctionRemap*> one_param_remaps;
          set<FunctionRemap*> two_param_remaps;

          set<FunctionRemap*>::const_iterator ri;
          for (ri = def._remaps.begin(); ri != def._remaps.end(); ++ri) {
            FunctionRemap *remap = (*ri);

            if (remap->_parameters.size() == 2) {
              one_param_remaps.insert(remap);

            } else if (remap->_parameters.size() == 3) {
              two_param_remaps.insert(remap);
            }
          }

          string expected_params;

          out << "  if (arg2 != (PyObject *)NULL) {\n";
          out << "    PyObject *args = PyTuple_Pack(2, arg, arg2);\n";
          write_function_forset(out, two_param_remaps, expected_params, 4,
                                true, true, AT_varargs, RF_pyobject | RF_err_null | RF_decref_args, true);
          out << "    Py_DECREF(args);\n";
          out << "  } else {\n";
          write_function_forset(out, one_param_remaps, expected_params, 4,
                                true, true, AT_single_arg, RF_pyobject | RF_err_null, true);
          out << "  }\n\n";

          out << "  if (!PyErr_Occurred()) {\n";
          out << "    PyErr_SetString(PyExc_TypeError,\n";
          out << "      \"Arguments must match:\\n\"\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return NULL;\n";
          out << "}\n\n";
        }
        break;

      case WT_none:
        // Nothing special about the wrapper function: just write it normally.
        string fname = "static PyObject *" + func->_name + "(PyObject *self, PyObject *args, PyObject *kwds)\n";
        string expected_params;
        write_function_for_name(out, obj, func->_remaps, fname, expected_params, true, AT_keyword_args, RF_pyobject | RF_err_null);
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
    out << "//////////////////\n";
    out << "//  A rich comparison function\n";
    out << "//     " << ClassName << "\n";
    out << "//////////////////\n";
    out << "static PyObject *Dtool_RichCompare_" << ClassName << "(PyObject *self, PyObject *arg, int op) {\n";
    out << "  " << cClassName  << " *local_this = NULL;\n";
    out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
    out << "  if (local_this == NULL) {\n";
    out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
    out << "    return NULL;\n";
    out << "  }\n\n";

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
        if (is_remap_legal(remap) && remap->_has_this && (remap->_args_type == AT_single_arg)) {
          remaps.insert(remap);
        }
      }
      const string &fname = func->_ifunc.get_name();
      if (fname == "operator <") {
        out << "  case Py_LT: {\n";
      } else if (fname == "operator <=") {
        out << "  case Py_LE: {\n";
      } else if (fname == "operator ==") {
        out << "  case Py_EQ: {\n";
      } else if (fname == "operator !=") {
        out << "  case Py_NE: {\n";
      } else if (fname == "operator >") {
        out << "  case Py_GT: {\n";
      } else if (fname == "operator >=") {
        out << "  case Py_GE: {\n";
      } else if (fname == "compare_to") {
        compare_to_func = func;
        continue;
      } else {
        continue;
      }

      string expected_params;
      write_function_forset(out, remaps, expected_params, 4, true, false,
                            AT_single_arg, RF_pyobject | RF_err_null, false);

      out << "    if (PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_TypeError)) {\n";
      out << "      PyErr_Clear();\n";
      out << "    }\n";
      out << "    break;\n";
      out << "  }\n";
      has_local_richcompare = true;
    }

    out << "  }\n\n";

    out << "  if (PyErr_Occurred()) {\n";
    out << "    return (PyObject *)NULL;\n";
    out << "  }\n\n";

    if (compare_to_func != NULL) {
      out << "#if PY_MAJOR_VERSION >= 3\n";
      out << "  // All is not lost; we still have the compare_to function to fall back onto.\n";
      out << "  PyObject *result = " << compare_to_func->_name << "(self, arg);\n";
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

  int num_getset = 0;

  if (obj->_properties.size() > 0) {
    // Write out the array of properties, telling Python which getter and setter
    // to call when they are assigned or queried in Python code.
    out << "PyGetSetDef Dtool_Properties_" << ClassName << "[] = {\n";

    Properties::const_iterator pit;
    for (pit = obj->_properties.begin(); pit != obj->_properties.end(); ++pit) {
      Property *property = (*pit);
      const InterrogateElement &ielem = property->_ielement;
      if (property->_getter == NULL || !is_function_legal(property->_getter)) {
        continue;
      }

      ++num_getset;

      string name1 = methodNameFromCppName(ielem.get_name(), "", false);
      string name2 = methodNameFromCppName(ielem.get_name(), "", true);

      string getter = "&Dtool_" + ClassName + "_" + ielem.get_name() + "_Getter";
      string setter = "NULL";
      if (property->_setter != NULL && is_function_legal(property->_setter)) {
        setter = "&Dtool_" + ClassName + "_" + ielem.get_name() + "_Setter";
      }

      out << "  {(char *)\"" << name1 << "\", " << getter << ", " << setter;

      if (ielem.has_comment()) {
        out << ", (char *)\n";
        output_quoted(out, 4, ielem.get_comment());
        out << ",\n    ";
      } else {
        out << ", NULL, ";
      }

      // Extra void* argument; we don't make use of it.
      out << "NULL},\n";

      if (name1 != name2 && name1 != "__dict__") {
        // Add alternative spelling.
        out << "  {(char *)\"" << name2 << "\", " << getter << ", " << setter
            << ", (char *)\n"
            << "    \"Alias of " << name1 << ", for consistency with old naming conventions.\",\n"
            << "    NULL},\n";
      }
    }

    out << "  {NULL},\n";
    out << "};\n\n";
  }

  out << "void Dtool_PyModuleClassInit_" << ClassName << "(PyObject *module) {\n";
  out << "  (void) module; // Unused\n";
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
    out << "#if PY_VERSION_HEX < 0x03000000\n";
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_flags |= Py_TPFLAGS_HAVE_ITER;\n";
    out << "#endif";
  }
  if (has_local_getbuffer) {
    out << "#if PY_VERSION_HEX >= 0x02060000 && PY_VERSION_HEX < 0x03000000\n";
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_flags |= Py_TPFLAGS_HAVE_NEWBUFFER;\n";
    out << "#endif";
  }

  // Add bases.
  if (bases.size() > 0) {
    out << "    // Dependent objects\n";
    string baseargs;
    for (vector<string>::iterator bi = bases.begin(); bi != bases.end(); ++bi) {
      baseargs += ", &Dtool_" + *bi + ".As_PyTypeObject()";
      out << "    Dtool_PyModuleClassInit_" << make_safe_name(*bi) << "(NULL);\n";
    }

    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_bases = PyTuple_Pack(" << bases.size() << baseargs << ");\n";
  }

  // get dictionary
  out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_dict = PyDict_New();\n";
  out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"DtoolClassDict\", Dtool_" << ClassName << ".As_PyTypeObject().tp_dict);\n";

  // Now assign the slotted function definitions.
  map<string, SlottedFunctionDef>::const_iterator rfi;
  int prev_min_version = 0;

  for (rfi = slotted_functions.begin(); rfi != slotted_functions.end(); rfi++) {
    const SlottedFunctionDef &def = rfi->second;
    Function *func = def._func;

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

  if (num_getset > 0) {
    // GetSet descriptor slots.
    out << "    Dtool_" << ClassName << ".As_PyTypeObject().tp_getset = Dtool_Properties_" << ClassName << ";\n";
  }

  int num_nested = obj->_itype.number_of_nested_types();
  for (int ni = 0; ni < num_nested; ni++) {
    TypeIndex nested_index = obj->_itype.get_nested_type(ni);
    if (_objects.count(nested_index) == 0) {
      // Illegal type.
      continue;
    }

    Object *nested_obj = _objects[nested_index];
    assert(nested_obj != (Object *)NULL);

    if (nested_obj->_itype.is_class() || nested_obj->_itype.is_struct()) {
      std::string ClassName1 = make_safe_name(nested_obj->_itype.get_scoped_name());
      std::string ClassName2 = make_safe_name(nested_obj->_itype.get_name());
      out << "    // Nested Object   " << ClassName1 << ";\n";
      out << "    Dtool_PyModuleClassInit_" << ClassName1 << "(NULL);\n";
      string name1 = classNameFromCppName(ClassName2, false);
      string name2 = classNameFromCppName(ClassName2, true);
      out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name1 << "\", (PyObject *)&Dtool_" << ClassName1 << ".As_PyTypeObject());\n";
      if (name1 != name2) {
        out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name2 << "\", (PyObject *)&Dtool_" << ClassName1 << ".As_PyTypeObject());\n";
      }

    } else if (nested_obj->_itype.is_typedef()) {
      // Unwrap typedefs.
      TypeIndex wrapped = nested_obj->_itype._wrapped_type;
      while (interrogate_type_is_typedef(wrapped)) {
        wrapped = interrogate_type_wrapped_type(wrapped);
      }

      // Er, we can only export typedefs to structs.
      if (!interrogate_type_is_struct(wrapped)) {
        continue;
      }

      string ClassName1 = make_safe_name(interrogate_type_scoped_name(wrapped));
      string ClassName2 = make_safe_name(interrogate_type_name(wrapped));

      string name1 = classNameFromCppName(ClassName2, false);
      out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name1 << "\", (PyObject *)&Dtool_" << ClassName1 << ".As_PyTypeObject());\n";
      // No need to support mangled names for nested typedefs; we only added support recently.

    } else if (nested_obj->_itype.is_enum()) {
      out << "    // Enum  " << nested_obj->_itype.get_scoped_name() << ";\n";
      CPPEnumType *enum_type = nested_obj->_itype._cpptype->as_enum_type();
      CPPEnumType::Elements::const_iterator ei;
      for (ei = enum_type->_elements.begin(); ei != enum_type->_elements.end(); ++ei) {
        string name1 = classNameFromCppName((*ei)->get_simple_name(), false);
        string name2;
        if (nested_obj->_itype.has_true_name()) {
          name2 = classNameFromCppName((*ei)->get_simple_name(), true);
        } else {
          // Don't generate the alternative syntax for anonymous enums, since we added support
          // for those after we started deprecating the alternative syntax.
          name2 = name1;
        }
        string enum_value = obj->_itype.get_scoped_name() + "::" + (*ei)->get_simple_name();
        out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name1 << "\", PyLongOrInt_FromLong(" << enum_value << "));\n";
        if (name1 != name2) {
          out << "    PyDict_SetItemString(Dtool_" << ClassName << ".As_PyTypeObject().tp_dict, \"" << name2 << "\", PyLongOrInt_FromLong(" << enum_value << "));\n";
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

  // Also write out the explicit alternate names.
  //int num_alt_names = obj->_itype.get_num_alt_names();
  //for (int i = 0; i < num_alt_names; ++i) {
  //  string alt_name = make_safe_name(obj->_itype.get_alt_name(i));
  //  if (export_class_name != alt_name) {
  //    out << "    PyModule_AddObject(module, \"" << alt_name << "\", (PyObject *)&Dtool_" << ClassName << ".As_PyTypeObject());\n";
  //  }
  //}

  //out << "  }\n";
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
write_function_for_top(ostream &out, InterfaceMaker::Object *obj, InterfaceMaker::Function *func) {

  // First check if this function has non-slotted and legal remaps,
  // ie. if we should even write it.
  bool has_remaps = false;

  Function::Remaps::const_iterator ri;
  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    if (!is_remap_legal(remap)) {
      continue;
    }

    SlottedFunctionDef slotted_def;
    if (!get_slotted_function_def(obj, func, remap, slotted_def)) {
      has_remaps = true;
    }
  }

  if (!has_remaps) {
    // Nope.
    return;
  }

  std::string fname;

  if (func->_ifunc.is_unary_op()) {
    assert(func->_args_type == AT_no_args);
  }

  fname = "static PyObject *" + func->_name + "(PyObject *";

  // This will be NULL for static funcs, so prevent code from using it.
  if (func->_has_this) {
    fname += "self";
  }

  switch (func->_args_type) {
  case AT_keyword_args:
    fname += ", PyObject *args, PyObject *kwds";
    break;

  case AT_varargs:
    fname += ", PyObject *args";
    break;

  case AT_single_arg:
    fname += ", PyObject *arg";
    break;

  default:
    break;
  }
  fname += ")";

  string expected_params;
  write_function_for_name(out, obj, func->_remaps, fname, expected_params, true, func->_args_type, RF_pyobject | RF_err_null);

  // Now synthesize a variable for the docstring.
  ostringstream comment;
  if (!expected_params.empty()) {
    comment << "C++ Interface:\n"
            << expected_params;
  }

  if (func->_ifunc._comment.size() > 2) {
    if (!expected_params.empty()) {
      comment << "\n";
    }
    comment << func->_ifunc._comment;
  }

  out << "#ifndef NDEBUG\n";
  out << "static const char *" << func->_name << "_comment =\n";
  output_quoted(out, 2, comment.str());
  out << ";\n";
  out << "#else\n";
  out << "static const char *" << func->_name << "_comment = NULL;\n";
  out << "#endif\n\n";
}

////////////////////////////////////////////////////////////////////
/// Function  : write_function_for_name
//
//   Wrap a complete name override function for Py.....
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_function_for_name(ostream &out, Object *obj,
                        const Function::Remaps &remaps,
                        const string &function_name,
                        string &expected_params,
                        bool coercion_allowed,
                        ArgsType args_type, int return_flags) {
  std::map<int, std::set<FunctionRemap *> > map_sets;
  std::map<int, std::set<FunctionRemap *> >::iterator mii;
  std::set<FunctionRemap *>::iterator sii;

  bool has_this = false;
  Function::Remaps::const_iterator ri;
  FunctionRemap *remap = NULL;
  out << "/******************************************************************\n" << " * Python type method wrapper for\n";
  for (ri = remaps.begin(); ri != remaps.end(); ++ri) {
    remap = (*ri);
    if (is_remap_legal(remap)) {
      int parameter_size = remap->_parameters.size();
      if (remap->_has_this) {
        has_this = true;

        if (remap->_type != FunctionRemap::T_constructor) {
          parameter_size --;
        }
      }

      map_sets[parameter_size].insert(remap);
      out << " * ";
      remap->write_orig_prototype(out, 0);
      out << "\n";
    } else {
      out << " * Rejected Remap [";
      remap->write_orig_prototype(out, 0);
      out << "]\n";
    }
  }

  out << " *******************************************************************/\n";

  out << function_name << " {\n";

  if (has_this) {
    std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
    std::string cClassName = obj->_itype.get_true_name();

    // Extract pointer from 'self' parameter.
    out << "  " << cClassName << " *local_this = NULL;\n";
    out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
    out << "  if (local_this == NULL) {\n";
    out << "    PyErr_SetString(PyExc_AttributeError, \"C++ object is not yet constructed, or already destructed.\");\n";
    error_return(out, 4, return_flags);
    out << "  }\n";
  }

  if (map_sets.empty()) {
    error_return(out, 2, return_flags);
    out << "}\n\n";
    return;
  }

  if (map_sets.size() > 1) {
    switch (args_type) {
    case AT_keyword_args:
      indent(out, 2) << "int parameter_count = PyTuple_Size(args);\n";
      indent(out, 2) << "if (kwds != NULL) {\n";
      indent(out, 2) << "  parameter_count += PyDict_Size(kwds);\n";
      indent(out, 2) << "}\n";
      break;

    case AT_varargs:
      indent(out, 2) << "int parameter_count = PyTuple_Size(args);\n";
      break;

    case AT_single_arg:
      // It shouldn't get here, but we'll handle these cases nonetheless.
      indent(out, 2) << "const int parameter_count = 1;\n";
      break;

    default:
      indent(out, 2) << "const int parameter_count = 0;\n";
      break;
    }

    indent(out, 2) << "switch (parameter_count) {\n";
    for (mii = map_sets.begin(); mii != map_sets.end(); mii ++) {
      indent(out, 2) << "case " << mii->first << ": {\n";

      write_function_forset(out, mii->second, expected_params, 4,
                            coercion_allowed, true, args_type, return_flags, true);

      indent(out, 4) << "break;\n";
      indent(out, 2) << "}\n";
    }

    indent(out, 2) << "default:\n";
    indent(out, 4)
      << "PyErr_Format(PyExc_TypeError, \""
      << methodNameFromCppName(remap, "", false)
      << "() takes ";

    // We add one to the parameter count for "self", following the
    // Python convention.
    int add_self = has_this ? 1 : 0;
    size_t mic;
    for (mic = 0, mii = map_sets.begin();
         mii != map_sets.end();
         ++mii, ++mic) {
      if (mic == map_sets.size() - 1) {
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

    error_return(out, 4, return_flags);
    indent(out, 2) << "}\n";

    out << "  if (!PyErr_Occurred()) { // Let error pass on\n";
    out << "    PyErr_SetString(PyExc_TypeError,\n";
    out << "      \"Arguments must match one of:\\n\"\n";
    output_quoted(out, 6, expected_params);
    out << ");\n";
    out << "  }\n";

    error_return(out, 2, return_flags);

  } else {
    mii = map_sets.begin();

    // If no parameters are accepted, we do need to check that the argument
    // count is indeed 0, since we won't check that in write_function_instance.
    if (mii->first == 0 && args_type != AT_no_args) {
      switch (args_type) {
      case AT_keyword_args:
        out << "  if (PyTuple_Size(args) > 0 || (kwds != NULL && PyDict_Size(kwds) > 0)) {\n";
        out << "    int parameter_count = PyTuple_Size(args);\n";
        out << "    if (kwds != NULL) {\n";
        out << "      parameter_count += PyDict_Size(kwds);\n";
        out << "    }\n";
        break;
      case AT_varargs:
        out << "  if (PyTuple_Size(args) > 0) {\n";
        out << "    const int parameter_count = PyTuple_GET_SIZE(args);\n";
        break;
      case AT_single_arg:
        // Shouldn't happen, but let's handle this case nonetheless.
        out << "  {\n";
        out << "    const int parameter_count = 1;\n";
        break;
      case AT_no_args:
        break;
      case AT_unknown:
        break;
      }

      out << "    PyErr_Format(PyExc_TypeError,\n"
          << "                 \"" << methodNameFromCppName(remap, "", false)
          << "() takes no arguments (%d given)\",\n"
          << "                 parameter_count);\n";

      error_return(out, 4, return_flags);
      out << "  }\n";
    }

    write_function_forset(out, mii->second, expected_params, 2,
                          coercion_allowed, true, args_type, return_flags, true);

    out << "  if (!PyErr_Occurred()) {\n";
    out << "    PyErr_SetString(PyExc_TypeError,\n";
    out << "      \"Arguments must match:\\n\"\n";
    output_quoted(out, 6, expected_params);
    out << ");\n";
    out << "  }\n";

    error_return(out, 2, return_flags);
  }

  out << "}\n\n";
}

////////////////////////////////////////////////////////////////////
/// Function  : write_coerce_constructor
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_coerce_constructor(ostream &out, Object *obj, bool is_const) {
  std::map<int, std::set<FunctionRemap *> > map_sets;
  std::map<int, std::set<FunctionRemap *> >::iterator mii;
  std::set<FunctionRemap *>::iterator sii;

  Functions::iterator fi;
  Function::Remaps::const_iterator ri;

  for (fi = obj->_methods.begin(); fi != obj->_methods.end(); ++fi) {
    Function *func = (*fi);
    for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
      FunctionRemap *remap = (*ri);
      if (is_remap_legal(remap) && remap->_flags & FunctionRemap::F_coerce_constructor) {
        // It's a static make() function.
        CPPType *return_type = remap->_return_type->get_new_type();

        if (!is_const && TypeManager::is_const_pointer_or_ref(return_type)) {
          continue;
        }

        int parameter_size = remap->_parameters.size();
        map_sets[parameter_size].insert(remap);
      }
    }
  }

  for (fi = obj->_constructors.begin(); fi != obj->_constructors.end(); ++fi) {
    Function *func = (*fi);
    for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
      FunctionRemap *remap = (*ri);
      if (is_remap_legal(remap) && remap->_flags & FunctionRemap::F_coerce_constructor) {
        int parameter_size = remap->_parameters.size();
        map_sets[parameter_size].insert(remap);
      }
    }
  }

  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName = obj->_itype.get_true_name();

  if (is_const) {
    out << "bool Dtool_Coerce_" << ClassName << "(PyObject *args, " << cClassName << " const *&coerced) {\n";
  } else {
    out << "bool Dtool_Coerce_" << ClassName << "(PyObject *args, " << cClassName << " *&coerced) {\n";
  }

  // Special case for coerce constructor.
  out << "  DTOOL_Call_ExtractThisPointerForType(args, &Dtool_" << ClassName << ", (void**)&coerced);\n";
  out << "  if (coerced != NULL) {\n";
  out << "    // The argument is already of matching type, no need to coerce.\n";
  if (!is_const) {
    out << "    if (!((Dtool_PyInstDef *)args)->_is_const) {\n";
    out << "      // A non-const instance is required, which this is.\n";
    out << "      return false;\n";
    out << "    }\n";
  } else {
    out << "    return false;\n";
  }
  out << "  }\n\n";

  if (map_sets.empty()) {
    error_return(out, 2, RF_coerced | RF_err_false);
    out << "}\n\n";
    return;
  }

  string expected_params;
  mii = map_sets.find(1);
  if (mii != map_sets.end()) {
    out << "  if (!PyTuple_Check(args)) {\n";
    out << "    PyObject *arg = args;\n";

    write_function_forset(out, mii->second, expected_params, 4, false, false,
                          AT_single_arg, RF_coerced | RF_err_false, true);

    if (map_sets.size() == 1) {
      out << "  }\n";
      out << "  PyErr_Clear();\n";
      error_return(out, 2, RF_coerced | RF_err_false);
      out << "}\n\n";
      return;
    }

    map_sets.erase(mii);

    out << "  } else {\n";
  } else {
    out << "  if (PyTuple_Check(args)) {\n";
  }

  if (map_sets.size() > 1) {
    indent(out, 4) << "switch (PyTuple_GET_SIZE(args)) {\n";

    for (mii = map_sets.begin(); mii != map_sets.end(); mii ++) {
      indent(out, 6) << "case " << mii->first << ": {\n";

      write_function_forset(out, mii->second, expected_params, 8, false, false,
                            AT_varargs, RF_coerced | RF_err_false, true);

      indent(out, 8) << "break;\n";
      indent(out, 6) << "}\n";
    }
    indent(out, 4) << "}\n";

  } else {
    mii = map_sets.begin();

    indent(out, 4) << "if (PyTuple_GET_SIZE(args) == " << mii->first << ") {\n";
    write_function_forset(out, mii->second, expected_params, 6, false, false,
                          AT_varargs, RF_coerced | RF_err_false, true);
    indent(out, 4) << "}\n";
  }

  out << "  }\n\n";
  out << "  PyErr_Clear();\n";
  error_return(out, 2, RF_coerced | RF_err_false);
  out << "}\n\n";
}

////////////////////////////////////////////////////////
// Function : GetParnetDepth
//
// Support Function used to Sort the name based overrides.. For know must be complex to simple
////////////////////////////////////////////////////////
int get_type_sort(CPPType *type) {
  int answer = 0;
//  printf("    %s\n",type->get_local_name().c_str());

  // The highest numbered one will be checked first.
  if (TypeManager::is_pointer_to_Py_buffer(type)) {
    return 14;
  } else if (TypeManager::is_pointer_to_PyTypeObject(type)) {
    return 13;
  } else if (TypeManager::is_pointer_to_PyObject(type)) {
    return 12;
  } else if (TypeManager::is_wstring(type)) {
    return 11;
  } else if (TypeManager::is_wchar_pointer(type)) {
    return 10;
  } else if (TypeManager::is_string(type)) {
    return 9;
  } else if (TypeManager::is_char_pointer(type)) {
    return 8;
  } else if (TypeManager::is_unsigned_longlong(type)) {
    return 7;
  } else if (TypeManager::is_longlong(type)) {
    return 6;
  } else if (TypeManager::is_integer(type)) {
    return 5;
  } else if (TypeManager::is_double(type)) {
    return 4;
  } else if (TypeManager::is_float(type)) {
    return 3;
  } else if (TypeManager::is_pointer_to_simple(type)) {
    return 2;
  } else if (TypeManager::is_bool(type)) {
    return 1;
  } else if (TypeManager::is_pointer(type) ||
             TypeManager::is_reference(type) ||
             TypeManager::is_struct(type)) {
    answer = 20;
    int deepest = 0;
    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)), false);
    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
    const InterrogateType &itype = idb->get_type(type_index);

    if (itype.is_class() || itype.is_struct()) {
      int num_derivations = itype.number_of_derivations();
      for (int di = 0; di < num_derivations; di++) {
        TypeIndex d_type_Index = itype.get_derivation(di);
        const InterrogateType &d_itype = idb->get_type(d_type_Index);
        int this_one = get_type_sort(d_itype._cpptype);
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
bool RemapCompareLess(FunctionRemap *in1, FunctionRemap *in2) {
  assert(in1 != NULL);
  assert(in2 != NULL);

  if (in1->_parameters.size() != in2->_parameters.size()) {
    return (in1->_parameters.size() > in2->_parameters.size());
  }

  int pcount = in1->_parameters.size();
  for (int x = 0; x < pcount; x++) {
    CPPType *orig_type1 = in1->_parameters[x]._remap->get_orig_type();
    CPPType *orig_type2 = in2->_parameters[x]._remap->get_orig_type();

    int pd1 = get_type_sort(orig_type1);
    int pd2 = get_type_sort(orig_type2);
    if (pd1 != pd2) {
      return (pd1 > pd2);
    }
  }

  // ok maybe something to do with return strength..

  return false;
}

///////////////////////////////////////////////////////////
//  Function  : write_function_forset
//
//  A set is defined as all remaps that have the same number of paramaters..
///////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_function_forset(ostream &out,
                      const std::set<FunctionRemap *> &remapsin,
                      string &expected_params, int indent_level,
                      bool coercion_allowed, bool report_errors,
                      ArgsType args_type, int return_flags,
                      bool check_exceptions,
                      const string &first_pexpr) {

  if (remapsin.empty()) {
    return;
  }

  if (remapsin.size() > 1) {
    // There are multiple different overloads for this number of
    // parameters.  Sort them all into order from most-specific to
    // least-specific, then try them one at a time.
    std::vector<FunctionRemap *> remaps (remapsin.begin(), remapsin.end());
    std::sort(remaps.begin(), remaps.end(), RemapCompareLess);

    std::vector<FunctionRemap *>::iterator sii;
    for (sii = remaps.begin(); sii != remaps.end(); sii ++) {
      FunctionRemap *remap = (*sii);
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

      // NB.  We don't pass on report_errors here because we want
      // it to silently drop down to the next overload.

      write_function_instance(out, remap, expected_params, indent_level + 2,
                              false, false, args_type, return_flags,
                              check_exceptions, first_pexpr);

      indent(out, indent_level + 2) << "PyErr_Clear();\n";
      indent(out, indent_level) << "}\n\n";
    }

    // Go through one more time, but allow coercion this time.
    if (coercion_allowed) {
      for (sii = remaps.begin(); sii != remaps.end(); sii ++) {
        FunctionRemap *remap = (*sii);
        if (!is_remap_coercion_possible(remap)) {
          continue;
        }

        if (remap->_has_this && !remap->_const_method) {
          indent(out, indent_level)
            << "if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
        } else {
          indent(out, indent_level)
            << "{\n";
        }

        indent(out, indent_level) << "// -2 ";
        remap->write_orig_prototype(out, 0); out << "\n";

        string ignore_expected_params;
        write_function_instance(out, remap, ignore_expected_params, indent_level + 2,
                                true, false, args_type, return_flags,
                                check_exceptions, first_pexpr);

        indent(out, indent_level + 2) << "PyErr_Clear();\n";
        indent(out, indent_level) << "}\n\n";
      }
    }
  } else {
    // There is only one possible overload with this number of
    // parameters.  Just call it.
    std::set<FunctionRemap *>::iterator sii;
    sii = remapsin.begin();

    FunctionRemap *remap = (*sii);
    if (remap->_has_this && !remap->_const_method) {
      // If it's a non-const method, we only allow a
      // non-const this.
      indent(out, indent_level)
        << "if (!((Dtool_PyInstDef *)self)->_is_const) {\n";
      indent_level += 2;
    }

    indent(out, indent_level)
      << "// 1-" ;
    remap->write_orig_prototype(out, 0);
    out << "\n";

    write_function_instance(out, remap, expected_params, indent_level,
                            coercion_allowed, report_errors,
                            args_type, return_flags,
                            check_exceptions, first_pexpr);

    if (remap->_has_this && !remap->_const_method) {
      if (report_errors) {
        indent(out, indent_level - 2)
          << "} else {\n";
        indent(out, indent_level)
          << "PyErr_SetString(PyExc_TypeError,\n";
        string class_name = remap->_cpptype->get_simple_name();
        indent(out, indent_level)
          << "                \"Cannot call "
          << classNameFromCppName(class_name, false)
          << "." << methodNameFromCppName(remap, class_name, false)
          << "() on a const object.\");\n";

        error_return(out, indent_level, return_flags);
      }
      indent_level -= 2;
      indent(out, indent_level) << "}\n\n";
    } else {
      out << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::write_function_instance
//       Access: Private
//  Description: Writes out the particular function that handles a
//               single instance of an overloaded function.
//
//               return_flags indicates which value should be
//               returned from the wrapper function and what should
//               be returned on error.
//
//               If coercion_possible is true, it will attempt
//               to convert arguments to the appropriate parameter
//               type using the appropriate Dtool_Coerce function.
//
//               If report_errors is true, it will print an error
//               and exit when one has occurred, instead of falling
//               back to the next overload.  This should be done
//               if it is the only overload.
//
//               If check_exceptions is false, it will not check
//               if the function raised an exception, except if
//               it took PyObject* arguments.  This should NEVER
//               be done for C++ functions that call Python code.
//
//               If first_pexpr is not empty, it represents the
//               preconverted value of the first parameter.  This
//               is a special-case hack for one of the slot functions.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
write_function_instance(ostream &out,
                        FunctionRemap *remap, string &expected_params,
                        int indent_level,
                        bool coercion_possible, bool report_errors,
                        ArgsType args_type, int return_flags,
                        bool check_exceptions,
                        const string &first_pexpr) {
  string format_specifiers;
  std::string keyword_list;
  string parameter_list;
  string container;
  vector_string pexprs;
  LineStream extra_convert;
  ostringstream extra_param_check;
  LineStream extra_cleanup;
  int min_version = 0;

  bool is_constructor = (remap->_type == FunctionRemap::T_constructor);

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  // Make one pass through the parameter list.  We will output a
  // one-line temporary variable definition for each parameter, while
  // simultaneously building the ParseTuple() function call and also
  // the parameter expression list for call_function().

  expected_params += methodNameFromCppName(remap, "", false);
  expected_params += "(";

  int num_params = 0;
  bool only_pyobjects = true;

  int pn;
  for (pn = 0; pn < (int)remap->_parameters.size(); ++pn) {
    if (pn > 0) {
      expected_params += ", ";
    }

    if (((remap->_has_this && pn == 1) ||
        (!remap->_has_this && pn == 0)) && !first_pexpr.empty()) {
      // The first param was already converted.
      pexprs.push_back(first_pexpr);
      continue;
    }

    CPPType *orig_type = remap->_parameters[pn]._remap->get_orig_type();
    CPPType *type = remap->_parameters[pn]._remap->get_new_type();
    string param_name = remap->get_parameter_name(pn);

    // This is the string to convert our local variable to the
    // appropriate C++ type.  Normally this is just a cast.
    string pexpr_string =
      "(" + orig_type->get_local_name(&parser) + ")" + param_name;

    const string &reported_name = remap->_parameters[pn]._name;

    if (!remap->_has_this || pn != 0) {
      keyword_list += "(char *)\"" + reported_name + "\", ";
    }

    if (remap->_parameters[pn]._remap->new_type_is_atomic_string()) {

      if (TypeManager::is_char_pointer(orig_type)) {
        indent(out, indent_level) << "char *" << param_name << ";\n";
        format_specifiers += "z";
        parameter_list += ", &" + param_name;
        expected_params += "str";

      } else if (TypeManager::is_wchar_pointer(orig_type)) {
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        extra_convert
          << "#if PY_VERSION_HEX >= 0x03030000\n"
          << "wchar_t *" << param_name << "_str = PyUnicode_AsWideCharString((PyObject *)" << param_name << ", NULL);\n"
          << "#else"
          << "Py_ssize_t " << param_name << "_len = PyUnicode_GET_SIZE(" << param_name << ");\n"
          << "wchar_t *" << param_name << "_str = (wchar_t *)alloca(sizeof(wchar_t) * (" + param_name + "_len + 1));\n"
          << "PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_str, " << param_name << "_len);\n"
          << param_name << "_str[" << param_name << "_len] = 0;\n"
          << "#endif\n";

        pexpr_string = param_name + "_str";

        extra_cleanup
          << "#if PY_VERSION_HEX >= 0x03030000\n"
          << "PyMem_Free(" << param_name << "_str);\n"
          << "#endif\n";

        expected_params += "unicode";

      } else if (TypeManager::is_wstring(orig_type)) {
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        extra_convert
          << "#if PY_VERSION_HEX >= 0x03030000\n"
          << "Py_ssize_t " << param_name << "_len;\n"
          << "wchar_t *" << param_name << "_str = PyUnicode_AsWideCharString((PyObject *)"
          << param_name << ", &" << param_name << "_len);\n"
          << "#else\n"
          << "Py_ssize_t " << param_name << "_len = PyUnicode_GET_SIZE(" << param_name << ");\n"
          << "wchar_t *" << param_name << "_str = (wchar_t *)alloca(sizeof(wchar_t) * (" + param_name + "_len + 1));\n"
          << "PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_str, " << param_name << "_len);\n"
          << "#endif\n";

        pexpr_string = "basic_string<wchar_t>(" +
          param_name + "_str, " + param_name + "_len)";

        extra_cleanup
          << "#if PY_VERSION_HEX >= 0x03030000\n"
          << "PyMem_Free(" << param_name << "_str);\n"
          << "#endif\n";

        expected_params += "unicode";

      } else if (TypeManager::is_const_ptr_to_basic_string_wchar(orig_type)) {
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        extra_convert
          << "#if PY_VERSION_HEX >= 0x03030000\n"
          << "Py_ssize_t " << param_name << "_len;\n"
          << "wchar_t *" << param_name << "_str = PyUnicode_AsWideCharString((PyObject *)"
          << param_name << ", &" << param_name << "_len);\n"
          << "#else\n"
          << "Py_ssize_t " << param_name << "_len = PyUnicode_GET_SIZE(" << param_name << ");\n"
          << "wchar_t *" << param_name << "_str = (wchar_t *)alloca(sizeof(wchar_t) * (" + param_name + "_len + 1));\n"
          << "PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_str, " << param_name << "_len);\n"
          << "#endif\n";

        pexpr_string = "&basic_string<wchar_t>(" +
          param_name + "_str, " + param_name + "_len)";

        extra_cleanup
          << "#if PY_VERSION_HEX >= 0x03030000\n"
          << "PyMem_Free(" << param_name << "_str);\n"
          << "#endif\n";

        expected_params += "unicode";

      } else { // A regular string.
        indent(out, indent_level) << "char *" << param_name << "_str = NULL;\n";
        indent(out, indent_level) << "Py_ssize_t " << param_name << "_len;\n";

        if (args_type == AT_single_arg) {
          out << "#if PY_MAJOR_VERSION >= 3\n";
          indent(out, indent_level)
            << param_name << "_str = PyUnicode_AsUTF8AndSize(arg, &"
            << param_name << "_len);\n";
          out << "#else\n"; // NB. PyString_AsStringAndSize also accepts a PyUnicode.
          indent(out, indent_level) << "if (PyString_AsStringAndSize(arg, &"
            << param_name << "_str, &" << param_name << "_len) == -1) {\n";
          indent(out, indent_level + 2) << param_name << "_str = NULL;\n";
          indent(out, indent_level) << "}\n";
          out << "#endif\n";

          extra_param_check << " && " << param_name << "_str != NULL";
        } else {
          format_specifiers += "s#";
          parameter_list += ", &" + param_name
            + "_str, &" + param_name + "_len";
        }

        if (TypeManager::is_const_ptr_to_basic_string_char(orig_type)) {
          pexpr_string = "&basic_string<char>(" +
            param_name + "_str, " + param_name + "_len)";
        } else {
          pexpr_string = "basic_string<char>(" +
            param_name + "_str, " + param_name + "_len)";
        }
        expected_params += "str";
      }
      only_pyobjects = false;
      ++num_params;

    } else if (TypeManager::is_bool(type)) {
      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      pexpr_string = "(PyObject_IsTrue(" + param_name + ") != 0)";
      expected_params += "bool";
      ++num_params;

    } else if (TypeManager::is_char(type)) {
      indent(out, indent_level) << "char " << param_name << ";\n";

      format_specifiers += "c";
      parameter_list += ", &" + param_name;

      extra_param_check << " && isascii(" << param_name << ")";
      pexpr_string = "(char) " + param_name;
      expected_params += "char";
      only_pyobjects = false;
      ++num_params;

    } else if (TypeManager::is_wchar(type)) {
      indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
      format_specifiers += "U";
      parameter_list += ", &" + param_name;

      // We tell it to copy 2 characters, but make sure it only
      // copied one, as a trick to check for the proper length in one go.
      extra_convert << "wchar_t " << param_name << "_chars[2];\n";
      extra_param_check << " && PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_chars, 2) == 1";

      pexpr_string = param_name + "_chars[0]";
      expected_params += "unicode char";
      only_pyobjects = false;
      ++num_params;

    } else if (TypeManager::is_longlong(type)) {
      if (TypeManager::is_unsigned_longlong(type)) {
        indent(out, indent_level) << "unsigned PY_LONG_LONG " << param_name << ";\n";
        format_specifiers += "K";
      } else {
        indent(out, indent_level) << "PY_LONG_LONG " << param_name << ";\n";
        format_specifiers += "L";
      }
      parameter_list += ", &" + param_name;
      expected_params += "long";
      only_pyobjects = false;
      ++num_params;

    } else if (TypeManager::is_integer(type)) {
      if (args_type == AT_single_arg) {
        pexpr_string = "(" + type->get_local_name(&parser) + ")PyInt_AS_LONG(arg)";
        extra_param_check << " && PyInt_Check(arg)";
      } else {
        if (TypeManager::is_unsigned_integer(type)) {
          indent(out, indent_level) << "unsigned int " << param_name << ";\n";
          format_specifiers += "I";
        } else {
          indent(out, indent_level) << "int " << param_name << ";\n";
          format_specifiers += "i";
        }
        parameter_list += ", &" + param_name;
      }
      expected_params += "int";
      only_pyobjects = false;
      ++num_params;

    } else if (TypeManager::is_double(type)) {
      if (args_type == AT_single_arg) {
        pexpr_string = "PyFloat_AsDouble(arg)";
        extra_param_check << " && PyNumber_Check(arg)";
      } else {
        indent(out, indent_level) << "double " << param_name << ";\n";
        format_specifiers += "d";
        parameter_list += ", &" + param_name;
      }
      expected_params += "double";
      only_pyobjects = false;
      ++num_params;

    } else if (TypeManager::is_float(type)) {
      if (args_type == AT_single_arg) {
        pexpr_string = "(float) PyFloat_AsDouble(arg)";
        extra_param_check << " && PyNumber_Check(arg)";
      } else {
        indent(out, indent_level) << "float " << param_name << ";\n";
        format_specifiers += "f";
        parameter_list += ", &" + param_name;
      }
      expected_params += "float";
      only_pyobjects = false;
      ++num_params;

    } else if (TypeManager::is_const_char_pointer(type)) {
      indent(out, indent_level) << "const char *" << param_name << ";\n";
      format_specifiers += "z";
      parameter_list += ", &" + param_name;
      expected_params += "buffer";
      only_pyobjects = false;
      ++num_params;

    } else if (TypeManager::is_pointer_to_PyTypeObject(type)) {
      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
        pexpr_string = param_name;
      }
      extra_param_check << " && PyType_Check(" << param_name << ")";
      pexpr_string = "(PyTypeObject *)" + param_name;
      expected_params += "type";
      ++num_params;

      // It's reasonable to assume that a function taking a PyTypeObject
      // might also throw a TypeError if the type is incorrect.
      check_exceptions = true;

    } else if (TypeManager::is_pointer_to_PyStringObject(type)) {
      if (args_type == AT_single_arg) {
        // This is a single-arg function, so there's no need
        // to convert anything.
        param_name = "arg";
        extra_param_check << " && PyString_Check(arg)";
        pexpr_string = "(PyStringObject *)" + param_name;
      } else {
        indent(out, indent_level) << "PyStringObject *" << param_name << ";\n";
        format_specifiers += "S";
        parameter_list += ", &" + param_name;
        pexpr_string = param_name;
      }
      expected_params += "string";
      ++num_params;

    } else if (TypeManager::is_pointer_to_PyUnicodeObject(type)) {
      if (args_type == AT_single_arg) {
        // This is a single-arg function, so there's no need
        // to convert anything.
        param_name = "arg";
        extra_param_check << " && PyUnicode_Check(arg)";
        pexpr_string = "(PyUnicodeObject *)" + param_name;
      } else {
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;
        pexpr_string = param_name;
      }
      expected_params += "unicode";
      ++num_params;

    } else if (TypeManager::is_pointer_to_PyObject(type)) {
      if (args_type == AT_single_arg) {
        // This is a single-arg function, so there's no need
        // to convert anything.
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      pexpr_string = param_name;
      expected_params += "any";
      ++num_params;

      // It's reasonable to assume that a function taking a PyObject
      // might also throw a TypeError if the type is incorrect.
      check_exceptions = true;

    } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
      min_version = 0x02060000; // Only support this remap in version 2.6+.

      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      indent(out, indent_level) << "Py_buffer " << param_name << "_view;\n";

      extra_param_check << " && PyObject_GetBuffer("
                        << param_name << ", &"
                        << param_name << "_view, PyBUF_FULL) == 0";
      pexpr_string = "&" + param_name + "_view";
      extra_cleanup << "PyBuffer_Release(&" << param_name << "_view);\n";
      expected_params += "buffer";
      ++num_params;
      check_exceptions = true;

    } else if (TypeManager::is_pointer_to_simple(type)) {
      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      indent(out, indent_level) << "Py_buffer " << param_name << "_view;\n";

      // Unravel the type to determine its properties.
      int array_len = -1;
      bool is_const = true;
      CPPSimpleType *simple = NULL;
      CPPType *unwrap = TypeManager::unwrap_const_reference(type);
      if (unwrap != NULL) {
        CPPArrayType *array_type = unwrap->as_array_type();
        CPPPointerType *pointer_type = unwrap->as_pointer_type();

        if (array_type != NULL) {
          if (array_type->_bounds != NULL) {
            array_len = array_type->_bounds->evaluate().as_integer();
          }
          unwrap = array_type->_element_type;
        } else if (pointer_type != NULL) {
          unwrap = pointer_type->_pointing_at;
        }

        CPPConstType *const_type = unwrap->as_const_type();
        if (const_type != NULL) {
          unwrap = const_type->_wrapped_around;
        } else {
          is_const = false;
        }

        while (unwrap->get_subtype() == CPPDeclaration::ST_typedef) {
          unwrap = unwrap->as_typedef_type()->_type;
        }
        simple = unwrap->as_simple_type();
      }

      // Determine the format, so we can check the type of the buffer we get.
      char format_chr = 'B';

      switch (simple->_type) {
      case CPPSimpleType::T_char:
        if (simple->_flags & CPPSimpleType::F_unsigned) {
          format_chr = 'B';
        } else if (simple->_flags & CPPSimpleType::F_signed) {
          format_chr = 'b';
        } else {
          format_chr = 'c';
        }
        break;

      case CPPSimpleType::T_int:
        if (simple->_flags & CPPSimpleType::F_longlong) {
          format_chr = 'q';
        } else if (simple->_flags & CPPSimpleType::F_long) {
          format_chr = 'l';
        } else if (simple->_flags & CPPSimpleType::F_short) {
          format_chr = 'h';
        } else {
          format_chr = 'i';
        }

        if (simple->_flags & CPPSimpleType::F_unsigned) {
          format_chr &= 0x5f; // Uppercase
        }
        break;

      case CPPSimpleType::T_float:
        format_chr = 'f';
        break;

      case CPPSimpleType::T_double:
        format_chr = 'd';
        break;

      default:
        nout << "Warning: cannot determine buffer format string for type "
             << type->get_local_name(&parser)
             << " (simple type " << *simple << ")\n";
        extra_param_check << " && false";
      }

      const char *flags;
      if (format_chr == 'B') {
        if (is_const) {
          flags = "PyBUF_SIMPLE";
        } else {
          flags = "PyBUF_WRITABLE";
        }
      } else if (is_const) {
        flags = "PyBUF_FORMAT";
      } else {
        flags = "PyBUF_FORMAT | PyBUF_WRITABLE";
      }

      extra_param_check << " && PyObject_GetBuffer(" << param_name << ", &"
                        << param_name << "_view, " << flags << ") == 0";

      if (format_chr != 'B') {
        extra_param_check
          << " && " << param_name << "_view.format[0] == '" << format_chr << "'"
          << " && " << param_name << "_view.format[1] == 0";
      }
      if (array_len != -1) {
        extra_param_check
          << " && " << param_name << "_view.len == " << array_len;
      }

      pexpr_string = "(" + simple->get_local_name(&parser) + " *)" +
                     param_name + "_view.buf";

      extra_cleanup << "PyBuffer_Release(&" << param_name << "_view);\n";
      expected_params += "buffer";
      ++num_params;

    } else if (TypeManager::is_pointer(type)) {
      CPPType *obj_type = TypeManager::unwrap(TypeManager::resolve_type(type));
      bool const_ok = !TypeManager::is_non_const_pointer_or_ref(orig_type);

      if (TypeManager::is_const_pointer_or_ref(orig_type)) {
        expected_params += "const ";
      //} else {
      //  expected_params += "non-const ";
      }
      string expected_class_name = classNameFromCppName(obj_type->get_simple_name(), false);
      expected_params += expected_class_name;

      if (!remap->_has_this || pn != 0) {
        if (args_type == AT_single_arg) {
          param_name = "arg";
        } else {
          indent(out, indent_level) << "PyObject *" << param_name << ";\n";
          format_specifiers += "O";
          parameter_list += ", &" + param_name;
        }

        ++num_params;

        string class_name = obj_type->get_local_name(&parser);

        // need to a forward scope for this class..
        if (!isExportThisRun(obj_type)) {
          _external_imports.insert(obj_type);
        }

        string this_class_name;
        string method_prefix;
        if (remap->_cpptype) {
          this_class_name = remap->_cpptype->get_simple_name();
          method_prefix = classNameFromCppName(this_class_name, false) + string(".");
        }

        //if (const_ok) {
        //  extra_convert << "const ";
        //}

        //extra_convert
        //  << class_name << " *" << param_name << "_this";
        type->output_instance(extra_convert, param_name + "_this", &parser);

        if (coercion_possible) {
          if (has_coerce_constructor(obj_type->as_struct_type()) == 0) {
            // Doesn't actually have a coerce constructor.
            coercion_possible = false;
          }
        }

        if (coercion_possible) {
          // Call the coercion function directly, which will try to
          // extract the pointer directly before trying coercion.
          extra_convert
            << ";\n"
            << "bool " << param_name << "_manage = "
            << "Dtool_Coerce_" << make_safe_name(class_name)
            << "(" << param_name << ", " << param_name << "_this);\n";

          extra_cleanup << "if (" << param_name << "_manage) {\n";

          if (manage_reference_counts &&
              TypeManager::is_reference_count(obj_type)) {
            extra_cleanup << "  unref_delete(" << param_name << "_this);\n";
          } else {
            extra_cleanup << "  delete " << param_name << "_this;\n";
          }
          extra_cleanup << "}\n";

        } else {
          extra_convert << boolalpha
            << " = (" << class_name << " *)"
            << "DTOOL_Call_GetPointerThisClass(" << param_name
            << ", &Dtool_" << make_safe_name(class_name)
            << ", " << pn << ", \""
            << method_prefix << methodNameFromCppName(remap, this_class_name, false)
            << "\", " << const_ok << ", " << report_errors << ");\n";
        }

        if (report_errors && coercion_possible) {
          // Display error like: Class.func() argument 0 must be A, not B
          extra_convert
            << "if (" << param_name << "_this == NULL) {\n"
            << "  PyTypeObject *tp = Py_TYPE(" << param_name << ");\n"
            << "  PyErr_Format(PyExc_TypeError,\n"
            << "               \"" << method_prefix
            << methodNameFromCppName(remap, this_class_name, false)
            << "() argument " << pn << " must be "
            << expected_class_name << ", not %s\",\n"
            << "               tp->tp_name);\n";
          error_return(extra_convert, 2, return_flags);
          extra_convert << "}\n";
        } else {
          extra_param_check << " && " << param_name << "_this != NULL";
        }
        pexpr_string =  param_name + "_this";
      }

    } else {
      // Ignore a parameter.
      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      expected_params += "any";
      ++num_params;
    }

    if (!reported_name.empty()) {
      expected_params += " " + reported_name;
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

  if (min_version > 0) {
    out << "#if PY_VERSION_HEX >= 0x" << hex << min_version << dec << "\n";
  }

  // Track how many curly braces we've opened.
  short open_scopes = 0;

  if (!format_specifiers.empty()) {
    string method_name = methodNameFromCppName(remap, "", false);

    switch (args_type) {
    case AT_keyword_args:
      // Wrapper takes a varargs tuple and a keyword args dict.
      indent(out, indent_level)
        << "static char *keyword_list[] = {" << keyword_list << "NULL};\n";
      indent(out, indent_level)
        << "if (PyArg_ParseTupleAndKeywords(args, kwds, \""
        << format_specifiers << ":" << method_name
        << "\", keyword_list" << parameter_list << ")) {\n";

      ++open_scopes;
      indent_level += 2;
      break;

    case AT_varargs:
      // Wrapper takes a varargs tuple.
      if (only_pyobjects) {
        // All parameters are PyObject*, so we can use the slightly
        // more efficient PyArg_UnpackTuple function instead.
        indent(out, indent_level)
          << "if (PyArg_UnpackTuple(args, \""
          << methodNameFromCppName(remap, "", false)
          << "\", " << num_params
          << ", " << num_params
          << parameter_list << ")) {\n";

      } else {
        indent(out, indent_level)
          << "if (PyArg_ParseTuple(args, \""
          << format_specifiers << ":" << method_name
          << "\"" << parameter_list << ")) {\n";
      }
      ++open_scopes;
      indent_level += 2;
      break;

    case AT_single_arg:
      // Single argument.  If not a PyObject*, use PyArg_Parse.
      if (!only_pyobjects && format_specifiers != "O") {
        indent(out, indent_level)
          << "if (PyArg_Parse(arg, \"" << format_specifiers << ":"
          << method_name << "\"" << parameter_list << ")) {\n";

        ++open_scopes;
        indent_level += 2;
      }

    default:
      break;
    }
  }

  while (extra_convert.is_text_available()) {
    string line = extra_convert.get_line();
    if (line.size() == 0 || line[0] == '#') {
      out << line << "\n";
    } else {
      indent(out, indent_level) << line << "\n";
    }
  }

  string extra_param_check_str = extra_param_check.str();
  if (!extra_param_check_str.empty()) {
    indent(out, indent_level)
      << "if (" << extra_param_check_str.substr(4) << ") {\n";

    ++open_scopes;
    indent_level += 2;
  }

  if (!remap->_has_this && (remap->_flags & FunctionRemap::F_explicit_self) != 0) {
    // If we'll be passing "self" to the constructor, we need to
    // pre-initialize it here.  Unfortunately, we can't pre-load the
    // "this" pointer, but the constructor itself can do this.

    CPPType *orig_type = remap->_return_type->get_orig_type();
    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)), false);
    const InterrogateType &itype = idb->get_type(type_index);

    indent(out, indent_level)
      << "// Pre-initialize self for the constructor\n";

    if (!is_constructor || (return_flags & RF_int) == 0) {
      // This is not a constructor, but somehow we landed up here at a
      // static method requiring a 'self' pointer.  This happens in
      // coercion constructors in particular.  We'll have to create
      // a temporary PyObject instance to pass to it.

      indent(out, indent_level)
        << "PyObject *self = Dtool_new_"
        << make_safe_name(itype.get_scoped_name()) << "(&"
        << CLASS_PREFIX << make_safe_name(itype.get_scoped_name())
        << "._PyType, NULL, NULL);\n";

      extra_cleanup << "PyObject_Del(self);\n";
    } else {
      //XXX rdb: this isn't needed, is it, because tp_new already
      // initializes the instance?
      indent(out, indent_level)
        << "DTool_PyInit_Finalize(self, NULL, &"
        << CLASS_PREFIX << make_safe_name(itype.get_scoped_name())
        << ", false, false);\n";
    }
  }

  string return_expr;

  if (remap->_blocking) {
    // With SIMPLE_THREADS, it's important that we never release the
    // interpreter lock.
    out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
    indent(out, indent_level)
      << "PyThreadState *_save;\n";
    indent(out, indent_level)
      << "Py_UNBLOCK_THREADS\n";
    out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
  }
  if (track_interpreter) {
    indent(out, indent_level) << "in_interpreter = 0;\n";
  }

  if (remap->_return_type->new_type_is_atomic_string()) {
    // Treat strings as a special case.  We don't want to format the
    // return expression.
    return_expr = remap->call_function(out, indent_level, false, container, pexprs);
    CPPType *type = remap->_return_type->get_orig_type();
    indent(out, indent_level);
    type->output_instance(out, "return_value", &parser);
    out << " = " << return_expr << ";\n";

  } else {
    return_expr = remap->call_function(out, indent_level, true, container, pexprs);

    if (return_flags & RF_self) {
      // We won't be using the return value, anyway.
      return_expr.clear();
    }

    if (!return_expr.empty()) {
      CPPType *type = remap->_return_type->get_temporary_type();
      indent(out, indent_level);
      type->output_instance(out, "return_value", &parser);
      out << " = " << return_expr << ";\n";
    }
  }

  // Clean up any memory we might have allocate for parsing the parameters.
  while (extra_cleanup.is_text_available()) {
    string line = extra_cleanup.get_line();
    if (line.size() == 0 || line[0] == '#') {
      out << line << "\n";
    } else {
      indent(out, indent_level) << line << "\n";
    }
  }

  if (track_interpreter) {
    indent(out, indent_level) << "in_interpreter = 1;\n";
  }
  if (remap->_blocking) {
    out << "#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)\n";
    indent(out, indent_level)
      << "Py_BLOCK_THREADS\n";
    out << "#endif  // HAVE_THREADS && !SIMPLE_THREADS\n";
  }

  if (!return_expr.empty()) {
    if (remap->_return_type->return_value_needs_management()) {
      // If a constructor returns NULL, that means allocation failed.
      indent(out, indent_level) << "if (return_value == NULL) {\n";
      indent(out, indent_level) << "  PyErr_NoMemory();\n";
      error_return(out, indent_level + 2, return_flags);
      indent(out, indent_level) << "}\n";
    }

    return_expr = manage_return_value(out, indent_level, remap, "return_value");
    return_expr = remap->_return_type->temporary_to_return(return_expr);
  }

  // Generated getters and setters don't raise exceptions or asserts
  // since they don't contain any code.
  if (remap->_type != FunctionRemap::T_getter &&
      remap->_type != FunctionRemap::T_setter) {
    // If a method raises TypeError, continue.
    if (check_exceptions) {
      indent(out, indent_level)
        << "if (PyErr_Occurred()) {\n";
      if (!return_expr.empty()) {
        delete_return_value(out, indent_level + 2, remap, return_expr);
      }

      if (report_errors) {
        error_return(out, indent_level + 2, return_flags);
      } else {
        indent(out, indent_level)
          << "  if (PyErr_ExceptionMatches(PyExc_TypeError)) {\n";
        indent(out, indent_level)
          << "    // TypeError raised; continue to next overload type.\n";
        indent(out, indent_level)
          << "  } else {\n";
        error_return(out, indent_level + 4, return_flags);
        indent(out, indent_level)
          << "  }\n";
      }
      indent(out, indent_level)
        << "} else {\n";

      ++open_scopes;
      indent_level += 2;
    }

    // Outputs code to check to see if an assertion has failed while
    // the C++ code was executing, and report this failure back to Python.
    if (watch_asserts && (return_flags & RF_coerced) == 0) {
      out << "#ifndef NDEBUG\n";
      indent(out, indent_level)
        << "Notify *notify = Notify::ptr();\n";
      indent(out, indent_level)
        << "if (notify->has_assert_failed()) {\n";
      indent(out, indent_level + 2)
        << "PyErr_SetString(PyExc_AssertionError, notify->get_assert_error_message().c_str());\n";
      indent(out, indent_level + 2)
        << "notify->clear_assert_failed();\n";
      if (!return_expr.empty()) {
        delete_return_value(out, indent_level + 2, remap, return_expr);
      }
      error_return(out, indent_level + 2, return_flags);

      indent(out, indent_level)
        << "}\n";
      out << "#endif\n";
    }
  }

  if (return_flags & RF_decref_args) {
    indent(out, indent_level) << "Py_DECREF(args);\n";
  }

  if (return_flags & RF_int) {
    CPPType *orig_type = remap->_return_type->get_orig_type();
    if (is_constructor) {
      // Special case for constructor.
      TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)), false);
      const InterrogateType &itype = idb->get_type(type_index);
      indent(out, indent_level)
        << "return DTool_PyInit_Finalize(self, " << return_expr << ", &" << CLASS_PREFIX  << make_safe_name(itype.get_scoped_name()) << ", true, false);\n";

    } else if (TypeManager::is_integer(orig_type)) {
      indent(out, indent_level) << "return " << return_expr << ";\n";

    } else if (TypeManager::is_void(orig_type)) {
      indent(out, indent_level) << "return 0;\n";

    } else {
      cerr << "Warning: function has return type " << *orig_type
           << ", expected int or void:\n" << expected_params << "\n";
      indent(out, indent_level) << "// Don't know what to do with return type "
                                << *orig_type << ".\n";
      indent(out, indent_level) << "return 0;\n";
    }

  } else if (return_flags & RF_self) {
    indent(out, indent_level) << "Py_INCREF(self);\n";
    indent(out, indent_level) << "return self;\n";

  } else if (return_flags & RF_pyobject) {
    if (return_expr.empty()) {
      indent(out, indent_level) << "Py_INCREF(Py_None);\n";
      indent(out, indent_level) << "return Py_None;\n";

    } else {
      pack_return_value(out, indent_level, remap, return_expr);
    }

  } else if (return_flags & RF_coerced) {
    // Special case for make function: cast to the right type.
    if (!is_constructor && (remap->_flags & FunctionRemap::F_coerce_constructor) != 0) {
      CPPType *new_type = remap->_return_type->get_new_type();

      CPPType *return_type = remap->_cpptype;
      if (TypeManager::is_const_pointer_to_anything(new_type)) {
        return_type = CPPType::new_type(new CPPConstType(return_type));
      }

      if (IsPandaTypedObject(remap->_cpptype->as_struct_type())) {
        indent(out, indent_level) << "coerced = DCAST("
          << return_type->get_local_name(&parser)
          << ", " << return_expr << ");\n";
      } else {
        return_type = CPPType::new_type(new CPPPointerType(return_type));
        indent(out, indent_level) << "coerced = ("
          << return_type->get_local_name(&parser)
          << ") " << return_expr << ";\n";
      }
    } else {
      indent(out, indent_level) << "coerced = " << return_expr << ";\n";
    }

    if (remap->_return_type->return_value_needs_management()) {
      indent(out, indent_level) << "return true;\n";
    } else {
      indent(out, indent_level) << "return false;\n";
    }
  }

  // Close the extra braces opened earlier.
  while (open_scopes > 0) {
    indent_level -= 2;
    indent(out, indent_level) << "}\n";

    --open_scopes;
  }

  if (min_version > 0) {
    // Close the #if PY_VERSION_HEX check.
    out << "#endif\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::error_return
//       Access: Private
//  Description: Outputs the correct return statement that should be
//               used in case of error based on the ReturnFlags.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
error_return(ostream &out, int indent_level, int return_flags) {
  if (return_flags & RF_coerced) {
    indent(out, indent_level) << "coerced = NULL;\n";
  }

  if (return_flags & RF_decref_args) {
    indent(out, indent_level) << "Py_DECREF(args);\n";
  }

  if (return_flags & RF_int) {
    indent(out, indent_level) << "return -1;\n";

  } else if (return_flags & RF_err_notimplemented) {
    indent(out, indent_level) << "Py_INCREF(Py_NotImplemented);\n";
    indent(out, indent_level) << "return Py_NotImplemented;\n";

  } else if (return_flags & RF_err_null) {
    indent(out, indent_level) << "return NULL;\n";

  } else if (return_flags & RF_err_false) {
    indent(out, indent_level) << "return false;\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InterfaceMakerPythonNative::pack_return_value
//       Access: Private
//  Description: Outputs a command to pack the indicated expression,
//               of the return_type type, as a Python return value.
////////////////////////////////////////////////////////////////////
void InterfaceMakerPythonNative::
pack_return_value(ostream &out, int indent_level, FunctionRemap *remap,
                  const string &return_expr) {

  ParameterRemap *return_type = remap->_return_type;
  CPPType *orig_type = return_type->get_orig_type();
  CPPType *type = return_type->get_new_type();

  if (return_type->new_type_is_atomic_string()) {
    if (TypeManager::is_char_pointer(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
      indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
      indent(out, indent_level) << "  return Py_None;\n";
      indent(out, indent_level) << "} else {\n";

      out << "#if PY_MAJOR_VERSION >= 3\n";
      indent(out, indent_level) << "  return "
        << "PyUnicode_FromString(" << return_expr << ");\n";
      out << "#else\n";
      indent(out, indent_level) << "  return "
        << "PyString_FromString(" << return_expr << ");\n";
      out << "#endif\n";

      indent(out, indent_level) << "}\n";

    } else if (TypeManager::is_wchar_pointer(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
      indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
      indent(out, indent_level) << "  return Py_None;\n";
      indent(out, indent_level) << "} else {\n";
      indent(out, indent_level+2)
        << "return PyUnicode_FromWideChar("
        << return_expr << ", wcslen(" << return_expr << "));\n";
      indent(out, indent_level) << "}\n";

    } else if (TypeManager::is_wstring(orig_type)) {
      indent(out, indent_level)
        << "return PyUnicode_FromWideChar("
        << return_expr << ".data(), (int) " << return_expr << ".length());\n";

    } else if (TypeManager::is_const_ptr_to_basic_string_wchar(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
      indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
      indent(out, indent_level) << "  return Py_None;\n";
      indent(out, indent_level) << "} else {\n";

      indent(out, indent_level) << "  return "
        << "PyUnicode_FromWideChar("
        << return_expr << "->data(), (int) " << return_expr << "->length());\n";

      indent(out, indent_level) << "}\n";

    } else if (TypeManager::is_const_ptr_to_basic_string_char(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr<< " == NULL) {\n";
      indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
      indent(out, indent_level) << "  return Py_None;\n";
      indent(out, indent_level) << "} else {\n";

      out << "#if PY_MAJOR_VERSION >= 3\n";
      indent(out, indent_level) << "  return "
        << "PyUnicode_FromStringAndSize("
        << return_expr << "->data(), (Py_ssize_t)" << return_expr << "->length());\n";
      out << "#else\n";
      indent(out, indent_level) << "  return "
        << "PyString_FromStringAndSize("
        << return_expr << "->data(), (Py_ssize_t)" << return_expr << "->length());\n";
      out << "#endif\n";

      indent(out, indent_level) << "}\n";

    } else {
      out << "#if PY_MAJOR_VERSION >= 3\n";
      indent(out, indent_level)
        << "return PyUnicode_FromStringAndSize("
        << return_expr << ".data(), (Py_ssize_t)" << return_expr << ".length());\n";
      out << "#else\n";
      indent(out, indent_level)
        << "return PyString_FromStringAndSize("
        << return_expr << ".data(), (Py_ssize_t)" << return_expr << ".length());\n";
      out << "#endif\n";
    }

  } else if (TypeManager::is_bool(type)) {
    indent(out, indent_level)
      << "return PyBool_FromLong(" << return_expr << ");\n";

  } else if (TypeManager::is_size(type)) {
    indent(out, indent_level)
      << "return PyLongOrInt_FromSize_t(" << return_expr << ");\n";

  } else if (TypeManager::is_char(type)) {
    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level)
      << "return PyUnicode_FromStringAndSize(&" << return_expr << ", 1);\n";
    out << "#else\n";
    indent(out, indent_level)
      << "return PyString_FromStringAndSize(&" << return_expr << ", 1);\n";
    out << "#endif\n";

  } else if (TypeManager::is_wchar(type)) {
    indent(out, indent_level)
      << "return PyUnicode_FromWideChar(&" << return_expr << ", 1);\n";

  } else if (TypeManager::is_unsigned_longlong(type)) {
    indent(out, indent_level)
      << "return PyLong_FromUnsignedLongLong(" << return_expr << ");\n";

  } else if (TypeManager::is_longlong(type)) {
    indent(out, indent_level)
      << "return PyLong_FromLongLong(" << return_expr << ");\n";

  } else if (TypeManager::is_unsigned_integer(type)){
    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level)
      << "return PyLong_FromUnsignedLong(" << return_expr << ");\n";
    out << "#else\n";
    indent(out, indent_level)
      << "return PyLongOrInt_FromUnsignedLong(" << return_expr << ");\n";
    out << "#endif\n";

  } else if (TypeManager::is_integer(type)) {
    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level)
      << "return PyLong_FromLong(" << return_expr << ");\n";
    out << "#else\n";
    indent(out, indent_level)
      << "return PyInt_FromLong(" << return_expr << ");\n";
    out << "#endif\n";

  } else if (TypeManager::is_float(type)) {
    indent(out, indent_level)
      << "return PyFloat_FromDouble(" << return_expr << ");\n";

  } else if (TypeManager::is_char_pointer(type)) {
    indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
    indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
    indent(out, indent_level) << "  return Py_None;\n";
    indent(out, indent_level) << "} else {\n";

    out << "#if PY_MAJOR_VERSION >= 3\n";
    indent(out, indent_level) << "  return "
      << "PyUnicode_FromString(" << return_expr << ");\n";
    out << "#else\n";
    indent(out, indent_level) << "  return "
      << "PyString_FromString(" << return_expr << ");\n";
    out << "#endif\n";

    indent(out, indent_level) << "}\n";

  } else if (TypeManager::is_wchar_pointer(type)) {
    indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
    indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
    indent(out, indent_level) << "  return Py_None;\n";
    indent(out, indent_level) << "} else {\n";
    indent(out, indent_level) << "  return "
      << "PyUnicode_FromWideChar("
      << return_expr << ", wcslen(" << return_expr << "));\n";

    indent(out, indent_level) << "}\n";

  } else if (TypeManager::is_pointer_to_PyObject(type)) {
    indent(out, indent_level)
      << "return " << return_expr << ";\n";

  } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
    indent(out, indent_level) << "if (" << return_expr << " == NULL) {\n";
    indent(out, indent_level) << "  Py_INCREF(Py_None);\n";
    indent(out, indent_level) << "  return Py_None;\n";
    indent(out, indent_level) << "} else {\n";
    indent(out, indent_level) << "  return "
      << "PyMemoryView_FromBuffer(" << return_expr << ");\n";
    indent(out, indent_level) << "}\n";

  } else if (TypeManager::is_pointer(type)) {
    bool is_const = TypeManager::is_const_pointer_to_anything(type);

    if (TypeManager::is_struct(orig_type) || TypeManager::is_ref_to_anything(orig_type)) {
      if (TypeManager::is_ref_to_anything(orig_type)) {
        TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)),false);
        InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
        const InterrogateType &itype = idb->get_type(type_index);

        bool owns_memory = remap->_return_value_needs_management;

        if (!isExportThisRun(itype._cpptype)) {
          _external_imports.insert(itype._cpptype);
        }

        write_python_instance(out, indent_level, return_expr, owns_memory, itype.get_scoped_name(), itype._cpptype, is_const);

      } else {
        bool owns_memory = remap->_return_value_needs_management;

        if (remap->_manage_reference_count) {
          TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)),false);
          InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
          const InterrogateType &itype = idb->get_type(type_index);

          if (!isExportThisRun(itype._cpptype)) {
            _external_imports.insert(itype._cpptype);
          }

          write_python_instance(out, indent_level, return_expr, owns_memory, itype.get_scoped_name(), itype._cpptype, is_const);
        } else {
          TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
          InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
          const InterrogateType &itype = idb->get_type(type_index);

          if (!isExportThisRun(itype._cpptype)) {
            _external_imports.insert(itype._cpptype);
          }

          write_python_instance(out, indent_level, return_expr, owns_memory, itype.get_scoped_name(), itype._cpptype, is_const);
        }
      }
    } else if (TypeManager::is_struct(orig_type->as_pointer_type()->_pointing_at)) {
      TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
      InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
      const InterrogateType &itype = idb->get_type(type_index);

      bool owns_memory = remap->_return_value_needs_management;

      if (!isExportThisRun(itype._cpptype)) {
        _external_imports.insert(itype._cpptype);
      }

      write_python_instance(out, indent_level, return_expr, owns_memory, itype.get_scoped_name(), itype._cpptype, is_const);

    } else {
      indent(out, indent_level) << "  Should Never Reach This InterfaceMakerPythonNative::pack_python_value";
          //<< "return PyLongOrInt_FromLong((int) " << return_expr << ");\n";
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
          Object *pobject = record_object(base_type_index);
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

    Property *property = new Property(ielement);

    if (ielement.has_setter()) {
      FunctionIndex func_index = ielement.get_setter();
      Function *setter = record_function(itype, func_index);
      if (is_function_legal(setter)) {
        property->_setter = setter;
      }
    }

    if (ielement.has_getter()) {
      FunctionIndex func_index = ielement.get_getter();
      Function *getter = record_function(itype, func_index);
      if (is_function_legal(getter)) {
        property->_getter = getter;
      }
    }

    if (property->_getter != NULL) {
      object->_properties.push_back(property);
    } else {
      // No use exporting a property without a getter.
      delete property;
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

  string name = in_ctype->get_local_name(&parser);

  if (builder.in_ignoretype(name)) {
    return false;
  }

  if (builder.in_forcetype(name)) {
    return true;
  }

  //bool answer = false;
  CPPType *type = TypeManager::resolve_type(in_ctype);
  type = TypeManager::unwrap(type);

  if (TypeManager::is_void(type)) {
    return true;
  } else if (TypeManager::is_basic_string_char(type)) {
    return true;
  } else if (TypeManager::is_basic_string_wchar(type)) {
    return true;
  } else if (TypeManager::is_simple(type)) {
    return true;
  } else if (TypeManager::is_pointer_to_simple(type)) {
    return true;
  } else if (TypeManager::is_exported(type)) {
    return true;
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
  if (builder.in_forcetype(ctype->get_local_name(&parser))) {
    return true;
  }

  if (!TypeManager::is_exported(ctype)) {
    return false;
  }

  if (TypeManager::is_local(ctype)) {
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
is_remap_legal(FunctionRemap *remap) {
  if (remap == NULL) {
    return false;
  }

  // return must be legal and managable..
  if (!is_cpp_type_legal(remap->_return_type->get_orig_type())) {
//        printf("  is_remap_legal Return Is Bad %s\n",remap->_return_type->get_orig_type()->get_fully_scoped_name().c_str());
    return false;
  }

  // We don't currently support returning pointers, but we accept
  // them as function parameters.
  if (TypeManager::is_pointer_to_simple(remap->_return_type->get_orig_type())) {
    return false;
  }

  // ouch .. bad things will happen here ..  do not even try..
  if (remap->_ForcedVoidReturn) {
    return false;
  }

  // all params must be legal
  for (int pn = 0; pn < (int)remap->_parameters.size(); pn++) {
    CPPType *orig_type = remap->_parameters[pn]._remap->get_orig_type();
    if (!is_cpp_type_legal(orig_type)) {
      return false;
    }
  }

  // ok all looks ok.
  return true;
}

//////////////////////////////////////////////
// Function : has_coerce_constructor
//            Returns 1 if coerce constructor
//            returns const, 2 if non-const.
//////////////////////////////////////////////
int InterfaceMakerPythonNative::
has_coerce_constructor(CPPStructType *type) {
  if (type == NULL) {
    return 0;
  }

  CPPScope *scope = type->get_scope();
  if (scope == NULL) {
    return 0;
  }

  int result = 0;

  CPPScope::Functions::iterator fgi;
  for (fgi = scope->_functions.begin(); fgi != scope->_functions.end(); ++fgi) {
    CPPFunctionGroup *fgroup = fgi->second;

    CPPFunctionGroup::Instances::iterator ii;
    for (ii = fgroup->_instances.begin(); ii != fgroup->_instances.end(); ++ii) {
      CPPInstance *inst = (*ii);
      CPPFunctionType *ftype = inst->_type->as_function_type();
      if (ftype == NULL) {
        continue;
      }
      if (inst->_storage_class & CPPInstance::SC_explicit) {
        // Skip it if it is marked not to allow coercion.
        continue;
      }

      if (inst->_vis > min_vis) {
        // Not published.
        continue;
      }

      CPPParameterList::Parameters &params = ftype->_parameters->_parameters;
      if (params.size() == 0) {
        // It's useless if it doesn't take any parameters.
        continue;
      }

      if (ftype->_flags & CPPFunctionType::F_constructor) {
        if (params.size() == 1 &&
            TypeManager::unwrap(params[0]->_type) == type) {
          // Skip a copy constructor.
          continue;
        } else {
          return 2;
        }

      } else if (fgroup->_name == "make" && (inst->_storage_class & CPPInstance::SC_static) != 0) {
        if (TypeManager::is_const_pointer_or_ref(ftype->_return_type)) {
          result = 1;
        } else {
          return 2;
        }
      }
    }
  }

  return result;
}

//////////////////////////////////////////////
// Function : is_remap_coercion_possible
//////////////////////////////////////////////
bool InterfaceMakerPythonNative::
is_remap_coercion_possible(FunctionRemap *remap) {
  if (remap == NULL) {
    return false;
  }

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
    } else if (TypeManager::is_pointer_to_simple(type)) {
    } else if (TypeManager::is_pointer(type)) {
      // This is a pointer to an object, so we
      // might be able to coerce a parameter to it.
      CPPType *obj_type = TypeManager::unwrap(TypeManager::resolve_type(type));
      if (has_coerce_constructor(obj_type->as_struct_type()) > 0) {
        // It has a coercion constructor, so go for it.
        return true;
      }
      break;
    }
    ++pn;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////
// Function  : is_function_legal
////////////////////////////////////////////////////////////////////////
bool InterfaceMakerPythonNative::
is_function_legal(Function *func) {
  Function::Remaps::const_iterator ri;
  for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
    FunctionRemap *remap = (*ri);
    if (is_remap_legal(remap)) {
//    printf("  Function Is Marked Legal %s\n",func->_name.c_str());

      return true;
    }
  }

//    printf("  Function Is Marked Illegal %s\n",func->_name.c_str());
  return false;
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
