// Filename: nodeRelation.h
// Created by:  drose (26Oct98)
//
////////////////////////////////////////////////////////////////////

#ifndef NODERELATION_H
#define NODERELATION_H

#include <pandabase.h>

#include "nodeTransitions.h"
#include "nodeTransitionCache.h"
#include "boundedObject.h"
#include "pt_Node.h"

#include <typedWriteableReferenceCount.h>
#include <bamWriter.h>
#include <bamReader.h>
#include <pointerTo.h>
#include <updateSeq.h>
#include <factory.h>

#include <set>
#include <map>
#include <stdlib.h>

class Node;

// This structure keeps a monotonically incrementing sequence number
// for each change made to the graph, for the purpose of invalidating
// wrt cache values.  A different sequence number is kept for each
// type of graph, hence the map.
typedef map<TypeHandle, UpdateSeq> LastGraphUpdate;
extern LastGraphUpdate EXPCL_PANDA last_graph_update;

///////////////////////////////////////////////////////////////////
// 	 Class : NodeRelation
// Description : The base class for all scene graph arcs.  This is the
//               glue between Nodes that defines the scene graph.
//               There are no arcs of type NodeRelation per se, but
//               there may be any number of types of arcs that inherit
//               from NodeRelation.
//
//               All arcs are directed binary relations, from a parent
//               node to a child node, and may include any number of
//               NodeTransitions which affect the attributes of the
//               child node and later descendants.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeRelation : public TypedWriteableReferenceCount, public BoundedObject {
public:
  INLINE NodeRelation(Node *from, Node *to, int sort, TypeHandle type);

protected:
  // Normally, this should only be used from derived classes for
  // passing to the factory.  Don't attempt to create an unattached
  // arc directly.
  INLINE NodeRelation(TypeHandle type);

  //make_NodeRelation needs to have a construct that takes nothing
  //since it creates the object before it has any information
  INLINE NodeRelation(void);

private:
  // It is an error to attempt to copy an arc.
  NodeRelation(const NodeRelation &copy);
  void operator = (const NodeRelation &copy);

public:
  // The destructor needs to be virtual so that we can delete the
  // NodeRelations of various types when the Node they're attached to
  // destructs.  However, a NodeRelation should not generally be
  // deleted directly, because it is reference-counted in the tree;
  // instead, you should call remove_arc().
  virtual ~NodeRelation();

  void output(ostream &out) const;
  INLINE void output_transitions(ostream &out) const;
  INLINE void write_transitions(ostream &out, int indent = 0) const;

  INLINE Node *get_parent() const;
  INLINE Node *get_child() const;
  INLINE int get_sort() const;

  INLINE void change_parent(Node *parent);
  INLINE void change_child(Node *child);
  INLINE void change_parent_and_child(Node *parent, Node *child);
  INLINE void set_sort(int sort);

  INLINE PT(NodeTransition) set_transition(TypeHandle handle, 
					   NodeTransition *trans);
  INLINE PT(NodeTransition) set_transition(NodeTransition *trans);
  INLINE PT(NodeTransition) set_transition(NodeTransition *trans, int priority);
  INLINE PT(NodeTransition) clear_transition(TypeHandle handle);
  INLINE bool has_transition(TypeHandle handle) const;
  INLINE bool has_any_transition() const;
  INLINE NodeTransition *get_transition(TypeHandle handle) const;
  void copy_transitions_from(const NodeRelation *arc);
  void compose_transitions_from(const NodeRelation *arc);
  void copy_transitions_from(const NodeTransitions &trans);
  void compose_transitions_from(const NodeTransitions &trans);

  INLINE int compare_transitions_to(const NodeRelation *arc) const;

  bool sub_render_trans(const AllAttributesWrapper &attrib,
			AllTransitionsWrapper &trans,
			GraphicsStateGuardianBase *gsgbase);
  bool has_sub_render_trans() const;

public:
  // Factory stuff: to create a new NodeRelation based on its
  // TypeHandle.
  INLINE static NodeRelation *
  create_typed_arc(TypeHandle type, Node *parent, Node *child);

  // This is just to be called at initialization time; don't try to
  // call this directly.
  INLINE static void register_with_factory();

protected:
  INLINE static Factory<NodeRelation> &get_factory(); 
 
private:
  static NodeRelation *make_arc(const FactoryParams &params);
  static Factory<NodeRelation> *_factory;

protected:
  void attach();
  PT(NodeRelation) detach();
  PT(NodeRelation) detach_below();

private:
  // We reference-count the child pointer, but not the parent pointer,
  // to avoid circular reference counting.

  Node *_parent;
  PT_Node _child;
  int _sort;
  TypeHandle _type;
  bool _attached;

private:
  // This is the set of transitions assigned to the arc.  You should
  // never access this directly; use set_transition() and friends
  // instead.
  NodeTransitions _transitions;

  // This stores the known net transitions from the _top_subtree node,
  // which is either NULL to indicate the root of the graph, or a
  // pointer to the nearest descendent with multiple parents.  You
  // should never even attempt to access it directly; it exists only
  // to support caching in wrt().
  PT(NodeTransitionCache) _net_transitions;
  Node *_top_subtree;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

  static TypedWriteable *make_NodeRelation(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

private:
  //This value is only used for the process of re-construction
  //from a binary source. DO NOT ACCESS.  The value is only 
  //guaranteed to be accurate during that process
  int _num_transitions;

public:
  virtual void changed_transition(TypeHandle transition_type);

protected:
  virtual void propagate_stale_bound();
  virtual void recompute_bound();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWriteableReferenceCount::init_type();
    BoundedObject::init_type();
    register_type(_type_handle, "NodeRelation",
		  TypedWriteableReferenceCount::get_class_type(),
		  BoundedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend INLINE void remove_arc(NodeRelation *arc);
  friend class Node;
  friend class NodeTransitionWrapper;
  friend class AllTransitionsWrapper;
};

INLINE ostream &
operator << (ostream &out, const NodeRelation &arc) {
  arc.output(out);
  return out;
}

INLINE void remove_arc(NodeRelation *arc);

template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const NodeRelation *arc,
		    TypeHandle transition_type);

template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const NodeRelation *arc);

typedef vector<PT(NodeRelation)> DownRelationPointers;
typedef map<TypeHandle, DownRelationPointers> DownRelations;

typedef vector<NodeRelation *> UpRelationPointers;
typedef map<TypeHandle, UpRelationPointers> UpRelations;

#include "nodeRelation.I"

// We include node.h here at the end, so we'll know that nodes are of
// type ReferenceCount and will be able to compile anything that uses
// a NodeRelation.  We have to include it here at the end instead of
// the beginning because node.h also includes nodeRelation.h.
#include "node.h"

#endif
