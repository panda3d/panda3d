// Filename: nodeTransitions.h
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODETRANSITIONS_H
#define NODETRANSITIONS_H

#include <pandabase.h>

#include "nodeTransition.h"

#include <pointerTo.h>

#include <map>

class NodeRelation;

////////////////////////////////////////////////////////////////////
// 	 Class : NodeTransitions
// Description : This represents a set of zero or more NodeTransition
//               pointers, organized by the transitions' get_handle()
//               value.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeTransitions {
public:
  NodeTransitions();
  NodeTransitions(const NodeTransitions &copy);
  void operator = (const NodeTransitions &copy);
  ~NodeTransitions();

  bool is_empty() const;
  PT(NodeTransition) set_transition(TypeHandle handle, NodeTransition *trans);
  INLINE PT(NodeTransition) set_transition(NodeTransition *trans);
  PT(NodeTransition) clear_transition(TypeHandle handle);
  bool has_transition(TypeHandle handle) const;
  NodeTransition *get_transition(TypeHandle handle) const;
  void copy_transitions_from(const NodeTransitions &other);
  void copy_transitions_from(const NodeTransitions &other, 
			     NodeRelation *to_arc);
  void compose_transitions_from(const NodeTransitions &other, 
				NodeRelation *to_arc);

  void clear();

  int compare_to(const NodeTransitions &other) const;

  void remove_all_from_arc(NodeRelation *arc);

private:
  typedef map<TypeHandle, PT(NodeTransition)> Transitions;

public:
  // STL-like definitions to allow read-only traversal of the
  // individual transitions.  Beware!  These are not safe to use
  // outside of PANDA.DLL.
  typedef Transitions::const_iterator iterator;
  typedef Transitions::const_iterator const_iterator;
  typedef Transitions::value_type value_type;
  typedef Transitions::size_type size_type;

  INLINE size_type size() const;
  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

public:
  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  Transitions _transitions;
  friend class NodeTransitionCache;
};

INLINE ostream &operator << (ostream &out, const NodeTransitions &nts) {
  nts.output(out);
  return out;
}

template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const NodeTransitions &trans,
		    TypeHandle transition_type);

template<class Transition>
INLINE bool 
get_transition_into(Transition *&ptr, const NodeTransitions &trans);

#include "nodeTransitions.I"

#endif
