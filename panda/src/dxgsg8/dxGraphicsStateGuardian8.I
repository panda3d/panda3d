// Filename: dxGraphicsStateGuardian.I
// Created by:  mike (02Feb99)
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

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_line_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_line_smooth(bool val) {
  if(_line_smooth_enabled != val) {
    _line_smooth_enabled = val;
  #ifdef NDEBUG
    {
        if(val && (scrn.d3dcaps.RasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES))
           dxgsg_cat.error() << "no HW support for line smoothing!!\n";
    }
  #endif

    scrn.pD3DDevice->SetRenderState(D3DRS_EDGEANTIALIAS, (DWORD)val);
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
    scrn.pD3DDevice->SetRenderState(D3DRS_LIGHTING, (DWORD)val);
    if(_lighting_enabled = val)
      _lighting_enabled_this_frame = true;
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

  #ifdef _DEBUG
    {
        if(val && !(scrn.d3dcaps.RasterCaps & D3DPRASTERCAPS_DITHER))
           dxgsg_cat.error() << "no HW support for color dithering!!\n";
        return;
    }
  #endif

    _dither_enabled = val;
    scrn.pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, (DWORD)val);
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
    scrn.pD3DDevice->SetRenderState(D3DRS_STENCILENABLE, (DWORD)val);
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
enable_clip_plane(int clip_plane_id, bool val)
{
  assert(clip_plane_id < D3DMAXUSERCLIPPLANES);

  if (_clip_plane_enabled[clip_plane_id] != val) {
    DWORD bitflag = 1 << clip_plane_id;

    _clip_plane_enabled[clip_plane_id] = val;
    DWORD  ClipPlaneBits;
    scrn.pD3DDevice->GetRenderState(D3DRS_CLIPPLANEENABLE , &ClipPlaneBits);
    if (val)
        ClipPlaneBits |= bitflag;
    else
        ClipPlaneBits &= ~bitflag;
    scrn.pD3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE , ClipPlaneBits);
  }
}

#if 0
////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_clip_plane
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_clip_plane(int clip_plane_id, bool val)
{
  assert(clip_plane_id < D3DMAXUSERCLIPPLANES);

  // clip plane stuff needs total rewriting to get rid of these arrays
//  if (((_clip_planes_enabled & bitflag)!=0) != val) 
  if (_clip_plane_enabled[clip_plane_id] != val) {
    _clip_plane_enabled[clip_plane_id] = val;
    if(val)
      _clip_plane_enabled |= bitflag;
     else _clip_plane_enabled &= ~bitflag;

    scrn.pD3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE , _clip_planes_enabled);
  }
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
    scrn.pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_gouraud_shading(bool val) {
  if (_bGouraudShadingOn != val) {
    _bGouraudShadingOn = val;
    scrn.pD3DDevice->SetRenderState(D3DRS_SHADEMODE, (val ? D3DSHADE_GOURAUD : D3DSHADE_FLAT));
  }
}

INLINE void DXGraphicsStateGuardian::
enable_primitive_clipping(bool val) {
  if (_clipping_enabled != val) {
    _clipping_enabled = val;
    scrn.pD3DDevice->SetRenderState(D3DRS_CLIPPING, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_fog
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_fog(bool val) {
  if ((_fog_enabled != val) && (_doFogType!=None)) {
    _fog_enabled = val;
    scrn.pD3DDevice->SetRenderState(D3DRS_FOGENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::set_vertex_format
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
set_vertex_format(DWORD NewFvfType) {
  if (_CurFVFType != NewFvfType) {
    _CurFVFType = NewFvfType;
    HRESULT hr = scrn.pD3DDevice->SetVertexShader(NewFvfType);
 #ifndef NDEBUG
    if(FAILED(hr)) {
           dxgsg_cat.error() << "SetVertexShader(0x" << (void*)NewFvfType<<") failed, hr = " << D3DERRORSTRING(hr);
           exit(1);
    }
 #endif
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
    scrn.pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)val);
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
    dxgsg_cat.debug() << "dxLightModel(LIGHT_MODEL_AMBIENT, " << color << ")" << endl;
#endif
    scrn.pD3DDevice->SetRenderState( D3DRS_AMBIENT,
                D3DCOLOR_COLORVALUE(color[0], color[1], color[2], color[3]));
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
    dxgsg_cat.debug() << "dxAlphaFunc(";
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
    scrn.pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, func);
    scrn.pD3DDevice->SetRenderState(D3DRS_ALPHAREF, ref);
  }
}


INLINE void DXGraphicsStateGuardian::
call_dxBlendFunc(D3DBLEND sfunc, D3DBLEND dfunc )
{
  if (_blend_source_func != sfunc)
    {
    _blend_source_func = sfunc;
    scrn.pD3DDevice->SetRenderState(D3DRS_SRCBLEND, sfunc);
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
    scrn.pD3DDevice->SetRenderState(D3DRS_DESTBLEND, dfunc);
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

INLINE void DXGraphicsStateGuardian::
enable_zwritemask(bool val) {
    if (_depth_write_enabled != val) {
        _depth_write_enabled = val;
        scrn.pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, val);
    }
}

/**  unimplemented

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_multisample
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_multisample(bool val) {
  _multisample_enabled = val;
  #ifdef NDEBUG
    dxgsg_cat.error() << "dx multisample unimplemented!!\n";
  #endif
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
//     Function: DXGraphicsStateGuardian::enable_point_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_point_smooth(bool val) {
  // _point_smooth_enabled = val;

  #ifdef NDEBUG
    dxgsg_cat.error() << "dx point smoothing unimplemented!!\n";
  #endif
}
*/

