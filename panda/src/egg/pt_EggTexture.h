// Filename: pt_EggTexture.h
// Created by:  drose (01May01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
