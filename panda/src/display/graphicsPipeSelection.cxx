// Filename: graphicsPipeSelection.cxx
// Created by:  drose (15Aug02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "graphicsPipeSelection.h"
#include "mutexHolder.h"
#include "string_utils.h"
#include "filename.h"
#include "load_dso.h"
#include "config_display.h"

GraphicsPipeSelection *GraphicsPipeSelection::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsPipeSelection::
GraphicsPipeSelection() {
  _resolved_modules = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsPipeSelection::
~GraphicsPipeSelection() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::get_num_pipe_types
//       Access: Published
//  Description: Returns the number of different types of
//               GraphicsPipes that are available to create through
//               this interface.
////////////////////////////////////////////////////////////////////
int GraphicsPipeSelection::
get_num_pipe_types() const {
  resolve_modules();

  int result;
  {
    MutexHolder holder(_lock);
    result = _pipe_types.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::get_pipe_type
//       Access: Published
//  Description: Returns the nth type of GraphicsPipe available
//               through this interface.
////////////////////////////////////////////////////////////////////
TypeHandle GraphicsPipeSelection::
get_pipe_type(int n) const {
  resolve_modules();

  TypeHandle result;
  {
    MutexHolder holder(_lock);
    if (n >= 0 && n < (int)_pipe_types.size()) {
      result = _pipe_types[n]._type;
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::make_pipe
//       Access: Published
//  Description: Creates a new GraphicsPipe of the indicated type (or
//               a type more specific than the indicated type, if
//               necessary) and returns it.  Returns NULL if the type
//               cannot be matched.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) GraphicsPipeSelection::
make_pipe(TypeHandle type) {
  resolve_modules();

  MutexHolder holder(_lock);
  PipeTypes::const_iterator ti;

  // First, look for an exact match of the requested type.
  for (ti = _pipe_types.begin(); ti != _pipe_types.end(); ++ti) {
    const PipeType &ptype = (*ti);
    if (ptype._type == type) {
      // Here's an exact match.
      PT(GraphicsPipe) pipe = (*ptype._constructor)();
      if (pipe != (GraphicsPipe *)NULL) {
        return pipe;
      }
    }
  }

  // Now look for a more-specific type.
  for (ti = _pipe_types.begin(); ti != _pipe_types.end(); ++ti) {
    const PipeType &ptype = (*ti);
    if (ptype._type.is_derived_from(type)) {
      // Here's an approximate match.
      PT(GraphicsPipe) pipe = (*ptype._constructor)();
      if (pipe != (GraphicsPipe *)NULL) {
        return pipe;
      }
    }
  }

  // Couldn't find a matching pipe type.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::make_default_pipe
//       Access: Published
//  Description: Creates a new GraphicsPipe of some arbitrary type.
//               The user may specify a preference using the Configrc
//               file; otherwise, one will be chosen arbitrarily.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) GraphicsPipeSelection::
make_default_pipe() {
  resolve_modules();

  MutexHolder holder(_lock);
  PipeTypes::const_iterator ti;

  // First, look for an exact match of the requested type (excepting
  // case and hyphen/underscore).
  for (ti = _pipe_types.begin(); ti != _pipe_types.end(); ++ti) {
    const PipeType &ptype = (*ti);
    if (cmp_nocase_uh(ptype._type.get_name(), preferred_pipe) == 0) {
      // Here's an exact match.
      PT(GraphicsPipe) pipe = (*ptype._constructor)();
      if (pipe != (GraphicsPipe *)NULL) {
        return pipe;
      }
    }
  }

  // No match; look for a substring match.
  string preferred_name = downcase(preferred_pipe);
  for (ti = _pipe_types.begin(); ti != _pipe_types.end(); ++ti) {
    const PipeType &ptype = (*ti);
    string ptype_name = downcase(ptype._type.get_name());
    if (ptype_name.find(preferred_name) != string::npos) {
      // Here's a substring match.
      PT(GraphicsPipe) pipe = (*ptype._constructor)();
      if (pipe != (GraphicsPipe *)NULL) {
        return pipe;
      }
    }
  }

  // Couldn't find a matching pipe type; choose one arbitrarily. 
  for (ti = _pipe_types.begin(); ti != _pipe_types.end(); ++ti) {
    const PipeType &ptype = (*ti);
    PT(GraphicsPipe) pipe = (*ptype._constructor)();
    if (pipe != (GraphicsPipe *)NULL) {
      return pipe;
    }
  }

  // Nothing.  Probably the list was empty.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::add_pipe_type
//       Access: Public
//  Description: Adds a new kind of GraphicsPipe to the list of
//               available pipes for creation.  Normally, this is
//               called at static init type by the various shared
//               libraries as they are linked in.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool GraphicsPipeSelection::
add_pipe_type(TypeHandle type, PipeConstructorFunc *func) {
  if (!type.is_derived_from(GraphicsPipe::get_class_type())) {
    display_cat.warning()
      << "Attempt to register " << type << " as a GraphicsPipe type.\n";
    return false;
  }
  
  // First, make sure we don't already have a GraphicsPipe of this
  // type.
  MutexHolder holder(_lock);
  PipeTypes::const_iterator ti;
  for (ti = _pipe_types.begin(); ti != _pipe_types.end(); ++ti) {
    const PipeType &ptype = (*ti);
    if (ptype._type == type) {
      display_cat.warning()
        << "Attempt to register GraphicsPipe type " << type
        << " more than once.\n";
      return false;
    }
  }

  // Ok, now add a new entry.
  _pipe_types.push_back(PipeType(type, func));

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipeSelection::do_resolve_modules
//       Access: Private
//  Description: Loads the shared objects listed in the load-display
//               Configrc variable, which should make all of the
//               dynamic GraphicsPipes available.
////////////////////////////////////////////////////////////////////
void GraphicsPipeSelection::
do_resolve_modules() {
  Config::ConfigTable::Symbol::iterator ci;

  // Build up a set of the modules we've already loaded as we go, so
  // we don't attempt to load a given module more than once.
  pset<string> already_loaded;

  for (ci = display_modules_begin(); ci != display_modules_end(); ++ci) {
    string name = (*ci).Val();
    if (already_loaded.insert(name).second) {
      Filename dlname = Filename::dso_filename("lib" + name + ".so");
      display_cat.info()
        << "loading display module: " << dlname.to_os_specific() << endl;
      void *tmp = load_dso(dlname);
      if (tmp == (void *)NULL) {
        display_cat.info()
          << "Unable to load: " << load_dso_error() << endl;
      }
    }
  }

  _resolved_modules = true;
}

