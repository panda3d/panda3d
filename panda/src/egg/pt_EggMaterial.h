// Filename: pt_EggMaterial.h
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

#ifndef PT_EGGMATERIAL_H
#define PT_EGGMATERIAL_H

#include "pandabase.h"

#include "eggMaterial.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PT_EggMaterial
// Description : A PT(EggMaterial).  This is defined here solely we can
//               explicitly export the template class.  It's not
//               strictly necessary, but it doesn't hurt.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerToBase<EggMaterial>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, PointerTo<EggMaterial>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, ConstPointerTo<EggMaterial>)

typedef PointerTo<EggMaterial> PT_EggMaterial;
typedef ConstPointerTo<EggMaterial> CPT_EggMaterial;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
