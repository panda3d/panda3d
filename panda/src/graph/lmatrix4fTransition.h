// Filename: lmatrix4fTransition.h
// Created by:  drose (05May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LMATRIX4FTRANSITION_H
#define LMATRIX4FTRANSITION_H

#include <pandabase.h>

#include "matrixTransition.h"
#include "matrixAttribute.h"

#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : LMatrix4fTransition
// Description : This is just an instantation of MatrixTransition
//               using LMatrix4f, the most common transform matrix
//               type.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MatrixTransition<LMatrix4f>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MatrixAttribute<LMatrix4f>);

typedef MatrixTransition<LMatrix4f> LMatrix4fTransition;
typedef MatrixAttribute<LMatrix4f> LMatrix4fAttribute;

#ifdef __GNUC__
#pragma interface
#endif

#endif


