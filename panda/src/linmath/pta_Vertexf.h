// Filename: pta_Vertexf.h
// Created by:  drose (10May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTA_VERTEXF_H
#define PTA_VERTEXF_H

#include <pandabase.h>

#include "vector_Vertexf.h"

#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
//       Class : PTA_Vertexf
// Description : A pta of Vertexfs.  This class is defined once here,
//               and exported to PANDA.DLL; other packages that want
//               to use a pta of this type (whether they need to
//               export it or not) should include this header file,
//               rather than defining the pta again.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, RefCountObj<vector_Vertexf>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToBase<RefCountObj<vector_Vertexf> >);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerToArray<Vertexf>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, ConstPointerToArray<Vertexf>);

typedef PointerToArray<Vertexf> PTA_Vertexf;
typedef ConstPointerToArray<Vertexf> CPTA_Vertexf;

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
