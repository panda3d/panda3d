// Filename: partBundleNode.h
// Created by:  drose (06Mar02)
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

#ifndef PARTBUNDLENODE_H
#define PARTBUNDLENODE_H

#include "pandabase.h"

#include "partBundle.h"

#include "pandaNode.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//       Class : PartBundleNode
// Description : This is a node that contains a pointer to an
//               PartBundle.  Like AnimBundleNode, it exists solely to
//               make it easy to store PartBundles in the scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PartBundleNode : public PandaNode {
public:
  INLINE PartBundleNode(const string &name, PartBundle *bundle);

protected:
  INLINE PartBundleNode();
  INLINE PartBundleNode(const PartBundleNode &copy, PartBundle *bundle = NULL);

public:
  virtual bool safe_to_flatten() const;

PUBLISHED:
  INLINE PartBundle *get_bundle() const;

private:
  PT(PartBundle) _bundle;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
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
    PandaNode::init_type();
    register_type(_type_handle, "PartBundleNode",
                  PandaNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "partBundleNode.I"

#endif
