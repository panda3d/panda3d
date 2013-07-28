// Filename: glmisc_src.h
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

#include "pandabase.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableEnum.h"
#include "geomEnums.h"

// Define some macros to transparently map to the double or float
// versions of the OpenGL function names.
#ifndef GLf

#ifndef STDFLOAT_DOUBLE
#define GLf(name) name ## f
#define GLfv(name) name ## fv
#define GLfc(name) name ## fc
#define GLfr(name) name ## fr
#define GLf_str "f"
#else  // STDFLOAT_DOUBLE
#define GLf(name) name ## d
#define GLfv(name) name ## dv
#define GLfc(name) name ## dc
#define GLfr(name) name ## dr
#define GLf_str "d"
#endif  // STDFLOAT_DOUBLE

#endif  // GLf

//#define GSG_VERBOSE 1

extern ConfigVariableBool CLP(cheap_textures);
extern ConfigVariableBool CLP(ignore_clamp);
extern ConfigVariableBool CLP(support_clamp_to_border);
extern ConfigVariableBool CLP(ignore_filters);
extern ConfigVariableBool CLP(ignore_mipmaps);
extern ConfigVariableBool CLP(force_mipmaps);
extern ConfigVariableBool CLP(show_texture_usage);
extern ConfigVariableInt CLP(show_texture_usage_max_size);
extern ConfigVariableBool CLP(color_mask);
extern ConfigVariableBool CLP(support_occlusion_query);
extern ConfigVariableBool CLP(compile_and_execute);
extern ConfigVariableBool CLP(interleaved_arrays);
extern ConfigVariableBool CLP(parallel_arrays);
extern ConfigVariableInt CLP(max_errors);
extern ConfigVariableEnum<GeomEnums::UsageHint> CLP(min_buffer_usage_hint);
extern ConfigVariableBool CLP(debug_buffers);
extern ConfigVariableBool CLP(finish);
extern ConfigVariableBool CLP(force_depth_stencil);
extern ConfigVariableBool CLP(matrix_palette);
extern ConfigVariableBool CLP(force_no_error);
extern ConfigVariableBool CLP(force_no_flush);
extern ConfigVariableBool CLP(separate_specular_color);

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

#define report_my_gl_errors() \
  report_my_errors(__LINE__, __FILE__)

#define clear_my_gl_errors() \
  clear_my_errors(__LINE__, __FILE__)
