// Filename: get_rel_pos.I
// Created by:  drose (18Feb99)
// 
////////////////////////////////////////////////////////////////////

#include <transformTransition.h>
#include <nodeTransitionWrapper.h>
#include <wrt.h>

////////////////////////////////////////////////////////////////////
//     Function: get_pos
//  Description: Returns the position in space of the node's origin,
//               relative to another node (such as render).
////////////////////////////////////////////////////////////////////
INLINE LPoint3f
get_rel_pos(const Node *node, const Node *relative_to,
            TypeHandle graph_type) {
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  wrt(node, relative_to, ntw, graph_type);
  const TransformTransition *tt;

  if (!get_transition_into(tt, ntw)) {
    // No relative transform.
    return LPoint3f(0.0, 0.0, 0.0);
  }

  LVector3f pos;
  tt->get_matrix().get_row3(pos,3);
  return pos;
}

////////////////////////////////////////////////////////////////////
//     Function: get_mat
//  Description: Returns the net transform of the node, relative to
//               another node (such as render).
////////////////////////////////////////////////////////////////////
INLINE void
get_rel_mat(const Node *node, const Node *relative_to,
            LMatrix4f &mat, TypeHandle graph_type) {
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  wrt(node, relative_to, ntw, graph_type);
  const TransformTransition *tt;
  if (!get_transition_into(tt, ntw)) {
    // No relative transform.
    mat = LMatrix4f::ident_mat();
    return;
  }

  mat = tt->get_matrix();
}


////////////////////////////////////////////////////////////////////
//     Function: get_up
//  Description: Returns the vector which indicates "up"
//               (e.g. the positive z axis, in a Z-up right-handed
//               system) to the the indicated node, relative to
//               another node (such as render).
////////////////////////////////////////////////////////////////////
INLINE LVector3f 
get_rel_up(const Node *node, const Node *relative_to,
           CoordinateSystem cs) {
  LMatrix4f mat;
  get_rel_rot_mat(node, relative_to, mat);
  return LVector3f::up(cs) * mat;
}

////////////////////////////////////////////////////////////////////
//     Function: get_right
//  Description: Returns the vector which indicates "right"
//               (e.g. the positive x axis, in a Z-up right-handed
//               system) to the the indicated node, relative to
//               another node (such as render).
////////////////////////////////////////////////////////////////////
INLINE LVector3f 
get_rel_right(const Node *node, const Node *relative_to,
              CoordinateSystem cs) {
  LMatrix4f mat;
  get_rel_rot_mat(node, relative_to, mat);
  return LVector3f::right(cs) * mat;
}

////////////////////////////////////////////////////////////////////
//     Function: get_forward
//  Description: Returns the vector which indicates "forward"
//               (e.g. the positive y axis, in a Z-up right-handed
//               system) to the the indicated node, relative to
//               another node (such as render).
////////////////////////////////////////////////////////////////////
INLINE LVector3f 
get_rel_forward(const Node *node, const Node *relative_to,
                CoordinateSystem cs) {
  LMatrix4f mat;
  get_rel_rot_mat(node, relative_to, mat);
  return LVector3f::forward(cs) * mat;
}

////////////////////////////////////////////////////////////////////
//     Function: get_down
//  Description: Returns the vector which indicates "down"
//               (e.g. the negative y axis, in a Z-up right-handed
//               system) to the the indicated node, relative to
//               another node (such as render).
////////////////////////////////////////////////////////////////////
INLINE LVector3f 
get_rel_down(const Node *node, const Node *relative_to,
             CoordinateSystem cs) {
  LMatrix4f mat;
  get_rel_rot_mat(node, relative_to, mat);
  return LVector3f::down(cs) * mat;
}

////////////////////////////////////////////////////////////////////
//     Function: get_left
//  Description: Returns the vector which indicates "left"
//               (e.g. the negative x axis, in a Z-up right-handed
//               system) to the the indicated node, relative to
//               another node (such as render).
////////////////////////////////////////////////////////////////////
INLINE LVector3f 
get_rel_left(const Node *node, const Node *relative_to,
             CoordinateSystem cs) {
  LMatrix4f mat;
  get_rel_rot_mat(node, relative_to, mat);
  return LVector3f::left(cs) * mat;
}

////////////////////////////////////////////////////////////////////
//     Function: get_back
//  Description: Returns the vector which indicates "back"
//               (e.g. the negative y axis, in a Z-up right-handed
//               system) to the the indicated node, relative to
//               another node (such as render).
////////////////////////////////////////////////////////////////////
INLINE LVector3f 
get_rel_back(const Node *node, const Node *relative_to,
             CoordinateSystem cs) {
  LMatrix4f mat;
  get_rel_rot_mat(node, relative_to, mat);
  return LVector3f::back(cs) * mat;
}
