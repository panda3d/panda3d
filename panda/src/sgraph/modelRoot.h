// Filename: modelRoot.h
// Created by:  drose (09Nov00)
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

#ifndef MODELROOT_H
#define MODELROOT_H

#include <pandabase.h>

#include "modelNode.h"

////////////////////////////////////////////////////////////////////
//       Class : ModelRoot
// Description : A node of this type is created automatically at the
//               root of each model file that is loaded.  It may
//               eventually contain some information about the
//               contents of the model; at the moment, it contains no
//               special information, but can be used as a flag to
//               indicate the presence of a loaded model file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ModelRoot : public ModelNode {
PUBLISHED:
  INLINE ModelRoot(const string &name = "");

public:
  INLINE ModelRoot(const ModelRoot &copy);
  INLINE void operator = (const ModelRoot &copy);

  virtual Node *make_copy() const;

public:
  static void register_with_read_factory(void);

protected:
  static TypedWritable *make_ModelRoot(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ModelNode::init_type();
    register_type(_type_handle, "ModelRoot",
                  ModelNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "modelRoot.I"

#endif


