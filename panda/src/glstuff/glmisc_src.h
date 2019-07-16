/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glmisc_src.h
 * @author drose
 * @date 2004-02-09
 */

#include "pandabase.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableEnum.h"
#include "geomEnums.h"
#include "coordinateSystem.h"

// Define some macros to transparently map to the double or float versions of
// the OpenGL function names.
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

// #define GSG_VERBOSE 1

extern EXPCL_GL ConfigVariableInt gl_version;
extern EXPCL_GL ConfigVariableBool gl_forward_compatible;
extern EXPCL_GL ConfigVariableBool gl_support_fbo;
extern ConfigVariableBool gl_cheap_textures;
extern ConfigVariableBool gl_ignore_clamp;
extern ConfigVariableBool gl_support_clamp_to_border;
extern ConfigVariableBool gl_support_texture_lod;
extern ConfigVariableBool gl_ignore_filters;
extern ConfigVariableBool gl_ignore_mipmaps;
extern ConfigVariableBool gl_force_mipmaps;
extern ConfigVariableBool gl_show_texture_usage;
extern ConfigVariableInt gl_show_texture_usage_max_size;
extern ConfigVariableBool gl_color_mask;
extern ConfigVariableBool gl_support_occlusion_query;
extern ConfigVariableBool gl_compile_and_execute;
extern ConfigVariableBool gl_interleaved_arrays;
extern ConfigVariableBool gl_parallel_arrays;
extern ConfigVariableInt gl_max_errors;
extern ConfigVariableEnum<GeomEnums::UsageHint> gl_min_buffer_usage_hint;
extern EXPCL_GL ConfigVariableBool gl_debug;
extern ConfigVariableBool gl_debug_synchronous;
extern ConfigVariableEnum<NotifySeverity> gl_debug_abort_level;
extern ConfigVariableBool gl_debug_object_labels;
extern ConfigVariableBool gl_debug_buffers;
extern ConfigVariableBool gl_finish;
extern ConfigVariableBool gl_force_depth_stencil;
extern ConfigVariableBool gl_force_fbo_color;
extern ConfigVariableBool gl_check_errors;
extern ConfigVariableBool gl_force_flush;
extern ConfigVariableBool gl_separate_specular_color;
extern ConfigVariableBool gl_cube_map_seamless;
extern ConfigVariableBool gl_dump_compiled_shaders;
extern ConfigVariableBool gl_validate_shaders;
extern ConfigVariableBool gl_immutable_texture_storage;
extern ConfigVariableBool gl_use_bindless_texture;
extern ConfigVariableBool gl_enable_memory_barriers;
extern ConfigVariableBool gl_vertex_array_objects;
extern ConfigVariableBool gl_fixed_vertex_attrib_locations;
extern ConfigVariableBool gl_support_primitive_restart_index;
extern ConfigVariableBool gl_support_sampler_objects;
extern ConfigVariableBool gl_support_shadow_filter;
extern ConfigVariableBool gl_force_image_bindings_writeonly;
extern ConfigVariableEnum<CoordinateSystem> gl_coordinate_system;

extern EXPCL_GL void CLP(init_classes)();


#if !defined(WIN32) && defined(GSG_VERBOSE)
std::ostream &output_gl_enum(std::ostream &out, GLenum v);
INLINE std::ostream &operator << (std::ostream &out, GLenum v) {
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
