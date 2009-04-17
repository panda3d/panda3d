// Filename: glesgsg.h
// Created by:  drose (09Apr09)
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

#ifndef GLESGSG_H
#define GLESGSG_H

// This header file compiles a GSG for the limited subset of OpenGL
// that is OpenGL ES.

#include "pandabase.h"
#include "config_iphone.h"

#define GLP(name) gl##name
#define GLUP(name) glu##name
#define CLP(name) GLES##name
#define GLPREFIX_QUOTED "gl"
#define CLASSPREFIX_QUOTED "GLES"
#define GLSYSTEM_NAME "OpenGL ES"
#define CONFIGOBJ config_iphone
#define GLCAT iphone_cat
#define EXPCL_GL EXPCL_PANDAGL
#define EXPTP_GL EXPTP_PANDAGL
#define OPENGLES_1
#undef HAVE_GLU

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "glesext_shadow.h"

#undef SUPPORT_IMMEDIATE_MODE
#define APIENTRY
#define APIENTRYP *

#include "glstuff_src.h"

#endif  // GLESGSG_H
