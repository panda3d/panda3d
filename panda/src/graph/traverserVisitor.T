// Filename: traverserVisitor.I
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TraverserVisitor::reached_node
//       Access: Public
//  Description: Called for each node visited.  It may return true if
//               the traversal is to be continued, or false if it
//               should be pruned at this node.
////////////////////////////////////////////////////////////////////
template<class TW, class LevelState>
INLINE_GRAPH bool TraverserVisitor<TW, LevelState>::
reached_node(Node *, 
             TraverserVisitor<TW, LevelState>::AttributeWrapper &,
             LevelState &) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TraverserVisitor::reached_node
//       Access: Public
//  Description: Called each time an arc is traversed in the forward
//               direction.  As above, it may return true if the
//               traversal should continue, or false if it should be
//               pruned and not reach the destination node.
////////////////////////////////////////////////////////////////////
template<class TW, class LevelState>
INLINE_GRAPH bool TraverserVisitor<TW, LevelState>:: 
forward_arc(NodeRelation *, 
            TraverserVisitor<TW, LevelState>::TransitionWrapper &,
            TraverserVisitor<TW, LevelState>::AttributeWrapper &, 
            TraverserVisitor<TW, LevelState>::AttributeWrapper &,
            LevelState &) {
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: TraverserVisitor::reached_node
//       Access: Public
//  Description: Called each time an arc is traversed in the backward
//               direction.  It is guaranteed to be paired stack-wise
//               with the corresponding call to forward_arc, so that
//               each call to backward_arc matches the previous
//               unmatched call to forward_arc.
////////////////////////////////////////////////////////////////////
template<class TW, class LevelState>
INLINE_GRAPH void TraverserVisitor<TW, LevelState>::
backward_arc(NodeRelation *, 
             TraverserVisitor<TW, LevelState>::TransitionWrapper &,
             TraverserVisitor<TW, LevelState>::AttributeWrapper &, 
             TraverserVisitor<TW, LevelState>::AttributeWrapper &,
             const LevelState &) {
}
