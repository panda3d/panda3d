/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeRegistryNode.h
 * @author drose
 * @date 2001-08-06
 */

#ifndef TYPEREGISTRYNODE_H
#define TYPEREGISTRYNODE_H

#include "dtoolbase.h"

#include "typeHandle.h"
#include "numeric_types.h"

#include <assert.h>
#include <vector>

/**
 * This is a single entry in the TypeRegistry.  Normally, user code will never
 * directly access this class; this class is hidden within the TypeRegistry
 * accessors.
 */
class EXPCL_DTOOL_DTOOLBASE TypeRegistryNode {
public:
  TypeRegistryNode(TypeHandle handle, const std::string &name, TypeHandle &ref);

  static bool is_derived_from(const TypeRegistryNode *child,
                              const TypeRegistryNode *base);

  static TypeHandle get_parent_towards(const TypeRegistryNode *child,
                                       const TypeRegistryNode *base);

  INLINE PyObject *get_python_type() const;

  void clear_subtree();
  void define_subtree();

  TypeHandle _handle;
  std::string _name;
  TypeHandle &_ref;
  typedef std::vector<TypeRegistryNode *> Classes;
  Classes _parent_classes;
  Classes _child_classes;
  PyObject *_python_type = nullptr;

  AtomicAdjust::Integer _memory_usage[TypeHandle::MC_limit];

  static bool _paranoid_inheritance;

private:
  typedef int SubtreeMaskType;

  // This class defines the inheritance relationship of this node from some
  // ancestor denoted as a "subtree top" node.  This is usually the nearest
  // ancestor that has multiple inheritance.
  class Inherit {
  public:
    INLINE Inherit();
    INLINE Inherit(TypeRegistryNode *top, int bit_count,
                   SubtreeMaskType bits);
    INLINE Inherit(const Inherit &copy);
    INLINE void operator = (const Inherit &copy);

    INLINE bool operator < (const Inherit &other) const;
    INLINE static bool is_derived_from(const Inherit &child, const Inherit &base);

    TypeRegistryNode *_top;
    SubtreeMaskType _mask;
    SubtreeMaskType _bits;
  };
  typedef std::vector<Inherit> TopInheritance;

  void r_build_subtrees(TypeRegistryNode *top,
                        int bit_count, SubtreeMaskType bits);

  PyObject *r_get_python_type() const;

  static bool check_derived_from(const TypeRegistryNode *child,
                                 const TypeRegistryNode *base);

  Inherit _inherit;

  // The _top_inheritance member is only filled for nodes that are denoted as
  // "subtree top" nodes.  It represents the complete set of subtree_top nodes
  // that this node inherits from, directly or indirectly.
  TopInheritance _top_inheritance;

  // _visit_count is only used during r_build_subtree().
  int _visit_count;
};

#include "typeRegistryNode.I"

#endif
