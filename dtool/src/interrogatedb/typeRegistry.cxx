// Filename: typeRegistry.cxx
// Created by:  drose (06Aug01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "typeRegistry.h"
#include "typeRegistryNode.h"
#include "typeHandle.h"
#include "typedObject.h"
#include "indent.h"
#include "config_interrogatedb.h"
#include "configVariableBool.h"

#include <algorithm>

// In general, we use the interrogatedb_cat->info() syntax in this file
// (instead of interrogatedb_cat.info()), because much of this work is done at
// static init time, and we must use the arrow syntax to force
// initialization of the interrogatedb_cat category.

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
  if (type_handle != TypeHandle::none()) {
    // Here's a type that was already registered.  Just make sure
    // everything's still kosher.
    TypeRegistryNode *rnode = look_up(type_handle, NULL);
    if (&type_handle == &rnode->_ref) {
      // No problem.
      nassertr(rnode->_name == name, false);
      return false;
    }
  }

  NameRegistry::iterator ri;
  ri = _name_registry.find(name);

  if (ri == _name_registry.end()) {
    // The name was not already used; this is the first time this
    // class has been defined.

#ifdef NOTIFY_DEBUG
    // This code runs at static init time, so cannot use the
    // interrogatedb_cat.is_spam() syntax.
    if (interrogatedb_cat->is_spam()) {
      interrogatedb_cat->spam() << "Registering type " << name << "\n";
    }
#endif

    TypeHandle new_handle;
    new_handle._index = _handle_registry.size();

    TypeRegistryNode *rnode = new TypeRegistryNode(new_handle, name, type_handle);
    _handle_registry.push_back(rnode);
    _name_registry[name] = rnode;
    _derivations_fresh = false;

    type_handle = new_handle;
    return true;
  }
  TypeRegistryNode *rnode = (*ri).second;
  nassertr(rnode->_name == (*ri).first, false);
  nassertr(rnode->_handle._index >= 0 &&
           rnode->_handle._index < (int)_handle_registry.size(), false);
  nassertr(_handle_registry[rnode->_handle._index] == rnode, false);
  nassertr(rnode->_handle._index != 0, false);

  // The name was previously used; make sure the type_handle matches.
  if (&type_handle == &rnode->_ref) {
    // Ok, this was just a repeated attempt to register the same type.

    if (type_handle == rnode->_handle) {
      // No problem.
      return false;
    }
    // But wait--the type_handle has changed!  We kept a reference to
    // the static _type_handle member in the class that was passed in
    // at the first call to register_type(), and we got the same
    // reference passed in this time, but now it's different!  Bad
    // juju.
    interrogatedb_cat->error()
      << "Reregistering " << name << "\n";
    type_handle == rnode->_handle;
    return false;
  }

  if (type_handle != rnode->_handle) {
    // Hmm, we seem to have a contradictory type registration!
    interrogatedb_cat->warning()
      << "Attempt to register type " << name << " more than once!\n";

    // This is invalid, but we'll allow it anyway.  It seems to happen
    // for some reason under GNU libc5 that we occasionally end up
    // with two legitimate copies of the same class object in
    // memory--each with its own static _type_handle member.

    type_handle = rnode->_handle;
  }
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
  NameRegistry::iterator ri;
  ri = _name_registry.find(name);

  if (ri == _name_registry.end()) {
    // The name was not already used; this is the first time this
    // class has been defined.

#ifdef NOTIFY_DEBUG
    // This code runs at static init time, so cannot use the
    // interrogatedb_cat.is_spam() syntax.
    if (interrogatedb_cat->is_spam()) {
      interrogatedb_cat->spam() << "Registering type " << name << "\n";
    }
#endif

    // We must dynamically allocate a new handle so the TypeRegistryNode
    // has something unique to point to.  This doesn't really mean
    // anything, though.
    TypeHandle *new_handle = new TypeHandle;
    new_handle->_index = _handle_registry.size();

    TypeRegistryNode *rnode = new TypeRegistryNode(*new_handle, name, *new_handle);
    _handle_registry.push_back(rnode);
    _name_registry[name] = rnode;
    _derivations_fresh = false;

    return *new_handle;
  }

  // Return the TypeHandle previously obtained.
  TypeRegistryNode *rnode = (*ri).second;
  return rnode->_handle;
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
  TypeRegistryNode *cnode = look_up(child, NULL);
  nassertv(cnode != (TypeRegistryNode *)NULL);
  TypeRegistryNode *pnode = look_up(parent, NULL);
  nassertv(pnode != (TypeRegistryNode *)NULL);

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
  TypeRegistryNode *rnode = look_up(type, (TypedObject *)NULL);
  if (rnode != (TypeRegistryNode *)NULL) {
    NameRegistry::iterator ri =
      _name_registry.insert(NameRegistry::value_type(name, rnode)).first;
    if ((*ri).second != rnode) {
      interrogatedb_cat.warning()
        << "Name " << name << " already assigned to TypeHandle "
        << rnode->_name << "; cannot reassign to " << type << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::find_type
//       Access: Public
//  Description: Looks for a previously-registered type of the given
//               name.  Returns its TypeHandle if it exists, or
//               TypeHandle::none() if there is no such type.
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
find_type(const string &name) const {
  NameRegistry::const_iterator ri;
  ri = _name_registry.find(name);
  if (ri == _name_registry.end()) {
    return TypeHandle::none();
  } else {
    return (*ri).second->_handle;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_name
//       Access: Public
//  Description: Returns the name of the indicated type.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
string TypeRegistry::
get_name(TypeHandle type, TypedObject *object) const {
  TypeRegistryNode *rnode = look_up(type, object);
  nassertr(rnode != (TypeRegistryNode *)NULL, "");
  return rnode->_name;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::is_derived_from
//       Access: Public
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
  const TypeRegistryNode *child_node = look_up(child, child_object);
  const TypeRegistryNode *base_node = look_up(base, (TypedObject *)NULL);
  nassertr(child_node != (TypeRegistryNode *)NULL &&
           base_node != (TypeRegistryNode *)NULL, false);
  freshen_derivations();
  return TypeRegistryNode::is_derived_from(child_node, base_node);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_num_root_classes
//       Access: Public
//  Description: Returns the number of root classes--that is, classes
//               that do not inherit from any other classes--known in
//               the system.
////////////////////////////////////////////////////////////////////
int TypeRegistry::
get_num_root_classes() {
  freshen_derivations();
  return _root_classes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_root_class
//       Access: Public
//  Description: Returns the nth root class in the system.  See
//               get_num_root_classes().
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
get_root_class(int n) {
  freshen_derivations();
  nassertr(n >= 0 && n < get_num_root_classes(), TypeHandle::none());
  return _root_classes[n]->_handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_num_parent_classes
//       Access: Public
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
  TypeRegistryNode *rnode = look_up(child, child_object);
  nassertr(rnode != (TypeRegistryNode *)NULL, 0);
  return rnode->_parent_classes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_parent_class
//       Access: Public
//  Description: Returns the nth parent class of this type.  The index
//               should be in the range 0 <= index <
//               get_num_parent_classes().
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
get_parent_class(TypeHandle child, int index) const {
  TypeRegistryNode *rnode = look_up(child, (TypedObject *)NULL);
  nassertr(rnode != (TypeRegistryNode *)NULL, TypeHandle::none());
  nassertr(index >= 0 && index < (int)rnode->_parent_classes.size(),
           TypeHandle::none());
  return rnode->_parent_classes[index]->_handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_num_child_classes
//       Access: Public
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
  TypeRegistryNode *rnode = look_up(child, child_object);
  nassertr(rnode != (TypeRegistryNode *)NULL, 0);
  return rnode->_child_classes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_child_class
//       Access: Public
//  Description: Returns the nth child class of this type.  The index
//               should be in the range 0 <= index <
//               get_num_child_classes().
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::
get_child_class(TypeHandle child, int index) const {
  TypeRegistryNode *rnode = look_up(child, (TypedObject *)NULL);
  nassertr(rnode != (TypeRegistryNode *)NULL, TypeHandle::none());
  nassertr(index >= 0 && index < (int)rnode->_child_classes.size(),
           TypeHandle::none());
  return rnode->_child_classes[index]->_handle;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::get_parent_towards
//       Access: Public
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
  const TypeRegistryNode *child_node = look_up(child, child_object);
  const TypeRegistryNode *base_node = look_up(base, NULL);
  nassertr(child_node != (TypeRegistryNode *)NULL && 
           base_node != (TypeRegistryNode *)NULL, TypeHandle::none());
  freshen_derivations();
  return TypeRegistryNode::get_parent_towards(child_node, base_node);
}


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::reregister_types
//       Access: Public, Static
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
  HandleRegistry::iterator ri;
  TypeRegistry *reg = ptr();
  for (ri = reg->_handle_registry.begin();
       ri != reg->_handle_registry.end();
       ++ri) {
    TypeRegistryNode *rnode = (*ri);
    if (rnode != NULL && rnode->_handle != rnode->_ref) {
      interrogatedb_cat->warning()
        << "Reregistering " << rnode->_name << "\n";
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::write
//       Access: Public
//  Description: Makes an attempt to format the entire TypeRegistry in
//               a nice way that shows the derivation tree as
//               intelligently as possible.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
write(ostream &out) const {
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
//     Function: TypeRegistry::ptr
//       Access: Public, Static
//  Description: Returns the pointer to the global TypeRegistry
//               object.
////////////////////////////////////////////////////////////////////
TypeRegistry *TypeRegistry::
ptr() {
  if (_global_pointer == NULL) {
#ifdef NOTIFY_DEBUG
    if (interrogatedb_cat->is_spam()) {
      interrogatedb_cat->spam()
        << "Creating global TypeRegistry\n";
    }
#endif
    init_global_pointer();
  }
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
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::init_global_pointer
//       Access: Private, Static
//  Description: Constructs the TypeRegistry object for the first
//               time.  It is initially created on the local heap,
//               then as soon as shared memory becomes available, it
//               should be moved into shared memory.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
init_global_pointer() {
  _global_pointer = new TypeRegistry;

  // Now that we've created the TypeRegistry, we can assign this
  // Config variable.

  ConfigVariableBool paranoid_inheritance
    ("paranoid-inheritance", true,
     PRC_DESC("Set this to true to double-check the test for inheritance of "
              "TypeHandles, e.g. via is_of_type().  This has no effect if NDEBUG "
              "is defined."));
  TypeRegistryNode::_paranoid_inheritance = paranoid_inheritance;
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
#ifdef NOTIFY_DEBUG
  interrogatedb_cat->debug()
    << "Rebuilding derivation tree.\n";
#endif

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
//     Function: TypeRegistry::write_node
//       Access: Private
//  Description: Writes a single TypeRegistryNode out, along with all of
//               its descendants.
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

#ifndef NDEBUG
// This function is only non-inline if NDEBUG is not defined.
// Otherwise, it is inline and its definition appears in
// typeRegistry.I.

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::look_up
//       Access: Private
//  Description: Returns the TypeRegistryNode associated with the
//               indicated TypeHandle.  If there is no associated
//               TypeRegistryNode, reports an error condition and
//               returns NULL.
//
//               The associated TypedObject pointer is the pointer to
//               the object that owns the handle, if available.  It is
//               only used in an error condition, if for some reason
//               the handle was uninitialized.
////////////////////////////////////////////////////////////////////
TypeRegistryNode *TypeRegistry::
look_up(TypeHandle handle, TypedObject *object) const {
  if (handle._index == 0) {
    // The TypeHandle is unregistered.  This is an error condition.

    if (object != NULL) {
      // But we're lucky enough to have a TypedObject pointer handy!
      // Maybe we can use it to resolve the error.
      handle = object->force_init_type();
      if (handle._index == 0) {
        // Strange.
        interrogatedb_cat->error()
          << "Unable to force_init_type() on unregistered TypeHandle.\n";
        nassertr(false, NULL);
      }
      if (handle == object->get_type()) {
        // Problem solved!
        interrogatedb_cat->warning()
          << "Type " << handle << " was unregistered!\n";
      } else {
        // No good; it looks like the TypeHandle belongs to a class
        // that defined get_type(), but didn't define
        // force_init_type().
        interrogatedb_cat->error()
          << "Attempt to reference unregistered TypeHandle.  Type is of some\n"
          << "class derived from " << handle << " that doesn't define a good\n"
          << "force_init_type() method.\n";
        nassertr(false, NULL);
      }

    } else {
      // We don't have a TypedObject pointer, so there's nothing we
      // can do about it.
      interrogatedb_cat->error()
        << "Attempt to reference unregistered TypeHandle!\n"
        << "Registered TypeHandles are:\n";
      write(interrogatedb_cat->error(false));
      nassertr(false, NULL);
    }
  }

  if (handle._index < 0 ||
      handle._index >= (int)_handle_registry.size()) {
    interrogatedb_cat->fatal()
      << "Invalid TypeHandle index " << handle._index
      << "!  Is memory corrupt?\n";
    nassertr(false, NULL);
  }

  return _handle_registry[handle._index];
}

////////////////////////////////////////////////////////////////////
//     Function: find_type_by_id
//       Access: Private
///////////////////////////////////////////////////////////////////
TypeHandle  TypeRegistry::find_type_by_id(int id) const
{
  if (id < 0 ||id >= (int)_handle_registry.size()) 
  {
    interrogatedb_cat->fatal()
      << "Invalid TypeHandle index " << id
      << "!  Is memory corrupt?\n";
    //nassertr(false, NULL);
    return TypeHandle::none();
  }

    return _handle_registry[id]->_handle;
};

////////////////////////////////////////////////////////////////////
//     Function: get_best_parent_from_Set
//       Access: Private
///////////////////////////////////////////////////////////////////
extern "C" int get_best_parent_from_Set(int id, const std::set<int> &set)
{
    // most common case..
    if(set.find(id) != set.end())
        return id;

    TypeHandle th = TypeRegistry::ptr()->find_type_by_id(id);
    if(th == TypeHandle::none())
        return -1;

    return th.get_best_parent_from_Set(set);
}


#endif  // NDEBUG

