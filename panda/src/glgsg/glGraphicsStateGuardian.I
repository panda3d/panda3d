// Filename: glGraphicsStateGuardian.I
// Created by:  drose (02Feb99)
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

#include "config_glgsg.h"

#include <graphicsWindow.h>

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::LightInfo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GLGraphicsStateGuardian::LightInfo::
LightInfo() {
  _light = (Light *)NULL;
  _enabled = false;
  _next_enabled = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::activate
//       Access: Public
//  Description: Sets this context to be the active context for future
//               GL commands.
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
activate() {
  // This operation seems to be incredibly expensive on some
  // platforms--particularly for the NVidia drivers under Linux.  It
  // doesn't seem to check if we already had the context current
  // before we switch contexts.  For now, we'll limit our use of this
  // function.

  _win->make_current();
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glClearColor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glClearColor(GLclampf red, GLclampf green, GLclampf blue,
                  GLclampf alpha) {
  if (red != _clear_color_red ||
      green != _clear_color_green ||
      blue != _clear_color_blue ||
      alpha != _clear_color_alpha) {
    glClearColor(red, green, blue, alpha);
    _clear_color_red = red;
    _clear_color_green = green;
    _clear_color_blue = blue;
    _clear_color_alpha = alpha;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glClearDepth
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glClearDepth(GLclampd depth) {
  if (depth != _clear_depth) {
#ifdef GSG_VERBOSE
    glgsg_cat.debug()
      << "glClearDepth(" << (double)depth << ")" << endl;
#endif
    glClearDepth(depth);
    _clear_depth = depth;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glClearStencil
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glClearStencil(GLint s) {
  if (s != _clear_stencil) {
    glClearStencil(s);
    _clear_stencil = s;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glClearAccum
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glClearAccum(GLclampf red, GLclampf green, GLclampf blue,
                  GLclampf alpha) {
  if (red != _clear_accum_red ||
      green != _clear_accum_green ||
      blue != _clear_accum_blue ||
      alpha != _clear_accum_alpha) {
    glClearAccum(red, green, blue, alpha);
    _clear_accum_red = red;
    _clear_accum_green = green;
    _clear_accum_blue = blue;
    _clear_accum_alpha = alpha;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glDrawBuffer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glDrawBuffer(GLenum mode) {
  if (mode != _draw_buffer_mode) {
#ifdef GSG_VERBOSE
    glgsg_cat.debug() << "glDrawBuffer(";
    switch (mode) {
    case GL_FRONT:
      glgsg_cat.debug(false) << "GL_FRONT)";
      break;
    case GL_BACK:
      glgsg_cat.debug(false) << "GL_BACK)";
      break;
    case GL_RIGHT:
      glgsg_cat.debug(false) << "GL_RIGHT)";
      break;
    case GL_LEFT:
      glgsg_cat.debug(false) << "GL_LEFT)";
      break;
    case GL_FRONT_RIGHT:
      glgsg_cat.debug(false) << "GL_FRONT_RIGHT)";
      break;
    case GL_FRONT_LEFT:
      glgsg_cat.debug(false) << "GL_FRONT_LEFT)";
      break;
    case GL_BACK_RIGHT:
      glgsg_cat.debug(false) << "GL_BACK_RIGHT)";
      break;
    case GL_BACK_LEFT:
      glgsg_cat.debug(false) << "GL_BACK_LEFT)";
      break;
    case GL_FRONT_AND_BACK:
      glgsg_cat.debug(false) << "GL_FRONT_AND_BACK)";
      break;
    }
    glgsg_cat.debug(false) << endl;
#endif
    glDrawBuffer(mode);
    _draw_buffer_mode = mode;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glReadBuffer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glReadBuffer(GLenum mode) {
  if (mode != _read_buffer_mode) {
    glReadBuffer(mode);
    _read_buffer_mode = mode;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glShadeModel
//       Access:
//  Description: Set the shading model to be either GL_FLAT or GL_SMOOTH
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glShadeModel(GLenum mode) {
  if (_shade_model_mode != mode) {
    glShadeModel(mode);
    _shade_model_mode = mode;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glScissor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if ( _scissor_x != x || _scissor_y != y ||
        _scissor_width != width || _scissor_height != height )
    {
        _scissor_x = x; _scissor_y = y;
        _scissor_width = width; _scissor_height = height;
        glScissor( x, y, width, height );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glViewport
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if ( _viewport_x != x || _viewport_y != y ||
        _viewport_width != width || _viewport_height != height )
    {
        _viewport_x = x; _viewport_y = y;
        _viewport_width = width; _viewport_height = height;
        glViewport( x, y, width, height );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glLightModelAmbient
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glLightModelAmbient( const Colorf& color )
{
  if (_lmodel_ambient != color) {
    _lmodel_ambient = color;
#ifdef GSG_VERBOSE
    glgsg_cat.debug()
      << "glLightModel(GL_LIGHT_MODEL_AMBIENT, " << color << ")" << endl;
#endif
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.get_data());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glLightModelLocal
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glLightModelLocal(GLboolean local)
{
    if ( _lmodel_local != local )
    {
        _lmodel_local = local;
        glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, local );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glLightModelLocal
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glLightModelTwoSide(GLboolean twoside)
{
  if (_lmodel_twoside != twoside) {
    _lmodel_twoside = twoside;
#ifdef GSG_VERBOSE
    glgsg_cat.debug()
      << "glLightModel(GL_LIGHT_MODEL_TWO_SIDE, " << (int)twoside << ")" << endl;
#endif
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, twoside);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glStencilFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glStencilFunc(GLenum func) {
  if (_stencil_func != func) {
    _stencil_func = func;
#ifdef GSG_VERBOSE
    glgsg_cat.debug() << "glStencilFunc(";
    switch (func) {
    case GL_NEVER:
      glgsg_cat.debug(false) << "GL_NEVER, ";
      break;
    case GL_LESS:
      glgsg_cat.debug(false) << "GL_LESS, ";
      break;
    case GL_EQUAL:
      glgsg_cat.debug(false) << "GL_EQUAL, ";
      break;
    case GL_LEQUAL:
      glgsg_cat.debug(false) << "GL_LEQUAL, ";
      break;
    case GL_GREATER:
      glgsg_cat.debug(false) << "GL_GREATER, ";
      break;
    case GL_NOTEQUAL:
      glgsg_cat.debug(false) << "GL_NOTEQUAL, ";
      break;
    case GL_GEQUAL:
      glgsg_cat.debug(false) << "GL_GEQUAL, ";
      break;
    case GL_ALWAYS:
      glgsg_cat.debug(false) << "GL_ALWAYS, ";
      break;
    default:
      glgsg_cat.debug(false) << "unknown, ";
      break;
    }
    glgsg_cat.debug(false) << "1, 1)" << endl;
#endif
    glStencilFunc(func, 1, 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glStencilOp
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glStencilOp(GLenum op) {
  if (_stencil_op != op) {
    _stencil_op = op;
#ifdef GSG_VERBOSE
    glgsg_cat.debug() << "glStencilOp(GL_KEEP, GL_KEEP, ";
    switch (op) {
    case GL_KEEP:
      glgsg_cat.debug(false) << "GL_KEEP)";
      break;
    case GL_ZERO:
      glgsg_cat.debug(false) << "GL_ZERO)";
      break;
    case GL_REPLACE:
      glgsg_cat.debug(false) << "GL_REPLACE)";
      break;
    case GL_INCR:
      glgsg_cat.debug(false) << "GL_INCR)";
      break;
    case GL_DECR:
      glgsg_cat.debug(false) << "GL_DECR)";
      break;
    case GL_INVERT:
      glgsg_cat.debug(false) << "GL_INVERT)";
      break;
    default:
      glgsg_cat.debug(false) << "unknown)";
      break;
    }
    glgsg_cat.debug(false) << endl;
#endif
    glStencilOp(GL_KEEP, GL_KEEP, op);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glLineWidth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glLineWidth(GLfloat width) {
  if (_line_width != width) {
    _line_width = width;
#ifdef GSG_VERBOSE
  glgsg_cat.debug()
    << "glLineWidth(" << width << ")" << endl;
#endif
    glLineWidth(width);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glPointSize
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glPointSize(GLfloat size) {
  if (_point_size != size) {
    _point_size = size;
#ifdef GSG_VERBOSE
  glgsg_cat.debug()
    << "glPointSize(" << size << ")" << endl;
#endif
    glPointSize(size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glBlendFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glBlendFunc(GLenum sfunc, GLenum dfunc) {
  if (_blend_source_func != sfunc || _blend_dest_func != dfunc) {
    _blend_source_func = sfunc;
    _blend_dest_func = dfunc;
#ifdef GSG_VERBOSE
    glgsg_cat.debug() << "glBlendFunc(";
    switch (sfunc) {
    case GL_ZERO:
      glgsg_cat.debug(false) << "GL_ZERO, ";
      break;
    case GL_ONE:
      glgsg_cat.debug(false) << "GL_ONE, ";
      break;
    case GL_DST_COLOR:
      glgsg_cat.debug(false) << "GL_DST_COLOR, ";
      break;
    case GL_ONE_MINUS_DST_COLOR:
      glgsg_cat.debug(false) << "GL_ONE_MINUS_DST_COLOR, ";
      break;
    case GL_SRC_ALPHA:
      glgsg_cat.debug(false) << "GL_SRC_ALPHA, ";
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      glgsg_cat.debug(false) << "GL_ONE_MINUS_SRC_ALPHA, ";
      break;
    case GL_DST_ALPHA:
      glgsg_cat.debug(false) << "GL_DST_ALPHA, ";
      break;
    case GL_ONE_MINUS_DST_ALPHA:
      glgsg_cat.debug(false) << "GL_ONE_MINUS_DST_ALPHA, ";
      break;
    case GL_SRC_ALPHA_SATURATE:

      glgsg_cat.debug(false) << "GL_SRC_ALPHA_SATURATE, ";
      break;
    default:
      glgsg_cat.debug(false) << "unknown, ";
      break;
    }
    switch (dfunc) {
    case GL_ZERO:
      glgsg_cat.debug(false) << "GL_ZERO)";
      break;
    case GL_ONE:
      glgsg_cat.debug(false) << "GL_ONE)";
      break;
    case GL_SRC_COLOR:
      glgsg_cat.debug(false) << "GL_SRC_COLOR)";
      break;
    case GL_ONE_MINUS_SRC_COLOR:
      glgsg_cat.debug(false) << "GL_ONE_MINUS_SRC_COLOR)";
      break;
    case GL_SRC_ALPHA:
      glgsg_cat.debug(false) << "GL_SRC_ALPHA)";
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      glgsg_cat.debug(false) << "GL_ONE_MINUS_SRC_ALPHA)";
      break;
    case GL_DST_ALPHA:
      glgsg_cat.debug(false) << "GL_DST_ALPHA)";
      break;
    case GL_ONE_MINUS_DST_ALPHA:
      glgsg_cat.debug(false) << "GL_ONE_MINUS_DST_ALPHA)";
      break;
    default:
      glgsg_cat.debug(false) << "unknown)";
      break;
    }
    glgsg_cat.debug(false) << endl;
#endif
    glBlendFunc(sfunc, dfunc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glDepthMask
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glDepthMask(GLboolean mask) {
  if (_depth_mask != mask) {
    _depth_mask = mask;
#ifdef GSG_VERBOSE
    glgsg_cat.debug()
      << "glDepthMask(" << (int)mask << ")" << endl;
#endif
    glDepthMask(mask);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glFogMode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glFogMode(GLint mode) {
  if (_fog_mode != mode) {
    _fog_mode = mode;
#ifdef GSG_VERBOSE
    glgsg_cat.debug() << "glFog(GL_FOG_MODE, ";
    switch(mode) {
    case GL_LINEAR:
      glgsg_cat.debug(false) << "GL_LINEAR)" << endl;
      break;
    case GL_EXP:
      glgsg_cat.debug(false) << "GL_EXP)" << endl;
      break;
    case GL_EXP2:
      glgsg_cat.debug(false) << "GL_EXP2)" << endl;
      break;
#ifdef GL_FOG_FUNC_SGIS
    case GL_FOG_FUNC_SGIS:
      glgsg_cat.debug(false) << "GL_FOG_FUNC_SGIS)" << endl;
      break;
#endif
    default:
      glgsg_cat.debug(false) << "unknown)" << endl;
      break;
    }
#endif
    glFogi(GL_FOG_MODE, mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glFogStart
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glFogStart(GLfloat start) {
  if (_fog_start != start) {
    _fog_start = start;
#ifdef GSG_VERBOSE
  glgsg_cat.debug()
    << "glFog(GL_FOG_START, " << start << ")" << endl;
#endif
    glFogf(GL_FOG_START, start);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glFogEnd
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glFogEnd(GLfloat end) {
  if (_fog_end != end) {
    _fog_end = end;
#ifdef GSG_VERBOSE
  glgsg_cat.debug()
    << "glFog(GL_FOG_END, " << end << ")" << endl;
#endif
    glFogf(GL_FOG_END, end);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glFogDensity
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glFogDensity(GLfloat density) {
  if (_fog_density != density) {
    _fog_density = density;
#ifdef GSG_VERBOSE
  glgsg_cat.debug()
    << "glFog(GL_FOG_DENSITY, " << density << ")" << endl;
#endif
    glFogf(GL_FOG_DENSITY, density);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glFogColor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glFogColor(const Colorf &color) {
  if (_fog_color != color) {
    _fog_color = color;
#ifdef GSG_VERBOSE
  glgsg_cat.debug()
    << "glFog(GL_FOG_COLOR, " << color << ")" << endl;
#endif
    glFogfv(GL_FOG_COLOR, color.get_data());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glAlphaFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glAlphaFunc(GLenum func, GLclampf ref) {
  if (_alpha_func != func || _alpha_func_ref != ref) {
    _alpha_func = func;
    _alpha_func_ref = ref;
#ifdef GSG_VERBOSE
    glgsg_cat.debug() << "glAlphaFunc(";
    switch (func) {
    case GL_NEVER:
      glgsg_cat.debug(false) << "GL_NEVER, ";
      break;
    case GL_LESS:
      glgsg_cat.debug(false) << "GL_LESS, ";
      break;
    case GL_EQUAL:
      glgsg_cat.debug(false) << "GL_EQUAL, ";
      break;
    case GL_LEQUAL:
      glgsg_cat.debug(false) << "GL_LEQUAL, ";
      break;
    case GL_GREATER:
      glgsg_cat.debug(false) << "GL_GREATER, ";
      break;
    case GL_NOTEQUAL:
      glgsg_cat.debug(false) << "GL_NOTEQUAL, ";
      break;
    case GL_GEQUAL:
      glgsg_cat.debug(false) << "GL_GEQUAL, ";
      break;
    case GL_ALWAYS:
      glgsg_cat.debug(false) << "GL_ALWAYS, ";
      break;
    }
    glgsg_cat.debug() << ref << ")" << endl;
#endif
    glAlphaFunc(func, ref);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glPolygonMode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
call_glPolygonMode(GLenum mode) {
  if (_polygon_mode != mode) {
    _polygon_mode = mode;
#ifdef GSG_VERBOSE
    glgsg_cat.debug() << "glPolygonMode(GL_BACK_AND_FRONT, ";
    switch (mode) {
    case GL_POINT:
      glgsg_cat.debug(false) << "GL_POINT)" << endl;
      break;
    case GL_LINE:
      glgsg_cat.debug(false) << "GL_LINE)" << endl;
      break;
    case GL_FILL:
      glgsg_cat.debug(false) << "GL_FILL)" << endl;
      break;
    }
#endif
    glPolygonMode(GL_FRONT_AND_BACK, mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::set_pack_alignment
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
set_pack_alignment(GLint alignment) {
  if (_pack_alignment != alignment) {
    glPixelStorei(GL_PACK_ALIGNMENT, alignment);
    _pack_alignment = alignment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::set_unpack_alignment
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
set_unpack_alignment(GLint alignment) {
  if (_unpack_alignment != alignment) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    _unpack_alignment = alignment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_multisample
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_multisample(bool val) {
  if (_multisample_enabled != val) {
    _multisample_enabled = val;
    if (val) {
#ifdef GL_MULTISAMPLE_SGIS
      glEnable(GL_MULTISAMPLE_SGIS);
#endif
      glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    } else {
#ifdef GL_MULTISAMPLE_SGIS
      glDisable(GL_MULTISAMPLE_SGIS);
#endif
      glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_line_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_line_smooth(bool val) {
  if (_line_smooth_enabled != val) {
    _line_smooth_enabled = val;
    if (val) {
      glEnable(GL_LINE_SMOOTH);
      glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    } else {
      glDisable(GL_LINE_SMOOTH);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_point_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_point_smooth(bool val) {
  if (_point_smooth_enabled != val) {
    _point_smooth_enabled = val;
    if (val) {
      glEnable(GL_POINT_SMOOTH);
      glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    } else {
      glDisable(GL_POINT_SMOOTH);
      glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_lighting
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_lighting(bool val) {
  if (_lighting_enabled != val) {
    _lighting_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_LIGHTING)" << endl;
#endif
      glEnable(GL_LIGHTING);
      _lighting_enabled_this_frame = true;
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_LIGHTING)" << endl;
#endif
      glDisable(GL_LIGHTING);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_dither
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_dither(bool val) {
  if (_dither_enabled != val) {
    _dither_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_DITHER)" << endl;
#endif
      glEnable(GL_DITHER);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_DITHER)" << endl;
#endif
      glDisable(GL_DITHER);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_stencil_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_stencil_test(bool val) {
  if (_stencil_test_enabled != val) {
    _stencil_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_STENCIL_TEST)" << endl;
#endif
      glEnable(GL_STENCIL_TEST);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_STENCIL_TEST)" << endl;
#endif
      glDisable(GL_STENCIL_TEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_light_id
//       Access: Public
//  Description: Convert index to gl light id
////////////////////////////////////////////////////////////////////
INLINE GLenum GLGraphicsStateGuardian::get_light_id(int index) const
{
    switch( index )
    {
        case 0: return GL_LIGHT0;
        case 1: return GL_LIGHT1;
        case 2: return GL_LIGHT2;
        case 3: return GL_LIGHT3;
        case 4: return GL_LIGHT4;
        case 5: return GL_LIGHT5;
        case 6: return GL_LIGHT6;
        case 7: return GL_LIGHT7;
        default:
            glgsg_cat.error()
              << "get_light_id() - we don't currently support ids "
              << "> 8" << endl;
            break;
    }
    return GL_LIGHT0;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_clip_plane_id
//       Access: Public
//  Description: Convert index to gl clip plane id
////////////////////////////////////////////////////////////////////
INLINE GLenum GLGraphicsStateGuardian::
get_clip_plane_id(int index) const {
  switch(index) {
    case 0: return GL_CLIP_PLANE0;
    case 1: return GL_CLIP_PLANE1;
    case 2: return GL_CLIP_PLANE2;
    case 3: return GL_CLIP_PLANE3;
    case 4: return GL_CLIP_PLANE4;
    case 5: return GL_CLIP_PLANE5;
    default:
      glgsg_cat.error()
        << "get_clip_plane_id() - we don't currently support ids "
        << "> 5" << endl;
      break;
  }
  return GL_CLIP_PLANE0;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_light
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::enable_light(int light, bool val)
{
    if ( _light_info[light]._enabled != val )
    {
        _light_info[light]._enabled = val;
        if ( val )
        {
#ifdef GSG_VERBOSE
          glgsg_cat.debug()
            << "glEnable(GL_LIGHT" << light << ")" << endl;
#endif
            glEnable( get_light_id( light ) );
        }
        else
        {
#ifdef GSG_VERBOSE
          glgsg_cat.debug()
            << "glDisable(GL_LIGHT" << light << ")" << endl;
#endif
            glDisable( get_light_id( light ) );
        }
    }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_texturing
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_texturing(bool val) {
  if (_texturing_enabled != val) {
    _texturing_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_TEXTURE_2D)" << endl;
#endif
      glEnable(GL_TEXTURE_2D);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_TEXTURE_2D)" << endl;
#endif
      glDisable(GL_TEXTURE_2D);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_scissor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_scissor(bool val)
{
    if ( _scissor_enabled != val ) {
        _scissor_enabled = val;
        if ( val )
            glEnable( GL_SCISSOR_TEST );
        else
            glDisable( GL_SCISSOR_TEST );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_clip_plane
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_clip_plane(int clip_plane, bool val) {
  if (_clip_plane_enabled[clip_plane] != val) {
    _clip_plane_enabled[clip_plane] = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_CLIP_PLANE_" << clip_plane << ")" << endl;
#endif
      glEnable(get_clip_plane_id(clip_plane));
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_CLIP_PLANE_" << clip_plane << ")" << endl;
#endif
      glDisable(get_clip_plane_id(clip_plane));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_multisample_alpha_one
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_multisample_alpha_one(bool val) {
  if (_multisample_alpha_one_enabled != val) {
    _multisample_alpha_one_enabled = val;
#ifdef GL_SAMPLE_ALPHA_TO_ONE_SGIS
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_SAMPLE_ALPHA_TO_ONE_SGIS)" << endl;
#endif
      glEnable(GL_SAMPLE_ALPHA_TO_ONE_SGIS);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_SAMPLE_ALPHA_TO_ONE_SGIS)" << endl;
#endif
      glDisable(GL_SAMPLE_ALPHA_TO_ONE_SGIS);
    }
#endif  // GL_SAMPLE_ALPHA_TO_ONE_SGIS
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_multisample_alpha_mask
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_multisample_alpha_mask(bool val) {
  if (_multisample_alpha_mask_enabled != val) {
    _multisample_alpha_mask_enabled = val;
#ifdef GL_SAMPLE_ALPHA_TO_MASK_SGIS
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_SAMPLE_ALPHA_TO_MASK_SGIS)" << endl;
#endif
      glEnable(GL_SAMPLE_ALPHA_TO_MASK_SGIS);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_SAMPLE_ALPHA_TO_MASK_SGIS)" << endl;
#endif
      glDisable(GL_SAMPLE_ALPHA_TO_MASK_SGIS);
    }
#endif  // GL_SAMPLE_ALPHA_TO_MASK_SGIS
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_BLEND)" << endl;
#endif
      glEnable(GL_BLEND);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_BLEND)" << endl;
#endif
      glDisable(GL_BLEND);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_depth_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_depth_test(bool val) {
  if (_depth_test_enabled != val) {
    _depth_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_DEPTH_TEST)" << endl;
#endif
      glEnable(GL_DEPTH_TEST);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_DEPTH_TEST)" << endl;
#endif
      glDisable(GL_DEPTH_TEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_fog
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_fog(bool val) {
  if (_fog_enabled != val) {
    _fog_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_FOG)" << endl;
#endif
      glEnable(GL_FOG);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_FOG)" << endl;
#endif
      glDisable(GL_FOG);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_alpha_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_alpha_test(bool val) {
  if (_alpha_test_enabled != val) {
    _alpha_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_ALPHA_TEST)" << endl;
#endif
      glEnable(GL_ALPHA_TEST);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_ALPHA_TEST)" << endl;
#endif
      glDisable(GL_ALPHA_TEST);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_polygon_offset
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
enable_polygon_offset(bool val) {
  if (_polygon_offset_enabled != val) {
    _polygon_offset_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glEnable(GL_POLYGON_OFFSET_*)" << endl;
#endif
      glEnable(GL_POLYGON_OFFSET_FILL);
    } else {
#ifdef GSG_VERBOSE
      glgsg_cat.debug()
        << "glDisable(GL_POLYGON_OFFSET_*)" << endl;
#endif
      glDisable(GL_POLYGON_OFFSET_FILL);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_current_color_mat
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &GLGraphicsStateGuardian::
get_current_color_mat() const {
  return _current_color_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_current_alpha_offset
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const float &GLGraphicsStateGuardian::
get_current_alpha_offset() const {
  return _current_alpha_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_current_alpha_scale
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const float &GLGraphicsStateGuardian::
get_current_alpha_scale() const {
  return _current_alpha_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::report_errors
//       Access: Protected
//  Description: Checks for any outstanding error codes and outputs
//               them, if found.  If NDEBUG is defined, this function
//               does nothing.
////////////////////////////////////////////////////////////////////
INLINE void GLGraphicsStateGuardian::
report_errors() const {
#ifndef NDEBUG
  GLenum error_code = glGetError();
  if (error_code != GL_NO_ERROR) {
    report_errors_loop(error_code);
  }
#endif
}
