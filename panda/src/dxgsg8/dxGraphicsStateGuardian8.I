// Filename: dxGraphicsStateGuardian8.I
// Created by:  mike (02Feb99)
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
//     Function: DXGraphicsStateGuardian8::enable_line_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_line_smooth(bool val) {
  if(_line_smooth_enabled != val) {
    _line_smooth_enabled = val;
  #ifndef NDEBUG
    {
        if(val && (_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES))
           dxgsg8_cat.error() << "no HW support for line smoothing!!\n";
    }
  #endif

    _pD3DDevice->SetRenderState(D3DRS_EDGEANTIALIAS, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_dither
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_dither(bool val) {
  if (_dither_enabled != val) {
    _dither_enabled = val;

  #ifndef NDEBUG
    {
        if(val && !(_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_DITHER))
           dxgsg8_cat.error() << "no HW support for color dithering!!\n";
    }
  #endif

    _pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_stencil_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_stencil_test(bool val) {
  if (_stencil_test_enabled != val) {
    _stencil_test_enabled = val;
    _pD3DDevice->SetRenderState(D3DRS_STENCILENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_color_material
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_color_material(bool val) {
  if (_color_material_enabled != val) {
    _color_material_enabled = val;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
    _pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_color_writemask
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
set_color_writemask(UINT color_writemask) {
  if (_color_writemask != color_writemask) {
    _color_writemask = color_writemask;
    if(_pScrn->bCanDirectDisableColorWrites) {
        // only newer HW supports this rstate
        _pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, (DWORD)color_writemask);
    } else {
        // blending can only handle on/off
        assert((color_writemask==0x0)||(color_writemask==0xFFFFFFFF));
        set_blend_mode(_color_write_mode, _color_blend_mode, _transparency_mode);        
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_gouraud_shading(bool val) {
  if (_bGouraudShadingOn != val) {
    _bGouraudShadingOn = val;
    _pD3DDevice->SetRenderState(D3DRS_SHADEMODE, (val ? D3DSHADE_GOURAUD : D3DSHADE_FLAT));
  }
}

INLINE void DXGraphicsStateGuardian8::
enable_primitive_clipping(bool val) {
  if (_clipping_enabled != val) {
    _clipping_enabled = val;
    _pD3DDevice->SetRenderState(D3DRS_CLIPPING, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_fog
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_fog(bool val) {
  if ((_fog_enabled != val) && (_doFogType!=None)) {
    _fog_enabled = val;
    _pD3DDevice->SetRenderState(D3DRS_FOGENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_vertex_format
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
set_vertex_format(DWORD NewFvfType) {
#ifdef USE_VERTEX_SHADERS
  if(_CurVertexShader!=NULL) {
    // this needs optimization
    HRESULT hr = _pD3DDevice->SetVertexShader(_CurVertexShader);
    #ifndef NDEBUG
    if(FAILED(hr)) {
           dxgsg8_cat.error() << "SetVertexShader for custom vtx shader failed" << D3DERRORSTRING(hr);
           exit(1);
    }
    #endif
    _CurFVFType = NewFvfType;    
    return;
  }
#endif

  if (_CurFVFType != NewFvfType) {
    _CurFVFType = NewFvfType;

    HRESULT hr = _pD3DDevice->SetVertexShader(NewFvfType);
 #ifndef NDEBUG
    if(FAILED(hr)) {
           dxgsg8_cat.error() << "SetVertexShader(0x" << (void*)NewFvfType<<") failed" << D3DERRORSTRING(hr);
           exit(1);
    }
 #endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_alpha_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_alpha_test(bool val )
{
  if (_alpha_test_enabled != val) {
    _alpha_test_enabled = val;
    _pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::call_dxLightModelAmbient
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
call_dxLightModelAmbient( const Colorf& color)
{
  if (_lmodel_ambient != color) {
    _lmodel_ambient = color;
#ifdef GSG_VERBOSE
    dxgsg8_cat.debug() << "dxLightModel(LIGHT_MODEL_AMBIENT, " << color << ")" << endl;
#endif
    _pD3DDevice->SetRenderState( D3DRS_AMBIENT,
                D3DCOLOR_COLORVALUE(color[0], color[1], color[2], color[3]));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::call_dxAlphaFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
call_dxAlphaFunc(D3DCMPFUNC func, float reference_alpha) {
  if (_alpha_func != func) {
    _alpha_func = func;
#ifdef GSG_VERBOSE
    dxgsg8_cat.debug() << "dxAlphaFunc(";
    switch (func) {
    case D3DCMP_NEVER:
      dxgsg8_cat.debug(false) << "D3DCMP_NEVER";
      break;
    case D3DCMP_LESS:
      dxgsg8_cat.debug(false) << "D3DCMP_LESS";
      break;
    case D3DCMP_EQUAL:
      dxgsg8_cat.debug(false) << "D3DCMP_EQUAL";
      break;
#ifdef D3DCMP_LEQUAL
    case D3DCMP_LEQUAL:
      dxgsg8_cat.debug(false) << "D3DCMP_LEQUAL";
      break;
#endif
    case D3DCMP_GREATER:
      dxgsg8_cat.debug(false) << "D3DCMP_GREATER";
      break;
    case D3DCMP_NOTEQUAL:
      dxgsg8_cat.debug(false) << "D3DCMP_NOTEQUAL";
      break;
#ifdef D3DCMP_GEQUAL
    case D3DCMP_GEQUAL:
      dxgsg8_cat.debug(false) << "D3DCMP_GEQUAL";
      break;
#endif
    case D3DCMP_ALWAYS:
      dxgsg8_cat.debug(false) << "D3DCMP_ALWAYS";
      break;
    }
    dxgsg8_cat.debug() << " , " << reference_alpha << ")" << endl;
#endif
    _pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, func);
  }

  if(_alpha_func_refval != reference_alpha) {
      _alpha_func_refval = reference_alpha;
      _pD3DDevice->SetRenderState(D3DRS_ALPHAREF, (UINT) (reference_alpha*255.0f));  //d3d uses 0x0-0xFF, not a float
  }
}


INLINE void DXGraphicsStateGuardian8::
call_dxBlendFunc(D3DBLEND sfunc, D3DBLEND dfunc )
{
  if (_blend_source_func != sfunc)
    {
    _blend_source_func = sfunc;
    _pD3DDevice->SetRenderState(D3DRS_SRCBLEND, sfunc);
#ifdef GSG_VERBOSE
    dxgsg8_cat.debug() << "dxSrcBlendFunc(";
    switch (sfunc)
        {
        case D3DBLEND_ZERO:
          dxgsg8_cat.debug(false) << "ZERO, ";
          break;
        case D3DBLEND_ONE:
          dxgsg8_cat.debug(false) << "ONE, ";
          break;
        case D3DBLEND_DESTCOLOR:
          dxgsg8_cat.debug(false) << "DESTCOLOR, ";
          break;
        case D3DBLEND_INVDESTCOLOR:
          dxgsg8_cat.debug(false) << "INVDESTCOLOR, ";
          break;
        case D3DBLEND_SRCALPHA:
          dxgsg8_cat.debug(false) << "SRCALPHA, ";
          break;
        case D3DBLEND_INVSRCALPHA:
          dxgsg8_cat.debug(false) << "INVSRCALPHA, ";
          break;
        case D3DBLEND_DESTALPHA:
          dxgsg8_cat.debug(false) << "DESTALPHA, ";
          break;
        case D3DBLEND_INVDESTALPHA:
          dxgsg8_cat.debug(false) << "INVDESTALPHA, ";
          break;
        case D3DBLEND_SRCALPHASAT:
          dxgsg8_cat.debug(false) << "SRCALPHASAT, ";
          break;
        default:
          dxgsg8_cat.debug(false) << "unknown, ";
          break;
        }
    dxgsg8_cat.debug(false) << endl;
#endif
    }
  if ( _blend_dest_func != dfunc)
    {
    _blend_dest_func = dfunc;
    _pD3DDevice->SetRenderState(D3DRS_DESTBLEND, dfunc);
#ifdef GSG_VERBOSE
    dxgsg8_cat.debug() << "dxDstBlendFunc(";
    switch (dfunc)
        {
        case D3DBLEND_ZERO:
          dxgsg8_cat.debug(false) << "ZERO, ";
          break;
        case D3DBLEND_ONE:
          dxgsg8_cat.debug(false) << "ONE, ";
          break;
        case D3DBLEND_DESTCOLOR:
          dxgsg8_cat.debug(false) << "DESTCOLOR, ";
          break;
        case D3DBLEND_INVDESTCOLOR:
          dxgsg8_cat.debug(false) << "INVDESTCOLOR, ";
          break;
        case D3DBLEND_SRCALPHA:
          dxgsg8_cat.debug(false) << "SRCALPHA, ";
          break;
        case D3DBLEND_INVSRCALPHA:
          dxgsg8_cat.debug(false) << "INVSRCALPHA, ";
          break;
        case D3DBLEND_DESTALPHA:
          dxgsg8_cat.debug(false) << "DESTALPHA, ";
          break;
        case D3DBLEND_INVDESTALPHA:
          dxgsg8_cat.debug(false) << "INVDESTALPHA, ";
          break;
        case D3DBLEND_SRCALPHASAT:
          dxgsg8_cat.debug(false) << "SRCALPHASAT, ";
          break;
        }
    dxgsg8_cat.debug(false) << endl;
#endif
    }
}

INLINE void DXGraphicsStateGuardian8::
enable_zwritemask(bool val) {
    if (_depth_write_enabled != val) {
        _depth_write_enabled = val;
        _pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, val);
    }
}

/**  unimplemented

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_multisample
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_multisample(bool val) {
  _multisample_enabled = val;
  #ifdef NDEBUG
    dxgsg8_cat.error() << "dx multisample unimplemented!!\n";
  #endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_multisample_alpha_one
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_multisample_alpha_one(bool val) {
  if (_multisample_alpha_one_enabled != val) {
    _multisample_alpha_one_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_multisample_alpha_mask
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_multisample_alpha_mask(bool val) {
  if (_multisample_alpha_mask_enabled != val) {
    _multisample_alpha_mask_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_point_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_point_smooth(bool val) {
  // _point_smooth_enabled = val;

  #ifdef NDEBUG
    dxgsg8_cat.error() << "dx point smoothing unimplemented!!\n";
  #endif
}
*/

