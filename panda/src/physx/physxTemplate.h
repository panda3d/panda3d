// Filename: physxTemplate.h
// Created by:  pratt (Dec 12, 2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSXTEMPLATE_H
#define PHYSXTEMPLATE_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

//superHeader
//forwardDeclarations
#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxTemplate
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxTemplate /*baseclassref*/{
PUBLISHED:
//publicstructors
//publicmethods
//publicattributes
public:
//nxreference
//typeinfo
};

#include "physxTemplate.I"

#endif // HAVE_PHYSX

#endif // PHYSXTEMPLATE_H
