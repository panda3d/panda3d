// Filename: pt_NamedNode.h
// Created by:  drose (16May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PT_NAMEDNODE_H
#define PT_NAMEDNODE_H

#include <pandabase.h>

#include "namedNode.h"

#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : PT_NamedNode
// Description : A PT(NamedNode).  This is defined here solely we can
//               explicitly export the template class.  It's not
//               strictly necessary, but it doesn't hurt.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<NamedNode>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerTo<NamedNode>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerTo<NamedNode>)

typedef PointerTo<NamedNode> PT_NamedNode;
typedef ConstPointerTo<NamedNode> CPT_NamedNode;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
