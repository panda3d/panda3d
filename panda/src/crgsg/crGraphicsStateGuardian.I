// Filename: chromium.GraphicsStateGuardian.I
// Created by:  drose (02Feb99)
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

#include "config_crgsg.h"

#include "graphicsWindow.h"

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::activate
//       Access: Public
//  Description: Sets this context to be the active context for future
//               GL commands.
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
activate() {
  // This operation seems to be incredibly expensive on some
  // platforms--particularly for the NVidia drivers under Linux.  It
  // doesn't seem to check if we already had the context current
  // before we switch contexts.  For now, we'll limit our use of this
  // function.

  _win->make_current();
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glClearColor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glClearColor(GLclampf red, GLclampf green, GLclampf blue,
                  GLclampf alpha) {
  if (red != _clear_color_red ||
      green != _clear_color_green ||
      blue != _clear_color_blue ||
      alpha != _clear_color_alpha) {
    chromium.ClearColor(red, green, blue, alpha);
    _clear_color_red = red;
    _clear_color_green = green;
    _clear_color_blue = blue;
    _clear_color_alpha = alpha;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glClearDepth
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glClearDepth(GLclampd depth) {
  if (depth != _clear_depth) {
#ifdef GSG_VERBOSE
    crgsg_cat.debug()
      << "crClearDepth(" << (double)depth << ")" << endl;
#endif
    chromium.ClearDepth(depth);
    _clear_depth = depth;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glClearStencil
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glClearStencil(GLint s) {
  if (s != _clear_stencil) {
    chromium.ClearStencil(s);
    _clear_stencil = s;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glClearAccum
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glClearAccum(GLclampf red, GLclampf green, GLclampf blue,
                  GLclampf alpha) {
  if (red != _clear_accum_red ||
      green != _clear_accum_green ||
      blue != _clear_accum_blue ||
      alpha != _clear_accum_alpha) {
    chromium.ClearAccum(red, green, blue, alpha);
    _clear_accum_red = red;
    _clear_accum_green = green;
    _clear_accum_blue = blue;
    _clear_accum_alpha = alpha;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glDrawBuffer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glDrawBuffer(GLenum mode) {
  if (mode != _draw_buffer_mode) {
#ifdef GSG_VERBOSE
    crgsg_cat.debug() << "crDrawBuffer(";
    switch (mode) {
    case GL_FRONT:
      crgsg_cat.debug(false) << "GL_FRONT)";
      break;
    case GL_BACK:
      crgsg_cat.debug(false) << "GL_BACK)";
      break;
    case GL_RIGHT:
      crgsg_cat.debug(false) << "GL_RIGHT)";
      break;
    case GL_LEFT:
      crgsg_cat.debug(false) << "GL_LEFT)";
      break;
    case GL_FRONT_RIGHT:
      crgsg_cat.debug(false) << "GL_FRONT_RIGHT)";
      break;
    case GL_FRONT_LEFT:
      crgsg_cat.debug(false) << "GL_FRONT_LEFT)";
      break;
    case GL_BACK_RIGHT:
      crgsg_cat.debug(false) << "GL_BACK_RIGHT)";
      break;
    case GL_BACK_LEFT:
      crgsg_cat.debug(false) << "GL_BACK_LEFT)";
      break;
    case GL_FRONT_AND_BACK:
      crgsg_cat.debug(false) << "GL_FRONT_AND_BACK)";
      break;
    }
    crgsg_cat.debug(false) << endl;
#endif
    chromium.DrawBuffer(mode);
    _draw_buffer_mode = mode;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glReadBuffer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glReadBuffer(GLenum mode) {
  if (mode != _read_buffer_mode) {
    chromium.ReadBuffer(mode);
    _read_buffer_mode = mode;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glShadeModel
//       Access:
//  Description: Set the shading model to be either GL_FLAT or GL_SMOOTH
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glShadeModel(GLenum mode) {
  if (_shade_model_mode != mode) {
    chromium.ShadeModel(mode);
    _shade_model_mode = mode;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glScissor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if ( _scissor_x != x || _scissor_y != y ||
        _scissor_width != width || _scissor_height != height )
    {
        _scissor_x = x; _scissor_y = y;
        _scissor_width = width; _scissor_height = height;
        chromium.Scissor( x, y, width, height );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glViewport
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if ( _viewport_x != x || _viewport_y != y ||
        _viewport_width != width || _viewport_height != height )
    {
        _viewport_x = x; _viewport_y = y;
        _viewport_width = width; _viewport_height = height;
        chromium.Viewport( x, y, width, height );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glLightModelLocal
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glLightModelLocal(GLboolean local)
{
    if ( _lmodel_local != local )
    {
        _lmodel_local = local;
        chromium.LightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, local );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glLightModelLocal
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glLightModelTwoSide(GLboolean twoside)
{
  if (_lmodel_twoside != twoside) {
    _lmodel_twoside = twoside;
#ifdef GSG_VERBOSE
    crgsg_cat.debug()
      << "crLightModel(GL_LIGHT_MODEL_TWO_SIDE, " << (int)twoside << ")" << endl;
#endif
    chromium.LightModeli(GL_LIGHT_MODEL_TWO_SIDE, twoside);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glStencilFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glStencilFunc(GLenum func,GLint ref,GLuint mask) {

#ifdef GSG_VERBOSE
    crgsg_cat.debug() << "crStencilFunc(";
    switch (func) {
    case GL_NEVER:
      crgsg_cat.debug(false) << "GL_NEVER, ";
      break;
    case GL_LESS:
      crgsg_cat.debug(false) << "GL_LESS, ";
      break;
    case GL_EQUAL:
      crgsg_cat.debug(false) << "GL_EQUAL, ";
      break;
    case GL_LEQUAL:
      crgsg_cat.debug(false) << "GL_LEQUAL, ";
      break;
    case GL_GREATER:
      crgsg_cat.debug(false) << "GL_GREATER, ";
      break;
    case GL_NOTEQUAL:
      crgsg_cat.debug(false) << "GL_NOTEQUAL, ";
      break;
    case GL_GEQUAL:
      crgsg_cat.debug(false) << "GL_GEQUAL, ";
      break;
    case GL_ALWAYS:
      crgsg_cat.debug(false) << "GL_ALWAYS, ";
      break;
    default:
      crgsg_cat.debug(false) << "unknown, ";
      break;
    }
    crgsg_cat.debug(false) << ref << mask << ")\n";
#endif
    chromium.StencilFunc(func, ref, mask);
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glStencilOp
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glStencilOp(GLenum fail,GLenum zfail,GLenum zpass) {
#ifdef GSG_VERBOSE
    crgsg_cat.debug() << "crStencilOp(fail, zfail, ";
    switch (zpass) {
    case GL_KEEP:
      crgsg_cat.debug(false) << "GL_KEEP)";
      break;
    case GL_ZERO:
      crgsg_cat.debug(false) << "GL_ZERO)";
      break;
    case GL_REPLACE:
      crgsg_cat.debug(false) << "GL_REPLACE)";
      break;
    case GL_INCR:
      crgsg_cat.debug(false) << "GL_INCR)";
      break;
    case GL_DECR:
      crgsg_cat.debug(false) << "GL_DECR)";
      break;
    case GL_INVERT:
      crgsg_cat.debug(false) << "GL_INVERT)";
      break;
    default:
      crgsg_cat.debug(false) << "unknown)";
      break;
    }
    crgsg_cat.debug(false) << endl;
#endif
    chromium.StencilOp(fail,zfail,zpass);
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glLineWidth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glLineWidth(GLfloat width) {
  if (_line_width != width) {
    _line_width = width;
#ifdef GSG_VERBOSE
  crgsg_cat.debug()
    << "crLineWidth(" << width << ")" << endl;
#endif
    chromium.LineWidth(width);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glPointSize
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glPointSize(GLfloat size) {
  if (_point_size != size) {
    _point_size = size;
#ifdef GSG_VERBOSE
  crgsg_cat.debug()
    << "crPointSize(" << size << ")" << endl;
#endif
    chromium.PointSize(size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glBlendFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glBlendFunc(GLenum sfunc, GLenum dfunc) {
  if (_blend_source_func != sfunc || _blend_dest_func != dfunc) {
    _blend_source_func = sfunc;
    _blend_dest_func = dfunc;
#ifdef GSG_VERBOSE
    crgsg_cat.debug() << "crBlendFunc(";
    switch (sfunc) {
    case GL_ZERO:
      crgsg_cat.debug(false) << "GL_ZERO, ";
      break;
    case GL_ONE:
      crgsg_cat.debug(false) << "GL_ONE, ";
      break;
    case GL_DST_COLOR:
      crgsg_cat.debug(false) << "GL_DST_COLOR, ";
      break;
    case GL_ONE_MINUS_DST_COLOR:
      crgsg_cat.debug(false) << "GL_ONE_MINUS_DST_COLOR, ";
      break;
    case GL_SRC_ALPHA:
      crgsg_cat.debug(false) << "GL_SRC_ALPHA, ";
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      crgsg_cat.debug(false) << "GL_ONE_MINUS_SRC_ALPHA, ";
      break;
    case GL_DST_ALPHA:
      crgsg_cat.debug(false) << "GL_DST_ALPHA, ";
      break;
    case GL_ONE_MINUS_DST_ALPHA:
      crgsg_cat.debug(false) << "GL_ONE_MINUS_DST_ALPHA, ";
      break;
    case GL_SRC_ALPHA_SATURATE:

      crgsg_cat.debug(false) << "GL_SRC_ALPHA_SATURATE, ";
      break;
    default:
      crgsg_cat.debug(false) << "unknown, ";
      break;
    }
    switch (dfunc) {
    case GL_ZERO:
      crgsg_cat.debug(false) << "GL_ZERO)";
      break;
    case GL_ONE:
      crgsg_cat.debug(false) << "GL_ONE)";
      break;
    case GL_SRC_COLOR:
      crgsg_cat.debug(false) << "GL_SRC_COLOR)";
      break;
    case GL_ONE_MINUS_SRC_COLOR:
      crgsg_cat.debug(false) << "GL_ONE_MINUS_SRC_COLOR)";
      break;
    case GL_SRC_ALPHA:
      crgsg_cat.debug(false) << "GL_SRC_ALPHA)";
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      crgsg_cat.debug(false) << "GL_ONE_MINUS_SRC_ALPHA)";
      break;
    case GL_DST_ALPHA:
      crgsg_cat.debug(false) << "GL_DST_ALPHA)";
      break;
    case GL_ONE_MINUS_DST_ALPHA:
      crgsg_cat.debug(false) << "GL_ONE_MINUS_DST_ALPHA)";
      break;
    default:
      crgsg_cat.debug(false) << "unknown)";
      break;
    }
    crgsg_cat.debug(false) << endl;
#endif
    chromium.BlendFunc(sfunc, dfunc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glFogMode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glFogMode(GLint mode) {
  if (_fog_mode != mode) {
    _fog_mode = mode;
#ifdef GSG_VERBOSE
    crgsg_cat.debug() << "crFog(GL_FOG_MODE, ";
    switch(mode) {
    case GL_LINEAR:
      crgsg_cat.debug(false) << "GL_LINEAR)" << endl;
      break;
    case GL_EXP:
      crgsg_cat.debug(false) << "GL_EXP)" << endl;
      break;
    case GL_EXP2:
      crgsg_cat.debug(false) << "GL_EXP2)" << endl;
      break;
#ifdef GL_FOG_FUNC_SGIS
    case GL_FOG_FUNC_SGIS:
      crgsg_cat.debug(false) << "GL_FOG_FUNC_SGIS)" << endl;
      break;
#endif
    default:
      crgsg_cat.debug(false) << "unknown)" << endl;
      break;
    }
#endif
    chromium.Fogi(GL_FOG_MODE, mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glFogStart
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glFogStart(GLfloat start) {
  if (_fog_start != start) {
    _fog_start = start;
#ifdef GSG_VERBOSE
  crgsg_cat.debug()
    << "crFog(GL_FOG_START, " << start << ")" << endl;
#endif
    chromium.Fogf(GL_FOG_START, start);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glFogEnd
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glFogEnd(GLfloat end) {
  if (_fog_end != end) {
    _fog_end = end;
#ifdef GSG_VERBOSE
  crgsg_cat.debug()
    << "crFog(GL_FOG_END, " << end << ")" << endl;
#endif
    chromium.Fogf(GL_FOG_END, end);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glFogDensity
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glFogDensity(GLfloat density) {
  if (_fog_density != density) {
    _fog_density = density;
#ifdef GSG_VERBOSE
  crgsg_cat.debug()
    << "crFog(GL_FOG_DENSITY, " << density << ")" << endl;
#endif
    chromium.Fogf(GL_FOG_DENSITY, density);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glFogColor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glFogColor(const Colorf &color) {
  if (_fog_color != color) {
    _fog_color = color;
#ifdef GSG_VERBOSE
  crgsg_cat.debug()
    << "crFog(GL_FOG_COLOR, " << color << ")" << endl;
#endif
    chromium.Fogfv(GL_FOG_COLOR, color.get_data());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glAlphaFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glAlphaFunc(GLenum func, GLclampf ref) {
  if (_alpha_func != func || _alpha_func_ref != ref) {
    _alpha_func = func;
    _alpha_func_ref = ref;
#ifdef GSG_VERBOSE
    crgsg_cat.debug() << "crAlphaFunc(";
    switch (func) {
    case GL_NEVER:
      crgsg_cat.debug(false) << "GL_NEVER, ";
      break;
    case GL_LESS:
      crgsg_cat.debug(false) << "GL_LESS, ";
      break;
    case GL_EQUAL:
      crgsg_cat.debug(false) << "GL_EQUAL, ";
      break;
    case GL_LEQUAL:
      crgsg_cat.debug(false) << "GL_LEQUAL, ";
      break;
    case GL_GREATER:
      crgsg_cat.debug(false) << "GL_GREATER, ";
      break;
    case GL_NOTEQUAL:
      crgsg_cat.debug(false) << "GL_NOTEQUAL, ";
      break;
    case GL_GEQUAL:
      crgsg_cat.debug(false) << "GL_GEQUAL, ";
      break;
    case GL_ALWAYS:
      crgsg_cat.debug(false) << "GL_ALWAYS, ";
      break;
    }
    crgsg_cat.debug() << ref << ")" << endl;
#endif
    chromium.AlphaFunc(func, ref);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::call_glPolygonMode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
call_glPolygonMode(GLenum mode) {
  if (_polygon_mode != mode) {
    _polygon_mode = mode;
#ifdef GSG_VERBOSE
    crgsg_cat.debug() << "crPolygonMode(GL_BACK_AND_FRONT, ";
    switch (mode) {
    case GL_POINT:
      crgsg_cat.debug(false) << "GL_POINT)" << endl;
      break;
    case GL_LINE:
      crgsg_cat.debug(false) << "GL_LINE)" << endl;
      break;
    case GL_FILL:
      crgsg_cat.debug(false) << "GL_FILL)" << endl;
      break;
    }
#endif
    chromium.PolygonMode(GL_FRONT_AND_BACK, mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::set_pack_alignment
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
set_pack_alignment(GLint alignment) {
  if (_pack_alignment != alignment) {
    chromium.PixelStorei(GL_PACK_ALIGNMENT, alignment);
    _pack_alignment = alignment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::set_unpack_alignment
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
set_unpack_alignment(GLint alignment) {
  if (_unpack_alignment != alignment) {
    chromium.PixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    _unpack_alignment = alignment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_multisample
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_multisample(bool val) {
  if (_multisample_enabled != val) {
    _multisample_enabled = val;
    if (val) {
#ifdef GL_MULTISAMPLE_SGIS
      chromium.Enable(GL_MULTISAMPLE_SGIS);
#endif
      chromium.Hint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    } else {
#ifdef GL_MULTISAMPLE_SGIS
      chromium.Disable(GL_MULTISAMPLE_SGIS);
#endif
      chromium.Hint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_line_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_line_smooth(bool val) {
  if (_line_smooth_enabled != val) {
    _line_smooth_enabled = val;
    if (val) {
      chromium.Enable(GL_LINE_SMOOTH);
      chromium.Hint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    } else {
      chromium.Disable(GL_LINE_SMOOTH);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_point_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_point_smooth(bool val) {
  if (_point_smooth_enabled != val) {
    _point_smooth_enabled = val;
    if (val) {
      chromium.Enable(GL_POINT_SMOOTH);
      chromium.Hint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    } else {
      chromium.Disable(GL_POINT_SMOOTH);
      chromium.Hint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_dither
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_dither(bool val) {
  if (_dither_enabled != val) {
    _dither_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_DITHER)" << endl;
#endif
      chromium.Enable(GL_DITHER);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_DITHER)" << endl;
#endif
      chromium.Disable(GL_DITHER);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_stencil_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_stencil_test(bool val) {
  if (_stencil_test_enabled != val) {
    _stencil_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_STENCIL_TEST)" << endl;
#endif
      chromium.Enable(GL_STENCIL_TEST);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_STENCIL_TEST)" << endl;
#endif
      chromium.Disable(GL_STENCIL_TEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_texturing
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_texturing(bool val) {
  if (_texturing_enabled != val) {
    _texturing_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_TEXTURE_2D)" << endl;
#endif
      chromium.Enable(GL_TEXTURE_2D);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_TEXTURE_2D)" << endl;
#endif
      chromium.Disable(GL_TEXTURE_2D);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_scissor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_scissor(bool val)
{
    if ( _scissor_enabled != val ) {
        _scissor_enabled = val;
        if ( val )
            chromium.Enable( GL_SCISSOR_TEST );
        else
            chromium.Disable( GL_SCISSOR_TEST );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_clip_plane
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_clip_plane(int clip_plane, bool val) {
  if (_clip_plane_enabled[clip_plane] != val) {
    _clip_plane_enabled[clip_plane] = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_CLIP_PLANE_" << clip_plane << ")" << endl;
#endif
      chromium.Enable(get_clip_plane_id(clip_plane));
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_CLIP_PLANE_" << clip_plane << ")" << endl;
#endif
      chromium.Disable(get_clip_plane_id(clip_plane));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_multisample_alpha_one
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_multisample_alpha_one(bool val) {
  if (_multisample_alpha_one_enabled != val) {
    _multisample_alpha_one_enabled = val;
#ifdef GL_SAMPLE_ALPHA_TO_ONE_SGIS
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_SAMPLE_ALPHA_TO_ONE_SGIS)" << endl;
#endif
      chromium.Enable(GL_SAMPLE_ALPHA_TO_ONE_SGIS);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_SAMPLE_ALPHA_TO_ONE_SGIS)" << endl;
#endif
      chromium.Disable(GL_SAMPLE_ALPHA_TO_ONE_SGIS);
    }
#endif  // GL_SAMPLE_ALPHA_TO_ONE_SGIS
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_multisample_alpha_mask
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_multisample_alpha_mask(bool val) {
  if (_multisample_alpha_mask_enabled != val) {
    _multisample_alpha_mask_enabled = val;
#ifdef GL_SAMPLE_ALPHA_TO_MASK_SGIS
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_SAMPLE_ALPHA_TO_MASK_SGIS)" << endl;
#endif
      chromium.Enable(GL_SAMPLE_ALPHA_TO_MASK_SGIS);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_SAMPLE_ALPHA_TO_MASK_SGIS)" << endl;
#endif
      chromium.Disable(GL_SAMPLE_ALPHA_TO_MASK_SGIS);
    }
#endif  // GL_SAMPLE_ALPHA_TO_MASK_SGIS
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_BLEND)" << endl;
#endif
      chromium.Enable(GL_BLEND);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_BLEND)" << endl;
#endif
      chromium.Disable(GL_BLEND);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_depth_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_depth_test(bool val) {
  if (_depth_test_enabled != val) {
    _depth_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_DEPTH_TEST)" << endl;
#endif
      chromium.Enable(GL_DEPTH_TEST);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_DEPTH_TEST)" << endl;
#endif
      chromium.Disable(GL_DEPTH_TEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_fog
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_fog(bool val) {
  if (_fog_enabled != val) {
    _fog_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_FOG)" << endl;
#endif
      chromium.Enable(GL_FOG);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_FOG)" << endl;
#endif
      chromium.Disable(GL_FOG);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_alpha_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_alpha_test(bool val) {
  if (_alpha_test_enabled != val) {
    _alpha_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_ALPHA_TEST)" << endl;
#endif
      chromium.Enable(GL_ALPHA_TEST);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_ALPHA_TEST)" << endl;
#endif
      chromium.Disable(GL_ALPHA_TEST);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::enable_polygon_offset
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
enable_polygon_offset(bool val) {
  if (_polygon_offset_enabled != val) {
    _polygon_offset_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crEnable(GL_POLYGON_OFFSET_*)" << endl;
#endif
      chromium.Enable(GL_POLYGON_OFFSET_FILL);
    } else {
#ifdef GSG_VERBOSE
      crgsg_cat.debug()
        << "crDisable(GL_POLYGON_OFFSET_*)" << endl;
#endif
      chromium.Disable(GL_POLYGON_OFFSET_FILL);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::get_current_color_mat
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CRGraphicsStateGuardian::
get_current_color_mat() const {
  return _current_color_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::get_current_alpha_offset
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const float &CRGraphicsStateGuardian::
get_current_alpha_offset() const {
  return _current_alpha_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::get_current_alpha_scale
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const float &CRGraphicsStateGuardian::
get_current_alpha_scale() const {
  return _current_alpha_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::get_light_id
//       Access: Public
//  Description: Convert index to gl light id
////////////////////////////////////////////////////////////////////
INLINE GLenum CRGraphicsStateGuardian::get_light_id(int index) const
{
  return GL_LIGHT0 + index;
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::get_clip_plane_id
//       Access: Public
//  Description: Convert index to gl clip plane id
////////////////////////////////////////////////////////////////////
INLINE GLenum CRGraphicsStateGuardian::
get_clip_plane_id(int index) const {
  switch(index) {
    case 0: return GL_CLIP_PLANE0;
    case 1: return GL_CLIP_PLANE1;
    case 2: return GL_CLIP_PLANE2;
    case 3: return GL_CLIP_PLANE3;
    case 4: return GL_CLIP_PLANE4;
    case 5: return GL_CLIP_PLANE5;
    default:
      crgsg_cat.error()
        << "get_clip_plane_id() - we don't currently support ids "
        << "> 5" << endl;
      break;
  }
  return GL_CLIP_PLANE0;
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::issue_scene_graph_color
//       Access: Public
//  Description: Checks whether the scene graph color needs to be
//               issued, and sends the appropriate chromium.Color command if
//               it does.
////////////////////////////////////////////////////////////////////
INLINE void CRGraphicsStateGuardian::
issue_scene_graph_color() {
  if (_scene_graph_color_stale) {
    issue_transformed_color(_scene_graph_color);
    _scene_graph_color_stale = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CRGraphicsStateGuardian::report_errors
//       Access: Public
//  Description: Checks for any outstanding error codes and outputs
//               them, if found.  If NDEBUG is defined, this function
//               does nothing.
////////////////////////////////////////////////////////////////////
INLINE void 
report_errors() {
    // pulling this out of gsg since I'd like to call it when there's no gsg ptr around
#ifndef NDEBUG
  extern void report_errors_loop(GLenum error_code);

  GLenum error_code = chromium.GetError();
  if (error_code != GL_NO_ERROR) {
    report_errors_loop(error_code);
  }
#endif
}
