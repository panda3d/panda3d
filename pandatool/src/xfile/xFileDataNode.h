// Filename: xFileDataNode.h
// Created by:  drose (08Oct04)
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

#ifndef XFILEDATANODE_H
#define XFILEDATANODE_H

#include "pandatoolbase.h"
#include "xFileNode.h"
#include "xFileDataObject.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileDataNode
// Description : This is an XFileNode which is also an
//               XFileDataObject.  Specifically, this is the base
//               class of XFileDataNodeTemplate and
//               XFileDataNodeReference, both of which have a similar
//               interface--they can both appear as the child of a
//               XFileNode, and they both contain additional nodes and
//               data objects.
////////////////////////////////////////////////////////////////////
class XFileDataNode : public XFileNode, public XFileDataObject {
public:
  XFileDataNode(XFile *x_file, const string &name);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileNode::init_type();
    XFileDataObject::init_type();
    register_type(_type_handle, "XFileDataNode",
                  XFileNode::get_class_type(),
                  XFileDataObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataNode.I"

#endif
  



