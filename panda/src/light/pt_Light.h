// Filename: pt_Light.h
// Created by:  drose (16May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PT_LIGHT_H
#define PT_LIGHT_H

#include <pandabase.h>

#include "light.h"

#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : PT_Light
// Description : A PT(Light).  This is defined here solely we can
//               explicitly export the template class.  It's not
//               strictly necessary, but it doesn't hurt.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<Light>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerTo<Light>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerTo<Light>)

typedef PointerTo<Light> PT_Light;
typedef ConstPointerTo<Light> CPT_Light;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
