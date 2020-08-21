/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glGraphicsStateGuardian_src.I
 * @author drose
 * @date 1999-02-02
 */

/**
 * If debug markers are enabled, pushes the beginning of a group marker.
 */
INLINE void CLP(GraphicsStateGuardian)::
push_group_marker(const std::string &marker) {
#if !defined(NDEBUG) && !defined(OPENGLES_1)
  if (_glPushGroupMarker != nullptr) {
    _glPushGroupMarker(marker.size(), marker.data());
  }
#endif
}

/**
 * If debug markers are enabled, closes a group debug marker.
 */
INLINE void CLP(GraphicsStateGuardian)::
pop_group_marker() {
#if !defined(NDEBUG) && !defined(OPENGLES_1)
  if (_glPopGroupMarker != nullptr) {
    _glPopGroupMarker();
  }
#endif
}

/**
 * Checks for any outstanding error codes and outputs them, if found.  If
 * NDEBUG is defined, this function does nothing.  The return value is true if
 * everything is ok, or false if we should shut down.
 *
 * This is a static method so it can be called when there's no gsg pointer
 * around.
 */
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

/**
 * Like report_errors(), above, but non-static so we can throw an event on
 * failure.
 */
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

/**
 * This works like report_errors(), except that it always runs, even in the
 * NDEBUG case.
 *
 * It is designed to be called when it is important to clear the error stack
 * (for instance, because we want to be able to reliably check the result of
 * some upcoming GL operation).
 */
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

/**
 * This works like report_my_errors(), except that it always runs, even in the
 * NDEBUG case (but not when _check_errors is false), and it never calls
 * panic_deactivate().  It is designed to be called when it is important to
 * clear the error stack (for instance, because we want to be able to reliably
 * check the result of some upcoming GL operation).
 */
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

/**
 * Returns the GL vendor string reported by the driver.
 */
INLINE const std::string &CLP(GraphicsStateGuardian)::
get_gl_vendor() const {
  return _gl_vendor;
}

/**
 * Returns the GL renderer string reported by the driver.
 */
INLINE const std::string &CLP(GraphicsStateGuardian)::
get_gl_renderer() const {
  return _gl_renderer;
}

/**
 * Returns the GL version string reported by the driver.
 */
INLINE const std::string &CLP(GraphicsStateGuardian)::
get_gl_version() const {
  return _gl_version;
}

/**
 * Returns the major part of the reported GL version number.
 */
INLINE int CLP(GraphicsStateGuardian)::
get_gl_version_major() const {
  return _gl_version_major;
}

/**
 * Returns the minor part of the reported GL version number.
 */
INLINE int CLP(GraphicsStateGuardian)::
get_gl_version_minor() const {
  return _gl_version_minor;
}

/**
 * Returns whether a core profile or a compatibility mode is considered.
 */
/*INLINE bool CLP(GraphicsStateGuardian)::
has_core_profile() const {
  return _core_profile;
}*/

/**
 * Returns whether the fixed function pipeline is supported.
 */
INLINE bool CLP(GraphicsStateGuardian)::
has_fixed_function_pipeline() const {
#ifndef SUPPORT_FIXED_FUNCTION
  return false;
#elif defined(OPENGLES_1)
  return true;
#elif defined(OPENGLES)
  return false;
#else
  // Otherwise, we can just check whether we are using a core profile or a
  // compatibility mode. The variable _core_profile is already taking into
  // account if a GL < 3.2 is considered (becoming false)
  return !_core_profile;
#endif
}

/**
 * Calls glFinish() if the config variable gl-finish is set True.
 */
INLINE void CLP(GraphicsStateGuardian)::
maybe_gl_finish() const {
#ifdef DO_PSTATS
  if (gl_finish) {
    glFinish();
  }
#endif
}

/**
 * Returns true if the indicated extension is reported by the GL system, false
 * otherwise.  The extension name is case-sensitive.
 */
INLINE bool CLP(GraphicsStateGuardian)::
has_extension(const std::string &extension) const {
  bool has_ext = (_extensions.find(extension) != _extensions.end());
#ifndef NDEBUG
  if (GLCAT.is_debug()) {
    GLCAT.debug()
      << "HAS EXT " << extension << " " << has_ext << "\n";
  }
#endif
  return has_ext;
}

/**
 * Returns true if we are compiled for mainline OpenGL, and the runtime GL
 * version number is at least the indicated value, false otherwise.  Under
 * OpenGL ES, this always returns false.
 */
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

/**
 * Returns true if we are compiled for OpenGL ES, and the runtime GL ES
 * version number is at least the indicated value, false otherwise.  Under
 * mainline OpenGL, this always returns false.
 */
INLINE bool CLP(GraphicsStateGuardian)::
is_at_least_gles_version(int major_version, int minor_version) const {
#ifndef OPENGLES
  return false;
#elif defined(OPENGLES_1)
  return major_version == 1 && _gl_version_minor >= minor_version;
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

#ifndef OPENGLES_1
/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
enable_vertex_attrib_array(GLuint index) {
  if (!_enabled_vertex_attrib_arrays.get_bit(index)) {
    _glEnableVertexAttribArray(index);
    _enabled_vertex_attrib_arrays.set_bit(index);
  }
}

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
disable_vertex_attrib_array(GLuint index) {
  if (_enabled_vertex_attrib_arrays.get_bit(index)) {
    _glDisableVertexAttribArray(index);
    _enabled_vertex_attrib_arrays.clear_bit(index);

    if (_vertex_attrib_divisors[index] != 0) {
      _glVertexAttribDivisor(index, 0);
      _vertex_attrib_divisors[index] = 0;
    }
  }
}

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
set_vertex_attrib_divisor(GLuint index, GLuint divisor) {
  if (_supports_vertex_attrib_divisor &&
      _vertex_attrib_divisors[index] != divisor) {
    _glVertexAttribDivisor(index, divisor);
    _vertex_attrib_divisors[index] = divisor;
  }
}

#endif

/**
 * Calls glActiveTexture.
 */
INLINE void CLP(GraphicsStateGuardian)::
set_active_texture_stage(int i) {
  if (i != _active_texture_stage) {
#ifdef OPENGLES_2
    glActiveTexture(GL_TEXTURE0 + i);
#else
    _glActiveTexture(GL_TEXTURE0 + i);
#endif
    _active_texture_stage = i;
  }
}

/**
 * Specifies whether multisample should be enabled for antialiasing purposes.
 */
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

/**
 * Specifies whether multisample should be enabled for transparency purposes,
 * using the sample_alpha_to_one mode.
 */
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

/**
 * Specifies whether multisample should be enabled for transparency purposes,
 * using the sample_alpha_to_mask mode.
 */
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

/**
 *
 */
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

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
enable_point_smooth(bool val) {
#ifdef SUPPORT_FIXED_FUNCTION
  if (has_fixed_function_pipeline() && _point_smooth_enabled != val) {
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

/**
 *
 */
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

/**
 * Sets the appropriate antialiasing modes to render a series of line
 * primitives, according to _auto_antialias_mode.
 */
INLINE void CLP(GraphicsStateGuardian)::
setup_antialias_line() {
  if (_auto_antialias_mode) {
    // Lines supposedly look better using line smoothing, even if we have
    // multisample available.
    enable_multisample_antialias(false);
    enable_line_smooth(true);
  }
}

/**
 * Sets the appropriate antialiasing modes to render a series of point
 * primitives, according to _auto_antialias_mode.
 */
INLINE void CLP(GraphicsStateGuardian)::
setup_antialias_point() {
  if (_auto_antialias_mode) {
    // Points supposedly look better using point smoothing, even if we have
    // multisample available.
    enable_multisample_antialias(false);
    enable_point_smooth(true);
  }
}

/**
 * Sets the appropriate antialiasing modes to render a series of polygon
 * primitives, according to _auto_antialias_mode.
 */
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

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
enable_stencil_test(bool val) {
  if (_stencil_test_enabled != val) {
    _stencil_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_STENCIL_TEST)" << std::endl;
#endif
      glEnable(GL_STENCIL_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_STENCIL_TEST)" << std::endl;
#endif
      glDisable(GL_STENCIL_TEST);
    }
  }
}

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_BLEND)" << std::endl;
#endif
      glEnable(GL_BLEND);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_BLEND)" << std::endl;
#endif
      glDisable(GL_BLEND);
    }
  }
}

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
enable_depth_test(bool val) {
  if (_depth_test_enabled != val) {
    _depth_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_DEPTH_TEST)" << std::endl;
#endif
      glEnable(GL_DEPTH_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_DEPTH_TEST)" << std::endl;
#endif
      glDisable(GL_DEPTH_TEST);
    }
  }
}

#ifdef SUPPORT_FIXED_FUNCTION
/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
enable_fog(bool val) {
  if (_fog_enabled != val) {
    _fog_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_FOG)" << std::endl;
#endif
      glEnable(GL_FOG);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_FOG)" << std::endl;
#endif
      glDisable(GL_FOG);
    }
  }
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
enable_alpha_test(bool val) {
  if (_alpha_test_enabled != val) {
    _alpha_test_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_ALPHA_TEST)" << std::endl;
#endif
      glEnable(GL_ALPHA_TEST);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_ALPHA_TEST)" << std::endl;
#endif
      glDisable(GL_ALPHA_TEST);
    }
  }
}
#endif

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
enable_polygon_offset(bool val) {
  if (_polygon_offset_enabled != val) {
    _polygon_offset_enabled = val;
    if (val) {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glEnable(GL_POLYGON_OFFSET_*)" << std::endl;
#endif
      glEnable(GL_POLYGON_OFFSET_FILL);
      // glEnable(GL_POLYGON_OFFSET_LINE);   not widely supported anyway
      // glEnable(GL_POLYGON_OFFSET_POINT);
    } else {
#ifdef GSG_VERBOSE
      GLCAT.spam()
        << "glDisable(GL_POLYGON_OFFSET_*)" << std::endl;
#endif
      glDisable(GL_POLYGON_OFFSET_FILL);
      // glDisable(GL_POLYGON_OFFSET_LINE);   not widely supported anyway
      // glDisable(GL_POLYGON_OFFSET_POINT);
    }
  }
}

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
set_color_write_mask(int mask) {
  if (gl_color_mask && _active_color_write_mask != mask) {
    _active_color_write_mask = mask;
    glColorMask((mask & ColorWriteAttrib::C_red) != 0,
                (mask & ColorWriteAttrib::C_green) != 0,
                (mask & ColorWriteAttrib::C_blue) != 0,
                (mask & ColorWriteAttrib::C_alpha) != 0);
  }
}

/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
clear_color_write_mask() {
  if (gl_color_mask && _active_color_write_mask != ColorWriteAttrib::C_all) {
    _active_color_write_mask = ColorWriteAttrib::C_all;
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  }
}

#ifdef SUPPORT_FIXED_FUNCTION
/**
 *
 */
INLINE void CLP(GraphicsStateGuardian)::
call_glLoadMatrix(const LMatrix4 &mat) {
#if defined(OPENGLES) && defined(STDFLOAT_DOUBLE)
  LMatrix4f matf = LCAST(float, mat);
  glLoadMatrixf(matf.get_data());
#elif defined(STDFLOAT_DOUBLE)
  glLoadMatrixd(mat.get_data());
#else
  glLoadMatrixf(mat.get_data());
#endif
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
/**
 * This method is necessary because there is no glFogdv().
 */
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
/**
 * This method is necessary because there is no glMaterialdv().
 */
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
/**
 * This method is necessary because there is no glLightdv().
 */
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
/**
 * This method is necessary because there is no glLightdv().
 */
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
/**
 * This method is necessary because there is no glLightModeldv().
 */
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
/**
 * This method is necessary because there is no glTexEnvdv().
 */
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

/**
 * This method is necessary because there is no glTexParameterdv().
 */
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
/**
 * Convert index to gl light id
 */
INLINE GLenum CLP(GraphicsStateGuardian)::
get_light_id(int index) const {
  return GL_LIGHT0 + index;
}
#endif

#ifdef SUPPORT_FIXED_FUNCTION
/**
 * Convert index to gl clip plane id
 */
INLINE GLenum CLP(GraphicsStateGuardian)::
get_clip_plane_id(int index) const {
  return GL_CLIP_PLANE0 + index;
}
#endif

/**
 * Returns if this glGsg supports multisample antialiasing for framebuffer
 * objects.
 */
INLINE bool CLP(GraphicsStateGuardian)::
get_supports_framebuffer_multisample() {
  return _supports_framebuffer_multisample;
}

/**
 * Returns if this glGsg supports multisample antialiasing for framebuffer
 * objects.
 */
INLINE bool CLP(GraphicsStateGuardian)::
get_supports_framebuffer_multisample_coverage_nv() {
  return _supports_framebuffer_multisample_coverage_nv;
}


/**
 * Returns if this glGsg supports multisample antialiasing for framebuffer
 * objects.
 */
INLINE bool CLP(GraphicsStateGuardian)::
get_supports_framebuffer_blit() {
  return _supports_framebuffer_blit;
}

#ifndef NDEBUG
/**
 *
 */
INLINE CLP(GraphicsStateGuardian)::UsageTextureKey::
UsageTextureKey(int x_size, int y_size) :
  _x_size(x_size),
  _y_size(y_size)
{
}
#endif  // NDEBUG

#ifndef NDEBUG
/**
 *
 */
INLINE bool CLP(GraphicsStateGuardian)::UsageTextureKey::
operator < (const CLP(GraphicsStateGuardian)::UsageTextureKey &other) const {
  if (_x_size != other._x_size) {
    return _x_size < other._x_size;
  }
  return _y_size < other._y_size;
}
#endif  // NDEBUG
