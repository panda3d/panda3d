/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMakerPythonNative.cxx
 */

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
#include "cppArrayType.h"
#include "cppConstType.h"
#include "cppEnumType.h"
#include "cppFunctionType.h"
#include "cppFunctionGroup.h"
#include "cppPointerType.h"
#include "cppTypeDeclaration.h"
#include "cppTypedefType.h"
#include "cppSimpleType.h"
#include "cppStructType.h"
#include "cppExpression.h"
#include "cppParameterList.h"
#include "lineStream.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

using std::dec;
using std::hex;
using std::max;
using std::min;
using std::oct;
using std::ostream;
using std::ostringstream;
using std::set;
using std::string;

extern InterrogateType dummy_type;
extern std::string EXPORT_IMPORT_PREFIX;

#define CLASS_PREFIX "Dtool_"

// Name Remapper... Snagged from ffi py code....
struct RenameSet {
  const char *_from;
  const char *_to;
  int function_type;
};

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
  { nullptr, nullptr, -1 }
};

const char *pythonKeywords[] = {
  "and",
  "as",
  "assert",
  "async",
  "await",
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
  nullptr
};

std::string
checkKeyword(std::string &cppName) {
  for (int x = 0; pythonKeywords[x] != nullptr; x++) {
    if (cppName == pythonKeywords[x]) {
      return std::string("_") + cppName;
    }
  }
  return cppName;
}

std::string
classNameFromCppName(const std::string &cppName, bool mangle) {
  if (!mangle_names) {
    mangle = false;
  }

  // # initialize to empty string
  std::string className = "";

  // # These are the characters we want to strip out of the name
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

  if (className.empty()) {
    std::string text = "** ERROR ** Renaming class: " + cppName + " to empty string";
    printf("%s", text.c_str());
  }

  className = checkKeyword(className);
  // # FFIConstants.notify.debug('Renaming class: ' + cppName + ' to: ' +
  // className)
  return className;
}

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

  for (int x = 0; methodRenameDictionary[x]._from != nullptr; x++) {
    if (origName == methodRenameDictionary[x]._from) {
      methodName = methodRenameDictionary[x]._to;
    }
  }

  // # Mangle names that happen to be python keywords so they are not anymore
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

/**
 * Determines whether this method should be mapped to one of Python's special
 * slotted functions, those hard-coded functions that are assigned to
 * particular function pointers within the object structure, for special
 * functions like __getitem__ and __len__.
 *
 * Returns true if it has such a mapping, false if it is just a normal method.
 * If it returns true, the SlottedFunctionDef structure is filled in with the
 * important details.
 */
bool InterfaceMakerPythonNative::
get_slotted_function_def(Object *obj, Function *func, FunctionRemap *remap,
                         SlottedFunctionDef &def) {
  if (obj == nullptr) {
    // Only methods may be slotted.
    return false;
  }

  def._answer_location = string();
  def._wrapper_type = WT_none;
  def._min_version = 0;
  def._keep_method = false;

  string method_name = func->_ifunc.get_name();
  bool is_unary_op = func->_ifunc.is_unary_op();

  if (method_name == "operator +") {
    def._answer_location = "nb_add";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator -" && is_unary_op) {
    def._answer_location = "nb_negative";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "operator -") {
    def._answer_location = "nb_subtract";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator *") {
    def._answer_location = "nb_multiply";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator /") {
    def._answer_location = "nb_divide";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator %") {
    def._answer_location = "nb_remainder";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator <<") {
    def._answer_location = "nb_lshift";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator >>") {
    def._answer_location = "nb_rshift";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator ^") {
    def._answer_location = "nb_xor";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator ~" && is_unary_op) {
    def._answer_location = "nb_invert";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "operator &") {
    def._answer_location = "nb_and";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "operator |") {
    def._answer_location = "nb_or";
    def._wrapper_type = WT_binary_operator;
    return true;
  }

  if (method_name == "__pow__") {
    def._answer_location = "nb_power";
    def._wrapper_type = WT_ternary_operator;
    return true;
  }

  if (method_name == "operator +=") {
    def._answer_location = "nb_inplace_add";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator -=") {
    def._answer_location = "nb_inplace_subtract";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator *=") {
    def._answer_location = "nb_inplace_multiply";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator /=") {
    def._answer_location = "nb_inplace_divide";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator %=") {
    def._answer_location = "nb_inplace_remainder";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator <<=") {
    def._answer_location = "nb_inplace_lshift";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator >>=") {
    def._answer_location = "nb_inplace_rshift";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator &=") {
    def._answer_location = "nb_inplace_and";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator ^=") {
    def._answer_location = "nb_inplace_xor";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "operator |=") {
    def._answer_location = "nb_inplace_or";
    def._wrapper_type = WT_inplace_binary_operator;
    return true;
  }

  if (method_name == "__ipow__") {
    def._answer_location = "nb_inplace_power";
    def._wrapper_type = WT_inplace_ternary_operator;
    return true;
  }

  if (obj->_protocol_types & Object::PT_sequence) {
    if (remap->_flags & FunctionRemap::F_getitem_int) {
      def._answer_location = "sq_item";
      def._wrapper_type = WT_sequence_getitem;
      return true;
    }
    if (remap->_flags & FunctionRemap::F_setitem_int ||
        remap->_flags & FunctionRemap::F_delitem_int) {
      def._answer_location = "sq_ass_item";
      def._wrapper_type = WT_sequence_setitem;
      return true;
    }
    if (remap->_flags & FunctionRemap::F_size) {
      def._answer_location = "sq_length";
      def._wrapper_type = WT_sequence_size;
      return true;
    }
  }

  if (obj->_protocol_types & Object::PT_mapping) {
    if (remap->_flags & FunctionRemap::F_getitem) {
      def._answer_location = "mp_subscript";
      def._wrapper_type = WT_one_param;
      return true;
    }
    if (remap->_flags & FunctionRemap::F_setitem ||
        remap->_flags & FunctionRemap::F_delitem) {
      def._answer_location = "mp_ass_subscript";
      def._wrapper_type = WT_mapping_setitem;
      return true;
    }
    if (remap->_flags & FunctionRemap::F_size) {
      def._answer_location = "mp_length";
      def._wrapper_type = WT_sequence_size;
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

  if (method_name == "__await__") {
    def._answer_location = "am_await";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "__aiter__") {
    def._answer_location = "am_aiter";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "__anext__") {
    def._answer_location = "am_anext";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "operator ()") {
    def._answer_location = "tp_call";
    def._wrapper_type = WT_none;
    return true;
  }

  if (method_name == "__getattribute__") {
    // Like __getattr__, but is called unconditionally, ie.  does not try
    // PyObject_GenericGetAttr first.
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
    // __delattr__ shares the slot with __setattr__, except that it takes only
    // one argument.
    def._answer_location = "tp_setattro";
    def._wrapper_type = WT_setattr;
    return true;
  }

  if (method_name == "__nonzero__" || method_name == "__bool__") {
    // Python 2 named it nb_nonzero, Python 3 nb_bool.  We refer to it just as
    // nb_bool.
    def._answer_location = "nb_bool";
    def._wrapper_type = WT_inquiry;
    return true;
  }

  if (method_name == "__getbuffer__") {
    def._answer_location = "bf_getbuffer";
    def._wrapper_type = WT_getbuffer;
    return true;
  }

  if (method_name == "__releasebuffer__") {
    def._answer_location = "bf_releasebuffer";
    def._wrapper_type = WT_releasebuffer;
    return true;
  }

  if (method_name == "__traverse__") {
    def._answer_location = "tp_traverse";
    def._wrapper_type = WT_traverse;
    return true;
  }

  if (method_name == "__clear__") {
    def._answer_location = "tp_clear";
    def._wrapper_type = WT_inquiry;
    return true;
  }

  if (method_name == "__repr__") {
    def._answer_location = "tp_repr";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "__str__") {
    def._answer_location = "tp_str";
    def._wrapper_type = WT_no_params;
    return true;
  }

  if (method_name == "__cmp__" || (remap->_flags & FunctionRemap::F_compare_to) != 0) {
    def._answer_location = "tp_compare";
    def._wrapper_type = WT_compare;
    def._keep_method = (method_name != "__cmp__");
    return true;
  }

  if (method_name == "__hash__" || (remap->_flags & FunctionRemap::F_hash) != 0) {
    def._answer_location = "tp_hash";
    def._wrapper_type = WT_hash;
    def._keep_method = (method_name != "__hash__");
    return true;
  }

  if (remap->_type == FunctionRemap::T_typecast_method) {
    // A typecast operator.  Check for a supported low-level typecast type.
    if (TypeManager::is_bool(remap->_return_type->get_orig_type())) {
      // If it's a bool type, then we wrap it with the __nonzero__ slot
      // method.
      def._answer_location = "nb_bool";
      def._wrapper_type = WT_inquiry;
      return true;

    } else if (TypeManager::is_integer(remap->_return_type->get_orig_type())) {
      // An integer type.
      def._answer_location = "nb_int";
      def._wrapper_type = WT_no_params;
      return true;

    } else if (TypeManager::is_float(remap->_return_type->get_orig_type())) {
      // A floating-point (or double) type.
      def._answer_location = "nb_float";
      def._wrapper_type = WT_no_params;
      return true;

    } else if (remap->_return_type->new_type_is_atomic_string()) {
      // A string type.
      def._answer_location = "tp_str";
      def._wrapper_type = WT_no_params;
      return true;
    }
  }

  return false;
}

/**
 * Determines whether the slot occurs in the map of slotted functions, and if
 * so, writes out a pointer to its wrapper.  If not, writes out def (usually
 * 0).
 */
void InterfaceMakerPythonNative::
write_function_slot(ostream &out, int indent_level, const SlottedFunctions &slots,
                    const string &slot, const string &default_) {

  SlottedFunctions::const_iterator rfi = slots.find(slot);
  if (rfi == slots.end()) {
    indent(out, indent_level) << default_ << ",";
    if (default_ == "0") {
      out << " // " << slot;
    }
    out << "\n";
    return;
  }

  const SlottedFunctionDef &def = rfi->second;

  // Add an #ifdef if there is a specific version requirement on this
  // function.
  if (def._min_version > 0) {
    out << "#if PY_VERSION_HEX >= 0x" << hex << def._min_version << dec << "\n";
  }

  indent(out, indent_level) << "&" << def._wrapper_name << ",\n";

  if (def._min_version > 0) {
    out << "#else\n";
    indent(out, indent_level) << default_ << ",\n";
    out << "#endif\n";
  }
}

void InterfaceMakerPythonNative::
get_valid_child_classes(std::map<std::string, CastDetails> &answer, CPPStructType *inclass, const std::string &upcast_seed, bool can_downcast) {
  if (inclass == nullptr) {
    return;
  }

  for (const CPPStructType::Base &base : inclass->_derivation) {
// if (base._vis <= V_public) can_downcast = false;
    CPPStructType *base_type = TypeManager::resolve_type(base._base)->as_struct_type();
    if (base_type != nullptr) {
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

/**

 */
void InterfaceMakerPythonNative::
write_python_instance(ostream &out, int indent_level, const string &return_expr,
                      bool owns_memory, const InterrogateType &itype, bool is_const) {
  out << std::boolalpha;

  if (!isExportThisRun(itype._cpptype)) {
    _external_imports.insert(TypeManager::resolve_type(itype._cpptype));
  }

  string class_name = itype.get_scoped_name();

  // We don't handle final classes via DTool_CreatePyInstanceTyped since we
  // know it can't be of a subclass type, so we don't need to do the downcast.
  CPPStructType *struct_type = itype._cpptype->as_struct_type();
  if (IsPandaTypedObject(struct_type) && !struct_type->is_final()) {
    // We can't let DTool_CreatePyInstanceTyped do the NULL check since we
    // will be grabbing the type index (which would obviously crash when
    // called on a NULL pointer), so we do it here.
    indent(out, indent_level)
      << "if (" << return_expr << " == nullptr) {\n";
    indent(out, indent_level)
      << "  Py_INCREF(Py_None);\n";
    indent(out, indent_level)
      << "  return Py_None;\n";
    indent(out, indent_level)
      << "} else {\n";
    // Special exception if we are returning TypedWritable, which might
    // actually be a derived class that inherits from ReferenceCount.
    if (!owns_memory && !is_const && class_name == "TypedWritable") {
      indent(out, indent_level)
        << "  ReferenceCount *rc = " << return_expr << "->as_reference_count();\n";
      indent(out, indent_level)
        << "  bool is_refcount = (rc != nullptr);\n";
      indent(out, indent_level)
        << "  if (is_refcount) {\n";
      indent(out, indent_level)
        << "    rc->ref();\n";
      indent(out, indent_level)
        << "  }\n";
      indent(out, indent_level)
        << "  return DTool_CreatePyInstanceTyped((void *)" << return_expr
        << ", *Dtool_Ptr_" << make_safe_name(class_name) << ", is_refcount, "
        << is_const << ", " << return_expr
        << "->get_type_index());\n";
    } else {
      indent(out, indent_level)
        << "  return DTool_CreatePyInstanceTyped((void *)" << return_expr
        << ", *Dtool_Ptr_" << make_safe_name(class_name) << ", "
        << owns_memory << ", " << is_const << ", "
        << return_expr << "->as_typed_object()->get_type_index());\n";
    }
    indent(out, indent_level)
      << "}\n";
  } else {
    // DTool_CreatePyInstance will do the NULL check.
    indent(out, indent_level)
      << "return "
      << "DTool_CreatePyInstance((void *)" << return_expr << ", "
      << "*Dtool_Ptr_" << make_safe_name(class_name) << ", "
      << owns_memory << ", " << is_const << ");\n";
  }
}

/**
 *
 */
InterfaceMakerPythonNative::
InterfaceMakerPythonNative(InterrogateModuleDef *def) :
  InterfaceMakerPython(def)
{
}

/**
 *
 */
InterfaceMakerPythonNative::
~InterfaceMakerPythonNative() {
}

/**
 * Generates the list of function prototypes corresponding to the functions
 * that will be output in write_functions().
 */
void InterfaceMakerPythonNative::
write_prototypes(ostream &out_code, ostream *out_h) {
  if (out_h != nullptr) {
    *out_h << "#include \"py_panda.h\"\n\n";
  }

  /*
  for (Function *func : _functions) {
    if (!func->_itype.is_global() && is_function_legal(func)) {
      write_prototype_for(out_code, func);
    }
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
          // write_prototypes_class_external(out_code, object);
          // _external_imports.insert(object->_itype._cpptype);
        }
      }
    } else if (object->_itype.is_scoped_enum() && isExportThisRun(object->_itype._cpptype)) {
      // Forward declare where we will put the scoped enum type.
      string class_name = object->_itype._cpptype->get_local_name(&parser);
      string safe_name = make_safe_name(class_name);
      out_code << "static PyTypeObject *Dtool_Ptr_" << safe_name << " = nullptr;\n";
    }
  }

  out_code << "/**\n";
  out_code << " * Declarations for exported classes\n";
  out_code << " */\n";

  out_code << "static const Dtool_TypeDef exports[] = {\n";

  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;

    if (object->_itype.is_class() || object->_itype.is_struct()) {
      CPPType *type = object->_itype._cpptype;

      if (isExportThisRun(type) && is_cpp_type_legal(type)) {
        string class_name = type->get_local_name(&parser);
        string safe_name = make_safe_name(class_name);

        out_code << "  {\"" << class_name << "\", &Dtool_" << safe_name << "},\n";
      }
    }
  }

  out_code << "  {nullptr, nullptr},\n";
  out_code << "};\n\n";

  out_code << "/**\n";
  out_code << " * Extern declarations for imported classes\n";
  out_code << " */\n";

  // Write out a table of the externally imported types that will be filled in
  // upon module initialization.
  if (!_external_imports.empty()) {
    out_code << "#ifndef LINK_ALL_STATIC\n";
    out_code << "static Dtool_TypeDef imports[] = {\n";

    int idx = 0;
    for (CPPType *type : _external_imports) {
      string class_name = type->get_local_name(&parser);
      string safe_name = make_safe_name(class_name);

      out_code << "  {\"" << class_name << "\", nullptr},\n";
      out_code << "#define Dtool_Ptr_" << safe_name << " (imports[" << idx << "].type)\n";
      ++idx;
    }
    out_code << "  {nullptr, nullptr},\n";
    out_code << "};\n";
    out_code << "#endif\n\n";
  }

  for (CPPType *type : _external_imports) {
    string class_name = type->get_local_name(&parser);
    string safe_name = make_safe_name(class_name);

    out_code << "// " << class_name << "\n";

    out_code << "#ifndef LINK_ALL_STATIC\n";
    // out_code << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" <<
    // safe_name << ";\n";
    //if (has_get_class_type_function(type)) {
    //  out_code << "static struct Dtool_PyTypedObject *Dtool_Ptr_" << safe_name << ";\n";
    //}
    // out_code << "#define Dtool_Ptr_" << safe_name << " &Dtool_" <<
    // safe_name << "\n"; out_code << "IMPORT_THIS void
    // Dtool_PyModuleClassInit_" << safe_name << "(PyObject *module);\n";

    // This is some really ugly code, because we have to store a pointer with
    // a function of a signature that differs from class to class.  If someone
    // can think of an elegant way to do this without sacrificing perf, let me
    // know.
    int has_coerce = has_coerce_constructor(type->as_struct_type());
    if (has_coerce > 0) {
      if (TypeManager::is_reference_count(type)) {
        out_code
          << "inline static bool Dtool_ConstCoerce_" << safe_name << "(PyObject *args, CPT(" << class_name << ") &coerced) {\n"
          << "  nassertr(Dtool_Ptr_" << safe_name << " != nullptr, false);\n"
          << "  nassertr(Dtool_Ptr_" << safe_name << "->_Dtool_ConstCoerce != nullptr, false);\n"
          << "  return ((bool (*)(PyObject *, CPT(" << class_name << ") &))Dtool_Ptr_" << safe_name << "->_Dtool_ConstCoerce)(args, coerced);\n"
          << "}\n";

        if (has_coerce > 1) {
          out_code
            << "inline static bool Dtool_Coerce_" << safe_name << "(PyObject *args, PT(" << class_name << ") &coerced) {\n"
            << "  nassertr(Dtool_Ptr_" << safe_name << " != nullptr, false);\n"
            << "  nassertr(Dtool_Ptr_" << safe_name << "->_Dtool_Coerce != nullptr, false);\n"
            << "  return ((bool (*)(PyObject *, PT(" << class_name << ") &))Dtool_Ptr_" << safe_name << "->_Dtool_Coerce)(args, coerced);\n"
            << "}\n";
        }
      } else {
        out_code
          << "inline static " << class_name << " *Dtool_Coerce_" << safe_name << "(PyObject *args, " << class_name << " &coerced) {\n"
          << "  nassertr(Dtool_Ptr_" << safe_name << " != nullptr, nullptr);\n"
          << "  nassertr(Dtool_Ptr_" << safe_name << "->_Dtool_Coerce != nullptr, nullptr);\n"
          << "  return ((" << class_name << " *(*)(PyObject *, " << class_name << " &))Dtool_Ptr_" << safe_name << "->_Dtool_Coerce)(args, coerced);\n"
          << "}\n";
      }
    }
    out_code << "#else\n";
    out_code << "extern struct Dtool_PyTypedObject Dtool_" << safe_name << ";\n";
    out_code << "static struct Dtool_PyTypedObject *const Dtool_Ptr_" << safe_name << " = &Dtool_" << safe_name << ";\n";

    if (has_coerce > 0) {
      if (TypeManager::is_reference_count(type)) {
        out_code << "extern bool Dtool_ConstCoerce_" << safe_name << "(PyObject *args, CPT(" << class_name << ") &coerced);\n";
        if (has_coerce > 1) {
          out_code << "extern bool Dtool_Coerce_" << safe_name << "(PyObject *args, PT(" << class_name << ") &coerced);\n";
        }
      } else {
        out_code << "extern " << class_name << " *Dtool_Coerce_" << safe_name << "(PyObject *args, " << class_name << " &coerced);\n";
      }
    }
    out_code << "#endif\n";
  }
}

/**
 * Output enough enformation to a declartion of a externally generated dtool
 * type object
 */
void InterfaceMakerPythonNative::
write_prototypes_class_external(ostream &out, Object *obj) {
  std::string class_name = make_safe_name(obj->_itype.get_scoped_name());
  std::string c_class_name =  obj->_itype.get_true_name();
  std::string preferred_name =  obj->_itype.get_name();


  out << "/**\n";
  out << " * Forward declaration of class " << class_name << "\n";
  out << " */\n";

  // This typedef is necessary for class templates since we can't pass a comma
  // to a macro function.
  out << "typedef  " << c_class_name << "  " << class_name << "_localtype;\n";
  out << "Define_Module_Class_Forward(" << _def->module_name << ", " << class_name << ", " << class_name << "_localtype, " << classNameFromCppName(preferred_name, false) << ");\n";
}

/**

 */
void InterfaceMakerPythonNative::
write_prototypes_class(ostream &out_code, ostream *out_h, Object *obj) {
  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());

  out_code << "/**\n";
  out_code << " * Forward declarations for top-level class " << ClassName << "\n";
  out_code << " */\n";

  /*
  for (Function *func : obj->_methods) {
    write_prototype_for(out_code, func);
  }
  */

  /*
  for (Function *func : obj->_constructors) {
    std::string fname = "int Dtool_Init_" + ClassName + "(PyObject *self, PyObject *args, PyObject *kwds)";
    write_prototype_for_name(out_code, obj, func, fname);
  }
  */

  write_class_declarations(out_code, out_h, obj);
}

/**
 * Generates the list of functions that are appropriate for this interface.
 * This function is called *before* write_prototypes(), above.
 */
void InterfaceMakerPythonNative::
write_functions(ostream &out) {
  out << "/**\n";
  out << " * Python wrappers for global functions\n" ;
  out << " */\n";
  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi).second;
    if (!func->_itype.is_global() && is_function_legal(func)) {
      write_function_for_top(out, nullptr, func);
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

  // Objects::iterator oi;
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
}

/**
 * Writes out all of the wrapper methods necessary to export the given object.
 * This is called by write_functions.
 */
void InterfaceMakerPythonNative::
write_class_details(ostream &out, Object *obj) {
  // std::string cClassName = obj->_itype.get_scoped_name();
  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName = obj->_itype.get_true_name();

  out << "/**\n";
  out << " * Python wrappers for functions of class " << cClassName << "\n" ;
  out << " */\n";

  // First write out all the wrapper functions for the methods.
  for (Function *func : obj->_methods) {
    if (func) {
      // Write the definition of the generic wrapper function for this
      // function.
      write_function_for_top(out, obj, func);
    }
  }

  // Now write out generated getters and setters for the properties.
  for (Property *property : obj->_properties) {
    write_getset(out, obj, property);
  }

  // Write the constructors.
  std::string fname = "static int Dtool_Init_" + ClassName + "(PyObject *self, PyObject *args, PyObject *kwds)";
  for (Function *func : obj->_constructors) {
    string expected_params;
    write_function_for_name(out, obj, func->_remaps, fname, expected_params, true, AT_keyword_args, RF_int);
  }
  if (obj->_constructors.size() == 0) {
    // We still need to write a dummy constructor to prevent inheriting the
    // constructor from a base class.
    out << fname << " {\n"
      "  Dtool_Raise_TypeError(\"cannot init abstract class\");\n"
      "  return -1;\n"
      "}\n\n";
  }

  CPPType *cpptype = TypeManager::resolve_type(obj->_itype._cpptype);

  // If we have "coercion constructors", write a single wrapper to consolidate
  // those.
  int has_coerce = has_coerce_constructor(cpptype->as_struct_type());
  if (has_coerce > 0) {
    write_coerce_constructor(out, obj, true);
    if (has_coerce > 1 && TypeManager::is_reference_count(obj->_itype._cpptype)) {
      write_coerce_constructor(out, obj, false);
    }
  }

  // Write make seqs: generated methods that return a sequence of items.
  for (MakeSeq *make_seq : obj->_make_seqs) {
    if (is_function_legal(make_seq->_length_getter) &&
        is_function_legal(make_seq->_element_getter)) {
      write_make_seq(out, obj, ClassName, cClassName, make_seq);
    } else {
      if (!is_function_legal(make_seq->_length_getter)) {
        std::cerr << "illegal length function for MAKE_SEQ: " << make_seq->_length_getter->_name << "\n";
      }
      if (!is_function_legal(make_seq->_element_getter)) {
        std::cerr << "illegal element function for MAKE_SEQ: " << make_seq->_element_getter->_name << "\n";
      }
    }
  }

  // Determine which external imports we will need.
  std::map<string, CastDetails> details;
  std::map<string, CastDetails>::iterator di;
  builder.get_type(TypeManager::unwrap(cpptype), false);
  get_valid_child_classes(details, cpptype->as_struct_type());
  for (di = details.begin(); di != details.end(); di++) {
    // InterrogateType ptype =idb->get_type(di->first);
    if (di->second._is_legal_py_class && !isExportThisRun(di->second._structType)) {
      _external_imports.insert(TypeManager::resolve_type(di->second._structType));
    }
    // out << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" <<
    // make_safe_name(di->second._to_class_name) << ";\n";
  }

  // Write support methods to cast from and to pointers of this type.
  {
    out << "static void *Dtool_UpcastInterface_" << ClassName << "(PyObject *self, Dtool_PyTypedObject *requested_type) {\n";
    out << "  Dtool_PyTypedObject *type = DtoolInstance_TYPE(self);\n";
    out << "  if (type != &Dtool_" << ClassName << ") {\n";
    out << "    printf(\"" << ClassName << " ** Bad Source Type-- Requesting Conversion from %s to %s\\n\", Py_TYPE(self)->tp_name, requested_type->_PyType.tp_name); fflush(nullptr);\n";;
    out << "    return nullptr;\n";
    out << "  }\n";
    out << "\n";
    out << "  " << cClassName << " *local_this = (" << cClassName << " *)DtoolInstance_VOID_PTR(self);\n";
    out << "  if (requested_type == &Dtool_" << ClassName << ") {\n";
    out << "    return local_this;\n";
    out << "  }\n";

    for (di = details.begin(); di != details.end(); di++) {
      if (di->second._is_legal_py_class) {
        out << "  if (requested_type == Dtool_Ptr_" << make_safe_name(di->second._to_class_name) << ") {\n";
        out << "    return " << di->second._up_cast_string << " local_this;\n";
        out << "  }\n";
      }
    }

    // Are there any implicit cast operators that can cast this object to our
    // desired pointer?
    for (Function *func : obj->_methods) {
      for (FunctionRemap *remap : func->_remaps) {
        if (remap->_type == FunctionRemap::T_typecast_method &&
            is_remap_legal(remap) &&
            !remap->_return_type->return_value_needs_management() &&
            (remap->_cppfunc->_storage_class & CPPInstance::SC_explicit) == 0 &&
            TypeManager::is_pointer(remap->_return_type->get_new_type())) {

          CPPType *cast_type = remap->_return_type->get_orig_type();
          CPPType *obj_type = TypeManager::unwrap(TypeManager::resolve_type(remap->_return_type->get_new_type()));
          string return_expr = "(" + cast_type->get_local_name(&parser) + ")*local_this";
          out << "  // " << *remap->_cppfunc << "\n";
          out << "  if (requested_type == Dtool_Ptr_" << make_safe_name(obj_type->get_local_name(&parser)) << ") {\n";
          out << "    return (void *)(" << remap->_return_type->get_return_expr(return_expr) << ");\n";
          out << "  }\n";
        }
      }
    }

    out << "  return nullptr;\n";
    out << "}\n\n";

    out << "static void *Dtool_DowncastInterface_" << ClassName << "(void *from_this, Dtool_PyTypedObject *from_type) {\n";
    out << "  if (from_this == nullptr || from_type == nullptr) {\n";
    out << "    return nullptr;\n";
    out << "  }\n";
    out << "  if (from_type == Dtool_Ptr_" << ClassName << ") {\n";
    out << "    return from_this;\n";
    out << "  }\n";
    for (di = details.begin(); di != details.end(); di++) {
      if (di->second._can_downcast && di->second._is_legal_py_class) {
        out << "  if (from_type == Dtool_Ptr_" << make_safe_name(di->second._to_class_name) << ") {\n";
        out << "    " << di->second._to_class_name << "* other_this = (" << di->second._to_class_name << "*)from_this;\n" ;
        out << "    return (" << cClassName << "*)other_this;\n";
        out << "  }\n";
      }
    }
    out << "  return nullptr;\n";
    out << "}\n\n";
  }
}

/**

 */
void InterfaceMakerPythonNative::
write_class_declarations(ostream &out, ostream *out_h, Object *obj) {
  const InterrogateType &itype = obj->_itype;
  std::string class_name = make_safe_name(obj->_itype.get_scoped_name());
  std::string c_class_name =  obj->_itype.get_true_name();
  std::string preferred_name =  itype.get_name();
  std::string class_struct_name = std::string(CLASS_PREFIX) + class_name;

  CPPType *type = obj->_itype._cpptype;

  // This typedef is necessary for class templates since we can't pass a comma
  // to a macro function.
  out << "typedef " << c_class_name << " " << class_name << "_localtype;\n";
  if (obj->_itype.has_destructor() ||
      obj->_itype.destructor_is_inherited() ||
      obj->_itype.destructor_is_implicit()) {

    if (TypeManager::is_reference_count(type)) {
      out << "Define_Module_ClassRef";
    } else {
      out << "Define_Module_Class";
    }
  } else {
    if (TypeManager::is_reference_count(type)) {
      out << "Define_Module_ClassRef_Private";
    } else {
      out << "Define_Module_Class_Private";
    }
  }
  out << "(" << _def->module_name << ", " << class_name << ", " << class_name << "_localtype, " << classNameFromCppName(preferred_name, false) << ");\n";

  out << "static struct Dtool_PyTypedObject *const Dtool_Ptr_" << class_name << " = &Dtool_" << class_name << ";\n";
  out << "static void Dtool_PyModuleClassInit_" << class_name << "(PyObject *module);\n";

  int has_coerce = has_coerce_constructor(type->as_struct_type());
  if (has_coerce > 0) {
    if (TypeManager::is_reference_count(type)) {
      out << "bool Dtool_ConstCoerce_" << class_name << "(PyObject *args, CPT(" << c_class_name << ") &coerced);\n";
      if (has_coerce > 1) {
        out << "bool Dtool_Coerce_" << class_name << "(PyObject *args, PT(" << c_class_name << ") &coerced);\n";
      }
    } else {
      out << "" << c_class_name << " *Dtool_Coerce_" << class_name << "(PyObject *args, " << c_class_name << " &coerced);\n";
    }
  }

  out << "\n";

  if (out_h != nullptr) {
    *out_h << "extern \"C\" " << EXPORT_IMPORT_PREFIX << " struct Dtool_PyTypedObject Dtool_" << class_name << ";\n";
  }
}

/**
 * Generates whatever additional code is required to support a module file.
 */
void InterfaceMakerPythonNative::
write_sub_module(ostream &out, Object *obj) {
  // Object * obj = _objects[_embeded_index] ;
  string class_name = make_safe_name(obj->_itype.get_scoped_name());
  string class_ptr;

  if (!obj->_itype.is_typedef()) {
    out << "  // " << *(obj->_itype._cpptype) << "\n";
    out << "  Dtool_PyModuleClassInit_" << class_name << "(module);\n";
    class_ptr = "&Dtool_" + class_name;

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
      _external_imports.insert(TypeManager::resolve_type(wrapped_itype._cpptype));

      class_ptr = "Dtool_Ptr_" + class_name;
      out << "  assert(" << class_ptr << " != nullptr);\n";
    } else {
      class_ptr = "&Dtool_" + class_name;

      // If this is a typedef to a class defined in the same module, make sure
      // that the class is initialized before we try to define the typedef.
      out << "  Dtool_PyModuleClassInit_" << class_name << "(module);\n";
    }
  }

  std::string export_class_name = classNameFromCppName(obj->_itype.get_name(), false);
  std::string export_class_name2 = classNameFromCppName(obj->_itype.get_name(), true);

  class_ptr = "(PyObject *)" + class_ptr;

  // Note: PyModule_AddObject steals a reference, so we have to call Py_INCREF
  // for every but the first time we add it to the module.
  if (obj->_itype.is_typedef()) {
    out << "  Py_INCREF(" << class_ptr << ");\n";
  }

  out << "  PyModule_AddObject(module, \"" << export_class_name << "\", " << class_ptr << ");\n";
  if (export_class_name != export_class_name2) {
    out << "  Py_INCREF(Dtool_Ptr_" << class_name << ");\n";
    out << "  PyModule_AddObject(module, \"" << export_class_name2 << "\", " << class_ptr << ");\n";
  }
}

/**

 */
void InterfaceMakerPythonNative::
write_module_support(ostream &out, ostream *out_h, InterrogateModuleDef *def) {
  out << "/**\n";
  out << " * Module Object Linker ..\n";
  out << " */\n";

  Objects::iterator oi;

  out << "void Dtool_" << def->library_name << "_RegisterTypes() {\n"
         "  TypeRegistry *registry = TypeRegistry::ptr();\n"
         "  nassertv(registry != nullptr);\n";

  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (object->_itype.is_class() || object->_itype.is_struct()) {
      CPPType *type = object->_itype._cpptype;
      if (is_cpp_type_legal(type) && isExportThisRun(type)) {
        string class_name = object->_itype.get_scoped_name();
        string safe_name = make_safe_name(class_name);
        bool is_typed = has_get_class_type_function(type);

        if (is_typed) {
          out << "  {\n";
          if (has_init_type_function(type)) {
            // Call the init_type function.  This isn't necessary for all
            // types as many of them are automatically initialized at static
            // init type, but for some extension classes it's useful.
            out << "    " << type->get_local_name(&parser)
                << "::init_type();\n";
          }
          out << "    TypeHandle handle = " << type->get_local_name(&parser)
              << "::get_class_type();\n";
          out << "    Dtool_" << safe_name << "._type = handle;\n";
          out << "    registry->record_python_type(handle, "
                 "(PyObject *)&Dtool_" << safe_name << ");\n";
          out << "  }\n";
        } else {
          if (IsPandaTypedObject(type->as_struct_type())) {
            nout << object->_itype.get_scoped_name() << " derives from TypedObject, "
                 << "but does not define a get_class_type() function.\n";
          }
        }
      }
    }
  }
  out << "}\n\n";

  out << "void Dtool_" << def->library_name << "_BuildInstants(PyObject *module) {\n";
  out << "  (void) module;\n";

  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    if (object->_itype.is_enum() && !object->_itype.is_nested() &&
        isExportThisRun(object->_itype._cpptype)) {
      int enum_count = object->_itype.number_of_enum_values();

      if (object->_itype.is_scoped_enum()) {
        // Convert as Python 3.4-style enum.
        string class_name = object->_itype._cpptype->get_local_name(&parser);
        string safe_name = make_safe_name(class_name);

        CPPType *underlying_type = TypeManager::unwrap_const(object->_itype._cpptype->as_enum_type()->get_underlying_type());
        string cast_to = underlying_type->get_local_name(&parser);
        out << "  // enum class " << object->_itype.get_scoped_name() << "\n";
        out << "  {\n";
        out << "    PyObject *members = PyTuple_New(" << enum_count << ");\n";
        out << "    PyObject *member;\n";
        for (int xx = 0; xx < enum_count; xx++) {
          out << "    member = PyTuple_New(2);\n"
                 "#if PY_MAJOR_VERSION >= 3\n"
                 "      PyTuple_SET_ITEM(member, 0, PyUnicode_FromString(\""
              << object->_itype.get_enum_value_name(xx) << "\"));\n"
                 "#else\n"
                 "      PyTuple_SET_ITEM(member, 0, PyString_FromString(\""
              << object->_itype.get_enum_value_name(xx) << "\"));\n"
                 "#endif\n"
                 "    PyTuple_SET_ITEM(member, 1, Dtool_WrapValue(("
              << cast_to << ")" << object->_itype.get_scoped_name() << "::"
              << object->_itype.get_enum_value_name(xx) << "));\n"
                 "    PyTuple_SET_ITEM(members, " << xx << ", member);\n";
        }
        out << "    Dtool_Ptr_" << safe_name << " = Dtool_EnumType_Create(\""
            << object->_itype.get_name() << "\", members, \""
            << _def->module_name << "\");\n";
        out << "    PyModule_AddObject(module, \"" << object->_itype.get_name()
            << "\", (PyObject *)Dtool_Ptr_" << safe_name << ");\n";
        out << "  }\n";
      } else {
        out << "  // enum " << object->_itype.get_scoped_name() << "\n";
        for (int xx = 0; xx < enum_count; xx++) {
          string name1 = classNameFromCppName(object->_itype.get_enum_value_name(xx), false);
          string name2 = classNameFromCppName(object->_itype.get_enum_value_name(xx), true);
          string enum_value = "::" + object->_itype.get_enum_value_name(xx);
          out << "  PyModule_AddObject(module, \"" << name1 << "\", Dtool_WrapValue(" << enum_value << "));\n";
          if (name1 != name2) {
            // Also write the mangled name, for historical purposes.
            out << "  PyModule_AddObject(module, \"" << name2 << "\", Dtool_WrapValue(" << enum_value << "));\n";
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
      string fptr = "&" + func->_name;
      switch (func->_args_type) {
      case AT_keyword_args:
        flags = "METH_VARARGS | METH_KEYWORDS";
        fptr = "(PyCFunction) " + fptr;
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

      // Note: we shouldn't add METH_STATIC here, since both METH_STATIC and
      // METH_CLASS are illegal for module-level functions.

      out << "  {\"" << name1 << "\", " << fptr
          << ", " << flags << ", (const char *)" << func->_name << "_comment},\n";
      if (name1 != name2) {
        out << "  {\"" << name2 << "\", " << fptr
            << ", " << flags << ", (const char *)" << func->_name << "_comment},\n";
      }
    }
  }

  if (force_base_functions) {
    out << "  // Support Function For Dtool_types ... for now in each module ??\n";
    out << "  {\"Dtool_BorrowThisReference\", &Dtool_BorrowThisReference, METH_VARARGS, \"Used to borrow 'this' pointer (to, from)\\nAssumes no ownership.\"},\n";
    //out << "  {\"Dtool_AddToDictionary\", &Dtool_AddToDictionary, METH_VARARGS, \"Used to add items into a tp_dict\"},\n";
  }

  out << "  {nullptr, nullptr, 0, nullptr}\n" << "};\n\n";

  if (_external_imports.empty()) {
    out << "extern const struct LibraryDef " << def->library_name << "_moddef = {python_simple_funcs, exports, nullptr};\n";
  } else {
    out <<
      "#ifdef LINK_ALL_STATIC\n"
      "extern const struct LibraryDef " << def->library_name << "_moddef = {python_simple_funcs, exports, nullptr};\n"
      "#else\n"
      "extern const struct LibraryDef " << def->library_name << "_moddef = {python_simple_funcs, exports, imports};\n"
      "#endif\n";
  }
  if (out_h != nullptr) {
    *out_h << "extern const struct LibraryDef " << def->library_name << "_moddef;\n";
  }
}

/**

 */
void InterfaceMakerPythonNative::
write_module(ostream &out, ostream *out_h, InterrogateModuleDef *def) {
  InterfaceMakerPython::write_module(out, out_h, def);
  Objects::iterator oi;

  out << "/**\n";
  out << " * Module initialization functions for Python module \"" << def->module_name << "\"\n";
  out << " */\n";

  out << "#if PY_MAJOR_VERSION >= 3\n"
      << "static struct PyModuleDef python_native_module = {\n"
      << "  PyModuleDef_HEAD_INIT,\n"
      << "  \"" << def->module_name << "\",\n"
      << "  nullptr,\n"
      << "  -1,\n"
      << "  nullptr,\n"
      << "  nullptr, nullptr, nullptr, nullptr\n"
      << "};\n"
      << "\n"
      << "extern \"C\" EXPORT_CLASS PyObject *PyInit_" << def->module_name << "();\n"
      << "\n"
      << "PyObject *PyInit_" << def->module_name << "() {\n"
      << "  LibraryDef *refs[] = {&" << def->library_name << "_moddef, nullptr};\n"
      << "  PyObject *module = Dtool_PyModuleInitHelper(refs, &python_native_module);\n"
      << "  Dtool_" << def->library_name << "_BuildInstants(module);\n"
      << "  return module;\n"
      << "}\n"
      << "\n"
      << "#else  // Python 2 case\n"
      << "\n"
      << "extern \"C\" EXPORT_CLASS void init" << def->module_name << "();\n"
      << "\n"
      << "void init" << def->module_name << "() {\n"
      << "  LibraryDef *refs[] = {&" << def->library_name << "_moddef, nullptr};\n"
      << "  PyObject *module = Dtool_PyModuleInitHelper(refs, \"" << def->module_name << "\");\n"
      << "  Dtool_" << def->library_name << "_BuildInstants(module);\n"
      << "}\n"
      << "\n"
      << "#endif\n"
      << "\n";
}
/**

 */
void InterfaceMakerPythonNative::
write_module_class(ostream &out, Object *obj) {
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
      assert(nested_obj != nullptr);

      if (nested_obj->_itype.is_class() || nested_obj->_itype.is_struct()) {
        write_module_class(out, nested_obj);
      }
    }
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName =  obj->_itype.get_true_name();
  std::string export_class_name = classNameFromCppName(obj->_itype.get_name(), false);

  bool is_runtime_typed = IsPandaTypedObject(obj->_itype._cpptype->as_struct_type());
  if (!is_runtime_typed && has_get_class_type_function(obj->_itype._cpptype)) {
    is_runtime_typed = true;
  }

  out << "/**\n";
  out << " * Python method tables for " << ClassName << " (" << export_class_name << ")\n" ;
  out << " */\n";
  out << "static PyMethodDef Dtool_Methods_" << ClassName << "[] = {\n";

  SlottedFunctions slots;
  // function Table
  bool got_copy = false;
  bool got_deepcopy = false;

  for (Function *func : obj->_methods) {
    if (func->_name == "__copy__") {
      got_copy = true;
    } else if (func->_name == "__deepcopy__") {
      got_deepcopy = true;
    }

    string name1 = methodNameFromCppName(func, export_class_name, false);
    string name2 = methodNameFromCppName(func, export_class_name, true);

    string flags;
    string fptr = "&" + func->_name;
    switch (func->_args_type) {
    case AT_keyword_args:
      flags = "METH_VARARGS | METH_KEYWORDS";
      fptr = "(PyCFunction) " + fptr;
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

      // Skip adding this entry if we also have a property with the same name.
      // In that case, we will use a Dtool_StaticProperty to disambiguate
      // access to this method.  See GitHub issue #444.
      for (const Property *property : obj->_properties) {
        if (property->_has_this &&
            property->_ielement.get_name() == func->_ifunc.get_name()) {
          continue;
        }
      }
    }

    bool has_nonslotted = false;

    for (FunctionRemap *remap : func->_remaps) {
      if (!is_remap_legal(remap)) {
        continue;
      }

      SlottedFunctionDef slotted_def;

      if (get_slotted_function_def(obj, func, remap, slotted_def)) {
        const string &key = slotted_def._answer_location;
        if (slotted_def._wrapper_type == WT_none) {
          slotted_def._wrapper_name = func->_name;
        } else {
          slotted_def._wrapper_name = func->_name + "_" + key;
        }
        if (slots.count(key)) {
          slots[key]._remaps.insert(remap);
        } else {
          slots[key] = slotted_def;
          slots[key]._remaps.insert(remap);
        }
        if (slotted_def._keep_method) {
          has_nonslotted = true;
        }

        // Python 3 doesn't support nb_divide.  It has nb_true_divide and also
        // nb_floor_divide, but they have different semantics than in C++.
        // Ugh.  Make special slots to store the nb_divide members that take a
        // float.  We'll use this to build up nb_true_divide, so that we can
        // still properly divide float vector types.
        if (remap->_flags & FunctionRemap::F_divide_float) {
          string true_key;
          if (key == "nb_inplace_divide") {
            true_key = "nb_inplace_true_divide";
          } else {
            true_key = "nb_true_divide";
          }
          if (slots.count(true_key) == 0) {
            SlottedFunctionDef def;
            def._answer_location = true_key;
            def._wrapper_type = slotted_def._wrapper_type;
            def._wrapper_name = func->_name + "_" + true_key;
            slots[true_key] = def;
          }
          slots[true_key]._remaps.insert(remap);
        }
      } else {
        has_nonslotted = true;
      }
    }

    if (has_nonslotted) {
      // This is a bit of a hack, as these methods should probably be going
      // through the slotted function system.  But it's kind of pointless to
      // write these out, and a waste of space.
      string fname = func->_ifunc.get_name();
      if (fname == "operator <" ||
          fname == "operator <=" ||
          fname == "operator ==" ||
          fname == "operator !=" ||
          fname == "operator >" ||
          fname == "operator >=") {
        continue;
      }

      // This method has non-slotted remaps, so write it out into the function
      // table.
      out << "  {\"" << name1 << "\", " << fptr
          << ", " << flags << ", (const char *)" << func->_name << "_comment},\n";
      if (name1 != name2) {
        out << "  {\"" << name2 << "\", " << fptr
            << ", " << flags << ", (const char *)" << func->_name << "_comment},\n";
      }
    }
  }

  if (obj->_protocol_types & Object::PT_make_copy) {
    if (!got_copy) {
      out << "  {\"__copy__\", &copy_from_make_copy, METH_NOARGS, nullptr},\n";
      got_copy = true;
    }
  } else if (obj->_protocol_types & Object::PT_copy_constructor) {
    if (!got_copy) {
      out << "  {\"__copy__\", &copy_from_copy_constructor, METH_NOARGS, nullptr},\n";
      got_copy = true;
    }
  }

  if (got_copy && !got_deepcopy) {
    out << "  {\"__deepcopy__\", &map_deepcopy_to_copy, METH_VARARGS, nullptr},\n";
  }

  for (MakeSeq *make_seq : obj->_make_seqs) {
    if (!is_function_legal(make_seq->_length_getter) ||
        !is_function_legal(make_seq->_element_getter)) {
      continue;
    }

    string seq_name = make_seq->_imake_seq.get_name();

    string flags = "METH_NOARGS";
    if (!make_seq->_length_getter->_has_this &&
        !make_seq->_element_getter->_has_this) {
      flags += " | METH_STATIC";
    }

    string name1 = methodNameFromCppName(seq_name, export_class_name, false);
    string name2 = methodNameFromCppName(seq_name, export_class_name, true);
    out << "  {\"" << name1
        << "\", (PyCFunction) &" << make_seq->_name << ", " << flags << ", nullptr},\n";
    if (name1 != name2) {
      out << "  { \"" << name2
          << "\", (PyCFunction) &" << make_seq->_name << ", " << flags << ", nullptr},\n";
    }
  }

  out << "  {nullptr, nullptr, 0, nullptr}\n"
      << "};\n\n";

  int num_derivations = obj->_itype.number_of_derivations();
  int di;
  for (di = 0; di < num_derivations; di++) {
    TypeIndex d_type_Index = obj->_itype.get_derivation(di);
    if (!interrogate_type_is_unpublished(d_type_Index)) {
      const InterrogateType &d_itype = idb->get_type(d_type_Index);
      if (is_cpp_type_legal(d_itype._cpptype)) {
        if (!isExportThisRun(d_itype._cpptype)) {
          _external_imports.insert(TypeManager::resolve_type(d_itype._cpptype));

          // out << "IMPORT_THIS struct Dtool_PyTypedObject Dtool_" <<
          // make_safe_name(d_itype.get_scoped_name().c_str()) << ";\n";
        }
      }
    }
  }

  std::vector<CPPType*> bases;
  for (di = 0; di < num_derivations; di++) {
    TypeIndex d_type_Index = obj->_itype.get_derivation(di);
    if (!interrogate_type_is_unpublished(d_type_Index)) {
      const InterrogateType &d_itype = idb->get_type(d_type_Index);
      if (is_cpp_type_legal(d_itype._cpptype)) {
        bases.push_back(d_itype._cpptype);
      }
    }
  }

  {
    SlottedFunctions::iterator rfi;
    for (rfi = slots.begin(); rfi != slots.end(); rfi++) {
      const SlottedFunctionDef &def = rfi->second;

      // This is just for reporting.  There might be remaps from multiple
      // functions with different names mapped to the same slot.
      string fname;
      if (def._remaps.size() > 0) {
        const FunctionRemap *first_remap = *def._remaps.begin();
        fname = first_remap->_cppfunc->get_simple_name();
      }

      if (def._min_version > 0) {
        out << "#if PY_VERSION_HEX >= 0x" << hex << def._min_version << dec << "\n";
      }

      switch (rfi->second._wrapper_type) {
      case WT_no_params:
      case WT_iter_next:
        // PyObject *func(PyObject *self)
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" << def._wrapper_name << "(PyObject *self) {\n";
          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return nullptr;\n";
          out << "  }\n\n";

          int return_flags = RF_pyobject | RF_err_null;
          if (rfi->second._wrapper_type == WT_iter_next) {
            // If the function returns NULL, we should return NULL to indicate
            // a StopIteration, rather than returning None.
            return_flags |= RF_preserve_null;
          }
          string expected_params;
          write_function_forset(out, def._remaps, 0, 0, expected_params, 2, true, true,
                                AT_no_args, return_flags, false);

          out << "  if (!_PyErr_OCCURRED()) {\n";
          out << "    return Dtool_Raise_BadArgumentsError(\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return nullptr;\n";
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
          bool all_nonconst = true;
          for (FunctionRemap *remap : def._remaps) {
            if (remap->_const_method) {
              all_nonconst = false;
            }
          }
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" << def._wrapper_name << "(PyObject *self, PyObject *arg) {\n";
          out << "  " << cClassName << " *local_this = nullptr;\n";
          if (rfi->second._wrapper_type != WT_one_param) {
            // WT_binary_operator means we must return NotImplemented, instead
            // of raising an exception, if the this pointer doesn't match.
            // This is for things like __sub__, which Python likes to call on
            // the wrong-type objects.
            out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
            if (all_nonconst) {
              out << "  if (local_this == nullptr || DtoolInstance_IS_CONST(self)) {\n";
            } else {
              out << "  if (local_this == nullptr) {\n";
            }
            out << "    Py_INCREF(Py_NotImplemented);\n";
            out << "    return Py_NotImplemented;\n";
          } else if (all_nonconst) {
            out << "  if (!Dtool_Call_ExtractThisPointer_NonConst(self, Dtool_"
                << ClassName << ", (void **)&local_this, \"" << ClassName
                << "." << methodNameFromCppName(fname, "", false) << "\")) {\n";
            out << "    return nullptr;\n";
          } else {
            out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
            out << "    return nullptr;\n";
          }
          out << "  }\n";

          string expected_params;
          write_function_forset(out, def._remaps, 1, 1, expected_params, 2, true, true,
                                AT_single_arg, return_flags, false, !all_nonconst);

          if (rfi->second._wrapper_type != WT_one_param) {
            out << "  Py_INCREF(Py_NotImplemented);\n";
            out << "  return Py_NotImplemented;\n";
          } else {
            out << "  if (!_PyErr_OCCURRED()) {\n";
            out << "    return Dtool_Raise_BadArgumentsError(\n";
            output_quoted(out, 6, expected_params);
            out << ");\n";
            out << "  }\n";
            out << "  return nullptr;\n";
          }
          out << "}\n\n";
        }
        break;

      case WT_setattr:
        // int func(PyObject *self, PyObject *one, PyObject *two = NULL)
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static int " << def._wrapper_name << "(PyObject *self, PyObject *arg, PyObject *arg2) {\n";
          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          set<FunctionRemap*> setattr_remaps;
          set<FunctionRemap*> delattr_remaps;

          // This function handles both delattr and setattr.  Fish out the
          // remaps for both types.
          for (FunctionRemap *remap : def._remaps) {
            if (remap->_cppfunc->get_simple_name() == "__delattr__" && remap->_parameters.size() == 2) {
              delattr_remaps.insert(remap);

            } else if (remap->_cppfunc->get_simple_name() == "__setattr__" && remap->_parameters.size() == 3) {
              setattr_remaps.insert(remap);
            }
          }

          out << "  // Determine whether to call __setattr__ or __delattr__.\n";
          out << "  if (arg2 != nullptr) { // __setattr__\n";

          if (!setattr_remaps.empty()) {
            out << "    PyObject *args = PyTuple_Pack(2, arg, arg2);\n";
            string expected_params;
            write_function_forset(out, setattr_remaps, 2, 2, expected_params, 4,
                                  true, true, AT_varargs, RF_int | RF_decref_args, true);

            out << "    Py_DECREF(args);\n";
            out << "    if (!_PyErr_OCCURRED()) {\n";
            out << "      Dtool_Raise_BadArgumentsError(\n";
            output_quoted(out, 8, expected_params);
            out << ");\n";
            out << "    }\n";
          } else {
            out << "    PyErr_Format(PyExc_TypeError,\n";
            out << "      \"can't set attributes of built-in/extension type '%s'\",\n";
            out << "      Py_TYPE(self)->tp_name);\n";
          }
          out << "    return -1;\n\n";

          out << "  } else { // __delattr__\n";

          if (!delattr_remaps.empty()) {
            string expected_params;
            write_function_forset(out, delattr_remaps, 1, 1, expected_params, 4,
                                  true, true, AT_single_arg, RF_int, true);

            out << "    if (!_PyErr_OCCURRED()) {\n";
            out << "      Dtool_Raise_BadArgumentsError(\n";
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
        // PyObject *func(PyObject *self, PyObject *one) Specifically to
        // implement __getattr__. First calls PyObject_GenericGetAttr(), and
        // only calls the wrapper if it returns NULL. If one wants to override
        // this completely, one should define __getattribute__ instead.
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" << def._wrapper_name << "(PyObject *self, PyObject *arg) {\n";
          out << "  PyObject *res = PyObject_GenericGetAttr(self, arg);\n";
          out << "  if (res != nullptr) {\n";
          out << "    return res;\n";
          out << "  }\n";
          out << "  if (_PyErr_OCCURRED() != PyExc_AttributeError) {\n";
          out << "    return nullptr;\n";
          out << "  }\n";
          out << "  PyErr_Clear();\n\n";

          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return nullptr;\n";
          out << "  }\n\n";

          string expected_params;
          write_function_forset(out, def._remaps, 1, 1, expected_params, 2,
                                true, true, AT_single_arg,
                                RF_pyobject | RF_err_null, true);

          // out << "  PyErr_Clear();\n";
          out << "  return nullptr;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_getitem:
        // PyObject *func(PyObject *self, Py_ssize_t index)
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" << def._wrapper_name << "(PyObject *self, Py_ssize_t index) {\n";
          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return nullptr;\n";
          out << "  }\n\n";

          // This is a getitem or setitem of a sequence type.  This means we
          // *need* to raise IndexError if we're out of bounds.  We have to
          // assume the bounds are 0 .. this->size() (this is the same
          // assumption that Python makes).
          out << "  if (index < 0 || index >= (Py_ssize_t) local_this->size()) {\n";
          out << "    PyErr_SetString(PyExc_IndexError, \"" << ClassName << " index out of range\");\n";
          out << "    return nullptr;\n";
          out << "  }\n";

          string expected_params;
          write_function_forset(out, def._remaps, 1, 1, expected_params, 2, true, true,
                                AT_no_args, RF_pyobject | RF_err_null, false, true, "index");

          out << "  if (!_PyErr_OCCURRED()) {\n";
          out << "    return Dtool_Raise_BadArgumentsError(\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return nullptr;\n";
          out << "}\n\n";
        }
        break;

      case WT_sequence_setitem:
        // int_t func(PyObject *self, Py_ssize_t index, PyObject *value)
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static int " << def._wrapper_name << "(PyObject *self, Py_ssize_t index, PyObject *arg) {\n";
          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          out << "  if (index < 0 || index >= (Py_ssize_t) local_this->size()) {\n";
          out << "    PyErr_SetString(PyExc_IndexError, \"" << ClassName << " index out of range\");\n";
          out << "    return -1;\n";
          out << "  }\n";

          set<FunctionRemap*> setitem_remaps;
          set<FunctionRemap*> delitem_remaps;

          // This function handles both delitem and setitem.  Fish out the
          // remaps for either one.
          for (FunctionRemap *remap : def._remaps) {
            if (remap->_flags & FunctionRemap::F_setitem_int) {
              setitem_remaps.insert(remap);

            } else if (remap->_flags & FunctionRemap::F_delitem_int) {
              delitem_remaps.insert(remap);
            }
          }

          string expected_params;
          out << "  if (arg != nullptr) { // __setitem__\n";
          write_function_forset(out, setitem_remaps, 2, 2, expected_params, 4,
                                true, true, AT_single_arg, RF_int, false, true, "index");
          out << "  } else { // __delitem__\n";
          write_function_forset(out, delitem_remaps, 1, 1, expected_params, 4,
                                true, true, AT_single_arg, RF_int, false, true, "index");
          out << "  }\n\n";

          out << "  if (!_PyErr_OCCURRED()) {\n";
          out << "    Dtool_Raise_BadArgumentsError(\n";
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
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static Py_ssize_t " << def._wrapper_name << "(PyObject *self) {\n";
          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          // This is a cheap cheat around all of the overhead of calling the
          // wrapper function.
          out << "  return (Py_ssize_t) local_this->" << fname << "();\n";
          out << "}\n\n";
        }
        break;

      case WT_mapping_setitem:
        // int func(PyObject *self, PyObject *one, PyObject *two)
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static int " << def._wrapper_name << "(PyObject *self, PyObject *arg, PyObject *arg2) {\n";
          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          set<FunctionRemap*> setitem_remaps;
          set<FunctionRemap*> delitem_remaps;

          // This function handles both delitem and setitem.  Fish out the
          // remaps for either one.
          for (FunctionRemap *remap : def._remaps) {
            if (remap->_flags & FunctionRemap::F_setitem) {
              setitem_remaps.insert(remap);

            } else if (remap->_flags & FunctionRemap::F_delitem) {
              delitem_remaps.insert(remap);
            }
          }

          string expected_params;
          out << "  if (arg2 != nullptr) { // __setitem__\n";
          out << "    PyObject *args = PyTuple_Pack(2, arg, arg2);\n";
          write_function_forset(out, setitem_remaps, 2, 2, expected_params, 4,
                                true, true, AT_varargs, RF_int | RF_decref_args, false);
          out << "    Py_DECREF(args);\n";
          out << "  } else { // __delitem__\n";
          write_function_forset(out, delitem_remaps, 1, 1, expected_params, 4,
                                true, true, AT_single_arg, RF_int, false);
          out << "  }\n\n";

          out << "  if (!_PyErr_OCCURRED()) {\n";
          out << "    Dtool_Raise_BadArgumentsError(\n";
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
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static int " << def._wrapper_name << "(PyObject *self) {\n";

          // Find the remap.  There should be only one.
          FunctionRemap *remap = *def._remaps.begin();
          const char *container = "";

          if (remap->_has_this) {
            out << "  " << cClassName  << " *local_this = nullptr;\n";
            out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
            out << "    return -1;\n";
            out << "  }\n\n";
            container = "local_this";
          }

          vector_string params;
          out << "  return (int) " << remap->call_function(out, 4, false, container, params) << ";\n";
          out << "}\n\n";
        }
        break;

      case WT_getbuffer:
        // int __getbuffer__(PyObject *self, Py_buffer *buffer, int flags) We
        // map this directly, and assume that the arguments match.  The whole
        // point of this is to be fast, and we don't want to negate that by
        // first wrapping and then unwrapping the arguments again.  We also
        // want to guarantee const correctness, since that will determine
        // whether a read-only buffer is given.
        {
          has_local_getbuffer = true;

          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static int " << def._wrapper_name << "(PyObject *self, Py_buffer *buffer, int flags) {\n";
          out << "  " << cClassName << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          vector_string params_const(1);
          vector_string params_nonconst(1);
          FunctionRemap *remap_const = nullptr;
          FunctionRemap *remap_nonconst = nullptr;

          // Iterate through the remaps to find the one that matches our
          // parameters.
          for (FunctionRemap *remap : def._remaps) {
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

          // We have to distinguish properly between const and nonconst,
          // because the function may depend on it to decide whether to
          // provide a writable buffer or a readonly buffer.
          const string const_this = "(const " + cClassName + " *)local_this";
          if (remap_const != nullptr && remap_nonconst != nullptr) {
            out << "  if (!DtoolInstance_IS_CONST(self)) {\n";
            out << "    return " << remap_nonconst->call_function(out, 4, false, "local_this", params_nonconst) << ";\n";
            out << "  } else {\n";
            out << "    return " << remap_const->call_function(out, 4, false, const_this, params_const) << ";\n";
            out << "  }\n";
          } else if (remap_nonconst != nullptr) {
            out << "  if (!DtoolInstance_IS_CONST(self)) {\n";
            out << "    return " << remap_nonconst->call_function(out, 4, false, "local_this", params_nonconst) << ";\n";
            out << "  } else {\n";
            out << "    Dtool_Raise_TypeError(\"Cannot call " << ClassName << ".__getbuffer__() on a const object.\");\n";
            out << "    return -1;\n";
            out << "  }\n";
          } else if (remap_const != nullptr) {
            out << "  return " << remap_const->call_function(out, 4, false, const_this, params_const) << ";\n";
          } else {
            nout << ClassName << "::__getbuffer__ does not match the required signature.\n";
            out << "  return -1;\n";
          }

          out << "}\n\n";
        }
        break;

      case WT_releasebuffer:
        // void __releasebuffer__(PyObject *self, Py_buffer *buffer) Same
        // story as __getbuffer__ above.
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static void " << def._wrapper_name << "(PyObject *self, Py_buffer *buffer) {\n";
          out << "  " << cClassName << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return;\n";
          out << "  }\n\n";

          vector_string params_const(1);
          vector_string params_nonconst(1);
          FunctionRemap *remap_const = nullptr;
          FunctionRemap *remap_nonconst = nullptr;

          // Iterate through the remaps to find the one that matches our
          // parameters.
          for (FunctionRemap *remap : def._remaps) {
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
          if (remap_const != nullptr && remap_nonconst != nullptr) {
            out << "  if (!DtoolInstance_IS_CONST(self)) {\n";
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

          } else if (remap_nonconst != nullptr) {
            // Doesn't matter if there's no const version.  We *have* to call
            // it or else we could leak memory.
            return_expr = remap_nonconst->call_function(out, 2, false, "local_this", params_nonconst);
            if (!return_expr.empty()) {
              out << "  " << return_expr << ";\n";
            }

          } else if (remap_const != nullptr) {
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
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static PyObject *" << def._wrapper_name << "(PyObject *self, PyObject *arg, PyObject *arg2) {\n";
          out << "  " << cClassName << " *local_this = nullptr;\n";
          out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **)&local_this);\n";
          out << "  if (local_this == nullptr) {\n";
          // WT_ternary_operator means we must return NotImplemented, instead
          // of raising an exception, if the this pointer doesn't match.  This
          // is for things like __pow__, which Python likes to call on the
          // wrong-type objects.
          out << "    Py_INCREF(Py_NotImplemented);\n";
          out << "    return Py_NotImplemented;\n";
          out << "  }\n";

          set<FunctionRemap*> one_param_remaps;
          set<FunctionRemap*> two_param_remaps;

          for (FunctionRemap *remap : def._remaps) {
            if (remap->_parameters.size() == 2) {
              one_param_remaps.insert(remap);

            } else if (remap->_parameters.size() == 3) {
              two_param_remaps.insert(remap);
            }
          }

          string expected_params;

          out << "  if (arg2 != nullptr && arg2 != Py_None) {\n";
          out << "    PyObject *args = PyTuple_Pack(2, arg, arg2);\n";
          write_function_forset(out, two_param_remaps, 2, 2, expected_params, 4,
                                true, true, AT_varargs, RF_pyobject | RF_err_null | RF_decref_args, true);
          out << "    Py_DECREF(args);\n";
          out << "  } else {\n";
          write_function_forset(out, one_param_remaps, 1, 1, expected_params, 4,
                                true, true, AT_single_arg, RF_pyobject | RF_err_null, true);
          out << "  }\n\n";

          out << "  if (!_PyErr_OCCURRED()) {\n";
          out << "    return Dtool_Raise_BadArgumentsError(\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return nullptr;\n";
          out << "}\n\n";
        }
        break;

      case WT_traverse:
        // int __traverse__(PyObject *self, visitproc visit, void *arg) This
        // is a low-level function.  Overloads are not supported.
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static int " << def._wrapper_name << "(PyObject *self, visitproc visit, void *arg) {\n";

          // Find the remap.  There should be only one.
          FunctionRemap *remap = *def._remaps.begin();
          const char *container = "";

          if (remap->_has_this) {
            out << "  " << cClassName << " *local_this = nullptr;\n";
            out << "  DTOOL_Call_ExtractThisPointerForType(self, &Dtool_" << ClassName << ", (void **) &local_this);\n";
            out << "  if (local_this == nullptr) {\n";
            out << "    return 0;\n";
            out << "  }\n\n";
            container = "local_this";
          }

          vector_string params((int)remap->_has_this);
          params.push_back("visit");
          params.push_back("arg");

          out << "  return " << remap->call_function(out, 2, false, container, params) << ";\n";
          out << "}\n\n";
        }
        break;

      case WT_compare:
        // int func(PyObject *self, Py_ssize_t index)
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static int " << def._wrapper_name << "(PyObject *self, PyObject *arg) {\n";
          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          string expected_params;
          write_function_forset(out, def._remaps, 1, 1, expected_params, 2, true, true,
                                AT_single_arg, RF_compare, false, true);

          out << "  if (!_PyErr_OCCURRED()) {\n";
          out << "    Dtool_Raise_BadArgumentsError(\n";
          output_quoted(out, 6, expected_params);
          out << ");\n";
          out << "  }\n";
          out << "  return -1;\n";
          out << "}\n\n";
        }
        break;

      case WT_hash:
        // Py_hash_t func(PyObject *self)
        {
          out << "//////////////////\n";
          out << "// A wrapper function to satisfy Python's internal calling conventions.\n";
          out << "// " << ClassName << " slot " << rfi->second._answer_location << " -> " << fname << "\n";
          out << "//////////////////\n";
          out << "static Py_hash_t " << def._wrapper_name << "(PyObject *self) {\n";
          out << "  " << cClassName  << " *local_this = nullptr;\n";
          out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
          out << "    return -1;\n";
          out << "  }\n\n";

          FunctionRemap *remap = *def._remaps.begin();
          vector_string params;
          out << "  return (Py_hash_t) " << remap->call_function(out, 4, false, "local_this", params) << ";\n";
          out << "}\n\n";
        }
        break;

      case WT_none:
        // Nothing special about the wrapper function: just write it normally.
        string fname = "static PyObject *" + def._wrapper_name + "(PyObject *self, PyObject *args, PyObject *kwds)\n";

        std::vector<FunctionRemap *> remaps;
        remaps.insert(remaps.end(), def._remaps.begin(), def._remaps.end());
        string expected_params;
        write_function_for_name(out, obj, remaps, fname, expected_params, true, AT_keyword_args, RF_pyobject | RF_err_null);
        break;
      }

      if (def._min_version > 0) {
        out << "#endif  // PY_VERSION_HEX >= 0x" << hex << def._min_version << dec << "\n";
      }
    }

    int need_repr = 0;
    if (slots.count("tp_repr") == 0) {
      need_repr = NeedsAReprFunction(obj->_itype);
    }
    if (need_repr > 0) {
      out << "//////////////////\n";
      out << "//  A __repr__ function\n";
      out << "//     " << ClassName << "\n";
      out << "//////////////////\n";
      out << "static PyObject *Dtool_Repr_" << ClassName << "(PyObject *self) {\n";
      out << "  " << cClassName << " *local_this = nullptr;\n";
      out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
      out << "    return nullptr;\n";
      out << "  }\n\n";
      out << "  std::ostringstream os;\n";
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
      out << "  return Dtool_WrapValue(ss);\n";
      out << "}\n\n";
      has_local_repr = true;
    }

    int need_str = 0;
    if (slots.count("tp_str") == 0) {
      need_str = NeedsAStrFunction(obj->_itype);
    }
    if (need_str > 0) {
      out << "//////////////////\n";
      out << "//  A __str__ function\n";
      out << "//     " << ClassName << "\n";
      out << "//////////////////\n";
      out << "static PyObject *Dtool_Str_" << ClassName << "(PyObject *self) {\n";
      out << "  " << cClassName  << " *local_this = nullptr;\n";
      out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
      out << "    return nullptr;\n";
      out << "  }\n\n";
      out << "  std::ostringstream os;\n";
      if (need_str == 2) {
        out << "  local_this->write(os, 0);\n";
      } else {
        out << "  local_this->write(os);\n";
      }
      out << "  std::string ss = os.str();\n";
      out << "  return Dtool_WrapValue(ss);\n";
      out << "}\n\n";
      has_local_str = true;
    }
  }

  if (NeedsARichCompareFunction(obj->_itype) || slots.count("tp_compare")) {
    out << "//////////////////\n";
    out << "//  A rich comparison function\n";
    out << "//     " << ClassName << "\n";
    out << "//////////////////\n";
    out << "static PyObject *Dtool_RichCompare_" << ClassName << "(PyObject *self, PyObject *arg, int op) {\n";
    out << "  " << cClassName  << " *local_this = nullptr;\n";
    out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
    out << "    return nullptr;\n";
    out << "  }\n\n";

    for (Function *func : obj->_methods) {
      std::set<FunctionRemap*> remaps;
      if (!func) {
        continue;
      }
      // We only accept comparison operators that take one parameter (besides
      // 'this').
      for (FunctionRemap *remap : func->_remaps) {
        if (is_remap_legal(remap) && remap->_has_this && (remap->_args_type == AT_single_arg)) {
          remaps.insert(remap);
        }
      }
      const string &fname = func->_ifunc.get_name();
      const char *op_type;
      if (fname == "operator <") {
        op_type = "Py_LT";
      } else if (fname == "operator <=") {
        op_type = "Py_LE";
      } else if (fname == "operator ==") {
        op_type = "Py_EQ";
      } else if (fname == "operator !=") {
        op_type = "Py_NE";
      } else if (fname == "operator >") {
        op_type = "Py_GT";
      } else if (fname == "operator >=") {
        op_type = "Py_GE";
      } else {
        continue;
      }
      if (!has_local_richcompare) {
        out << "  switch (op) {\n";
        has_local_richcompare = true;
      }
      out << "  case " << op_type << ":\n";

      out << "    {\n";

      string expected_params;
      write_function_forset(out, remaps, 1, 1, expected_params, 6, true, false,
                            AT_single_arg, RF_pyobject | RF_err_null, false);

      out << "      break;\n";
      out << "    }\n";
    }

    if (has_local_richcompare) {
      // End of switch block
      out << "  }\n\n";
      out << "  if (_PyErr_OCCURRED()) {\n";
      out << "    PyErr_Clear();\n";
      out << "  }\n\n";
    }

    if (slots.count("tp_compare")) {
      // A lot of Panda code depends on comparisons being done via the
      // compare_to function, which is mapped to the tp_compare slot, which
      // Python 3 no longer has.  So, we'll write code to fall back to that if
      // no matching comparison operator was found.
      out << "  // All is not lost; we still have the compare_to function to fall back onto.\n";
      out << "  int cmpval = " << slots["tp_compare"]._wrapper_name << "(self, arg);\n";
      out << "  if (cmpval == -1 && _PyErr_OCCURRED()) {\n";
      out << "    if (PyErr_ExceptionMatches(PyExc_TypeError)) {\n";
      out << "      PyErr_Clear();\n";
      out << "    } else {\n";
      out << "      return nullptr;\n";
      out << "    }\n";
      out << "  }\n";
      out << "  switch (op) {\n";
      out << "  case Py_LT:\n";
      out << "    return PyBool_FromLong(cmpval < 0);\n";
      out << "  case Py_LE:\n";
      out << "    return PyBool_FromLong(cmpval <= 0);\n";
      out << "  case Py_EQ:\n";
      out << "    return PyBool_FromLong(cmpval == 0);\n";
      out << "  case Py_NE:\n";
      out << "    return PyBool_FromLong(cmpval != 0);\n";
      out << "  case Py_GT:\n";
      out << "    return PyBool_FromLong(cmpval > 0);\n";
      out << "  case Py_GE:\n";
      out << "    return PyBool_FromLong(cmpval >= 0);\n";
      out << "  }\n";
      has_local_richcompare = true;
    }

    out << "  Py_INCREF(Py_NotImplemented);\n";
    out << "  return Py_NotImplemented;\n";
    out << "}\n\n";
  }

  int num_getset = 0;

  if (obj->_properties.size() > 0) {
    // Write out the array of properties, telling Python which getter and
    // setter to call when they are assigned or queried in Python code.
    for (Property *property : obj->_properties) {
      const InterrogateElement &ielem = property->_ielement;
      if (!property->_has_this || property->_getter_remaps.empty()) {
        continue;
      }

      // Actually, if we have a conflicting static method with the same name,
      // we will need to use Dtool_StaticProperty instead.
      for (const Function *func : obj->_methods) {
        if (!func->_has_this && func->_ifunc.get_name() == ielem.get_name()) {
          continue;
        }
      }

      if (num_getset == 0) {
        out << "static PyGetSetDef Dtool_Properties_" << ClassName << "[] = {\n";
      }

      ++num_getset;

      string name1 = methodNameFromCppName(ielem.get_name(), "", false);
      // string name2 = methodNameFromCppName(ielem.get_name(), "", true);

      string getter = "&Dtool_" + ClassName + "_" + ielem.get_name() + "_Getter";
      string setter = "nullptr";
      if (!ielem.is_sequence() && !ielem.is_mapping() && !property->_setter_remaps.empty()) {
        setter = "&Dtool_" + ClassName + "_" + ielem.get_name() + "_Setter";
      }

      out << "  {(char *)\"" << name1 << "\", " << getter << ", " << setter;

      if (ielem.has_comment()) {
        out << ", (char *)\n";
        output_quoted(out, 4, ielem.get_comment());
        out << ",\n    ";
      } else {
        out << ", nullptr, ";
      }

      // Extra void* argument; we don't make use of it.
      out << "nullptr},\n";

      /*if (name1 != name2 && name1 != "__dict__") {
        // Add alternative spelling.
        out << "  {(char *)\"" << name2 << "\", " << getter << ", " << setter
            << ", (char *)\n"
            << "    \"Alias of " << name1 << ", for consistency with old naming conventions.\",\n"
            << "    NULL},\n";
      }*/
    }

    if (num_getset != 0) {
      out << "  {nullptr},\n";
      out << "};\n\n";
    }
  }

  // These fields are inherited together.  We should either write all of them
  // or none of them so that they are inherited from DTOOL_SUPER_BASE.
  bool has_hash_compare = (slots.count("tp_hash") != 0 ||
                           slots.count("tp_compare") != 0 ||
                           has_local_richcompare);

  bool has_parent_class = (obj->_itype.number_of_derivations() != 0);

  // Output the type slot tables.
  out << "static PyNumberMethods Dtool_NumberMethods_" << ClassName << " = {\n";
  write_function_slot(out, 2, slots, "nb_add");
  write_function_slot(out, 2, slots, "nb_subtract");
  write_function_slot(out, 2, slots, "nb_multiply");
  out << "#if PY_MAJOR_VERSION < 3\n";
  // Note: nb_divide does not exist in Python 3.  We will probably need some
  // smart mechanism for dispatching to either floor_divide or true_divide.
  write_function_slot(out, 2, slots, "nb_divide");
  out << "#endif\n";
  write_function_slot(out, 2, slots, "nb_remainder");
  write_function_slot(out, 2, slots, "nb_divmod");
  write_function_slot(out, 2, slots, "nb_power");
  write_function_slot(out, 2, slots, "nb_negative");
  write_function_slot(out, 2, slots, "nb_positive");
  write_function_slot(out, 2, slots, "nb_absolute");
  write_function_slot(out, 2, slots, "nb_bool");
  write_function_slot(out, 2, slots, "nb_invert");
  write_function_slot(out, 2, slots, "nb_lshift");
  write_function_slot(out, 2, slots, "nb_rshift");
  write_function_slot(out, 2, slots, "nb_and");
  write_function_slot(out, 2, slots, "nb_xor");
  write_function_slot(out, 2, slots, "nb_or");
  out << "#if PY_MAJOR_VERSION < 3\n";
  write_function_slot(out, 2, slots, "nb_coerce");
  out << "#endif\n";
  write_function_slot(out, 2, slots, "nb_int");
  out << "  nullptr, // nb_long\n"; // removed in Python 3
  write_function_slot(out, 2, slots, "nb_float");
  out << "#if PY_MAJOR_VERSION < 3\n";
  write_function_slot(out, 2, slots, "nb_oct");
  write_function_slot(out, 2, slots, "nb_hex");
  out << "#endif\n";

  write_function_slot(out, 2, slots, "nb_inplace_add");
  write_function_slot(out, 2, slots, "nb_inplace_subtract");
  write_function_slot(out, 2, slots, "nb_inplace_multiply");
  out << "#if PY_MAJOR_VERSION < 3\n";
  write_function_slot(out, 2, slots, "nb_inplace_divide");
  out << "#endif\n";
  write_function_slot(out, 2, slots, "nb_inplace_remainder");
  write_function_slot(out, 2, slots, "nb_inplace_power");
  write_function_slot(out, 2, slots, "nb_inplace_lshift");
  write_function_slot(out, 2, slots, "nb_inplace_rshift");
  write_function_slot(out, 2, slots, "nb_inplace_and");
  write_function_slot(out, 2, slots, "nb_inplace_xor");
  write_function_slot(out, 2, slots, "nb_inplace_or");

  write_function_slot(out, 2, slots, "nb_floor_divide");
  write_function_slot(out, 2, slots, "nb_true_divide");
  write_function_slot(out, 2, slots, "nb_inplace_floor_divide");
  write_function_slot(out, 2, slots, "nb_inplace_true_divide");

  out << "#if PY_VERSION_HEX >= 0x02050000\n";
  write_function_slot(out, 2, slots, "nb_index");
  out << "#endif\n";

  out << "#if PY_VERSION_HEX >= 0x03050000\n";
  write_function_slot(out, 2, slots, "nb_matrix_multiply");
  write_function_slot(out, 2, slots, "nb_inplace_matrix_multiply");
  out << "#endif\n";

  out << "};\n\n";

  // NB: it's tempting not to write this table when a class doesn't have them.
  // But then Python won't inherit them from base classes either!  So we
  // always write this table for now even if it will be full of 0's, unless
  // this type has no base classes at all.
  if (has_parent_class || (obj->_protocol_types & Object::PT_sequence) != 0) {
    out << "static PySequenceMethods Dtool_SequenceMethods_" << ClassName << " = {\n";
    write_function_slot(out, 2, slots, "sq_length");
    write_function_slot(out, 2, slots, "sq_concat");
    write_function_slot(out, 2, slots, "sq_repeat");
    write_function_slot(out, 2, slots, "sq_item");
    out << "  nullptr, // sq_slice\n"; // removed in Python 3
    write_function_slot(out, 2, slots, "sq_ass_item");
    out << "  nullptr, // sq_ass_slice\n"; // removed in Python 3
    write_function_slot(out, 2, slots, "sq_contains");

    write_function_slot(out, 2, slots, "sq_inplace_concat");
    write_function_slot(out, 2, slots, "sq_inplace_repeat");
    out << "};\n\n";
  }

  // Same note applies as for the SequenceMethods.
  if (has_parent_class || (obj->_protocol_types & Object::PT_mapping) != 0) {
    out << "static PyMappingMethods Dtool_MappingMethods_" << ClassName << " = {\n";
    write_function_slot(out, 2, slots, "mp_length");
    write_function_slot(out, 2, slots, "mp_subscript");
    write_function_slot(out, 2, slots, "mp_ass_subscript");
    out << "};\n\n";
  }

  // Same note applies as above.
  if (has_parent_class || has_local_getbuffer) {
    out << "static PyBufferProcs Dtool_BufferProcs_" << ClassName << " = {\n";
    out << "#if PY_MAJOR_VERSION < 3\n";
    write_function_slot(out, 2, slots, "bf_getreadbuffer");
    write_function_slot(out, 2, slots, "bf_getwritebuffer");
    write_function_slot(out, 2, slots, "bf_getsegcount");
    write_function_slot(out, 2, slots, "bf_getcharbuffer");
    out << "#endif\n";
    out << "#if PY_VERSION_HEX >= 0x02060000\n";
    write_function_slot(out, 2, slots, "bf_getbuffer");
    write_function_slot(out, 2, slots, "bf_releasebuffer");
    out << "#endif\n";
    out << "};\n\n";
  }

  bool have_async = false;
  if (has_parent_class || slots.count("am_await") != 0 ||
                          slots.count("am_aiter") != 0 ||
                          slots.count("am_anext") != 0) {
    out << "#if PY_VERSION_HEX >= 0x03050000\n";
    out << "static PyAsyncMethods Dtool_AsyncMethods_" << ClassName << " = {\n";
    write_function_slot(out, 2, slots, "am_await");
    write_function_slot(out, 2, slots, "am_aiter");
    write_function_slot(out, 2, slots, "am_anext");
    out << "};\n";
    out << "#endif\n\n";
    have_async = true;
  }

  // Output the actual PyTypeObject definition.
  out << "struct Dtool_PyTypedObject Dtool_" << ClassName << " = {\n";
  out << "  {\n";
  out << "    PyVarObject_HEAD_INIT(nullptr, 0)\n";
  // const char *tp_name;
  out << "    \"" << _def->module_name << "." << export_class_name << "\",\n";
  // Py_ssize_t tp_basicsize;
  out << "    sizeof(Dtool_PyInstDef),\n";
  // Py_ssize_t tp_itemsize;
  out << "    0, // tp_itemsize\n";

  // destructor tp_dealloc;
  out << "    &Dtool_FreeInstance_" << ClassName << ",\n";

  out << "#if PY_VERSION_HEX >= 0x03080000\n";
  out << "    0, // tp_vectorcall_offset\n";
  out << "#else\n";
  write_function_slot(out, 4, slots, "tp_print");
  out << "#endif\n";

  // getattrfunc tp_getattr;
  write_function_slot(out, 4, slots, "tp_getattr");
  // setattrfunc tp_setattr;
  write_function_slot(out, 4, slots, "tp_setattr");

  // cmpfunc tp_compare;  (reserved in Python 3)
  out << "#if PY_VERSION_HEX >= 0x03050000\n";
  if (have_async) {
    out << "    &Dtool_AsyncMethods_" << ClassName << ",\n";
  } else {
    out << "    nullptr, // tp_as_async\n";
  }
  out << "#elif PY_MAJOR_VERSION >= 3\n";
  out << "    nullptr, // tp_reserved\n";
  out << "#else\n";
  if (has_hash_compare) {
    write_function_slot(out, 4, slots, "tp_compare",
                        "&DtoolInstance_ComparePointers");
  } else {
    out << "    nullptr, // tp_compare\n";
  }
  out << "#endif\n";

  // reprfunc tp_repr;
  if (has_local_repr) {
    out << "    &Dtool_Repr_" << ClassName << ",\n";
  } else {
    write_function_slot(out, 4, slots, "tp_repr");
  }

  // PyNumberMethods *tp_as_number;
  out << "    &Dtool_NumberMethods_" << ClassName << ",\n";
  // PySequenceMethods *tp_as_sequence;
  if (has_parent_class || (obj->_protocol_types & Object::PT_sequence) != 0) {
    out << "    &Dtool_SequenceMethods_" << ClassName << ",\n";
  } else {
    out << "    nullptr, // tp_as_sequence\n";
  }
  // PyMappingMethods *tp_as_mapping;
  if (has_parent_class || (obj->_protocol_types & Object::PT_mapping) != 0) {
    out << "    &Dtool_MappingMethods_" << ClassName << ",\n";
  } else {
    out << "    nullptr, // tp_as_mapping\n";
  }

  // hashfunc tp_hash;
  if (has_hash_compare) {
    write_function_slot(out, 4, slots, "tp_hash", "&DtoolInstance_HashPointer");
  } else {
    out << "    nullptr, // tp_hash\n";
  }

  // ternaryfunc tp_call;
  write_function_slot(out, 4, slots, "tp_call");

  // reprfunc tp_str;
  if (has_local_str) {
    out << "    &Dtool_Str_" << ClassName << ",\n";
  } else if (has_local_repr) {
    out << "    &Dtool_Repr_" << ClassName << ",\n";
  } else {
    write_function_slot(out, 4, slots, "tp_str");
  }

  // getattrofunc tp_getattro;
  write_function_slot(out, 4, slots, "tp_getattro");
  // setattrofunc tp_setattro;
  write_function_slot(out, 4, slots, "tp_setattro");

  // PyBufferProcs *tp_as_buffer;
  if (has_parent_class || has_local_getbuffer) {
    out << "    &Dtool_BufferProcs_" << ClassName << ",\n";
  } else {
    out << "    nullptr, // tp_as_buffer\n";
  }

  string gcflag;
  if (obj->_protocol_types & Object::PT_python_gc) {
    gcflag = " | Py_TPFLAGS_HAVE_GC";
  }

  // long tp_flags;
  if (has_local_getbuffer) {
    out << "#if PY_VERSION_HEX >= 0x02060000 && PY_VERSION_HEX < 0x03000000\n";
    out << "    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES | Py_TPFLAGS_HAVE_NEWBUFFER" << gcflag << ",\n";
    out << "#else\n";
    out << "    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES" << gcflag << ",\n";
    out << "#endif\n";
  } else {
    out << "    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES" << gcflag << ",\n";
  }

  // const char *tp_doc;
  if (obj->_itype.has_comment()) {
    out << "#ifdef NDEBUG\n";
    out << "    0,\n";
    out << "#else\n";
    output_quoted(out, 4, obj->_itype.get_comment());
    out << ",\n";
    out << "#endif\n";
  } else {
    out << "    nullptr, // tp_doc\n";
  }

  // traverseproc tp_traverse;
  out << "    nullptr, // tp_traverse\n";
  //write_function_slot(out, 4, slots, "tp_traverse");

  // inquiry tp_clear;
  out << "    nullptr, // tp_clear\n";
  //write_function_slot(out, 4, slots, "tp_clear");

  // richcmpfunc tp_richcompare;
  if (has_local_richcompare) {
    out << "    &Dtool_RichCompare_" << ClassName << ",\n";
  } else if (has_hash_compare) {
    // All hashable types need to be comparable.
    out << "#if PY_MAJOR_VERSION >= 3\n";
    out << "    &DtoolInstance_RichComparePointers,\n";
    out << "#else\n";
    out << "    nullptr, // tp_richcompare\n";
    out << "#endif\n";
  } else {
    out << "    nullptr, // tp_richcompare\n";
  }

  // Py_ssize_t tp_weaklistoffset;
  out << "    0, // tp_weaklistoffset\n";

  // getiterfunc tp_iter;
  write_function_slot(out, 4, slots, "tp_iter");
  // iternextfunc tp_iternext;
  write_function_slot(out, 4, slots, "tp_iternext");

  // struct PyMethodDef *tp_methods;
  out << "    Dtool_Methods_" << ClassName << ",\n";
  // struct PyMemberDef *tp_members;
  out << "    nullptr, // tp_members\n";

  // struct PyGetSetDef *tp_getset;
  if (num_getset > 0) {
    out << "    Dtool_Properties_" << ClassName << ",\n";
  } else {
    out << "    nullptr, // tp_getset\n";
  }

  // struct _typeobject *tp_base;
  out << "    nullptr, // tp_base\n";
  // PyObject *tp_dict;
  out << "    nullptr, // tp_dict\n";
  // descrgetfunc tp_descr_get;
  write_function_slot(out, 4, slots, "tp_descr_get");
  // descrsetfunc tp_descr_set;
  write_function_slot(out, 4, slots, "tp_descr_set");
  // Py_ssize_t tp_dictoffset;
  out << "    0, // tp_dictoffset\n";
  // initproc tp_init;
  out << "    Dtool_Init_" << ClassName << ",\n";
  // allocfunc tp_alloc;
  out << "    PyType_GenericAlloc,\n";
  // newfunc tp_new;
  out << "    Dtool_new_" << ClassName << ",\n";
  // freefunc tp_free;
  if (obj->_protocol_types & Object::PT_python_gc) {
    out << "    PyObject_GC_Del,\n";
  } else {
    out << "    PyObject_Del,\n";
  }
  // inquiry tp_is_gc;
  out << "    nullptr, // tp_is_gc\n";
  // PyObject *tp_bases;
  out << "    nullptr, // tp_bases\n";
  // PyObject *tp_mro;
  out << "    nullptr, // tp_mro\n";
  // PyObject *tp_cache;
  out << "    nullptr, // tp_cache\n";
  // PyObject *tp_subclasses;
  out << "    nullptr, // tp_subclasses\n";
  // PyObject *tp_weaklist;
  out << "    nullptr, // tp_weaklist\n";
  // destructor tp_del;
  out << "    nullptr, // tp_del\n";
  // unsigned int tp_version_tag
  out << "#if PY_VERSION_HEX >= 0x02060000\n";
  out << "    0, // tp_version_tag\n";
  out << "#endif\n";
  // destructor tp_finalize
  out << "#if PY_VERSION_HEX >= 0x03040000\n";
  out << "    nullptr, // tp_finalize\n";
  out << "#endif\n";
  // vectorcallfunc tp_vectorcall
  out << "#if PY_VERSION_HEX >= 0x03080000\n";
  out << "    nullptr, // tp_vectorcall\n";
  out << "#endif\n";
  out << "  },\n";

  // It's tempting to initialize the type handle here, but this causes static
  // init ordering issues; this may run before init_type is called.
  out << "  TypeHandle::none(),\n";
  out << "  Dtool_PyModuleClassInit_" << ClassName << ",\n";
  out << "  Dtool_UpcastInterface_" << ClassName << ",\n";
  out << "  Dtool_DowncastInterface_" << ClassName << ",\n";

  int has_coerce = has_coerce_constructor(obj->_itype._cpptype->as_struct_type());
  if (has_coerce > 0) {
    if (TypeManager::is_reference_count(obj->_itype._cpptype)) {
      out << "  (CoerceFunction)Dtool_ConstCoerce_" << ClassName << ",\n";
      if (has_coerce > 1) {
        out << "  (CoerceFunction)Dtool_Coerce_" << ClassName << ",\n";
      } else {
        out << "  nullptr,\n";
      }
    } else {
      out << "  nullptr,\n";
      out << "  (CoerceFunction)Dtool_Coerce_" << ClassName << ",\n";
    }
  } else {
    out << "  nullptr,\n";
    out << "  nullptr,\n";
  }

  out << "};\n\n";

  out << "static void Dtool_PyModuleClassInit_" << ClassName << "(PyObject *module) {\n";
  out << "  (void) module; // Unused\n";
  out << "  static bool initdone = false;\n";
  out << "  if (!initdone) {\n";
  out << "    initdone = true;\n";

  // Add bases.
  out << "    // Dependent objects\n";
  if (bases.size() > 0) {
    string baseargs;
    for (CPPType *base : bases) {
      string safe_name = make_safe_name(base->get_local_name(&parser));

      if (isExportThisRun(base)) {
        baseargs += ", (PyTypeObject *)&Dtool_" + safe_name;
        out << "    Dtool_PyModuleClassInit_" << safe_name << "(nullptr);\n";

      } else {
        baseargs += ", (PyTypeObject *)Dtool_Ptr_" + safe_name;

        out << "    assert(Dtool_Ptr_" << safe_name << " != nullptr);\n"
            << "    assert(Dtool_Ptr_" << safe_name << "->_Dtool_ModuleClassInit != nullptr);\n"
            << "    Dtool_Ptr_" << safe_name << "->_Dtool_ModuleClassInit(nullptr);\n";
      }
    }

    out << "    Dtool_" << ClassName << "._PyType.tp_bases = PyTuple_Pack(" << bases.size() << baseargs << ");\n";
  } else {
    out << "    Dtool_" << ClassName << "._PyType.tp_base = (PyTypeObject *)Dtool_GetSuperBase();\n";
  }

  int num_nested = obj->_itype.number_of_nested_types();
  int num_dict_items = 1;

  // Go through once to estimate the number of elements the dict will hold.
  for (int ni = 0; ni < num_nested; ni++) {
    TypeIndex nested_index = obj->_itype.get_nested_type(ni);
    if (_objects.count(nested_index) == 0) {
      continue;
    }
    Object *nested_obj = _objects[nested_index];
    assert(nested_obj != nullptr);

    if (nested_obj->_itype.is_class() || nested_obj->_itype.is_struct()) {
      num_dict_items += 2;

    } else if (nested_obj->_itype.is_typedef()) {
      ++num_dict_items;

    } else if (nested_obj->_itype.is_enum() && !nested_obj->_itype.is_scoped_enum()) {
      CPPEnumType *enum_type = nested_obj->_itype._cpptype->as_enum_type();
      num_dict_items += 2 * enum_type->_elements.size();
    }
  }

  // Build type dictionary.  The size is just an estimation.
  if (num_dict_items > 5) {
    out << "    PyObject *dict = _PyDict_NewPresized(" << num_dict_items << ");\n";
  } else {
    out << "    PyObject *dict = PyDict_New();\n";
  }
  out << "    Dtool_" << ClassName << "._PyType.tp_dict = dict;\n";
  out << "    PyDict_SetItemString(dict, \"DtoolClassDict\", dict);\n";

  // Now go through the nested types again to actually add the dict items.
  for (int ni = 0; ni < num_nested; ni++) {
    TypeIndex nested_index = obj->_itype.get_nested_type(ni);
    if (_objects.count(nested_index) == 0) {
      // Illegal type.
      continue;
    }

    Object *nested_obj = _objects[nested_index];
    assert(nested_obj != nullptr);

    if (nested_obj->_itype.is_class() || nested_obj->_itype.is_struct()) {
      std::string ClassName1 = make_safe_name(nested_obj->_itype.get_scoped_name());
      std::string ClassName2 = make_safe_name(nested_obj->_itype.get_name());
      out << "    // Nested Object   " << ClassName1 << ";\n";
      out << "    Dtool_PyModuleClassInit_" << ClassName1 << "(nullptr);\n";
      string name1 = classNameFromCppName(ClassName2, false);
      string name2 = classNameFromCppName(ClassName2, true);
      out << "    PyDict_SetItemString(dict, \"" << name1 << "\", (PyObject *)&Dtool_" << ClassName1 << ");\n";
      if (name1 != name2) {
        out << "    PyDict_SetItemString(dict, \"" << name2 << "\", (PyObject *)&Dtool_" << ClassName1 << ");\n";
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
      out << "    PyDict_SetItemString(dict, \"" << name1 << "\", (PyObject *)&Dtool_" << ClassName1 << ");\n";
      // No need to support mangled names for nested typedefs; we only added
      // support recently.

    } else if (nested_obj->_itype.is_scoped_enum()) {
      // Convert enum class as Python 3.4-style enum.
      string class_name = nested_obj->_itype._cpptype->get_local_name(&parser);
      string safe_name = make_safe_name(class_name);

      int enum_count = nested_obj->_itype.number_of_enum_values();
      CPPType *underlying_type = TypeManager::unwrap_const(nested_obj->_itype._cpptype->as_enum_type()->get_underlying_type());
      string cast_to = underlying_type->get_local_name(&parser);
      out << "    // enum class " << nested_obj->_itype.get_scoped_name() << ";\n";
      out << "    {\n";
      out << "      PyObject *members = PyTuple_New(" << enum_count << ");\n";
      out << "      PyObject *member;\n";
      for (int xx = 0; xx < enum_count; xx++) {
        out << "      member = PyTuple_New(2);\n"
               "#if PY_MAJOR_VERSION >= 3\n"
               "      PyTuple_SET_ITEM(member, 0, PyUnicode_FromString(\""
            << nested_obj->_itype.get_enum_value_name(xx) << "\"));\n"
               "#else\n"
               "      PyTuple_SET_ITEM(member, 0, PyString_FromString(\""
            << nested_obj->_itype.get_enum_value_name(xx) << "\"));\n"
               "#endif\n"
               "      PyTuple_SET_ITEM(member, 1, Dtool_WrapValue(("
            << cast_to << ")" << nested_obj->_itype.get_scoped_name() << "::"
            << nested_obj->_itype.get_enum_value_name(xx) << "));\n"
               "      PyTuple_SET_ITEM(members, " << xx << ", member);\n";
      }
      out << "      Dtool_Ptr_" << safe_name << " = Dtool_EnumType_Create(\""
          << nested_obj->_itype.get_name() << "\", members, \""
          << _def->module_name << "\");\n";
      out << "      PyDict_SetItemString(dict, \"" << nested_obj->_itype.get_name()
          << "\", (PyObject *)Dtool_Ptr_" << safe_name << ");\n";
      out << "    }\n";

    } else if (nested_obj->_itype.is_enum()) {
      out << "    // enum " << nested_obj->_itype.get_scoped_name() << ";\n";
      CPPEnumType *enum_type = nested_obj->_itype._cpptype->as_enum_type();
      CPPEnumType::Elements::const_iterator ei;
      for (ei = enum_type->_elements.begin(); ei != enum_type->_elements.end(); ++ei) {
        string name1 = classNameFromCppName((*ei)->get_simple_name(), false);
        string name2;
        if (nested_obj->_itype.has_true_name()) {
          name2 = classNameFromCppName((*ei)->get_simple_name(), true);
        } else {
          // Don't generate the alternative syntax for anonymous enums, since
          // we added support for those after we started deprecating the
          // alternative syntax.
          name2 = name1;
        }
        string enum_value = obj->_itype.get_scoped_name() + "::" + (*ei)->get_simple_name();
        out << "    PyDict_SetItemString(dict, \"" << name1 << "\", Dtool_WrapValue(" << enum_value << "));\n";
        if (name1 != name2) {
          out << "    PyDict_SetItemString(dict, \"" << name2 << "\", Dtool_WrapValue(" << enum_value << "));\n";
        }
      }
    }
  }

  // Also add the static properties, which can't be added via getset.
  for (Property *property : obj->_properties) {
    const InterrogateElement &ielem = property->_ielement;
    if (property->_getter_remaps.empty()) {
      continue;
    }
    if (property->_has_this) {
      // Actually, continue if we have a conflicting static method with the
      // same name, which may still require use of Dtool_StaticProperty.
      bool have_shadow = false;
      for (const Function *func : obj->_methods) {
        if (!func->_has_this && func->_ifunc.get_name() == ielem.get_name()) {
          have_shadow = true;
          break;
        }
      }
      if (!have_shadow) {
        continue;
      }
    }

    string name1 = methodNameFromCppName(ielem.get_name(), "", false);
    // string name2 = methodNameFromCppName(ielem.get_name(), "", true);

    string getter = "&Dtool_" + ClassName + "_" + ielem.get_name() + "_Getter";
    string setter = "nullptr";
    if (!ielem.is_sequence() && !ielem.is_mapping() && !property->_setter_remaps.empty()) {
      setter = "&Dtool_" + ClassName + "_" + ielem.get_name() + "_Setter";
    }

    out << "    static const PyGetSetDef def_" << name1 << " = {(char *)\"" << name1 << "\", " << getter << ", " << setter;

    if (ielem.has_comment()) {
      out << ", (char *)\n";
      output_quoted(out, 4, ielem.get_comment());
      out << ",\n    ";
    } else {
      out << ", nullptr, ";
    }

    // Extra void* argument; we don't make use of it.
    out << "nullptr};\n";

    out << "    PyDict_SetItemString(dict, \"" << name1 << "\", Dtool_NewStaticProperty(&Dtool_" << ClassName << "._PyType, &def_" << name1 << "));\n";
    /* Alternative spelling:
    out << "    PyDict_SetItemString(\"" << name2 << "\", &def_" << name1 << ");\n";
    */
  }

  out << "    if (PyType_Ready((PyTypeObject *)&Dtool_" << ClassName << ") < 0) {\n"
         "      Dtool_Raise_TypeError(\"PyType_Ready(" << ClassName << ")\");\n"
         "      return;\n"
         "    }\n"
         "    Py_INCREF((PyTypeObject *)&Dtool_" << ClassName << ");\n"
         "  }\n";

/*
 * Also write out the explicit alternate names.  int num_alt_names =
 * obj->_itype.get_num_alt_names(); for (int i = 0; i < num_alt_names; ++i) {
 * string alt_name = make_safe_name(obj->_itype.get_alt_name(i)); if
 * (export_class_name != alt_name) { out << "    PyModule_AddObject(module,
 * \"" << alt_name << "\", (PyObject *)&Dtool_" << ClassName <<
 * ".As_PyTypeObject());\n"; } }
 */

  // out << "  }\n";
  out << "}\n\n";
}

/**
 * This method should be overridden and redefined to return true for
 * interfaces that require the implicit "this" parameter, if present, to be
 * passed as the first parameter to any wrapper functions.
 */
bool InterfaceMakerPythonNative::
synthesize_this_parameter() {
  return true;
}

/**
 * This method should be overridden and redefined to return true for
 * interfaces that require overloaded instances of a function to be defined as
 * separate functions (each with its own hashed name), or false for interfaces
 * that can support overloading natively, and thus only require one wrapper
 * function per each overloaded input function.
 */
bool InterfaceMakerPythonNative::
separate_overloading() {
  // We used to return true here.  Nowadays, some of the default arguments are
  // handled in the PyArg_ParseTuple code, and some are still being considered
  // as separate overloads (this depends on a bunch of factors, see
  // collapse_default_remaps). This is all handled elsewhere.
  return false;
}

/**
 * Returns the prefix string used to generate wrapper function names.
 */
string InterfaceMakerPythonNative::
get_wrapper_prefix() {
  return "Dtool_";
}

/**
 * Returns the prefix string used to generate unique symbolic names, which are
 * not necessarily C-callable function names.
 */
string InterfaceMakerPythonNative::
get_unique_prefix() {
  return "Dtool_";
}

/**
 * Associates the function wrapper with its function in the appropriate
 * structures in the database.
 */
void InterfaceMakerPythonNative::
record_function_wrapper(InterrogateFunction &ifunc, FunctionWrapperIndex wrapper_index) {
  ifunc._python_wrappers.push_back(wrapper_index);
}

/**
 * Writes the prototype for the indicated function.
 */
void InterfaceMakerPythonNative::
write_prototype_for(ostream &out, InterfaceMaker::Function *func) {
  std::string fname = "PyObject *" + func->_name + "(PyObject *self, PyObject *args)";
  write_prototype_for_name(out, func, fname);
}
/**
 *
 */
void InterfaceMakerPythonNative::
write_prototype_for_name(ostream &out, InterfaceMaker::Function *func, const std::string &function_namename) {
// Function::Remaps::const_iterator ri;

// for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
// FunctionRemap *remap = (*ri);
    if (!output_function_names) {
      // If we're not saving the function names, don't export it from the
      // library.
      out << "static ";
    } else {
      out << "extern \"C\" ";
    }
    out << function_namename << ";\n";
// }
}

/**
 * Writes the definition for a function that will call the indicated C++
 * function or method.
 */
void InterfaceMakerPythonNative::
write_function_for_top(ostream &out, InterfaceMaker::Object *obj, InterfaceMaker::Function *func) {

  // First check if this function has non-slotted and legal remaps, ie.  if we
  // should even write it.
  bool has_remaps = false;

  for (FunctionRemap *remap : func->_remaps) {
    if (!is_remap_legal(remap)) {
      continue;
    }

    SlottedFunctionDef slotted_def;
    if (!get_slotted_function_def(obj, func, remap, slotted_def) || slotted_def._keep_method) {
      // It has a non-slotted remap, so we should write it.
      has_remaps = true;
      break;
    }
  }

  if (!has_remaps) {
    // Nope.
    return;
  }

  // This is a bit of a hack, as these methods should probably be going
  // through the slotted function system.  But it's kind of pointless to write
  // these out, and a waste of space.
  string fname = func->_ifunc.get_name();
  if (fname == "operator <" ||
      fname == "operator <=" ||
      fname == "operator ==" ||
      fname == "operator !=" ||
      fname == "operator >" ||
      fname == "operator >=") {
    return;
  }

  if (func->_ifunc.is_unary_op()) {
    assert(func->_args_type == AT_no_args);
  }

  string prototype = "static PyObject *" + func->_name + "(PyObject *";

  // This will be NULL for static funcs, so prevent code from using it.
  if (func->_has_this) {
    prototype += "self";
  }

  switch (func->_args_type) {
  case AT_keyword_args:
    prototype += ", PyObject *args, PyObject *kwds";
    break;

  case AT_varargs:
    prototype += ", PyObject *args";
    break;

  case AT_single_arg:
    prototype += ", PyObject *arg";
    break;

  default:
    prototype += ", PyObject *";
    break;
  }
  prototype += ")";

  string expected_params;
  write_function_for_name(out, obj, func->_remaps, prototype, expected_params, true, func->_args_type, RF_pyobject | RF_err_null);

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
  out << "static const char *" << func->_name << "_comment = nullptr;\n";
  out << "#endif\n\n";
}

/**
 * Writes the definition for a function that will call the indicated C++
 * function or method.
 */
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
  FunctionRemap *remap = nullptr;
  int max_required_args = 0;
  bool all_nonconst = true;
  bool has_keywords = false;

  out << "/**\n * Python function wrapper for:\n";
  for (ri = remaps.begin(); ri != remaps.end(); ++ri) {
    remap = (*ri);
    if (is_remap_legal(remap)) {
      int min_num_args = remap->get_min_num_args();
      int max_num_args = remap->get_max_num_args();
      if (remap->_has_this) {
        has_this = true;
      }

      if (!remap->_has_this || remap->_const_method) {
        all_nonconst = false;
      }

      if (remap->_args_type == AT_keyword_args) {
        has_keywords = true;
      }

      max_required_args = max(max_num_args, max_required_args);

      for (int i = min_num_args; i <= max_num_args; ++i) {
        map_sets[i].insert(remap);
      }
      out << " * ";
      remap->write_orig_prototype(out, 0, false, (max_num_args - min_num_args));
      out << "\n";
    } else {
      out << " * Rejected Remap [";
      remap->write_orig_prototype(out, 0);
      out << "]\n";
    }
  }

  out << " */\n";

  if (has_this && obj == nullptr) {
    assert(obj != nullptr);
  }

  out << function_name << " {\n";

  if (has_this) {
    std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
    std::string cClassName = obj->_itype.get_true_name();
    // string class_name = remap->_cpptype->get_simple_name();

    // Extract pointer from 'self' parameter.
    out << "  " << cClassName << " *local_this = nullptr;\n";

    if (all_nonconst) {
      // All remaps are non-const.  Also check that this object isn't const.
      out << "  if (!Dtool_Call_ExtractThisPointer_NonConst(self, Dtool_" << ClassName << ", "
          << "(void **)&local_this, \"" << classNameFromCppName(cClassName, false)
          << "." << methodNameFromCppName(remap, cClassName, false) << "\")) {\n";

    } else {
      out << "  if (!DtoolInstance_GetPointer(self, local_this, Dtool_" << ClassName << ")) {\n";
    }

    error_return(out, 4, return_flags);
    out << "  }\n";
  }

  if (map_sets.empty()) {
    error_return(out, 2, return_flags);
    out << "}\n\n";
    return;
  }

  if (args_type == AT_keyword_args && !has_keywords) {
    // We don't actually take keyword arguments.  Make sure we didn't get any.
    out << "  if (kwds != nullptr && PyDict_Size(kwds) > 0) {\n";
    out << "#ifdef NDEBUG\n";
    error_raise_return(out, 4, return_flags, "TypeError", "function takes no keyword arguments");
    out << "#else\n";
    error_raise_return(out, 4, return_flags, "TypeError",
      methodNameFromCppName(remap, "", false) + "() takes no keyword arguments");
    out << "#endif\n";
    out << "  }\n";
    args_type = AT_varargs;
  }

  if (args_type == AT_keyword_args || args_type == AT_varargs) {
    max_required_args = collapse_default_remaps(map_sets, max_required_args);
  }

  if (remap->_flags & FunctionRemap::F_explicit_args) {
    // We have a remap that wants to handle the wrapper itself.
    string expected_params;
    write_function_instance(out, remap, 0, 0, expected_params, 2, true, true,
                            args_type, return_flags);

  } else if (map_sets.size() > 1 && (args_type == AT_varargs || args_type == AT_keyword_args)) {
    // We have more than one remap.
    switch (args_type) {
    case AT_keyword_args:
      indent(out, 2) << "int parameter_count = (int)PyTuple_Size(args);\n";
      indent(out, 2) << "if (kwds != nullptr) {\n";
      indent(out, 2) << "  parameter_count += (int)PyDict_Size(kwds);\n";
      indent(out, 2) << "}\n";
      break;

    case AT_varargs:
      indent(out, 2) << "int parameter_count = (int)PyTuple_Size(args);\n";
      break;

    case AT_single_arg:
      // It shouldn't get here, but we'll handle these cases nonetheless.
      indent(out, 2) << "const int parameter_count = 1;\n";
      break;

    default:
      indent(out, 2) << "const int parameter_count = 0;\n";
      break;
    }

    // Keep track of how many args this function actually takes for the error
    // message.  We add one to the parameter count for "self", following the
    // Python convention.
    int add_self = has_this ? 1 : 0;
    set<int> num_args;

    indent(out, 2) << "switch (parameter_count) {\n";
    for (mii = map_sets.begin(); mii != map_sets.end(); ++mii) {
      int max_args = mii->first;
      int min_args = min(max_required_args, max_args);

      for (int i = min_args; i <= max_args; ++i) {
        indent(out, 2) << "case " << i << ":\n";
        num_args.insert(i + add_self);
      }
      num_args.insert(max_args + add_self);

      bool strip_keyword_args = false;

      // Check whether any remap actually takes keyword arguments.  If not,
      // then we don't have to bother checking that for every remap.
      if (args_type == AT_keyword_args && max_args > 0) {
        strip_keyword_args = true;

        std::set<FunctionRemap *>::iterator sii;
        for (sii = mii->second.begin(); sii != mii->second.end(); ++sii) {
          remap = (*sii);
          size_t first_param = remap->_has_this ? 1u : 0u;
          for (size_t i = first_param; i < remap->_parameters.size(); ++i) {
            if (remap->_parameters[i]._has_name) {
              strip_keyword_args = false;
              break;
            }
          }
        }
      }

      if (strip_keyword_args) {
        // None of the remaps take any keyword arguments, so let's check that
        // we take none.  This saves some checks later on.
        indent(out, 4) << "if (kwds == nullptr || PyDict_GET_SIZE(kwds) == 0) {\n";
        if (min_args == 1 && min_args == 1) {
          indent(out, 4) << "  PyObject *arg = PyTuple_GET_ITEM(args, 0);\n";
          write_function_forset(out, mii->second, min_args, max_args, expected_params, 6,
                coercion_allowed, true, AT_single_arg, return_flags, true, !all_nonconst);
        } else {
          write_function_forset(out, mii->second, min_args, max_args, expected_params, 6,
                coercion_allowed, true, AT_varargs, return_flags, true, !all_nonconst);
        }
      } else if (min_args == 1 && max_args == 1 && args_type == AT_varargs) {
        // We already checked that the args tuple has only one argument, so
        // we might as well extract that from the tuple now.
        indent(out, 4) << "{\n";
        indent(out, 4) << "  PyObject *arg = PyTuple_GET_ITEM(args, 0);\n";

        write_function_forset(out, mii->second, min_args, max_args, expected_params, 6,
                      coercion_allowed, true, AT_single_arg, return_flags, true, !all_nonconst);
      } else {
        indent(out, 4) << "{\n";
        write_function_forset(out, mii->second, min_args, max_args, expected_params, 6,
                      coercion_allowed, true, args_type, return_flags, true, !all_nonconst);
      }

      indent(out, 4) << "}\n";
      indent(out, 4) << "break;\n";
    }

    // In NDEBUG case, fall through to the error at end of function.
    out << "#ifndef NDEBUG\n";

    indent(out, 2) << "default:\n";

    // Format an error saying how many arguments we actually take.  So much
    // logic for such a silly matter.  Sheesh.
    ostringstream msg;
    msg << methodNameFromCppName(remap, "", false) << "() takes ";

    set<int>::iterator si = num_args.begin();
    msg << *si;
    if (num_args.size() == 2) {
      msg << " or " << *(++si);
    } else if (num_args.size() > 2) {
      ++si;
      while (si != num_args.end()) {
        int num = *si;
        if ((++si) == num_args.end()) {
          msg << " or " << num;
        } else {
          msg << ", " << num;
        }
      }
    }
    msg << " arguments (%d given)";

    string count_var = "parameter_count";
    if (add_self) {
      count_var += " + 1";
    }

    error_raise_return(out, 4, return_flags, "TypeError",
                       msg.str(), count_var);
    out << "#endif\n";
    indent(out, 2) << "}\n";

    out << "  if (!_PyErr_OCCURRED()) {\n"
        << "    ";
    if ((return_flags & ~RF_pyobject) == RF_err_null) {
      out << "return ";
    }
    out << "Dtool_Raise_BadArgumentsError(\n";
    output_quoted(out, 6, expected_params);
    out << ");\n"
        << "  }\n";

    error_return(out, 2, return_flags);

  } else {
    mii = map_sets.begin();

    // If no parameters are accepted, we do need to check that the argument
    // count is indeed 0, since we won't check that in
    // write_function_instance.
    if (mii->first == 0 && args_type != AT_no_args) {
      switch (args_type) {
      case AT_keyword_args:
        out << "  if (!Dtool_CheckNoArgs(args, kwds)) {\n";
        out << "    int parameter_count = (int)PyTuple_Size(args);\n";
        out << "    if (kwds != nullptr) {\n";
        out << "      parameter_count += (int)PyDict_Size(kwds);\n";
        out << "    }\n";
        break;
      case AT_varargs:
        out << "  if (!Dtool_CheckNoArgs(args)) {\n";
        out << "    const int parameter_count = (int)PyTuple_GET_SIZE(args);\n";
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

      out << "#ifdef NDEBUG\n";
      error_raise_return(out, 4, return_flags, "TypeError", "function takes no arguments");
      out << "#else\n";
      error_raise_return(out, 4, return_flags, "TypeError",
        methodNameFromCppName(remap, "", false) + "() takes no arguments (%d given)",
        "parameter_count");
      out << "#endif\n";
      out << "  }\n";

    } else if (args_type == AT_keyword_args && max_required_args == 1 && mii->first == 1) {
      // Check this to be sure, as we handle the case of only 1 keyword arg in
      // write_function_forset (not using ParseTupleAndKeywords).
      out << "  int parameter_count = (int)PyTuple_Size(args);\n"
             "  if (kwds != nullptr) {\n"
             "    parameter_count += (int)PyDict_Size(kwds);\n"
             "  }\n"
             "  if (parameter_count != 1) {\n"
             "#ifdef NDEBUG\n";
      error_raise_return(out, 4, return_flags, "TypeError",
                        "function takes exactly 1 argument");
      out << "#else\n";
      error_raise_return(out, 4, return_flags, "TypeError",
        methodNameFromCppName(remap, "", false) + "() takes exactly 1 argument (%d given)",
        "parameter_count");
      out << "#endif\n";
      out << "  }\n";
    }

    int min_args = min(max_required_args, mii->first);
    write_function_forset(out, mii->second, min_args, mii->first, expected_params, 2,
                          coercion_allowed, true, args_type, return_flags, true, !all_nonconst);

    // This block is often unreachable for many functions... maybe we can
    // figure out a way in the future to better determine when it will be and
    // won't be necessary to write this out.
    if (args_type != AT_no_args) {
      out << "  if (!_PyErr_OCCURRED()) {\n"
          << "    ";
      if ((return_flags & ~RF_pyobject) == RF_err_null) {
        out << "return ";
      }
      out << "Dtool_Raise_BadArgumentsError(\n";
      output_quoted(out, 6, expected_params);
      out << ");\n"
          << "  }\n";

      error_return(out, 2, return_flags);
    }
  }

  out << "}\n\n";
}

/**
 * Writes the definition for a coerce constructor: a special constructor that
 * is called to implicitly cast a tuple or other type to a desired type.  This
 * is done by calling the appropriate constructor or static make() function.
 * Constructors marked with the "explicit" keyword aren't considered, just
 * like in C++.
 *
 * There are usually two coerce constructors: one for const pointers, one for
 * non-const pointers.  This is due to the possibility that a static make()
 * function may return a const pointer.
 *
 * There are two variants of this: if the class in question is a
 * ReferenceCount, the coerce constructor takes a reference to a PointerTo or
 * ConstPointerTo to store the converted pointer in.  Otherwise, it is a
 * regular pointer, and an additional boolean indicates whether the caller is
 * supposed to call "delete" on the coerced pointer or not.
 *
 * In all cases, the coerce constructor returns a bool indicating whether the
 * conversion was possible.  It does not raise exceptions when none of the
 * constructors matched, but just returns false.
 */
void InterfaceMakerPythonNative::
write_coerce_constructor(ostream &out, Object *obj, bool is_const) {
  std::map<int, std::set<FunctionRemap *> > map_sets;
  std::map<int, std::set<FunctionRemap *> >::iterator mii;

  int max_required_args = 0;

  // Go through the methods and find appropriate static make() functions.
  for (Function *func : obj->_methods) {
    for (FunctionRemap *remap : func->_remaps) {
      if (is_remap_legal(remap) && remap->_flags & FunctionRemap::F_coerce_constructor) {
        nassertd(!remap->_has_this) continue;

        // It's a static make() function.
        CPPType *return_type = remap->_return_type->get_new_type();

        if (!is_const && TypeManager::is_const_pointer_or_ref(return_type)) {
          // If we're making the non-const coerce constructor, reject this
          // remap if it returns a const pointer.
          continue;
        }

        int min_num_args = remap->get_min_num_args();
        int max_num_args = remap->get_max_num_args();

        // Coerce constructor should take at least one argument.
        nassertd(max_num_args > 0) continue;
        min_num_args = max(min_num_args, 1);

        max_required_args = max(max_num_args, max_required_args);

        for (int i = min_num_args; i <= max_num_args; ++i) {
          map_sets[i].insert(remap);
        }

        size_t parameter_size = remap->_parameters.size();
        map_sets[parameter_size].insert(remap);
      }
    }
  }

  // Now go through the constructors that are suitable for coercion.  This
  // excludes copy constructors and ones marked "explicit".
  for (Function *func : obj->_constructors) {
    for (FunctionRemap *remap : func->_remaps) {
      if (is_remap_legal(remap) && remap->_flags & FunctionRemap::F_coerce_constructor) {
        nassertd(!remap->_has_this) continue;

        int min_num_args = remap->get_min_num_args();
        int max_num_args = remap->get_max_num_args();

        // Coerce constructor should take at least one argument.
        nassertd(max_num_args > 0) continue;
        min_num_args = max(min_num_args, 1);

        max_required_args = max(max_num_args, max_required_args);

        for (int i = min_num_args; i <= max_num_args; ++i) {
          map_sets[i].insert(remap);
        }

        size_t parameter_size = remap->_parameters.size();
        map_sets[parameter_size].insert(remap);
      }
    }
  }

  std::string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName = obj->_itype.get_true_name();

  int return_flags = RF_coerced;

  if (TypeManager::is_reference_count(obj->_itype._cpptype)) {
    // The coercion works slightly different for reference counted types,
    // since we can handle those a bit more nicely by taking advantage of the
    // refcount instead of having to use a boolean to indicate that it should
    // be managed.
    if (is_const) {
      out << "bool Dtool_ConstCoerce_" << ClassName << "(PyObject *args, CPT(" << cClassName << ") &coerced) {\n";
    } else {
      out << "bool Dtool_Coerce_" << ClassName << "(PyObject *args, PT(" << cClassName << ") &coerced) {\n";
    }

    // Note: this relies on the PT() being initialized to NULL.  This is
    // currently the case in all invocations, but this may not be true in the
    // future.
    out << "  if (DtoolInstance_GetPointer(args, coerced.cheat(), Dtool_" << ClassName << ")) {\n";
    out << "    // The argument is already of matching type, no need to coerce.\n";
    if (!is_const) {
      out << "    if (!DtoolInstance_IS_CONST(args)) {\n";
      out << "      // A non-const instance is required, which this is.\n";
      out << "      coerced->ref();\n";
      out << "      return true;\n";
      out << "    }\n";
    } else {
      out << "    coerced->ref();\n";
      out << "    return true;\n";
    }
    return_flags |= RF_err_false;

  } else {
    out << cClassName << " *Dtool_Coerce_" << ClassName << "(PyObject *args, " << cClassName << " &coerced) {\n";

    out << "  " << cClassName << " *local_this;\n";
    out << "  if (DtoolInstance_GetPointer(args, local_this, Dtool_" << ClassName << ")) {\n";
    out << "    if (DtoolInstance_IS_CONST(args)) {\n";
    out << "      // This is a const object.  Make a copy.\n";
    out << "      coerced = *(const " << cClassName << " *)local_this;\n";
    out << "      return &coerced;\n";
    out << "    }\n";
    out << "    return local_this;\n";

    return_flags |= RF_err_null;
  }

  out << "  }\n\n";

  if (map_sets.empty()) {
    error_return(out, 2, return_flags);
    out << "}\n\n";
    return;
  }

  // Coercion constructors are special cases in that they can take either a
  // single value or a tuple.  (They never, however, take a tuple containing a
  // single value.)
  string expected_params;
  mii = map_sets.find(1);
  if (mii != map_sets.end()) {
    out << "  if (!PyTuple_Check(args)) {\n";
    out << "    PyObject *arg = args;\n";

    write_function_forset(out, mii->second, mii->first, mii->first, expected_params, 4, false, false,
                          AT_single_arg, return_flags, true, false);

    if (map_sets.size() == 1) {
      out << "  }\n";
      // out << "  PyErr_Clear();\n";
      error_return(out, 2, return_flags);
      out << "}\n\n";
      return;
    }

    // We take this one out of the map sets.  There's not much value in
    // coercing tuples containing just one value.
    map_sets.erase(mii);

    out << "  } else {\n";
  } else {
    out << "  if (PyTuple_Check(args)) {\n";
  }

  max_required_args = collapse_default_remaps(map_sets, max_required_args);

  if (map_sets.size() > 1) {
    indent(out, 4) << "switch (PyTuple_GET_SIZE(args)) {\n";

    for (mii = map_sets.begin(); mii != map_sets.end(); ++mii) {
      int max_args = mii->first;
      int min_args = min(max_required_args, max_args);

      // This is not called for tuples containing just one value or no values
      // at all, so we should never have to consider that case.
      if (min_args < 2) {
        min_args = 2;
      }
      nassertd(max_args >= min_args) continue;

      for (int i = min_args; i < max_args; ++i) {
        if (i != 1) {
          indent(out, 6) << "case " << i << ":\n";
        }
      }
      indent(out, 6) << "case " << max_args << ": {\n";

      write_function_forset(out, mii->second, min_args, max_args, expected_params, 8, false, false,
                            AT_varargs, return_flags, true, false);

      indent(out, 8) << "break;\n";
      indent(out, 6) << "}\n";
    }
    indent(out, 4) << "}\n";

  } else {
    mii = map_sets.begin();

    int max_args = mii->first;
    int min_args = min(max_required_args, max_args);

    // This is not called for tuples containing just one value or no values at
    // all, so we should never have to consider that case.
    if (min_args < 2) {
      min_args = 2;
    }
    nassertv(max_args >= min_args);

    if (min_args == max_args) {
      indent(out, 4) << "if (PyTuple_GET_SIZE(args) == " << mii->first << ") {\n";
    } else {
      indent(out, 4) << "Py_ssize_t size = PyTuple_GET_SIZE(args);\n";
      // Not sure if this check really does any good.  I guess it's a useful
      // early-fail test.
      indent(out, 4) << "if (size >= " << min_args << " && size <= " << max_args << ") {\n";
    }

    write_function_forset(out, mii->second, min_args, max_args, expected_params, 6, false, false,
                          AT_varargs, return_flags, true, false);
    indent(out, 4) << "}\n";
  }

  out << "  }\n\n";
  // out << "  PyErr_Clear();\n";
  error_return(out, 2, return_flags);
  out << "}\n\n";
}

/**
 * Special case optimization: if the last map is a subset of the map before
 * it, we can merge the cases.  When this happens, we can make use of a
 * special feature of PyArg_ParseTuple for handling of these last few default
 * arguments.
 *
 * This isn't just to help reduce the amount of generated code; it also
 * enables arbitrary selection of keyword arguments for many functions, ie.
 * for this function:
 *
 * int func(int a=0, int b=0, bool c=false, string d="");
 *
 * Thanks to this mechanism, we can call it like so:
 *
 * func(c=True, d=".")
 */
int InterfaceMakerPythonNative::
collapse_default_remaps(std::map<int, std::set<FunctionRemap *> > &map_sets,
                        int max_required_args) {
  if (map_sets.size() < 1) {
    return max_required_args;
  }

  std::map<int, std::set<FunctionRemap *> >::reverse_iterator rmi, rmi_next;
  rmi = map_sets.rbegin();
  rmi_next = rmi;
  for (++rmi_next; rmi_next != map_sets.rend();) {
    if (std::includes(rmi_next->second.begin(), rmi_next->second.end(),
                      rmi->second.begin(), rmi->second.end())) {

      // rmi_next has a superset of the remaps in rmi, and we are going to
      // erase rmi_next, so put all the remaps in rmi.

      max_required_args = rmi_next->first;
      rmi = rmi_next;
      ++rmi_next;
    } else {
      break;
    }
  }

  // Now erase the other remap sets.  Reverse iterators are weird, we first
  // need to get forward iterators and decrement them by one.
  std::map<int, std::set<FunctionRemap *> >::iterator erase_begin, erase_end;
  erase_begin = rmi.base();
  erase_end = map_sets.rbegin().base();
  --erase_begin;
  --erase_end;

  if (erase_begin == erase_end) {
    return max_required_args;
  }

  // We're never erasing the map set with the highest number of args.
  nassertr(erase_end != map_sets.end(), max_required_args);

  // We know erase_begin is a superset of erase_end, but we want all the
  // remaps in erase_end (which we aren't erasing). if (rmi ==
  // map_sets.rbegin()) {
  erase_end->second = erase_begin->second;
  // }

  map_sets.erase(erase_begin, erase_end);

  assert(map_sets.size() >= 1);

  return max_required_args;
}

/**

 */
int get_type_sort(CPPType *type) {
  int answer = 0;
// printf("    %s\n",type->get_local_name().c_str());

  // The highest numbered one will be checked first.
  if (TypeManager::is_nullptr(type)) {
    return 15;
  } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
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
  } else if (TypeManager::is_integer(type) && !TypeManager::is_bool(type)) {
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
// printf(" Class Name %s  %d\n",itype.get_name().c_str(),answer);
  }

// printf(" Class Name %s  %d\n",itype.get_name().c_str(),answer);
  return answer;
}

// The Core sort function for remap calling orders..
bool RemapCompareLess(FunctionRemap *in1, FunctionRemap *in2) {
  assert(in1 != nullptr);
  assert(in2 != nullptr);

  if (in1->_const_method != in2->_const_method) {
    // Non-const methods should come first.
    return in2->_const_method;
  }

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

/**
 * Writes out a set of function wrappers that handle all instances of a
 * particular function with the same number of parameters.  (Actually, in some
 * cases relating to default argument handling, this may be called with remaps
 * taking a range of parameters.)
 *
 * min_num_args and max_num_args are the range of parameter counts to respect
 * for these functions.  This is important for default argument handling.
 *
 * expected_params is a reference to a string that will be filled in with a
 * list of overloads that this function takes, for displaying in the doc
 * string and error messages.
 *
 * If coercion_allowed is true, it will attempt to convert arguments to the
 * appropriate parameter type using the appropriate Dtool_Coerce function.
 * This means it may write some remaps twice: once without coercion, and then
 * it may go back and write it a second time to try parameter coercion.
 *
 * If report_errors is true, it will print an error and exit when one has
 * occurred, instead of falling back to the next overload.  This is
 * automatically disabled when more than one function is passed.
 *
 * args_type indicates whether this function takes no args, a single PyObject*
 * arg, an args tuple, or an args tuple and kwargs dictionary.
 *
 * return_flags indicates which value should be returned from the wrapper
 * function and what should be returned on error.
 *
 * If check_exceptions is false, it will not check if the function raised an
 * exception, except if it took PyObject* arguments.  This should NEVER be
 * false for C++ functions that call Python code, since that would block a
 * meaningful exception like SystemExit or KeyboardInterrupt.
 *
 * If verify_const is set, it will write out a check to make sure that non-
 * const functions aren't called for a const "this".  This is usually only
 * false when write_function_for_name has already done this check (which it
 * does when *all* remaps are non-const).
 *
 * If first_pexpr is not empty, it represents the preconverted value of the
 * first parameter.  This is a special-case hack for one of the slot
 * functions.
 */
void InterfaceMakerPythonNative::
write_function_forset(ostream &out,
                      const std::set<FunctionRemap *> &remapsin,
                      int min_num_args, int max_num_args,
                      string &expected_params, int indent_level,
                      bool coercion_allowed, bool report_errors,
                      ArgsType args_type, int return_flags,
                      bool check_exceptions, bool verify_const,
                      const string &first_pexpr) {

  if (remapsin.empty()) {
    return;
  }

  FunctionRemap *remap = nullptr;
  std::set<FunctionRemap *>::iterator sii;

  bool all_nonconst = false;

  if (verify_const) {
    // Check if all of the remaps are non-const.  If so, we only have to check
    // the constness of the self pointer once, rather than per remap.
    all_nonconst = true;

    for (sii = remapsin.begin(); sii != remapsin.end(); ++sii) {
      remap = (*sii);
      if (!remap->_has_this || remap->_const_method) {
        all_nonconst = false;
        break;
      }
    }

    if (all_nonconst) {
      // Yes, they do.  Check that the parameter has the required constness.
      indent(out, indent_level)
        << "if (!DtoolInstance_IS_CONST(self)) {\n";
      indent_level += 2;
      verify_const = false;
    }
  }

  string first_param_name;
  bool same_first_param = false;

  // If there's only one arg and all remaps have the same parameter name, we
  // extract it from the dictionary, so we don't have to call
  // ParseTupleAndKeywords.
  if (first_pexpr.empty() && min_num_args == 1 && max_num_args == 1 &&
      args_type == AT_keyword_args) {
    sii = remapsin.begin();
    remap = (*sii);
    if (remap->_parameters[(int)remap->_has_this]._has_name) {
      first_param_name = remap->_parameters[(int)remap->_has_this]._name;
      same_first_param = true;

      for (++sii; sii != remapsin.end(); ++sii) {
        remap = (*sii);
        if (remap->_parameters[(int)remap->_has_this]._name != first_param_name) {
          same_first_param = false;
          break;
        }
      }
    }
  }

  if (same_first_param) {
    // Yes, they all have the same argument name (or there is only one remap).
    // Extract it from the dict so we don't have to call
    // ParseTupleAndKeywords.
    indent(out, indent_level) << "PyObject *arg;\n";
    indent(out, indent_level) << "if (Dtool_ExtractArg(&arg, args, kwds, \"" << first_param_name << "\")) {\n";
    indent_level += 2;
    args_type = AT_single_arg;
  }

  if (remapsin.size() > 1) {
    // There are multiple different overloads for this number of parameters.
    // Sort them all into order from most-specific to least-specific, then try
    // them one at a time.
    std::vector<FunctionRemap *> remaps (remapsin.begin(), remapsin.end());
    std::sort(remaps.begin(), remaps.end(), RemapCompareLess);
    std::vector<FunctionRemap *>::const_iterator sii;

    int num_coercion_possible = 0;
    sii = remaps.begin();
    while (sii != remaps.end()) {
      remap = *(sii++);

      if (coercion_allowed && is_remap_coercion_possible(remap)) {
        if (++num_coercion_possible == 1 && sii == remaps.end()) {
          // This is the last remap, and it happens to be the only one with
          // coercion possible.  So we might as well just break off now, and
          // let this case be handled by the coercion loop, below.  BUG: this
          // remap doesn't get listed in expected_params.
          break;
        }
      }

      if (verify_const && (remap->_has_this && !remap->_const_method)) {
        // If it's a non-const method, we only allow a non-const this.
        indent(out, indent_level)
          << "if (!DtoolInstance_IS_CONST(self)) {\n";
      } else {
        indent(out, indent_level)
          << "{\n";
      }

      indent(out, indent_level) << "  // -2 ";
      remap->write_orig_prototype(out, 0, false, (max_num_args - min_num_args));
      out << "\n";

      // NB.  We don't pass on report_errors here because we want it to
      // silently drop down to the next overload.

      write_function_instance(out, remap, min_num_args, max_num_args,
                              expected_params, indent_level + 2,
                              false, false, args_type, return_flags,
                              check_exceptions, first_pexpr);

      indent(out, indent_level) << "}\n\n";
    }

    // Go through one more time, but allow coercion this time.
    if (coercion_allowed) {
      for (sii = remaps.begin(); sii != remaps.end(); sii ++) {
        remap = (*sii);
        if (!is_remap_coercion_possible(remap)) {
          indent(out, indent_level)
            << "// No coercion possible: ";
          remap->write_orig_prototype(out, 0, false, (max_num_args - min_num_args));
          out << "\n";
          continue;
        }

        if (verify_const && (remap->_has_this && !remap->_const_method)) {
          indent(out, indent_level)
            << "if (!DtoolInstance_IS_CONST(self)) {\n";
        } else {
          indent(out, indent_level)
            << "{\n";
        }

        indent(out, indent_level) << "  // -2 ";
        remap->write_orig_prototype(out, 0, false, (max_num_args - min_num_args));
        out << "\n";

        string ignore_expected_params;
        write_function_instance(out, remap, min_num_args, max_num_args,
                                ignore_expected_params, indent_level + 2,
                                true, false, args_type, return_flags,
                                check_exceptions, first_pexpr);

        indent(out, indent_level) << "}\n\n";
      }
    }
  } else {
    // There is only one possible overload with this number of parameters.
    // Just call it.
    sii = remapsin.begin();

    remap = (*sii);
    indent(out, indent_level)
      << "// 1-" ;
    remap->write_orig_prototype(out, 0, false, (max_num_args - min_num_args));
    out << "\n";

    write_function_instance(out, remap, min_num_args, max_num_args,
                            expected_params, indent_level,
                            coercion_allowed, report_errors,
                            args_type, return_flags,
                            check_exceptions, first_pexpr);
  }

  // Close the brace we opened earlier.
  if (same_first_param) {
    indent_level -= 2;
    indent(out, indent_level) << "}\n";
  }

  // If we did a const check earlier, and we were asked to report errors,
  // write out an else case raising an exception.
  if (all_nonconst) {
    if (report_errors) {
      indent(out, indent_level - 2)
        << "} else {\n";

      string class_name = remap->_cpptype->get_simple_name();
      ostringstream msg;
      msg << "Cannot call "
          << classNameFromCppName(class_name, false)
          << "." << methodNameFromCppName(remap, class_name, false)
          << "() on a const object.";

      out << "#ifdef NDEBUG\n";
      error_raise_return(out, indent_level, return_flags, "TypeError",
                         "non-const method called on const object");
      out << "#else\n";
      error_raise_return(out, indent_level, return_flags, "TypeError", msg.str());
      out << "#endif\n";
    }
    indent_level -= 2;
    indent(out, indent_level) << "}\n";
  }
}

/**
 * Writes out the code to handle a a single instance of an overloaded
 * function.  This will convert all of the arguments from PyObject* to the
 * appropriate C++ type, call the C++ function, possibly check for errors, and
 * construct a Python wrapper for the return value.
 *
 * return_flags indicates which value should be returned from the wrapper
 * function and what should be returned on error.
 *
 * If coercion_possible is true, it will attempt to convert arguments to the
 * appropriate parameter type using the appropriate Dtool_Coerce function.
 *
 * If report_errors is true, it will print an error and exit when one has
 * occurred, instead of falling back to the next overload.  This should be
 * done if it is the only overload.
 *
 * If check_exceptions is false, it will not check if the function raised an
 * exception, except if it took PyObject* arguments.  This should NEVER be
 * false for C++ functions that call Python code, since that would block a
 * meaningful exception like SystemExit or KeyboardInterrupt.
 *
 * If first_pexpr is not empty, it represents the preconverted value of the
 * first parameter.  This is a special-case hack for one of the slot
 * functions.
 */
void InterfaceMakerPythonNative::
write_function_instance(ostream &out, FunctionRemap *remap,
                        int min_num_args, int max_num_args,
                        string &expected_params, int indent_level,
                        bool coercion_possible, bool report_errors,
                        ArgsType args_type, int return_flags,
                        bool check_exceptions,
                        const string &first_pexpr) {
  string format_specifiers;
  string keyword_list;
  string parameter_list;
  string container;
  string type_check;
  string param_name;
  bool has_keywords = false;
  vector_string pexprs;
  LineStream extra_convert;
  ostringstream extra_param_check;
  LineStream extra_cleanup;
  int min_version = 0;

  // This will be set if the function itself is suspected of possibly raising
  // a TypeError.
  bool may_raise_typeerror = false;

  // This will be set to true if one of the things we're about to do *might*
  // raise a TypeError that we may have to clear.
  bool clear_error = false;

  bool is_constructor = (remap->_type == FunctionRemap::T_constructor);

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  // Make one pass through the parameter list.  We will output a one-line
  // temporary variable definition for each parameter, while simultaneously
  // building the ParseTuple() function call and also the parameter expression
  // list for call_function().

  expected_params += methodNameFromCppName(remap, "", false);
  expected_params += "(";

  int num_params = 0;

  if ((remap->_flags & FunctionRemap::F_explicit_args) == 0) {
    num_params = max_num_args;
    if (remap->_has_this) {
      num_params += 1;
    }
    if (num_params > (int)remap->_parameters.size()) {
      // Limit to how many parameters this remap actually has.
      num_params = (int)remap->_parameters.size();
      max_num_args = num_params;
      if (remap->_has_this) {
        --max_num_args;
      }
    }
    nassertv(num_params <= (int)remap->_parameters.size());
  }

  bool only_pyobjects = true;

  int pn = 0;

  if (remap->_has_this) {
    // The first parameter is the 'this' parameter.
    string expected_class_name = classNameFromCppName(remap->_cpptype->get_simple_name(), false);
    if (remap->_const_method) {
      expected_params += expected_class_name + " self";
      string class_name = remap->_cpptype->get_local_name(&parser);
      container = "(const " + class_name + "*)local_this";
    } else {
      expected_params += "const " + expected_class_name + " self";
      container = "local_this";
    }
    pexprs.push_back(container);
    ++pn;
  }

  if (!first_pexpr.empty()) {
    if (pn >= num_params) {
      // first_pexpr was passed even though the function takes no arguments.
      nassert_raise("pn < num_params");
    } else {
      // The first actual argument was already converted.
      if (pn > 0) {
        expected_params += ", ";
      }
      expected_params += first_pexpr;
      pexprs.push_back(first_pexpr);
      ++pn;
    }
  }

  if (remap->_flags & FunctionRemap::F_explicit_args) {
    // The function handles the arguments by itself.
    expected_params += "*args";
    pexprs.push_back("args");
    if (args_type == AT_keyword_args) {
      expected_params += ", **kwargs";
      pexprs.push_back("kwds");
    }
    num_params = 0;
  }

  // Now convert (the rest of the) actual arguments, one by one.
  for (; pn < num_params; ++pn) {
    ParameterRemap *param = remap->_parameters[pn]._remap;
    CPPType *orig_type = param->get_orig_type();
    CPPType *type = param->get_new_type();
    CPPExpression *default_value = param->get_default_value();
    param_name = remap->get_parameter_name(pn);

    if (!is_cpp_type_legal(orig_type)) {
      // We can't wrap this.  We sometimes get here for default arguments.
      // Just skip this parameter.
      continue;
    }

    // Has this remap been selected to consider optional arguments for this
    // parameter?  We can do that by adding a vertical bar to the
    // PyArg_ParseTuple format string, coupled with some extra logic in the
    // argument handling, below.
    bool is_optional = false;
    if (remap->_has_this && !is_constructor) {
      if (pn > min_num_args) {
        is_optional = true;
        if ((pn - 1) == min_num_args) {
          format_specifiers += "|";
        }
      }
    } else {
      if (pn >= min_num_args) {
        is_optional = true;
        if (pn == min_num_args) {
          format_specifiers += "|";
        }
      }
    }

    if (pn > 0) {
      expected_params += ", ";
    }

    // This is the string to convert our local variable to the appropriate C++
    // type.  Normally this is just a cast.
    string pexpr_string =
      "(" + orig_type->get_local_name(&parser) + ")" + param_name;

    string default_expr;
    const char *null_assign = "";

    if (is_optional) {
      // If this is an optional argument, PyArg_ParseTuple will leave the
      // variable unchanged if it has been omitted, so we have to initialize
      // it to the desired default expression.  Format it.
      ostringstream default_expr_str;
      default_expr_str << " = ";
      default_value->output(default_expr_str, 0, &parser, false);
      default_expr = default_expr_str.str();
      null_assign = " = nullptr";

      // We should only ever have to consider optional arguments for functions
      // taking a variable number of arguments.
      nassertv(args_type == AT_varargs || args_type == AT_keyword_args);
    }

    string reported_name = remap->_parameters[pn]._name;
    if (!keyword_list.empty()) {
      keyword_list += ", \"" + reported_name + "\"";
    } else {
      keyword_list = "\"" + reported_name + "\"";
    }
    if (remap->_parameters[pn]._has_name) {
      has_keywords = true;
    }

    if (param->new_type_is_atomic_string()) {

      if (TypeManager::is_char_pointer(orig_type)) {
        indent(out, indent_level) << "char ";
        if (TypeManager::is_const_char_pointer(orig_type)) {
          out << "const ";
        }
        out << "*" << param_name << default_expr << ";\n";
        format_specifiers += "z";
        parameter_list += ", &" + param_name;
        expected_params += "str";

      } else if (TypeManager::is_wchar_pointer(orig_type)) {
        out << "#if PY_VERSION_HEX >= 0x03020000\n";
        indent(out, indent_level) << "PyObject *" << param_name << null_assign << ";\n";
        out << "#else\n";
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << null_assign << ";\n";
        out << "#endif\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        if (is_optional) {
          extra_convert
            << "wchar_t *" << param_name << "_str = nullptr;\n"
            << "if (" << param_name << " != nullptr) {\n"
            << "#if PY_VERSION_HEX >= 0x03030000\n"
            << "  " << param_name << "_str = PyUnicode_AsWideCharString(" << param_name << ", nullptr);\n"
            << "#else"
            << "Py_ssize_t " << param_name << "_len = PyUnicode_GET_SIZE(" << param_name << ");\n"
            << "  " << param_name << "_str = (wchar_t *)alloca(sizeof(wchar_t) * (" + param_name + "_len + 1));\n"
            << "PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_str, " << param_name << "_len);\n"
            << param_name << "_str[" << param_name << "_len] = 0;\n"
            << "#endif\n"
            << "} else {\n"
            << "  " << param_name << "_str" << default_expr << ";\n"
            << "}\n";
        } else {
          extra_convert
            << "#if PY_VERSION_HEX >= 0x03030000\n"
            << "wchar_t *" << param_name << "_str = PyUnicode_AsWideCharString(" << param_name << ", nullptr);\n"
            << "#else"
            << "Py_ssize_t " << param_name << "_len = PyUnicode_GET_SIZE(" << param_name << ");\n"
            << "wchar_t *" << param_name << "_str = (wchar_t *)alloca(sizeof(wchar_t) * (" + param_name + "_len + 1));\n"
            << "PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_str, " << param_name << "_len);\n"
            << param_name << "_str[" << param_name << "_len] = 0;\n"
            << "#endif\n";
        }

        pexpr_string = param_name + "_str";

        extra_cleanup
          << "#if PY_VERSION_HEX >= 0x03030000\n"
          << "PyMem_Free(" << param_name << "_str);\n"
          << "#endif\n";

        expected_params += "unicode";

      } else if (TypeManager::is_wstring(orig_type) ||
                 TypeManager::is_const_ptr_to_basic_string_wchar(orig_type)) {
        out << "#if PY_VERSION_HEX >= 0x03020000\n";
        indent(out, indent_level) << "PyObject *" << param_name << null_assign << ";\n";
        out << "#else\n";
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << null_assign << ";\n";
        out << "#endif\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;

        if (is_optional) {
          extra_convert
            << "Py_ssize_t " << param_name << "_len;\n"
            << "wchar_t *" << param_name << "_str = nullptr;\n"
            << "std::wstring " << param_name << "_wstr;\n"
            << "if (" << param_name << " != nullptr) {\n"
            << "#if PY_VERSION_HEX >= 0x03030000\n"
            << "  " << param_name << "_str = PyUnicode_AsWideCharString("
            << param_name << ", &" << param_name << "_len);\n"
            << "#else\n"
            << "  " << param_name << "_len = PyUnicode_GET_SIZE(" << param_name << ");\n"
            << "  " << param_name << "_str = (wchar_t *)alloca(sizeof(wchar_t) * (" + param_name + "_len + 1));\n"
            << "PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_str, " << param_name << "_len);\n"
            << "#endif\n"
            << "  " << param_name << "_wstr.assign(" << param_name << "_str, " << param_name << "_len);\n"
            << "} else {\n"
            << "  " << param_name << "_wstr" << default_expr << ";\n"
            << "}\n";
          pexpr_string = "std::move(" + param_name + "_wstr)";
        } else {
          extra_convert
            << "#if PY_VERSION_HEX >= 0x03030000\n"
            << "Py_ssize_t " << param_name << "_len;\n"
            << "wchar_t *" << param_name << "_str = PyUnicode_AsWideCharString("
            << param_name << ", &" << param_name << "_len);\n"
            << "#else\n"
            << "Py_ssize_t " << param_name << "_len = PyUnicode_GET_SIZE(" << param_name << ");\n"
            << "wchar_t *" << param_name << "_str = (wchar_t *)alloca(sizeof(wchar_t) * (" + param_name + "_len + 1));\n"
            << "PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_str, " << param_name << "_len);\n"
            << "#endif\n";
          pexpr_string = param_name + "_str, " + param_name + "_len";
        }

        extra_cleanup
          << "#if PY_VERSION_HEX >= 0x03030000\n"
          << "PyMem_Free(" << param_name << "_str);\n"
          << "#endif\n";

        expected_params += "unicode";

      } else { // A regular string.
        if (is_optional) {
          CPPExpression::Type expr_type = default_value->_type;
          if (expr_type == CPPExpression::T_default_construct) {
            // The default string constructor yields an empty string.
            indent(out, indent_level) << "const char *" << param_name << "_str = \"\";\n";
            indent(out, indent_level) << "Py_ssize_t " << param_name << "_len = 0;\n";
          } else {
            // We only get here for string literals, so this should be fine
            indent(out, indent_level) << "const char *" << param_name << "_str"
                                      << default_expr << ";\n";
            indent(out, indent_level) << "Py_ssize_t " << param_name << "_len = "
                                      << default_value->_str.size() << ";\n";
          }
        } else {
          indent(out, indent_level) << "const char *" << param_name << "_str = nullptr;\n";
          indent(out, indent_level) << "Py_ssize_t " << param_name << "_len;\n";
        }

        if (args_type == AT_single_arg) {
          out << "#if PY_MAJOR_VERSION >= 3\n";
          indent(out, indent_level)
            << param_name << "_str = PyUnicode_AsUTF8AndSize(arg, &"
            << param_name << "_len);\n";
          out << "#else\n"; // NB. PyString_AsStringAndSize also accepts a PyUnicode.
          indent(out, indent_level) << "if (PyString_AsStringAndSize(arg, (char **)&"
            << param_name << "_str, &" << param_name << "_len) == -1) {\n";
          indent(out, indent_level + 2) << param_name << "_str = nullptr;\n";
          indent(out, indent_level) << "}\n";
          out << "#endif\n";

          extra_param_check << " && " << param_name << "_str != nullptr";
        } else {
          format_specifiers += "s#";
          parameter_list += ", &" + param_name
            + "_str, &" + param_name + "_len";
        }

        //if (TypeManager::is_const_ptr_to_basic_string_char(orig_type)) {
        //  pexpr_string = "&std::string(" + param_name + "_str, " + param_name + "_len)";
        //} else {
        pexpr_string = param_name + "_str, " + param_name + "_len";
        //}
        expected_params += "str";
      }
      // Remember to clear the TypeError that any of the above methods raise.
      clear_error = true;
      only_pyobjects = false;

    } else if (TypeManager::is_vector_unsigned_char(type)) {
      indent(out, indent_level) << "unsigned char *" << param_name << "_str = nullptr;\n";
      indent(out, indent_level) << "Py_ssize_t " << param_name << "_len;\n";

      if (args_type == AT_single_arg) {
        extra_param_check << " && PyBytes_AsStringAndSize(arg, (char **)&"
          << param_name << "_str, &" << param_name << "_len) >= 0";
      } else {
        format_specifiers += "\" FMTCHAR_BYTES \"#";
        parameter_list += ", &" + param_name + "_str, &" + param_name + "_len";
      }

      pexpr_string = type->get_local_name(&parser);
      pexpr_string += "(" + param_name + "_str, " + param_name + "_str + " + param_name + "_len" + ")";
      expected_params += "bytes";

      // Remember to clear the TypeError that any of the above methods raise.
      clear_error = true;
      only_pyobjects = false;

    } else if (TypeManager::is_scoped_enum(type)) {
      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name;
        if (default_value != nullptr) {
          out << " = nullptr";
        }
        out << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }

      CPPEnumType *enum_type = (CPPEnumType *)TypeManager::unwrap(type);
      CPPType *underlying_type = enum_type->get_underlying_type();
      underlying_type = TypeManager::unwrap_const(underlying_type);

      //indent(out, indent_level);
      //underlying_type->output_instance(out, param_name + "_val", &parser);
      //out << default_expr << ";\n";
      extra_convert << "long " << param_name << "_val";

      if (default_value != nullptr) {
        extra_convert << " = (long)";
        default_value->output(extra_convert, 0, &parser, false);
        extra_convert <<
          ";\nif (" << param_name << " != nullptr) {\n"
          "  " << param_name << "_val = Dtool_EnumValue_AsLong(" + param_name + ");\n"
          "}";
      } else {
        extra_convert
          << ";\n"
          << param_name << "_val = Dtool_EnumValue_AsLong(" + param_name + ");\n";
      }

      pexpr_string = "(" + enum_type->get_local_name(&parser) + ")" + param_name + "_val";
      expected_params += classNameFromCppName(enum_type->get_simple_name(), false);
      extra_param_check << " && " << param_name << "_val != -1";
      clear_error = true;

    } else if (TypeManager::is_bool(type)) {
      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name;
        if (is_optional) {
          CPPExpression::Result res = default_value->evaluate();
          if (res._type != CPPExpression::RT_error) {
            // It's a compile-time constant.  Write Py_True or Py_False.
            out << " = " << (res.as_boolean() ? "Py_True" : "Py_False");
          } else {
            // Select Py_True or Py_False at runtime.
            out << " = (";
            default_value->output(out, 0, &parser, false);
            out << ") ? Py_True : Py_False";
          }
        }
        out << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      pexpr_string = "(PyObject_IsTrue(" + param_name + ") != 0)";
      expected_params += "bool";

    } else if (TypeManager::is_nullptr(type)) {
      if (args_type == AT_single_arg) {
        type_check = "arg == Py_None";
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << default_expr << ";\n";
        extra_param_check << " && " << param_name << " == Py_None";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      pexpr_string = "nullptr";
      expected_params += "NoneType";

    } else if (TypeManager::is_char(type)) {
      indent(out, indent_level) << "char *" << param_name << "_str;\n";
      indent(out, indent_level) << "Py_ssize_t " << param_name << "_len;\n";

      format_specifiers += "s#";
      parameter_list += ", &" + param_name + "_str, &" + param_name + "_len";
      extra_param_check << " && " << param_name << "_len == 1";

      pexpr_string = param_name + "_str[0]";
      expected_params += "char";
      only_pyobjects = false;

    } else if (TypeManager::is_wchar(type)) {
      out << "#if PY_VERSION_HEX >= 0x03020000\n";
      indent(out, indent_level) << "PyObject *" << param_name << ";\n";
      out << "#else\n";
      indent(out, indent_level) << "PyUnicodeObject *" << param_name << ";\n";
      out << "#endif\n";
      format_specifiers += "U";
      parameter_list += ", &" + param_name;

      // We tell it to copy 2 characters, but make sure it only copied one, as
      // a trick to check for the proper length in one go.
      extra_convert << "wchar_t " << param_name << "_chars[2];\n";
      extra_param_check << " && PyUnicode_AsWideChar(" << param_name << ", " << param_name << "_chars, 2) == 1";

      pexpr_string = param_name + "_chars[0]";
      expected_params += "unicode char";
      only_pyobjects = false;
      clear_error = true;

    } else if (TypeManager::is_ssize(type)) {
      indent(out, indent_level) << "Py_ssize_t " << param_name << default_expr << ";\n";
      format_specifiers += "n";
      parameter_list += ", &" + param_name;
      expected_params += "int";
      only_pyobjects = false;

    } else if (TypeManager::is_size(type)) {
      if (args_type == AT_single_arg) {
        type_check = "PyLongOrInt_Check(arg)";

        extra_convert <<
          "size_t arg_val = PyLongOrInt_AsSize_t(arg);\n"
          "#ifndef NDEBUG\n"
          "if (arg_val == (size_t)-1 && _PyErr_OCCURRED()) {\n";
        error_return(extra_convert, 2, return_flags);
        extra_convert <<
          "}\n"
          "#endif\n";

        pexpr_string = "arg_val";

      } else {
        // It certainly isn't the exact same thing as size_t, but Py_ssize_t
        // should at least be the same size.  The problem with mapping this to
        // unsigned int is that that doesn't work well on 64-bit systems, on
        // which size_t is a 64-bit integer.
        indent(out, indent_level) << "Py_ssize_t " << param_name << default_expr << ";\n";
        format_specifiers += "n";
        parameter_list += ", &" + param_name;

        extra_convert
          << "#ifndef NDEBUG\n"
          << "if (" << param_name << " < 0) {\n";

        error_raise_return(extra_convert, 2, return_flags, "OverflowError",
                          "can't convert negative value %zd to size_t",
                          param_name);
        extra_convert
          << "}\n"
          << "#endif\n";
      }
      expected_params += "int";
      only_pyobjects = false;

    } else if (TypeManager::is_longlong(type)) {
      // It's not trivial to do overflow checking for a long long, so we
      // simply don't do it.
      if (TypeManager::is_unsigned_longlong(type)) {
        indent(out, indent_level) << "unsigned PY_LONG_LONG " << param_name << default_expr << ";\n";
        format_specifiers += "K";
      } else {
        indent(out, indent_level) << "PY_LONG_LONG " << param_name << default_expr << ";\n";
        format_specifiers += "L";
      }
      parameter_list += ", &" + param_name;
      expected_params += "long";
      only_pyobjects = false;

    } else if (TypeManager::is_unsigned_short(type) ||
               TypeManager::is_unsigned_char(type) || TypeManager::is_signed_char(type)) {

      if (args_type == AT_single_arg) {
        type_check = "PyLongOrInt_Check(arg)";
        extra_convert
          << "long " << param_name << " = PyLongOrInt_AS_LONG(arg);\n";

        pexpr_string = "(" + type->get_local_name(&parser) + ")" + param_name;
      } else {
        indent(out, indent_level) << "long " << param_name << default_expr << ";\n";
        format_specifiers += "l";
        parameter_list += ", &" + param_name;
      }

      // The "H" format code, unlike "h", does not do overflow checking, so we
      // have to do it ourselves (except in release builds).
      extra_convert
        << "#ifndef NDEBUG\n";

      if (TypeManager::is_unsigned_short(type)) {
        extra_convert << "if (" << param_name << " < 0 || " << param_name << " > USHRT_MAX) {\n";
        error_raise_return(extra_convert, 2, return_flags, "OverflowError",
                           "value %ld out of range for unsigned short integer",
                           param_name);
      } else if (TypeManager::is_unsigned_char(type)) {
        extra_convert << "if (" << param_name << " < 0 || " << param_name << " > UCHAR_MAX) {\n";
        error_raise_return(extra_convert, 2, return_flags, "OverflowError",
                           "value %ld out of range for unsigned byte",
                           param_name);
      } else {
        extra_convert << "if (" << param_name << " < SCHAR_MIN || " << param_name << " > SCHAR_MAX) {\n";
        error_raise_return(extra_convert, 2, return_flags, "OverflowError",
                           "value %ld out of range for signed byte",
                           param_name);
      }

      extra_convert
        << "}\n"
        << "#endif\n";

      expected_params += "int";
      only_pyobjects = false;

    } else if (TypeManager::is_short(type)) {
      if (args_type == AT_single_arg) {
        type_check = "PyLongOrInt_Check(arg)";

        // Perform overflow checking in debug builds.
        extra_convert
          << "long arg_val = PyLongOrInt_AS_LONG(arg);\n"
          << "#ifndef NDEBUG\n"
          << "if (arg_val < SHRT_MIN || arg_val > SHRT_MAX) {\n";

        error_raise_return(extra_convert, 2, return_flags, "OverflowError",
                           "value %ld out of range for signed short integer",
                           "arg_val");
        extra_convert
          << "}\n"
          << "#endif\n";

        pexpr_string = "(" + type->get_local_name(&parser) + ")arg_val";

      } else {
        indent(out, indent_level) << "short " << param_name << default_expr << ";\n";
        format_specifiers += "h";
        parameter_list += ", &" + param_name;
      }
      expected_params += "int";
      only_pyobjects = false;

    } else if (TypeManager::is_unsigned_integer(type)) {
      if (args_type == AT_single_arg) {
        // Windows has 32-bit longs, and Python 2 stores a C long for PyInt
        // internally, so a PyInt wouldn't cover the whole range; that's why
        // we have to accept PyLong as well here.
        type_check = "PyLongOrInt_Check(arg)";
        extra_convert
          << "unsigned long " << param_name << " = PyLong_AsUnsignedLong(arg);\n";
        pexpr_string = "(" + type->get_local_name(&parser) + ")" + param_name;
      } else {
        indent(out, indent_level) << "unsigned long " << param_name << default_expr << ";\n";
        format_specifiers += "k";
        parameter_list += ", &" + param_name;
      }

      // The "I" format code, unlike "i", does not do overflow checking, so we
      // have to do it ourselves (in debug builds).  Note that Python 2 stores
      // longs internally, for ints, so we don't do it for Python 2 on
      // Windows, where longs are the same size as ints.  BUG: does not catch
      // negative values on Windows when going through the PyArg_ParseTuple
      // case.
      if (!TypeManager::is_long(type)) {
        extra_convert
          << "#if (SIZEOF_LONG > SIZEOF_INT) && !defined(NDEBUG)\n"
          << "if (" << param_name << " > UINT_MAX) {\n";

        error_raise_return(extra_convert, 2, return_flags, "OverflowError",
                           "value %lu out of range for unsigned integer",
                           param_name);

        extra_convert
          << "}\n"
          << "#endif\n";
      }
      expected_params += "int";
      only_pyobjects = false;

    } else if (TypeManager::is_long(type)) {
      // Signed longs are equivalent to Python's int type.
      if (args_type == AT_single_arg) {
        pexpr_string = "PyLongOrInt_AS_LONG(arg)";
        type_check = "PyLongOrInt_Check(arg)";
      } else {
        indent(out, indent_level) << "long " << param_name << default_expr << ";\n";
        format_specifiers += "l";
        parameter_list += ", &" + param_name;
      }
      expected_params += "int";
      only_pyobjects = false;

    } else if (TypeManager::is_integer(type)) {
      if (args_type == AT_single_arg) {
        type_check = "PyLongOrInt_Check(arg)";

        // Perform overflow checking in debug builds.  Note that Python 2
        // stores longs internally, for ints, so we don't do it on Windows,
        // where longs are the same size as ints.
        extra_convert
          << "long arg_val = PyLongOrInt_AS_LONG(arg);\n"
          << "#if (SIZEOF_LONG > SIZEOF_INT) && !defined(NDEBUG)\n"
          << "if (arg_val < INT_MIN || arg_val > INT_MAX) {\n";

        error_raise_return(extra_convert, 2, return_flags, "OverflowError",
                           "value %ld out of range for signed integer",
                           "arg_val");

        extra_convert
          << "}\n"
          << "#endif\n";

        pexpr_string = "(" + type->get_local_name(&parser) + ")arg_val";

      } else {
        indent(out, indent_level) << "int " << param_name << default_expr << ";\n";
        format_specifiers += "i";
        parameter_list += ", &" + param_name;
      }
      expected_params += "int";
      only_pyobjects = false;

    } else if (TypeManager::is_double(type)) {
      if (args_type == AT_single_arg) {
        pexpr_string = "PyFloat_AsDouble(arg)";
        type_check = "PyNumber_Check(arg)";
      } else {
        indent(out, indent_level) << "double " << param_name << default_expr << ";\n";
        format_specifiers += "d";
        parameter_list += ", &" + param_name;
      }
      expected_params += "double";
      only_pyobjects = false;

    } else if (TypeManager::is_float(type)) {
      if (args_type == AT_single_arg) {
        pexpr_string = "(" + type->get_local_name(&parser) + ")PyFloat_AsDouble(arg)";
        type_check = "PyNumber_Check(arg)";
      } else {
        indent(out, indent_level) << "float " << param_name << default_expr << ";\n";
        format_specifiers += "f";
        parameter_list += ", &" + param_name;
      }
      expected_params += "float";
      only_pyobjects = false;

    } else if (TypeManager::is_const_char_pointer(type)) {
      indent(out, indent_level) << "const char *" << param_name << default_expr << ";\n";
      format_specifiers += "z";
      parameter_list += ", &" + param_name;
      expected_params += "buffer";
      only_pyobjects = false;

    } else if (TypeManager::is_pointer_to_PyTypeObject(type)) {
      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << default_expr << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
        pexpr_string = param_name;
      }
      extra_param_check << " && PyType_Check(" << param_name << ")";
      pexpr_string = "(PyTypeObject *)" + param_name;
      expected_params += "type";

      // It's reasonable to assume that a function taking a PyTypeObject might
      // also throw a TypeError if the type is incorrect.
      may_raise_typeerror = true;

    } else if (TypeManager::is_pointer_to_PyStringObject(type)) {
      if (args_type == AT_single_arg) {
        // This is a single-arg function, so there's no need to convert
        // anything.
        param_name = "arg";
        type_check = "PyString_Check(arg)";
        pexpr_string = "(PyStringObject *)" + param_name;
      } else {
        indent(out, indent_level) << "PyStringObject *" << param_name << default_expr << ";\n";
        format_specifiers += "S";
        parameter_list += ", &" + param_name;
        pexpr_string = param_name;
      }
      expected_params += "string";

    } else if (TypeManager::is_pointer_to_PyUnicodeObject(type)) {
      if (args_type == AT_single_arg) {
        // This is a single-arg function, so there's no need to convert
        // anything.
        param_name = "arg";
        type_check = "PyUnicode_Check(arg)";
        pexpr_string = "(PyUnicodeObject *)" + param_name;
      } else {
        indent(out, indent_level) << "PyUnicodeObject *" << param_name << default_expr << ";\n";
        format_specifiers += "U";
        parameter_list += ", &" + param_name;
        pexpr_string = param_name;
      }
      expected_params += "unicode";

    } else if (TypeManager::is_pointer_to_PyObject(type)) {
      if (args_type == AT_single_arg) {
        // This is a single-arg function, so there's no need to convert
        // anything.
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << default_expr << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      pexpr_string = param_name;
      expected_params += "object";

      // It's reasonable to assume that a function taking a PyObject might
      // also throw a TypeError if the type is incorrect.
      may_raise_typeerror = true;

    } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
      min_version = 0x02060000; // Only support this remap in version 2.6+.

      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << null_assign << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      indent(out, indent_level) << "Py_buffer " << param_name << "_view;\n";

      if (is_optional) {
        indent(out, indent_level) << "Py_buffer *" << param_name << "_viewp;\n";

        extra_convert
          << "bool " << param_name << "_success;\n"
          << "if (" << param_name << " != nullptr) {\n"
          << "  " << param_name << "_success = (PyObject_GetBuffer("
          << param_name << ", &" << param_name << "_view, PyBUF_FULL) == 0);\n"
          << "  " << param_name << "_viewp = &" << param_name << "_view;\n"
          << "} else {\n"
          << "  " << param_name << "_viewp" << default_expr << ";\n"
          << "  " << param_name << "_success = true;\n"
          << "}\n";

        extra_param_check << " && " << param_name << "_success";
        pexpr_string = param_name + "_viewp";
        extra_cleanup << "if (" << param_name << " != nullptr) PyBuffer_Release(&" << param_name << "_view);\n";
      } else {
        extra_param_check << " && PyObject_GetBuffer("
                          << param_name << ", &"
                          << param_name << "_view, PyBUF_FULL) == 0";
        pexpr_string = "&" + param_name + "_view";
        extra_cleanup << "PyBuffer_Release(&" << param_name << "_view);\n";
      }
      expected_params += "buffer";
      may_raise_typeerror = true;
      clear_error = true;

    } else if (TypeManager::is_pointer_to_simple(type)) {
      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << null_assign << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }
      indent(out, indent_level) << "Py_buffer " << param_name << "_view;\n";

      // Unravel the type to determine its properties.
      int array_len = -1;
      bool is_const = true;
      CPPSimpleType *simple = nullptr;
      CPPType *unwrap = TypeManager::unwrap_const_reference(type);
      if (unwrap != nullptr) {
        CPPArrayType *array_type = unwrap->as_array_type();
        CPPPointerType *pointer_type = unwrap->as_pointer_type();

        if (array_type != nullptr) {
          if (array_type->_bounds != nullptr) {
            array_len = array_type->_bounds->evaluate().as_integer();
          }
          unwrap = array_type->_element_type;
        } else if (pointer_type != nullptr) {
          unwrap = pointer_type->_pointing_at;
        }

        CPPConstType *const_type = unwrap->as_const_type();
        if (const_type != nullptr) {
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
      clear_error = true;

    } else if (TypeManager::is_pointer(type)) {
      CPPType *obj_type = TypeManager::unwrap(TypeManager::resolve_type(type));
      bool const_ok = !TypeManager::is_non_const_pointer_or_ref(orig_type);

      if (TypeManager::is_const_pointer_or_ref(orig_type)) {
        expected_params += "const ";
      // } else { expected_params += "non-const ";
      }
      string expected_class_name = classNameFromCppName(obj_type->get_simple_name(), false);
      expected_params += expected_class_name;

      if (args_type == AT_single_arg) {
        param_name = "arg";
      } else {
        indent(out, indent_level) << "PyObject *" << param_name << null_assign << ";\n";
        format_specifiers += "O";
        parameter_list += ", &" + param_name;
      }

      // If the default value is NULL, we also accept a None value.
      bool maybe_none = false;
      if (default_value != nullptr && (return_flags & RF_coerced) == 0 &&
          TypeManager::is_pointer(orig_type)) {
        CPPExpression::Result res = param->get_default_value()->evaluate();
        if (res._type == CPPExpression::RT_integer ||
            res._type == CPPExpression::RT_pointer) {
          if (res.as_integer() == 0) {
            maybe_none = true;
          }
        }
      }

      string class_name = obj_type->get_local_name(&parser);

      // need to a forward scope for this class..
      if (!isExportThisRun(obj_type)) {
        _external_imports.insert(TypeManager::resolve_type(obj_type));
      }

      string this_class_name;
      string method_prefix;
      if (remap->_cpptype) {
        this_class_name = remap->_cpptype->get_simple_name();
        method_prefix = classNameFromCppName(this_class_name, false) + string(".");
      }

      if (coercion_possible &&
          has_coerce_constructor(obj_type->as_struct_type())) {
        // Call the coercion function directly, which will try to extract the
        // pointer directly before trying coercion.
        string coerce_call;

        if (TypeManager::is_reference_count(obj_type)) {
          // We use a PointerTo to handle the management here.  It's cleaner
          // that way.
          if (default_expr == " = 0" || default_expr == " = nullptr") {
            default_expr.clear();
          }
          if (TypeManager::is_const_pointer_to_anything(type)) {
            extra_convert
              << "CPT(" << class_name << ") " << param_name << "_this"
              << default_expr << ";\n";

            coerce_call = "Dtool_ConstCoerce_" + make_safe_name(class_name) +
              "(" + param_name + ", " + param_name + "_this)";
          } else {
            extra_convert
              << "PT(" << class_name << ") " << param_name << "_this"
              << default_expr << ";\n";

            coerce_call = "Dtool_Coerce_" + make_safe_name(class_name) +
              "(" + param_name + ", " + param_name + "_this)";
          }

          // Use move constructor when available for functions that take an
          // actual PointerTo.  This eliminates an unref()ref() pair.
          pexpr_string = "std::move(" + param_name + "_this)";

        } else {
          // This is a move-assignable type, such as TypeHandle or LVecBase4.
          obj_type->output_instance(extra_convert, param_name + "_local", &parser);
          extra_convert << ";\n";

          type->output_instance(extra_convert, param_name + "_this", &parser);

          if (is_optional && maybe_none) {
            extra_convert
              << default_expr << ";\n"
              << "if (" << param_name << " != nullptr && " << param_name << " != Py_None) {\n"
              << "  " << param_name << "_this";
          } else if (is_optional) {
            if (TypeManager::is_pointer(orig_type)) {
              extra_convert << default_expr;
            }
            extra_convert
              << ";\n"
              << "if (" << param_name << " != nullptr) {\n"
              << "  " << param_name << "_this";
          } else if (maybe_none) {
            extra_convert
              << " = nullptr;\n"
              << "if (" << param_name << " != Py_None) {\n"
              << "  " << param_name << "_this";
          }

          extra_convert << " = Dtool_Coerce_" + make_safe_name(class_name) +
            "(" + param_name + ", " + param_name + "_local);\n";

          if (is_optional && !TypeManager::is_pointer(orig_type)) {
            extra_convert
              << "} else {\n"
              << "  " << param_name << "_local" << default_expr << ";\n"
              << "  " << param_name << "_this = &" << param_name << "_local;\n"
              << "}\n";
          } else if (is_optional || maybe_none) {
            extra_convert << "}\n";
          }

          coerce_call = "(" + param_name + "_this != nullptr)";

          pexpr_string = param_name + "_this";
        }

        if (report_errors) {
          // We were asked to report any errors.  Let's do it.
          if (is_optional && maybe_none) {
            extra_convert << "if (" << param_name << " != nullptr && " << param_name << " != Py_None && !" << coerce_call << ") {\n";
          } else if (is_optional) {
            extra_convert << "if (" << param_name << " != nullptr && !" << coerce_call << ") {\n";
          } else if (maybe_none) {
            extra_convert << "if (" << param_name << " != Py_None && !" << coerce_call << ") {\n";
          } else {
            extra_convert << "if (!" << coerce_call << ") {\n";
          }

          // Display error like: Class.func() argument 0 must be A, not B
          if ((return_flags & ~RF_pyobject) == RF_err_null) {
            // Dtool_Raise_ArgTypeError returns NULL already
            extra_convert << "  return ";
          } else {
            extra_convert << "  ";
          }
          extra_convert
            << "Dtool_Raise_ArgTypeError(" << param_name << ", "
            << pn << ", \"" << method_prefix
            << methodNameFromCppName(remap, this_class_name, false)
            << "\", \"" << expected_class_name << "\");\n";

          if ((return_flags & ~RF_pyobject) != RF_err_null) {
            error_return(extra_convert, 2, return_flags);
          }
          extra_convert << "}\n";

        } else if (is_optional && maybe_none) {
          extra_param_check << " && (" << param_name << " == nullptr || " << param_name << " == Py_None || " << coerce_call << ")";

        } else if (is_optional) {
          extra_param_check << " && (" << param_name << " == nullptr || " << coerce_call << ")";

        } else if (maybe_none) {
          extra_param_check << " && (" << param_name << " == Py_None || " << coerce_call << ")";

        } else {
          extra_param_check << " && " << coerce_call;
        }

      } else { // The regular, non-coercion case.
        type->output_instance(extra_convert, param_name + "_this", &parser);
        if (is_optional && maybe_none) {
          // This parameter has a default value of nullptr, so we need to also
          // allow passing in None.
          extra_convert
            << default_expr << ";\n"
            << "if (" << param_name << " != nullptr && " << param_name << " != Py_None) {\n"
            << "  " << param_name << "_this";
        }
        else if (is_optional && !TypeManager::is_pointer(orig_type) && !default_value->is_lvalue()) {
          // Most annoying case, where we have to use an rvalue reference to
          // extend the lifetime of the default argument.  In this case, the
          // default expression is invoked even if not used.
          extra_convert << ";\n";
          if (TypeManager::is_const_pointer_to_anything(type)) {
            extra_convert << "const ";
            obj_type->output_instance(extra_convert, "&" + param_name + "_ref", &parser);
          } else {
            obj_type->output_instance(extra_convert, "&&" + param_name + "_ref", &parser);
          }
          extra_convert
            << default_expr << ";\n"
            << "if (" << param_name << " == nullptr) {\n"
            << "  " << param_name << "_this = &" << param_name << "_ref;\n"
            << "} else {\n"
            << "  " << param_name << "_this";
        }
        else if (is_optional) {
          // General case where the default argument is either an lvalue or a
          // pointer.
          extra_convert
            << ";\n"
            << "if (" << param_name << " == nullptr) {\n"
            << "  " << param_name << "_this = ";
          if (TypeManager::is_pointer(orig_type)) {
            default_value->output(extra_convert, 0, &parser, false);
            extra_convert << ";\n";
          } else {
            // The rvalue case was handled above, so this is an lvalue, which
            // means we can safely take a reference to it.
            extra_convert << "&(";
            default_value->output(extra_convert, 0, &parser, false);
            extra_convert << ");\n";
          }
          extra_convert
            << "} else {\n"
            << "  " << param_name << "_this";
        }
        else if (maybe_none) {
          // No default argument, but we still need to check for None.
          extra_convert
            << " = nullptr;\n"
            << "if (" << param_name << " != Py_None) {\n"
            << "  " << param_name << "_this";
        }

        if (const_ok && !report_errors) {
          // This function does the same thing in this case and is slightly
          // simpler.  But maybe we should just reorganize these functions
          // entirely?
          extra_convert << " = nullptr;\n";
          int indent_level = (is_optional || maybe_none) ? 2 : 0;
          indent(extra_convert, indent_level)
            << "DtoolInstance_GetPointer(" << param_name
            << ", " << param_name << "_this"
            << ", *Dtool_Ptr_" << make_safe_name(class_name)
            << ");\n";
        } else {
          extra_convert << std::boolalpha
            << " = (" << class_name << " *)"
            << "DTOOL_Call_GetPointerThisClass(" << param_name
            << ", Dtool_Ptr_" << make_safe_name(class_name)
            << ", " << pn << ", \""
            << method_prefix << methodNameFromCppName(remap, this_class_name, false)
            << "\", " << const_ok << ", " << report_errors << ");\n";
        }

        if (is_optional && maybe_none) {
          extra_convert << "}\n";
          extra_param_check << " && (" << param_name << " == nullptr || " << param_name << " == Py_None || " << param_name << "_this != nullptr)";
        } else if (is_optional) {
          extra_convert << "}\n";
          extra_param_check << " && (" << param_name << " == nullptr || " << param_name << "_this != nullptr)";
        } else if (maybe_none) {
          extra_convert << "}\n";
          extra_param_check << " && (" << param_name << " == Py_None || " << param_name << "_this != nullptr)";
        } else {
          extra_param_check << " && " << param_name << "_this != nullptr";
        }

        pexpr_string = param_name + "_this";
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
    }

    if (!reported_name.empty()) {
      expected_params += " " + reported_name;
    }
    pexprs.push_back(pexpr_string);
  }
  expected_params += ")\n";

  if (min_version > 0) {
    out << "#if PY_VERSION_HEX >= 0x" << hex << min_version << dec << "\n";
  }

  // Track how many curly braces we've opened.
  short open_scopes = 0;

  if (!type_check.empty() && args_type == AT_single_arg) {
    indent(out, indent_level)
      << "if (" << type_check << ") {\n";

    ++open_scopes;
    indent_level += 2;

  } else if (!format_specifiers.empty()) {
    string method_name = methodNameFromCppName(remap, "", false);

    switch (args_type) {
    case AT_keyword_args:
      // Wrapper takes a varargs tuple and a keyword args dict.
      if (has_keywords) {
        if (only_pyobjects && max_num_args == 1) {
          // But we are only expecting one object arg, which is an easy common
          // case we have implemented ourselves.
          if (min_num_args == 1) {
            indent(out, indent_level)
              << "if (Dtool_ExtractArg(&" << param_name << ", args, kwds, " << keyword_list << ")) {\n";
          } else {
            indent(out, indent_level)
              << "if (Dtool_ExtractOptionalArg(&" << param_name << ", args, kwds, " << keyword_list << ")) {\n";
          }
        } else {
          // We have to use the more expensive PyArg_ParseTupleAndKeywords.
          clear_error = true;
          indent(out, indent_level)
            << "static const char *keyword_list[] = {" << keyword_list << ", nullptr};\n";
          indent(out, indent_level)
            << "if (PyArg_ParseTupleAndKeywords(args, kwds, \""
            << format_specifiers << ":" << method_name
            << "\", (char **)keyword_list" << parameter_list << ")) {\n";
        }

      } else if (only_pyobjects) {
        // This function actually has no named parameters, so let's not take
        // any keyword arguments.
        if (max_num_args == 1) {
          if (min_num_args == 1) {
            indent(out, indent_level)
              << "if (Dtool_ExtractArg(&" << param_name << ", args, kwds)) {\n";
          } else {
            indent(out, indent_level)
              << "if (Dtool_ExtractOptionalArg(&" << param_name << ", args, kwds)) {\n";
          }
        } else if (max_num_args == 0) {
          indent(out, indent_level)
            << "if (Dtool_CheckNoArgs(args, kwds)) {\n";
        } else {
          clear_error = true;
          indent(out, indent_level)
            << "if ((kwds == nullptr || PyDict_Size(kwds) == 0) && PyArg_UnpackTuple(args, \""
            << methodNameFromCppName(remap, "", false)
            << "\", " << min_num_args << ", " << max_num_args
            << parameter_list << ")) {\n";
        }

      } else {
        clear_error = true;
        indent(out, indent_level)
          << "if ((kwds == nullptr || PyDict_Size(kwds) == 0) && PyArg_ParseTuple(args, \""
          << format_specifiers << ":" << method_name
          << "\"" << parameter_list << ")) {\n";
      }

      ++open_scopes;
      indent_level += 2;
      break;

    case AT_varargs:
      // Wrapper takes a varargs tuple.
      if (only_pyobjects) {
        // All parameters are PyObject*, so we can use the slightly more
        // efficient PyArg_UnpackTuple function instead.
        if (min_num_args == 1 && max_num_args == 1) {
          indent(out, indent_level)
            << "if (PyTuple_GET_SIZE(args) == 1) {\n";
          indent(out, indent_level + 2)
            << param_name << " = PyTuple_GET_ITEM(args, 0);\n";
        } else {
          clear_error = true;
          indent(out, indent_level)
            << "if (PyArg_UnpackTuple(args, \""
            << methodNameFromCppName(remap, "", false)
            << "\", " << min_num_args << ", " << max_num_args
            << parameter_list << ")) {\n";
        }

      } else {
        clear_error = true;
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
        clear_error = true;
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

  if (is_constructor && !remap->_has_this &&
      (remap->_flags & FunctionRemap::F_explicit_self) != 0) {
    // If we'll be passing "self" to the constructor, we need to pre-
    // initialize it here.  Unfortunately, we can't pre-load the "this"
    // pointer, but the constructor itself can do this.

    CPPType *orig_type = remap->_return_type->get_orig_type();
    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)), false);
    const InterrogateType &itype = idb->get_type(type_index);

    indent(out, indent_level)
      << "// Pre-initialize self for the constructor\n";

    if (!is_constructor || (return_flags & RF_int) == 0) {
      // This is not a constructor, but somehow we landed up here at a static
      // method requiring a 'self' pointer.  This happens in coercion
      // constructors in particular.  We'll have to create a temporary
      // PyObject instance to pass to it.

      indent(out, indent_level)
        << "PyObject *self = Dtool_new_"
        << make_safe_name(itype.get_scoped_name()) << "(&"
        << CLASS_PREFIX << make_safe_name(itype.get_scoped_name())
        << "._PyType, nullptr, nullptr);\n";

      extra_cleanup << "PyObject_Del(self);\n";
    } else {
      // XXX rdb: this isn't needed, is it, because tp_new already initializes
      // the instance?
      indent(out, indent_level)
        << "DTool_PyInit_Finalize(self, nullptr, &"
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

  // If the function returns a pointer that we may need to manage, we store it
  // in a temporary return_value variable and set this to true.
  bool manage_return = false;

  if (remap->_return_type->new_type_is_atomic_string()) {
    // Treat strings as a special case.  We don't want to format the return
    // expression.
    return_expr = remap->call_function(out, indent_level, false, container, pexprs);
    CPPType *type = remap->_return_type->get_orig_type();
    indent(out, indent_level);
    type->output_instance(out, "return_value", &parser);
    out << " = " << return_expr << ";\n";
    manage_return = remap->_return_value_needs_management;
    return_expr = "return_value";

  } else if ((return_flags & RF_coerced) != 0 && !TypeManager::is_reference_count(remap->_cpptype)) {
    // Another special case is the coerce constructor for a trivial type.  We
    // don't want to invoke "operator new" unnecessarily.
    if (is_constructor && remap->_extension) {
      // Extension constructors are a special case, as usual.
      indent(out, indent_level)
        << remap->get_call_str("&coerced", pexprs) << ";\n";

    } else {
      indent(out, indent_level)
        << "coerced = " << remap->get_call_str(container, pexprs) << ";\n";
    }
    return_expr = "&coerced";

  } else {
    // The general case; an ordinary constructor or function.
    return_expr = remap->call_function(out, indent_level, true, container, pexprs);

    if (return_flags & RF_self) {
      // We won't be using the return value, anyway.
      return_expr.clear();
    }

    if (!return_expr.empty()) {
      manage_return = remap->_return_value_needs_management;
      CPPType *type = remap->_return_type->get_temporary_type();
      indent(out, indent_level);
      type->output_instance(out, "return_value", &parser);
      out << " = " << return_expr << ";\n";
      return_expr = "return_value";
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

  if (manage_return) {
    // If a constructor returns NULL, that means allocation failed.
    if (remap->_return_type->return_value_needs_management()) {
      indent(out, indent_level) << "if (return_value == nullptr) {\n";
      if ((return_flags & ~RF_pyobject) == RF_err_null) {
        // PyErr_NoMemory returns NULL, so allow tail call elimination.
        indent(out, indent_level) << "  return PyErr_NoMemory();\n";
      } else {
        indent(out, indent_level) << "  PyErr_NoMemory();\n";
        error_return(out, indent_level + 2, return_flags);
      }
      indent(out, indent_level) << "}\n";
    }

    if (TypeManager::is_pointer_to_PyObject(remap->_return_type->get_orig_type())) {
      indent(out, indent_level) << "Py_XINCREF(return_value);\n";
    } else {
      return_expr = manage_return_value(out, indent_level, remap, "return_value");
    }
    return_expr = remap->_return_type->temporary_to_return(return_expr);
  }

  // How could we raise a TypeError if we don't take any args?
  if (args_type == AT_no_args || max_num_args == 0) {
    may_raise_typeerror = false;
  }

  // If a function takes a PyObject* argument, it would be a good idea to
  // always check for exceptions.
  if (may_raise_typeerror) {
    check_exceptions = true;
  }

  // Generated getters and setters don't raise exceptions or asserts since
  // they don't contain any code.
  if (remap->_type == FunctionRemap::T_getter ||
      remap->_type == FunctionRemap::T_setter) {
    check_exceptions = false;
  }

  // The most common case of the below logic is consolidated in a single
  // function, as another way to reduce code bloat.  Sigh.
  if (check_exceptions && (!may_raise_typeerror || report_errors) &&
      watch_asserts && (return_flags & RF_coerced) == 0) {

    if (return_flags & RF_decref_args) {
      indent(out, indent_level) << "Py_DECREF(args);\n";
      return_flags &= ~RF_decref_args;
    }

    // An even specialer special case for functions with void return or bool
    // return.  We have our own functions that do all this in a single
    // function call, so it should reduce the amount of code output while not
    // being any slower.
    bool return_null = (return_flags & RF_pyobject) != 0 &&
                       (return_flags & RF_err_null) != 0;
    if (return_null && return_expr.empty()) {
      indent(out, indent_level)
        << "return Dtool_Return_None();\n";

      // Reset the return value bit so that the code below doesn't generate
      // the return statement a second time.
      return_flags &= ~RF_pyobject;

    } else if (return_null && TypeManager::is_bool(remap->_return_type->get_new_type())) {
      indent(out, indent_level)
        << "return Dtool_Return_Bool(" << return_expr << ");\n";
      return_flags &= ~RF_pyobject;

    } else if (return_null && TypeManager::is_pointer_to_PyObject(remap->_return_type->get_new_type())) {
      indent(out, indent_level)
        << "return Dtool_Return(" << return_expr << ");\n";
      return_flags &= ~RF_pyobject;

    } else {
      indent(out, indent_level)
        << "if (Dtool_CheckErrorOccurred()) {\n";

      if (manage_return) {
        delete_return_value(out, indent_level + 2, remap, return_expr);
      }
      error_return(out, indent_level + 2, return_flags);

      indent(out, indent_level) << "}\n";
    }

  } else {
    if (check_exceptions) {
      // Check if a Python exception has occurred.  We only do this when
      // check_exception is set.  If report_errors is set, this method must
      // terminate on error.
      if (!may_raise_typeerror || report_errors) {
        indent(out, indent_level)
          << "if (_PyErr_OCCURRED()) {\n";
      } else {
        // If a method is some extension method that takes a PyObject*, and it
        // raised a TypeError, continue.  The documentation tells us not to
        // compare the result of PyErr_Occurred against a specific exception
        // type.  However, in our case, this seems okay because we know that
        // the TypeError we want to catch here is going to be generated by a
        // PyErr_SetString call, not by user code.
        indent(out, indent_level)
          << "PyObject *exception = _PyErr_OCCURRED();\n";
        indent(out, indent_level)
          << "if (exception == PyExc_TypeError) {\n";
        indent(out, indent_level)
          << "  // TypeError raised; continue to next overload type.\n";
        indent(out, indent_level)
          << "} else if (exception != nullptr) {\n";
      }

      if (manage_return) {
        delete_return_value(out, indent_level + 2, remap, return_expr);
      }

      error_return(out, indent_level + 2, return_flags);

      indent(out, indent_level)
        << "} else {\n";

      ++open_scopes;
      indent_level += 2;
    }

    if (return_flags & RF_decref_args) {
      indent(out, indent_level) << "Py_DECREF(args);\n";
      return_flags &= ~RF_decref_args;
    }

    // Outputs code to check to see if an assertion has failed while the C++
    // code was executing, and report this failure back to Python.  Don't do
    // this for coercion constructors since they are called by other wrapper
    // functions which already check this on their own.  Generated getters
    // obviously can't raise asserts.
    if (watch_asserts && (return_flags & (RF_coerced | RF_raise_keyerror)) == 0 &&
        remap->_type != FunctionRemap::T_getter &&
        remap->_type != FunctionRemap::T_setter) {
      out << "#ifndef NDEBUG\n";
      indent(out, indent_level)
        << "Notify *notify = Notify::ptr();\n";
      indent(out, indent_level)
        << "if (UNLIKELY(notify->has_assert_failed())) {\n";

      if (manage_return) {
        // Output code to delete any temporary object we may have allocated.
        delete_return_value(out, indent_level + 2, remap, return_expr);
      }

      if (return_flags & RF_err_null) {
        // This function returns NULL, so we can pass it on.
        indent(out, indent_level + 2)
          << "return Dtool_Raise_AssertionError();\n";
      } else {
        indent(out, indent_level + 2)
          << "Dtool_Raise_AssertionError();\n";
        error_return(out, indent_level + 2, return_flags);
      }

      indent(out, indent_level)
        << "}\n";
      out << "#endif\n";
    }
  }

  // Okay, we're past all the error conditions and special cases.  Now return
  // the return type in the way that was requested.
  if ((return_flags & RF_int) != 0 && (return_flags & RF_raise_keyerror) == 0) {
    CPPType *orig_type = remap->_return_type->get_orig_type();
    if (is_constructor) {
      // Special case for constructor.
      TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)), false);
      const InterrogateType &itype = idb->get_type(type_index);
      indent(out, indent_level)
        << "return DTool_PyInit_Finalize(self, (void *)" << return_expr << ", &" << CLASS_PREFIX << make_safe_name(itype.get_scoped_name()) << ", true, false);\n";

    } else if (TypeManager::is_bool(orig_type)) {
      // It's an error return boolean, I guess.  Return 0 on success.
      indent(out, indent_level) << "return (" << return_expr << ") ? 0 : -1;\n";

    } else if (TypeManager::is_integer(orig_type)) {
      if ((return_flags & RF_compare) == RF_compare) {
        // Make sure it returns -1, 0, or 1, or Python complains with:
        // RuntimeWarning: tp_compare didn't return -1, 0 or 1
        indent(out, indent_level) << "return (int)(" << return_expr << " > 0) - (int)(" << return_expr << " < 0);\n";
      } else {
        indent(out, indent_level) << "return " << return_expr << ";\n";
      }

    } else if (TypeManager::is_void(orig_type)) {
      indent(out, indent_level) << "return 0;\n";

    } else {
      nout << "Warning: function has return type " << *orig_type
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

    } else if (return_flags & RF_preserve_null) {
      indent(out, indent_level) << "if (" << return_expr << " == nullptr) {\n";
      indent(out, indent_level) << "  return nullptr;\n";
      indent(out, indent_level) << "} else {\n";
      pack_return_value(out, indent_level + 2, remap, return_expr, return_flags);
      indent(out, indent_level) << "}\n";

    } else {
      pack_return_value(out, indent_level, remap, return_expr, return_flags);
    }

  } else if (return_flags & RF_coerced) {
    // We were asked to assign the result to a "coerced" reference.
    CPPType *return_type = remap->_cpptype;
    CPPType *orig_type = remap->_return_type->get_orig_type();

    // Special case for static make function that returns a pointer: cast the
    // pointer to the right pointer type.
    if (!is_constructor && (remap->_flags & FunctionRemap::F_coerce_constructor) != 0 &&
        (TypeManager::is_pointer(orig_type) || TypeManager::is_pointer_to_base(orig_type))) {

      CPPType *new_type = remap->_return_type->get_new_type();

      if (TypeManager::is_const_pointer_to_anything(new_type)) {
        return_type = CPPType::new_type(new CPPConstType(return_type));
      }

      if (IsPandaTypedObject(return_type->as_struct_type())) {
        return_expr = "DCAST("
          + return_type->get_local_name(&parser)
          + ", " + return_expr + ")";

      } else {
        return_type = CPPType::new_type(new CPPPointerType(return_type));
        return_expr = "(" + return_type->get_local_name(&parser) +
                      ") " + return_expr;
      }
    }

    if (return_expr == "coerced") {
      // We already did this earlier...
      indent(out, indent_level) << "return true;\n";

    } else if (TypeManager::is_reference_count(remap->_cpptype)) {
      indent(out, indent_level) << "coerced = std::move(" << return_expr << ");\n";
      indent(out, indent_level) << "return true;\n";

    } else {
      indent(out, indent_level) << "return &coerced;\n";
    }

  } else if (return_flags & RF_raise_keyerror) {
    CPPType *orig_type = remap->_return_type->get_orig_type();

    if (TypeManager::is_bool(orig_type) || TypeManager::is_pointer(orig_type)) {
      indent(out, indent_level) << "if (!" << return_expr << ") {\n";
    } else if (TypeManager::is_unsigned_integer(orig_type)) {
      indent(out, indent_level) << "if ((int)" << return_expr << " == -1) {\n";
    } else if (TypeManager::is_integer(orig_type)) {
      indent(out, indent_level) << "if (" << return_expr << " < 0) {\n";
    } else {
      indent(out, indent_level) << "if (false) {\n";
    }

    if (args_type == AT_single_arg) {
      indent(out, indent_level) << "  PyErr_SetObject(PyExc_KeyError, arg);\n";
    } else {
      indent(out, indent_level) << "  PyErr_SetObject(PyExc_KeyError, key);\n";
    }
    error_return(out, indent_level + 2, return_flags);
    indent(out, indent_level) << "}\n";
  }

  // Close the extra braces opened earlier.
  while (open_scopes > 0) {
    indent_level -= 2;
    indent(out, indent_level) << "}\n";

    --open_scopes;
  }

  if (clear_error && !report_errors) {
    // We were asked not to report errors, so clear the active exception if
    // this overload might have raised a TypeError.
    indent(out, indent_level) << "PyErr_Clear();\n";
  }

  if (min_version > 0) {
    // Close the #if PY_VERSION_HEX check.
    out << "#endif\n";
  }
}

/**
 * Outputs the correct return statement that should be used in case of error
 * based on the ReturnFlags.
 */
void InterfaceMakerPythonNative::
error_return(ostream &out, int indent_level, int return_flags) {
  // if (return_flags & RF_coerced) { indent(out, indent_level) << "coerced =
  // NULL;\n"; }

  if (return_flags & RF_decref_args) {
    indent(out, indent_level) << "Py_DECREF(args);\n";
  }

  if (return_flags & RF_int) {
    indent(out, indent_level) << "return -1;\n";

  } else if (return_flags & RF_err_notimplemented) {
    indent(out, indent_level) << "Py_INCREF(Py_NotImplemented);\n";
    indent(out, indent_level) << "return Py_NotImplemented;\n";

  } else if (return_flags & RF_err_null) {
    indent(out, indent_level) << "return nullptr;\n";

  } else if (return_flags & RF_err_false) {
    indent(out, indent_level) << "return false;\n";
  }
}

/**
 * Similar to error_return, except raises an exception before returning.  If
 * format_args are not the empty string, uses PyErr_Format instead of
 * PyErr_SetString.
 */
void InterfaceMakerPythonNative::
error_raise_return(ostream &out, int indent_level, int return_flags,
                   const string &exc_type, const string &message,
                   const string &format_args) {

  if (return_flags & RF_decref_args) {
    indent(out, indent_level) << "Py_DECREF(args);\n";
    return_flags &= ~RF_decref_args;
  }

  if (format_args.empty()) {
    if (exc_type == "TypeError") {
      if ((return_flags & RF_err_null) != 0) {
        // This is probably an over-optimization, but why the heck not.
        indent(out, indent_level) << "return Dtool_Raise_TypeError(";
        output_quoted(out, indent_level + 29, message, false);
        out << ");\n";
        return;
      } else {
        indent(out, indent_level) << "Dtool_Raise_TypeError(";
        output_quoted(out, indent_level + 22, message, false);
        out << ");\n";
      }
    } else {
      indent(out, indent_level) << "PyErr_SetString(PyExc_" << exc_type << ",\n";
      output_quoted(out, indent_level + 16, message);
      out << ");\n";
    }

  } else if ((return_flags & RF_err_null) != 0 &&
             (return_flags & RF_pyobject) != 0) {
    // PyErr_Format always returns NULL.  Passing it on directly allows the
    // compiler to make a tiny optimization, so why not.
    indent(out, indent_level) << "return PyErr_Format(PyExc_" << exc_type << ",\n";
    output_quoted(out, indent_level + 20, message);
    out << ",\n";
    indent(out, indent_level + 20) << format_args << ");\n";
    return;

  } else {
    indent(out, indent_level) << "PyErr_Format(PyExc_" << exc_type << ",\n";
    output_quoted(out, indent_level + 13, message);
    out << ",\n";
    indent(out, indent_level + 13) << format_args << ");\n";
  }

  error_return(out, indent_level, return_flags);
}

/**
 * Outputs a command to pack the indicated expression, of the return_type
 * type, as a Python return value.
 */
void InterfaceMakerPythonNative::
pack_return_value(ostream &out, int indent_level, FunctionRemap *remap,
                  string return_expr, int return_flags) {

  ParameterRemap *return_type = remap->_return_type;
  CPPType *orig_type = return_type->get_orig_type();
  CPPType *type = return_type->get_new_type();

  if (TypeManager::is_scoped_enum(type)) {
    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
    TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)), false);
    const InterrogateType &itype = idb->get_type(type_index);
    string safe_name = make_safe_name(itype.get_scoped_name());

    indent(out, indent_level)
      << "return PyObject_CallFunction((PyObject *)Dtool_Ptr_" << safe_name;

    CPPType *underlying_type = ((CPPEnumType *)itype._cpptype)->get_underlying_type();
    if (TypeManager::is_unsigned_integer(underlying_type)) {
      out << ", \"k\", (unsigned long)";
    } else {
      out << ", \"l\", (long)";
    }
    out << "(" << return_expr << "));\n";

  } else if (return_type->new_type_is_atomic_string() ||
      TypeManager::is_simple(type) ||
      TypeManager::is_char_pointer(type) ||
      TypeManager::is_wchar_pointer(type) ||
      TypeManager::is_pointer_to_PyObject(type) ||
      TypeManager::is_pointer_to_Py_buffer(type) ||
      TypeManager::is_vector_unsigned_char(type)) {
    // Most types are now handled by the many overloads of Dtool_WrapValue,
    // defined in py_panda.h.
    indent(out, indent_level)
      << "return Dtool_WrapValue(" << return_expr << ");\n";

  } else if (TypeManager::is_pointer(type)) {
    bool is_const = TypeManager::is_const_pointer_to_anything(type);
    bool owns_memory = remap->_return_value_needs_management;

    // Note, we don't check for NULL here any more.  This is now done by the
    // appropriate CreateInstance(Typed) function.

    if (manage_reference_counts && TypeManager::is_pointer_to_base(orig_type)) {
      // Use a trick to transfer the reference count to avoid a pair of
      // unnecessary ref() and unref() calls.  Ideally we'd use move
      // semantics, but py_panda.cxx cannot make use of PointerTo.
      indent(out, indent_level) << "// Transfer ownership of return_value.\n";
      indent(out, indent_level);
      type->output_instance(out, "return_ptr", &parser);
      out << " = " << return_expr << ";\n";
      indent(out, indent_level) << "return_value.cheat() = nullptr;\n";
      return_expr = "return_ptr";
    }

    InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

    if (TypeManager::is_struct(orig_type) || TypeManager::is_ref_to_anything(orig_type)) {
      if (TypeManager::is_ref_to_anything(orig_type) || remap->_manage_reference_count) {
        TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(type)),false);
        const InterrogateType &itype = idb->get_type(type_index);

        write_python_instance(out, indent_level, return_expr, owns_memory, itype, is_const);

      } else {
        TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
        const InterrogateType &itype = idb->get_type(type_index);

        write_python_instance(out, indent_level, return_expr, owns_memory, itype, is_const);
      }
    } else if (TypeManager::is_struct(orig_type->remove_pointer())) {
      TypeIndex type_index = builder.get_type(TypeManager::unwrap(TypeManager::resolve_type(orig_type)),false);
      const InterrogateType &itype = idb->get_type(type_index);

      write_python_instance(out, indent_level, return_expr, owns_memory, itype, is_const);

    } else {
      indent(out, indent_level) << "Should Never Reach This InterfaceMakerPythonNative::pack_python_value";
          // << "return Dtool_Integer((int) " << return_expr << ");\n";
    }

  } else {
    // Return None.
    indent(out, indent_level)
      << "return Py_BuildValue(\"\"); // Don't know how to wrap type.\n";
  }
}

/**
 * Generates the synthetic method described by the MAKE_SEQ() macro.
 */
void InterfaceMakerPythonNative::
write_make_seq(ostream &out, Object *obj, const std::string &ClassName,
               const std::string &cClassName, MakeSeq *make_seq) {

  out << "/*\n"
         " * Python make_seq wrapper\n"
         " */\n";

  out << "static PyObject *" << make_seq->_name + "(PyObject *self, PyObject *) {\n";

  // This used to return a list.  But it should really be a tuple, I think,
  // because it probably makes more sense for it to be immutable (as changes
  // to it won't be visible on the C++ side anyway).

  FunctionRemap *remap = make_seq->_length_getter->_remaps.front();
  vector_string pexprs;

  if (make_seq->_length_getter->_has_this) {
    out <<
      "  " << cClassName  << " *local_this = nullptr;\n"
      "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n"
      "    return nullptr;\n"
      "  }\n"
      "  Py_ssize_t count = (Py_ssize_t)" << remap->get_call_str("local_this", pexprs) << ";\n";
  } else {
    out << "  Py_ssize_t count = (Py_ssize_t)" << remap->get_call_str("", pexprs) << ";\n";
  }

  Function *elem_getter = make_seq->_element_getter;

  if ((elem_getter->_args_type & AT_varargs) == AT_varargs) {
    // Fast way to create a temporary tuple to hold only a single item, under
    // the assumption that the called method doesn't do anything with this
    // tuple other than unpack it (which is a fairly safe assumption to make).
    out << "  PyTupleObject args;\n";
    out << "  (void)PyObject_INIT_VAR((PyVarObject *)&args, &PyTuple_Type, 1);\n";
  }

  out <<
    "  PyObject *tuple = PyTuple_New(count);\n"
    "\n"
    "  for (Py_ssize_t i = 0; i < count; ++i) {\n"
    "    PyObject *index = Dtool_WrapValue(i);\n";

  switch (elem_getter->_args_type) {
  case AT_keyword_args:
    out << "    PyTuple_SET_ITEM(&args, 0, index);\n"
           "    PyObject *value = " << elem_getter->_name << "(self, (PyObject *)&args, nullptr);\n";
    break;

  case AT_varargs:
    out << "    PyTuple_SET_ITEM(&args, 0, index);\n"
           "    PyObject *value = " << elem_getter->_name << "(self, (PyObject *)&args);\n";
    break;

  case AT_single_arg:
    out << "    PyObject *value = " << elem_getter->_name << "(self, index);\n";
    break;

  default:
    out << "    PyObject *value = " << elem_getter->_name << "(self, nullptr);\n";
    break;
  }

  out <<
    "    PyTuple_SET_ITEM(tuple, i, value);\n"
    "    Py_DECREF(index);\n"
    "  }\n"
    "\n";

  if ((elem_getter->_args_type & AT_varargs) == AT_varargs) {
    out << "  _Py_ForgetReference((PyObject *)&args);\n";
  }

  out <<
    "  if (Dtool_CheckErrorOccurred()) {\n"
    "    Py_DECREF(tuple);\n"
    "    return nullptr;\n"
    "  }\n"
    "  return tuple;\n"
    "}\n"
    "\n";
}

/**
 * Generates the synthetic method described by the MAKE_PROPERTY() macro.
 */
void InterfaceMakerPythonNative::
write_getset(ostream &out, Object *obj, Property *property) {
  // We keep around this empty vector for passing to get_call_str.
  const vector_string pexprs;

  string ClassName = make_safe_name(obj->_itype.get_scoped_name());
  std::string cClassName = obj->_itype.get_true_name();

  const InterrogateElement &ielem = property->_ielement;

  FunctionRemap *len_remap = nullptr;
  if (property->_length_function != nullptr) {
    assert(!property->_length_function->_remaps.empty());

    // This is actually a sequence.  Wrap this with a special class.
    len_remap = property->_length_function->_remaps.front();

    out << "/**\n"
           " * sequence length function for property " << ielem.get_scoped_name() << "\n"
           " */\n"
           "static Py_ssize_t Dtool_" + ClassName + "_" + ielem.get_name() + "_Len(PyObject *self) {\n";
    if (property->_length_function->_has_this) {
      out <<
        "  " << cClassName  << " *local_this = nullptr;\n"
        "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n"
        "    return -1;\n"
        "  }\n"
        "  return (Py_ssize_t)" << len_remap->get_call_str("local_this", pexprs) << ";\n";
    } else {
      out << "  return (Py_ssize_t)" << len_remap->get_call_str("", pexprs) << ";\n";
    }
    out << "}\n\n";
  }

  if (property->_getter_remaps.empty()) {
    return;
  }

  if (ielem.is_sequence()) {
    assert(len_remap != nullptr);
    out <<
      "/**\n"
      " * sequence getter for property " << ielem.get_scoped_name() << "\n"
      " */\n"
      "static PyObject *Dtool_" + ClassName + "_" + ielem.get_name() + "_Sequence_Getitem(PyObject *self, Py_ssize_t index) {\n";

    if (property->_has_this) {
      out <<
        "  " << cClassName << " *local_this = nullptr;\n"
        "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n"
        "    return nullptr;\n"
        "  }\n";
    }

    // This is a getitem of a sequence type.  This means we *need* to raise
    // IndexError if we're out of bounds.
    out << "  if (index < 0 || index >= (Py_ssize_t)"
        << len_remap->get_call_str("local_this", pexprs) << ") {\n";
    out << "    PyErr_SetString(PyExc_IndexError, \"" << ClassName << "." << ielem.get_name() << "[] index out of range\");\n";
    out << "    return nullptr;\n";
    out << "  }\n";

    /*if (property->_has_function != NULL) {
      out << "  if (!local_this->" << property->_has_function->_ifunc.get_name() << "(index)) {\n"
          << "    Py_INCREF(Py_None);\n"
          << "    return Py_None;\n"
          << "  }\n";
    }*/

    std::set<FunctionRemap*> remaps;

    // Extract only the getters that take one integral argument.
    for (FunctionRemap *remap : property->_getter_remaps) {
      int min_num_args = remap->get_min_num_args();
      int max_num_args = remap->get_max_num_args();
      if (min_num_args <= 1 && max_num_args >= 1 &&
          TypeManager::is_integer(remap->_parameters[(size_t)remap->_has_this]._remap->get_new_type())) {
        remaps.insert(remap);
      }
    }

    string expected_params;
    write_function_forset(out, remaps, 1, 1, expected_params, 2, true, true,
                          AT_no_args, RF_pyobject | RF_err_null, false, true, "index");

    out << "  if (!_PyErr_OCCURRED()) {\n";
    out << "    return Dtool_Raise_BadArgumentsError(\n";
    output_quoted(out, 6, expected_params);
    out << ");\n"
            "  }\n"
            "}\n\n";

    // Write out a setitem if this is not a read-only property.
    if (!property->_setter_remaps.empty()) {
      out << "static int Dtool_" + ClassName + "_" + ielem.get_name() + "_Sequence_Setitem(PyObject *self, Py_ssize_t index, PyObject *arg) {\n";
      if (property->_has_this) {
        out << "  " << cClassName  << " *local_this = nullptr;\n";
        out << "  if (!Dtool_Call_ExtractThisPointer_NonConst(self, Dtool_" << ClassName << ", (void **)&local_this, \""
            << classNameFromCppName(cClassName, false) << "." << ielem.get_name() << "\")) {\n";
        out << "    return -1;\n";
        out << "  }\n\n";
      }

      out << "  if (index < 0 || index >= (Py_ssize_t)"
          << len_remap->get_call_str("local_this", pexprs) << ") {\n";
      out << "    PyErr_SetString(PyExc_IndexError, \"" << ClassName << "." << ielem.get_name() << "[] index out of range\");\n";
      out << "    return -1;\n";
      out << "  }\n";

      out << "  if (arg == nullptr) {\n";
      if (property->_deleter != nullptr) {
        if (property->_deleter->_has_this) {
          out << "    local_this->" << property->_deleter->_ifunc.get_name() << "(index);\n";
        } else {
          out << "    " << cClassName << "::" << property->_deleter->_ifunc.get_name() << "(index);\n";
        }
        out << "    return 0;\n";
      } else {
        out << "    Dtool_Raise_TypeError(\"can't delete " << ielem.get_name() << "[] attribute\");\n"
               "    return -1;\n";
      }
      out << "  }\n";

      if (property->_clear_function != nullptr) {
        out << "  if (arg == Py_None) {\n";
        if (property->_clear_function->_has_this) {
          out << "    local_this->" << property->_clear_function->_ifunc.get_name() << "(index);\n";
        } else {
          out << "    " << cClassName << "::" << property->_clear_function->_ifunc.get_name() << "(index);\n";
        }
        out << "    return 0;\n"
            << "  }\n";
      }

      std::set<FunctionRemap*> remaps;

      // Extract only the setters that take two arguments.
      for (FunctionRemap *remap : property->_setter_remaps) {
        int min_num_args = remap->get_min_num_args();
        int max_num_args = remap->get_max_num_args();
        if (min_num_args <= 2 && max_num_args >= 2 &&
            TypeManager::is_integer(remap->_parameters[1]._remap->get_new_type())) {
          remaps.insert(remap);
        }
      }

      string expected_params;
      write_function_forset(out, remaps, 2, 2,
                            expected_params, 2, true, true, AT_single_arg,
                            RF_int, false, false, "index");

      out << "  if (!_PyErr_OCCURRED()) {\n";
      out << "    Dtool_Raise_BadArgumentsError(\n";
      output_quoted(out, 6, expected_params);
      out << ");\n";
      out << "  }\n";
      out << "  return -1;\n";
      out << "}\n\n";
    }

    // Finally, add the inserter, if one exists.
    if (property->_inserter != nullptr) {
      out << "static PyObject *Dtool_" + ClassName + "_" + ielem.get_name() + "_Sequence_insert(PyObject *self, size_t index, PyObject *arg) {\n";
      if (property->_has_this) {
        out << "  " << cClassName  << " *local_this = nullptr;\n";
        out << "  if (!Dtool_Call_ExtractThisPointer_NonConst(self, Dtool_" << ClassName << ", (void **)&local_this, \""
            << classNameFromCppName(cClassName, false) << "." << ielem.get_name() << "\")) {\n";
        out << "    return nullptr;\n";
        out << "  }\n\n";
      }

      std::set<FunctionRemap*> remaps;
      remaps.insert(property->_inserter->_remaps.begin(),
                    property->_inserter->_remaps.end());

      string expected_params;
      write_function_forset(out, remaps, 2, 2,
                            expected_params, 2, true, true, AT_single_arg,
                            RF_pyobject | RF_err_null, false, false, "index");

      out << "  if (!_PyErr_OCCURRED()) {\n";
      out << "    Dtool_Raise_BadArgumentsError(\n";
      output_quoted(out, 6, expected_params);
      out << ");\n";
      out << "  }\n";
      out << "  return nullptr;\n";
      out << "}\n\n";
    }
  }


  // Write the getitem functions.
  if (ielem.is_mapping()) {
    out <<
      "/**\n"
      " * mapping getitem for property " << ielem.get_scoped_name() << "\n"
      " */\n"
      "static PyObject *Dtool_" + ClassName + "_" + ielem.get_name() + "_Mapping_Getitem(PyObject *self, PyObject *arg) {\n";

    // Before we do the has_function: if this is also a sequence, then we have
    // to also handle the case here that we were passed an index.
    if (ielem.is_sequence()) {
      out <<
        "#if PY_MAJOR_VERSION >= 3\n"
        "  if (PyLong_CheckExact(arg)) {\n"
        "#else\n"
        "  if (PyLong_CheckExact(arg) || PyInt_CheckExact(arg)) {\n"
        "#endif\n"
        "    return Dtool_" << ClassName << "_" << ielem.get_name() << "_Sequence_Getitem(self, PyLongOrInt_AsSize_t(arg));\n"
        "  }\n\n";
    }

    if (property->_has_this) {
      out <<
        "  " << cClassName << " *local_this = nullptr;\n"
        "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n"
        "    return nullptr;\n"
        "  }\n";
    }

    if (property->_has_function != nullptr) {
      std::set<FunctionRemap*> remaps;
      remaps.insert(property->_has_function->_remaps.begin(),
                    property->_has_function->_remaps.end());

      out << "  {\n";
      string expected_params;
      write_function_forset(out, remaps, 1, 1, expected_params, 4, true, true,
                            AT_single_arg, RF_raise_keyerror | RF_err_null, false, true);
      out << "  }\n";
    }

    std::set<FunctionRemap*> remaps;
    // Extract only the getters that take one argument.  Fish out the ones
    // already taken by the sequence getter.
    for (FunctionRemap *remap : property->_getter_remaps) {
      int min_num_args = remap->get_min_num_args();
      int max_num_args = remap->get_max_num_args();
      if (min_num_args <= 1 && max_num_args >= 1 &&
          (!ielem.is_sequence() || !TypeManager::is_integer(remap->_parameters[(size_t)remap->_has_this]._remap->get_new_type()))) {
        remaps.insert(remap);
      }
    }

    string expected_params;
    write_function_forset(out, remaps, 1, 1, expected_params, 2, true, true,
                          AT_single_arg, RF_pyobject | RF_err_null, false, true);

    out << "  if (!_PyErr_OCCURRED()) {\n";
    out << "    return Dtool_Raise_BadArgumentsError(\n";
    output_quoted(out, 6, expected_params);
    out << ");\n"
            "  }\n"
            "  return nullptr;\n"
            "}\n\n";

    // Write out a setitem if this is not a read-only property.
    if (!property->_setter_remaps.empty()) {
      out <<
        "/**\n"
        " * mapping setitem for property " << ielem.get_scoped_name() << "\n"
        " */\n"
        "static int Dtool_" + ClassName + "_" + ielem.get_name() + "_Mapping_Setitem(PyObject *self, PyObject *key, PyObject *value) {\n";

      if (property->_has_this) {
        out <<
          "  " << cClassName  << " *local_this = nullptr;\n"
          "  if (!Dtool_Call_ExtractThisPointer_NonConst(self, Dtool_" << ClassName << ", (void **)&local_this, \""
            << classNameFromCppName(cClassName, false) << "." << ielem.get_name() << "\")) {\n"
          "    return -1;\n"
          "  }\n\n";
      }

      out << "  if (value == nullptr) {\n";
      if (property->_deleter != nullptr) {
        out << "    PyObject *arg = key;\n";

        if (property->_has_function != nullptr) {
          std::set<FunctionRemap*> remaps;
          remaps.insert(property->_has_function->_remaps.begin(),
                        property->_has_function->_remaps.end());

          out << "    {\n";
          string expected_params;
          write_function_forset(out, remaps, 1, 1, expected_params, 6, true, true,
                                AT_single_arg, RF_raise_keyerror | RF_int, false, true);
          out << "    }\n";
        }

        std::set<FunctionRemap*> remaps;
        remaps.insert(property->_deleter->_remaps.begin(),
                      property->_deleter->_remaps.end());

        string expected_params;
        write_function_forset(out, remaps, 1, 1,
                              expected_params, 4, true, true, AT_single_arg,
                              RF_int, false, false);
        out << "    return -1;\n";
      } else {
        out << "    Dtool_Raise_TypeError(\"can't delete " << ielem.get_name() << "[] attribute\");\n"
               "    return -1;\n";
      }
      out << "  }\n";

      if (property->_clear_function != nullptr) {
        out << "  if (value == Py_None) {\n"
            << "    local_this->" << property->_clear_function->_ifunc.get_name() << "(key);\n"
            << "    return 0;\n"
            << "  }\n";
      }

      std::set<FunctionRemap*> remaps;
      remaps.insert(property->_setter_remaps.begin(),
                    property->_setter_remaps.end());

      // We have to create an args tuple only to unpack it later, ugh.
      out << "  PyObject *args = PyTuple_New(2);\n"
          << "  PyTuple_SET_ITEM(args, 0, key);\n"
          << "  PyTuple_SET_ITEM(args, 1, value);\n"
          << "  Py_INCREF(key);\n"
          << "  Py_INCREF(value);\n";

      string expected_params;
      write_function_forset(out, remaps, 2, 2,
                            expected_params, 2, true, true, AT_varargs,
                            RF_int | RF_decref_args, false, false);

      out << "  if (!_PyErr_OCCURRED()) {\n";
      out << "    Dtool_Raise_BadArgumentsError(\n";
      output_quoted(out, 6, expected_params);
      out << ");\n";
      out << "  }\n";
      out << "  Py_DECREF(args);\n";
      out << "  return -1;\n";
      out << "}\n\n";
    }

    if (property->_getkey_function != nullptr) {
      out <<
        "/**\n"
        " * mapping key-getter for property " << ielem.get_scoped_name() << "\n"
        " */\n"
        "static PyObject *Dtool_" + ClassName + "_" + ielem.get_name() + "_Mapping_Getkey(PyObject *self, Py_ssize_t index) {\n";

      if (property->_has_this) {
        out <<
          "  " << cClassName << " *local_this = nullptr;\n"
          "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n"
          "    return nullptr;\n"
          "  }\n";
      }

      // We need to raise IndexError if we're out of bounds.
      if (len_remap != nullptr) {
        out << "  if (index < 0 || index >= (Py_ssize_t)"
            << len_remap->get_call_str("local_this", pexprs) << ") {\n";
        out << "    PyErr_SetString(PyExc_IndexError, \"" << ClassName << "." << ielem.get_name() << "[] index out of range\");\n";
        out << "    return nullptr;\n";
        out << "  }\n";
      }

      std::set<FunctionRemap*> remaps;

      // Extract only the getters that take one integral argument.
      for (FunctionRemap *remap : property->_getkey_function->_remaps) {
        int min_num_args = remap->get_min_num_args();
        int max_num_args = remap->get_max_num_args();
        if (min_num_args <= 1 && max_num_args >= 1 &&
            TypeManager::is_integer(remap->_parameters[(size_t)remap->_has_this]._remap->get_new_type())) {
          remaps.insert(remap);
        }
      }

      string expected_params;
      write_function_forset(out, remaps, 1, 1, expected_params, 2, true, true,
                            AT_no_args, RF_pyobject | RF_err_null, false, true, "index");

      out << "  if (!_PyErr_OCCURRED()) {\n";
      out << "    return Dtool_Raise_BadArgumentsError(\n";
      output_quoted(out, 6, expected_params);
      out << ");\n"
              "  }\n"
              "}\n\n";
    }
  }

  // Now write the actual getter wrapper.  It will be a different wrapper
  // depending on whether it's a mapping or a sequence.
  out << "static PyObject *Dtool_" + ClassName + "_" + ielem.get_name() + "_Getter(PyObject *self, void *) {\n";

  // Is this property shadowing a static method with the same name?  This is a
  // special case to handle WindowProperties::make -- see GH #444.
  if (property->_has_this) {
    for (const Function *func : obj->_methods) {
      if (!func->_has_this && func->_ifunc.get_name() == ielem.get_name()) {
        string flags;
        string fptr = "&" + func->_name;
        switch (func->_args_type) {
        case AT_keyword_args:
          flags = "METH_VARARGS | METH_KEYWORDS";
          fptr = "(PyCFunction) " + fptr;
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
        out << "  if (self == nullptr) {\n"
            << "    static PyMethodDef def = {\"" << ielem.get_name() << "\", "
            << fptr << ", " << flags << " | METH_STATIC, (const char *)"
            << func->_name << "_comment};\n"
            << "    return PyCFunction_New(&def, nullptr);\n"
            << "  }\n\n";
        break;
      }
    }
  }

  if (ielem.is_mapping()) {
    if (property->_has_this) {
      out << "  nassertr(self != nullptr, nullptr);\n";
    }
    if (property->_setter_remaps.empty()) {
      out << "  Dtool_MappingWrapper *wrap = Dtool_NewMappingWrapper(self, \"" << ClassName << "." << ielem.get_name() << "\");\n";
    } else {
      out << "  Dtool_MappingWrapper *wrap = Dtool_NewMutableMappingWrapper(self, \"" << ClassName << "." << ielem.get_name() << "\");\n";
    }
    out << "  if (wrap != nullptr) {\n"
           "    wrap->_getitem_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Mapping_Getitem;\n";
    if (!property->_setter_remaps.empty()) {
      if (property->_has_this) {
        out << "    if (!DtoolInstance_IS_CONST(self)) {\n";
      } else {
        out << "    {\n";
      }
      out << "      wrap->_setitem_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Mapping_Setitem;\n";
      out << "    }\n";
    }
    if (property->_length_function != nullptr) {
      out << "    wrap->_keys._len_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Len;\n";
      if (property->_getkey_function != nullptr) {
        out << "    wrap->_keys._getitem_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Mapping_Getkey;\n";
      }
    }
    out << "  }\n"
           "  return (PyObject *)wrap;\n"
            "}\n\n";

  } else if (ielem.is_sequence()) {
    if (property->_has_this) {
      out << "  nassertr(self != nullptr, nullptr);\n";
    }
    if (property->_setter_remaps.empty()) {
      out <<
        "  Dtool_SequenceWrapper *wrap = Dtool_NewSequenceWrapper(self, \"" << ClassName << "." << ielem.get_name() << "\");\n"
        "  if (wrap != nullptr) {\n"
        "    wrap->_len_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Len;\n"
        "    wrap->_getitem_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Sequence_Getitem;\n";
    } else {
      out <<
        "  Dtool_MutableSequenceWrapper *wrap = Dtool_NewMutableSequenceWrapper(self, \"" << ClassName << "." << ielem.get_name() << "\");\n"
        "  if (wrap != nullptr) {\n"
        "    wrap->_len_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Len;\n"
        "    wrap->_getitem_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Sequence_Getitem;\n";
      if (!property->_setter_remaps.empty()) {
        if (property->_has_this) {
          out << "    if (!DtoolInstance_IS_CONST(self)) {\n";
        } else {
          out << "    {\n";
        }
        out << "      wrap->_setitem_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Sequence_Setitem;\n";
        if (property->_inserter != nullptr) {
          out << "      wrap->_insert_func = &Dtool_" << ClassName << "_" << ielem.get_name() << "_Sequence_insert;\n";
        }
        out << "    }\n";
      }
    }
    out << "  }\n"
           "  return (PyObject *)wrap;\n"
            "}\n\n";

  } else {
    // Write out a regular, unwrapped getter.
    FunctionRemap *remap = property->_getter_remaps.front();

    if (remap->_has_this) {
      if (remap->_const_method) {
        out << "  const " << cClassName  << " *local_this = nullptr;\n";
        out << "  if (!Dtool_Call_ExtractThisPointer(self, Dtool_" << ClassName << ", (void **)&local_this)) {\n";
      } else {
        out << "  " << cClassName  << " *local_this = nullptr;\n";
        out << "  if (!Dtool_Call_ExtractThisPointer_NonConst(self, Dtool_" << ClassName << ", (void **)&local_this, \""
            << classNameFromCppName(cClassName, false) << "." << ielem.get_name() << "\")) {\n";
      }
      out << "    return nullptr;\n";
      out << "  }\n\n";
    }

    if (property->_has_function != nullptr) {
      if (remap->_has_this) {
        out << "  if (!local_this->" << property->_has_function->_ifunc.get_name() << "()) {\n";
      } else {
        out << "  if (!" << cClassName << "::" << property->_has_function->_ifunc.get_name() << "()) {\n";
      }
      out << "    Py_INCREF(Py_None);\n"
          << "    return Py_None;\n"
          << "  }\n";
    }

    std::set<FunctionRemap*> remaps;
    remaps.insert(remap);

    string expected_params;
    write_function_forset(out, remaps, 0, 0,
                          expected_params, 2, false, true, AT_no_args,
                          RF_pyobject | RF_err_null, false, false);
    out << "}\n\n";

    // Write out a setter if this is not a read-only property.
    if (!property->_setter_remaps.empty()) {
      out << "static int Dtool_" + ClassName + "_" + ielem.get_name() + "_Setter(PyObject *self, PyObject *arg, void *) {\n";
      if (remap->_has_this) {
        out << "  " << cClassName  << " *local_this = nullptr;\n";
        out << "  if (!Dtool_Call_ExtractThisPointer_NonConst(self, Dtool_" << ClassName << ", (void **)&local_this, \""
            << classNameFromCppName(cClassName, false) << "." << ielem.get_name() << "\")) {\n";
        out << "    return -1;\n";
        out << "  }\n\n";
      }

      out << "  if (arg == nullptr) {\n";
      if (property->_deleter != nullptr && remap->_has_this) {
        out << "    local_this->" << property->_deleter->_ifunc.get_name() << "();\n"
            << "    return 0;\n";
      } else if (property->_deleter != nullptr) {
        out << "    " << cClassName << "::" << property->_deleter->_ifunc.get_name() << "();\n"
            << "    return 0;\n";
      } else {
        out << "    Dtool_Raise_TypeError(\"can't delete " << ielem.get_name() << " attribute\");\n"
                "    return -1;\n";
      }
      out << "  }\n";

      if (property->_clear_function != nullptr) {
        out << "  if (arg == Py_None) {\n";
        if (remap->_has_this) {
          out << "    local_this->" << property->_clear_function->_ifunc.get_name() << "();\n";
        } else {
          out << "    " << cClassName << "::" << property->_clear_function->_ifunc.get_name() << "();\n";
        }
        out << "    return 0;\n"
            << "  }\n";
      }

      std::set<FunctionRemap*> remaps;

      // Extract only the setters that take one argument.
      for (FunctionRemap *remap : property->_setter_remaps) {
        int min_num_args = remap->get_min_num_args();
        int max_num_args = remap->get_max_num_args();
        if (min_num_args <= 1 && max_num_args >= 1) {
          remaps.insert(remap);
        }
      }

      string expected_params;
      write_function_forset(out, remaps, 1, 1,
                            expected_params, 2, true, true, AT_single_arg,
                            RF_int, false, false);

      out << "  if (!_PyErr_OCCURRED()) {\n";
      out << "    Dtool_Raise_BadArgumentsError(\n";
      output_quoted(out, 6, expected_params);
      out << ");\n";
      out << "  }\n";
      out << "  return -1;\n";
      out << "}\n\n";
    }
  }
}

/**
 * Records the indicated type, which may be a struct type, along with all of
 * its associated methods, if any.
 */
InterfaceMaker::Object *InterfaceMakerPythonNative::
record_object(TypeIndex type_index) {
  if (type_index == 0) {
    return nullptr;
  }

  Objects::iterator oi = _objects.find(type_index);
  if (oi != _objects.end()) {
    return (*oi).second;
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  const InterrogateType &itype = idb->get_type(type_index);

  if (!is_cpp_type_legal(itype._cpptype)) {
    return nullptr;
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
      /*if (itype.derivation_has_downcast(di)) {
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
      }*/
    }
  }

  int num_elements = itype.number_of_elements();
  for (int ei = 0; ei < num_elements; ei++) {
    ElementIndex element_index = itype.get_element(ei);

    Property *property = record_property(itype, element_index);
    if (property != nullptr) {
      object->_properties.push_back(property);
    } else {
      // No use exporting a property without a getter.
      delete property;
    }
  }

  int num_make_seqs = itype.number_of_make_seqs();
  for (int msi = 0; msi < num_make_seqs; msi++) {
    MakeSeqIndex make_seq_index = itype.get_make_seq(msi);
    const InterrogateMakeSeq &imake_seq = idb->get_make_seq(make_seq_index);

    string class_name = itype.get_scoped_name();
    string clean_name = InterrogateBuilder::clean_identifier(class_name);
    string wrapper_name = "MakeSeq_" + clean_name + "_" + imake_seq.get_name();

    MakeSeq *make_seq = new MakeSeq(wrapper_name, imake_seq);
    make_seq->_length_getter = record_function(itype, imake_seq.get_length_getter());
    make_seq->_element_getter = record_function(itype, imake_seq.get_element_getter());
    object->_make_seqs.push_back(make_seq);
  }

  object->check_protocols();

  int num_nested = itype.number_of_nested_types();
  for (int ni = 0; ni < num_nested; ni++) {
    TypeIndex nested_index = itype.get_nested_type(ni);
    record_object(nested_index);
  }
  return object;
}

/**
 *
 */
InterfaceMaker::Property *InterfaceMakerPythonNative::
record_property(const InterrogateType &itype, ElementIndex element_index) {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  const InterrogateElement &ielement = idb->get_element(element_index);
  if (!ielement.has_getter()) {
    // A property needs at the very least a getter.
    return nullptr;
  }

  Property *property;
  {
    FunctionIndex func_index = ielement.get_getter();
    if (func_index != 0) {
      const InterrogateFunction &ifunc = idb->get_function(func_index);
      property = new Property(ielement);

      InterrogateFunction::Instances::const_iterator ii;
      for (ii = ifunc._instances->begin(); ii != ifunc._instances->end(); ++ii) {
        CPPInstance *cppfunc = (*ii).second;
        FunctionRemap *remap =
          make_function_remap(itype, ifunc, cppfunc, 0);

        if (remap != nullptr && is_remap_legal(remap)) {
          property->_getter_remaps.push_back(remap);
          property->_has_this |= remap->_has_this;
        }
      }
    } else {
      return nullptr;
    }
  }

  if (ielement.has_setter()) {
    FunctionIndex func_index = ielement.get_setter();
    if (func_index != 0) {
      const InterrogateFunction &ifunc = idb->get_function(func_index);

      InterrogateFunction::Instances::const_iterator ii;
      for (ii = ifunc._instances->begin(); ii != ifunc._instances->end(); ++ii) {
        CPPInstance *cppfunc = (*ii).second;
        FunctionRemap *remap =
          make_function_remap(itype, ifunc, cppfunc, 0);

        if (remap != nullptr && is_remap_legal(remap)) {
          property->_setter_remaps.push_back(remap);
          property->_has_this |= remap->_has_this;
        }
      }
    }
  }

  if (ielement.has_has_function()) {
    FunctionIndex func_index = ielement.get_has_function();
    Function *has_function = record_function(itype, func_index);
    if (is_function_legal(has_function)) {
      property->_has_function = has_function;
      property->_has_this |= has_function->_has_this;
    }
  }

  if (ielement.has_clear_function()) {
    FunctionIndex func_index = ielement.get_clear_function();
    Function *clear_function = record_function(itype, func_index);
    if (is_function_legal(clear_function)) {
      property->_clear_function = clear_function;
      property->_has_this |= clear_function->_has_this;
    }
  }

  if (ielement.has_del_function()) {
    FunctionIndex func_index = ielement.get_del_function();
    Function *del_function = record_function(itype, func_index);
    if (is_function_legal(del_function)) {
      property->_deleter = del_function;
      property->_has_this |= del_function->_has_this;
    }
  }

  if (ielement.is_sequence() || ielement.is_mapping()) {
    FunctionIndex func_index = ielement.get_length_function();
    if (func_index != 0) {
      property->_length_function = record_function(itype, func_index);
    }
  }

  if (ielement.is_sequence() && ielement.has_insert_function()) {
    FunctionIndex func_index = ielement.get_insert_function();
    Function *insert_function = record_function(itype, func_index);
    if (is_function_legal(insert_function)) {
      property->_inserter = insert_function;
      property->_has_this |= insert_function->_has_this;
    }
  }

  if (ielement.is_mapping() && ielement.has_getkey_function()) {
    FunctionIndex func_index = ielement.get_getkey_function();
    assert(func_index != 0);
    Function *getkey_function = record_function(itype, func_index);
    if (is_function_legal(getkey_function)) {
      property->_getkey_function = getkey_function;
      property->_has_this |= getkey_function->_has_this;
    }
  }

  return property;
}

/**
 * Walks through the set of functions in the database and generates wrappers
 * for each function, storing these in the database.  No actual code should be
 * output yet; this just updates the database with the wrapper information.
 */
void InterfaceMakerPythonNative::
generate_wrappers() {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  // We use a while loop rather than a simple for loop, because we might
  // increase the number of types recursively during the traversal.

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
}

/**

 */
bool InterfaceMakerPythonNative::
is_cpp_type_legal(CPPType *in_ctype) {
  if (in_ctype == nullptr) {
    return false;
  }

  string name = in_ctype->get_local_name(&parser);

  if (builder.in_ignoretype(name)) {
    return false;
  }

  if (builder.in_forcetype(name)) {
    return true;
  }

  // bool answer = false;
  CPPType *type = TypeManager::resolve_type(in_ctype);
  if (TypeManager::is_rvalue_reference(type)) {
    return false;
  }

  type = TypeManager::unwrap(type);

  if (TypeManager::is_void(type)) {
    return true;
  } else if (TypeManager::is_basic_string_char(type)) {
    return true;
  } else if (TypeManager::is_basic_string_wchar(type)) {
    return true;
  } else if (TypeManager::is_vector_unsigned_char(in_ctype)) {
    return true;
  } else if (TypeManager::is_simple(type)) {
    return true;
  } else if (TypeManager::is_pointer_to_simple(type)) {
    return true;
  } else if (builder.in_forcetype(type->get_local_name(&parser))) {
    return true;
  } else if (TypeManager::is_exported(type)) {
    return true;
  } else if (TypeManager::is_pointer_to_PyObject(in_ctype)) {
    return true;
  } else if (TypeManager::is_pointer_to_Py_buffer(in_ctype)) {
    return true;
  }

  // if (answer == false) printf(" -------------------- Bad Type ??
  // %s\n",type->get_local_name().c_str());

  return false;
}
/**

 */
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

/**

 */
bool InterfaceMakerPythonNative::
isExportThisRun(Function *func) {
  if (func == nullptr || !is_function_legal(func)) {
    return false;
  }

  for (FunctionRemap *remap : func->_remaps) {
    return isExportThisRun(remap->_cpptype);
  }

  return false;
}

/**

 */
bool InterfaceMakerPythonNative::
is_remap_legal(FunctionRemap *remap) {
  if (remap == nullptr) {
    return false;
  }

  // return must be legal and managable..
  if (!is_cpp_type_legal(remap->_return_type->get_orig_type())) {
// printf("  is_remap_legal Return Is Bad %s\n",remap->_return_type->get_orig_
// type()->get_fully_scoped_name().c_str());
    return false;
  }

  // We don't currently support returning pointers, but we accept them as
  // function parameters.  But const char * is an exception.
  if (!remap->_return_type->new_type_is_atomic_string() &&
      TypeManager::is_pointer_to_simple(remap->_return_type->get_orig_type())) {
    return false;
  }

  // ouch .. bad things will happen here ..  do not even try..
  if (remap->_ForcedVoidReturn) {
    return false;
  }

  // all non-optional params must be legal
  for (size_t pn = 0; pn < remap->_parameters.size(); pn++) {
    ParameterRemap *param = remap->_parameters[pn]._remap;
    CPPType *orig_type = param->get_orig_type();
    if (param->get_default_value() == nullptr && !is_cpp_type_legal(orig_type)) {
      return false;
    }
  }

  // Don't export global operators.
  if (!remap->_has_this &&
      remap->_cppfunc->get_simple_name().compare(0, 9, "operator ") == 0) {
    return false;
  }

  // ok all looks ok.
  return true;
}

/**

 */
int InterfaceMakerPythonNative::
has_coerce_constructor(CPPStructType *type) {
  if (type == nullptr) {
    return 0;
  }

  // It is convenient to set default-constructability and move-assignability
  // as requirement for non-reference-counted objects, since it simplifies the
  // implementation and it holds for all classes we need it for.
  if (!TypeManager::is_reference_count(type) &&
      (!type->is_default_constructible() || !type->is_move_assignable())) {
    return 0;
  }

  CPPScope *scope = type->get_scope();
  if (scope == nullptr) {
    return 0;
  }

  int result = 0;

  CPPScope::Functions::iterator fgi;
  for (fgi = scope->_functions.begin(); fgi != scope->_functions.end(); ++fgi) {
    CPPFunctionGroup *fgroup = fgi->second;
    for (CPPInstance *inst : fgroup->_instances) {
      CPPFunctionType *ftype = inst->_type->as_function_type();
      if (ftype == nullptr) {
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
        if (ftype->_flags & (CPPFunctionType::F_copy_constructor |
                             CPPFunctionType::F_move_constructor)) {
          // Skip a copy and move constructor.
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

/**

 */
bool InterfaceMakerPythonNative::
is_remap_coercion_possible(FunctionRemap *remap) {
  if (remap == nullptr) {
    return false;
  }

  size_t pn = 0;
  if (remap->_has_this) {
    // Skip the "this" parameter.  It's never coercible.
    ++pn;
  }
  while (pn < remap->_parameters.size()) {
    CPPType *type = remap->_parameters[pn]._remap->get_new_type();

    if (TypeManager::is_char_pointer(type)) {
    } else if (TypeManager::is_wchar_pointer(type)) {
    } else if (TypeManager::is_pointer_to_PyObject(type)) {
    } else if (TypeManager::is_pointer_to_Py_buffer(type)) {
    } else if (TypeManager::is_pointer_to_simple(type)) {
    } else if (TypeManager::is_pointer(type)) {
      // This is a pointer to an object, so we might be able to coerce a
      // parameter to it.
      CPPType *obj_type = TypeManager::unwrap(TypeManager::resolve_type(type));
      if (has_coerce_constructor(obj_type->as_struct_type()) > 0) {
        // It has a coercion constructor, so go for it.
        return true;
      }
    }
    ++pn;
  }

  return false;
}

/**

 */
bool InterfaceMakerPythonNative::
is_function_legal(Function *func) {
  for (FunctionRemap *remap : func->_remaps) {
    if (is_remap_legal(remap)) {
// printf("  Function Is Marked Legal %s\n",func->_name.c_str());

      return true;
    }
  }

// printf("  Function Is Marked Illegal %s\n",func->_name.c_str());
  return false;
}

/**

 */
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

/**

 */
bool InterfaceMakerPythonNative::
DoesInheritFromIsClass(const CPPStructType *inclass, const std::string &name) {
  if (inclass == nullptr) {
    return false;
  }

  std::string scoped_name = inclass->get_fully_scoped_name();
  if (scoped_name == name) {
    return true;
  }

  for (const CPPStructType::Base &base : inclass->_derivation) {
    CPPStructType *base_type = TypeManager::resolve_type(base._base)->as_struct_type();
    if (base_type != nullptr) {
      if (DoesInheritFromIsClass(base_type, name)) {
        return true;
      }
    }
  }
  return false;
}

/**

 */
bool InterfaceMakerPythonNative::
has_get_class_type_function(CPPType *type) {
  while (type->get_subtype() == CPPDeclaration::ST_typedef) {
    type = type->as_typedef_type()->_type;
  }

  CPPStructType *struct_type = type->as_struct_type();
  if (struct_type == nullptr) {
    return false;
  }

  CPPScope *scope = struct_type->get_scope();

  return scope->_functions.find("get_class_type") != scope->_functions.end();
}

/**
 *
 */
bool InterfaceMakerPythonNative::
has_init_type_function(CPPType *type) {
  while (type->get_subtype() == CPPDeclaration::ST_typedef) {
    type = type->as_typedef_type()->_type;
  }

  CPPStructType *struct_type = type->as_struct_type();
  if (struct_type == nullptr) {
    return false;
  }

  CPPScope *scope = struct_type->get_scope();
  CPPScope::Functions::const_iterator it = scope->_functions.find("init_type");
  if (it == scope->_functions.end()) {
    return false;
  }
  const CPPFunctionGroup *group = it->second;

  for (const CPPInstance *cppinst : group->_instances) {
    const CPPFunctionType *cppfunc = cppinst->_type->as_function_type();

    if (cppfunc != nullptr &&
        cppfunc->_parameters != nullptr &&
        cppfunc->_parameters->_parameters.size() == 0 &&
        (cppinst->_storage_class & CPPInstance::SC_static) != 0) {
      return true;
    }
  }

  return false;
}

/**
 * Returns -1 if the class does not define write() (and therefore cannot
 * support a __str__ function).
 *
 * Returns 1 if the class defines write(ostream).
 *
 * Returns 2 if the class defines write(ostream, int).
 *
 * Note that if you want specific behavior for Python str(), you should just
 * define a __str__ function, which maps directly to the appropriate type
 * slot.
 */
int InterfaceMakerPythonNative::
NeedsAStrFunction(const InterrogateType &itype_class) {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  int num_methods = itype_class.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; ++mi) {
    FunctionIndex func_index = itype_class.get_method(mi);
    const InterrogateFunction &ifunc = idb->get_function(func_index);
    if (ifunc.get_name() == "write") {
      if (ifunc._instances != nullptr) {
        InterrogateFunction::Instances::const_iterator ii;
        for (ii = ifunc._instances->begin();
             ii != ifunc._instances->end();
             ++ii) {
          CPPInstance *cppinst = (*ii).second;
          CPPFunctionType *cppfunc = cppinst->_type->as_function_type();

          if (cppfunc != nullptr) {
            if (cppfunc->_parameters != nullptr &&
                cppfunc->_return_type != nullptr &&
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
                  if (inst1->_initializer != nullptr) {
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

/**
 * Returns -1 if the class does not define output() or python_repr() (and
 * therefore cannot support a __repr__ function).
 *
 * Returns 1 if the class defines python_repr(ostream, string).
 *
 * Returns 2 if the class defines output(ostream).
 *
 * Returns 3 if the class defines an extension function for
 * python_repr(ostream, string).
 *
 * Note that defining python_repr is deprecated in favor of defining a
 * __repr__ that returns a string, which maps directly to the appropriate type
 * slot.
 */
int InterfaceMakerPythonNative::
NeedsAReprFunction(const InterrogateType &itype_class) {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  int num_methods = itype_class.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; ++mi) {
    FunctionIndex func_index = itype_class.get_method(mi);
    const InterrogateFunction &ifunc = idb->get_function(func_index);
    if (ifunc.get_name() == "python_repr") {
      if (ifunc._instances != nullptr) {
        InterrogateFunction::Instances::const_iterator ii;
        for (ii = ifunc._instances->begin();
             ii != ifunc._instances->end();
             ++ii) {
          CPPInstance *cppinst = (*ii).second;
          CPPFunctionType *cppfunc = cppinst->_type->as_function_type();

          if (cppfunc != nullptr) {
            if (cppfunc->_parameters != nullptr &&
                cppfunc->_return_type != nullptr &&
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
      if (ifunc._instances != nullptr) {
        InterrogateFunction::Instances::const_iterator ii;
        for (ii = ifunc._instances->begin();
             ii != ifunc._instances->end();
             ++ii) {
          CPPInstance *cppinst = (*ii).second;
          CPPFunctionType *cppfunc = cppinst->_type->as_function_type();

          if (cppfunc != nullptr) {
            if (cppfunc->_parameters != nullptr &&
                cppfunc->_return_type != nullptr &&
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
                  if (inst1->_initializer != nullptr) {
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

/**
 * Returns true if the class defines a rich comparison operator.
 */
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

/**
 * Outputs the indicated string as a single quoted, multi-line string to the
 * generated C++ source code.  The output point is left on the last line of
 * the string, following the trailing quotation mark.
 */
void InterfaceMakerPythonNative::
output_quoted(ostream &out, int indent_level, const std::string &str,
              bool first_line) {
  indent(out, (first_line ? indent_level : 0))
    << '"';
  std::string::const_iterator si;
  for (si = str.begin(); si != str.end();) {
    switch (*si) {
    case '"':
    case '\\':
      out << '\\' << *si;
      break;

    case '\n':
      out << "\\n\"";
      if (++si == str.end()) {
        return;
      }
      out << "\n";
      indent(out, indent_level)
        << '"';
      continue;

    default:
      if (!isprint(*si)) {
        out << "\\" << oct << std::setw(3) << std::setfill('0') << (unsigned int)(*si)
            << dec;
      } else {
        out << *si;
      }
    }
    ++si;
  }
  out << '"';
}
