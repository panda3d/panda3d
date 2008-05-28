// Filename: pt_EggVertex.h
// Created by:  drose (22Feb01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PT_EGGVERTEX_H
#define PT_EGGVERTEX_H

#include "pandabase.h"

#include "eggVertex.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PT_EggVertex
// Description : A PT(EggVertex).  This is defined here solely we can
//               explicitly export the template class.  It's not
//               strictly necessary, but it doesn't hurt.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToBase<EggVertex>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerTo<EggVertex>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, ConstPointerTo<EggVertex>)

typedef PointerTo<EggVertex> PT_EggVertex;
typedef ConstPointerTo<EggVertex> CPT_EggVertex;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
