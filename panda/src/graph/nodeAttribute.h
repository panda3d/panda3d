// Filename: nodeAttribute.h
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODEATTRIBUTE_H
#define NODEATTRIBUTE_H

#include <pandabase.h>

#include <typedReferenceCount.h>

class NodeTransition;
class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
// 	 Class : NodeAttribute
// Description : This is an abstract class defining a single
//               Attribute, a state property such as color or texture
//               that may be in effect when rendering nodes of the
//               scene graph.
//
//               In general, the scene graph represents state by
//               encoding transitions between various states on the
//               arcs of the graph.  The attribute values themselves
//               are not explicitly stored; they are computed by
//               repeated application of the transitions.
//
//               A NodeTransition (defined in nodeTransition.h)
//               represents a potential change from any one state (for
//               instance, the initial state) to any other.  For
//               example, it might represent the change from the
//               untextured state to rendering with a particular
//               texture.
//
//               Any number of Transitions may be applied along the
//               arcs leading from the top of the scene graph to a
//               geometry node.  The Attribute state that will be in
//               effect when the geometry node is rendered will be
//               that computed by the consecutive application of each
//               Transition encountered to the initial state.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeAttribute : public TypedReferenceCount {
protected:
  INLINE NodeAttribute();
  INLINE NodeAttribute(const NodeAttribute &copy);
  INLINE void operator = (const NodeAttribute &copy);

public:
  INLINE bool operator == (const NodeAttribute &other) const;
  INLINE bool operator != (const NodeAttribute &other) const;
  INLINE bool operator < (const NodeAttribute &other) const;
  INLINE bool operator <= (const NodeAttribute &other) const;
  INLINE bool operator > (const NodeAttribute &other) const;
  INLINE bool operator >= (const NodeAttribute &other) const;

  INLINE int compare_to(const NodeAttribute &other) const;

PUBLISHED:
  INLINE void set_priority(int priority);
  INLINE int get_priority() const;

public:
  virtual NodeAttribute *make_copy() const=0;
  virtual NodeAttribute *make_initial() const=0;

  virtual TypeHandle get_handle() const=0;

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const=0;

protected:
  int _priority;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "NodeAttribute",
		  TypedReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const NodeAttribute &nab) {
  nab.output(out);
  return out;
}

#include "nodeAttribute.I"

#endif
