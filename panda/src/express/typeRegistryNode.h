// Filename: typeRegistryNode.h
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

#ifndef TYPEREGISTRYNODE_H
#define TYPEREGISTRYNODE_H

#include "pandabase.h"

#include "typeHandle.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : TypeRegistryNode
// Description : This is a single entry in the TypeRegistry.
//               Normally, user code will never directly access this
//               class; this class is hidden within the TypeRegistry
//               accessors.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TypeRegistryNode {
public:
  TypeRegistryNode(TypeHandle handle, const string &name, TypeHandle &ref);

  static bool is_derived_from(const TypeRegistryNode *child,
                              const TypeRegistryNode *base);

  static TypeHandle get_parent_towards(const TypeRegistryNode *child,
                                       const TypeRegistryNode *base);

  void clear_subtree();
  void define_subtree();

  TypeHandle _handle;
  string _name;
  TypeHandle &_ref;
  typedef pvector<TypeRegistryNode *> Classes;
  Classes _parent_classes;
  Classes _child_classes;

  static bool _paranoid_inheritance;

private:
  typedef int SubtreeMaskType;

  // This class defines the inheritance relationship of this node from
  // some ancestor denoted as a "subtree top" node.  This is usually
  // the nearest ancestor that has multiple inheritance.
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
  typedef pvector<Inherit> TopInheritance;

  void r_build_subtrees(TypeRegistryNode *top, 
                        int bit_count, SubtreeMaskType bits);

  static bool check_derived_from(const TypeRegistryNode *child,
                                 const TypeRegistryNode *base);

  Inherit _inherit;

  // The _top_inheritance member is only filled for nodes that are
  // denoted as "subtree top" nodes.  It represents the complete set
  // of subtree_top nodes that this node inherits from, directly or
  // indirectly.
  TopInheritance _top_inheritance;

  // _visit_count is only used during r_build_subtree().
  int _visit_count;
};

#include "typeRegistryNode.I"

#endif
