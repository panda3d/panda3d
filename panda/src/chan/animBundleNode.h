// Filename: animBundleNode.h
// Created by:  drose (22Feb99)
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

#ifndef ANIMBUNDLENODE_H
#define ANIMBUNDLENODE_H

#include <pandabase.h>

#include "animBundle.h"

#include <namedNode.h>

////////////////////////////////////////////////////////////////////
//       Class : AnimBundleNode
// Description : This is a node that contains a pointer to an
//               AnimBundle.  Its primary raison d'être is to allow
//               the AnimBundles to be stored in the scene graph, so
//               they may conveniently be loaded as egg files, for
//               instance.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimBundleNode : public NamedNode {
PUBLISHED:
  INLINE AnimBundleNode(AnimBundle *bundle);
  INLINE AnimBundleNode(const AnimBundleNode &copy);
  INLINE void operator = (const AnimBundleNode &copy);

  virtual Node *make_copy() const;
  INLINE AnimBundle *get_bundle() const;

private:
  PT(AnimBundle) _bundle;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

  static TypedWritable *make_AnimBundleNode(const FactoryParams &params);

protected:
  INLINE AnimBundleNode(void);
  void fillin(DatagramIterator& scan, BamReader* manager);

public:

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NamedNode::init_type();
    register_type(_type_handle, "AnimBundleNode",
                  NamedNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animBundleNode.I"

#endif
