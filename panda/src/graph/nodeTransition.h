// Filename: nodeTransition.h
// Created by:  drose (20Mar00)
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

#ifndef NODETRANSITION_H
#define NODETRANSITION_H

#include "pandabase.h"

#include "graphHashGenerator.h"

#include "pset.h"
#include "typedWritableReferenceCount.h"

class Node;
class NodeTransitions;
class NodeRelation;
class RenderTraverser;
class AllTransitionsWrapper;
class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;
class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : NodeTransition
// Description : This is an abstract class defining the basic
//               interface to a Transition--the type of state change
//               request that is stored on the arcs of the scene
//               graph.
//
//               In general, the scene graph represents state by
//               encoding transitions between various states on the
//               arcs of the graph.  The state of a particular node is
//               determined by the composition of all the transitions
//               on arcs between that node and the root.
//
//               A NodeTransition represents a particular state, or a
//               change from any one state to another.  For example,
//               it might represent the change from the untextured
//               state to rendering with a particular texture, which
//               can also be thought of as representing the state of
//               rendering with that texture.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeTransition : public TypedWritableReferenceCount {
protected:
  INLINE_GRAPH NodeTransition();
  INLINE_GRAPH NodeTransition(const NodeTransition &copy);
  INLINE_GRAPH void operator = (const NodeTransition &copy);

public:
  typedef GraphHashGenerator HashGenerator;

  INLINE_GRAPH bool operator == (const NodeTransition &other) const;
  INLINE_GRAPH bool operator != (const NodeTransition &other) const;
  INLINE_GRAPH bool operator < (const NodeTransition &other) const;
  INLINE_GRAPH bool operator <= (const NodeTransition &other) const;
  INLINE_GRAPH bool operator > (const NodeTransition &other) const;
  INLINE_GRAPH bool operator >= (const NodeTransition &other) const;

  int compare_to(const NodeTransition &other) const;
  int compare_to_ignore_priority(const NodeTransition &other) const;
  INLINE_GRAPH void generate_hash(GraphHashGenerator &hashgen) const;

PUBLISHED:
  INLINE_GRAPH void set_priority(int priority);
  INLINE_GRAPH int get_priority() const;

public:
  virtual NodeTransition *make_copy() const=0;
  virtual NodeTransition *make_initial() const;

  virtual TypeHandle get_handle() const;

  virtual NodeTransition *compose(const NodeTransition *other) const=0;
  virtual NodeTransition *invert() const=0;

  virtual bool sub_render(NodeRelation *arc,
                          const AllTransitionsWrapper &input_trans,
                          AllTransitionsWrapper &modify_trans,
                          RenderTraverser *trav);
  virtual bool has_sub_render() const;

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual int internal_compare_to(const NodeTransition *other) const=0;
  virtual void internal_generate_hash(GraphHashGenerator &hashgen) const;

  // And this is the internal function we'll call whenever our value
  // changes.
  void state_changed();

public:
  // We need to keep a list of all of the arcs we're assigned to, so
  // that if our value changes we can keep the cache up-to-date.
  // These functions will be called by the arcs when appropriate.
  // Don't attempt to call them directly; they're public only to make
  // it convenient to call them from non-class template functions.
  INLINE_GRAPH void added_to_arc(NodeRelation *arc);
  INLINE_GRAPH void removed_from_arc(NodeRelation *arc);

protected:
  int _priority;

  typedef pset<NodeRelation *> Arcs;
  Arcs _arcs;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  //NodeTransition has no factory methods as it is not directly made

protected:
  virtual void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "NodeTransition",
                  TypedWritableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class NodeRelation;
  friend class NodeTransitions;
  friend class NodeTransitionCache;
};

EXPCL_PANDA INLINE_GRAPH ostream &
operator << (ostream &out, const NodeTransition &ntb);

#ifndef DONT_INLINE_GRAPH
#include "nodeTransition.I"
#endif

#endif
