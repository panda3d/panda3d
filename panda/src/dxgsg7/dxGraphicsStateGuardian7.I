// Filename: dxGraphicsStateGuardian7.I
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

#include "config_dxgsg7.h"
#include <graphicsWindow.h>

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_line_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_line_smooth(bool val) {
  if(_line_smooth_enabled != val) {
    _line_smooth_enabled = val;
  #ifdef NDEBUG
    {
        if(val && (_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES))
           dxgsg7_cat.error() << "no HW support for line smoothing!!\n";
    }
  #endif

    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_dither
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_dither(bool val) {
  if (_dither_enabled != val) {

  #ifdef _DEBUG
    {
        if(val && !(_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER))
           dxgsg7_cat.error() << "no HW support for color dithering!!\n";
        return;
    }
  #endif

    _dither_enabled = val;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_stencil_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_stencil_test(bool val) {
  if (_stencil_test_enabled != val) {
    _stencil_test_enabled = val;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_color_material
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_color_material(bool val) {
  if (_color_material_enabled != val) {
    _color_material_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_blend
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
set_shademode(D3DSHADEMODE val) {
  if (_CurShadeMode != val) {
    _CurShadeMode = val;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, (DWORD)val);
  }
}

INLINE void DXGraphicsStateGuardian7::
enable_primitive_clipping(bool val) {
  if (_clipping_enabled != val) {
    _clipping_enabled = val;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPING, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_fog
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_fog(bool val) {
  if ((_fog_enabled != val) && (_doFogType!=None)) {
    _fog_enabled = val;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_alpha_test
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_alpha_test(bool val )
{
  if (_alpha_test_enabled != val) {
    _alpha_test_enabled = val;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::call_dxLightModelAmbient
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
call_dxLightModelAmbient( const Colorf& color)
{
  if (_lmodel_ambient != color) {
    _lmodel_ambient = color;
#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "dxLightModel(LIGHT_MODEL_AMBIENT, " << color << ")" << endl;
#endif
    _pScrn->pD3DDevice->SetRenderState( D3DRENDERSTATE_AMBIENT,
                D3DRGBA(color[0], color[1], color[2], color[3]));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::call_dxAlphaFunc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
call_dxAlphaFunc(D3DCMPFUNC func, float reference_alpha) {
  if (_alpha_func != func) {
    _alpha_func = func;
#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "dxAlphaFunc(";
    switch (func) {
    case D3DCMP_NEVER:
      dxgsg7_cat.debug(false) << "D3DCMP_NEVER";
      break;
    case D3DCMP_LESS:
      dxgsg7_cat.debug(false) << "D3DCMP_LESS";
      break;
    case D3DCMP_EQUAL:
      dxgsg7_cat.debug(false) << "D3DCMP_EQUAL";
      break;
#ifdef D3DCMP_LEQUAL
    case D3DCMP_LEQUAL:
      dxgsg7_cat.debug(false) << "D3DCMP_LEQUAL";
      break;
#endif
    case D3DCMP_GREATER:
      dxgsg7_cat.debug(false) << "D3DCMP_GREATER";
      break;
    case D3DCMP_NOTEQUAL:
      dxgsg7_cat.debug(false) << "D3DCMP_NOTEQUAL";
      break;
#ifdef D3DCMP_GEQUAL
    case D3DCMP_GEQUAL:
      dxgsg7_cat.debug(false) << "D3DCMP_GEQUAL";
      break;
#endif
    case D3DCMP_ALWAYS:
      dxgsg7_cat.debug(false) << "D3DCMP_ALWAYS";
      break;
    }
    dxgsg7_cat.debug() << " , " << reference_alpha << ")" << endl;
#endif
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, func);
  }

  if(_alpha_func_refval != reference_alpha) {
      _alpha_func_refval = reference_alpha;
      _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (UINT) (reference_alpha*255.0f));  //d3d uses 0x0-0xFF, not a float
  }
}

INLINE void DXGraphicsStateGuardian7::
call_dxBlendFunc(D3DBLEND sfunc, D3DBLEND dfunc )
{
  if (_blend_source_func != sfunc)
    {
    _blend_source_func = sfunc;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, sfunc);
#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "dxSrcBlendFunc(";
    switch (sfunc)
        {
        case D3DBLEND_ZERO:
          dxgsg7_cat.debug(false) << "ZERO, ";
          break;
        case D3DBLEND_ONE:
          dxgsg7_cat.debug(false) << "ONE, ";
          break;
        case D3DBLEND_DESTCOLOR:
          dxgsg7_cat.debug(false) << "DESTCOLOR, ";
          break;
        case D3DBLEND_INVDESTCOLOR:
          dxgsg7_cat.debug(false) << "INVDESTCOLOR, ";
          break;
        case D3DBLEND_SRCALPHA:
          dxgsg7_cat.debug(false) << "SRCALPHA, ";
          break;
        case D3DBLEND_INVSRCALPHA:
          dxgsg7_cat.debug(false) << "INVSRCALPHA, ";
          break;
        case D3DBLEND_DESTALPHA:
          dxgsg7_cat.debug(false) << "DESTALPHA, ";
          break;
        case D3DBLEND_INVDESTALPHA:
          dxgsg7_cat.debug(false) << "INVDESTALPHA, ";
          break;
        case D3DBLEND_SRCALPHASAT:
          dxgsg7_cat.debug(false) << "SRCALPHASAT, ";
          break;
        default:
          dxgsg7_cat.debug(false) << "unknown, ";
          break;
        }
    dxgsg7_cat.debug(false) << endl;
#endif
    }
  if ( _blend_dest_func != dfunc)
    {
    _blend_dest_func = dfunc;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, dfunc);
#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "dxDstBlendFunc(";
    switch (dfunc)
        {
        case D3DBLEND_ZERO:
          dxgsg7_cat.debug(false) << "ZERO, ";
          break;
        case D3DBLEND_ONE:
          dxgsg7_cat.debug(false) << "ONE, ";
          break;
        case D3DBLEND_DESTCOLOR:
          dxgsg7_cat.debug(false) << "DESTCOLOR, ";
          break;
        case D3DBLEND_INVDESTCOLOR:
          dxgsg7_cat.debug(false) << "INVDESTCOLOR, ";
          break;
        case D3DBLEND_SRCALPHA:
          dxgsg7_cat.debug(false) << "SRCALPHA, ";
          break;
        case D3DBLEND_INVSRCALPHA:
          dxgsg7_cat.debug(false) << "INVSRCALPHA, ";
          break;
        case D3DBLEND_DESTALPHA:
          dxgsg7_cat.debug(false) << "DESTALPHA, ";
          break;
        case D3DBLEND_INVDESTALPHA:
          dxgsg7_cat.debug(false) << "INVDESTALPHA, ";
          break;
        case D3DBLEND_SRCALPHASAT:
          dxgsg7_cat.debug(false) << "SRCALPHASAT, ";
          break;
        }
    dxgsg7_cat.debug(false) << endl;
#endif
    }
}

INLINE void DXGraphicsStateGuardian7::
enable_zwritemask(bool val) {
    if (_depth_write_enabled != val) {
        _depth_write_enabled = val;
        _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, val);
    }
}

/**  unimplemented

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_multisample
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_multisample(bool val) {
  _multisample_enabled = val;
  #ifdef NDEBUG
    dxgsg7_cat.error() << "dx multisample unimplemented!!\n";
  #endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_multisample_alpha_one
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_multisample_alpha_one(bool val) {
  if (_multisample_alpha_one_enabled != val) {
    _multisample_alpha_one_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_multisample_alpha_mask
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_multisample_alpha_mask(bool val) {
  if (_multisample_alpha_mask_enabled != val) {
    _multisample_alpha_mask_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_point_smooth
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_point_smooth(bool val) {
  // _point_smooth_enabled = val;

  #ifdef NDEBUG
    dxgsg7_cat.error() << "dx point smoothing unimplemented!!\n";
  #endif
}
*/

