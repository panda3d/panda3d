// Filename: planeNode.h
// Created by:  mike (09Jan97)
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
#ifndef PLANENODE_H
#define PLANENODE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <namedNode.h>
#include <plane.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : PlaneNode
// Description : A node that has a plane
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PlaneNode : public NamedNode
{
PUBLISHED:
  INLINE PlaneNode(const string& name = "");

public:
  INLINE PlaneNode(const PlaneNode &copy);
  INLINE void operator = (const PlaneNode &copy);

  virtual Node *make_copy() const;

PUBLISHED:
  void set_plane(const Planef& plane);
  const Planef& get_plane(void) const;

protected:

  Planef                _plane;

public:

  static TypeHandle get_class_type( void ) {
    return _type_handle;
  }
  static void init_type(void) {
    NamedNode::init_type();
    register_type(_type_handle, "PlaneNode",
                  NamedNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle                       _type_handle;
};

#include "planeNode.I"

#endif
