// Filename: namedNode.h
// Created by:  drose (15Jan99)
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

#ifndef NAMEDNODE_H
#define NAMEDNODE_H

#include <pandabase.h>

#include "node.h"
#include <namable.h>

///////////////////////////////////////////////////////////////////
//       Class : NamedNode
// Description : A base class for all nodes which have names.  This
//               will be (almost?) all of them.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NamedNode : public Node, public Namable {
PUBLISHED:
  INLINE_GRAPH NamedNode(const string &initial_name = "");

public:
  INLINE_GRAPH NamedNode(const NamedNode &copy);
  INLINE_GRAPH void operator = (const NamedNode &copy);
  virtual ~NamedNode();

  virtual Node *make_copy() const;
  virtual Node *combine_with(Node *other); 

  virtual bool preserve_name() const;

  virtual void output(ostream &out) const;

  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_NamedNode(const FactoryParams &params);

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
    Node::init_type();
    Namable::init_type();
    register_type(_type_handle, "NamedNode",
                  Node::get_class_type(),
                  Namable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

// We need this operator to specify which output operator (from Node
// or Namable) that NamedNode uses.
EXPCL_PANDA INLINE_GRAPH ostream & operator << (ostream &out, const NamedNode &nod);

#ifndef DONT_INLINE_GRAPH
#include "namedNode.I"
#endif

#endif

