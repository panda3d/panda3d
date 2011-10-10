// Filename: mesagsg.h
// Created by:  drose (09Feb04)
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

#ifndef MESAGSG_H
#define MESAGSG_H

// This is the actual header file to include if you want to pick up
// any or all of the header files in this directory as compiled to use
// the Mesa library.

#include "pandabase.h"
#include "config_mesadisplay.h"

// This stuff seems to crash with Mesa.
#undef HAVE_CG
#undef HAVE_CGGL

#ifdef MESA_MGL
  #define GLP(name) mgl##name

#ifndef STDFLOAT_DOUBLE
#define GLPf(name) mgl ## name ## f
#define GLPfv(name) mgl ## name ## fv
#else  // STDFLOAT_DOUBLE
#define GLPf(name) mgl ## name ## d
#define GLPfv(name) mgl ## name ## dv
#endif  // STDFLOAT_DOUBLE

  #define GLPREFIX_QUOTED "mgl"
  #define USE_MGL_NAMESPACE 1

#else  // MESA_MGL

#ifndef STDFLOAT_DOUBLE
#define GLPf(name) gl ## name ## f
#define GLPfv(name) gl ## name ## fv
#else  // STDFLOAT_DOUBLE
#define GLPf(name) gl ## name ## d
#define GLPfv(name) gl ## name ## dv
#endif  // STDFLOAT_DOUBLE

  #define GLP(name) gl##name
  #define GLPREFIX_QUOTED "gl"

#endif  // MESA_MGL

#define CLP(name) Mesa##name
#define CLASSPREFIX_QUOTED "Mesa"
#define GLSYSTEM_NAME "Mesa"
#define CONFIGOBJ config_mesadisplay
#define GLCAT mesadisplay_cat
#define EXPCL_GL EXPCL_PANDAMESA
#define EXPTP_GL EXPTP_PANDAMESA

#if MIN_MESA_VERSION_MAJOR > 1 || (MIN_MESA_VERSION_MAJOR == 1 && MIN_MESA_VERSION_MINOR >= 2)
#define EXPECT_GL_VERSION_1_2
#endif

#if MIN_MESA_VERSION_MAJOR > 1 || (MIN_MESA_VERSION_MAJOR == 1 && MIN_MESA_VERSION_MINOR >= 3)
#define EXPECT_GL_VERSION_1_3
#endif

#if MIN_MESA_VERSION_MAJOR > 1 || (MIN_MESA_VERSION_MAJOR == 1 && MIN_MESA_VERSION_MINOR >= 4)
#define EXPECT_GL_VERSION_1_4
#endif

#if MIN_MESA_VERSION_MAJOR > 1 || (MIN_MESA_VERSION_MAJOR == 1 && MIN_MESA_VERSION_MINOR >= 5)
#define EXPECT_GL_VERSION_1_5
#endif

#if MIN_MESA_VERSION_MAJOR > 2 || (MIN_MESA_VERSION_MAJOR == 2 && MIN_MESA_VERSION_MINOR >= 0)
#define EXPECT_GL_VERSION_2_0
#endif

#if MIN_MESA_VERSION_MAJOR > 2 || (MIN_MESA_VERSION_MAJOR == 2 && MIN_MESA_VERSION_MINOR >= 1)
#define EXPECT_GL_VERSION_2_1
#endif

// This prevents glext.h from getting included by gl.h
// That way, we can provide our own, better version.
#define __glext_h_
#define GL_GLEXT_VERSION 0

#include <GL/gl.h>
#include <GL/osmesa.h>

#undef GL_GLEXT_VERSION
#include "panda_glext.h"

#include "glstuff_src.h"

#endif  // MESAGSG_H
