// Filename: typeHandle.cxx
// Created by:  drose (23Oct98)
// 
////////////////////////////////////////////////////////////////////

#include "typeHandle.h"
#include "indent.h"
#include "config_express.h"

#include <algorithm>

// In general, we use the express_cat->info() syntax in this file
// (instead of express_cat.info()), because much of this work is done at
// static init time, and we must use the arrow syntax to force
// initialization of the express_cat category.

TypeRegistry *TypeRegistry::_global_pointer = NULL;

TypeHandle TypedObject::_type_handle;

// This is initialized to zero by static initialization.
TypeHandle TypeHandle::_none;


TypeHandle long_type_handle;
TypeHandle int_type_handle;
TypeHandle short_type_handle;
TypeHandle char_type_handle;
TypeHandle bool_type_handle;
TypeHandle double_type_handle;
TypeHandle float_type_handle;

TypeHandle long_p_type_handle;
TypeHandle int_p_type_handle;
TypeHandle short_p_type_handle;
TypeHandle char_p_type_handle;
TypeHandle bool_p_type_handle;
TypeHandle double_p_type_handle;
TypeHandle float_p_type_handle;
TypeHandle void_p_type_handle;

void init_system_type_handles() {
  static bool done = false;
  if (!done) {
    done = true;
    register_type(long_type_handle, "long");
    register_type(int_type_handle, "int");
    register_type(short_type_handle, "short");
    register_type(char_type_handle, "char");
    register_type(bool_type_handle, "bool");
    register_type(double_type_handle, "double");
    register_type(float_type_handle, "float");
    
    register_type(int_p_type_handle, "int*");
    register_type(short_p_type_handle, "short*");
    register_type(char_p_type_handle, "char*");
    register_type(bool_p_type_handle, "bool*");
    register_type(double_p_type_handle, "double*");
    register_type(float_p_type_handle, "float*");
    register_type(void_p_type_handle, "void*");
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::RegistryNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TypeRegistry::RegistryNode::
RegistryNode(TypeHandle handle, const string &name, TypeHandle &ref) :
  _handle(handle), _name(name), _ref(ref) { 
}

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
    RegistryNode *rnode = look_up(type_handle, NULL);
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

    if (express_cat->is_spam()) {
      express_cat->spam() << "Registering type " << name << "\n";
    }
    
    TypeHandle new_handle;
    new_handle._index = _handle_registry.size();

    RegistryNode *rnode = new RegistryNode(new_handle, name, type_handle);
    _handle_registry.push_back(rnode);
    _name_registry[name] = rnode;

    type_handle = new_handle;
    return true;
  }
  RegistryNode *rnode = (*ri).second;
  nassertr(rnode->_name == (*ri).first, false);
  nassertr(rnode->_handle._index >= 0 && 
	   rnode->_handle._index < _handle_registry.size(), false);
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
    express_cat->error()
      << "Reregistering " << name << "\n";
    type_handle == rnode->_handle;
    return false;
  }

  if (type_handle != rnode->_handle) {
    // Hmm, we seem to have a contradictory type registration!
    express_cat->warning()
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

    if (express_cat->is_spam()) {
      express_cat->spam() << "Registering type " << name << "\n";
    }
    
    // We must dynamically allocate a new handle so the RegistryNode
    // has something unique to point to.  This doesn't really mean
    // anything, though.
    TypeHandle *new_handle = new TypeHandle;
    new_handle->_index = _handle_registry.size();

    RegistryNode *rnode = new RegistryNode(*new_handle, name, *new_handle);
    _handle_registry.push_back(rnode);
    _name_registry[name] = rnode;

    return *new_handle;
  }

  // Return the TypeHandle previously obtained.
  RegistryNode *rnode = (*ri).second;
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
  RegistryNode *cnode = look_up(child, NULL);
  nassertv(cnode != (RegistryNode *)NULL);
  RegistryNode *pnode = look_up(parent, NULL);
  nassertv(pnode != (RegistryNode *)NULL);

  // First, we'll just run through the list to make sure we hadn't
  // already made this connection.
  RegistryNode::Classes::iterator ni;
  ni = find(cnode->_parent_classes.begin(), cnode->_parent_classes.end(),
	    pnode);

  if (ni == cnode->_parent_classes.end()) {
    cnode->_parent_classes.push_back(pnode);
    pnode->_child_classes.push_back(cnode);
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
  RegistryNode *rnode = look_up(type, object);
  nassertr(rnode != (RegistryNode *)NULL, "");
  return rnode->_name;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::is_derived_from
//       Access: Public
//  Description: Returns true if the first type is derived from the
//               second type, false otherwise.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
bool TypeRegistry::
is_derived_from(TypeHandle child, TypeHandle parent,
		TypedObject *child_object) const {
  RegistryNode *rnode = look_up(child, child_object);
  nassertr(rnode != (RegistryNode *)NULL, false);
  return rnode->is_derived_from(parent);
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
  freshen_root_classes();
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
  freshen_root_classes();
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
  RegistryNode *rnode = look_up(child, child_object);
  nassertr(rnode != (RegistryNode *)NULL, 0);
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
  RegistryNode *rnode = look_up(child, (TypedObject *)NULL);
  nassertr(rnode != (RegistryNode *)NULL, TypeHandle::none());
  nassertr(index >= 0 && index < rnode->_parent_classes.size(),
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
  RegistryNode *rnode = look_up(child, child_object);
  nassertr(rnode != (RegistryNode *)NULL, 0);
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
  RegistryNode *rnode = look_up(child, (TypedObject *)NULL);
  nassertr(rnode != (RegistryNode *)NULL, TypeHandle::none());
  nassertr(index >= 0 && index < rnode->_child_classes.size(),
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
get_parent_towards(TypeHandle child, TypeHandle ancestor,
		   TypedObject *child_object) const {
  if (child_object != (TypedObject *)NULL) {
    // First, guarantee that the ancestor type is defined.
    look_up(ancestor, child_object);
  }
  RegistryNode *rnode = look_up(child, child_object);
  nassertr(rnode != (RegistryNode *)NULL, TypeHandle::none());
  return rnode->get_parent_towards(ancestor);
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
    RegistryNode *rnode = (*ri);
    if (rnode != NULL && rnode->_handle != rnode->_ref) {
      express_cat->warning()
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
    const RegistryNode *root = *hi;
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
 
  _root_classes_fresh = false;
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
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::freshen_root_classes
//       Access: Private
//  Description: Walks through the list of types registered, and adds
//               any type known to be a root (that is, that derives
//               from no other types) to the set of known root
//               classes.  This must be done from time to time because
//               we don't record this information when we initially
//               record the types.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
freshen_root_classes() {
  if (!_root_classes_fresh) {
    _root_classes.clear();

    HandleRegistry::iterator hi;
    for (hi = _handle_registry.begin();
	 hi != _handle_registry.end();
	 ++hi) {
      RegistryNode *root = *hi;
      if (root != NULL && root->_parent_classes.empty()) {
	_root_classes.push_back(root);
      }
    }
    _root_classes_fresh = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::write_node
//       Access: Private
//  Description: Writes a single RegistryNode out, along with all of
//               its descendants.
////////////////////////////////////////////////////////////////////
void TypeRegistry::
write_node(ostream &out, int indent_level, const RegistryNode *node) const {
  indent(out, indent_level) << node->_handle.get_index() << " " << node->_name;
  if (!node->_parent_classes.empty()) {
    out << " : " << node->_parent_classes[0]->_name;
    for (int pi = 1; pi < node->_parent_classes.size(); pi++) {
      out << ", " << node->_parent_classes[pi]->_name;
    }
  }
  out << "\n";
  
  for (int i = 0; i < node->_child_classes.size(); i++) {
    write_node(out, indent_level + 2, node->_child_classes[i]);
  }
}
    

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::look_up
//       Access: Private
//  Description: Returns the RegistryNode associated with the
//               indicated TypeHandle.  If there is no associated
//               RegistryNode, reports an error condition and aborts.
//
//               The associated TypedObject pointer is the pointer to
//               the object that owns the handle, if available.  It is
//               only used in an error condition, if for some reason
//               the handle was uninitialized.
////////////////////////////////////////////////////////////////////
TypeRegistry::RegistryNode *TypeRegistry::
look_up(TypeHandle handle, TypedObject *object) const {
  if (handle._index == 0) {
    // The TypeHandle is unregistered.  This is an error condition.

    if (object != NULL) {
      // But we're lucky enough to have a TypedObject pointer handy!
      // Maybe we can use it to resolve the error.
      handle = object->force_init_type();
      if (handle._index == 0) {
	// Strange.
	express_cat->error()
	  << "Unable to force_init_type() on unregistered TypeHandle.\n";
	nassertr(false, NULL);
      }
      if (handle == object->get_type()) {
	// Problem solved!
	express_cat->warning()
	  << "Type " << handle << " was unregistered!\n";
      } else {
	// No good; it looks like the TypeHandle belongs to a class
	// that defined get_type(), but didn't define
	// force_init_type().
	express_cat->error()
	  << "Attempt to reference unregistered TypeHandle.  Type is of some\n"
	  << "class derived from " << handle << " that doesn't define a good\n"
	  << "force_init_type() method.\n";
	nassertr(false, NULL);
      }

    } else {
      // We don't have a TypedObject pointer, so there's nothing we
      // can do about it.
      express_cat->error()
	<< "Attempt to reference unregistered TypeHandle!\n"
	<< "Registered TypeHandles are:\n";
      write(express_cat->error(false));
      nassertr(false, NULL);
    }
  }

  if (handle._index < 0 ||
      handle._index >= _handle_registry.size()) {
    express_cat->fatal()
      << "Invalid TypeHandle index " << handle._index 
      << "!  Is memory corrupt?\n";
    nassertr(false, NULL);
  }

  return _handle_registry[handle._index];
}


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::RegistryNode::is_derived_from
//       Access: Public
//  Description: Returns true if the current RegistryNode represents a
//               class of type TypeHandle, or any of its ancestors do.
////////////////////////////////////////////////////////////////////
bool TypeRegistry::RegistryNode::
is_derived_from(TypeHandle type) const {
  if (_handle == type) {
    return true;
  }

  Classes::const_iterator ni;
  for (ni = _parent_classes.begin(); ni != _parent_classes.end(); ++ni) {
    if ((*ni)->is_derived_from(type)) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::RegistryNode::get_parent_towards
//       Access: Public
//  Description: Returns the first derived class that is an descendant
//               of the indicated ancestor class.
////////////////////////////////////////////////////////////////////
TypeHandle TypeRegistry::RegistryNode::
get_parent_towards(TypeHandle type) const {
  if (_handle == type) {
    return type;
  }

  Classes::const_iterator ni;
  for (ni = _parent_classes.begin(); ni != _parent_classes.end(); ++ni) {
    if ((*ni)->is_derived_from(type)) {
      return (*ni)->_handle;
    }
  }

  return TypeHandle::none();
}

 
////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TypedObject::
~TypedObject() { 
}
