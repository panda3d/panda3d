// Filename: dxGraphicsStateGuardian.I
// Created by:  mike (02Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "config_dxgsg.h"

#include <graphicsWindow.h>

////////////////////////////////////////////////////////////////////
//     Function: DxGraphicsStateGuardian::activate
//       Access: Public
//  Description: Sets this context to be the active context for future
//               DX commands.
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
activate() {
//  _win->make_current();
}



#ifdef WBD_GL_MODE
////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glClearColor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
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
//     Function: DXGraphicsStateGuardian::call_glClearDepth
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glClearDepth(GLclampd depth) {
  if (depth != _clear_depth) {
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
      << "glClearDepth(" << (double)depth << ")" << endl;
#endif
    glClearDepth(depth);
    _clear_depth = depth;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glClearStencil
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glClearStencil(GLint s) {
  if (s != _clear_stencil) {
    glClearStencil(s);
    _clear_stencil = s;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glClearAccum
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
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
//     Function: DXGraphicsStateGuardian::call_glDrawBuffer
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glDrawBuffer(GLenum mode) {
  if (mode != _draw_buffer_mode) {
#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "glDrawBuffer(";
    switch (mode) {
    case GL_FRONT:
      dxgsg_cat.debug(false) << "GL_FRONT)"; 
      break;
    case GL_BACK:
      dxgsg_cat.debug(false) << "GL_BACK)"; 
      break;
    case GL_RIGHT:
      dxgsg_cat.debug(false) << "GL_RIGHT)"; 
      break;
    case GL_LEFT:
      dxgsg_cat.debug(false) << "GL_LEFT)"; 
      break;
    case GL_FRONT_RIGHT:
      dxgsg_cat.debug(false) << "GL_FRONT_RIGHT)"; 
      break;
    case GL_FRONT_LEFT:
      dxgsg_cat.debug(false) << "GL_FRONT_LEFT)"; 
      break;
    case GL_BACK_RIGHT:
      dxgsg_cat.debug(false) << "GL_BACK_RIGHT)"; 
      break;
    case GL_BACK_LEFT:
      dxgsg_cat.debug(false) << "GL_BACK_LEFT)"; 
      break;
    case GL_FRONT_AND_BACK:
      dxgsg_cat.debug(false) << "GL_FRONT_AND_BACK)"; 
      break;
    }
    dxgsg_cat.debug(false) << endl;
#endif
    glDrawBuffer(mode);
    _draw_buffer_mode = mode;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glReadBuffer
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glReadBuffer(GLenum mode) {
  if (mode != _read_buffer_mode) {
    glReadBuffer(mode);
    _read_buffer_mode = mode;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glShadeModel
//       Access:
//  Description: Set the shading model to be either GL_FLAT or GL_SMOOTH
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glShadeModel(GLenum mode) {
  if (_shade_model_mode != mode) {
    glShadeModel(mode);
    _shade_model_mode = mode;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glViewport
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
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
//     Function: DXGraphicsStateGuardian::call_glStencilFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glStencilFunc(GLenum func) {
  if (_stencil_func != func) {
    _stencil_func = func;
#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "glStencilFunc(";
    switch (func) {
    case GL_NEVER:
      dxgsg_cat.debug(false) << "GL_NEVER, "; 
      break;
    case GL_LESS:
      dxgsg_cat.debug(false) << "GL_LESS, "; 
      break;
    case GL_EQUAL:
      dxgsg_cat.debug(false) << "GL_EQUAL, "; 
      break;
    case GL_LEQUAL:
      dxgsg_cat.debug(false) << "GL_LEQUAL, "; 
      break;
    case GL_GREATER:
      dxgsg_cat.debug(false) << "GL_GREATER, "; 
      break;
    case GL_NOTEQUAL:
      dxgsg_cat.debug(false) << "GL_NOTEQUAL, "; 
      break;
    case GL_GEQUAL:
      dxgsg_cat.debug(false) << "GL_GEQUAL, "; 
      break;
    case GL_ALWAYS:
      dxgsg_cat.debug(false) << "GL_ALWAYS, "; 
      break;
    default:
      dxgsg_cat.debug(false) << "unknown, "; 
      break;
    }
    dxgsg_cat.debug(false) << "1, 1)" << endl;
#endif
    glStencilFunc(func, 1, 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glStencilOp
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glStencilOp(GLenum op) {
  if (_stencil_op != op) {
    _stencil_op = op;
#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "glStencilOp(GL_KEEP, GL_KEEP, ";
    switch (op) {
    case GL_KEEP:
      dxgsg_cat.debug(false) << "GL_KEEP)"; 
      break;
    case GL_ZERO:
      dxgsg_cat.debug(false) << "GL_ZERO)"; 
      break;
    case GL_REPLACE:
      dxgsg_cat.debug(false) << "GL_REPLACE)"; 
      break;
    case GL_INCR:
      dxgsg_cat.debug(false) << "GL_INCR)"; 
      break;
    case GL_DECR:
      dxgsg_cat.debug(false) << "GL_DECR)"; 
      break;
    case GL_INVERT:
      dxgsg_cat.debug(false) << "GL_INVERT)"; 
      break;
    default:
      dxgsg_cat.debug(false) << "unknown)"; 
      break;
    }
    dxgsg_cat.debug(false) << endl;
#endif
    glStencilOp(GL_KEEP, GL_KEEP, op);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glLineWidth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glLineWidth(GLfloat width) {
  if (_line_width != width) {
    _line_width = width;
#ifdef GSG_VERBOSE
  dxgsg_cat.debug()
    << "glLineWidth(" << width << ")" << endl;
#endif
    glLineWidth(width);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glPointSize
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glPointSize(GLfloat size) {
  if (_point_size != size) {
    _point_size = size;
#ifdef GSG_VERBOSE
  dxgsg_cat.debug()
    << "glPointSize(" << size << ")" << endl;
#endif
    glPointSize(size);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glFogMode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glFogMode(GLint mode) {
  if (_fog_mode != mode) {
    _fog_mode = mode;
#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "glFog(GL_FOG_MODE, ";
    switch(mode) {
    case GL_LINEAR:
      dxgsg_cat.debug(false) << "GL_LINEAR)" << endl; 
      break;
    case GL_EXP:
      dxgsg_cat.debug(false) << "GL_EXP)" << endl; 
      break;
    case GL_EXP2:
      dxgsg_cat.debug(false) << "GL_EXP2)" << endl; 
      break;
#ifdef GL_FOG_FUNC_SGIS
    case GL_FOG_FUNC_SGIS:
      dxgsg_cat.debug(false) << "GL_FOG_FUNC_SGIS)" << endl; 
      break;
#endif
    default:
      dxgsg_cat.debug(false) << "unknown)" << endl; 
      break;
    }
#endif
    glFogi(GL_FOG_MODE, mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glFogStart
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glFogStart(GLfloat start) {
  if (_fog_start != start) {
    _fog_start = start;
#ifdef GSG_VERBOSE
  dxgsg_cat.debug()
    << "glFog(GL_FOG_START, " << start << ")" << endl;
#endif
    glFogf(GL_FOG_START, start);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glFogEnd
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glFogEnd(GLfloat end) {
  if (_fog_end != end) {
    _fog_end = end;
#ifdef GSG_VERBOSE
  dxgsg_cat.debug()
    << "glFog(GL_FOG_END, " << end << ")" << endl;
#endif
    glFogf(GL_FOG_END, end);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glFogDensity
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glFogDensity(GLfloat density) {
  if (_fog_density != density) {
    _fog_density = density;
#ifdef GSG_VERBOSE
  dxgsg_cat.debug()
    << "glFog(GL_FOG_DENSITY, " << density << ")" << endl;
#endif
    glFogf(GL_FOG_DENSITY, density);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glFogColor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glFogColor(const Colorf &color) {
  if (_fog_color != color) {
    _fog_color = color;
#ifdef GSG_VERBOSE
  dxgsg_cat.debug()
    << "glFog(GL_FOG_COLOR, " << color << ")" << endl;
#endif
    glFogfv(GL_FOG_COLOR, color.get_data());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glAlphaFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_glAlphaFunc(GLenum func, GLclampf ref) {
  if (_alpha_func != func || _alpha_func_ref != ref) {
    _alpha_func = func;
    _alpha_func_ref = ref;
#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "glAlphaFunc(";
    switch (func) {
    case GL_NEVER:
      dxgsg_cat.debug(false) << "GL_NEVER, "; 
      break;
    case GL_LESS:
      dxgsg_cat.debug(false) << "GL_LESS, "; 
      break;
    case GL_EQUAL:
      dxgsg_cat.debug(false) << "GL_EQUAL, "; 
      break;
    case GL_LEQUAL:
      dxgsg_cat.debug(false) << "GL_LEQUAL, "; 
      break;
    case GL_GREATER:
      dxgsg_cat.debug(false) << "GL_GREATER, "; 
      break;
    case GL_NOTEQUAL:
      dxgsg_cat.debug(false) << "GL_NOTEQUAL, "; 
      break;
    case GL_GEQUAL:
      dxgsg_cat.debug(false) << "GL_GEQUAL, "; 
      break;
    case GL_ALWAYS:
      dxgsg_cat.debug(false) << "GL_ALWAYS, "; 
      break;
    }
    dxgsg_cat.debug() << ref << ")" << endl;
#endif
    glAlphaFunc(func, ref); 
  }
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_light_id
//       Access: Public
//  Description: Convert index to gl light id
////////////////////////////////////////////////////////////////////
INLINE GLenum DXGraphicsStateGuardian::get_light_id(int index) const
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
            dxgsg_cat.error()
	      << "get_light_id() - we don't currently support ids "
	      << "> 8" << endl;
            break;
    }
    return GL_LIGHT0;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_clip_plane_id
//       Access: Public
//  Description: Convert index to gl clip plane id
////////////////////////////////////////////////////////////////////
INLINE GLenum DXGraphicsStateGuardian::
get_clip_plane_id(int index) const {
  switch(index) {
    case 0: return GL_CLIP_PLANE0;
    case 1: return GL_CLIP_PLANE1;
    case 2: return GL_CLIP_PLANE2;
    case 3: return GL_CLIP_PLANE3;
    case 4: return GL_CLIP_PLANE4;
    case 5: return GL_CLIP_PLANE5;
    default:
      dxgsg_cat.error()
	<< "get_clip_plane_id() - we don't currently support ids "
	<< "> 5" << endl;
      break;
  }
  return GL_CLIP_PLANE0;
}


#endif		// WBD_GL_MODE

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::set_pack_alignment
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
set_pack_alignment(int alignment) {
  if (_pack_alignment != alignment) {
//    glPixelStorei(GL_PACK_ALIGNMENT, alignment);
    _pack_alignment = alignment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::set_unpack_alignment
//       Access:
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
set_unpack_alignment(int alignment) {
  if (_unpack_alignment != alignment) {   
//    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    _unpack_alignment = alignment;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_multisample
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_multisample(bool val) {
  if (_multisample_enabled != val) {   
    _multisample_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_line_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_line_smooth(bool val) {
  if (_line_smooth_enabled != val) {   
    _line_smooth_enabled = val;
 	_d3dDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_point_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_point_smooth(bool val) {
  if (_point_smooth_enabled != val) {   
    _point_smooth_enabled = val;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_lighting
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_lighting(bool val) {
  if (_lighting_enabled != val) {   
    _lighting_enabled = val;
 	_d3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_dither
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_dither(bool val) {
  if (_dither_enabled != val) {
    _dither_enabled = val;
	_d3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_stencil_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_stencil_test(bool val) {
  if (_stencil_test_enabled != val) {
    _stencil_test_enabled = val;
	_d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_color_material
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_color_material(bool val) {
  if (_color_material_enabled != val) {
    _color_material_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_clip_plane
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_clip_plane(int clip_plane, bool val) 
{
  if (_clip_plane_enabled[clip_plane] != val) 
	{
    _clip_plane_enabled[clip_plane] = val;
	DWORD  ClipPlaneBits;
	_d3dDevice->GetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE , &ClipPlaneBits);
	if (val)
		ClipPlaneBits |= 1 << clip_plane;
    else 
		ClipPlaneBits &= ~(1 << clip_plane);
	_d3dDevice->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE , ClipPlaneBits);
	}
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_multisample_alpha_one
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_multisample_alpha_one(bool val) {
  if (_multisample_alpha_one_enabled != val) {
    _multisample_alpha_one_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_multisample_alpha_mask
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_multisample_alpha_mask(bool val) {
  if (_multisample_alpha_mask_enabled != val) {
    _multisample_alpha_mask_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
	_d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_fog
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_fog(bool val) {
  if (_fog_enabled != val) {
    _fog_enabled = val;
	_d3dDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, (DWORD)val);
  }
}




////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_alpha_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_alpha_test(bool val ) 
{
  if (_alpha_test_enabled != val) {
    _alpha_test_enabled = val;
 	_d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, (DWORD)val);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_dxLightModelAmbient
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_dxLightModelAmbient( const Colorf& color) 
{
  if (_lmodel_ambient != color) {
    _lmodel_ambient = color;
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
      << "glLightModel(GL_LIGHT_MODEL_AMBIENT, " << color << ")" << endl;
#endif
    _d3dDevice->SetRenderState( D3DRENDERSTATE_AMBIENT, 
 				D3DRGBA(color[0], color[1], color[2], color[3]));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::call_glAlphaFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
call_dxAlphaFunc(D3DCMPFUNC func, DWORD ref) 
{
  if (_alpha_func != func || _alpha_func_ref != ref) {
    _alpha_func = func;
    _alpha_func_ref = ref;
#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "glAlphaFunc(";
    switch (func) {
    case D3DCMP_NEVER:
      dxgsg_cat.debug(false) << "D3DCMP_NEVER, "; 
      break;
    case D3DCMP_LESS:
      dxgsg_cat.debug(false) << "D3DCMP_LESS, "; 
      break;
    case D3DCMP_EQUAL:
      dxgsg_cat.debug(false) << "D3DCMP_EQUAL, "; 
      break;
    case D3DCMP_LEQUAL:
      dxgsg_cat.debug(false) << "D3DCMP_LEQUAL, "; 
      break;
    case D3DCMP_GREATER:
      dxgsg_cat.debug(false) << "D3DCMP_GREATER, "; 
      break;
    case D3DCMP_NOTEQUAL:
      dxgsg_cat.debug(false) << "D3DCMP_NOTEQUAL, "; 
      break;
    case D3DCMP_GEQUAL:
      dxgsg_cat.debug(false) << "D3DCMP_GEQUAL, "; 
      break;
    case D3DCMP_ALWAYS:
      dxgsg_cat.debug(false) << "D3DCMP_ALWAYS, "; 
      break;
    }
    dxgsg_cat.debug() << ref << ")" << endl;
#endif
	_d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, func);
	_d3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, ref);
  }
}


INLINE void DXGraphicsStateGuardian::
call_dxBlendFunc(D3DBLEND sfunc, D3DBLEND dfunc ) 
{
  if (_blend_source_func != sfunc)
	{
    _blend_source_func = sfunc;
 	_d3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, sfunc);
#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "dxSrcBlendFunc(";
    switch (sfunc) 
		{
	    case D3DBLEND_ZERO:
	      dxgsg_cat.debug(false) << "ZERO, "; 
	      break;
	    case D3DBLEND_ONE:
	      dxgsg_cat.debug(false) << "ONE, "; 
	      break;
	    case D3DBLEND_DESTCOLOR:
	      dxgsg_cat.debug(false) << "DESTCOLOR, "; 
	      break;
	    case D3DBLEND_INVDESTCOLOR:
	      dxgsg_cat.debug(false) << "INVDESTCOLOR, "; 
	      break;
	    case D3DBLEND_SRCALPHA:
	      dxgsg_cat.debug(false) << "SRCALPHA, "; 
	      break;
	    case D3DBLEND_INVSRCALPHA:
	      dxgsg_cat.debug(false) << "INVSRCALPHA, "; 
	      break;
	    case D3DBLEND_DESTALPHA:
	      dxgsg_cat.debug(false) << "DESTALPHA, "; 
	      break;
	    case D3DBLEND_INVDESTALPHA:
	      dxgsg_cat.debug(false) << "INVDESTALPHA, "; 
	      break;
	    case D3DBLEND_SRCALPHASAT:
	      dxgsg_cat.debug(false) << "SRCALPHASAT, "; 
	      break;
	    default:
	      dxgsg_cat.debug(false) << "unknown, "; 
	      break;
	    }
    dxgsg_cat.debug(false) << endl;
#endif
	}
  if ( _blend_dest_func != dfunc) 
	{
    _blend_dest_func = dfunc;
 	_d3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, dfunc);
#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "dxDstBlendFunc(";
    switch (dfunc) 
		{
	    case D3DBLEND_ZERO:
	      dxgsg_cat.debug(false) << "ZERO, "; 
	      break;
	    case D3DBLEND_ONE:
	      dxgsg_cat.debug(false) << "ONE, "; 
	      break;
	    case D3DBLEND_DESTCOLOR:
	      dxgsg_cat.debug(false) << "DESTCOLOR, "; 
	      break;
	    case D3DBLEND_INVDESTCOLOR:
	      dxgsg_cat.debug(false) << "INVDESTCOLOR, "; 
	      break;
	    case D3DBLEND_SRCALPHA:
	      dxgsg_cat.debug(false) << "SRCALPHA, "; 
	      break;
	    case D3DBLEND_INVSRCALPHA:
	      dxgsg_cat.debug(false) << "INVSRCALPHA, "; 
	      break;
	    case D3DBLEND_DESTALPHA:
	      dxgsg_cat.debug(false) << "DESTALPHA, "; 
	      break;
	    case D3DBLEND_INVDESTALPHA:
	      dxgsg_cat.debug(false) << "INVDESTALPHA, "; 
	      break;
	    case D3DBLEND_SRCALPHASAT:
	      dxgsg_cat.debug(false) << "SRCALPHASAT, "; 
	      break;
	    }
    dxgsg_cat.debug(false) << endl;
#endif
	}
}

