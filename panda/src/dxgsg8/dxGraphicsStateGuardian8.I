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
//     Function: DXGraphicsStateGuardian8::Colorf_to_D3DCOLOR
//       Access: Public, Static
//  Description: Converts Panda's floating-point Colorf structure to
//               DirectX's D3DCOLOR packed structure.
////////////////////////////////////////////////////////////////////
INLINE DWORD DXGraphicsStateGuardian8::
Colorf_to_D3DCOLOR(const Colorf &cColorf) {
// MS VC defines _M_IX86 for x86.  gcc should define _X86_
#if defined(_M_IX86) || defined(_X86_)
  DWORD d3dcolor, tempcolorval=255;

  // note the default FPU rounding mode will give 255*0.5f=0x80, not 0x7F as VC would force it to by resetting rounding mode
  // dont think this makes much difference
  
  __asm {
        push ebx   ; want to save this in case this fn is inlined
        push ecx
        mov ecx, cColorf
        fild tempcolorval
        fld DWORD PTR [ecx]
        fmul ST(0), ST(1)
        fistp tempcolorval  ; no way to store directly to int register
        mov eax, tempcolorval
        shl eax, 16

        fld DWORD PTR [ecx+4]  ;grn
        fmul ST(0), ST(1)
        fistp tempcolorval
        mov ebx, tempcolorval
        shl ebx, 8
        or eax, ebx

        fld DWORD PTR [ecx+8]  ;blue
        fmul ST(0), ST(1)
        fistp tempcolorval
        or eax, tempcolorval

        fld DWORD PTR [ecx+12] ;alpha
        fmul ST(0), ST(1)
        fistp tempcolorval
        ; simulate pop 255.0 off FP stack w/o store, mark top as empty and increment stk ptr
        ffree ST(0)
        fincstp
        mov ebx, tempcolorval
        shl ebx, 24
        or eax, ebx
        mov d3dcolor, eax
        pop ecx
        pop ebx
  }

  //   dxgsg8_cat.debug() << (void*)d3dcolor << endl;
  return d3dcolor;
#else //!_X86_
  return MY_D3DRGBA(cColorf[0], cColorf[1], cColorf[2], cColorf[3]);
#endif //!_X86_
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_color_material
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_color_material(bool val) {
  if (_color_material_enabled != val) {
    _color_material_enabled = val;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_fog
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_fog(bool val) {
  if ((_fog_enabled != val) && (_do_fog_type!=None)) {
    _fog_enabled = val;
    _d3d_device->SetRenderState(D3DRS_FOGENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_zwritemask
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_zwritemask(bool val) {
  if (_depth_write_enabled != val) {
    _depth_write_enabled = val;
    _d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_vertex_format
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
set_vertex_format(DWORD NewFvfType) {
  if (_cur_fvf_type != NewFvfType) {
    _cur_fvf_type = NewFvfType;

    HRESULT hr = _d3d_device->SetVertexShader(NewFvfType);
#ifndef NDEBUG
    if(FAILED(hr)) {
      dxgsg8_cat.error() << "SetVertexShader(0x" << (void*)NewFvfType<<") failed" << D3DERRORSTRING(hr);
      exit(1);
    }
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_texture_wrap_mode
//       Access: Protected, Static
//  Description: Maps from the Texture's internal wrap mode symbols to
//               GL's.
////////////////////////////////////////////////////////////////////
INLINE D3DTEXTUREADDRESS DXGraphicsStateGuardian8::
get_texture_wrap_mode(Texture::WrapMode wm) {
  switch (wm) {
  case Texture::WM_clamp:
    return D3DTADDRESS_CLAMP;
  case Texture::WM_repeat:
    return D3DTADDRESS_WRAP;
  case Texture::WM_mirror:
    return D3DTADDRESS_MIRROR;
  case Texture::WM_mirror_once:
    return D3DTADDRESS_MIRRORONCE;
  case Texture::WM_border_color:
    return D3DTADDRESS_BORDER;
  }
  dxgsg8_cat.error() << "Invalid Texture::Mode value" << endl;
  return D3DTADDRESS_WRAP;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_fog_mode_type
//       Access: Protected, Static
//  Description: Maps from the fog types to gl version
////////////////////////////////////////////////////////////////////
INLINE D3DFOGMODE DXGraphicsStateGuardian8::
get_fog_mode_type(Fog::Mode m) {
  switch (m) {
  case Fog::M_linear:
    return D3DFOG_LINEAR;
  case Fog::M_exponential:
    return D3DFOG_EXP;
  case Fog::M_exponential_squared:
    return D3DFOG_EXP2;
  }
  dxgsg8_cat.error() << "Invalid Fog::Mode value" << endl;
  return D3DFOG_EXP;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_tex_mat_sym
//       Access: Protected, Static
//  Description: Returns the nth D3DTS_TEXTURE(n) constant.
////////////////////////////////////////////////////////////////////
INLINE D3DTRANSFORMSTATETYPE DXGraphicsStateGuardian8::
get_tex_mat_sym(int stage_index) {
  return (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage_index);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_alpha_test
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_alpha_test(bool val) {
  if (_alpha_test_enabled != val) {
    _alpha_test_enabled = val;
    _d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_blend
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_blend(bool val) {
  if (_blend_enabled != val) {
    _blend_enabled = val;
    _d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD)val);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::call_dxLightModelAmbient
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
call_dxLightModelAmbient(const Colorf &color) {
  if (_lmodel_ambient != color) {
    _lmodel_ambient = color;
    _d3d_device->SetRenderState(D3DRS_AMBIENT, 
                                 D3DCOLOR_COLORVALUE(color[0], color[1], color[2], color[3]));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::call_dxAlphaFunc
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
call_dxAlphaFunc(D3DCMPFUNC func, float reference_alpha) {
  if (_alpha_func != func) {
    _alpha_func = func;
    _d3d_device->SetRenderState(D3DRS_ALPHAFUNC, func);
  }

  if(_alpha_func_refval != reference_alpha) {
    _alpha_func_refval = reference_alpha;
    _d3d_device->SetRenderState(D3DRS_ALPHAREF, (UINT) (reference_alpha*255.0f));  //d3d uses 0x0-0xFF, not a float
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::call_dxBlendFunc
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
call_dxBlendFunc(D3DBLEND sfunc, D3DBLEND dfunc) {
  if (_blend_source_func != sfunc) {
    _blend_source_func = sfunc;
    _d3d_device->SetRenderState(D3DRS_SRCBLEND, sfunc);
  }
  if (_blend_dest_func != dfunc) {
    _blend_dest_func = dfunc;
    _d3d_device->SetRenderState(D3DRS_DESTBLEND, dfunc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_dither
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian8::
enable_dither(bool val) {
  if (_dither_enabled != val) {
    _dither_enabled = val;

    _d3d_device->SetRenderState(D3DRS_DITHERENABLE, (DWORD)val);
  }
}

