/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interfaceMaker.cxx
 * @author drose
 * @date 2001-09-19
 */

#include "interfaceMaker.h"
#include "interrogateBuilder.h"
#include "typeManager.h"
#include "interrogate.h"
#include "functionRemap.h"
#include "parameterRemap.h"
#include "parameterRemapThis.h"
#include "parameterRemapUnchanged.h"
#include "parameterRemapReferenceToPointer.h"
#include "parameterRemapConcreteToPointer.h"
#include "parameterRemapEnumToInt.h"
#include "parameterRemapConstToNonConst.h"
#include "parameterRemapReferenceToConcrete.h"
#include "parameterRemapCharStarToString.h"
#include "parameterRemapBasicStringToString.h"
#include "parameterRemapBasicStringRefToString.h"
#include "parameterRemapBasicStringPtrToString.h"
#include "parameterRemapPTToPointer.h"

#include "interrogateDatabase.h"
#include "interrogateManifest.h"
#include "interrogateElement.h"
#include "cppFunctionType.h"
#include "cppParameterList.h"
#include "cppMakeSeq.h"
#include "cppStructType.h"
#include "pnotify.h"

using std::ostream;
using std::ostringstream;
using std::string;

InterrogateType dummy_type;

/**
 *
 */
InterfaceMaker::Function::
Function(const string &name,
         const InterrogateType &itype,
         const InterrogateFunction &ifunc) :
  _name(name),
  _itype(itype),
  _ifunc(ifunc)
{
  _has_this = false;
  _flags = 0;
  _args_type = AT_unknown;
}

/**
 *
 */
InterfaceMaker::Function::
~Function() {
  Remaps::iterator ri;
  for (ri = _remaps.begin(); ri != _remaps.end(); ++ri) {
    delete (*ri);
  }
}

/**
 *
 */
InterfaceMaker::MakeSeq::
MakeSeq(const string &name, const InterrogateMakeSeq &imake_seq) :
  _name(name),
  _imake_seq(imake_seq),
  _length_getter(nullptr),
  _element_getter(nullptr)
{
}

/**
 *
 */
InterfaceMaker::Property::
Property(const InterrogateElement &ielement) :
  _ielement(ielement),
  _length_function(nullptr),
  _has_function(nullptr),
  _clear_function(nullptr),
  _deleter(nullptr),
  _inserter(nullptr),
  _getkey_function(nullptr),
  _has_this(false)
{
}

/**
 *
 */
InterfaceMaker::Object::
Object(const InterrogateType &itype) :
  _itype(itype),
  _protocol_types(0)
{
}

/**
 *
 */
InterfaceMaker::Object::
~Object() {
}

/**
 * To be called after all of the methods have been added, this checks which
 * protocols this object appears to support (based on the methods it
 * provides).
 */
void InterfaceMaker::Object::
check_protocols() {
  int flags = 0;

  Functions::const_iterator fi;
  for (fi = _constructors.begin(); fi != _constructors.end(); ++fi) {
    Function *func = (*fi);
    flags |= func->_flags;
  }
  for (fi = _methods.begin(); fi != _methods.end(); ++fi) {
    Function *func = (*fi);
    flags |= func->_flags;

    if (func->_ifunc.get_name() == "__traverse__") {
      // If we have a method named __traverse__, we implement Python's cyclic
      // garbage collection protocol.
      //XXX disabled for now because it's too unstable.
      //_protocol_types |= PT_python_gc;
    }
  }

  if ((flags & (FunctionRemap::F_getitem_int | FunctionRemap::F_size)) ==
      (FunctionRemap::F_getitem_int | FunctionRemap::F_size)) {
    // If we have both a getitem that receives an int, and a size, then we
    // implement the sequence protocol: you can iterate through the elements
    // of this object.
    _protocol_types |= PT_sequence;

  } else if (flags & FunctionRemap::F_getitem) {
    // If we have any getitem, then we implement the mapping protocol.
    _protocol_types |= PT_mapping;
  }

  if (flags & FunctionRemap::F_make_copy) {
    // It's not exactly a protocol, but if we have a make_copy() method, we
    // can use it to synthesize a __copy__ and __deepcopy__ Python method to
    // support the copy module.
    _protocol_types |= PT_make_copy;
  } else if (flags & FunctionRemap::F_copy_constructor) {
    // Ditto for the copy constructor.
    _protocol_types |= PT_copy_constructor;
  }

  if (flags & FunctionRemap::F_iter) {
    _protocol_types |= PT_iter;
  }
}

/**
 * Returns true if the first method found with the indicated name is a static
 * method, false if it is an instance method.  This does not test all
 * overloads of the indicated name, merely the first one found.
 */
bool InterfaceMaker::Object::
is_static_method(const string &name) {
  Functions::const_iterator fi;
  for (fi = _methods.begin(); fi != _methods.end(); ++fi) {
    Function *func = (*fi);
    if (!func->_remaps.empty()) {
      FunctionRemap *remap = func->_remaps.front();
      string method_name = remap->_cppfunc->get_simple_name();
      if (method_name == name) {
        return !func->_has_this;
      }
    }
  }

  // Didn't find the requested function.
  return false;
}

/**
 *
 */
InterfaceMaker::
InterfaceMaker(InterrogateModuleDef *def) :
  _def(def)
{
}

/**
 *
 */
InterfaceMaker::
~InterfaceMaker() {
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    Object *object = (*oi).second;
    delete object;
  }
  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    delete (*fi).second;
  }
}

/**
 * Walks through the set of functions in the database and generates wrappers
 * for each function, storing these in the database.  No actual code should be
 * output yet; this just updates the database with the wrapper information.
 */
void InterfaceMaker::
generate_wrappers() {
  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();

  // We use a while loop rather than a simple for loop, because we might
  // increase the number of types recursively during the traversal.
  int ti = 0;
  while (ti < idb->get_num_all_types()) {
    TypeIndex type_index = idb->get_all_type(ti++);
    record_object(type_index);
  }

  int num_global_elements = idb->get_num_global_elements();
  for (int gi = 0; gi < num_global_elements; ++gi) {
    printf(" Global Type = %d", gi);
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
    printf(" Manifests %d\n", mi);
  }

  int num_elements = idb->get_num_global_elements();
  for (int ei = 0; ei < num_elements; ei++) {
    printf(" Element %d\n", ei);

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
 * Generates the list of #include ... whatever that's required by this
 * particular interface to the indicated output stream.
 */
void InterfaceMaker::
write_includes(ostream &) {
}

/**
 * Generates the list of function prototypes corresponding to the functions
 * that will be output in write_functions().
 */
void InterfaceMaker::
write_prototypes(ostream &out,ostream *out_h) {
  _function_writers.write_prototypes(out);
}

/**
 * Generates the list of functions that are appropriate for this interface.
 */
void InterfaceMaker::
write_functions(ostream &out) {
  _function_writers.write_code(out);
}

/**
 * Generates whatever additional code is required to support a module file.
 */
void InterfaceMaker::
write_module(ostream &, ostream *out_h, InterrogateModuleDef *) {
}

/**
 * Allocates a new ParameterRemap object suitable to the indicated parameter
 * type.  If struct_type is non-NULL, it is the type of the enclosing class
 * for the function (method) in question.
 *
 * The return value is a newly-allocated ParameterRemap object, if the
 * parameter type is acceptable, or NULL if the parameter type cannot be
 * handled.
 */
ParameterRemap *InterfaceMaker::
remap_parameter(CPPType *struct_type, CPPType *param_type) {
  nassertr(param_type != nullptr, nullptr);

  if (convert_strings) {
    if (TypeManager::is_char_pointer(param_type)) {
      return new ParameterRemapCharStarToString(param_type);
    }
    if (TypeManager::is_wchar_pointer(param_type)) {
      return new ParameterRemapWCharStarToWString(param_type);
    }

    // If we're exporting a method of basic_string<char> itself, don't convert
    // basic_string<char>'s to atomic strings.

    if (struct_type == nullptr ||
        !(TypeManager::is_basic_string_char(struct_type) ||
          TypeManager::is_basic_string_wchar(struct_type))) {
      if (TypeManager::is_basic_string_char(param_type)) {
        return new ParameterRemapBasicStringToString(param_type);

      } else if (TypeManager::is_const_ref_to_basic_string_char(param_type)) {
        return new ParameterRemapBasicStringRefToString(param_type);

      } else if (TypeManager::is_const_ptr_to_basic_string_char(param_type)) {
        return new ParameterRemapBasicStringPtrToString(param_type);

      } else if (TypeManager::is_basic_string_wchar(param_type)) {
        return new ParameterRemapBasicWStringToWString(param_type);

      } else if (TypeManager::is_const_ref_to_basic_string_wchar(param_type)) {
        return new ParameterRemapBasicWStringRefToWString(param_type);

      } else if (TypeManager::is_const_ptr_to_basic_string_char(param_type)) {
        return new ParameterRemapBasicStringPtrToString(param_type);

      } else if (TypeManager::is_const_ptr_to_basic_string_wchar(param_type)) {
        return new ParameterRemapBasicWStringPtrToWString(param_type);

      } else if (TypeManager::is_reference(param_type) ||
                 TypeManager::is_pointer(param_type)) {
        // Python strings are immutable, so we can't wrap a non-const pointer
        // or reference to a string.
        CPPType *pt_type = TypeManager::unwrap(param_type);
        if (TypeManager::is_basic_string_char(pt_type) ||
            TypeManager::is_basic_string_wchar(pt_type)) {
          return nullptr;
        }
      }
    }
    if (struct_type == nullptr ||
        !TypeManager::is_vector_unsigned_char(struct_type)) {
      if (TypeManager::is_vector_unsigned_char(param_type)) {
        if (TypeManager::is_reference(param_type)) {
          return new ParameterRemapReferenceToConcrete(param_type);
        } else if (TypeManager::is_const(param_type)) {
          return new ParameterRemapConstToNonConst(param_type);
        } else {
          return new ParameterRemapUnchanged(param_type);
        }
      }
    }
  }

  if (manage_reference_counts) {
    if (TypeManager::is_pointer_to_base(param_type) ||
        TypeManager::is_const_ref_to_pointer_to_base(param_type))
    {
      CPPType *pt_type = TypeManager::unwrap_reference(param_type);

      // Don't convert PointerTo<>'s to pointers for methods of the PointerTo
      // itself!
      if (struct_type == nullptr ||
          !(pt_type->get_local_name(&parser) == struct_type->get_local_name(&parser))) {
        return new ParameterRemapPTToPointer(param_type);
      }
    }
  }

  if (TypeManager::is_reference(param_type)) {
    return new ParameterRemapReferenceToPointer(param_type);

  } else if (TypeManager::is_struct(param_type)) {
    return new ParameterRemapConcreteToPointer(param_type);

    /*
  } else if (TypeManager::is_enum(param_type) || TypeManager::is_const_ref_to_enum(param_type)) {
    return new ParameterRemapEnumToInt(param_type);
    */

  // } else if (TypeManager::is_const_simple(param_type)) { return new
  // ParameterRemapConstToNonConst(param_type);

  } else if (TypeManager::is_const_ref_to_simple(param_type)) {
    return new ParameterRemapReferenceToConcrete(param_type);

  } else if (TypeManager::is_pointer(param_type) ||
             TypeManager::is_void(param_type) ||
             TypeManager::is_simple(param_type) ||
             TypeManager::is_simple_array(param_type)) {
    return new ParameterRemapUnchanged(param_type);

  } else {
    // Here's something we have a problem with.
    return nullptr;
  }
}

/**
 * This method should be overridden and redefined to return true for
 * interfaces that require the implicit "this" parameter, if present, to be
 * passed as the first parameter to any wrapper functions.
 */
bool InterfaceMaker::
synthesize_this_parameter() {
  return false;
}

/**
 * This method should be overridden and redefined to return true for
 * interfaces that require overloaded instances of a function to be defined as
 * separate functions (each with its own hashed name), or false for interfaces
 * that can support overloading natively, and thus only require one wrapper
 * function per each overloaded input function.
 */
bool InterfaceMaker::
separate_overloading() {
  return true;
}

/**
 * This method should be overridden and redefined to return false for
 * interfaces that don't support global functions and should therefore will
 * only accept function remaps that have a class associated.
 */
bool InterfaceMaker::
wrap_global_functions() {
  return true;
}

/**
 * Fills up the indicated vector with all of the FunctionRemap pointers
 * created by this InterfaceMaker.  It is the user's responsibility to empty
 * the vector before calling this function; the new pointers will simply be
 * added to the end.
 */
void InterfaceMaker::
get_function_remaps(std::vector<FunctionRemap *> &remaps) {
  FunctionsByIndex::iterator fi;
  for (fi = _functions.begin(); fi != _functions.end(); ++fi) {
    Function *func = (*fi).second;
    Function::Remaps::const_iterator ri;
    for (ri = func->_remaps.begin(); ri != func->_remaps.end(); ++ri) {
      FunctionRemap *remap = (*ri);
      remaps.push_back(remap);
    }
  }
}

/**
 *
 */
ostream &InterfaceMaker::
indent(ostream &out, int indent_level) {
  for (int i = 0; i < indent_level; i++) {
    out << ' ';
  }
  return out;
}

/**
 * Creates a FunctionRemap object corresponding to the particular function
 * wrapper.
 */
FunctionRemap *InterfaceMaker::
make_function_remap(const InterrogateType &itype,
                    const InterrogateFunction &ifunc,
                    CPPInstance *cppfunc, int num_default_parameters) {
  FunctionRemap *remap =
    new FunctionRemap(itype, ifunc, cppfunc, num_default_parameters, this);
  if (remap->_is_valid) {
    if (separate_overloading()) {
      hash_function_signature(remap);
      remap->_unique_name =
        get_unique_prefix() + _def->library_hash_name + remap->_hash;
      remap->_wrapper_name =
        get_wrapper_prefix() + _def->library_hash_name + remap->_hash;
      remap->_reported_name = remap->_wrapper_name;

      if (true_wrapper_names) {
        remap->_reported_name =
          InterrogateBuilder::clean_identifier(remap->_cppfunc->get_local_name(&parser));
      }
    }
    return remap;
  }

  // No such FunctionRemap is valid.  Return NULL.
  delete remap;
  return nullptr;
}

/**
 * Returns the function name that will be used to wrap the indicated function.
 *
 * This is the name for the overall wrapper function, including all of the
 * overloaded instances.  Interfaces that must define a different wrapper for
 * each FunctionRemap object (i.e.  for each instance of an overloaded
 * function) need not define a name here.
 */
string InterfaceMaker::
get_wrapper_name(const InterrogateType &itype,
                 const InterrogateFunction &ifunc,
                 FunctionIndex func_index) {
  string func_name = ifunc.get_scoped_name();
  string clean_name = InterrogateBuilder::clean_identifier(func_name);

  ostringstream new_name;
  new_name << get_wrapper_prefix() << clean_name << "_" << func_index;
  return new_name.str();
}

/**
 * Returns the prefix string used to generate wrapper function names.
 */
string InterfaceMaker::
get_wrapper_prefix() {
  return "xx_";
}

/**
 * Returns the prefix string used to generate unique symbolic names, which are
 * not necessarily C-callable function names.
 */
string InterfaceMaker::
get_unique_prefix() {
  return "x";
}

/**
 * Records the indicated function, along with all of its FunctionRemap flavors
 * and FunctionWriter helpers, for future output.  Returns the new Function
 * pointer.
 */
InterfaceMaker::Function *InterfaceMaker::
record_function(const InterrogateType &itype, FunctionIndex func_index) {
  assert(func_index != 0);

  if (_functions.count(func_index)) {
    // Already exists.
    return _functions[func_index];
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  const InterrogateFunction &ifunc = idb->get_function(func_index);

  string wrapper_name = get_wrapper_name(itype, ifunc, func_index);
  Function *func = new Function(wrapper_name, itype, ifunc);
  _functions[func_index] = func;

// printf(" Function Name = %s\n", ifunc.get_name().c_str());

  // Now get all the valid FunctionRemaps for the function.
  if (ifunc._instances != nullptr) {
    InterrogateFunction::Instances::const_iterator ii;
    for (ii = ifunc._instances->begin(); ii != ifunc._instances->end(); ++ii) {
      CPPInstance *cppfunc = (*ii).second;
      CPPFunctionType *ftype = cppfunc->_type->as_function_type();
      int max_default_parameters = 0;

      if (separate_overloading()) {
        // Count up the number of default parameters this function might take.
        CPPParameterList *parameters = ftype->_parameters;
        CPPParameterList::Parameters::reverse_iterator pi;
        for (pi = parameters->_parameters.rbegin();
             pi != parameters->_parameters.rend();
             ++pi) {
          CPPInstance *param = (*pi);
          if (param->_initializer != nullptr) {
            // This parameter has a default value.
            max_default_parameters++;
          } else {
            // The first parameter without a default value ends the search.
            break;
          }
        }
      }

      // Now make a different wrapper for each combination of default
      // parameters.  This will happen only if separate_overloading(), tested
      // above, returned true; otherwise, max_default_parameters will be 0 and
      // the loop will only be traversed once.
      for (int num_default_parameters = 0;
           num_default_parameters <= max_default_parameters;
           num_default_parameters++) {
        FunctionRemap *remap =
          make_function_remap(itype, ifunc, cppfunc, num_default_parameters);
        if (remap != nullptr) {

          func->_remaps.push_back(remap);

          // If *any* of the variants of this function has a "this" pointer,
          // the entire set of functions is deemed to have a "this" pointer.
          if (remap->_has_this || (remap->_flags & FunctionRemap::F_explicit_self) != 0) {
            func->_has_this = true;
          }

          func->_flags |= remap->_flags;
          func->_args_type = (ArgsType)((int)func->_args_type | (int)remap->_args_type);

          // Make a wrapper for the function.
          FunctionWrapperIndex wrapper_index =
            remap->make_wrapper_entry(func_index);
          if (wrapper_index != 0) {
            InterrogateFunction &mod_ifunc = idb->update_function(func_index);
            record_function_wrapper(mod_ifunc, wrapper_index);
          }
        }
      }
    }
  }

  return func;
}

/**
 * Associates the function wrapper with its function in the appropriate
 * structures in the database.
 */
void InterfaceMaker::
record_function_wrapper(InterrogateFunction &, FunctionWrapperIndex) {
}

/**
 * Records the indicated type, which may be a struct type, along with all of
 * its associated methods, if any.
 */
InterfaceMaker::Object *InterfaceMaker::
record_object(TypeIndex type_index) {
  if (type_index == 0) {
    // An invalid type.
    return nullptr;
  }

  Objects::iterator oi = _objects.find(type_index);
  if (oi != _objects.end()) {
    // The object has previously been recorded.
    return (*oi).second;
  }

  InterrogateDatabase *idb = InterrogateDatabase::get_ptr();
  const InterrogateType &itype = idb->get_type(type_index);

  Object *object = new Object(itype);
  bool inserted = _objects.insert(Objects::value_type(type_index, object)).second;
  assert(inserted);

  Function *function;

  int num_constructors = itype.number_of_constructors();
  for (int ci = 0; ci < num_constructors; ci++) {
    function = record_function(itype, itype.get_constructor(ci));
    object->_constructors.push_back(function);
  }

  int num_methods = itype.number_of_methods();
  int mi;
  for (mi = 0; mi < num_methods; mi++) {
    function = record_function(itype, itype.get_method(mi));
    object->_methods.push_back(function);
  }

  int num_casts = itype.number_of_casts();
  for (mi = 0; mi < num_casts; mi++) {
    function = record_function(itype, itype.get_cast(mi));
    object->_methods.push_back(function);
  }

  int num_derivations = itype.number_of_derivations();
  for (int di = 0; di < num_derivations; di++) {
    if (itype.derivation_has_upcast(di)) {
      record_function(itype, itype.derivation_get_upcast(di));
    }
    if (itype.derivation_has_downcast(di)) {
      // Downcasts are methods of the base class, not the child class.
      TypeIndex base_type_index = itype.get_derivation(di);
      const InterrogateType &base_type = idb->get_type(base_type_index);
      record_function(base_type, itype.derivation_get_downcast(di));
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

/**
 * Does any additional processing that we might want to do on the return value
 * for the function, just before we return it.  Returns the string
 * representing the new return value after processing.
 */
string InterfaceMaker::
manage_return_value(ostream &out, int indent_level,
                    FunctionRemap *remap, const string &return_expr) const {
  if (remap->_manage_reference_count) {
    // If we're managing reference counts, and we're about to return a
    // reference countable object, then increment its count.
    if (return_expr == "return_value") {
      // If the expression is just a variable name, we can just ref it
      // directly.
      output_ref(out, indent_level, remap, return_expr);
      return return_expr;

    } else {
      // Otherwise, we should probably assign it to a temporary first, so we
      // don't invoke the function twice or something.
      CPPType *type = remap->_return_type->get_temporary_type();
      indent(out, indent_level);
      type->output_instance(out, "refcount", &parser);
      out << " = " << return_expr << ";\n";

      indent(out, indent_level)
        << "if (" << return_expr << " != nullptr) {\n";
      indent(out, indent_level + 2)
        << "(" << return_expr << ")->ref();\n";
      indent(out, indent_level)
        << "}\n";
      output_ref(out, indent_level, remap, "refcount");
      return remap->_return_type->temporary_to_return("refcount");
    }
  }

  // Otherwise, just return the expression unchanged.
  return return_expr;
}

/**
 * Cleans up the given return value by deleting it or decrementing its
 * reference count or whatever is appropriate.
 */
void InterfaceMaker::
delete_return_value(ostream &out, int indent_level,
                    FunctionRemap *remap, const string &return_expr) const {
  if (remap->_manage_reference_count) {
    // If we're managing reference counts, and we're about to return a
    // reference countable object, then decrement its count.
    output_unref(out, indent_level, remap, return_expr);

  } else if (remap->_return_value_needs_management) {
    // We should just delete it directly.
    indent(out, indent_level) << "delete " << return_expr << ";\n";
  }
}

/**
 * Outputs the code to increment the reference count for the indicated
 * variable name.
 */
void InterfaceMaker::
output_ref(ostream &out, int indent_level, FunctionRemap *remap,
           const string &varname) const {

  if (TypeManager::is_pointer_to_base(remap->_return_type->get_temporary_type())) {
    // Actually, we have it stored in a PointerTo.  No need to do anything.
    return;
  }

  if (remap->_type == FunctionRemap::T_constructor ||
      remap->_type == FunctionRemap::T_typecast) {
    // In either of these cases, we can safely assume the pointer will never
    // be NULL.
    indent(out, indent_level)
      << varname << "->ref();\n";

  } else {
    // However, in the general case, we have to check for that before we
    // attempt to ref it.

    indent(out, indent_level)
      << "if (" << varname << " != nullptr) {\n";
    indent(out, indent_level + 2)
      << varname << "->ref();\n";
    indent(out, indent_level)
      << "}\n";
  }
}

/**
 * Outputs the code to decrement the reference count for the indicated
 * variable name.
 */
void InterfaceMaker::
output_unref(ostream &out, int indent_level, FunctionRemap *remap,
             const string &varname) const {

  if (TypeManager::is_pointer_to_base(remap->_return_type->get_temporary_type())) {
    // Actually, we have it stored in a PointerTo.  No need to do anything.
    return;
  }

  if (remap->_type == FunctionRemap::T_constructor ||
      remap->_type == FunctionRemap::T_typecast) {
    // In either of these cases, we can safely assume the pointer will never
    // be NULL.
    indent(out, indent_level)
      << "unref_delete(" << varname << ");\n";

  } else {
    // However, in the general case, we have to check for that before we
    // attempt to ref it.

    indent(out, indent_level)
      << "if (" << varname << " != nullptr) {\n";

    if (TypeManager::is_pointer_to_base(remap->_return_type->get_temporary_type())) {
      // We're sure the reference count won't reach zero since we have it
      // stored in a PointerTo, so call the unref() method directly.
      indent(out, indent_level + 2)
        << varname << "->unref();\n";
    } else {
      indent(out, indent_level + 2)
        << "unref_delete(" << varname << ");\n";
    }

    indent(out, indent_level)
      << "}\n";
  }
}

/**
 * Generates a unique string that corresponds to the function signature for
 * the indicated FunctionRemap object, and stores the generated string in the
 * _hash member of the FunctionRemap.
 */
void InterfaceMaker::
hash_function_signature(FunctionRemap *remap) {
  string hash = InterrogateBuilder::hash_string(remap->_function_signature, 5);

  // Now make sure we don't have another function with the same hash.
  WrappersByHash::iterator hi;
  hi = _wrappers_by_hash.find(hash);
  if (hi == _wrappers_by_hash.end()) {
    // No other name; we're in the clear.
    _wrappers_by_hash[hash] = remap;
    remap->_hash = hash;
    return;
  }

  if ((*hi).second != nullptr &&
      (*hi).second->_function_signature == remap->_function_signature) {
    // The same function signature has already appeared.  This shouldn't
    // happen.
    nout << "Internal error!  Function signature "
         << remap->_function_signature << " repeated!\n";
    remap->_hash = hash;
    abort();
    return;
  }

  // We have a conflict.  Extend both strings to resolve the ambiguity.
  if ((*hi).second != nullptr) {
    FunctionRemap *other_remap = (*hi).second;
    (*hi).second = nullptr;
    other_remap->_hash +=
      InterrogateBuilder::hash_string(other_remap->_function_signature, 11);
    bool inserted = _wrappers_by_hash.insert
      (WrappersByHash::value_type(other_remap->_hash, other_remap)).second;
    if (!inserted) {
      nout << "Internal error!  Hash " << other_remap->_hash
           << " already appears!\n";
    }
  }

  hash += InterrogateBuilder::hash_string(remap->_function_signature, 11);
  bool inserted = _wrappers_by_hash.insert
    (WrappersByHash::value_type(hash, remap)).second;

  if (!inserted) {
    // Huh.  We still have a conflict.  This should be extremely rare.  Well,
    // just tack on a letter until it's resolved.
    string old_hash = hash;
    for (char ch = 'a'; ch <= 'z' && !inserted; ch++) {
      hash = old_hash + ch;
      inserted = _wrappers_by_hash.insert
        (WrappersByHash::value_type(hash, remap)).second;
    }
    if (!inserted) {
      nout << "Internal error!  Too many conflicts with hash "
           << hash << "\n";
    }
  }

  remap->_hash = hash;
}

/**
 * Generates a string to output a spammy message to notify indicating we have
 * just called this function.
 */
void InterfaceMaker::
write_spam_message(ostream &out, FunctionRemap *remap) const {
  ostringstream strm;
  remap->write_orig_prototype(strm, 0);
  string prototype = strm.str();

  out <<
    "  if (interrogatedb_cat.is_spam()) {\n"
    "    interrogatedb_cat.spam() << \"";

  for (string::const_iterator si = prototype.begin();
       si != prototype.end();
       ++si) {
    switch (*si) {
    case '"':
      out << "\\\"";
      break;

    case '\\':
      out << "\\\\";
      break;

    default:
      out << *si;
    }
  }

  out << "\\n\";\n"
    "  }\n";
}
