// Filename: glmisc_src.h
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

#include "pandabase.h"
#include "configVariableBool.h"

//#define GSG_VERBOSE 1

extern ConfigVariableBool CLP(cheap_textures);
extern ConfigVariableBool CLP(ignore_clamp);
extern ConfigVariableBool CLP(ignore_filters);
extern ConfigVariableBool CLP(ignore_mipmaps);
extern ConfigVariableBool CLP(force_mipmaps);
extern ConfigVariableBool CLP(show_mipmaps);
extern ConfigVariableBool CLP(save_mipmaps);
extern ConfigVariableBool CLP(color_mask);
extern ConfigVariableBool CLP(compile_and_execute);

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

// We have to define this whether NDEBUG is defined or not, to prevent
// syntax errors.  Not to worry; the function itself does nothing in
// NDEBUG mode.
#define report_my_gl_errors() \
  report_my_errors(__LINE__, __FILE__)
