// Filename: glmisc_src.h
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

#include "pandabase.h"

//#define GSG_VERBOSE 1

extern bool CLP(cheap_textures);
extern bool CLP(always_decal_textures);
extern bool CLP(ignore_clamp);
extern bool CLP(ignore_filters);
extern bool CLP(ignore_mipmaps);
extern bool CLP(force_mipmaps);
extern bool CLP(show_mipmaps);
extern bool CLP(save_mipmaps);
extern bool CLP(auto_normalize_lighting);
extern bool CLP(depth_offset_decals);
extern bool CLP(color_mask);

extern EXPCL_GL void CLP(init_classes)();


#if !defined(WIN32) && defined(GSG_VERBOSE)
ostream &output_gl_enum(ostream &out, GLenum v);
INLINE ostream &operator << (ostream &out, GLenum v) {
  return output_gl_enum(out, v);
}
#endif


#ifdef DO_PSTATS
#define DO_PSTATS_STUFF(XX) XX;
#else
#define DO_PSTATS_STUFF(XX)
#endif

#define ISPOW2(X) (((X) & ((X)-1))==0)

#ifndef NDEBUG
#define report_gl_errors() \
  CLP(GraphicsStateGuardian)::report_errors(__LINE__, __FILE__)
#else
#define report_gl_errors()
#endif
