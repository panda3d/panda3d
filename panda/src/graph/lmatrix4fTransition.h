// Filename: lmatrix4fTransition.h
// Created by:  drose (05May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef LMATRIX4FTRANSITION_H
#define LMATRIX4FTRANSITION_H

#include <pandabase.h>

#include "matrixTransition.h"

#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : LMatrix4fTransition
// Description : This is just an instantation of MatrixTransition
//               using LMatrix4f, the most common transform matrix
//               type.
////////////////////////////////////////////////////////////////////

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MatrixTransition<LMatrix4f>);

typedef MatrixTransition<LMatrix4f> LMatrix4fTransition;

#ifdef __GNUC__
#pragma interface
#endif

#endif


