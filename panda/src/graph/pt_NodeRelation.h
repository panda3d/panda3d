// Filename: pt_NodeRelation.h
// Created by:  drose (07May01)
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

#ifndef PT_NODERELATION_H
#define PT_NODERELATION_H

#include <pandabase.h>

#include "nodeRelation.h"
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : PT_NodeRelation
// Description : PT(NodeRelation).  This is defined here solely we can
//               explicitly export the template class.  It's not
//               strictly necessary, but it doesn't hurt.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<NodeRelation>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerTo<NodeRelation>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerTo<NodeRelation>)

typedef PointerTo<NodeRelation> PT_NodeRelation;
typedef ConstPointerTo<NodeRelation> CPT_NodeRelation;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
