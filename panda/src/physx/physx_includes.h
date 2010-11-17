// Filename: ode_includes.h
// Created by:  joswilso (30Jan07)
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

#ifndef PHYSX_INCLUDES_H
#define PHYSX_INCLUDES_H


// Platform-specific defines
#if NATIVE_WORDSIZE == 64
#define NX64
#endif

#if NATIVE_WORDSIZE == 32
#define NX32
#endif

#ifdef IS_LINUX
#define LINUX 1
#define CORELIB 1
#define NX_DISABLE_FLUIDS 1
#define NX_DISABLE_HARDWARE 1
#endif


// Undefine min and max before any PhysX headers get included
#undef min
#undef max


// PhysX headers
#include "NxPhysics.h"
#include "NxExtended.h"
#include "NxStream.h"
#include "NxCooking.h"
#include "NxController.h"
#include "NxControllerManager.h"
#include "NxBoxController.h"
#include "NxCapsuleController.h"


#endif // PHYSX_INCLUDES_H
