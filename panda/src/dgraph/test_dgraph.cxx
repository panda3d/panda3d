// Filename: test_dgraph.cxx
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

#include <notify.h>

#include "dataNode.h"
#include "dataRelation.h"
#include "dataGraphTraversal.h"
#include "doubleDataTransition.h"
#include "doubleDataAttribute.h"
#include "vec3DataTransition.h"
#include "vec3DataAttribute.h"

#include <pt_NamedNode.h>


////////////////////////////////////////////////////////////////////
//       Class : Producer
// Description : Simulates some data-generating device, such as a
//               mouse.
////////////////////////////////////////////////////////////////////

class Producer : public DataNode {
public:
  Producer(const string &name);

  virtual void
  transmit_data(NodeAttributes &data);

  NodeAttributes _attrib;
  PT(DoubleDataAttribute) _t;
  PT(Vec3DataAttribute) _xyz;

  static TypeHandle _t_type;
  static TypeHandle _xyz_type;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "Producer",
                  DataNode::get_class_type());

    DoubleDataTransition::init_type();
    register_data_transition(_t_type, "t",
                             DoubleDataTransition::get_class_type());
    Vec3DataTransition::init_type();
    register_data_transition(_xyz_type, "xyz",
                             Vec3DataTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};


TypeHandle Producer::_type_handle;

TypeHandle Producer::_t_type;
TypeHandle Producer::_xyz_type;

Producer::
Producer(const string &name) : DataNode(name) {
  _t = new DoubleDataAttribute(0.0);
  _xyz = new Vec3DataAttribute(LPoint3f(0.0, 0.0, 0.0));

  // Set up our _attrib member to directly reference our data members.
  // This way we can simply return a reference to our _attrib member,
  // without bothering to construct a new one each time.
  _attrib.set_attribute(_t_type, _t);
  _attrib.set_attribute(_xyz_type, _xyz);
}

void Producer::
transmit_data(NodeAttributes &data) {
  nout << get_name() << " sending xyz " << *_xyz << " t " << *_t << "\n";
  data = _attrib;
}




////////////////////////////////////////////////////////////////////
//       Class : Consumer
// Description : Simulates some data-consuming object, such as a mouse
//               Tformer.
////////////////////////////////////////////////////////////////////

class Consumer : public DataNode {
public:
  Consumer(const string &name) : DataNode(name) { }

  virtual void
  transmit_data(NodeAttributes &data);


  static TypeHandle _s_type;
  static TypeHandle _t_type;
  static TypeHandle _xyz_type;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "Consumer",
                  DataNode::get_class_type());

    DoubleDataTransition::init_type();
    register_data_transition(_s_type, "s",
                             DoubleDataTransition::get_class_type());
    register_data_transition(_t_type, "t",
                             DoubleDataTransition::get_class_type());
    Vec3DataTransition::init_type();
    register_data_transition(_xyz_type, "xyz",
                             Vec3DataTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};


TypeHandle Consumer::_type_handle;

TypeHandle Consumer::_s_type;
TypeHandle Consumer::_t_type;
TypeHandle Consumer::_xyz_type;

void Consumer::
transmit_data(NodeAttributes &data) {
  const DoubleDataAttribute *s;
  const DoubleDataAttribute *t;
  const Vec3DataAttribute *xyz;

  if (get_attribute_into(s, data, _s_type) &&
      get_attribute_into(t, data, _t_type) &&
      get_attribute_into(xyz, data, _xyz_type)) {
    nout << get_name() << " got xyz " << *xyz << " s " << *s
         << " t " << *t << "\n";
  } else {
    nout << get_name() << " didn't get all data.\n";
  }
}



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

int
main() {
  Producer::init_type();
  Consumer::init_type();
  PT_NamedNode root = new NamedNode("root");

  PT(Producer) producer = new Producer("a");
  PT(Consumer) consumer = new Consumer("b");
  new DataRelation(root, producer);
  DataRelation *arc1 = new DataRelation(producer, consumer);

  producer->_t->set_value(10.0);
  producer->_xyz->set_value(LPoint3f(1.0, 2.0, 3.0));

  arc1->set_transition(Producer::_xyz_type,
                       new Vec3DataTransition(LMatrix4f::scale_mat(2.0)));

  producer->set_spam_mode(true);

  traverse_data_graph(root);

  return (0);
}
