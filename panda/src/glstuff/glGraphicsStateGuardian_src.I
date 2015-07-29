// Filename: glGraphicsStateGuardian_src.I
// Created by:  drose (02Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::report_errors
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
  PStatTimer timer(_check_error_pcollector);
  GLenum error_code = glGetError();
  if (error_code != GL_NO_ERROR) {
    int error_count = 0;
    return report_errors_loop(line, source_file, error_code, error_count);
  }
#endif
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::report_my_errors
//       Access: Public
//  Description: Like report_errors(), above, but non-static so we can
//               throw an event on failure.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
report_my_errors(int line, const char *source_file) {
#ifndef NDEBUG
  if (_check_errors) {
    PStatTimer timer(_check_error_pcollector);
    GLenum error_code = glGetError();
    if (error_code != GL_NO_ERROR) {
      if (!report_errors_loop(line, source_file, error_code, _error_count)) {
        panic_deactivate();
      }
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::clear_errors
//       Access: Public, Static
//  Description: This works like report_errors(), except that it
//               always runs, even in the NDEBUG case.
//
//               It is designed to be called when it is important to
//               clear the error stack (for instance, because we want
//               to be able to reliably check the result of some
//               upcoming GL operation).
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
clear_errors(int line, const char *source_file) {
  PStatTimer timer(_check_error_pcollector);
  GLenum error_code = glGetError();
  if (error_code != GL_NO_ERROR) {
    int error_count = 0;
    return report_errors_loop(line, source_file, error_code, error_count);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::clear_my_errors
//       Access: Public
//  Description: This works like report_my_errors(), except that it
//               always runs, even in the NDEBUG case (but not when
//               _check_errors is false), and it never calls
//               panic_deactivate().  It is designed to be called when
//               it is important to clear the error stack (for
//               instance, because we want to be able to reliably
//               check the result of some upcoming GL operation).
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
clear_my_errors(int line, const char *source_file) {
  if (_check_errors) {
    PStatTimer timer(_check_error_pcollector);
    GLenum error_code = glGetError();
    if (error_code != GL_NO_ERROR) {
      int error_count = 0;
      report_errors_loop(line, source_file, error_code, error_count);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_gl_vendor
//       Access: Public
//  Description: Returns the GL vendor string reported by the driver.
////////////////////////////////////////////////////////////////////
INLINE const string &CLP(GraphicsStateGuardian)::
get_gl_vendor() const {
  return _gl_vendor;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_gl_renderer
//       Access: Public
//  Description: Returns the GL renderer string reported by the driver.
////////////////////////////////////////////////////////////////////
INLINE const string &CLP(GraphicsStateGuardian)::
get_gl_renderer() const {
  return _gl_renderer;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_gl_version
//       Access: Public
//  Description: Returns the GL version string reported by the driver.
////////////////////////////////////////////////////////////////////
INLINE const string &CLP(GraphicsStateGuardian)::
get_gl_version() const {
  return _gl_version;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_gl_version_major
//       Access: Public
//  Description: Returns the major part of the reported GL version
//               number.
////////////////////////////////////////////////////////////////////
INLINE int CLP(GraphicsStateGuardian)::
get_gl_version_major() const {
  return _gl_version_major;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_gl_version_minor
//       Access: Public
//  Description: Returns the minor part of the reported GL version
//               number.
////////////////////////////////////////////////////////////////////
INLINE int CLP(GraphicsStateGuardian)::
get_gl_version_minor() const {
  return _gl_version_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::maybe_gl_finish
//       Access: Protected
//  Description: Calls glFinish() if the config variable gl-finish is
//               set True.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
maybe_gl_finish() const {
#ifdef DO_PSTATS
  if (gl_finish) {
    glFinish();
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::has_extension
//       Access: Published, Final
//  Description: Returns true if the indicated extension is reported
//               by the GL system, false otherwise.  The extension
//               name is case-sensitive.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
has_extension(const string &extension) const {
  bool has_ext = (_extensions.find(extension) != _extensions.end());
#ifndef NDEBUG
  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "HAS EXT " << extension << " " << has_ext << "\n";
  }
#endif
  return has_ext;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::is_at_least_gl_version
//       Access: Public
//  Description: Returns true if we are compiled for mainline OpenGL,
//               and the runtime GL version number is at least the
//               indicated value, false otherwise.  Under OpenGL ES,
//               this always returns false.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
is_at_least_gl_version(int major_version, int minor_version) const {
#ifdef OPENGLES
  return false;
#else
  if (_gl_version_major < major_version) {
    return false;
  } else if (_gl_version_major == major_version) {
    if (_gl_version_minor < minor_version) {
      return false;
    }
  }

  return true;
#endif  // OPENGLES
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::is_at_least_gles_version
//       Access: Public
//  Description: Returns true if we are compiled for OpenGL ES,
//               and the runtime GL ES version number is at least the
//               indicated value, false otherwise.  Under mainline
//               OpenGL, this always returns false.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
is_at_least_gles_version(int major_version, int minor_version) const {
#ifndef OPENGLES
  return false;
#else
  if (_gl_version_major < major_version) {
    return false;
  } else if (_gl_version_major == major_version) {
    if (_gl_version_minor < minor_version) {
      return false;
    }
  }

  return true;
#endif  // OPENGLES
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_multisample_antialias
//       Access: Protected
//  Description: Specifies whether multisample should be enabled for
//               antialiasing purposes.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_multisample_antialias(bool val) {
#ifndef OPENGLES_2
  if (_supports_multisample) {
    if ((_multisample_mode & MM_antialias) != 0 && !val) {
      // Turn off antialias multisample.
      _multisample_mode &= ~MM_antialias;
      if (_multisample_mode == 0) {
        glDisable(GL_MULTISAMPLE);
      }
    } else if ((_multisample_mode & MM_antialias) == 0 && val) {
      // Turn on antialias multisample.
      if (_multisample_mode == 0) {
        glEnable(GL_MULTISAMPLE);
      }
      _multisample_mode |= MM_antialias;
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_multisample_alpha_one
//       Access: Protected
//  Description: Specifies whether multisample should be enabled for
//               transparency purposes, using the sample_alpha_to_one
//               mode.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_multisample_alpha_one(bool val) {
#ifndef OPENGLES_2
  if (_supports_multisample) {
    if ((_multisample_mode & MM_alpha_one) != 0 && !val) {
      // Turn off sample_alpha_to_one multisample.
      _multisample_mode &= ~MM_alpha_one;
      glDisable(GL_SAMPLE_ALPHA_TO_ONE);
      if (_multisample_mode == 0) {
        glDisable(GL_MULTISAMPLE);
      }
    } else if ((_multisample_mode & MM_alpha_one) == 0 && val) {
      // Turn on sample_alpha_to_one multisample.
      if (_multisample_mode == 0) {
        glEnable(GL_MULTISAMPLE);
      }
      glEnable(GL_SAMPLE_ALPHA_TO_ONE);
      _multisample_mode |= MM_alpha_one;
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_multisample_alpha_mask
//       Access: Protected
//  Description: Specifies whether multisample should be enabled for
//               transparency purposes, using the sample_alpha_to_mask
//               mode.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_multisample_alpha_mask(bool val) {
#ifndef OPENGLES_2
  if (_supports_multisample) {
    if ((_multisample_mode & MM_alpha_mask) != 0 && !val) {
      // Turn off sample_alpha_to_mask multisample.
      _multisample_mode &= ~MM_alpha_mask;
      glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
      if (_multisample_mode == 0) {
        glDisable(GL_MULTISAMPLE);
      }
    } else if ((_multisample_mode & MM_alpha_mask) == 0 && val) {
      // Turn on sample_alpha_to_mask multisample.
      if (_multisample_mode == 0) {
        glEnable(GL_MULTISAMPLE);
      }
      glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
      _multisample_mode |= MM_alpha_mask;
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_line_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_line_smooth(bool val) {
#ifndef OPENGLES_2
  if (_line_smooth_enabled != val) {
    _state_mask.clear_bit(TransparencyAttrib::get_class_slot());
    _line_smooth_enabled = val;
    if (val) {
      glEnable(GL_LINE_SMOOTH);
    } else {
      glDisable(GL_LINE_SMOOTH);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_point_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_point_smooth(bool val) {
#ifdef SUPPORT_FIXED_FUNCTION
  if (_point_smooth_enabled != val) {
    _state_mask.clear_bit(TransparencyAttrib::get_class_slot());
    _point_smooth_enabled = val;
    if (val) {
      glEnable(GL_POINT_SMOOTH);
    } else {
      glDisable(GL_POINT_SMOOTH);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_polygon_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_polygon_smooth(bool val) {
#ifndef OPENGLES  // GL_POLYGON_SMOOTH not supported in OpenGL ES.
  if (_polygon_smooth_enabled != val) {
    _polygon_smooth_enabled = val;
    if (val) {
      glEnable(GL_POLYGON_SMOOTH);
    } else {
      glDisable(GL_POLYGON_SMOOTH);
    }
  }
#endif  // OPENGLES
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::setup_antialias_line
//       Access: Protected
//  Description: Sets the appropriate antialiasing modes to render a
//               series of line primitives, according to
//               _auto_antialias_mode.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
setup_antialias_line() {
  if (_auto_antialias_mode) {
    // Lines supposedly look better using line smoothing, even if we
    // have multisample available.
    enable_multisample_antialias(false);
    enable_line_smooth(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::setup_antialias_point
//       Access: Protected
//  Description: Sets the appropriate antialiasing modes to render a
//               series of point primitives, according to
//               _auto_antialias_mode.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
setup_antialias_point() {
  if (_auto_antialias_mode) {
    // Points supposedly look better using point smoothing, even if we
    // have multisample available.
    enable_multisample_antialias(false);
    enable_point_smooth(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::setup_antialias_polygon
//       Access: Protected
//  Description: Sets the appropriate antialiasing modes to render a
//               series of polygon primitives, according to
//               _auto_antialias_mode.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
setup_antialias_polygon() {
  if (_auto_antialias_mode) {
    switch (_render_mode) {
    case RenderModeAttrib::M_wireframe:
      // In wireframe mode, we're really drawing lines.
      enable_multisample_antialias(false);
      enable_line_smooth(true);
      break;

    case RenderModeAttrib::M_point:
      // In point mode, we're drawing points.
      enable_multisample_antialias(false);
      enable_point_smooth(true);
      break;

    default:
      // For polygons, multisample is best if it's available, otherwise
      // polygon smoothing will do.
      enable_line_smooth(false);
      enable_point_smooth(false);
      if (_supports_multisample) {
        enable_multisample_antialias(true);
      } else {
        enable_polygon_smooth(true);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_stencil_test
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
      glEnable(GL_STENCIL_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_STENCIL_TEST)" << endl;
#endif
      glDisable(GL_STENCIL_TEST);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_blend
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
      glEnable(GL_BLEND);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
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
INLINE void CLP(GraphicsStateGuardian)::
enable_depth_test(bool val) {
  if (_depth_test_enabled != val) {
    _depth_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_DEPTH_TEST)" << endl;
#endif
      glEnable(GL_DEPTH_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_DEPTH_TEST)" << endl;
#endif
      glDisable(GL_DEPTH_TEST);
    }
  }
}

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_fog
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
      glEnable(GL_FOG);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_FOG)" << endl;
#endif
      glDisable(GL_FOG);
    }
  }
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_alpha_test
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
      glEnable(GL_ALPHA_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_ALPHA_TEST)" << endl;
#endif
      glDisable(GL_ALPHA_TEST);
    }
  }
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::enable_polygon_offset
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
      glEnable(GL_POLYGON_OFFSET_FILL);
      //glEnable(GL_POLYGON_OFFSET_LINE);  // not widely supported anyway
      //glEnable(GL_POLYGON_OFFSET_POINT);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_POLYGON_OFFSET_*)" << endl;
#endif
      glDisable(GL_POLYGON_OFFSET_FILL);
      //glDisable(GL_POLYGON_OFFSET_LINE);  // not widely supported anyway
      //glDisable(GL_POLYGON_OFFSET_POINT);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::set_color_write_mask
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
set_color_write_mask(int mask) {
  if (gl_color_mask && _active_color_write_mask != mask) {
    _active_color_write_mask = mask;
    glColorMask((_color_write_mask & ColorWriteAttrib::C_red) != 0,
                (_color_write_mask & ColorWriteAttrib::C_green) != 0,
                (_color_write_mask & ColorWriteAttrib::C_blue) != 0,
                (_color_write_mask & ColorWriteAttrib::C_alpha) != 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::clear_color_write_mask
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
clear_color_write_mask() {
  if (gl_color_mask && _active_color_write_mask != ColorWriteAttrib::C_all) {
    _active_color_write_mask = ColorWriteAttrib::C_all;
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  }
}

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glFogfv
//       Access: Public
//  Description: This method is necessary because there is no
//               glFogdv().
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glFogfv(GLenum pname, const LColor &color) {
#ifndef STDFLOAT_DOUBLE
  glFogfv(pname, color.get_data());
#else  // STDFLOAT_DOUBLE
  LColorf fcolor = LCAST(float, color);
  glFogfv(pname, fcolor.get_data());
#endif  //  STDFLOAT_DOUBLE
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glMaterialfv
//       Access: Public
//  Description: This method is necessary because there is no
//               glMaterialdv().
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glMaterialfv(GLenum face, GLenum pname, const LColor &color) {
#ifndef STDFLOAT_DOUBLE
  glMaterialfv(face, pname, color.get_data());
#else  // STDFLOAT_DOUBLE
  LColorf fcolor = LCAST(float, color);
  glMaterialfv(face, pname, fcolor.get_data());
#endif  //  STDFLOAT_DOUBLE
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glLightfv
//       Access: Public
//  Description: This method is necessary because there is no
//               glLightdv().
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glLightfv(GLenum light, GLenum pname, const LVecBase4 &value) {
#ifndef STDFLOAT_DOUBLE
  glLightfv(light, pname, value.get_data());
#else  // STDFLOAT_DOUBLE
  LVecBase4f fvalue = LCAST(float, value);
  glLightfv(light, pname, fvalue.get_data());
#endif  //  STDFLOAT_DOUBLE
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glLightfv
//       Access: Public
//  Description: This method is necessary because there is no
//               glLightdv().
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glLightfv(GLenum light, GLenum pname, const LVecBase3 &value) {
#ifndef STDFLOAT_DOUBLE
  glLightfv(light, pname, value.get_data());
#else  // STDFLOAT_DOUBLE
  LVecBase3f fvalue = LCAST(float, value);
  glLightfv(light, pname, fvalue.get_data());
#endif  //  STDFLOAT_DOUBLE
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glLightModelfv
//       Access: Public
//  Description: This method is necessary because there is no
//               glLightModeldv().
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glLightModelfv(GLenum pname, const LVecBase4 &value) {
#ifndef STDFLOAT_DOUBLE
  glLightModelfv(pname, value.get_data());
#else  // STDFLOAT_DOUBLE
  LVecBase4f fvalue = LCAST(float, value);
  glLightModelfv(pname, fvalue.get_data());
#endif  //  STDFLOAT_DOUBLE
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glTexEnvfv
//       Access: Public
//  Description: This method is necessary because there is no
//               glTexEnvdv().
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glTexEnvfv(GLenum target, GLenum pname, const LVecBase4 &value) {
#ifndef STDFLOAT_DOUBLE
  glTexEnvfv(target, pname, value.get_data());
#else  // STDFLOAT_DOUBLE
  LVecBase4f fvalue = LCAST(float, value);
  glTexEnvfv(target, pname, fvalue.get_data());
#endif  //  STDFLOAT_DOUBLE
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::call_glTexParameterfv
//       Access: Public
//  Description: This method is necessary because there is no
//               glTexParameterdv().
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
call_glTexParameterfv(GLenum target, GLenum pname, const LVecBase4 &value) {
#ifndef STDFLOAT_DOUBLE
  glTexParameterfv(target, pname, value.get_data());
#else  // STDFLOAT_DOUBLE
  LVecBase4f fvalue = LCAST(float, value);
  glTexParameterfv(target, pname, fvalue.get_data());
#endif  //  STDFLOAT_DOUBLE
}

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_light_id
//       Access: Public
//  Description: Convert index to gl light id
////////////////////////////////////////////////////////////////////
INLINE GLenum CLP(GraphicsStateGuardian)::
get_light_id(int index) const {
  return GL_LIGHT0 + index;
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_clip_plane_id
//       Access: Public
//  Description: Convert index to gl clip plane id
////////////////////////////////////////////////////////////////////
INLINE GLenum CLP(GraphicsStateGuardian)::
get_clip_plane_id(int index) const {
  return GL_CLIP_PLANE0 + index;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_supports_framebuffer_multisample
//       Access: Public
//  Description: Returns if this glGsg supports multisample
//               antialiasing for framebuffer objects.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
get_supports_framebuffer_multisample() {
  return _supports_framebuffer_multisample;
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_supports_framebuffer_multisample
//       Access: Public
//  Description: Returns if this glGsg supports multisample
//               antialiasing for framebuffer objects.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
get_supports_framebuffer_multisample_coverage_nv() {
  return _supports_framebuffer_multisample_coverage_nv;
}


////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::get_supports_framebuffer_blit
//       Access: Public
//  Description: Returns if this glGsg supports multisample
//               antialiasing for framebuffer objects.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
get_supports_framebuffer_blit() {
  return _supports_framebuffer_blit;
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::UsageTextureKey::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLP(GraphicsStateGuardian)::UsageTextureKey::
UsageTextureKey(int x_size, int y_size) :
  _x_size(x_size),
  _y_size(y_size)
{
}
#endif  // NDEBUG

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::UsageTextureKey::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::UsageTextureKey::
operator < (const CLP(GraphicsStateGuardian)::UsageTextureKey &other) const {
  if (_x_size != other._x_size) {
    return _x_size < other._x_size;
  }
  return _y_size < other._y_size;
}
#endif  // NDEBUG
