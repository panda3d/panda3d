// Filename: test_graph.cxx
// Created by:  drose (02Feb00)
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

#include "nodeRelation.h"
#include "onOffTransition.h"
#include "onOffAttribute.h"
#include "onTransition.h"
#include "onAttribute.h"
#include "namedNode.h"
#include "pt_NamedNode.h"
#include "wrt.h"
#include "nodeTransitionWrapper.h"
#include "nodeAttributeWrapper.h"
#include "allTransitionsWrapper.h"
#include "allAttributesWrapper.h"
#include "nullTransitionWrapper.h"
#include "nullAttributeWrapper.h"
#include "traverserVisitor.h"
#include "dftraverser.h"
#include "multiTransition.h"
#include "nullLevelState.h"

#include <indent.h>

typedef int TestType;

//#define USE_ONOFF

#ifdef USE_ONOFF
typedef OnOffTransition BaseTransition;
typedef OnOffAttribute BaseAttribute;
#else
typedef OnTransition BaseTransition;
typedef OnAttribute BaseAttribute;
#endif

class TestTransition : public BaseTransition {
public:
  TestTransition();
  TestTransition(TestType value);
#ifdef USE_ONOFF
  static TestTransition off();
#endif

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

  TestType get_value() const;

protected:
  virtual void set_value_from(const BaseTransition *other);
  virtual int compare_values(const BaseTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  TestType _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BaseTransition::init_type();
    register_type(_type_handle, "TestTransition",
                  BaseTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

class TestAttribute : public BaseAttribute {
public:
  virtual TypeHandle get_handle() const;

  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

protected:
  virtual void set_value_from(const BaseTransition *other);
  virtual int compare_values(const BaseAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  TestType _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BaseAttribute::init_type();
    register_type(_type_handle, "TestAttribute",
                  BaseAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

class Test2Transition : public TestTransition {
public:
  Test2Transition();
  Test2Transition(TestType value);
#ifdef USE_ONOFF
  static Test2Transition off();
#endif

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TestTransition::init_type();
    register_type(_type_handle, "Test2Transition",
                  TestTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

class Test2Attribute : public TestAttribute {
public:
  virtual TypeHandle get_handle() const;

  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TestAttribute::init_type();
    register_type(_type_handle, "Test2Attribute",
                  TestAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

class TestMName {
public:
  static string get_class_name() {
    return "TestType";
  }
};

class TestMAttribute : public MultiAttribute<TestType, TestMName> {
protected:
  virtual NodeAttribute *make_copy() const {
    return new TestMAttribute(*this);
  }
  virtual NodeAttribute *make_initial() const {
    return new TestMAttribute();
  }

  virtual TypeHandle get_handle() const;

  virtual void output_property(ostream &out, const TestType &prop) const {
    out << prop;
  }
  virtual void write_property(ostream &out, const TestType &prop,
                              int indent_level) const {
    indent(out, indent_level) << prop << "\n";
  }

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MultiAttribute<TestType, TestMName>::init_type();
    register_type(_type_handle, "TestMAttribute",
                  MultiAttribute<TestType, TestMName>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

class TestMTransition : public MultiTransition<TestType, TestMName> {
protected:

  virtual NodeTransition *make_copy() const {
    return new TestMTransition(*this);
  }
  virtual NodeAttribute *make_attrib() const {
    return new TestMAttribute;
  }
  virtual NodeTransition *make_identity() const {
    return new TestMTransition;
  }
  virtual void output_property(ostream &out, const TestType &prop) const {
    out << prop;
  }
  virtual void write_property(ostream &out, const TestType &prop,
                              int indent_level) const {
    indent(out, indent_level) << prop << "\n";
  }

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MultiTransition<TestType, TestMName>::init_type();
    register_type(_type_handle, "TestMTransition",
                  MultiTransition<TestType, TestMName>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

TypeHandle TestMAttribute::get_handle() const {
  return TestMTransition::get_class_type();
}

template<class TW>
class PrintNodes : public TraverserVisitor<TW, NullLevelState> {
public:
  PrintNodes() {
    _indent_level = 0;
  }
  bool reached_node(Node *node, AttributeWrapper &state, NullLevelState &) {
    indent(nout, _indent_level)
      << "Reached " << *node << ", state is " << state << "\n";
    return true;
  }
  bool forward_arc(NodeRelation *arc, TransitionWrapper &trans,
                   AttributeWrapper &pre, AttributeWrapper &post,
                   NullLevelState &) {
    //    indent(nout, _indent_level + 1)
    //      << "Passing " << *arc << ", trans is " << trans << "\n";
    _indent_level += 2;
    return true;
  }
  void backward_arc(NodeRelation *arc, TransitionWrapper &trans,
                    AttributeWrapper &pre, AttributeWrapper &post,
                    const NullLevelState &) {
    _indent_level -= 2;
  }
  int _indent_level;
};


TestTransition::
TestTransition() {
}

TestTransition::
TestTransition(TestType value) {
  _value = value;
#ifdef USE_ONOFF
  set_on();
#endif
}

#ifdef USE_ONOFF
TestTransition TestTransition::
off() {
  TestTransition tt;
  tt._direction = D_off;
  return tt;
}
#endif

NodeTransition *TestTransition::
make_copy() const {
  return new TestTransition(*this);
}

NodeAttribute *TestTransition::
make_attrib() const {
  return new TestAttribute;
}

TestType TestTransition::
get_value() const {
  return _value;
}

void TestTransition::
set_value_from(const BaseTransition *other) {
  const TestTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

int TestTransition::
compare_values(const BaseTransition *other) const {
  const TestTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value - ot->_value;
}

void TestTransition::
output_value(ostream &out) const {
  out << _value;
}

void TestTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

TypeHandle TestAttribute::
get_handle() const {
  return TestTransition::get_class_type();
}

NodeAttribute *TestAttribute::
make_copy() const {
  return new TestAttribute(*this);
}

NodeAttribute *TestAttribute::
make_initial() const {
  return new TestAttribute;
}

void TestAttribute::
set_value_from(const BaseTransition *other) {
  const TestTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->get_value();
}

int TestAttribute::
compare_values(const BaseAttribute *other) const {
  const TestAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return _value - ot->_value;
}

void TestAttribute::
output_value(ostream &out) const {
  out << _value;
}

void TestAttribute::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

Test2Transition::
Test2Transition() {
}

Test2Transition::
Test2Transition(TestType value) : TestTransition(value) {
}

#ifdef USE_ONOFF
Test2Transition Test2Transition::
off() {
  Test2Transition tt;
  tt._direction = D_off;
  return tt;
}
#endif USE_ONOFF

NodeTransition *Test2Transition::
make_copy() const {
  return new Test2Transition(*this);
}

NodeAttribute *Test2Transition::
make_attrib() const {
  return new Test2Attribute;
}

TypeHandle Test2Attribute::
get_handle() const {
  return Test2Transition::get_class_type();
}

NodeAttribute *Test2Attribute::
make_copy() const {
  return new Test2Attribute(*this);
}

NodeAttribute *Test2Attribute::
make_initial() const {
  return new Test2Attribute;
}

TypeHandle TestTransition::_type_handle;
TypeHandle TestAttribute::_type_handle;
TypeHandle Test2Transition::_type_handle;
TypeHandle Test2Attribute::_type_handle;
TypeHandle TestMTransition::_type_handle;
TypeHandle TestMAttribute::_type_handle;


int main() {
  TestTransition::init_type();
  TestAttribute::init_type();
  Test2Transition::init_type();
  Test2Attribute::init_type();
  TestMTransition::init_type();
  TestMAttribute::init_type();

  PT_NamedNode r = new NamedNode("r");

  PT_NamedNode a = new NamedNode("a");
  PT_NamedNode b = new NamedNode("b");

  PT_NamedNode aa = new NamedNode("aa");
  PT_NamedNode ab = new NamedNode("ab");
  PT_NamedNode ba = new NamedNode("ba");

  NodeRelation *r_a =
    new NodeRelation(r, a, 0, NodeRelation::get_class_type());
  NodeRelation *r_b =
    new NodeRelation(r, b, 0, NodeRelation::get_class_type());

  NodeRelation *a_aa =
    new NodeRelation(a, aa, 0, NodeRelation::get_class_type());
  NodeRelation *a_ab =
    new NodeRelation(a, ab, 0, NodeRelation::get_class_type());
  NodeRelation *b_ba =
    new NodeRelation(b, ba, 0, NodeRelation::get_class_type());

  r_a->set_transition(new TestTransition(1));
  //  a_aa->set_transition(new TestTransition(2));
  a_aa->set_transition(new Test2Transition(3));

#ifdef USE_ONOFF
  a_aa->set_transition(new Test2Transition());
  a_ab->set_transition(new Test2Transition(Test2Transition::off()));
#endif

  /*
  TestMTransition *tm = new TestMTransition;
  tm->set_on(101);
  tm->set_off(102);
  tm->set_identity(103);
  r_a->set_transition(tm);

  tm = new TestMTransition;
  tm->set_on(102);
  a_aa->set_transition(tm);

  tm = new TestMTransition;
  tm->set_complete(true);
  a_ab->set_transition(tm);
  */

  nout << "\nr to a has ";
  r_a->output_transitions(nout);
  nout << "\nr to b has ";
  r_b->output_transitions(nout);
  nout << "\na to aa has ";
  a_aa->output_transitions(nout);
  nout << "\na to ab has ";
  a_ab->output_transitions(nout);
  nout << "\nb to ba has ";
  b_ba->output_transitions(nout);
  nout << "\n";

#if 0
  {
    nout << "\n";
    PrintNodes<NullTransitionWrapper> pn;
    df_traverse(r, pn,
                NullAttributeWrapper(),
                NullLevelState(),
                NodeRelation::get_class_type());
    nout << "\n";

    NullTransitionWrapper result;

    wrt(r, aa, result, NodeRelation::get_class_type());
    nout << "wrt of r to aa is " << result << "\n";

    wrt(aa, r, result, NodeRelation::get_class_type());
    nout << "wrt of aa to r is " << result << "\n";

    wrt(r, ab, result, NodeRelation::get_class_type());
    nout << "wrt of r to ab is " << result << "\n";

    wrt(ab, r, result, NodeRelation::get_class_type());
    nout << "wrt of ab to r is " << result << "\n";

    wrt(ab, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ab to aa is " << result << "\n";

    wrt(aa, ab, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ab is " << result << "\n";

    wrt(r, ba, result, NodeRelation::get_class_type());
    nout << "wrt of r to ba is " << result << "\n";

    wrt(ba, r, result, NodeRelation::get_class_type());
    nout << "wrt of ba to r is " << result << "\n";

    wrt(aa, ba, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ba is " << result << "\n";

    wrt(ba, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ba to aa is " << result << "\n";

    wrt(ab, ba, result, NodeRelation::get_class_type());
    nout << "wrt of ab to ba is " << result << "\n";

    wrt(ba, ab, result, NodeRelation::get_class_type());
    nout << "wrt of ba to ab is " << result << "\n";
  }

  {
    nout << "\n";
    PrintNodes<NodeTransitionWrapper> pn;
    df_traverse(r, pn,
                NodeAttributeWrapper(TestTransition::get_class_type()),
                NullLevelState(),
                NodeRelation::get_class_type());
    nout << "\n";

    NodeTransitionWrapper result(TestTransition::get_class_type());

    wrt(r, aa, result, NodeRelation::get_class_type());
    nout << "wrt of r to aa is " << result << "\n";

    wrt(aa, r, result, NodeRelation::get_class_type());
    nout << "wrt of aa to r is " << result << "\n";

    wrt(r, ab, result, NodeRelation::get_class_type());
    nout << "wrt of r to ab is " << result << "\n";

    wrt(ab, r, result, NodeRelation::get_class_type());
    nout << "wrt of ab to r is " << result << "\n";

    wrt(ab, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ab to aa is " << result << "\n";

    wrt(aa, ab, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ab is " << result << "\n";

    wrt(r, ba, result, NodeRelation::get_class_type());
    nout << "wrt of r to ba is " << result << "\n";

    wrt(ba, r, result, NodeRelation::get_class_type());
    nout << "wrt of ba to r is " << result << "\n";

    wrt(aa, ba, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ba is " << result << "\n";

    wrt(ba, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ba to aa is " << result << "\n";

    wrt(ab, ba, result, NodeRelation::get_class_type());
    nout << "wrt of ab to ba is " << result << "\n";

    wrt(ba, ab, result, NodeRelation::get_class_type());
    nout << "wrt of ba to ab is " << result << "\n";
  }

  {
    nout << "\n";
    PrintNodes<NodeTransitionWrapper> pn;
    df_traverse(r, pn,
                NodeAttributeWrapper(Test2Transition::get_class_type()),
                NullLevelState(),
                NodeRelation::get_class_type());
    nout << "\n";

    NodeTransitionWrapper result(Test2Transition::get_class_type());

    wrt(r, aa, result, NodeRelation::get_class_type());
    nout << "wrt of r to aa is " << result << "\n";

    wrt(aa, r, result, NodeRelation::get_class_type());
    nout << "wrt of aa to r is " << result << "\n";

    wrt(r, ab, result, NodeRelation::get_class_type());
    nout << "wrt of r to ab is " << result << "\n";

    wrt(ab, r, result, NodeRelation::get_class_type());
    nout << "wrt of ab to r is " << result << "\n";

    wrt(ab, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ab to aa is " << result << "\n";

    wrt(aa, ab, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ab is " << result << "\n";

    wrt(r, ba, result, NodeRelation::get_class_type());
    nout << "wrt of r to ba is " << result << "\n";

    wrt(ba, r, result, NodeRelation::get_class_type());
    nout << "wrt of ba to r is " << result << "\n";

    wrt(aa, ba, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ba is " << result << "\n";

    wrt(ba, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ba to aa is " << result << "\n";

    wrt(ab, ba, result, NodeRelation::get_class_type());
    nout << "wrt of ab to ba is " << result << "\n";

    wrt(ba, ab, result, NodeRelation::get_class_type());
    nout << "wrt of ba to ab is " << result << "\n";
  }

  {
    nout << "\n";
    PrintNodes<NodeTransitionWrapper> pn;
    df_traverse(r, pn,
                NodeAttributeWrapper(TestMTransition::get_class_type()),
                NullLevelState(),
                NodeRelation::get_class_type());
    nout << "\n";

    NodeTransitionWrapper result(TestMTransition::get_class_type());

    wrt(r, aa, result, NodeRelation::get_class_type());
    nout << "wrt of r to aa is " << result << "\n";

    wrt(aa, r, result, NodeRelation::get_class_type());
    nout << "wrt of aa to r is " << result << "\n";

    wrt(r, ab, result, NodeRelation::get_class_type());
    nout << "wrt of r to ab is " << result << "\n";

    wrt(ab, r, result, NodeRelation::get_class_type());
    nout << "wrt of ab to r is " << result << "\n";

    wrt(ab, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ab to aa is " << result << "\n";

    wrt(aa, ab, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ab is " << result << "\n";

    wrt(r, ba, result, NodeRelation::get_class_type());
    nout << "wrt of r to ba is " << result << "\n";

    wrt(ba, r, result, NodeRelation::get_class_type());
    nout << "wrt of ba to r is " << result << "\n";

    wrt(aa, ba, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ba is " << result << "\n";

    wrt(ba, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ba to aa is " << result << "\n";

    wrt(ab, ba, result, NodeRelation::get_class_type());
    nout << "wrt of ab to ba is " << result << "\n";

    wrt(ba, ab, result, NodeRelation::get_class_type());
    nout << "wrt of ba to ab is " << result << "\n";
  }

  {
    nout << "\n";
    PrintNodes<AllTransitionsWrapper> pn;
    df_traverse(r, pn,
                AllAttributesWrapper(),
                NullLevelState(),
                NodeRelation::get_class_type());
    nout << "\n";

    AllTransitionsWrapper result;

    wrt(r, aa, result, NodeRelation::get_class_type());
    nout << "wrt of r to aa is " << result << "\n";

    wrt(aa, r, result, NodeRelation::get_class_type());
    nout << "wrt of aa to r is " << result << "\n";

    wrt(r, ab, result, NodeRelation::get_class_type());
    nout << "wrt of r to ab is " << result << "\n";

    wrt(ab, r, result, NodeRelation::get_class_type());
    nout << "wrt of ab to r is " << result << "\n";

    wrt(ab, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ab to aa is " << result << "\n";

    wrt(aa, ab, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ab is " << result << "\n";

    wrt(r, ba, result, NodeRelation::get_class_type());
    nout << "wrt of r to ba is " << result << "\n";

    wrt(ba, r, result, NodeRelation::get_class_type());
    nout << "wrt of ba to r is " << result << "\n";

    wrt(aa, ba, result, NodeRelation::get_class_type());
    nout << "wrt of aa to ba is " << result << "\n";

    wrt(ba, aa, result, NodeRelation::get_class_type());
    nout << "wrt of ba to aa is " << result << "\n";

    wrt(ab, ba, result, NodeRelation::get_class_type());
    nout << "wrt of ab to ba is " << result << "\n";

    wrt(ba, ab, result, NodeRelation::get_class_type());
    nout << "wrt of ba to ab is " << result << "\n";
  }
#endif

  AllTransitionsWrapper result;

  wrt(aa, r, result, NodeRelation::get_class_type());
  nout << "wrt of aa to r is " << result << "\n";
  a_aa->set_transition(new Test2Transition(4));

  wrt(aa, r, result, NodeRelation::get_class_type());
  nout << "wrt of aa to r is now " << result << "\n";

  return 0;
}
