// Filename: pta_TexCoordf.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_TEXCOORDF_H
#define PTA_TEXCOORDF_H

#include <pandabase.h>

#include "vector_TexCoordf.h"

#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
//       Class : PTA_TexCoordf
// Description : A pta of TexCoordfs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, RefCountObj<vector_TexCoordf>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<RefCountObj<vector_TexCoordf> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToArray<TexCoordf>)
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerToArray<TexCoordf>)

typedef PointerToArray<TexCoordf> PTA_TexCoordf;
typedef ConstPointerToArray<TexCoordf> CPTA_TexCoordf;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
