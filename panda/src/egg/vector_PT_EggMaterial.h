// Filename: vector_PT_EggMaterial.h
// Created by:  drose (01May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VECTOR_PT_EGGMATERIAL_H
#define VECTOR_PT_EGGMATERIAL_H

#include <pandabase.h>

#include "eggMaterial.h"
#include "pt_EggMaterial.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : vector_PT_EggMaterial
// Description : A vector of PT(EggMaterial)'s.  This class is defined once
//               here, and exported to PANDAEGG.DLL; other packages
//               that want to use a vector of this type (whether they
//               need to export it or not) should include this header
//               file, rather than defining the vector again.
////////////////////////////////////////////////////////////////////

#ifdef HAVE_DINKUM
#define VV_PT_EGGMATERIAL std::_Vector_val<PT_EggMaterial, std::allocator<PT_EggMaterial> >
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, VV_PT_EGGMATERIAL)
#endif
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, std::vector<PT_EggMaterial>)
typedef vector<PT_EggMaterial> vector_PT_EggMaterial;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
