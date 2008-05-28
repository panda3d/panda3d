// Filename: pt_EggTexture.h
// Created by:  drose (01May01)
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

#ifndef PT_EGGTEXTURE_H
#define PT_EGGTEXTURE_H

#include "pandabase.h"

#include "eggTexture.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PT_EggTexture
// Description : A PT(EggTexture).  This is defined here solely we can
//               explicitly export the template class.  It's not
//               strictly necessary, but it doesn't hurt.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToBase<EggTexture>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerTo<EggTexture>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, ConstPointerTo<EggTexture>)

typedef PointerTo<EggTexture> PT_EggTexture;
typedef ConstPointerTo<EggTexture> CPT_EggTexture;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
