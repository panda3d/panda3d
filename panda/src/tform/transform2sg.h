// Filename: transform2sg.h
// Created by:  drose (27Jan99)
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

#ifndef TRANSFORM2SG_H
#define TRANSFORM2SG_H

#include "pandabase.h"

#include "dataNode.h"
#include "pandaNode.h"

class NodeRelation;

////////////////////////////////////////////////////////////////////
//       Class : Transform2SG
// Description : input: Transform (matrix)
//
//               output: none, but applies the matrix as the transform
//               transition for a given arc of the scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Transform2SG : public DataNode {
PUBLISHED:
  Transform2SG(const string &name = "");

  void set_arc(NodeRelation *arc);
  NodeRelation *get_arc() const;

  void set_node(PandaNode *node);
  PandaNode *get_node() const;

private:
  NodeRelation *_arc;
  PandaNode *_node;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:

  virtual void
  transmit_data(AllTransitionsWrapper &data);

  // inputs
  static TypeHandle _transform_type;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#endif

