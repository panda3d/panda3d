// Filename: glGraphicsStateGuardian_src.I
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


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::report_errors
//       Access: Public, Static
//  Description: Checks for any outstanding error codes and outputs
//               them, if found.  If NDEBUG is defined, this function
//               does nothing.  The return value is true if everything is
//               ok, or false if we should shut down.
//
//               This is a static method so it can be called when
//               there's no gsg pointer around.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
report_errors(int line, const char *source_file) {
#ifndef NDEBUG
  GLenum error_code = GLP(GetError)();
  if (error_code != GL_NO_ERROR) {
    int error_count = 0;
    return report_errors_loop(line, source_file, error_code, error_count);
  }
#endif
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::report_my_errors
//       Access: Public
//  Description: Like report_errors(), above, but non-static so we can
//               throw an event on failure.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
report_my_errors(int line, const char *source_file) {
#ifndef NDEBUG
  GLenum error_code = GLP(GetError)();
  if (error_code != GL_NO_ERROR) {
    if (!report_errors_loop(line, source_file, error_code, _error_count)) {
      panic_deactivate();
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glClearColor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glClearColor(GLclampf red, GLclampf green, GLclampf blue,
                  GLclampf alpha) {
  if (red != _clear_color_red ||
      green != _clear_color_green ||
      blue != _clear_color_blue ||
      alpha != _clear_color_alpha) {
    GLP(ClearColor)(red, green, blue, alpha);
    _clear_color_red = red;
    _clear_color_green = green;
    _clear_color_blue = blue;
    _clear_color_alpha = alpha;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glClearDepth
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glClearDepth(GLclampd depth) {
  if (depth != _clear_depth) {
#ifdef GSG_VERBOSE
    GLCAT.spam()
      << "glClearDepth(" << (double)depth << ")" << endl;
#endif
    GLP(ClearDepth)(depth);
    _clear_depth = depth;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glClearStencil
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glClearStencil(GLint s) {
  if (s != _clear_stencil) {
    GLP(ClearStencil)(s);
    _clear_stencil = s;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glClearAccum
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glClearAccum(GLclampf red, GLclampf green, GLclampf blue,
                  GLclampf alpha) {
  if (red != _clear_accum_red ||
      green != _clear_accum_green ||
      blue != _clear_accum_blue ||
      alpha != _clear_accum_alpha) {
    GLP(ClearAccum)(red, green, blue, alpha);
    _clear_accum_red = red;
    _clear_accum_green = green;
    _clear_accum_blue = blue;
    _clear_accum_alpha = alpha;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glShadeModel
//       Access:
//  Description: Set the shading model to be either GL_FLAT or GL_SMOOTH
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glShadeModel(GLenum mode) {
  if (_shade_model_mode != mode) {
    GLP(ShadeModel)(mode);
    _shade_model_mode = mode;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glScissor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if ( _scissor_x != x || _scissor_y != y ||
        _scissor_width != width || _scissor_height != height )
    {
        _scissor_x = x; _scissor_y = y;
        _scissor_width = width; _scissor_height = height;
        GLP(Scissor)( x, y, width, height );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glViewport
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if ( _viewport_x != x || _viewport_y != y ||
        _viewport_width != width || _viewport_height != height )
    {
        _viewport_x = x; _viewport_y = y;
        _viewport_width = width; _viewport_height = height;
        GLP(Viewport)( x, y, width, height );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glLightModelLocal
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glLightModelLocal(GLboolean local)
{
    if ( _lmodel_local != local )
    {
        _lmodel_local = local;
        GLP(LightModeli)( GL_LIGHT_MODEL_LOCAL_VIEWER, local );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glLightModelLocal
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glLightModelTwoSide(GLboolean twoside)
{
  if (_lmodel_twoside != twoside) {
    _lmodel_twoside = twoside;
#ifdef GSG_VERBOSE
    GLCAT.spam()
      << "glLightModel(GL_LIGHT_MODEL_TWO_SIDE, " << (int)twoside << ")" << endl;
#endif
    GLP(LightModeli)(GL_LIGHT_MODEL_TWO_SIDE, twoside);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glStencilFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glStencilFunc(GLenum func,GLint ref,GLuint mask) {

#ifdef GSG_VERBOSE
    GLCAT.spam() << "glStencilFunc(";
    switch (func) {
    case GL_NEVER:
      GLCAT.spam(false) << "GL_NEVER, ";
      break;
    case GL_LESS:
      GLCAT.spam(false) << "GL_LESS, ";
      break;
    case GL_EQUAL:
      GLCAT.spam(false) << "GL_EQUAL, ";
      break;
    case GL_LEQUAL:
      GLCAT.spam(false) << "GL_LEQUAL, ";
      break;
    case GL_GREATER:
      GLCAT.spam(false) << "GL_GREATER, ";
      break;
    case GL_NOTEQUAL:
      GLCAT.spam(false) << "GL_NOTEQUAL, ";
      break;
    case GL_GEQUAL:
      GLCAT.spam(false) << "GL_GEQUAL, ";
      break;
    case GL_ALWAYS:
      GLCAT.spam(false) << "GL_ALWAYS, ";
      break;
    default:
      GLCAT.spam(false) << "unknown, ";
      break;
    }
    GLCAT.spam(false) << (int)ref << ", " << (int)mask << ")\n";
#endif
    GLP(StencilFunc)(func, ref, mask);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glStencilOp
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glStencilOp(GLenum fail,GLenum zfail,GLenum zpass) {
#ifdef GSG_VERBOSE
    GLCAT.spam() << "glStencilOp(fail, zfail, ";
    switch (zpass) {
    case GL_KEEP:
      GLCAT.spam(false) << "GL_KEEP)";
      break;
    case GL_ZERO:
      GLCAT.spam(false) << "GL_ZERO)";
      break;
    case GL_REPLACE:
      GLCAT.spam(false) << "GL_REPLACE)";
      break;
    case GL_INCR:
      GLCAT.spam(false) << "GL_INCR)";
      break;
    case GL_DECR:
      GLCAT.spam(false) << "GL_DECR)";
      break;
    case GL_INVERT:
      GLCAT.spam(false) << "GL_INVERT)";
      break;
    default:
      GLCAT.spam(false) << "unknown)";
      break;
    }
    GLCAT.spam(false) << endl;
#endif
    GLP(StencilOp)(fail,zfail,zpass);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glLineWidth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glLineWidth(GLfloat width) {
  if (_line_width != width) {
    _line_width = width;
#ifdef GSG_VERBOSE
  GLCAT.spam()
    << "glLineWidth(" << width << ")" << endl;
#endif
    GLP(LineWidth)(width);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glPointSize
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glPointSize(GLfloat size) {
  if (_point_size != size) {
    _point_size = size;
#ifdef GSG_VERBOSE
  GLCAT.spam()
    << "glPointSize(" << size << ")" << endl;
#endif
    GLP(PointSize)(size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glBlendFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glBlendFunc(GLenum sfunc, GLenum dfunc) {
  if (_blend_source_func != sfunc || _blend_dest_func != dfunc) {
    _blend_source_func = sfunc;
    _blend_dest_func = dfunc;
#ifdef GSG_VERBOSE
    GLCAT.spam() << "glBlendFunc(";
    switch (sfunc) {
    case GL_ZERO:
      GLCAT.spam(false) << "GL_ZERO, ";
      break;
    case GL_ONE:
      GLCAT.spam(false) << "GL_ONE, ";
      break;
    case GL_DST_COLOR:
      GLCAT.spam(false) << "GL_DST_COLOR, ";
      break;
    case GL_ONE_MINUS_DST_COLOR:
      GLCAT.spam(false) << "GL_ONE_MINUS_DST_COLOR, ";
      break;
    case GL_SRC_ALPHA:
      GLCAT.spam(false) << "GL_SRC_ALPHA, ";
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      GLCAT.spam(false) << "GL_ONE_MINUS_SRC_ALPHA, ";
      break;
    case GL_DST_ALPHA:
      GLCAT.spam(false) << "GL_DST_ALPHA, ";
      break;
    case GL_ONE_MINUS_DST_ALPHA:
      GLCAT.spam(false) << "GL_ONE_MINUS_DST_ALPHA, ";
      break;
    case GL_SRC_ALPHA_SATURATE:

      GLCAT.spam(false) << "GL_SRC_ALPHA_SATURATE, ";
      break;
    default:
      GLCAT.spam(false) << "unknown, ";
      break;
    }
    switch (dfunc) {
    case GL_ZERO:
      GLCAT.spam(false) << "GL_ZERO)";
      break;
    case GL_ONE:
      GLCAT.spam(false) << "GL_ONE)";
      break;
    case GL_SRC_COLOR:
      GLCAT.spam(false) << "GL_SRC_COLOR)";
      break;
    case GL_ONE_MINUS_SRC_COLOR:
      GLCAT.spam(false) << "GL_ONE_MINUS_SRC_COLOR)";
      break;
    case GL_SRC_ALPHA:
      GLCAT.spam(false) << "GL_SRC_ALPHA)";
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      GLCAT.spam(false) << "GL_ONE_MINUS_SRC_ALPHA)";
      break;
    case GL_DST_ALPHA:
      GLCAT.spam(false) << "GL_DST_ALPHA)";
      break;
    case GL_ONE_MINUS_DST_ALPHA:
      GLCAT.spam(false) << "GL_ONE_MINUS_DST_ALPHA)";
      break;
    default:
      GLCAT.spam(false) << "unknown)";
      break;
    }
    GLCAT.spam(false) << endl;
#endif
    GLP(BlendFunc)(sfunc, dfunc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glFogMode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glFogMode(GLint mode) {
  if (_fog_mode != mode) {
    _fog_mode = mode;
#ifdef GSG_VERBOSE
    GLCAT.spam() << "glFog(GL_FOG_MODE, ";
    switch(mode) {
    case GL_LINEAR:
      GLCAT.spam(false) << "GL_LINEAR)" << endl;
      break;
    case GL_EXP:
      GLCAT.spam(false) << "GL_EXP)" << endl;
      break;
    case GL_EXP2:
      GLCAT.spam(false) << "GL_EXP2)" << endl;
      break;
#ifdef GL_FOG_FUNC_SGIS
    case GL_FOG_FUNC_SGIS:
      GLCAT.spam(false) << "GL_FOG_FUNC_SGIS)" << endl;
      break;
#endif
    default:
      GLCAT.spam(false) << "unknown)" << endl;
      break;
    }
#endif
    GLP(Fogi)(GL_FOG_MODE, mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glFogStart
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glFogStart(GLfloat start) {
  if (_fog_start != start) {
    _fog_start = start;
#ifdef GSG_VERBOSE
  GLCAT.spam()
    << "glFog(GL_FOG_START, " << start << ")" << endl;
#endif
    GLP(Fogf)(GL_FOG_START, start);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glFogEnd
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glFogEnd(GLfloat end) {
  if (_fog_end != end) {
    _fog_end = end;
#ifdef GSG_VERBOSE
  GLCAT.spam()
    << "glFog(GL_FOG_END, " << end << ")" << endl;
#endif
    GLP(Fogf)(GL_FOG_END, end);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glFogDensity
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glFogDensity(GLfloat density) {
  if (_fog_density != density) {
    _fog_density = density;
#ifdef GSG_VERBOSE
  GLCAT.spam()
    << "glFog(GL_FOG_DENSITY, " << density << ")" << endl;
#endif
    GLP(Fogf)(GL_FOG_DENSITY, density);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glFogColor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glFogColor(const Colorf &color) {
  if (_fog_color != color) {
    _fog_color = color;
#ifdef GSG_VERBOSE
  GLCAT.spam()
    << "glFog(GL_FOG_COLOR, " << color << ")" << endl;
#endif
    GLP(Fogfv)(GL_FOG_COLOR, color.get_data());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glAlphaFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glAlphaFunc(GLenum func, GLclampf ref) {
  if (_alpha_func != func || _alpha_func_ref != ref) {
    _alpha_func = func;
    _alpha_func_ref = ref;
#ifdef GSG_VERBOSE
    GLCAT.spam() << "glAlphaFunc(";
    switch (func) {
    case GL_NEVER:
      GLCAT.spam(false) << "GL_NEVER, ";
      break;
    case GL_LESS:
      GLCAT.spam(false) << "GL_LESS, ";
      break;
    case GL_EQUAL:
      GLCAT.spam(false) << "GL_EQUAL, ";
      break;
    case GL_LEQUAL:
      GLCAT.spam(false) << "GL_LEQUAL, ";
      break;
    case GL_GREATER:
      GLCAT.spam(false) << "GL_GREATER, ";
      break;
    case GL_NOTEQUAL:
      GLCAT.spam(false) << "GL_NOTEQUAL, ";
      break;
    case GL_GEQUAL:
      GLCAT.spam(false) << "GL_GEQUAL, ";
      break;
    case GL_ALWAYS:
      GLCAT.spam(false) << "GL_ALWAYS, ";
      break;
    }
    GLCAT.spam() << ref << ")" << endl;
#endif
    GLP(AlphaFunc)(func, ref);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::call_glPolygonMode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glPolygonMode(GLenum mode) {
  if (_polygon_mode != mode) {
    _polygon_mode = mode;
#ifdef GSG_VERBOSE
    GLCAT.spam() << "glPolygonMode(GL_BACK_AND_FRONT, ";
    switch (mode) {
    case GL_POINT:
      GLCAT.spam(false) << "GL_POINT)" << endl;
      break;
    case GL_LINE:
      GLCAT.spam(false) << "GL_LINE)" << endl;
      break;
    case GL_FILL:
      GLCAT.spam(false) << "GL_FILL)" << endl;
      break;
    }
#endif
    GLP(PolygonMode)(GL_FRONT_AND_BACK, mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::set_pack_alignment
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
set_pack_alignment(GLint alignment) {
  if (_pack_alignment != alignment) {
    GLP(PixelStorei)(GL_PACK_ALIGNMENT, alignment);
    _pack_alignment = alignment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::set_unpack_alignment
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
set_unpack_alignment(GLint alignment) {
  if (_unpack_alignment != alignment) {
    GLP(PixelStorei)(GL_UNPACK_ALIGNMENT, alignment);
    _unpack_alignment = alignment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_multisample
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_multisample(bool val) {
  if (_multisample_enabled != val) {
    _multisample_enabled = val;
    if (val) {
#ifdef GL_MULTISAMPLE_SGIS
      GLP(Enable)(GL_MULTISAMPLE_SGIS);
#endif
      GLP(Hint)(GL_POINT_SMOOTH_HINT, GL_NICEST);
    } else {
#ifdef GL_MULTISAMPLE_SGIS
      GLP(Disable)(GL_MULTISAMPLE_SGIS);
#endif
      GLP(Hint)(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_line_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_line_smooth(bool val) {
  if (_line_smooth_enabled != val) {
    _line_smooth_enabled = val;
    if (val) {
      GLP(Enable)(GL_LINE_SMOOTH);
      GLP(Hint)(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    } else {
      GLP(Disable)(GL_LINE_SMOOTH);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_point_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_point_smooth(bool val) {
  if (_point_smooth_enabled != val) {
    _point_smooth_enabled = val;
    if (val) {
      GLP(Enable)(GL_POINT_SMOOTH);
      GLP(Hint)(GL_POINT_SMOOTH_HINT, GL_NICEST);
    } else {
      GLP(Disable)(GL_POINT_SMOOTH);
      GLP(Hint)(GL_POINT_SMOOTH_HINT, GL_FASTEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_stencil_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_stencil_test(bool val) {
  if (_stencil_test_enabled != val) {
    _stencil_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_STENCIL_TEST)" << endl;
#endif
      GLP(Enable)(GL_STENCIL_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_STENCIL_TEST)" << endl;
#endif
      GLP(Disable)(GL_STENCIL_TEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_texturing
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_texturing(bool val) {
  if (_texturing_enabled != val) {
    _texturing_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_TEXTURE_2D)" << endl;
#endif
      GLP(Enable)(GL_TEXTURE_2D);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_TEXTURE_2D)" << endl;
#endif
      GLP(Disable)(GL_TEXTURE_2D);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_scissor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_scissor(bool val)
{
    if ( _scissor_enabled != val ) {
        _scissor_enabled = val;
        if ( val )
            GLP(Enable)( GL_SCISSOR_TEST );
        else
            GLP(Disable)( GL_SCISSOR_TEST );
    }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_multisample_alpha_one
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_multisample_alpha_one(bool val) {
  if (_multisample_alpha_one_enabled != val) {
    _multisample_alpha_one_enabled = val;
#ifdef GL_SAMPLE_ALPHA_TO_ONE_SGIS
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_SAMPLE_ALPHA_TO_ONE_SGIS)" << endl;
#endif
      GLP(Enable)(GL_SAMPLE_ALPHA_TO_ONE_SGIS);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_SAMPLE_ALPHA_TO_ONE_SGIS)" << endl;
#endif
      GLP(Disable)(GL_SAMPLE_ALPHA_TO_ONE_SGIS);
    }
#endif  // GL_SAMPLE_ALPHA_TO_ONE_SGIS
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_multisample_alpha_mask
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_multisample_alpha_mask(bool val) {
  if (_multisample_alpha_mask_enabled != val) {
    _multisample_alpha_mask_enabled = val;
#ifdef GL_SAMPLE_ALPHA_TO_MASK_SGIS
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_SAMPLE_ALPHA_TO_MASK_SGIS)" << endl;
#endif
      GLP(Enable)(GL_SAMPLE_ALPHA_TO_MASK_SGIS);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_SAMPLE_ALPHA_TO_MASK_SGIS)" << endl;
#endif
      GLP(Disable)(GL_SAMPLE_ALPHA_TO_MASK_SGIS);
    }
#endif  // GL_SAMPLE_ALPHA_TO_MASK_SGIS
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_BLEND)" << endl;
#endif
      GLP(Enable)(GL_BLEND);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_BLEND)" << endl;
#endif
      GLP(Disable)(GL_BLEND);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_depth_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_depth_test(bool val) {
  if (_depth_test_enabled != val) {
    _depth_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_DEPTH_TEST)" << endl;
#endif
      GLP(Enable)(GL_DEPTH_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_DEPTH_TEST)" << endl;
#endif
      GLP(Disable)(GL_DEPTH_TEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_fog
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_fog(bool val) {
  if (_fog_enabled != val) {
    _fog_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_FOG)" << endl;
#endif
      GLP(Enable)(GL_FOG);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_FOG)" << endl;
#endif
      GLP(Disable)(GL_FOG);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_alpha_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_alpha_test(bool val) {
  if (_alpha_test_enabled != val) {
    _alpha_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_ALPHA_TEST)" << endl;
#endif
      GLP(Enable)(GL_ALPHA_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_ALPHA_TEST)" << endl;
#endif
      GLP(Disable)(GL_ALPHA_TEST);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_polygon_offset
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_polygon_offset(bool val) {
  if (_polygon_offset_enabled != val) {
    _polygon_offset_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_POLYGON_OFFSET_*)" << endl;
#endif
      GLP(Enable)(GL_POLYGON_OFFSET_FILL);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_POLYGON_OFFSET_*)" << endl;
#endif
      GLP(Disable)(GL_POLYGON_OFFSET_FILL);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_light_id
//       Access: Public
//  Description: Convert index to gl light id
////////////////////////////////////////////////////////////////////
INLINE GLenum CLP(GraphicsStateGuardian)::get_light_id(int index) const {
  return GL_LIGHT0 + index;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_clip_plane_id
//       Access: Public
//  Description: Convert index to gl clip plane id
////////////////////////////////////////////////////////////////////
INLINE GLenum CLP(GraphicsStateGuardian)::
get_clip_plane_id(int index) const {
  return GL_CLIP_PLANE0 + index;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::issue_scene_graph_color
//       Access: Public
//  Description: Checks whether the scene graph color needs to be
//               issued, and sends the appropriate glColor command if
//               it does.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
issue_scene_graph_color() {
  if (_scene_graph_color_stale) {
    issue_transformed_color(_scene_graph_color);
    _scene_graph_color_stale = false;
  }
}
