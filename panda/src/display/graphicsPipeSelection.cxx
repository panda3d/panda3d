/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsPipeSelection.cxx
 * @author drose
 * @date 2002-08-15
 */

#include "graphicsPipeSelection.h"
#include "lightMutexHolder.h"
#include "string_utils.h"
#include "filename.h"
#include "load_dso.h"
#include "config_display.h"
#include "typeRegistry.h"
#include "config_putil.h"

#include <algorithm>

using std::string;

GraphicsPipeSelection *GraphicsPipeSelection::_global_ptr = nullptr;

/**
 *
 */
GraphicsPipeSelection::
GraphicsPipeSelection() : _lock("GraphicsPipeSelection") {
  // We declare these variables here instead of in config_display, in case
  // this constructor is running at static init time.
  ConfigVariableString load_display
    ("load-display", "*",
     PRC_DESC("Specify the name of the default graphics display library or "
              "GraphicsPipe to load.  It is the name of a shared library (or * for "
              "all libraries named in aux-display), optionally followed by the "
              "name of the particular GraphicsPipe class to create."));

  ConfigVariableList aux_display
    ("aux-display",
     PRC_DESC("Names each of the graphics display libraries that are available on "
              "a particular platform.  This variable may be repeated several "
              "times.  These libraries will be tried one at a time if the library "
              "specified by load_display cannot be loaded."));

  _default_display_module = load_display.get_word(0);
  _default_pipe_name = load_display.get_word(1);

  if (_default_display_module == "*") {
    // '*' or empty string is the key for all display modules.
    _default_display_module = string();

  } else if (!_default_display_module.empty()) {
    _display_modules.push_back(_default_display_module);
  }

  // Also get the set of modules named in the various aux-display Config
  // variables.  We'll want to know this when we call load_modules() later.
  size_t num_aux = aux_display.get_num_unique_values();
  for (size_t i = 0; i < num_aux; ++i) {
    string name = aux_display.get_unique_value(i);
    if (name != _default_display_module) {
      _display_modules.push_back(name);
    }
  }

  _default_module_loaded = false;
}

/**
 *
 */
GraphicsPipeSelection::
~GraphicsPipeSelection() {
}

/**
 * Returns the number of different types of GraphicsPipes that are available
 * to create through this interface.
 */
int GraphicsPipeSelection::
get_num_pipe_types() const {
  load_default_module();

  int result;
  {
    LightMutexHolder holder(_lock);
    result = _pipe_types.size();
  }
  return result;
}

/**
 * Returns the nth type of GraphicsPipe available through this interface.
 */
TypeHandle GraphicsPipeSelection::
get_pipe_type(int n) const {
  load_default_module();

  TypeHandle result;
  {
    LightMutexHolder holder(_lock);
    if (n >= 0 && n < (int)_pipe_types.size()) {
      result = _pipe_types[n]._type;
    } else {
      result = TypeHandle::none();
    }
  }
  return result;
}

/**
 * Writes a list of the currently known GraphicsPipe types to nout, for the
 * user's information.
 */
void GraphicsPipeSelection::
print_pipe_types() const {
  load_default_module();

  LightMutexHolder holder(_lock);
  nout << "Known pipe types:" << std::endl;
  for (const PipeType &pipe_type : _pipe_types) {
    nout << "  " << pipe_type._type << "\n";
  }
  if (_display_modules.empty()) {
    nout << "(all display modules loaded.)\n";
  } else {
    nout << "(" << _display_modules.size()
         << " aux display modules not yet loaded.)\n";
  }
}

/**
 * Creates a new GraphicsPipe of the indicated type (or a type more specific
 * than the indicated type, if necessary) and returns it.  Returns NULL if the
 * type cannot be matched.
 *
 * If the type is not already defined, this will implicitly load the named
 * module, or if module_name is empty, it will call load_aux_modules().
 */
PT(GraphicsPipe) GraphicsPipeSelection::
make_pipe(const string &type_name, const string &module_name) {
  TypeRegistry *type_reg = TypeRegistry::ptr();

  // First, see if the type is already available.
  TypeHandle type = type_reg->find_type(type_name);

  // If it isn't, try the named module.
  if (type == TypeHandle::none()) {
    if (!module_name.empty()) {
      load_named_module(module_name);
      type = type_reg->find_type(type_name);
    }
  }

  // If that didn't help, try the default module.
  if (type == TypeHandle::none()) {
    load_default_module();
    type = type_reg->find_type(type_name);
  }

  // Still not enough, try all modules.
  if (type == TypeHandle::none()) {
    load_aux_modules();
    type = type_reg->find_type(type_name);
  }

  if (type == TypeHandle::none()) {
    return nullptr;
  }

  return make_pipe(type);
}

/**
 * Creates a new GraphicsPipe of the indicated type (or a type more specific
 * than the indicated type, if necessary) and returns it.  Returns NULL if the
 * type cannot be matched.
 */
PT(GraphicsPipe) GraphicsPipeSelection::
make_pipe(TypeHandle type) {
  LightMutexHolder holder(_lock);

  // First, look for an exact match of the requested type.
  for (const PipeType &ptype : _pipe_types) {
    if (ptype._type == type) {
      // Here's an exact match.
      PT(GraphicsPipe) pipe = (*ptype._constructor)();
      if (pipe != nullptr) {
        return pipe;
      }
    }
  }

  // Now look for a more-specific type.
  for (const PipeType &ptype : _pipe_types) {
    if (ptype._type.is_derived_from(type)) {
      // Here's an approximate match.
      PT(GraphicsPipe) pipe = (*ptype._constructor)();
      if (pipe != nullptr) {
        return pipe;
      }
    }
  }

  // Couldn't find any match; load the default module and try again.
  load_default_module();
  for (const PipeType &ptype : _pipe_types) {
    if (ptype._type.is_derived_from(type)) {
      // Here's an approximate match.
      PT(GraphicsPipe) pipe = (*ptype._constructor)();
      if (pipe != nullptr) {
        return pipe;
      }
    }
  }

  // Couldn't find a matching pipe type.
  return nullptr;
}

/**
 * Returns a new GraphicsPipe of a type defined by the indicated module.
 * Returns NULL if the module is not found or does not properly recommend a
 * GraphicsPipe.
 */
PT(GraphicsPipe) GraphicsPipeSelection::
make_module_pipe(const string &module_name) {
  if (display_cat.is_debug()) {
    display_cat.debug()
      << "make_module_pipe(" << module_name << ")\n";
  }

  TypeHandle pipe_type = load_named_module(module_name);
  if (pipe_type == TypeHandle::none()) {
    return nullptr;
  }

  return make_pipe(pipe_type);
}

/**
 * Creates a new GraphicsPipe of some arbitrary type.  The user may specify a
 * preference using the Configrc file; otherwise, one will be chosen
 * arbitrarily.
 */
PT(GraphicsPipe) GraphicsPipeSelection::
make_default_pipe() {
  load_default_module();

  LightMutexHolder holder(_lock);

  if (!_default_pipe_name.empty()) {
    // First, look for an exact match of the default type name from the
    // Configrc file (excepting case and hyphenunderscore).
    for (const PipeType &ptype : _pipe_types) {
      if (cmp_nocase_uh(ptype._type.get_name(), _default_pipe_name) == 0) {
        // Here's an exact match.
        PT(GraphicsPipe) pipe = (*ptype._constructor)();
        if (pipe != nullptr) {
          return pipe;
        }
      }
    }

    // No match; look for a substring match.
    string preferred_name = downcase(_default_pipe_name);
    for (const PipeType &ptype : _pipe_types) {
      string ptype_name = downcase(ptype._type.get_name());
      if (ptype_name.find(preferred_name) != string::npos) {
        // Here's a substring match.
        PT(GraphicsPipe) pipe = (*ptype._constructor)();
        if (pipe != nullptr) {
          return pipe;
        }
      }
    }
  }

  // Couldn't find a matching pipe type; choose the first one on the list.
  for (const PipeType &ptype : _pipe_types) {
    PT(GraphicsPipe) pipe = (*ptype._constructor)();
    if (pipe != nullptr) {
      return pipe;
    }
  }

  // Nothing.  Probably the list was empty.
  return nullptr;
}

/**
 * Loads all the modules named in the aux-display Configrc variable, making as
 * many graphics pipes as possible available.
 */
void GraphicsPipeSelection::
load_aux_modules() {
  for (const string &module : _display_modules) {
    load_named_module(module);
  }

  _display_modules.clear();
  _default_module_loaded = true;
}

/**
 * Adds a new kind of GraphicsPipe to the list of available pipes for
 * creation.  Normally, this is called at static init type by the various
 * shared libraries as they are linked in.  Returns true on success, false on
 * failure.
 */
bool GraphicsPipeSelection::
add_pipe_type(TypeHandle type, PipeConstructorFunc *func) {
  nassertr(func != nullptr, false);

  if (!type.is_derived_from(GraphicsPipe::get_class_type())) {
    display_cat->warning()
      << "Attempt to register " << type << " as a GraphicsPipe type.\n";
    return false;
  }

  // First, make sure we don't already have a GraphicsPipe of this type.
  LightMutexHolder holder(_lock);
  for (const PipeType &ptype : _pipe_types) {
    if (ptype._type == type) {
      display_cat->warning()
        << "Attempt to register GraphicsPipe type " << type
        << " more than once.\n";
      return false;
    }
  }

  if (display_cat->is_debug()) {
    display_cat->debug()
      << "Registering " << type << " as a GraphicsPipe type.\n";
  }

  // Ok, now add a new entry.
  _pipe_types.push_back(PipeType(type, func));

  return true;
}


/**
 * Loads the particular display module listed in the load-display Configrc
 * variable, which should default the default pipe time.  If this string is
 * empty or "*", loads all modules named in aux-display.
 */
void GraphicsPipeSelection::
do_load_default_module() {
  if (_default_display_module.empty()) {
    load_aux_modules();
    return;
  }

  load_named_module(_default_display_module);

  DisplayModules::iterator di =
    std::find(_display_modules.begin(), _display_modules.end(),
              _default_display_module);
  if (di != _display_modules.end()) {
    _display_modules.erase(di);
  }

  _default_module_loaded = true;

  if (_pipe_types.empty()) {
    // If we still don't have any pipes after loading the default module,
    // automatically load the aux modules.
    load_aux_modules();
  }
}

/**
 * Loads the indicated display module by looking for a matching .dll or .so
 * file.  Returns the TypeHandle recommended by the module, or
 * TypeHandle::none() on failure.
 */
TypeHandle GraphicsPipeSelection::
load_named_module(const string &name) {
  LightMutexHolder holder(_loaded_modules_lock);

  LoadedModules::iterator mi = _loaded_modules.find(name);
  if (mi != _loaded_modules.end()) {
    // We have previously loaded this module.  Don't attempt to re-load it.
    return (*mi).second._default_pipe_type;
  }

  // We have not yet loaded this module.  Load it now.
  Filename dlname = Filename::dso_filename("lib" + name + ".so");
  display_cat.info()
    << "loading display module: " << dlname.to_os_specific() << std::endl;
  void *handle = load_dso(get_plugin_path().get_value(), dlname);
  if (handle == nullptr) {
    std::string error = load_dso_error();
    display_cat.warning()
      << "Unable to load " << dlname.get_basename() << ": " << error << std::endl;
    return TypeHandle::none();
  }

  // Now get the module's recommended pipe type.  This requires calling a
  // specially-named function that should have been exported from the module.
  string symbol_name = "get_pipe_type_" + name;
  void *dso_symbol = get_dso_symbol(handle, symbol_name);
  if (display_cat.is_debug()) {
    display_cat.debug()
      << "symbol of " << symbol_name << " = " << dso_symbol << "\n";
  }

  TypeHandle pipe_type = TypeHandle::none();

  if (dso_symbol == nullptr) {
    // Couldn't find the module function.
    display_cat.warning()
      << "Unable to find " << symbol_name << " in " << dlname.get_basename()
      << "\n";

  } else {
    // We successfully loaded the module, and we found the get_pipe_type_*
    // recommendation function.  Call it to figure out what pipe type we
    // should expect.
    typedef int FuncType();
    int pipe_type_index = (*(FuncType *)dso_symbol)();
    if (display_cat.is_debug()) {
      display_cat.debug()
        << "pipe_type_index = " << pipe_type_index << "\n";
    }

    if (pipe_type_index != 0) {
      TypeRegistry *type_reg = TypeRegistry::ptr();
      pipe_type = type_reg->find_type_by_id(pipe_type_index);
      if (display_cat.is_debug()) {
        display_cat.debug()
          << "pipe_type = " << pipe_type << "\n";
      }
    }
  }

  if (pipe_type == TypeHandle::none()) {
    // The recommendation function returned a bogus type index, or the
    // function didn't work at all.  We can't safely unload the module,
    // though, because it may have assigned itself into the
    // GraphicsPipeSelection table.  So we carry on.
    display_cat.warning()
      << "No default pipe type available for " << dlname.get_basename()
      << "\n";
  }

  LoadedModule &module = _loaded_modules[name];
  module._module_name = name;
  module._module_handle = handle;
  module._default_pipe_type = pipe_type;

  return pipe_type;
}
