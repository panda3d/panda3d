// Filename: modelNode.h
// Created by:  drose (16Mar02)
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

#ifndef MODELNODE_H
#define MODELNODE_H

#include "pandabase.h"

#include "pandaNode.h"

////////////////////////////////////////////////////////////////////
//       Class : ModelNode
// Description : This node is placed at key points within the scene
//               graph to indicate the roots of "models": subtrees
//               that are conceptually to be treated as a single unit,
//               like a car or a room, for instance.  It doesn't
//               affect rendering or any other operations; it's
//               primarily useful as a high-level model indication.
//
//               ModelNodes are created in response to a <Model> { 1 }
//               flag within an egg file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ModelNode : public PandaNode {
PUBLISHED:
  INLINE ModelNode(const string &name);

protected:
  INLINE ModelNode(const ModelNode &copy);

public:
  virtual PandaNode *make_copy() const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool preserve_name() const;

PUBLISHED:
  enum PreserveTransform {
    PT_none,
    PT_local,
    PT_net
  };

  INLINE void set_preserve_transform(PreserveTransform preserve_transform);
  INLINE PreserveTransform get_preserve_transform() const;

private:
  PreserveTransform _preserve_transform;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "ModelNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "modelNode.I"

#endif


