// Filename: get_rel_pos.I
// Created by:  drose (18Feb99)
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
