// Filename: vector_PT_Light.h
// Created by:  drose (16May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_PT_LIGHT_H
#define VECTOR_PT_LIGHT_H

#include <pandabase.h>

#include "pt_Light.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_PT_Light
// Description : A vector of PT(Light)'s.  This class is defined once
//               here, and exported to PANDA.DLL; other packages that
//               want to use a vector of this type (whether they need
//               to export it or not) should include this header file,
//               rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#ifdef HAVE_DINKUM
#define VV_PT_LIGHT std::_Vector_val<PT_Light, std::allocator<PT_Light> >
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VV_PT_LIGHT)
#endif
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, std::vector<PT_Light>)
typedef vector<PT_Light> vector_PT_Light;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
