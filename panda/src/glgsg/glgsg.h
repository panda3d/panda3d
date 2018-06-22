/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glgsg.h
 * @author drose
 * @date 2004-02-09
 */

#ifndef GLGSG_H
#define GLGSG_H

// This is the actual header file to include if you want to pick up any or all
// of the header files in this directory as compiled to use the "true" GL
// library.

#include "pandabase.h"
#include "config_glgsg.h"

#define GLP(name) gl ## name

#ifndef STDFLOAT_DOUBLE
#define GLPf(name) gl ## name ## f
#define GLPfv(name) gl ## name ## fv
#else  // STDFLOAT_DOUBLE
#define GLPf(name) gl ## name ## d
#define GLPfv(name) gl ## name ## dv
#endif  // STDFLOAT_DOUBLE

#define CLP(name) GL ## name
#define GLPREFIX_QUOTED "gl"
#define CLASSPREFIX_QUOTED "GL"
#define GLSYSTEM_NAME "OpenGL"
#define CONFIGOBJ config_glgsg
#define GLCAT glgsg_cat
#define EXPCL_GL EXPCL_PANDA_GLGSG
#define EXPTP_GL EXPTP_PANDA_GLGSG

#if MIN_GL_VERSION_MAJOR > 1 || (MIN_GL_VERSION_MAJOR == 1 && MIN_GL_VERSION_MINOR >= 2)
#define EXPECT_GL_VERSION_1_2
#endif

#if MIN_GL_VERSION_MAJOR > 1 || (MIN_GL_VERSION_MAJOR == 1 && MIN_GL_VERSION_MINOR >= 3)
#define EXPECT_GL_VERSION_1_3
#endif

#if MIN_GL_VERSION_MAJOR > 1 || (MIN_GL_VERSION_MAJOR == 1 && MIN_GL_VERSION_MINOR >= 4)
#define EXPECT_GL_VERSION_1_4
#endif

#if MIN_GL_VERSION_MAJOR > 1 || (MIN_GL_VERSION_MAJOR == 1 && MIN_GL_VERSION_MINOR >= 5)
#define EXPECT_GL_VERSION_1_5
#endif

#if MIN_GL_VERSION_MAJOR > 2 || (MIN_GL_VERSION_MAJOR == 2 && MIN_GL_VERSION_MINOR >= 0)
#define EXPECT_GL_VERSION_2_0
#endif

#if MIN_GL_VERSION_MAJOR > 2 || (MIN_GL_VERSION_MAJOR == 2 && MIN_GL_VERSION_MINOR >= 1)
#define EXPECT_GL_VERSION_2_1
#endif

// Before including gl.h, need to include windows.h
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

// This prevents glext.h from getting included by gl.h That way, we can
// provide our own, better version.
#define __glext_h_
#define GL_GLEXT_VERSION 0

#ifdef IS_OSX
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif

#undef __glext_h_
#undef GL_GLEXT_VERSION
#include "panda_glext.h"

#include "glstuff_src.h"

#endif  // GLGSG_H
