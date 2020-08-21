/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pre_maya_include.h
 * @author drose
 * @date 2002-04-11
 */

// This header file defines a few things that are necessary to define before
// including any Maya headers, just to work around some of Maya's assumptions
// about the compiler.  It must not try to protect itself from multiple
// inclusion with #ifdef .. #endif, since it must be used each time it is
// included.

// Maya 2008 will declare some VS2005-specific hacks unless we define this.
#if defined(_MSC_VER) && _MSC_VER < 1400
#define MLIBRARY_DONTUSE_MFC_MANIFEST
#endif

// Maya will try to typedef bool unless this symbol is defined.
#ifndef _BOOL
#define _BOOL 1
#endif

// In Maya 5.0, the headers seem to provide the manifest REQUIRE_IOSTREAM,
// which forces it to use the new <iostream> headers instead of the old
// <iostream.h> headers.  It also says this is for Linux only, but it seems to
// work just fine on Windows, obviating the need for sneaky #defines in this
// and in post_maya_include.h.
#ifdef PHAVE_IOSTREAM
#define REQUIRE_IOSTREAM
#endif  // PHAVE_IOSTREAM

#ifdef __MACH__
#define OSMac_ 1
// This defines MAYA_API_VERSION
#include <maya/MTypes.h>
#if MAYA_API_VERSION < 201600
#include <maya/OpenMayaMac.h>
#endif
#else
// This defines MAYA_API_VERSION
#include <maya/MTypes.h>
#endif

#if MAYA_API_VERSION >= 20180000
#include <maya/MApiNamespace.h>
#else
class MObject;
class MDagPath;
class MFloatArray;
class MFnDagNode;
class MFnMesh;
class MFnNurbsCurve;
class MFnNurbsSurface;
class MPlug;
class MPointArray;
#endif
