// Filename: gles2gsg.h
// Created by:  pro-rsoft (14Jun09)
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

#ifndef GLES2GSG_H
#define GLES2GSG_H

// This header file compiles a GSG for the limited subset of OpenGL
// that is OpenGL ES 2.

#include "pandabase.h"
#include "config_gles2gsg.h"

#define GLP(name) gl##name

#ifndef STDFLOAT_DOUBLE
#define GLPf(name) gl ## name ## f
#define GLPfv(name) gl ## name ## fv
#else  // STDFLOAT_DOUBLE
#define GLPf(name) gl ## name ## d
#define GLPfv(name) gl ## name ## dv
#endif  // STDFLOAT_DOUBLE

#define CLP(name) GLES2##name
#define GLPREFIX_QUOTED "gl"
#define CLASSPREFIX_QUOTED "GLES2"
#define GLSYSTEM_NAME "OpenGL ES 2"
#define CONFIGOBJ config_gles2gsg
#define GLCAT gles2gsg_cat
#define EXPCL_GL EXPCL_PANDAGLES2
#define EXPTP_GL EXPTP_PANDAGLES2
#ifdef OPENGLES_1
  #error OPENGLES_1 should not be defined!
#endif
#ifndef OPENGLES
  #define OPENGLES
#endif
#ifndef OPENGLES_2
  #define OPENGLES_2
#endif

#ifdef IS_OSX
  #include <OpenGLES/ES2/gl.h>
//  #include <OpenGLES/ES2/glext.h>
#else
  #include <GLES2/gl2.h>
//  #include <GLES2/gl2ext.h>
#endif

#include "panda_esgl2ext.h" 

// This helps to keep the source clean of hundreds of #ifdefs.
typedef char GLchar;
#define GL_RENDERBUFFER_EXT GL_RENDERBUFFER
#define GL_RENDERBUFFER_RED_SIZE_EXT GL_RENDERBUFFER_RED_SIZE
#define GL_RENDERBUFFER_GREEN_SIZE_EXT GL_RENDERBUFFER_GREEN_SIZE
#define GL_RENDERBUFFER_BLUE_SIZE_EXT GL_RENDERBUFFER_BLUE_SIZE
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT GL_RENDERBUFFER_ALPHA_SIZE
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT GL_RENDERBUFFER_DEPTH_SIZE
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT GL_RENDERBUFFER_STENCIL_SIZE
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER_EXT GL_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER_EXT GL_FRAMEBUFFER
#define GL_FRAMEBUFFER_COMPLETE_EXT GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT GL_FRAMEBUFFER_UNSUPPORTED
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT GL_FRAMEBUFFER_INCOMPLETE_FORMATS
#define GL_DEPTH_ATTACHMENT_EXT GL_DEPTH_ATTACHMENT
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0
#define GL_STENCIL_ATTACHMENT_EXT GL_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
#define GL_DEPTH_STENCIL_EXT GL_DEPTH_STENCIL_OES
#define GL_UNSIGNED_INT_24_8_EXT GL_UNSIGNED_INT_24_8_OES
#define GL_DEPTH24_STENCIL8_EXT GL_DEPTH24_STENCIL8_OES
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
#define GL_TEXTURE_3D GL_TEXTURE_3D_OES
#define GL_MAX_3D_TEXTURE_SIZE GL_MAX_3D_TEXTURE_SIZE_OES
#define GL_SAMPLER_3D GL_SAMPLER_3D_OES
#define GL_BGRA GL_BGRA_EXT
#define GL_RED GL_RED_EXT
#define GL_RG GL_RG_EXT
#define GL_R16F GL_R16F_EXT
#define GL_RG16F GL_RG16F_EXT
#define GL_RGB16F GL_RGB16F_EXT
#define GL_RGBA16F GL_RGBA16F_EXT

#undef SUPPORT_IMMEDIATE_MODE
#define APIENTRY
#define APIENTRYP *

#include "glstuff_src.h"
 
#endif  // GLES2GSG_H
