// Filename: pt_EggMaterial.h
// Created by:  drose (01May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef PT_EGGMATERIAL_H
#define PT_EGGMATERIAL_H

#include <pandabase.h>

#include "eggMaterial.h"
#include <pointerTo.h>

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
