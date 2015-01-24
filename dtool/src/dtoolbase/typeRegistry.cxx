// Filename: typeRegistry.cxx
// Created by:  drose (06Aug01)
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

#include "typeRegistry.h"
#include "typeRegistryNode.h"
#include "typeHandle.h"
#include "typedObject.h"
#include "indent.h"
#include "numeric_types.h"

#include <algorithm>

MutexImpl *TypeRegistry::_lock = NULL;
TypeRegistry *TypeRegistry::_global_pointer = NULL;

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::register_type
//       Access: Public
//  Description: Creates a new Type of the given name and assigns a
//               unique value to the type_handle.  All type names must
//               be unique.  If the type name has already been used,
//               the supplied type_handle value must match the name's
//               assigned type_handle or an error is triggered.
//               Returns true if the name wasn't defined before, false
//               if it was.
////////////////////////////////////////////////////////////////////
bool TypeRegistry::
register_type(TypeHandle &type_handle, const string &name) {
  _lock->acquire();

  if (type_handle != TypeHandle::none()) {
    // Here's a type that was already registered.  Just make sure
    // everything's still kosher.
    TypeRegistryNode *rnode = look_up(type_handle, NULL);
    if (&type_handle == &rnode->_ref) {
      // No problem.
      _lock->release();
      assert(rnode->_name == name);
      return false;
    }
  }

  NameRegistry::iterator ri;
  ri = _name_registry.find(name);

  if (ri == _name_registry.end()) {
    // The name was not already used; this is the first time this
    // class has been defined.

    TypeHandle new_handle;
    new_handle._index = _handle_registry.size();

    TypeRegistryNode *rnode = new TypeRegistryNode(new_handle, name, type_handle);
    _handle_registry.push_back(rnode);
    _name_registry[name] = rnode;
    _derivations_fresh = false;

    type_handle = new_handle;
    _lock->release();
    return true;
  }
  TypeRegistryNode *rnode = (*ri).second;
  assert(rnode->_name == (*ri).first);
  assert(rnode->_handle._index >= 0 &&
           rnode->_handle._index < (int)_handle_registry.size());
  assert(_handle_registry[rnode->_handle._index] == rnode);
  assert(rnode->_handle._index != 0);

  // The name was previously used; make sure the type_handle matches.
  if (&type_handle == &rnode->_ref) {
    // Ok, this was just a repeated attempt to register the same type.

    if (type_handle == rnode->_handle) {
      // No problem.
      _lock->release();
      return false;
    }
    // But wait--the type_handle has changed!  We kept a reference to
    // the static _type_handle member in the class that was passed in
    // at the first call to register_type(), and we got the same
    // reference passed in this time, but now it's different!  Bad
    // juju.
    cerr << "Reregistering " << name << "\n";
    type_handle = rnode->_handle;
    _lock->release();
    return false;
  }

  if (type_handle != rnode->_handle) {
    // Hmm, we seem to have a contradictory type registration!
    cerr
      << "Attempt to register type " << name << " more than once!\n";

    // This is invalid, but we'll allow it anyway.  It seems to happen
    // for some reason under GNU libc5 that we occasionally end up
    // with two legitimate copies of the same class object in
    // memory--each with its own static _type_handle member.

    type_handle = rnode->_handle;
  }
  _lock->release();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::register_dynamic_type
//       Access: Public
//  Description: Registers a new type on-the-fly, presumably at
//               runtime.  A new TypeHandle is returned if the
//               typename was not seen before; otherwise the same
//               TypeHandle that was last used for this typename is
//               returned.
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
register_dynamic_type(const string &name) {
  _lock->acquire();

  NameRegistry::iterator ri;
  ri = _name_registry.find(name);

  if (ri == _name_registry.end()) {
    // The name was not already used; this is the first time this
    // class has been defined.

    // We must dynamically allocate a new handle so the TypeRegistryNode
    // has something unique to point to.  This doesn't really mean
    // anything, though.
    TypeHandle *new_handle = new TypeHandle;
    new_handle->_index = _handle_registry.size();

    TypeRegistryNode *rnode = new TypeRegistryNode(*new_handle, name, *new_handle);
    _handle_registry.push_back(rnode);
    _name_registry[name] = rnode;
    _derivations_fresh = false;

    _lock->release();
    return *new_handle;
  }

  // Return the TypeHandle previously obtained.
  TypeRegistryNode *rnode = (*ri).second;
  TypeHandle handle = rnode->_handle;
  _lock->release();
  return handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::record_derivation
//       Access: Public
//  Description: Records that the type referenced by child inherits
//               directly from the type referenced by parent.  In the
//               event of multiple inheritance, this should be called
//               once for each parent class.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
record_derivation(TypeHandle child, TypeHandle parent) {
  _lock->acquire();

  TypeRegistryNode *cnode = look_up(child, NULL);
  assert(cnode != (TypeRegistryNode *)NULL);
  TypeRegistryNode *pnode = look_up(parent, NULL);
  assert(pnode != (TypeRegistryNode *)NULL);

  // First, we'll just run through the list to make sure we hadn't
  // already made this connection.
  TypeRegistryNode::Classes::iterator ni;
  ni = find(cnode->_parent_classes.begin(), cnode->_parent_classes.end(),
            pnode);

  if (ni == cnode->_parent_classes.end()) {
    cnode->_parent_classes.push_back(pnode);
    pnode->_child_classes.push_back(cnode);
    _derivations_fresh = false;
  }

  _lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::record_alternate_name
//       Access: Public
//  Description: Indicates an alternate name for the same type.  This
//               is particularly useful when a type has changed names,
//               since the type is stored in a Bam file by name;
//               setting the original name as the alternate will allow
//               the type to be correctly read from old Bam files.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
record_alternate_name(TypeHandle type, const string &name) {
  _lock->acquire();

  TypeRegistryNode *rnode = look_up(type, (TypedObject *)NULL);
  if (rnode != (TypeRegistryNode *)NULL) {
    NameRegistry::iterator ri =
      _name_registry.insert(NameRegistry::value_type(name, rnode)).first;

    if ((*ri).second != rnode) {
      _lock->release();
      cerr
        << "Name " << name << " already assigned to TypeHandle "
        << rnode->_name << "; cannot reassign to " << type << "\n";
      return;
    }

  }

  _lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::find_type
//       Access: Published
//  Description: Looks for a previously-registered type of the given
//               name.  Returns its TypeHandle if it exists, or
//               TypeHandle::none() if there is no such type.
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
find_type(const string &name) const {
  _lock->acquire();

  TypeHandle handle = TypeHandle::none();
  NameRegistry::const_iterator ri;
  ri = _name_registry.find(name);
  if (ri != _name_registry.end()) {
    handle = (*ri).second->_handle;
  }
  _lock->release();

  return handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::find_type_by_id
//       Access: Published
//  Description: Looks for a previously-registered type with the given
//               id number (as returned by TypeHandle::get_index()).
//               Returns its TypeHandle if it exists, or
//               TypeHandle::none() if there is no such type.
///////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
find_type_by_id(int id) const {
  if (id < 0 ||id >= (int)_handle_registry.size()) {
    cerr
      << "Invalid TypeHandle index " << id
      << "!  Is memory corrupt?\n";
    return TypeHandle::none();
  }

  return _handle_registry[id]->_handle;
}


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_name
//       Access: Published
//  Description: Returns the name of the indicated type.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
string TypeRegistry::
get_name(TypeHandle type, TypedObject *object) const {
  _lock->acquire();
  TypeRegistryNode *rnode = look_up(type, object);
  assert(rnode != (TypeRegistryNode *)NULL);
  string name = rnode->_name;
  _lock->release();

  return name;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::is_derived_from
//       Access: Published
//  Description: Returns true if the first type is derived from the
//               second type, false otherwise.
//
//               The "child_object" pointer is an optional pointer to
//               the TypedObject class that owns the child TypeHandle.
//               It is only used in case the TypeHandle is
//               inadvertently undefined.
//
//               This function definition follows the definitions for
//               look_up() and freshen_derivations() just to maximize
//               the chance the the compiler will be able to inline
//               the above functions.  Yeah, a compiler shouldn't
//               care, but there's a big different between "shouldn't"
//               and "doesn't".
////////////////////////////////////////////////////////////////////
bool TypeRegistry::
is_derived_from(TypeHandle child, TypeHandle base,
                TypedObject *child_object) {
  _lock->acquire();

  const TypeRegistryNode *child_node = look_up(child, child_object);
  const TypeRegistryNode *base_node = look_up(base, (TypedObject *)NULL);

  assert(child_node != (TypeRegistryNode *)NULL);
  assert(base_node != (TypeRegistryNode *)NULL);

  freshen_derivations();

  bool result = TypeRegistryNode::is_derived_from(child_node, base_node);
  _lock->release();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_num_type_handles
//       Access: Published
//  Description: Returns the total number of unique TypeHandles in the
//               system.
////////////////////////////////////////////////////////////////////
int TypeRegistry::
get_num_typehandles() {
  _lock->acquire();
  int num_types = (int)_handle_registry.size();
  _lock->release();
  return num_types;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_typehandle
//       Access: Published
//  Description: Returns the nth TypeHandle in the system.  See
//               get_num_typehandles().
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
get_typehandle(int n) {
  _lock->acquire();
  TypeRegistryNode *rnode = NULL;
  if (n >= 0 && n < (int)_handle_registry.size()) {
    rnode = _handle_registry[n];
  }
  _lock->release();

  if (rnode != (TypeRegistryNode *)NULL) {
    return rnode->_handle;
  }

  return TypeHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_num_root_classes
//       Access: Published
//  Description: Returns the number of root classes--that is, classes
//               that do not inherit from any other classes--known in
//               the system.
////////////////////////////////////////////////////////////////////
int TypeRegistry::
get_num_root_classes() {
  _lock->acquire();
  freshen_derivations();
  int num_roots = _root_classes.size();
  _lock->release();
  return num_roots;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_root_class
//       Access: Published
//  Description: Returns the nth root class in the system.  See
//               get_num_root_classes().
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
get_root_class(int n) {
  _lock->acquire();
  freshen_derivations();
  TypeHandle handle;
  if (n >= 0 && n < (int)_root_classes.size()) {
    handle = _root_classes[n]->_handle;
  }
  _lock->release();

  return handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_num_parent_classes
//       Access: Published
//  Description: Returns the number of parent classes that the
//               indicated type is known to have.  This may then be
//               used to index into get_parent_class().  The result
//               will be 0 if this class does not inherit from any
//               other classes, 1 if normal, single inheritance is in
//               effect, or greater than one if multiple inheritance
//               is in effect.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
int TypeRegistry::
get_num_parent_classes(TypeHandle child, TypedObject *child_object) const {
  _lock->acquire();
  TypeRegistryNode *rnode = look_up(child, child_object);
  assert(rnode != (TypeRegistryNode *)NULL);
  int num_parents = rnode->_parent_classes.size();
  _lock->release();
  return num_parents;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_parent_class
//       Access: Published
//  Description: Returns the nth parent class of this type.  The index
//               should be in the range 0 <= index <
//               get_num_parent_classes().
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
get_parent_class(TypeHandle child, int index) const {
  _lock->acquire();
  TypeHandle handle;
  TypeRegistryNode *rnode = look_up(child, (TypedObject *)NULL);
  assert(rnode != (TypeRegistryNode *)NULL);
  if (index >= 0 && index < (int)rnode->_parent_classes.size()) {
    handle = rnode->_parent_classes[index]->_handle;
  }
  _lock->release();
  return handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_num_child_classes
//       Access: Published
//  Description: Returns the number of child classes that the
//               indicated type is known to have.  This may then be
//               used to index into get_child_class().
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
int TypeRegistry::
get_num_child_classes(TypeHandle child, TypedObject *child_object) const {
  _lock->acquire();
  TypeRegistryNode *rnode = look_up(child, child_object);
  assert(rnode != (TypeRegistryNode *)NULL);
  int num_children = rnode->_child_classes.size();
  _lock->release();
  return num_children;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_child_class
//       Access: Published
//  Description: Returns the nth child class of this type.  The index
//               should be in the range 0 <= index <
//               get_num_child_classes().
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
get_child_class(TypeHandle child, int index) const {
  _lock->acquire();
  TypeHandle handle;
  TypeRegistryNode *rnode = look_up(child, (TypedObject *)NULL);
  assert(rnode != (TypeRegistryNode *)NULL);
  if (index >= 0 && index < (int)rnode->_child_classes.size()) {
    handle = rnode->_child_classes[index]->_handle;
  }
  _lock->release();
  return handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_parent_towards
//       Access: Published
//  Description: Returns the parent of the indicated child class that
//               is in a direct line of inheritance to the indicated
//               ancestor class.  This is useful in the presence of
//               multiple inheritance to try to determine what
//               properties an unknown type may have.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
get_parent_towards(TypeHandle child, TypeHandle base,
                   TypedObject *child_object) {
  _lock->acquire();
  TypeHandle handle;
  const TypeRegistryNode *child_node = look_up(child, child_object);
  const TypeRegistryNode *base_node = look_up(base, NULL);
  assert(child_node != (TypeRegistryNode *)NULL &&
         base_node != (TypeRegistryNode *)NULL);
  freshen_derivations();
  handle = TypeRegistryNode::get_parent_towards(child_node, base_node);
  _lock->release();
  return handle;
}


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::reregister_types
//       Access: Published, Static
//  Description: Walks through the TypeRegistry tree and makes sure
//               that each type that was previously registered is
//               *still* registered.  This seems to get broken in
//               certain circumstances when compiled against libc5--it
//               is as if the static initializer stomps on the
//               _type_handle values of each class after they've been
//               registered.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
reregister_types() {
  init_lock();
  _lock->acquire();
  HandleRegistry::iterator ri;
  TypeRegistry *reg = ptr();
  for (ri = reg->_handle_registry.begin();
       ri != reg->_handle_registry.end();
       ++ri) {
    TypeRegistryNode *rnode = (*ri);
    if (rnode != NULL && rnode->_handle != rnode->_ref) {
      cerr << "Reregistering " << rnode->_name << "\n";
    }
  }
  _lock->release();
}


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::write
//       Access: Published
//  Description: Makes an attempt to format the entire TypeRegistry in
//               a nice way that shows the derivation tree as
//               intelligently as possible.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
write(ostream &out) const {
  _lock->acquire();
  do_write(out);
  _lock->release();
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::ptr
//       Access: Published, Static
//  Description: Returns the pointer to the global TypeRegistry
//               object.
////////////////////////////////////////////////////////////////////
TypeRegistry *TypeRegistry::
ptr() {
  init_lock();
  _lock->acquire();
  if (_global_pointer == NULL) {
    init_global_pointer();
  }
  _lock->release();
  return _global_pointer;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
TypeRegistry::
TypeRegistry() {
  // We'll start out our handle_registry with a default entry for the
  // TypeHandles whose index number is zero, and are therefore
  // (probably) uninitialized.
  _handle_registry.push_back(NULL);

  _derivations_fresh = false;

  // Here's a few sanity checks on the sizes of our words.  We have to
  // put it here, at runtime, since there doesn't appear to be a
  // cross-platform compile-time way to verify that we've chosen the
  // right word sizes.
  assert(sizeof(PN_uint8) == 1 && sizeof(PN_int8) == 1);
  assert(sizeof(PN_uint16) == 2 && sizeof(PN_int16) == 2);
  assert(sizeof(PN_uint32) == 4 && sizeof(PN_int32) == 4);
  assert(sizeof(PN_uint64) == 8 && sizeof(PN_int64) == 8);

  assert(sizeof(PN_float32) == 4);
  assert(sizeof(PN_float64) == 8);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::init_global_pointer
//       Access: Private, Static
//  Description: Constructs the TypeRegistry object for the first
//               time.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
init_global_pointer() {
  init_memory_hook();
  _global_pointer = new TypeRegistry;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::rebuild_derivations
//       Access: Private
//  Description: Rebuilds the derivation data structures after some
//               derivation relationship has been modified, so that
//               class relationships can quickly be determined.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
rebuild_derivations() {
  // First, remove all of the old data from the last type
  // rebuild_derivations() was called.
  _root_classes.clear();

  HandleRegistry::iterator hi;
  for (hi = _handle_registry.begin();
       hi != _handle_registry.end();
       ++hi) {
    TypeRegistryNode *node = *hi;
    if (node != (TypeRegistryNode *)NULL) {
      node->clear_subtree();
    }
  }

  // Start by getting the list of root classes: those classes which do
  // not derive from anything.
  for (hi = _handle_registry.begin();
       hi != _handle_registry.end();
       ++hi) {
    TypeRegistryNode *node = *hi;
    if (node != NULL && node->_parent_classes.empty()) {
      _root_classes.push_back(node);

      // Also, for each root class, define a subtree.
      node->define_subtree();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::do_write
//       Access: Private
//  Description: The private implementation of write(), this assumes
//               the lock is already held.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
do_write(ostream &out) const {
  // Recursively write out the tree, starting from each node that has
  // no parent.
  HandleRegistry::const_iterator hi;
  for (hi = _handle_registry.begin();
       hi != _handle_registry.end();
       ++hi) {
    const TypeRegistryNode *root = *hi;
    if (root != NULL && root->_parent_classes.empty()) {
      write_node(out, 2, root);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::write_node
//       Access: Private
//  Description: Writes a single TypeRegistryNode out, along with all of
//               its descendants.  Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
write_node(ostream &out, int indent_level, const TypeRegistryNode *node) const {
  indent(out, indent_level) << node->_handle.get_index() << " " << node->_name;
  if (!node->_parent_classes.empty()) {
    out << " : " << node->_parent_classes[0]->_name;
    for (int pi = 1; pi < (int)node->_parent_classes.size(); pi++) {
      out << ", " << node->_parent_classes[pi]->_name;
    }
  }
  out << "\n";

  for (int i = 0; i < (int)node->_child_classes.size(); i++) {
    write_node(out, indent_level + 2, node->_child_classes[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::look_up_invalid
//       Access: Private
//  Description: Called by look_up when it detects an invalid
//               TypeHandle pointer.  In non-release builds, this
//               method will do what it can to recover from this
//               and initialize the type anyway.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
TypeRegistryNode *TypeRegistry::
look_up_invalid(TypeHandle handle, TypedObject *object) const {
#ifndef NDEBUG
  if (handle._index == 0) {
    // The TypeHandle is unregistered.  This is an error condition.

    if (object != NULL) {
      // But we're lucky enough to have a TypedObject pointer handy!
      // Maybe we can use it to resolve the error.  We have to drop
      // the lock while we do this, so we don't get a recursive lock.
      _lock->release();
      handle = object->force_init_type();
      _lock->acquire();

      if (handle._index == 0) {
        // Strange.
        cerr
          << "Unable to force_init_type() on unregistered TypeHandle.\n";
        return NULL;
      }

      // Now get the name for printing.  We can't use TypeHandle::
      // get_name() since that recursively calls look_up().
      ostringstream name;
      if (handle._index > 0 && handle._index < (int)_handle_registry.size()) {
        TypeRegistryNode *rnode = _handle_registry[handle._index];
        if (rnode != (TypeRegistryNode *)NULL) {
          name << rnode->_name;
          name << " (index " << handle._index << ")";
        } else {
          name << "NULL (index " << handle._index << ")";
        }
      } else {
        name << "index " << handle._index;
      }

      if (handle == object->get_type()) {
        // Problem solved!
        cerr
          << "Type " << name.str() << " was unregistered!\n";
      } else {
        // No good; it looks like the TypeHandle belongs to a class
        // that defined get_type(), but didn't define
        // force_init_type().
        cerr
          << "Attempt to reference unregistered TypeHandle.  Type is of some\n"
          << "class derived from type " << name.str() << " that doesn't define\n"
          << "a good force_init_type() method.\n";
        return NULL;
      }

    } else {
      // We don't have a TypedObject pointer, so there's nothing we
      // can do about it.
      cerr
        << "Attempt to reference unregistered TypeHandle!\n"
        << "Registered TypeHandles are:\n";
      do_write(cerr);
      return NULL;
    }
  }

  if (handle._index < 0 ||
      handle._index >= (int)_handle_registry.size()) {
    cerr
      << "Invalid TypeHandle index " << handle._index
      << "!  Is memory corrupt?\n";
    return NULL;
  }
#endif  // NDEBUG

  return _handle_registry[handle._index];
}

////////////////////////////////////////////////////////////////////
//     Function: get_best_parent_from_Set
//       Access: Private
///////////////////////////////////////////////////////////////////
extern "C" int
get_best_parent_from_Set(int id, const std::set<int> &this_set) {
  // most common case..
  if (this_set.find(id) != this_set.end()) {
    return id;
  }

  TypeHandle th = TypeRegistry::ptr()->find_type_by_id(id);
  if (th == TypeHandle::none()) {
    return -1;
  }

  return th.get_best_parent_from_Set(this_set);
}
