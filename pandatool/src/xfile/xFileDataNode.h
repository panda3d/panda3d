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
#include "xFileTemplate.h"
#include "pointerTo.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileDataNode
// Description : This is an abstract base class for an XFileNode which
//               is also an XFileDataObject.  That is to say, objects
//               that inherit from this class may be added to the
//               toplevel X file graph as nodes, and they also may be
//               containers for data elements.
//
//               Specifically, this is the base class of both
//               XFileDataNodeTemplate and XFileDataNodeReference.
////////////////////////////////////////////////////////////////////
class XFileDataNode : public XFileNode, public XFileDataObject {
public:
  XFileDataNode(XFile *x_file, const string &name,
                XFileTemplate *xtemplate);

  virtual bool is_object() const;
  virtual bool is_standard_object(const string &template_name) const;
  virtual string get_type_name() const;

  INLINE const XFileDataNode &get_data_child(int n) const;

  INLINE XFileTemplate *get_template() const;
  INLINE const string &get_template_name() const;

protected:
  PT(XFileTemplate) _template;

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
  



