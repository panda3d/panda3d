// Filename: glstuff_src.h
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

// This header file includes all of the gl-related header files in
// this directory.  To include a CLP(GraphicsStateGuardian)-like thing of
// some kind (e.g., "true" GL calls, or Mesa-prefixed GL calls, or
// some such), define the following symbols and #include this header
// file.

// #define CLP(name): returns name prefixed by the class prefix, e.g. GL##name
// #define CLASSPREFIX_QUOTED: the quoted prefix of CLP, e.g. "GL"
// #define CONFIGOBJ: a Configrc object, e.g. config_glgsg
// #define GLCAT: a Notify category, e.g. glgsg_cat
// #define EXPCL_GL, EXPTP_GL: according to the DLL currently being compiled.

// Also, be sure you include the appropriate gl.h header
// file to get all the standard GL symbols declared.  GL extensions
// are included here via glext.h.

// This file is not protected from multiple inclusion; it may need to
// be included multiple times.

#include "glmisc_src.h"
#include "glTextureContext_src.h"
#include "glSamplerContext_src.h"
#include "glVertexBufferContext_src.h"
#include "glIndexBufferContext_src.h"
#include "glOcclusionQueryContext_src.h"
#include "glTimerQueryContext_src.h"
#include "glLatencyQueryContext_src.h"
#include "glGeomContext_src.h"
#include "glGeomMunger_src.h"
#include "glShaderContext_src.h"
#include "glCgShaderContext_src.h"
#include "glImmediateModeSender_src.h"
#include "glGraphicsBuffer_src.h"
#include "glGraphicsStateGuardian_src.h"
