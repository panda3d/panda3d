// Filename: nodeTransition.h
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODETRANSITION_H
#define NODETRANSITION_H

#include <pandabase.h>

#include <typedWriteableReferenceCount.h>

class Node;
class NodeAttribute;
class NodeAttributes;
class NodeTransitions;
class NodeRelation;
class RenderTraverser;
class AllAttributesWrapper;
class AllTransitionsWrapper;
class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : NodeTransition
// Description : This is an abstract class defining the basic
//               interface to a Transition--the type of state change
//               request that is stored on the arcs of the scene
//               graph.
//
//               See the comments at the beginning of NodeAttribute
//               for a fuller description of the purpose of this
//               class.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeTransition : public TypedWriteableReferenceCount {
protected:
  INLINE_GRAPH NodeTransition();
  INLINE_GRAPH NodeTransition(const NodeTransition &copy);
  INLINE_GRAPH void operator = (const NodeTransition &copy);

public:
  INLINE_GRAPH bool operator == (const NodeTransition &other) const;
  INLINE_GRAPH bool operator != (const NodeTransition &other) const;
  INLINE_GRAPH bool operator < (const NodeTransition &other) const;
  INLINE_GRAPH bool operator <= (const NodeTransition &other) const;
  INLINE_GRAPH bool operator > (const NodeTransition &other) const;
  INLINE_GRAPH bool operator >= (const NodeTransition &other) const;

  INLINE_GRAPH int compare_to(const NodeTransition &other) const;

PUBLISHED:
  INLINE_GRAPH void set_priority(int priority);
  INLINE_GRAPH int get_priority() const;

public:
  virtual NodeTransition *make_copy() const=0;
  virtual NodeAttribute *make_attrib() const=0;

  virtual TypeHandle get_handle() const;

  virtual NodeTransition *compose(const NodeTransition *other) const=0;
  virtual NodeTransition *invert() const=0;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const=0;

  virtual bool sub_render(NodeRelation *arc,
			  const AllAttributesWrapper &attrib,
			  AllTransitionsWrapper &trans,
			  RenderTraverser *trav);
  virtual bool has_sub_render() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const=0;

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

  typedef set<NodeRelation *> Arcs;
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
    TypedWriteableReferenceCount::init_type();
    register_type(_type_handle, "NodeTransition",
		  TypedWriteableReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class NodeRelation;
  friend class NodeTransitions;
  friend class NodeTransitionCache;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NodeTransition &ntb);

#ifndef DONT_INLINE_GRAPH
#include "nodeTransition.I"
#endif

#endif
