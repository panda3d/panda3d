// Filename: glgsg.h
// Created by:  drose (09Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GLGSG_H
#define GLGSG_H

// This is the actual header file to include if you want to pick up
// any or all of the header files in this directory as compiled to use
// the "true" GL library.

#include "pandabase.h"
#include "config_glgsg.h"

#define GLP(name) gl##name
#define GLUP(name) glu##name
#define CLP(name) GL##name
#define GLPREFIX_QUOTED "gl"
#define CLASSPREFIX_QUOTED "GL"
#define GLSYSTEM_NAME "OpenGL"
#define CONFIGOBJ config_glgsg
#define GLCAT glgsg_cat
#define EXPCL_GL EXPCL_PANDAGL
#define EXPTP_GL EXPTP_PANDAGL

// Before including gl.h, need to include windows.h
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

// This prevents glext.h from getting included by gl.h
// That way, we can provide our own, better version.
#define __glext_h_
#define GL_GLEXT_VERSION 0

#ifdef IS_OSX
  #include <OpenGL/gl.h>
  #ifdef HAVE_GLU
  #include <OpenGL/glu.h>
  #endif
#else
  #include <GL/gl.h>
  #ifdef HAVE_GLU
  #include <GL/glu.h>
  #endif
#endif

#undef GL_GLEXT_VERSION
#include "panda_glext.h"

#include "glstuff_src.h"

#endif  // GLGSG_H
