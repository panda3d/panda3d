// Filename: cullStateLookup.h
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CULLSTATELOOKUP_H
#define CULLSTATELOOKUP_H

#include <pandabase.h>

#include <geomNode.h>
#include <pointerTo.h>
#include <pt_Node.h>

#include <map>

class CullState;
class CullStateSubtree;

////////////////////////////////////////////////////////////////////
// 	 Class : CullStateLookup
// Description : This object stores a set of CullState pointers, one
//               for each unique state encountered in the scene graph.
//               It can combine two identical states encountered at
//               different parts of the scene graph into the same
//               CullState pointer.
//
//               It also supports instancing by maintaining a tree of
//               instanced nodes: for each node in the scene graph
//               with multiple parents, a new CullStateLookup object
//               (actually, a CullStateSubtree object) is kept which
//               tracks the state changes below that node.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullStateLookup {
public:
  CullStateLookup();
  virtual ~CullStateLookup();

  void clear();
  INLINE bool is_completely_empty() const;

  CullState *find_node(Node *node, 
		       const AllTransitionsWrapper &trans,
		       UpdateSeq now);
  CullStateSubtree *get_subtree(const PT(NodeRelation) &arc,
				const AllTransitionsWrapper &trans,
				Node *top_subtree,
				UpdateSeq now);

  INLINE void record_node(Node *node, CullState *cs,
			  UpdateSeq now);

  void clean_out_old_nodes();

  virtual Node *get_top_subtree() const;

  virtual void compose_trans(const AllTransitionsWrapper &from,
			     AllTransitionsWrapper &to) const;

  virtual void write(ostream &out, int indent_level = 0) const;

private:
  typedef map<PT_Node, PT(CullState)> CullStates;
  CullStates _cull_states;

  typedef map<PT(NodeRelation), CullStateSubtree *> Subtrees;
  Subtrees _subtrees;
};

#include "cullStateLookup.I"

#endif
