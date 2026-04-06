/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGraphicsStateGuardian9.I
 * @author mike
 * @date 1999-02-02
 */

/**
 * Converts Panda's floating-point LColor structure to DirectX's D3DCOLOR
 * packed structure.
 */
INLINE DWORD DXGraphicsStateGuardian9::
LColor_to_D3DCOLOR(const LColor &cLColor) {
// MS VC defines _M_IX86 for x86.  gcc should define _X86_
#if (defined(_M_IX86) || defined(_X86_)) && !defined(STDFLOAT_DOUBLE)
  DWORD d3dcolor, tempcolorval=255;

  // note the default FPU rounding mode will give 255*0.5f=0x80, not 0x7F as
  // VC would force it to by resetting rounding mode don't think this makes
  // much difference

  __asm {
        push ebx   ; want to save this in case this fn is inlined
        push ecx
        mov ecx, cLColor
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

  // dxgsg9_cat.debug() << (void*)d3dcolor << endl;
  return d3dcolor;
#else //!_X86_
  return D3DCOLOR_COLORVALUE(cLColor[0], cLColor[1], cLColor[2], cLColor[3]);
#endif //!_X86_
}

/**
 * Maps from the Texture's internal wrap mode symbols to GL's.
 */
INLINE D3DTEXTUREADDRESS DXGraphicsStateGuardian9::
get_texture_wrap_mode(SamplerState::WrapMode wm) {
  switch (wm) {
  case SamplerState::WM_clamp:
    return D3DTADDRESS_CLAMP;
  case SamplerState::WM_repeat:
    return D3DTADDRESS_WRAP;
  case SamplerState::WM_mirror:
    return D3DTADDRESS_MIRROR;
  case SamplerState::WM_mirror_once:
    return D3DTADDRESS_MIRRORONCE;
  case SamplerState::WM_border_color:
    return D3DTADDRESS_BORDER;
  }
  dxgsg9_cat.error() << "Invalid Texture::Mode value" << std::endl;
  return D3DTADDRESS_WRAP;
}

/**
 * Maps from the fog types to gl version
 */
INLINE D3DFOGMODE DXGraphicsStateGuardian9::
get_fog_mode_type(Fog::Mode m) {
  switch (m) {
  case Fog::M_linear:
    return D3DFOG_LINEAR;
  case Fog::M_exponential:
    return D3DFOG_EXP;
  case Fog::M_exponential_squared:
    return D3DFOG_EXP2;
  }
  dxgsg9_cat.error() << "Invalid Fog::Mode value" << std::endl;
  return D3DFOG_EXP;
}

/**
 * Returns the nth D3DTS_TEXTURE(n) constant.
 */
INLINE D3DTRANSFORMSTATETYPE DXGraphicsStateGuardian9::
get_tex_mat_sym(int stage_index) {
  return (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage_index);
}

/**
 * Returns the address of a 64K buffer that is allocated at the beginning of a
 * 64K block.
 */
INLINE unsigned char *DXGraphicsStateGuardian9::
get_safe_buffer_start() {
  if (_temp_buffer == nullptr) {
    // Guarantee we get a buffer of size 0x10000 bytes that begins on an even
    // multiple of 0x10000.  We do this by allocating double the required
    // buffer, and then pointing to the first multiple of 0x10000 within that
    // buffer.
    _temp_buffer = new unsigned char[0x1ffff];
    _safe_buffer_start = (unsigned char *)(((uintptr_t)_temp_buffer + 0xffff) & ~0xffff);
  }

  return _safe_buffer_start;
}

#define ALWAYS_SET_RENDER_STATE true

/**
 * This function creates a common layer between DX and Panda for
 * SetRenderState.  It also keeps avoids setting redundant render states.
 */
INLINE HRESULT DXGraphicsStateGuardian9::
set_render_state (D3DRENDERSTATETYPE state, DWORD value)
{
  HRESULT hr;

  hr = D3D_OK;
  if (ALWAYS_SET_RENDER_STATE || _render_state_array [state] != value)
  {
    hr = _d3d_device->SetRenderState(state, value);
    _render_state_array [state] = value;
  }

  return hr;
}

/**
 * This function creates a common layer between DX and Panda.  It also keeps
 * avoids setting redundant render states.
 */
INLINE HRESULT DXGraphicsStateGuardian9::
set_texture_stage_state (DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
  HRESULT hr;

  hr = D3D_OK;
  if (ALWAYS_SET_RENDER_STATE || _texture_stage_states_array [stage].state_array [type] != value)
  {
    hr = _d3d_device->SetTextureStageState(stage, type, value);
    _texture_stage_states_array [stage].state_array [type] = value;
  }

  return hr;
}

/**
 * This function creates a common layer between DX and Panda.  It also keeps
 * avoids setting redundant render states.
 */
INLINE HRESULT DXGraphicsStateGuardian9::
set_sampler_state (DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
{
  HRESULT hr;

  hr = D3D_OK;
  if (ALWAYS_SET_RENDER_STATE || _texture_render_states_array [sampler].state_array [type] != value)
  {
    hr = _d3d_device->SetSamplerState(sampler, type, value);
    _texture_render_states_array [sampler].state_array [type] = value;
  }

  return hr;
}


/**
 * Returns true if this particular GSG can render from a wdxGraphicsBuffer9
 * directly into a texture, or false if it must always copy-to-texture at the
 * end of each frame to achieve this effect.
 */
INLINE bool DXGraphicsStateGuardian9::
get_supports_render_texture() const {
  return _supports_render_texture;
}
