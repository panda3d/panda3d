// Filename: get_rel_pos.h
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

#ifndef GET_REL_POS_H
#define GET_REL_POS_H

#include <pandabase.h>

#include <luse.h>
#include <lmatrix.h>
#include <coordinateSystem.h>
#include <renderRelation.h>

class Node;

INLINE LPoint3f EXPCL_PANDA
get_rel_pos(const Node *node, const Node *relative_to,
            TypeHandle graph_type = RenderRelation::get_class_type());
INLINE void EXPCL_PANDA
get_rel_mat(const Node *node, const Node *relative_to,
            LMatrix4f &mat,
            TypeHandle graph_type = RenderRelation::get_class_type());
void EXPCL_PANDA
get_rel_rot_mat(const Node *node, const Node *relative_to,
                LMatrix4f &mat,
                TypeHandle graph_type = RenderRelation::get_class_type());

LVecBase3f EXPCL_PANDA
get_rel_scale(const Node *node, const Node *relative_to,
              TypeHandle graph_type = RenderRelation::get_class_type());


INLINE LVector3f EXPCL_PANDA
get_rel_up(const Node *node, const Node *relative_to,
           CoordinateSystem cs = CS_default);
INLINE LVector3f EXPCL_PANDA
get_rel_right(const Node *node, const Node *relative_to,
              CoordinateSystem cs = CS_default);
INLINE LVector3f EXPCL_PANDA
get_rel_forward(const Node *node, const Node *relative_to,
                CoordinateSystem cs = CS_default);
INLINE LVector3f EXPCL_PANDA
get_rel_down(const Node *node, const Node *relative_to,
             CoordinateSystem cs = CS_default);
INLINE LVector3f EXPCL_PANDA
get_rel_left(const Node *node, const Node *relative_to,
             CoordinateSystem cs = CS_default);
INLINE LVector3f EXPCL_PANDA
get_rel_back(const Node *node, const Node *relative_to,
             CoordinateSystem cs = CS_default);

#include "get_rel_pos.I"

#endif

