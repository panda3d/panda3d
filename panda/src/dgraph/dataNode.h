// Filename: dataNode.h
// Created by:  drose (25Jan99)
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

#ifndef DATANODE_H
#define DATANODE_H

////////////////////////////////////////////////////////////////////
//
// The Data Graph.
//
// The data graph is intended to hook up devices and their inputs
// and/or outputs in a clean interface.  It uses the same graph
// relationship that is used to construct the scene graph, with the
// same nodes and arcs.
//
// In a data graph, each node may potentially produce and/or consume
// data, and the arcs transmit data downward, from the root of the
// graph to its leaves.  Thus, an input device such as a mouse might
// be added to the graph near the root, and a tformer-style object
// that interprets the mouse data as a trackball motion and outputs a
// matrix might be the immediate child of the mouse, followed by an
// object that accepts a matrix and sets it on some particular arc in
// the scene graph.
//
// In a normal scene graph, the arcs transmit state, and each node
// inherits a collection of NodeTransition values that are defined by
// the total set of arcs above it.  In a data graph, the arcs transmit
// data instead of state, and each piece of data is stored in a
// NodeTransition value.  Thus, each data node still inherits a
// collection of NodeTransition values, but now those values contain
// data instead of state information.  In addition, a data node may
// retransmit a different set of NodeTransition values further down the
// chain.  This is implemented via the transmit_data() function, below.
//
// Each data node may define its own set of input values and output
// values, each with its own unique transition type.  This could be
// done by subclassing a different kind of NodeTransition for each
// different input or output value, but this quickly gets unwieldy.
// Instead, you may find it convenient to use the function
// register_data_transition(), below, which creates a new TypeHandle
// associated with some existing NodeTransition type, based on the
// unique name you give it.  This allows producers and consumers to
// match their corresponding data values up by TypeHandle number (and
// hence by name); this matching happens more or less transparently by
// the graph traversal logic.
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include <namedNode.h>

class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
//       Class : DataNode
// Description : The main node of the data graph.  This could
//               represent a device that produces data, or an object
//               that operates on data received from a device.
//
//               The main difference between this and a normal node is
//               the transmit_data() function.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DataNode : public NamedNode {
public:
  DataNode(const string &name = "");

  virtual void
  transmit_data(AllTransitionsWrapper &data)=0;

  virtual void
  transmit_data_per_child(AllTransitionsWrapper &data, int child_index);

PUBLISHED:
  void set_spam_mode(bool flag);
  bool get_spam_mode() const;


private:
  bool _spam_mode;


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
    register_type(_type_handle, "DataNode",
                  NamedNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};


////////////////////////////////////////////////////////////////////
//     Function: register_data_transition
//  Description: Defines a new data transition of the indicated name
//               and inheriting from the indicated transition type.
//               This will represent a channel of data, for instance a
//               3-component position from a tracker.
//
//               All data transitions that have a common name and base
//               class will share the same TypeHandle.  This makes it
//               possible to unify data producers and consumers based
//               on the TypeHandle of the data they share.
////////////////////////////////////////////////////////////////////
void EXPCL_PANDA
register_data_transition(TypeHandle &type_handle, const string &name,
                         TypeHandle derived_from);

#endif

