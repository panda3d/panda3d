// Filename: mesagsg.h
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

#ifndef MESAGSG_H
#define MESAGSG_H

// This is the actual header file to include if you want to pick up
// any or all of the header files in this directory as compiled to use
// the Mesa library.

#include "pandabase.h"
#include "config_mesadisplay.h"

#ifdef MESA_MGL
  #define GLP(name) mgl##name
  #define GLUP(name) mglu##name
  #define USE_MGL_NAMESPACE 1
#else
  #define GLP(name) gl##name
  #define GLUP(name) glu##name
#endif
#define CLP(name) Mesa##name
#define CLASSPREFIX_QUOTED "Mesa"
#define CONFIGOBJ config_mesadisplay
#define GLCAT mesadisplay_cat
#define EXPCL_GL EXPCL_PANDAMESA
#define EXPTP_GL EXPTP_PANDAMESA

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/osmesa.h>

#include "glstuff_src.h"

#endif  // MESAGSG_H
