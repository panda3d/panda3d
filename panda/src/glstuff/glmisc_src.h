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
