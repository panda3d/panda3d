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
//     Function: CLP(GraphicsStateGuardian)::draw_display_list
//       Access: Public
//  Description: If the GeomContext is non-NULL and contains a valid
//               display list, uses it to draw the geometry if
//               appropriate, and returns true.  If the display list
//               is absent or cannot be used for some reason, does
//               nothing and returns false.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(GraphicsStateGuardian)::
draw_display_list(GeomContext *gc) {
  if (gc != (GeomContext *)NULL && _vertex_colors_enabled) {
    _draw_primitive_pcollector.start();
    CLP(GeomContext) *ggc = DCAST(CLP(GeomContext), gc);
    GLP(CallList)(ggc->_index);
#ifdef DO_PSTATS
    _vertices_display_list_pcollector.add_level(ggc->_num_verts);
#endif
    _draw_primitive_pcollector.stop();
    return true;
  }
  return false;
}

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
//     Function: CLP(GraphicsStateGuardian)::enable_multisample
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_multisample(bool val) {
  if (_multisample_enabled != val && _supports_multisample) {
    _multisample_enabled = val;
    if (val) {
      GLP(Enable)(GL_MULTISAMPLE);
    } else {
      if (!_multisample_alpha_one_enabled && !_multisample_alpha_mask_enabled) {
        GLP(Disable)(GL_MULTISAMPLE);
      }
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
    } else {
      GLP(Disable)(GL_POINT_SMOOTH);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::enable_polygon_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
enable_polygon_smooth(bool val) {
  if (_polygon_smooth_enabled != val) {
    _polygon_smooth_enabled = val;
    if (val) {
      GLP(Enable)(GL_POLYGON_SMOOTH);
    } else {
      GLP(Disable)(GL_POLYGON_SMOOTH);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::setup_antialias_line
//       Access: Protected
//  Description: Sets the appropriate antialiasing modes to render a
//               series of line primitives, according to
//               _antialias_mode.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
setup_antialias_line() {
  if (_antialias_mode == (unsigned short)AntialiasAttrib::M_best) {
    // Lines supposedly look better using line smoothing, even if we
    // have multisample available.
    enable_multisample(false);
    enable_line_smooth(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::setup_antialias_point
//       Access: Protected
//  Description: Sets the appropriate antialiasing modes to render a
//               series of point primitives, according to
//               _antialias_mode.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
setup_antialias_point() {
  if (_antialias_mode == (unsigned short)AntialiasAttrib::M_best) {
    // Points supposedly look better using point smoothing, even if we
    // have multisample available.
    enable_multisample(false);
    enable_point_smooth(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GraphicsStateGuardian)::setup_antialias_polygon
//       Access: Protected
//  Description: Sets the appropriate antialiasing modes to render a
//               series of point primitives, according to
//               _antialias_mode.
////////////////////////////////////////////////////////////////////
INLINE void CLP(GraphicsStateGuardian)::
setup_antialias_polygon() {
  if (_antialias_mode == (unsigned short)AntialiasAttrib::M_best) {
    if (_render_mode == RenderModeAttrib::M_wireframe) {
      // In wireframe mode, we're really drawing lines.
      enable_multisample(false);
      enable_line_smooth(true);

    } else {
      // For polygons, multisample is best if it's available, otherwise
      // polygon smoothing will do.
      enable_multisample(true);
      enable_polygon_smooth(true);
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
  if (_supports_multisample) {
    if (_multisample_alpha_one_enabled != val) {
      _multisample_alpha_one_enabled = val;
      if (val) {
        GLP(Enable)(GL_SAMPLE_ALPHA_TO_ONE);
        GLP(Enable)(GL_MULTISAMPLE);
      } else {
        GLP(Disable)(GL_SAMPLE_ALPHA_TO_ONE);
        if (!_multisample_enabled) {
          GLP(Disable)(GL_MULTISAMPLE);
        }
      }
    }
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
    if (val) {
      GLP(Enable)(GL_SAMPLE_ALPHA_TO_COVERAGE);
      GLP(Enable)(GL_MULTISAMPLE);
    } else {
      GLP(Disable)(GL_SAMPLE_ALPHA_TO_COVERAGE);
      if (!_multisample_enabled) {
        GLP(Disable)(GL_MULTISAMPLE);
      }
    }
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
