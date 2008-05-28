// Filename: factoryBase.cxx
// Created by:  drose (08May00)
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

#include "factoryBase.h"
#include "indent.h"
#include "config_util.h"

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FactoryBase::
FactoryBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FactoryBase::
~FactoryBase() {
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::make_instance
//       Access: Public
//  Description: Attempts to create a new instance of some class of
//               the indicated type, or some derivative if necessary.
//               If an instance of the exact type cannot be created,
//               the specified preferred will specify which derived
//               class will be preferred.
////////////////////////////////////////////////////////////////////
TypedObject *FactoryBase::
make_instance(TypeHandle handle, const FactoryParams &params) {
  TypedObject *instance = (TypedObject *)NULL;

  instance = make_instance_exact(handle, params);
  if (instance == (TypedObject *)NULL) {
    // Can't create an exact instance; try for a derived type.
    instance = make_instance_more_specific(handle, params);
  }

  if (util_cat.is_debug()) {
    util_cat.debug()
      << "make_instance(" << handle << ", params) returns "
      << (void *)instance;
    if (instance != (TypedObject *)NULL) {
      util_cat.debug(false)
        << ", of type " << instance->get_type();
    }
    util_cat.debug(false) << "\n";
  }
  return instance;
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::make_instance_more_general
//       Access: Public
//  Description: Attempts to create an instance of the type requested,
//               or some base type of the type requested.  Returns the
//               new instance created, or NULL if the instance could
//               not be created.
////////////////////////////////////////////////////////////////////
TypedObject *FactoryBase::
make_instance_more_general(TypeHandle handle, const FactoryParams &params) {
  TypedObject *object = make_instance_exact(handle, params);

  if (object == (TypedObject *)NULL) {
    // Recursively search through the entire inheritance tree until we
    // find something we know about.
    if (handle.get_num_parent_classes() == 0) {
      return NULL;
    }

    int num_parents = handle.get_num_parent_classes();
    for (int i = 0; i < num_parents && object == (TypedObject *)NULL; i++) {
      object = make_instance_more_general(handle.get_parent_class(i), params);
    }
  }

  if (util_cat.is_debug()) {
    util_cat.debug()
      << "make_instance(" << handle << ", params) returns "
      << (void *)object;
    if (object != (TypedObject *)NULL) {
      util_cat.debug(false)
        << ", of type " << object->get_type();
    }
    util_cat.debug(false) << "\n";
  }

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::find_registered_type
//       Access: Public
//  Description: Returns the TypeHandle given, if it is a registered
//               type, or if it is not registered, searches for the
//               nearest ancestor of the indicated type that is
//               registered and returns it.  If no ancestor of the
//               indicated type is registered, returns
//               TypeHandle::none().
////////////////////////////////////////////////////////////////////
TypeHandle FactoryBase::
find_registered_type(TypeHandle handle) {
  Creators::const_iterator ci = _creators.find(handle);
  if (ci != _creators.end()) {
    // This type is registered.
    return handle;
  }

  // Recursively search through the entire inheritance tree until we
  // find something we know about.
  if (handle.get_num_parent_classes() == 0) {
    return TypeHandle::none();
  }

  int num_parents = handle.get_num_parent_classes();
  for (int i = 0; i < num_parents; i++) {
    TypeHandle result = find_registered_type(handle.get_parent_class(i));
    if (result != TypeHandle::none()) {
      return result;
    }
  }

  // No known types.
  return TypeHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::register_factory
//       Access: Public
//  Description: Registers a new kind of thing the Factory will be
//               able to create.
////////////////////////////////////////////////////////////////////
void FactoryBase::
register_factory(TypeHandle handle, BaseCreateFunc *func) {
  nassertv(handle != TypeHandle::none());
  nassertv(func != (BaseCreateFunc *)NULL);
  _creators[handle] = func;
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::get_num_types
//       Access: Public
//  Description: Returns the number of different types the Factory
//               knows how to create.
////////////////////////////////////////////////////////////////////
int FactoryBase::
get_num_types() const {
  return _creators.size();
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::get_type
//       Access: Public
//  Description: Returns the nth type the Factory knows how to create.
//               This is not a terribly efficient function; it's
//               included primarily for debugging output.  Normally
//               you wouldn't need to traverse the list of the
//               Factory's types.
////////////////////////////////////////////////////////////////////
TypeHandle FactoryBase::
get_type(int n) const {
  nassertr(n >= 0 && n < get_num_types(), TypeHandle::none());
  Creators::const_iterator ci;
  for (ci = _creators.begin(); ci != _creators.end(); ++ci) {
    if (n == 0) {
      return (*ci).first;
    }
    n--;
  }

  // We shouldn't get here.
  nassertr(false, TypeHandle::none());
  return TypeHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::clear_preferred
//       Access: Public
//  Description: Empties the list of preferred types.
////////////////////////////////////////////////////////////////////
void FactoryBase::
clear_preferred() {
  _preferred.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::add_preferred
//       Access: Public
//  Description: Adds the indicated type to the end of the list of
//               preferred types.  On the next call to
//               make_instance(), if the exact type requested cannot
//               be created, the preferred types are first tried in
//               the order specified.
////////////////////////////////////////////////////////////////////
void FactoryBase::
add_preferred(TypeHandle handle) {
  nassertv(handle != TypeHandle::none());
  _preferred.push_back(handle);
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::get_num_preferred
//       Access: Public
//  Description: Returns the number of types added to the
//               preferred-type list.
////////////////////////////////////////////////////////////////////
int FactoryBase::
get_num_preferred() const {
  return _preferred.size();
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::get_preferred
//       Access: Public
//  Description: Returns the nth type added to the preferred-type
//               list.
////////////////////////////////////////////////////////////////////
TypeHandle FactoryBase::
get_preferred(int n) const {
  nassertr(n >= 0 && n < get_num_preferred(), TypeHandle::none());
  return _preferred[n];
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::write_types
//       Access: Public
//  Description: Writes a list of all known types the Factory can
//               create to the indicated output stream, one per line.
////////////////////////////////////////////////////////////////////
void FactoryBase::
write_types(ostream &out, int indent_level) const {
  Creators::const_iterator ci;
  for (ci = _creators.begin(); ci != _creators.end(); ++ci) {
    indent(out, indent_level) << (*ci).first << "\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::Copy Constructor
//       Access: Private
//  Description: Don't copy Factories.
////////////////////////////////////////////////////////////////////
FactoryBase::
FactoryBase(const FactoryBase &) {
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::Copy Assignment Operator
//       Access: Private
//  Description: Don't copy Factories.
////////////////////////////////////////////////////////////////////
void FactoryBase::
operator = (const FactoryBase &) {
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::make_instance_exact
//       Access: Private
//  Description: Attempts to create an instance of the exact type
//               requested by the given handle.  Returns the new
//               instance created, or NULL if the instance could not
//               be created.
////////////////////////////////////////////////////////////////////
TypedObject *FactoryBase::
make_instance_exact(TypeHandle handle, const FactoryParams &params) {
  Creators::const_iterator ci = _creators.find(handle);
  if (ci == _creators.end()) {
    return NULL;
  }

  BaseCreateFunc *func = (BaseCreateFunc *)((*ci).second);
  nassertr(func != (BaseCreateFunc *)NULL, NULL);
  return (*func)(params);
}

////////////////////////////////////////////////////////////////////
//     Function: FactoryBase::make_instance_more_specific
//       Access: Private
//  Description: Attempts to create an instance of some derived type
//               of the type requested by the given handle.  Returns
//               the new instance created, or NULL if the instance
//               could not be created.
////////////////////////////////////////////////////////////////////
TypedObject *FactoryBase::
make_instance_more_specific(TypeHandle handle, const FactoryParams &params) {
  // First, walk through the established preferred list.  Maybe one
  // of these qualifies.

  Preferred::const_iterator pi;
  for (pi = _preferred.begin(); pi != _preferred.end(); ++pi) {
    TypeHandle ptype = (*pi);
    if (ptype.is_derived_from(handle)) {
      TypedObject *object = make_instance_exact(ptype, params);
      if (object != (TypedObject *)NULL) {
        return object;
      }
    }
  }

  // No, we couldn't create anything on the preferred list, so create
  // the first thing we know about that derives from the indicated
  // type.
  Creators::const_iterator ci;
  for (ci = _creators.begin(); ci != _creators.end(); ++ci) {
    TypeHandle ctype = (*ci).first;
    if (ctype.is_derived_from(handle)) {
      BaseCreateFunc *func = (BaseCreateFunc *)((*ci).second);
      nassertr(func != (BaseCreateFunc *)NULL, NULL);
      TypedObject *object = (*func)(params);
      if (object != (TypedObject *)NULL) {
        return object;
      }
    }
  }

  return NULL;
}

