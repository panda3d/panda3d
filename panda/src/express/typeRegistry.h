// Filename: typeRegistry.h
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

#ifndef TYPEREGISTRY_H
#define TYPEREGISTRY_H

#include "pandabase.h"

#include "typeHandle.h"
#include "typeRegistryNode.h"

#include "notify.h"
#include "pvector.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : TypeRegistry
// Description : The TypeRegistry class maintains all the assigned
//               TypeHandles in a given system.  There should be only
//               one TypeRegistry class during the lifetime of the
//               application.  It will be created on the local heap
//               initially, and it should be migrated to shared memory
//               as soon as shared memory becomes available.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TypeRegistry {
public:
  // User code shouldn't generally need to call
  // TypeRegistry::register_type() or record_derivation() directly;
  // instead, use the register_type convenience function, defined
  // below.
  bool register_type(TypeHandle &type_handle, const string &name);
  TypeHandle register_dynamic_type(const string &name);

  void record_derivation(TypeHandle child, TypeHandle parent);
  void record_alternate_name(TypeHandle type, const string &name);

PUBLISHED:
  TypeHandle find_type(const string &name) const;

  string get_name(TypeHandle type, TypedObject *object) const;
  INLINE bool is_derived_from(TypeHandle child, TypeHandle base,
                              TypedObject *child_object);

  int get_num_root_classes();
  TypeHandle get_root_class(int n);

  int get_num_parent_classes(TypeHandle child,
                             TypedObject *child_object) const;
  TypeHandle get_parent_class(TypeHandle child, int index) const;

  int get_num_child_classes(TypeHandle child,
                            TypedObject *child_object) const;
  TypeHandle get_child_class(TypeHandle child, int index) const;

  TypeHandle get_parent_towards(TypeHandle child, TypeHandle base,
                                TypedObject *child_object);

  static void reregister_types();

  void write(ostream &out) const;

  // ptr() returns the pointer to the global TypeRegistry object.
  static TypeRegistry *ptr();

private:
  // The TypeRegistry class should never be constructed by user code.
  // There is only one in the universe, and it constructs itself!
  TypeRegistry();

  static void init_global_pointer();

  // In NDEBUG mode, this is an inline function; otherwise, it's
  // out-of-line.
#ifdef NDEBUG
  INLINE TypeRegistryNode *look_up(TypeHandle type, TypedObject *object) const;
#else
  TypeRegistryNode *look_up(TypeHandle type, TypedObject *object) const;
#endif

  INLINE void freshen_derivations();
  void rebuild_derivations();
  void write_node(ostream &out, int indent_level,
                  const TypeRegistryNode *node) const;

  typedef pvector<TypeRegistryNode *> HandleRegistry;
  HandleRegistry _handle_registry;

  typedef phash_map<string, TypeRegistryNode *, string_hash> NameRegistry;
  NameRegistry _name_registry;

  typedef pvector<TypeRegistryNode *> RootClasses;
  RootClasses _root_classes;

  bool _derivations_fresh;

  static TypeRegistry *_global_pointer;
};

#include "typeRegistry.I"

#endif
