// Filename: attribTraverser.h
// Created by:  mike (16Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef ATTRIBTRAVERSER_H
#define ATTRIBTRAVERSER_H

#include <pandabase.h>

#include <traverserVisitor.h>
#include <nodeTransitionWrapper.h>
#include <nodeAttributeWrapper.h>
#include <nullLevelState.h>

class AllAttributesWrapper;

////////////////////////////////////////////////////////////////////
//       Class : AttribTraverser
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AttribTraverser : 
  public TraverserVisitor<NodeTransitionWrapper, NullLevelState> {
public:
  AttribTraverser();
  bool reached_node(Node *node, NodeAttributeWrapper &state, NullLevelState &);
  bool forward_arc(NodeRelation *arc, TransitionWrapper &trans,
                   NodeAttributeWrapper &, NodeAttributeWrapper &,
                   NullLevelState &);


  void set_attrib_type(TypeHandle type);
  void set_transition_type(TypeHandle type);
public:
  bool _has_attrib;
  bool _has_transition;
private:
  TypeHandle _attrib_type;
  TypeHandle _transition_type;
};

bool EXPCL_PANDA is_textured(Node* root);
bool EXPCL_PANDA is_textured(Node* root, const AllAttributesWrapper &init_state);

bool EXPCL_PANDA is_shaded(Node* root);

#endif

