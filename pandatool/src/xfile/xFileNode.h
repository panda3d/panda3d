// Filename: xFileNode.h
// Created by:  drose (03Oct04)
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

#ifndef XFILENODE_H
#define XFILENODE_H

#include "pandatoolbase.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "namable.h"
#include "notify.h"
#include "pvector.h"
#include "pmap.h"

class XFile;
class WindowsGuid;

////////////////////////////////////////////////////////////////////
//       Class : XFileNode
// Description : A single node of an X file.  This may be either a
//               template or a data node.
////////////////////////////////////////////////////////////////////
class XFileNode : public TypedReferenceCount, public Namable {
public:
  XFileNode(XFile *x_file, const string &name);
  virtual ~XFileNode();

  INLINE int get_num_children() const;
  INLINE XFileNode *get_child(int n) const;
  XFileNode *find_child(const string &name) const;
  XFileNode *find_descendent(const string &name) const;

  virtual bool has_guid() const;
  virtual const WindowsGuid &get_guid() const;

  virtual void add_child(XFileNode *node);
  virtual void clear();

  virtual void write_text(ostream &out, int indent_level) const;

protected:
  XFile *_x_file;
  
  typedef pvector< PT(XFileNode) > Children;
  Children _children;

  typedef pmap<string, XFileNode *> ChildrenByName;
  ChildrenByName _children_by_name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "XFileNode",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileNode.I"

#endif
  


