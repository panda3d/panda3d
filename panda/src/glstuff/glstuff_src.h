// Filename: glstuff_src.h
// Created by:  drose (09Feb04)
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

// This header file includes all of the gl-related header files in
// this directory.  To include a CLP(GraphicsStateGuardian)-like thing of
// some kind (e.g., "true" GL calls, or Mesa-prefixed GL calls, or
// some such), define the following symbols and #include this header
// file.

// #define GLP(name): returns name prefixed by the gl prefix, e.g. gl##name
// #define CLP(name): returns name prefixed by the class prefix, e.g. GL##name
// #define CLASSPREFIX_QUOTED: the quoted prefix of CLP, e.g. "GL"
// #define CONFIGOBJ: a Configrc object, e.g. config_glgsg
// #define GLCAT: a Notify category, e.g. glgsg_cat
// #define EXPCL_GL, EXPTP_GL: according to the DLL currently being compiled.

// Also, be sure you include the appropriate GL.h header file to get
// all the GL symbols declared.

// This file is not protected from multiple inclusion; it may need to
// be included multiple times.


#include "glmisc_src.h"
#include "glGeomNodeContext_src.h"
#include "glTextureContext_src.h"
#include "glSavedFrameBuffer_src.h"
#include "glGraphicsStateGuardian_src.h"

