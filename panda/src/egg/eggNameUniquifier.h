// Filename: eggNameUniquifier.h
// Created by:  drose (09Nov00)
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

#ifndef EGGNAMEUNIQUIFIER_H
#define EGGNAMEUNIQUIFIER_H

////////////////////////////////////////////////////////////////////
//
// EggNameUniquifier
//
// This is a utility class for renaming nodes in an egg hierarchy so
// that no two nodes share the same name.  It's useful, for instance,
// as a preprocess before translating the egg hierarchy to a scene
// graph format that doesn't tolerate two identically-named nodes;
// it's also particularly useful for guaranteeing that VertexPools and
// Textures do not have conflicting names.
//
// This is actually an abstract class; in order to use it, you must
// derive your own class and redefine some key functions (but see
// EggPoolUniquifier and EggGroupUniquifier).
//
// You must define at least the following function:
//
// virtual string get_category(EggNode *node);
//
//    This function defines the particular category that the
//    particular node should be grouped into.  All nodes that share
//    the same category name will be considered in the same name pool
//    and may not have the same name; two nodes that have different
//    categories will be allowed to keep the same name.
//
//    If the category is the empty string, the node will not be
//    considered for uniquification.
//
//
// You may also define the following function:
//
// virtual string filter_name(EggNode *node);
//
//    This returns the name of the node, or at least the name it ought
//    to be.  This provides a hook for, for instance, filtering out
//    invalid characters before the node name is uniquified.
//
//
// virtual string generate_name(EggNode *node,
//                              const string &category, int index);
//
//    This returns a new name for the given node, once a node has been
//    identified as having the same name as another node.  It may use
//    any algorithm you please to generate a new name, using any
//    combination of the node's original name, the category (as
//    returned by get_category()), and/or the supplied unique index
//    number.
//
//    If this function returns a name that happens to collide with
//    some other already-existing node, it will simply be called again
//    (with a new index number) until it finally returns a unique
//    name.
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "eggObject.h"

#include "pmap.h"

class EggNode;

////////////////////////////////////////////////////////////////////
//       Class : EggNameUniquifier
// Description : This is a handy class for guaranteeing unique node
//               names in an egg hierarchy.  It is an abstract class;
//               to use it you must subclass off of it.  See the
//               comment above.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggNameUniquifier : public EggObject {
PUBLISHED:
  EggNameUniquifier();
  ~EggNameUniquifier();

  void clear();

  void uniquify(EggNode *node);

  EggNode *get_node(const string &category, const string &name) const;
  bool has_name(const string &category, const string &name) const;
  bool add_name(const string &category, const string &name,
                EggNode *node = NULL);

  virtual string get_category(EggNode *node)=0;
  virtual string filter_name(EggNode *node);
  virtual string generate_name(EggNode *node,
                               const string &category, int index);

private:
  typedef pmap<string, EggNode *> UsedNames;
  typedef pmap<string, UsedNames> Categories;

  Categories _categories;
  int _index;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggObject::init_type();
    register_type(_type_handle, "EggNameUniquifier",
                  EggObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#endif


