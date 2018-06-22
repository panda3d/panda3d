/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file factoryBase.cxx
 * @author drose
 * @date 2000-05-08
 */

#include "factoryBase.h"
#include "indent.h"
#include "config_putil.h"

/**
 * Attempts to create a new instance of some class of the indicated type, or
 * some derivative if necessary.  If an instance of the exact type cannot be
 * created, the specified preferred will specify which derived class will be
 * preferred.
 */
TypedObject *FactoryBase::
make_instance(TypeHandle handle, const FactoryParams &params) {
  TypedObject *instance = nullptr;

  instance = make_instance_exact(handle, params);
  if (instance == nullptr) {
    // Can't create an exact instance; try for a derived type.
    instance = make_instance_more_specific(handle, params);
  }

  if (util_cat.is_debug()) {
    util_cat.debug()
      << "make_instance(" << handle << ", params) returns "
      << (void *)instance;
    if (instance != nullptr) {
      util_cat.debug(false)
        << ", of type " << instance->get_type();
    }
    util_cat.debug(false) << "\n";
  }
  return instance;
}

/**
 * Attempts to create an instance of the type requested, or some base type of
 * the type requested.  Returns the new instance created, or NULL if the
 * instance could not be created.
 */
TypedObject *FactoryBase::
make_instance_more_general(TypeHandle handle, const FactoryParams &params) {
  TypedObject *object = make_instance_exact(handle, params);

  if (object == nullptr) {
    // Recursively search through the entire inheritance tree until we find
    // something we know about.
    if (handle.get_num_parent_classes() == 0) {
      return nullptr;
    }

    int num_parents = handle.get_num_parent_classes();
    for (int i = 0; i < num_parents && object == nullptr; i++) {
      object = make_instance_more_general(handle.get_parent_class(i), params);
    }
  }

  if (util_cat.is_debug()) {
    util_cat.debug()
      << "make_instance(" << handle << ", params) returns "
      << (void *)object;
    if (object != nullptr) {
      util_cat.debug(false)
        << ", of type " << object->get_type();
    }
    util_cat.debug(false) << "\n";
  }

  return object;
}

/**
 * Returns the TypeHandle given, if it is a registered type, or if it is not
 * registered, searches for the nearest ancestor of the indicated type that is
 * registered and returns it.  If no ancestor of the indicated type is
 * registered, returns TypeHandle::none().
 */
TypeHandle FactoryBase::
find_registered_type(TypeHandle handle) {
  Creators::const_iterator ci = _creators.find(handle);
  if (ci != _creators.end()) {
    // This type is registered.
    return handle;
  }

  // Recursively search through the entire inheritance tree until we find
  // something we know about.
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

/**
 * Registers a new kind of thing the Factory will be able to create.
 *
 * @param user_data an optional pointer to be passed along to the function.
 */
void FactoryBase::
register_factory(TypeHandle handle, BaseCreateFunc *func, void *user_data) {
  nassertv(handle != TypeHandle::none());
  nassertv(func != nullptr);

  Creator creator;
  creator._func = func;
  creator._user_data = user_data;
  _creators[handle] = creator;
}

/**
 * Returns the number of different types the Factory knows how to create.
 */
size_t FactoryBase::
get_num_types() const {
  return _creators.size();
}

/**
 * Returns the nth type the Factory knows how to create.  This is not a
 * terribly efficient function; it's included primarily for debugging output.
 * Normally you wouldn't need to traverse the list of the Factory's types.
 */
TypeHandle FactoryBase::
get_type(size_t n) const {
  nassertr(n < get_num_types(), TypeHandle::none());
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

/**
 * Empties the list of preferred types.
 */
void FactoryBase::
clear_preferred() {
  _preferred.clear();
}

/**
 * Adds the indicated type to the end of the list of preferred types.  On the
 * next call to make_instance(), if the exact type requested cannot be
 * created, the preferred types are first tried in the order specified.
 */
void FactoryBase::
add_preferred(TypeHandle handle) {
  nassertv(handle != TypeHandle::none());
  _preferred.push_back(handle);
}

/**
 * Returns the number of types added to the preferred-type list.
 */
size_t FactoryBase::
get_num_preferred() const {
  return _preferred.size();
}

/**
 * Returns the nth type added to the preferred-type list.
 */
TypeHandle FactoryBase::
get_preferred(size_t n) const {
  nassertr(n < get_num_preferred(), TypeHandle::none());
  return _preferred[n];
}

/**
 * Writes a list of all known types the Factory can create to the indicated
 * output stream, one per line.
 */
void FactoryBase::
write_types(std::ostream &out, int indent_level) const {
  Creators::const_iterator ci;
  for (ci = _creators.begin(); ci != _creators.end(); ++ci) {
    indent(out, indent_level) << (*ci).first << "\n";
  }
}

/**
 * Attempts to create an instance of the exact type requested by the given
 * handle.  Returns the new instance created, or NULL if the instance could
 * not be created.
 */
TypedObject *FactoryBase::
make_instance_exact(TypeHandle handle, FactoryParams params) {
  Creators::const_iterator ci = _creators.find(handle);
  if (ci == _creators.end()) {
    return nullptr;
  }

  Creator creator = (*ci).second;
  nassertr(creator._func != nullptr, nullptr);
  params._user_data = creator._user_data;
  return (*creator._func)(params);
}

/**
 * Attempts to create an instance of some derived type of the type requested
 * by the given handle.  Returns the new instance created, or NULL if the
 * instance could not be created.
 */
TypedObject *FactoryBase::
make_instance_more_specific(TypeHandle handle, FactoryParams params) {
  // First, walk through the established preferred list.  Maybe one of these
  // qualifies.

  for (TypeHandle ptype : _preferred) {
    if (ptype.is_derived_from(handle)) {
      TypedObject *object = make_instance_exact(ptype, params);
      if (object != nullptr) {
        return object;
      }
    }
  }

  // No, we couldn't create anything on the preferred list, so create the
  // first thing we know about that derives from the indicated type.
  Creators::const_iterator ci;
  for (ci = _creators.begin(); ci != _creators.end(); ++ci) {
    TypeHandle ctype = (*ci).first;
    if (ctype.is_derived_from(handle)) {
      Creator creator = (*ci).second;
      nassertr(creator._func != nullptr, nullptr);
      params._user_data = creator._user_data;
      TypedObject *object = (*creator._func)(params);
      if (object != nullptr) {
        return object;
      }
    }
  }

  return nullptr;
}
