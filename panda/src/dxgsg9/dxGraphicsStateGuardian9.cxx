// Filename: dxGraphicsStateGuardian.cxx
// Created by:   masad (02Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2004, Disney Enterprises, Inc.  All rights reserved
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

#include "dxGraphicsStateGuardian9.h"
#include "config_dxgsg9.h"
#include <d3dx9.h>
#include "displayRegion.h"
#include "renderBuffer.h"
#include "geom.h"
#include "geomSphere.h"
#include "geomIssuer.h"
#include "graphicsWindow.h"
#include "graphicsEngine.h"
#include "lens.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"
#include "textureAttrib.h"
#include "lightAttrib.h"
#include "cullFaceAttrib.h"
#include "transparencyAttrib.h"
#include "alphaTestAttrib.h"
#include "depthTestAttrib.h"
#include "depthWriteAttrib.h"
#include "colorWriteAttrib.h"
#include "texMatrixAttrib.h"
#include "materialAttrib.h"
#include "renderModeAttrib.h"
#include "rescaleNormalAttrib.h"
#include "fogAttrib.h"
#include "depthOffsetAttrib.h"
#include "fog.h"
#include "throw_event.h"

#ifdef DO_PSTATS
#include "pStatTimer.h"
#include "pStatCollector.h"
#endif

// disable nameless struct 'warning'
#pragma warning (disable : 4201)

#include <mmsystem.h>

// print out simple drawprim stats every few secs
//#define COUNT_DRAWPRIMS
//#define PRINT_RESOURCESTATS  // uses d3d GetInfo

//#define DISABLE_DECALING
#define DISABLE_POLYGON_OFFSET_DECALING
// currently doesnt work well enough in toontown models for us to use
// prob is when viewer gets close to decals, they disappear into wall poly, need to investigate

//const int VERT_BUFFER_SIZE = (8*1024L);
// For sparkle particles, we can have 4 vertices per sparkle, and a
// particle pool size of 1024 particles

// for sprites, 1000 prims, 6 verts/prim, 24 bytes/vert
const int VERT_BUFFER_SIZE = (32*6*1024L);

TypeHandle DXGraphicsStateGuardian9::_type_handle;

// bit masks used for drawing primitives
// bitmask type: normal=0x1,color=0x2,texcoord=0x4
typedef enum { NothingSet=0,NormalOnly,ColorOnly,Normal_Color,TexCoordOnly,
               Normal_TexCoord,Color_TexCoord,Normal_Color_TexCoord
} DrawLoopFlags;

#define PER_NORMAL   NormalOnly
#define PER_COLOR    ColorOnly
#define PER_TEXCOORD TexCoordOnly

static D3DMATRIX matIdentity;

#define __D3DLIGHT_RANGE_MAX ((float)sqrt(FLT_MAX))  //for some reason this is missing in dx9 hdrs

#ifdef COUNT_DRAWPRIMS
// instead of this use nvidia stat drvr or GetInfo VtxStats?
static DWORD cDPcount=0;
static DWORD cVertcount=0;
static DWORD cTricount=0;
static DWORD cGeomcount=0;

static IDirect3DTexture9 *pLastTexture=NULL;
static DWORD cDP_noTexChangeCount=0;
static LPDIRECT3DDEVICE9 global_pD3DDevice = NULL;

static void CountDPs(DWORD nVerts,DWORD nTris) {
    cDPcount++;
    cVertcount+=nVerts;
    cTricount+=nTris;

    if(_pCurDeviceTexture==pLastTexture) {
        cDP_noTexChangeCount++;
    } else pLastTexture = _pCurDeviceTexture;
}
#else
#define CountDPs(nv,nt)
#endif

#define MY_D3DRGBA(r,g,b,a) ((D3DCOLOR) D3DCOLOR_COLORVALUE(r,g,b,a))

#if defined(DO_PSTATS) || defined(PRINT_RESOURCESTATS)
static bool bTexStatsRetrievalImpossible=false;
#endif

//#define Colorf_to_D3DCOLOR(out_color) (MY_D3DRGBA((out_color)[0], (out_color)[1], (out_color)[2], (out_color)[3]))

void DXGraphicsStateGuardian9::
set_color_clear_value(const Colorf& value) {
  _color_clear_value = value;
  _d3dcolor_clear_value =  Colorf_to_D3DCOLOR(value);
}

#if defined(_DEBUG) || defined(COUNT_DRAWPRIMS)
typedef enum {DrawPrim,DrawIndexedPrim} DP_Type;
static const char *DP_Type_Strs[3] = {"DrawPrimitive","DrawIndexedPrimitive"};

void INLINE TestDrawPrimFailure(DP_Type dptype,HRESULT hr,IDirect3DDevice9 *pD3DDevice,DWORD nVerts,DWORD nTris) {
        if(FAILED(hr)) {
            // loss of exclusive mode is not a real DrawPrim problem, ignore it
            HRESULT testcooplvl_hr = pD3DDevice->TestCooperativeLevel();
            if((testcooplvl_hr != D3DERR_DEVICELOST)||(testcooplvl_hr != D3DERR_DEVICENOTRESET)) {
                dxgsg9_cat.fatal() << DP_Type_Strs[dptype] << "() failed: result = " << D3DERRORSTRING(hr);
                exit(1);
            }
        }

        CountDPs(nVerts,nTris);
}
#else
#define TestDrawPrimFailure(a,b,c,nVerts,nTris) CountDPs(nVerts,nTris);
#endif

void DXGraphicsStateGuardian9::
reset_panda_gsg(void) {
    GraphicsStateGuardian::reset();

    _auto_rescale_normal = false;

    // overwrite gsg defaults with these values

    // All implementations have the following buffers.
    _buffer_mask = (RenderBuffer::T_color |
                    RenderBuffer::T_back
//                    RenderBuffer::T_depth |
//                  RenderBuffer::T_stencil |
//                  RenderBuffer::T_accum
                    );

    //   stmt below is incorrect for general mono displays, need both right and left flags set.
    //   stereo not supported in dx9
    //    _buffer_mask &= ~RenderBuffer::T_right;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian9::
DXGraphicsStateGuardian9(const FrameBufferProperties &properties) :
  GraphicsStateGuardian(properties, CS_yup_left) 
{

    reset_panda_gsg();

    _pScrn = NULL;
    _pD3DDevice = NULL;
    
    _bDXisReady = false;
    _overlay_windows_supported = false;

    _pFvfBufBasePtr = NULL;
    _index_buf=NULL;

    //    _max_light_range = __D3DLIGHT_RANGE_MAX;

    // non-dx obj values inited here should not change if resize is
    // called and dx objects need to be recreated (otherwise they
    // belong in dx_init, with other renderstate

    ZeroMemory(&matIdentity,sizeof(D3DMATRIX));
    matIdentity._11 = matIdentity._22 = matIdentity._33 = matIdentity._44 = 1.0f;

    _cur_read_pixel_buffer=RenderBuffer::T_front;
    set_color_clear_value(_color_clear_value);

    // DirectX drivers seem to consistently invert the texture when
    // they copy framebuffer-to-texture.  Ok.
    _copy_texture_inverted = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian9::
~DXGraphicsStateGuardian9() {
    if (IS_VALID_PTR(_pD3DDevice)) 
        _pD3DDevice->SetTexture(0, NULL);  // this frees reference to the old texture
    _pCurTexContext = NULL;

    //free_d3d_device();   ???

    free_nondx_resources();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.  The GraphicsWindow pointer represents a
//               typical window that might be used for this context;
//               it may be required to set up the frame buffer
//               properly the first time.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
reset() {
    reset_panda_gsg();
    dxgsg9_cat.error() << "DXGSG reset() not implemented properly yet!\n";
    // what else do we need to do?
    // delete all the objs too, right?
    // need to do a
    //dx_init();
}

// setup up for re-calling dx_init(), this is not the final exit cleanup routine (see dx_cleanup)
void DXGraphicsStateGuardian9::
free_d3d_device(void) {
    // dont want a full reset of gsg, just a state clear
    set_state(RenderState::make_empty());
    // want gsg to pass all state settings through

    _bDXisReady = false;

    if(_pD3DDevice!=NULL)
     for(int i=0;i<D3D_MAXTEXTURESTAGES;i++)
         _pD3DDevice->SetTexture(i,NULL);  // d3d should release this stuff internally anyway, but whatever

    DeleteAllDeviceObjects();

    if (_pD3DDevice!=NULL)
        RELEASE(_pD3DDevice,dxgsg9,"d3dDevice",RELEASE_DOWN_TO_ZERO);

    free_nondx_resources();

    // obviously we dont release ID3D9, just ID3DDevice9
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::free_nondx_resources
//       Access: Public
//  Description: Frees some memory that was explicitly allocated
//               within the dxgsg.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
free_nondx_resources() {
    // this must not release any objects associated with D3D/DX!
    // those should be released in free_dxgsg_objects instead
    SAFE_DELETE_ARRAY(_index_buf);
    SAFE_DELETE_ARRAY(_pFvfBufBasePtr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::reset
//       Access: Public, Virtual
//  Description: Handles initialization which assumes that DX has already been
//               set up.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
dx_init(void) {
    HRESULT hr;

    // make sure gsg passes all current state down to us
    set_state(RenderState::make_empty());
    // want gsg to pass all state settings down so any non-matching defaults we set here get overwritten

    assert(_pScrn->pD3D9!=NULL);
    assert(_pD3DDevice!=NULL);

    ZeroMemory(&_lmodel_ambient,sizeof(Colorf));
    _pD3DDevice->SetRenderState(D3DRS_AMBIENT, 0x0);

    if(_pFvfBufBasePtr==NULL)
        _pFvfBufBasePtr = new BYTE[VERT_BUFFER_SIZE];  // allocate storage for vertex info.
    if(_index_buf==NULL)
        _index_buf = new WORD[PANDA_MAXNUMVERTS];  // allocate storage for vertex index info.

    _pCurFvfBufPtr = NULL;

    _clip_plane_bits = 0;
    _pD3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE , 0x0);

    _pD3DDevice->SetRenderState(D3DRS_CLIPPING, true);
    _clipping_enabled = true;

    // these both reflect d3d defaults
    _color_writemask = 0xFFFFFFFF;
    _CurFVFType = 0x0;  // guards SetVertexShader fmt

    _bGouraudShadingOn = false;
    _pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

//   this specifies if lighting model uses material color or vertex color
//   (not related to gouraud/flat shading)
//   _pD3DDevice->SetRenderState(D3DRS_COLORVERTEX, true);

    _depth_test_enabled = true;
    _pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, _depth_test_enabled);

    _pCurTexContext = NULL;

    _line_smooth_enabled = false;
    _pD3DDevice->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, false);

    _color_material_enabled = false;

    _depth_test_enabled = D3DZB_FALSE;
    _pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

    _blend_enabled = false;
    _pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD)_blend_enabled);

    // just use whatever d3d defaults to here
    _pD3DDevice->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&_blend_source_func);
    _pD3DDevice->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&_blend_dest_func);

    _fog_enabled = false;
    _pD3DDevice->SetRenderState(D3DRS_FOGENABLE, _fog_enabled);

    _current_projection_mat = LMatrix4f::ident_mat();
    _projection_mat_stack_count = 0;
    _has_scene_graph_color = false;

//  GL stuff that hasnt been translated to DX
    // none of these are implemented
    //_multisample_enabled = false;         // bugbug:  translate this to dx_multisample_antialiasing_level?
    //_point_smooth_enabled = false;

//    _scissor_enabled = false;
//    _multisample_alpha_one_enabled = false;
//    _multisample_alpha_mask_enabled = false;
//    _line_width = 1.0f;
//    _point_size = 1.0f;

#ifdef COUNT_DRAWPRIMS
     global_pD3DDevice = pDevice;
#endif
    _bDrawPrimDoSetupVertexBuffer = false;

    _last_testcooplevel_result = D3D_OK;

    for(int i=0;i<MAX_POSSIBLE_TEXFMTS;i++) {
      // look for all possible DX9 texture fmts
      D3DFORMAT_FLAG fmtflag=D3DFORMAT_FLAG(1<<i);
      hr = _pScrn->pD3D9->CheckDeviceFormat(_pScrn->CardIDNum,D3DDEVTYPE_HAL,_pScrn->DisplayMode.Format,
                                            0x0,D3DRTYPE_TEXTURE,g_D3DFORMATmap[fmtflag]);
      if(SUCCEEDED(hr)){
        _pScrn->SupportedTexFmtsMask|=fmtflag;
      }
    }

    // s3 virge drivers sometimes give crap values for these
    if(_pScrn->d3dcaps.MaxTextureWidth==0)
       _pScrn->d3dcaps.MaxTextureWidth=256;

    if(_pScrn->d3dcaps.MaxTextureHeight==0)
       _pScrn->d3dcaps.MaxTextureHeight=256;

#define REQUIRED_DESTBLENDCAPS (D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE| D3DPBLENDCAPS_SRCALPHA)
#define REQUIRED_SRCBLENDCAPS  (D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE| D3DPBLENDCAPS_INVSRCALPHA)

    if (((_pScrn->d3dcaps.SrcBlendCaps & REQUIRED_SRCBLENDCAPS)!=REQUIRED_SRCBLENDCAPS) ||
        ((_pScrn->d3dcaps.DestBlendCaps & REQUIRED_DESTBLENDCAPS)!=REQUIRED_DESTBLENDCAPS)) {
        dxgsg9_cat.error() << "device is missing alpha blending capabilities, blending may not work correctly: SrcBlendCaps: 0x"<< (void*) _pScrn->d3dcaps.SrcBlendCaps << "  DestBlendCaps: "<< (void*) _pScrn->d3dcaps.DestBlendCaps << endl;
    }

// just 'require' bilinear with mip nearest.
#define REQUIRED_TEXFILTERCAPS (D3DPTFILTERCAPS_MAGFLINEAR | D3DPTFILTERCAPS_MIPFPOINT | D3DPTFILTERCAPS_MINFLINEAR)

    if ((_pScrn->d3dcaps.TextureFilterCaps & REQUIRED_TEXFILTERCAPS)!=REQUIRED_TEXFILTERCAPS) {
        dxgsg9_cat.error() << "device is missing texture bilinear filtering capability, textures may appear blocky!  TextureFilterCaps: 0x"<< (void*) _pScrn->d3dcaps.TextureFilterCaps << endl;
    }

#define TRILINEAR_MIPMAP_TEXFILTERCAPS (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_MINFLINEAR)

    // give a warning if we dont at least have bilinear + nearest mip filtering
    if (!(_pScrn->d3dcaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP)) {
        if(dxgsg9_cat.is_debug())
           dxgsg9_cat.debug() << "device does not have mipmap texturing filtering capability! TextureFilterCaps: 0x"<< (void*) _pScrn->d3dcaps.TextureFilterCaps << endl;
        dx_ignore_mipmaps = TRUE;
    } else if ((_pScrn->d3dcaps.TextureFilterCaps & TRILINEAR_MIPMAP_TEXFILTERCAPS)!=TRILINEAR_MIPMAP_TEXFILTERCAPS) {
        if(dxgsg9_cat.is_debug())
           dxgsg9_cat.debug() << "device is missing tri-linear mipmap filtering capability, textures may look crappy\n";
    } else if(_pScrn->d3dcaps.DevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES) {
        // this cap is pretty much voodoo2-specific
        // turn off trilinear filtering on voodoo2 since it doubles the reqd texture memory, degrade to mip point filtering
        _pScrn->d3dcaps.TextureFilterCaps &= (~D3DPTFILTERCAPS_MIPFLINEAR);
    }

#define REQUIRED_TEXBLENDCAPS (D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2)
    if ((_pScrn->d3dcaps.TextureOpCaps & REQUIRED_TEXBLENDCAPS)!=REQUIRED_TEXBLENDCAPS) {
        dxgsg9_cat.error() << "device is missing some required texture blending capabilities, texture blending may not work properly! TextureOpCaps: 0x"<< (void*) _pScrn->d3dcaps.TextureOpCaps << endl;
    }

    if(_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGTABLE) {
        // watch out for drivers that emulate per-pixel fog with per-vertex fog (Riva128, Matrox Millen G200)
        // some of these require gouraud-shading to be set to work, as if you were using vertex fog
        _doFogType=PerPixelFog;
    } else {
        // every card is going to have vertex fog, since it's implemented in d3d runtime
        assert((_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX )!=0);

        // vtx fog may look crappy if you have large polygons in the foreground and they get clipped,
        // so you may want to disable it

        if(dx_no_vertex_fog) {
            _doFogType = None;
        } else {
            _doFogType = PerVertexFog;

            // range-based fog only works with vertex fog in dx7/8
            if(dx_use_rangebased_fog && (_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGRANGE))
                _pD3DDevice->SetRenderState(D3DRS_RANGEFOGENABLE, true);
        }
    }

    _pScrn->bCanDirectDisableColorWrites=((_pScrn->d3dcaps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE)!=0);

    // Lighting, let's turn it off by default
    _pD3DDevice->SetRenderState(D3DRS_LIGHTING, false);

    // turn on dithering if the rendertarget is < 8bits/color channel
    _dither_enabled = ((!dx_no_dithering) && IS_16BPP_DISPLAY_FORMAT(_pScrn->PresParams.BackBufferFormat)
                        && (_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_DITHER));
    _pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, _dither_enabled);

    _pD3DDevice->SetRenderState(D3DRS_CLIPPING,true);

    // Stencil test is off by default
    _stencil_test_enabled = false;
    _pD3DDevice->SetRenderState(D3DRS_STENCILENABLE, _stencil_test_enabled);

    // Antialiasing.
    enable_line_smooth(false);
//  enable_multisample(true);

    _current_fill_mode = RenderModeAttrib::M_filled;
    _pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    _pD3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);  // Use the diffuse vertex color.

    /*
      Panda no longer requires us to specify the maximum number of
      lights up front, but instead we can define slot_new_light() to
      decide one-at-a-time whether a particular light fits within our
      limit or not.  Until we override this function, there is no
      limit.

    if(_pScrn->d3dcaps.MaxActiveLights==0) {
        // 0 indicates no limit on # of lights, but we use DXGSG_MAX_LIGHTS anyway for now
      init_lights(DXGSG_MAX_LIGHTS);
    } else {
      init_lights(min(DXGSG_MAX_LIGHTS,_pScrn->d3dcaps.MaxActiveLights));
    }
    */

    // must do SetTSS here because redundant states are filtered out by our code based on current values above, so
    // initial conditions must be correct

    _CurTexBlendMode = TextureApplyAttrib::M_modulate;
    SetTextureBlendMode(_CurTexBlendMode,false);
    _texturing_enabled = false;
    _pD3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);  // disables texturing

    // Init more Texture State
    _CurTexMagFilter=_CurTexMinFilter=_CurTexMipFilter=D3DTEXF_NONE;
    _CurTexWrapModeU=_CurTexWrapModeV=Texture::WM_clamp;
    _CurTexAnisoDegree=1;

    // this code must match apply_texture() code for states above
    // so DX TSS renderstate matches dxgsg state

    _pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    _pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    _pD3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    _pD3DDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY,_CurTexAnisoDegree);
    _pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU,get_texture_wrap_mode(_CurTexWrapModeU));
    _pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV,get_texture_wrap_mode(_CurTexWrapModeV));

#ifdef _DEBUG
    if ((_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS) &&
        (dx_global_miplevel_bias!=0.0f)) {
        _pD3DDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *((LPDWORD) (&dx_global_miplevel_bias)) );
    }
#endif

#ifndef NDEBUG
    if(dx_force_backface_culling!=0) {
      if((dx_force_backface_culling > 0) &&
         (dx_force_backface_culling < D3DCULL_FORCE_DWORD)) {
             _pD3DDevice->SetRenderState(D3DRS_CULLMODE, dx_force_backface_culling);
      } else {
          dx_force_backface_culling=0;
          if(dxgsg9_cat.is_debug())
              dxgsg9_cat.debug() << "error, invalid value for dx-force-backface-culling\n";
      }
    }
    _pD3DDevice->SetRenderState(D3DRS_CULLMODE, dx_force_backface_culling);
#else
    _pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
#endif

    _alpha_func = D3DCMP_ALWAYS;
    _alpha_func_refval = 1.0f;
    _pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, _alpha_func);
    _pD3DDevice->SetRenderState(D3DRS_ALPHAREF, (UINT)(_alpha_func_refval*255.0f));
    _alpha_test_enabled = false;
    _pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, _alpha_test_enabled);

    // must check (_pScrn->d3dcaps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP) (yes on GF2/Radeon8500, no on TNT)
    _pD3DDevice->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);

    PRINT_REFCNT(dxgsg9,_pD3DDevice);

    // Make sure the DX state matches all of our initial attribute states.
    CPT(RenderAttrib) dta = DepthTestAttrib::make(DepthTestAttrib::M_less);
    CPT(RenderAttrib) dwa = DepthWriteAttrib::make(DepthWriteAttrib::M_on);
    CPT(RenderAttrib) cfa = CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise);

    dta->issue(this);
    dwa->issue(this);
    cfa->issue(this);

    PRINT_REFCNT(dxgsg9,_pD3DDevice);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::support_overlay_window
//       Access: Public
//  Description: Specifies whether dialog windows placed on top of the
//               dx rendering window should be supported.  This
//               requires a bit of extra overhead, so it should only
//               be activated when necessary; however, if it is not
//               activated, a window that pops up over the fullscreen
//               DX window (like a dialog box, or particularly like
//               the IME composition or candidate windows) may not be
//               visible.
//
//               This is not necessary when running in windowed mode,
//               but it does no harm.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
support_overlay_window(bool flag) {
  // How is this supposed to be done in DX9?
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::do_clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
do_clear(const RenderBuffer &buffer) {
  //    DO_PSTATS_STUFF(PStatTimer timer(_win->_clear_pcollector));

    nassertv(buffer._gsg == this);
    int buffer_type = buffer._buffer_type;

    DWORD flags = 0;

    if(buffer_type & RenderBuffer::T_depth) {
        flags |=  D3DCLEAR_ZBUFFER;
        assert(_pScrn->PresParams.EnableAutoDepthStencil);
    }

    if(buffer_type & RenderBuffer::T_back)       //set appropriate flags
        flags |=  D3DCLEAR_TARGET;

    if(buffer_type & RenderBuffer::T_stencil) {
        flags |=  D3DCLEAR_STENCIL;
        assert(_pScrn->PresParams.EnableAutoDepthStencil && IS_STENCIL_FORMAT(_pScrn->PresParams.AutoDepthStencilFormat));
    }

    HRESULT hr = _pD3DDevice->Clear(0, NULL, flags, _d3dcolor_clear_value,
                                         _depth_clear_value, (DWORD)_stencil_clear_value);
    if(FAILED(hr)) {
        dxgsg9_cat.error() << "clear_buffer failed:  Clear returned " << D3DERRORSTRING(hr);
        throw_event("panda3d-render-error");
    }
    /* The following line will cause the background to always clear to a medium red
    _color_clear_value[0] = .5;
    /* The following lines will cause the background color to cycle from black to red.
    _color_clear_value[0] += .001;
     if (_color_clear_value[0] > 1.0f) _color_clear_value[0] = 0.0f;
     */
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::prepare_display_region
//       Access: Public, Virtual
//  Description: Prepare a display region for rendering (set up
//       scissor region and viewport)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
prepare_display_region() {
  if (_current_display_region == (DisplayRegion*)0L) {
    dxgsg9_cat.error()
      << "Invalid NULL display region in prepare_display_region()\n";
  } else if (_current_display_region != _actual_display_region) {
    _actual_display_region = _current_display_region;
    
    int l, u, w, h;
    _actual_display_region->get_region_pixels_i(l, u, w, h);

    // Create the viewport
    D3DVIEWPORT9 vp = { l, u, w, h, 0.0f, 1.0f };
    HRESULT hr = _pD3DDevice->SetViewport( &vp );
    if (FAILED(hr)) {
      dxgsg9_cat.error()
        << "SetViewport(" << l << ", " << u << ", " << w << ", " << h
        << ") failed" << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
      nassertv(false);
    }
    // Note: for DX9, also change scissor clipping state here
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::prepare_lens
//       Access: Public, Virtual
//  Description: Makes the current lens (whichever lens was most
//               recently specified with set_scene()) active, so
//               that it will transform future rendered geometry.
//               Normally this is only called from the draw process,
//               and usually it is called by set_scene().
//
//               The return value is true if the lens is acceptable,
//               false if it is not.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian9::
prepare_lens() {
  if (_current_lens == (Lens *)NULL) {
    return false;
  }

  if (!_current_lens->is_linear()) {
    return false;
  }

  // Start with the projection matrix from the lens.
  const LMatrix4f &projection_mat = _current_lens->get_projection_mat();

  // The projection matrix must always be left-handed Y-up internally,
  // to match DirectX's convention, even if our coordinate system of
  // choice is otherwise.
  const LMatrix4f &convert_mat = 
    LMatrix4f::convert_mat(CS_yup_left, _current_lens->get_coordinate_system());

  // DirectX also uses a Z range of 0 to 1, whereas the Panda
  // convention is for the projection matrix to produce a Z range of
  // -1 to 1.  We have to rescale to compensate.
  static const LMatrix4f rescale_mat
    (1, 0, 0, 0,
     0, 1, 0, 0,
     0, 0, 0.5, 0,
     0, 0, 0.5, 1);
  
  LMatrix4f new_projection_mat =
    convert_mat * projection_mat * rescale_mat;

  if (_scene_setup->get_inverted()) {
    // If the scene is supposed to be inverted, then invert the
    // projection matrix.
    static LMatrix4f invert_mat = LMatrix4f::scale_mat(1.0f, -1.0f, 1.0f);
    new_projection_mat *= invert_mat;
  }

  HRESULT hr = 
    _pD3DDevice->SetTransform(D3DTS_PROJECTION,
                              (D3DMATRIX*)new_projection_mat.get_data());
  return SUCCEEDED(hr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::get_blend_func
//       Access: Protected, Static
//  Description: Maps from ColorBlendAttrib::Operand to D3DBLEND
//               value.
////////////////////////////////////////////////////////////////////
D3DBLEND DXGraphicsStateGuardian9::
get_blend_func(ColorBlendAttrib::Operand operand) {
  switch (operand) {
  case ColorBlendAttrib::O_zero:
    return D3DBLEND_ZERO;

  case ColorBlendAttrib::O_one:
    return D3DBLEND_ONE;

  case ColorBlendAttrib::O_incoming_color:
    return D3DBLEND_SRCCOLOR;

  case ColorBlendAttrib::O_one_minus_incoming_color:
    return D3DBLEND_INVSRCCOLOR;

  case ColorBlendAttrib::O_fbuffer_color:
    return D3DBLEND_DESTCOLOR;

  case ColorBlendAttrib::O_one_minus_fbuffer_color:
    return D3DBLEND_INVDESTCOLOR;

  case ColorBlendAttrib::O_incoming_alpha:
    return D3DBLEND_SRCALPHA;

  case ColorBlendAttrib::O_one_minus_incoming_alpha:
    return D3DBLEND_INVSRCALPHA;

  case ColorBlendAttrib::O_fbuffer_alpha:
    return D3DBLEND_DESTALPHA;

  case ColorBlendAttrib::O_one_minus_fbuffer_alpha:
    return D3DBLEND_INVDESTALPHA;

  case ColorBlendAttrib::O_constant_color:
    // Not supported by DX.
    return D3DBLEND_SRCCOLOR;

  case ColorBlendAttrib::O_one_minus_constant_color:
    // Not supported by DX.
    return D3DBLEND_INVSRCCOLOR;

  case ColorBlendAttrib::O_constant_alpha:
    // Not supported by DX.
    return D3DBLEND_SRCALPHA;

  case ColorBlendAttrib::O_one_minus_constant_alpha:
    // Not supported by DX.
    return D3DBLEND_INVSRCALPHA;

  case ColorBlendAttrib::O_incoming_color_saturate:
    return D3DBLEND_SRCALPHASAT;
  }

  dxgsg9_cat.error()
    << "Unknown color blend operand " << (int)operand << endl;
  return D3DBLEND_ZERO;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::report_texmgr_stats
//       Access: Protected
//  Description: Reports the DX texture manager's activity to PStats.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
report_texmgr_stats() {

#if defined(DO_PSTATS)||defined(PRINT_RESOURCESTATS)
  HRESULT hr;

#ifdef TEXMGRSTATS_USES_GETAVAILVIDMEM
  DWORD dwTexTotal,dwTexFree,dwVidTotal,dwVidFree;

#ifndef PRINT_RESOURCESTATS
  if (_total_texmem_pcollector.is_active())
#endif
  {
      DDSCAPS2 ddsCaps;

      ZeroMemory(&ddsCaps,sizeof(ddsCaps));

      ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
      if(FAILED(  hr = _pD3DDevice->GetAvailableVidMem(&ddsCaps,&dwVidTotal,&dwVidFree))) {
            dxgsg9_cat.fatal() << "report_texmgr GetAvailableVidMem for VIDMEM failed : result = " << D3DERRORSTRING(hr);
            exit(1);
      }

      ddsCaps.dwCaps = DDSCAPS_TEXTURE;
      if(FAILED(  hr = _pD3DDevice->GetAvailableVidMem(&ddsCaps,&dwTexTotal,&dwTexFree))) {
            dxgsg9_cat.fatal() << "report_texmgr GetAvailableVidMem for TEXTURE failed : result = " << D3DERRORSTRING(hr);
            exit(1);
      }
  }
#endif

  IDirect3DQuery9 *pQuery = NULL;
  D3DDEVINFO_RESOURCEMANAGER all_resource_stats;
  ZeroMemory(&all_resource_stats,sizeof(D3DDEVINFO_RESOURCEMANAGER));

  if(!bTexStatsRetrievalImpossible) {
    hr = _pD3DDevice->CreateQuery(D3DQUERYTYPE_RESOURCEMANAGER, &pQuery);
    if (hr == D3D_OK) {
      hr = pQuery->Issue(D3DISSUE_END);
    }
    if (hr == D3D_OK) {
      hr = pQuery->GetData((void*)&all_resource_stats,sizeof(D3DDEVINFO_RESOURCEMANAGER), 0);
    }
    if (hr!=D3D_OK) {
      if (hr==S_FALSE) {
        static int PrintedMsg=2;
        if(PrintedMsg>0) {
          if(dxgsg9_cat.is_debug())
            dxgsg9_cat.debug() << "Error: texstats GetInfo() requires debug DX DLLs to be installed!!  ***********\n";
          ZeroMemory(&all_resource_stats,sizeof(D3DDEVINFO_RESOURCEMANAGER));
          bTexStatsRetrievalImpossible=true;
        }
      } else {
        dxgsg9_cat.error() << "GetInfo(RESOURCEMANAGER) failed to get tex stats: result = " << D3DERRORSTRING(hr);
        bTexStatsRetrievalImpossible = true;
        return;
      }
    }
  }

#ifdef PRINT_RESOURCESTATS
#ifdef TEXMGRSTATS_USES_GETAVAILVIDMEM
    char tmpstr1[50],tmpstr2[50],tmpstr3[50],tmpstr4[50];
    sprintf(tmpstr1,"%.4g",dwVidTotal/1000000.0);
    sprintf(tmpstr2,"%.4g",dwVidFree/1000000.0);
    sprintf(tmpstr3,"%.4g",dwTexTotal/1000000.0);
    sprintf(tmpstr4,"%.4g",dwTexFree/1000000.0);
    dxgsg9_cat.debug() << "\nAvailableVidMem for RenderSurfs: (megs) total: " << tmpstr1 << "  free: " << tmpstr2
                      << "\nAvailableVidMem for Textures:    (megs) total: " << tmpstr3 << "  free: " << tmpstr4 << endl;
#endif

   #define REAL_D3DRTYPECOUNT ((UINT) D3DRTYPE_INDEXBUFFER)     // d3d boneheads defined D3DRTYPECOUNT wrong
   static char *ResourceNameStrs[REAL_D3DRTYPECOUNT]={"SURFACE","VOLUME","TEXTURE","VOLUME TEXTURE","CUBE TEXTURE","VERTEX BUFFER","INDEX BUFFER"};
   static bool bDoGetInfo[REAL_D3DRTYPECOUNT]={true,false,true,false,false,true,false};  // not using volume or cube textures yet

   if(!bTexStatsRetrievalImpossible) {
        for(UINT r=0; r<(UINT)REAL_D3DRTYPECOUNT;r++) {
            if(!bDoGetInfo[r])
               continue;

            D3DRESOURCESTATS *pRStats=&all_resource_stats.stats[r];
            if(pRStats->NumUsed>0) {
                char hitrate_str[20];
                float fHitRate = (pRStats->NumUsedInVidMem * 100.0f) / pRStats->NumUsed;
                sprintf(hitrate_str,"%.1f",fHitRate);

                dxgsg9_cat.spam()
                    << "\n***** Stats for " << ResourceNameStrs[r] << " ********"
                    << "\n HitRate:\t" << hitrate_str << "%"
                    << "\n bThrashing:\t" << pRStats->bThrashing
                    << "\n NumEvicts:\t" << pRStats->NumEvicts
                    << "\n NumVidCreates:\t" << pRStats->NumVidCreates
                    << "\n NumUsed:\t" << pRStats->NumUsed
                    << "\n NumUsedInVidMem:\t" << pRStats->NumUsedInVidMem
                    << "\n WorkingSet:\t" << pRStats->WorkingSet
                    << "\n WorkingSetBytes:\t" << pRStats->WorkingSetBytes
                    << "\n ApproxBytesDownloaded:\t" << pRStats->ApproxBytesDownloaded
                    << "\n TotalManaged:\t" << pRStats->TotalManaged
                    << "\n TotalBytes:\t" << pRStats->TotalBytes
                    << "\n LastPri:\t" << pRStats->LastPri << endl;
            } else {
                dxgsg9_cat.spam()
                    << "\n***** Stats for " << ResourceNameStrs[r] << " ********"
                    << "\n NumUsed: 0\n";
            }
        }

        D3DDEVINFO_D3DVERTEXSTATS vtxstats;
        ZeroMemory(&vtxstats,sizeof(D3DDEVINFO_D3DVERTEXSTATS));
        hr = _pD3DDevice->GetInfo(D3DDEVINFOID_VERTEXSTATS,&vtxstats,sizeof(D3DDEVINFO_D3DVERTEXSTATS));
        if (hr!=D3D_OK) {
            dxgsg9_cat.error() << "GetInfo(D3DVERTEXSTATS) failed : result = " << D3DERRORSTRING(hr);
            return;
        } else {
            dxgsg9_cat.spam()
            << "\n***** Triangle Stats ********"
            << "\n NumRenderedTriangles:\t" << vtxstats.NumRenderedTriangles
            << "\n NumExtraClippingTriangles:\t" << vtxstats.NumExtraClippingTriangles << endl;
        }
    }
#endif

#ifdef DO_PSTATS
  // Tell PStats about the state of the texture memory.

  if (_texmgrmem_total_pcollector.is_active()) {
      // report zero if no debug dlls, to signal this info is invalid
      _texmgrmem_total_pcollector.set_level(all_resource_stats.stats[D3DRTYPE_TEXTURE].TotalBytes);
      _texmgrmem_resident_pcollector.set_level(all_resource_stats.stats[D3DRTYPE_TEXTURE].WorkingSetBytes);
  }
#ifdef TEXMGRSTATS_USES_GETAVAILVIDMEM
  if (_total_texmem_pcollector.is_active()) {
    _total_texmem_pcollector.set_level(dwTexTotal);
    _used_texmem_pcollector.set_level(dwTexTotal - dwTexFree);
  }
#endif
#endif
#endif
}

// generates slightly fewer instrs
#define add_DWORD_to_FVFBuf(data) { *((DWORD *)_pCurFvfBufPtr) = (DWORD) data;  _pCurFvfBufPtr += sizeof(DWORD);}

typedef enum {
    FlatVerts,IndexedVerts,MixedFmtVerts
} GeomVertFormat;


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_prim_setup
//       Access: Private
//  Description: This adds data to the flexible vertex format
////////////////////////////////////////////////////////////////////
size_t DXGraphicsStateGuardian9::
draw_prim_setup(const Geom *geom) {
    //  Set the flags for the flexible vertex format and compute the bytes
    //  required to store a single vertex.
    // Assumes _perVertex,_perPrim,_perComp flags are setup prior to entry
    // (especially for shademode).  maybe should change this, since we usually
    // get attr info anyway)

    #ifdef _DEBUG
      assert(geom->get_binding(G_COORD) != G_OFF);
    #endif

#define GET_NEXT_VERTEX(NEXTVERT) { NEXTVERT = geom->get_next_vertex(vi); }
#define GET_NEXT_NORMAL() { p_normal = geom->get_next_normal(ni); }
#define GET_NEXT_TEXCOORD() { p_texcoord = geom->get_next_texcoord(ti); }

#define GET_NEXT_COLOR() {                                                           \
    Colorf tempcolor = geom->get_next_color(ci);                                     \
    if(!_color_scale_enabled) {                                              \
        _curD3Dcolor = Colorf_to_D3DCOLOR(tempcolor);                                \
    } else {                                                                         \
        transform_color(tempcolor,_curD3Dcolor);                                     \
    }}

////////

   // this stuff should eventually replace the iterators below
   PTA_Vertexf coords;
   PTA_ushort vindexes;

   geom->get_coords(coords,vindexes);
   if(vindexes!=NULL) {
      _pCurCoordIndex = _coordindex_array = &vindexes[0];
   } else {
      _pCurCoordIndex = _coordindex_array = NULL;
   }
   _pCurCoord = _coord_array = &coords[0];

   ///////////////

   vi = geom->make_vertex_iterator();
   DWORD newFVFflags = D3DFVF_XYZ;
   size_t vertex_size = sizeof(float) * 3;

   GeomBindType ColorBinding=geom->get_binding(G_COLOR);
   bool bDoColor=(ColorBinding != G_OFF);

   if (bDoColor || _has_scene_graph_color) {
        ci = geom->make_color_iterator();
        newFVFflags |= D3DFVF_DIFFUSE;
        vertex_size += sizeof(D3DCOLOR);

        if (_has_scene_graph_color) {
            if (_scene_graph_color_stale) {
              // Compute the D3DCOLOR for the scene graph override color.
              if(!_color_scale_enabled) {
                _scene_graph_color_D3DCOLOR = Colorf_to_D3DCOLOR(_scene_graph_color);
              } else {
                transform_color(_scene_graph_color, _scene_graph_color_D3DCOLOR);
              }
              _scene_graph_color_stale = false;
            }
            _curD3Dcolor = _scene_graph_color_D3DCOLOR;  // set primitive color if there is one.

            _perVertex &= ~PER_COLOR;
            _perPrim &= ~PER_COLOR;
            _perComp &= ~PER_COLOR;
         } else if(ColorBinding == G_OVERALL){
            GET_NEXT_COLOR();
            _perVertex &= ~PER_COLOR;
            _perPrim &= ~PER_COLOR;
            _perComp &= ~PER_COLOR;
        }
   }
   if (geom->get_binding(G_NORMAL) != G_OFF) {
        ni = geom->make_normal_iterator();
        newFVFflags |= D3DFVF_NORMAL;
        vertex_size += sizeof(float) * 3;

        if (geom->get_binding(G_NORMAL) == G_OVERALL)
            p_normal = geom->get_next_normal(ni);    // set overall normal if there is one
   }


   GeomBindType TexCoordBinding;
   PTA_TexCoordf texcoords;
   PTA_ushort tindexes;
   geom->get_texcoords(texcoords,TexCoordBinding,tindexes);
   if (TexCoordBinding != G_OFF) {
       assert(TexCoordBinding == G_PER_VERTEX);

       // used by faster path
       if(tindexes!=NULL) {
          _pCurTexCoordIndex = _texcoordindex_array = &tindexes[0];
       } else {
          _pCurTexCoordIndex = _texcoordindex_array = NULL;
       }
       _pCurTexCoord = _texcoord_array = &texcoords[0];
       //////

       ti = geom->make_texcoord_iterator();
        newFVFflags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
       vertex_size += sizeof(float) * 2;
   }

    // If we have per-vertex colors or normals, we need smooth shading.
    // Otherwise we want flat shading for performance reasons.

   // Note on fogging:
   // the fogging expression should really be || (_fog_enabled && (_doFogType==PerVertexFog))
   // instead of just || (_fog_enabled), since GOURAUD shading should not be required for PerPixel
   // fog, but the problem is some cards (Riva128,Matrox G200) emulate pixel fog with table fog
   // but dont force the shading mode to gouraud internally, so you end up with flat-shaded fog colors
   // (note, TNT does the right thing tho).  So I guess we must do gouraud shading for all fog rendering for now
   // note that if _doFogType==None, _fog_enabled will always be false

   bool need_gouraud_shading = ((_perVertex & (PER_COLOR | (wants_normals() ? PER_NORMAL : 0))) || _fog_enabled);

   enable_gouraud_shading(need_gouraud_shading);
   set_vertex_format(newFVFflags);

   return vertex_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_prim_inner_loop
//       Access: Private
//  Description: This adds data to the flexible vertex format with a check
//               for component normals and color
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_prim_inner_loop(int nVerts, const Geom *geom, ushort perFlags) {
    Vertexf NextVert;

    for(;nVerts > 0;nVerts--) {
         // coord info will always be _perVertex
        GET_NEXT_VERTEX(NextVert);     // need to optimize these
        add_to_FVFBuf((void *)&NextVert, 3*sizeof(float));

        if(perFlags==(ushort)TexCoordOnly) {
            // break out the common case (for animated chars) 1st
            GET_NEXT_TEXCOORD();
        } else {
            switch (DrawLoopFlags(perFlags)) {
                case Color_TexCoord:
                    GET_NEXT_TEXCOORD();
                case ColorOnly:
                    GET_NEXT_COLOR();
                    break;
                case Normal_Color:
                    GET_NEXT_COLOR();
                case NormalOnly:
                    GET_NEXT_NORMAL();
                    break;
                case Normal_Color_TexCoord:
                    GET_NEXT_COLOR();
                case Normal_TexCoord:
                    GET_NEXT_NORMAL();
                // case TexCoordOnly:
                    GET_NEXT_TEXCOORD();
                    break;
            }
        }

        if (_CurFVFType & D3DFVF_NORMAL)
            add_to_FVFBuf((void *)&p_normal, 3*sizeof(float));
        if (_CurFVFType & D3DFVF_DIFFUSE)
            add_DWORD_to_FVFBuf(_curD3Dcolor);
        if (_CurFVFType & D3DFVF_TEXCOUNT_MASK)
            add_to_FVFBuf((void *)&p_texcoord, sizeof(TexCoordf));
    }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_prim_inner_loop_coordtexonly
//       Access: Private
//  Description: FastPath loop used by animated character data
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_prim_inner_loop_coordtexonly(int nVerts, const Geom *geom) {
    // assumes coord and texcoord data is per-vertex,
    // color is not per-vert/component (which would require fetching new vals in the vertex loop),
    // and no normal data. this should be common situation for animated character data
    // inc'ing local ptrs instead of member ones, seems to optimize better
    // bypass all the slow vertex iterator stuff

    #ifdef _DEBUG
     {
      assert(geom->get_binding(G_NORMAL) == G_OFF);
      GeomBindType ColorBinding = geom->get_binding(G_COLOR);
      assert((ColorBinding != G_PER_VERTEX) || (ColorBinding != G_PER_COMPONENT));
      assert(geom->get_binding(G_TEXCOORD) == G_PER_VERTEX);
     }
    #endif

    Vertexf *pCurCoord = _pCurCoord;
    ushort *pCurCoordIndex = _pCurCoordIndex;
    TexCoordf *pCurTexCoord = _pCurTexCoord;
    ushort *pCurTexCoordIndex = _pCurTexCoordIndex;

    BYTE *pLocalFvfBufPtr = _pCurFvfBufPtr;
    DWORD cur_color = _curD3Dcolor;
    bool bDoIndexedTexCoords = (_texcoordindex_array != NULL);
    bool bDoIndexedCoords = (_coordindex_array != NULL);

    for(;nVerts>0;nVerts--) {
        if(bDoIndexedCoords) {
           memcpy(pLocalFvfBufPtr,(void*)&_coord_array[*pCurCoordIndex],3*sizeof(float));
           pCurCoordIndex++;
        } else {
           memcpy(pLocalFvfBufPtr,(void*)pCurCoord,3*sizeof(float));
           pCurCoord++;
        }

        pLocalFvfBufPtr+=3*sizeof(float);

        *((DWORD *)pLocalFvfBufPtr) = cur_color;
        pLocalFvfBufPtr += sizeof(DWORD);

        if(bDoIndexedTexCoords) {
           memcpy(pLocalFvfBufPtr,(void*)&_texcoord_array[*pCurTexCoordIndex],sizeof(TexCoordf));
           pCurTexCoordIndex++;
        } else {
           memcpy(pLocalFvfBufPtr,(void*)pCurTexCoord,sizeof(TexCoordf));
           pCurTexCoord++;
        }
        pLocalFvfBufPtr+=sizeof(TexCoordf);
    }

    _pCurFvfBufPtr=pLocalFvfBufPtr;
    _pCurCoord = pCurCoord;
    _pCurCoordIndex = pCurCoordIndex;
    _pCurTexCoord = pCurTexCoord;
    _pCurTexCoordIndex = pCurTexCoordIndex;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_point
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_point(GeomPoint *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg9_cat.debug() << "draw_point()" << endl;
#endif

    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    // The DX Way

    int nPrims = geom->get_num_prims();

    if (nPrims==0) {
        dxgsg9_cat.warning() << "draw_point() called with ZERO vertices!!" << endl;
        return;
    }

    //#ifdef _DEBUG
    //    static bool bPrintedMsg=false;
    //
    //    if (!bPrintedMsg && (geom->get_size()!=1.0f)) {
    //        bPrintedMsg=true;
    //        dxgsg9_cat.warning() << "D3D does not support drawing points of non-unit size, setting point size to 1.0f!\n";
    //    }
    //#endif

    nassertv(nPrims < PANDA_MAXNUMVERTS );

    PTA_Vertexf coords;
    PTA_Normalf norms;
    PTA_Colorf colors;
    PTA_TexCoordf texcoords;
    GeomBindType bind;
    PTA_ushort vindexes,nindexes,tindexes,cindexes;

    geom->get_coords(coords,vindexes);
    geom->get_normals(norms,bind,nindexes);
    geom->get_colors(colors,bind,cindexes);
    geom->get_texcoords(texcoords,bind,tindexes);

    // for Indexed Prims and mixed indexed/non-indexed prims, we will use old pipeline for now
    // need to add code to handle fully indexed mode (and handle cases with index arrays of different lengths,
    // values (may only be possible to handle certain cases without reverting to old pipeline)

        _perVertex = 0x0;
        _perPrim = 0;
        if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) _perVertex |= PER_NORMAL;
        if (geom->get_binding(G_COLOR) == G_PER_VERTEX) _perVertex |= PER_COLOR;

        size_t vertex_size = draw_prim_setup(geom);

        nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
        nassertv(nPrims * vertex_size < VERT_BUFFER_SIZE);
        _pCurFvfBufPtr = _pFvfBufBasePtr;          // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

        // iterate through the point
        draw_prim_inner_loop(nPrims, geom, _perVertex | _perPrim);

        HRESULT hr = _pD3DDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nPrims, _pFvfBufBasePtr, vertex_size);
        TestDrawPrimFailure(DrawPrim,hr,_pD3DDevice,nPrims,0);

    _pCurFvfBufPtr = NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_line(GeomLine* geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg9_cat.debug() << "draw_line()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    //#ifdef _DEBUG
    //    static bool bPrintedMsg=false;
    //
    //    // note: need to implement approximation of non-1.0 width lines with quads
    //
    //    if (!bPrintedMsg && (geom->get_width()!=1.0f)) {
    //        bPrintedMsg=true;
    //        if(dxgsg9_cat.is_debug())
    //            dxgsg9_cat.debug() << "DX does not support drawing lines with a non-1.0f pixel width, setting width to 1.0f!\n";
    //    }
    //#endif

    int nPrims = geom->get_num_prims();

    if (nPrims==0) {
        if(dxgsg9_cat.is_debug())
           dxgsg9_cat.debug() << "draw_line() called with ZERO vertices!!" << endl;
        return;
    }

    _perVertex = 0x0;
    _perPrim = 0x0;
    _perComp = 0x0;

    switch(geom->get_binding(G_COLOR)) {
        case G_PER_VERTEX:
            _perVertex |=  PER_COLOR;
            break;
        case G_PER_COMPONENT:
            _perComp |= PER_COLOR;
            break;
        default:
            _perPrim |= PER_COLOR;
    }

    switch(geom->get_binding(G_NORMAL)) {
        case G_PER_VERTEX:
            _perVertex |=  PER_NORMAL;
            break;
        case G_PER_COMPONENT:
            _perComp |=  PER_NORMAL;
            break;
        default:
            _perPrim |=  PER_NORMAL;
    }

    size_t vertex_size = draw_prim_setup(geom);

    BYTE *_tmp_fvfOverrunBuf = NULL;
    nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
//  nassertv(nPrims * 2 * vertex_size < VERT_BUFFER_SIZE);

    if (nPrims * 2 * vertex_size > VERT_BUFFER_SIZE) {
        // bugbug: need cleaner way to handle tmp buffer size overruns (malloc/realloc?)
        _pCurFvfBufPtr = _tmp_fvfOverrunBuf = new BYTE[nPrims * 2 * vertex_size];
    } else  _pCurFvfBufPtr = _pFvfBufBasePtr;            // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

    for (int i = 0; i < nPrims; i++) {
        if (_perPrim & PER_COLOR) {
            GET_NEXT_COLOR();
        }
        if (_perPrim & PER_NORMAL)
            p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.
        draw_prim_inner_loop(2, geom, _perVertex);
    }

    HRESULT hr;

    DWORD nVerts = nPrims<<1;

    if (_tmp_fvfOverrunBuf == NULL) {
      nassertv((nVerts*vertex_size) == (_pCurFvfBufPtr-_pFvfBufBasePtr));
      hr = _pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, nPrims, _pFvfBufBasePtr, vertex_size);
    } else {
      nassertv((nVerts*vertex_size) == (_pCurFvfBufPtr-_tmp_fvfOverrunBuf));
      hr = _pD3DDevice->DrawPrimitiveUP(D3DPT_LINELIST, nPrims, _tmp_fvfOverrunBuf, vertex_size);
      delete [] _tmp_fvfOverrunBuf;
    }
    TestDrawPrimFailure(DrawPrim,hr,_pD3DDevice,nVerts,0);

    _pCurFvfBufPtr = NULL;
}

void DXGraphicsStateGuardian9::
draw_linestrip(GeomLinestrip* geom, GeomContext *gc) {

  //#ifdef _DEBUG
  //    static BOOL bPrintedMsg=false;
  //
  //    if (!bPrintedMsg && (geom->get_width()!=1.0f)) {
  //        bPrintedMsg=true;
  //        dxgsg9_cat.warning() << "DX does not support drawing lines with a non-1.0f pixel width, setting width to 1.0f!\n";
  //    }
  //#endif

  draw_linestrip_base(geom,gc,false);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_linestrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_linestrip_base(Geom* geom, GeomContext *gc, bool bConnectEnds) {
// Note draw_linestrip_base() may be called from non-line draw_fns to support wireframe mode

#ifdef GSG_VERBOSE
    dxgsg9_cat.debug() << "draw_linestrip()" << endl;
#endif

    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    int nPrims = geom->get_num_prims();
    const int *pLengthArr = geom->get_lengths();

    if(nPrims==0) {
        if(dxgsg9_cat.is_debug())
            dxgsg9_cat.debug() << "draw_linestrip() called with ZERO vertices!!" << endl;
        return;
    }

    _perVertex = 0x0;
    _perPrim = 0x0;
    _perComp = 0x0;

    switch(geom->get_binding(G_COLOR)) {
        case G_PER_VERTEX:
            _perVertex |= PER_COLOR;
            break;
        case G_PER_COMPONENT:
            _perComp |= PER_COLOR;
            break;
        default:
            _perPrim |= PER_COLOR;
    }

    switch(geom->get_binding(G_NORMAL)) {
        case G_PER_VERTEX:
            _perVertex |=  PER_NORMAL;
            break;
        case G_PER_COMPONENT:
            _perComp |=  PER_NORMAL;
            break;
        default:
            _perPrim |= PER_NORMAL;
    }

    size_t vertex_size = draw_prim_setup(geom);
    ushort perFlags = _perVertex | _perComp;

    bool bPerPrimColor = ((_perPrim & PER_COLOR)!=0);
    bool bPerPrimNormal = ((_perPrim & PER_NORMAL)!=0);

    DWORD nVerts;

    if(pLengthArr==NULL) // we've been called by draw_quad, which has no lengths array
      nVerts=4;

    for (int i = 0; i < nPrims; i++) {
        if (bPerPrimColor) {
            GET_NEXT_COLOR();
        }

        if (bPerPrimNormal) {
            p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.
        }

        if(pLengthArr!=NULL) {
            nVerts= *(pLengthArr++);
            nassertv(nVerts >= 2);
        }

        nassertv(_pCurFvfBufPtr == NULL);   // make sure the storage pointer is clean.
        nassertv(nVerts * vertex_size < VERT_BUFFER_SIZE);
        _pCurFvfBufPtr = _pFvfBufBasePtr;   // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

        draw_prim_inner_loop(nVerts, geom, perFlags);

        if(bConnectEnds) {
             // append first vertex to end
             memcpy(_pCurFvfBufPtr,_pFvfBufBasePtr,vertex_size);
             _pCurFvfBufPtr+=vertex_size;
             nVerts++;
        }

        nassertv((nVerts*vertex_size) == (_pCurFvfBufPtr-_pFvfBufBasePtr));

        HRESULT hr = _pD3DDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, nVerts-1, _pFvfBufBasePtr, vertex_size);
        TestDrawPrimFailure(DrawPrim,hr,_pD3DDevice,nVerts,0);

        _pCurFvfBufPtr = NULL;
    }
}

// this class exists because an alpha sort is necessary for correct
// sprite rendering, and we can't simply sort the vertex arrays as
// each vertex may or may not have corresponding information in the
// x/y texel-world-ratio and rotation arrays.
typedef struct {
    Vertexf _v;
    D3DCOLOR _c;
    float _x_ratio;
    float _y_ratio;
    float _theta;
} WrappedSprite;

class WrappedSpriteSortPtr {
public:
    float z;
    WrappedSprite *pSpr;
};

// this struct exists because the STL can sort faster than i can.
struct draw_sprite_vertex_less {
    INLINE bool operator ()(const WrappedSpriteSortPtr& v0,
                            const WrappedSpriteSortPtr& v1) const {
        return v0.z > v1.z; // reversed from gl due to left-handed coordsys of d3d
    }
};

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_sprite
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_sprite(GeomSprite *geom, GeomContext *gc) {

    // this is a little bit of a mess, but it's ok.  Here's the deal:
    // we want to draw, and draw quickly, an arbitrarily large number
    // of sprites all facing the screen.  Performing the billboard math
    // for ~1000 sprites is way too slow.  Ideally, we want one
    // matrix transformation that will handle everything, and this is
    // just about what ends up happening. We're getting the front-facing
    // effect by setting up a new frustum (of the same z-depth as the
    // current one) that is very small in x and y.  This way regularly
    // rendered triangles that might not be EXACTLY facing the camera
    // will certainly look close enough.  Then, we transform to camera-space
    // by hand and apply the inverse frustum to the transformed point.
    // For some cracked out reason, this actually works.


    // Note: for DX9, try to use the PointSprite primitive instead of doing all the stuff below

#ifdef GSG_VERBOSE
    dxgsg9_cat.debug() << "draw_sprite()" << endl;
#endif
    // get the array traversal set up.
    int nPrims = geom->get_num_prims();

    if (nPrims==0) {
        return;
    }

    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));

    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(nPrims));

    D3DMATRIX OldD3DWorldMatrix;
    _pD3DDevice->GetTransform(D3DTS_WORLD, &OldD3DWorldMatrix);

    bool bReEnableDither=false;

    _pD3DDevice->GetTransform(D3DTS_WORLD, &OldD3DWorldMatrix);

    Geom::VertexIterator vi = geom->make_vertex_iterator();
    Geom::ColorIterator ci = geom->make_color_iterator();

    // note although sprite particles technically dont require a texture,
    // the texture dimensions are used to initialize the size calculations
    // the code in spriteParticleRenderer.cxx does not handle the no-texture case now

    float tex_xsize = 1.0f;
    float tex_ysize = 1.0f;

    Texture *tex = geom->get_texture();
    if(tex !=NULL) {
      // set up the texture-rendering state
      modify_state(RenderState::make
                   (TextureAttrib::make(tex),
                    TextureApplyAttrib::make(TextureApplyAttrib::M_modulate)));
      tex_xsize = tex->get_x_size();
      tex_ysize = tex->get_y_size();
    }

    // save the modelview matrix
    const LMatrix4f &modelview_mat = _transform->get_mat();

    // We don't need to mess with the aspect ratio, since we are now
    // using the default projection matrix, which has the right aspect
    // ratio built in.

    // null the world xform, so sprites are orthog to scrn
    _pD3DDevice->SetTransform(D3DTS_WORLD, &matIdentity);
    // only need to change _WORLD xform, _VIEW xform is Identity

    // precomputation stuff
    float tex_left = geom->get_ll_uv()[0];
    float tex_right = geom->get_ur_uv()[0];
    float tex_bottom = geom->get_ll_uv()[1];
    float tex_top = geom->get_ur_uv()[1];

    float half_width =  0.5f * tex_xsize * fabs(tex_right - tex_left);
    float half_height = 0.5f * tex_ysize * fabs(tex_top - tex_bottom);
    float scaled_width, scaled_height;

    // the user can override alpha sorting if they want
    bool alpha = false;

    if (!geom->get_alpha_disable()) {
      // figure out if alpha's enabled (if not, no reason to sort)
      const TransparencyAttrib *trans = _state->get_transparency();
      if (trans != (const TransparencyAttrib *)NULL) {
        alpha = (trans->get_mode() != TransparencyAttrib::M_none);
      }
    }

    // inner loop vars
    int i;
    Vertexf source_vert, cameraspace_vert;
    float *x_walk, *y_walk, *theta_walk;
    float theta;

    nassertv(geom->get_x_bind_type() != G_PER_VERTEX);
    nassertv(geom->get_y_bind_type() != G_PER_VERTEX);

    // set up the non-built-in bindings
    bool x_overall = (geom->get_x_bind_type() == G_OVERALL);
    bool y_overall = (geom->get_y_bind_type() == G_OVERALL);
    bool theta_overall = (geom->get_theta_bind_type() == G_OVERALL);
    bool color_overall = (geom->get_binding(G_COLOR) == G_OVERALL);
    bool theta_on = !(geom->get_theta_bind_type() == G_OFF);

    // x direction
    if (x_overall)
        scaled_width = geom->_x_texel_ratio[0] * half_width;
    else {
        nassertv(((int)geom->_x_texel_ratio.size() >= geom->get_num_prims()));
        x_walk = &geom->_x_texel_ratio[0];
    }

    // y direction
    if (y_overall)
        scaled_height = geom->_y_texel_ratio[0] * half_height;
    else {
        nassertv(((int)geom->_y_texel_ratio.size() >= geom->get_num_prims()));
        y_walk = &geom->_y_texel_ratio[0];
    }

    // theta
    if (theta_on) {
        if (theta_overall)
            theta = geom->_theta[0];
        else {
            nassertv(((int)geom->_theta.size() >= geom->get_num_prims()));
            theta_walk = &geom->_theta[0];
        }
    }

    /////////////////////////////////////////////////////////////////////
    // INNER LOOP PART 1 STARTS HERE
    // Here we transform each point to cameraspace and fill our sort
    // vector with the final geometric information.
    /////////////////////////////////////////////////////////////////////

    Colorf v_color;

    // sort container and iterator
    pvector< WrappedSpriteSortPtr > sorted_sprite_vector;
    pvector< WrappedSpriteSortPtr >::iterator sorted_vec_iter;

    WrappedSprite *SpriteArray = new WrappedSprite[nPrims];

    //BUGBUG: could we use _fvfbuf for this to avoid perframe alloc?
    // alternately, alloc once when retained mode becomes available

    if (SpriteArray==NULL) {
        dxgsg9_cat.fatal() << "draw_sprite() out of memory!!" << endl;
        return;
    }

    // the state is set, start running the prims

    WrappedSprite *pSpr;

    for (pSpr=SpriteArray,i = 0; i < nPrims; i++,pSpr++) {

        source_vert = geom->get_next_vertex(vi);
        cameraspace_vert = source_vert * modelview_mat;

        pSpr->_v.set(cameraspace_vert[0],cameraspace_vert[1],cameraspace_vert[2]);

        if (!color_overall) {
            GET_NEXT_COLOR();
            pSpr->_c = _curD3Dcolor;
        }
        if (!x_overall)
            pSpr->_x_ratio = *x_walk++;
        if (!y_overall)
            pSpr->_y_ratio = *y_walk++;    // go along array of ratio values stored in geom
        if (theta_on && (!theta_overall))
            pSpr->_theta = *theta_walk++;
    }

    if (alpha) {
        sorted_sprite_vector.reserve(nPrims);   //pre-alloc space for nPrims

        for (pSpr=SpriteArray,i = 0; i < nPrims; i++,pSpr++) {   // build STL-sortable array
            WrappedSpriteSortPtr ws_ptr;
            ws_ptr.z=pSpr->_v[2];
            ws_ptr.pSpr=pSpr;
            sorted_sprite_vector.push_back(ws_ptr);
        }

        // sort the verts properly by alpha (if necessary).  Of course,
        // the sort is only local, not scene-global, so if you look closely you'll
        // notice that alphas may be screwy.  It's ok though, because this is fast.
        // if you want accuracy, use billboards and take the speed hit.

        sort(sorted_sprite_vector.begin(), sorted_sprite_vector.end(), draw_sprite_vertex_less());
        sorted_vec_iter = sorted_sprite_vector.begin();

        // disabling dither for alpha particle-systems.
        // ATI sez:  most applications ignore the fact that since alpha blended primitives
        // combine the data in the frame buffer with the data in the current pixel, pixels
        // can be dithered multiple times and accentuate the dither pattern. This is particularly
        // true in particle systems which rely on the cumulative visual effect of many overlapping
        // alpha blended primitives.

        if(_dither_enabled) {
            bReEnableDither=true;
            enable_dither(false);
        }
    }

    Vertexf ul, ur, ll, lr;

    ////////////////////////////////////////////////////////////////////////////
    // INNER LOOP PART 2 STARTS HERE
    // Now we run through the cameraspace vector and compute the geometry for each
    // tristrip.  This includes scaling as per the ratio arrays, as well as
    // rotating in the z.
    ////////////////////////////////////////////////////////////////////////////

    D3DCOLOR CurColor;
    DWORD FVFType = D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) | D3DFVF_DIFFUSE;
    DWORD vertex_size = sizeof(float) * 2 + sizeof(float) * 3 + sizeof(D3DCOLOR);

    if (color_overall) {
        GET_NEXT_COLOR();
        CurColor = _curD3Dcolor;
    }

    // see note on fog and gouraud-shading in draw_prim_setup
    bool bUseGouraudShadedColor=_fog_enabled;
    enable_gouraud_shading(_fog_enabled);
    set_vertex_format(FVFType);

    #ifdef _DEBUG
     nassertv(_pCurFvfBufPtr == NULL);   // make sure the storage pointer is clean.
     nassertv(nPrims * 4 * vertex_size < VERT_BUFFER_SIZE);
     nassertv(nPrims * 6 < PANDA_MAXNUMVERTS );
    #endif

    _pCurFvfBufPtr = _pFvfBufBasePtr;          // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

    const float TexCrdSets[4][2] = {
      { tex_left, tex_bottom },
      { tex_right, tex_bottom },
      { tex_left, tex_top },
      { tex_right, tex_top }
    };

#define QUADVERTLISTLEN 6

    DWORD QuadVertIndexList[QUADVERTLISTLEN] = { 0, 1, 2, 3, 2, 1};
    DWORD CurDPIndexArrLength=0,CurVertCount=0;

    for (pSpr=SpriteArray,i = 0; i < nPrims; i++,pSpr++) {   // build STL-sortable array

        if (alpha) {
            pSpr = sorted_vec_iter->pSpr;
            sorted_vec_iter++;
        }

        // if not G_OVERALL, calculate the scale factors    //huh??
        if (!x_overall)
            scaled_width = pSpr->_x_ratio * half_width;

        if (!y_overall)
            scaled_height = pSpr->_y_ratio * half_height;

        // if not G_OVERALL, do some trig for this z rotate   //what is the theta angle??
        if (theta_on) {
            if (!theta_overall)
                theta = pSpr->_theta;

            // create the rotated points.  BUGBUG: this matmult will be slow if we dont get inlining
            // rotate_mat calls sin() on an unbounded val, possible to make it faster with lookup table (modulate to 0-360 range?)

            LMatrix3f xform_mat = LMatrix3f::rotate_mat(theta) *
                                  LMatrix3f::scale_mat(scaled_width, scaled_height);

            ur = (LVector3f( 1.0f,  1.0f, 0.0f) * xform_mat) + pSpr->_v;
            ul = (LVector3f(-1.0f,  1.0f, 0.0f) * xform_mat) + pSpr->_v;
            lr = (LVector3f( 1.0f, -1.0f, 0.0f) * xform_mat) + pSpr->_v;
            ll = (LVector3f(-1.0f, -1.0f, 0.0f) * xform_mat) + pSpr->_v;
        } else {
            // create points for unrotated rect sprites
            float x,y,negx,negy,z;

            x = pSpr->_v[0] + scaled_width;
            y = pSpr->_v[1] + scaled_height;
            negx = pSpr->_v[0] - scaled_width;
            negy = pSpr->_v[1] - scaled_height;
            z = pSpr->_v[2];

            ur.set(x, y, z);
            ul.set(negx, y, z);
            lr.set(x, negy, z);
            ll.set(negx, negy, z);
        }

        // can no longer assume flat-shaded (because of vtx fog), so always copy full color in there

        /*********  LL vertex  **********/

        add_to_FVFBuf((void *)ll.get_data(), 3*sizeof(float));
        if (!color_overall)  // otherwise its already been set globally
           CurColor = pSpr->_c;
        add_DWORD_to_FVFBuf(CurColor); // only need to cpy color on 1st vert, others are just empty ignored space
        add_to_FVFBuf((void *)TexCrdSets[0], sizeof(float)*2);

        /*********  LR vertex  **********/

        add_to_FVFBuf((void *)lr.get_data(), 3*sizeof(float));

        // if flat shading, dont need to write color for middle vtx, just incr ptr
        if(bUseGouraudShadedColor)
            *((DWORD *)_pCurFvfBufPtr) = (DWORD) CurColor;
        _pCurFvfBufPtr += sizeof(D3DCOLOR);

        add_to_FVFBuf((void *)TexCrdSets[1], sizeof(float)*2);

        /*********  UL vertex  **********/

        add_to_FVFBuf((void *)ul.get_data(), 3*sizeof(float));
        // if flat shading, dont need to write color for middle vtx, just incr ptr
        if(bUseGouraudShadedColor)
            *((DWORD *)_pCurFvfBufPtr) = (DWORD) CurColor;
        _pCurFvfBufPtr += sizeof(D3DCOLOR);
        add_to_FVFBuf((void *)TexCrdSets[2], sizeof(float)*2);

        /*********  UR vertex  **********/

        add_to_FVFBuf((void *)ur.get_data(), 3*sizeof(float));
        add_DWORD_to_FVFBuf(CurColor);
        add_to_FVFBuf((void *)TexCrdSets[3], sizeof(float)*2);

        for (int ii=0;ii<QUADVERTLISTLEN;ii++) {
            _index_buf[CurDPIndexArrLength+ii]=QuadVertIndexList[ii]+CurVertCount;
        }
        CurDPIndexArrLength+=QUADVERTLISTLEN;
        CurVertCount+=4;
    }

    DWORD nVerts= nPrims << 2;  // 4*nPrims verts in vert array
    DWORD numTris = nPrims << 1;  // 2*nPrims

    // cant do tristrip/fan since multiple quads arent connected
    // best we can do is indexed primitive, which sends 2 redundant indices instead of sending 2 redundant full verts
    HRESULT hr = _pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0,  // start index in array
                                                         nVerts, numTris,
                                                         _index_buf, D3DFMT_INDEX16,
                                                         _pFvfBufBasePtr, vertex_size);
    TestDrawPrimFailure(DrawIndexedPrim,hr,_pD3DDevice,QUADVERTLISTLEN*nPrims,numTris);

    _pCurFvfBufPtr = NULL;
    delete [] SpriteArray;

    // restore the matrices
    _pD3DDevice->SetTransform(D3DTS_WORLD,
                                  (D3DMATRIX*)modelview_mat.get_data());

    if(bReEnableDither)
        enable_dither(true);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_polygon
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_polygon(GeomPolygon *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
   dxgsg9_cat.debug() << "draw_polygon()" << endl;
#endif
   DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
   DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

   // wireframe polygon will be drawn as linestrip, otherwise draw as multi-tri trifan
   DWORD rstate;
   _pD3DDevice->GetRenderState(D3DRS_FILLMODE, &rstate);
   if(rstate==D3DFILL_WIREFRAME) {
       draw_linestrip_base(geom,gc,true);
   } else {
       draw_multitri(geom, D3DPT_TRIANGLEFAN);
   }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_quad
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_quad(GeomQuad *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg9_cat.debug() << "draw_quad()" << endl;
#endif
   DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
   DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

   // wireframe quad will be drawn as linestrip, otherwise draw as multi-tri trifan
   DWORD rstate;
   _pD3DDevice->GetRenderState(D3DRS_FILLMODE, &rstate);
   if(rstate==D3DFILL_WIREFRAME) {
       draw_linestrip_base(geom,gc,true);
   } else {
       draw_multitri(geom, D3DPT_TRIANGLEFAN);
   }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_tri
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_tri(GeomTri *geom, GeomContext *gc) {
#ifdef GSG_VERBOSE
    dxgsg9_cat.debug() << "draw_tri()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_tri_pcollector.add_level(geom->get_num_vertices()));

#if 0
    if (_pCurTexContext!=NULL) {
        dxgsg9_cat.spam() << "Cur active DX texture: " << _pCurTexContext->_tex->get_name() << "\n";
    }
#endif

#ifdef COUNT_DRAWPRIMS
    cGeomcount++;
#endif

    DWORD nPrims = geom->get_num_prims();
    HRESULT hr;

    PTA_Vertexf coords;
    PTA_Normalf norms;
    PTA_Colorf colors;
    PTA_TexCoordf texcoords;
    GeomBindType TexCoordBinding,ColorBinding,NormalBinding;
    PTA_ushort vindexes,nindexes,tindexes,cindexes;

    geom->get_coords(coords,vindexes);
    geom->get_normals(norms,NormalBinding,nindexes);
    geom->get_colors(colors,ColorBinding,cindexes);
    geom->get_texcoords(texcoords,TexCoordBinding,tindexes);

    // this is the old geom setup, it reformats every vtx into an output array passed to d3d

    _perVertex = 0x0;
    _perPrim = 0x0;
    
    bool bUseTexCoordOnlyLoop = ((ColorBinding != G_PER_VERTEX) &&
                                 (NormalBinding == G_OFF) &&
                                 (TexCoordBinding != G_OFF));
    
    bool bPerPrimNormal;
    
    bool bPerPrimColor=(ColorBinding == G_PER_PRIM);
    if(bPerPrimColor)
      _perPrim = PER_COLOR;
    else if(ColorBinding == G_PER_VERTEX)
      _perVertex = PER_COLOR;
    
    if(bUseTexCoordOnlyLoop) {
      _perVertex |= PER_TEXCOORD;  // TexCoords are either G_OFF or G_PER_VERTEX
    } else {
      if(NormalBinding == G_PER_VERTEX)
        _perVertex |= PER_NORMAL;
      else if(NormalBinding == G_PER_PRIM)
        _perPrim |= PER_NORMAL;

      bPerPrimNormal=((_perPrim & PER_NORMAL)!=0);

      if(TexCoordBinding == G_PER_VERTEX)
        _perVertex |= PER_TEXCOORD;
    }
    
    size_t vertex_size = draw_prim_setup(geom);
    
    // Note: draw_prim_setup could unset color flags if global color is set, so must
    //       recheck this flag here!
    bPerPrimColor=(_perPrim & PER_COLOR)!=0x0;
    
#ifdef _DEBUG
    // is it Ok not to recompute bUseTexCoordOnlyLoop even if draw_prim_setup unsets color flags?
    // add this check to make sure
    bool bNewUseTexCoordOnlyLoop = (((_perVertex & PER_COLOR)==0x0) &&
                                    ((_CurFVFType & D3DFVF_NORMAL)==0x0) &&
                                    ((_CurFVFType & D3DFVF_TEX1)!=0x0));
    if(bUseTexCoordOnlyLoop && (!bNewUseTexCoordOnlyLoop)) {
      // ok for bUseTexCoordOnlyLoop to be false, and bNew to be true.
      // draw_prim_setup can sometimes turn off the _perComp color for
      // G_OVERALL and scene-graph-color cases, which causes bNew to be true,
      // while the original bUseTexCoordOnly is still false.
      // the case we want to prevent is accidently using the texcoordloop
      // instead of the general one, using the general one should always work.
      
      DebugBreak();
      assert(0);
    }
#endif
    
    nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
    nassertv(nPrims * 3 * vertex_size < VERT_BUFFER_SIZE);
    _pCurFvfBufPtr = _pFvfBufBasePtr;          // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't
    
    // iterate through the triangle primitive
    
    for (uint i = 0; i < nPrims; i++) {
      if(bPerPrimColor) {  // remember color might be G_OVERALL too!
        GET_NEXT_COLOR();
      }
      
      if(bUseTexCoordOnlyLoop) {
        draw_prim_inner_loop_coordtexonly(3, geom);
      } else {
        if(bPerPrimNormal)
          p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.
        
        draw_prim_inner_loop(3, geom, _perVertex);
      }
    }
    
    DWORD nVerts=nPrims*3;

    nassertv((nVerts*vertex_size) == (_pCurFvfBufPtr-_pFvfBufBasePtr));
    
    hr = _pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, nPrims, _pFvfBufBasePtr, vertex_size);
    TestDrawPrimFailure(DrawPrim,hr,_pD3DDevice,nVerts,nPrims);
    
    _pCurFvfBufPtr = NULL;
    
    
    ///////////////////////////
#if 0
    // test triangle for me to dbg experiments only
    float vert_buf[15] = {
      0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
      33.0, 0.0f, 0.0f,  0.0f, 2.0,
      0.0f, 0.0f, 33.0,  2.0, 0.0f
    };

    _pD3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_BORDER);
    _pD3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_BORDER);
    _pD3DDevice->SetTextureStageState(0,D3DTSS_BORDERCOLOR,MY_D3DRGBA(0,0,0,0));
    
    DWORD FVFType =  D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) ;
    set_vertex_format(FVFType);
    HRESULT hr = _pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,  vert_buf, 1, 5*sizeof(float));
    TestDrawPrimFailure(DrawPrim,hr,_pD3DDevice,3,1);
#endif

}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_tristrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_tristrip(GeomTristrip *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
  dxgsg9_cat.debug() << "draw_tristrip()" << endl;
#endif
  DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
  DO_PSTATS_STUFF(_vertices_tristrip_pcollector.add_level(geom->get_num_vertices()));

  draw_multitri(geom, D3DPT_TRIANGLESTRIP);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_trifan
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_trifan(GeomTrifan *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg9_cat.debug() << "draw_trifan()" << endl;
#endif
  DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
  DO_PSTATS_STUFF(_vertices_trifan_pcollector.add_level(geom->get_num_vertices()));

  draw_multitri(geom, D3DPT_TRIANGLEFAN);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_multitri
//       Access: Public, Virtual
//  Description: handles trifans and tristrips
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_multitri(Geom *geom, D3DPRIMITIVETYPE trilisttype) {

    DWORD nPrims = geom->get_num_prims();
    const uint *pLengthArr = (const uint *) ((const int *)geom->get_lengths());
    HRESULT hr;

    if(nPrims==0) {
        #ifdef _DEBUG
          dxgsg9_cat.warning() << "draw_multitri() called with ZERO vertices!!" << endl;
        #endif
        return;
    }

#ifdef COUNT_DRAWPRIMS
    cGeomcount++;
#endif

    PTA_Vertexf coords;
    PTA_Normalf norms;
    PTA_Colorf colors;
    PTA_TexCoordf texcoords;
    GeomBindType TexCoordBinding,ColorBinding,NormalBinding;
    PTA_ushort vindexes,nindexes,tindexes,cindexes;

    geom->get_coords(coords,vindexes);
    geom->get_normals(norms,NormalBinding,nindexes);
    geom->get_colors(colors,ColorBinding,cindexes);
    geom->get_texcoords(texcoords,TexCoordBinding,tindexes);

    {
        // this is the old geom setup, it reformats every vtx into an output array passed to d3d
        _perVertex = 0x0;
        _perPrim = 0x0;
        _perComp = 0x0;

        bool bIsTriList=(trilisttype==D3DPT_TRIANGLESTRIP);
        bool bPerPrimColor=(ColorBinding == G_PER_PRIM);
        bool bPerPrimNormal;
        bool bUseTexCoordOnlyLoop = (((ColorBinding == G_OVERALL) || bPerPrimColor) &&
                                     (NormalBinding == G_OFF) &&
                                     (TexCoordBinding != G_OFF));

        if(bUseTexCoordOnlyLoop) {
           if(bPerPrimColor) {
                _perPrim = PER_COLOR;
           }
        } else {
            switch (ColorBinding) {
                case G_PER_PRIM:
                    _perPrim = PER_COLOR;
                    break;
                case G_PER_COMPONENT:
                    _perComp = PER_COLOR;
                    break;
                case G_PER_VERTEX:
                    _perVertex = PER_COLOR;
                    break;
            }

            switch (NormalBinding) {
                case G_PER_VERTEX:
                    _perVertex |= PER_NORMAL;
                    break;
                case G_PER_PRIM:
                    _perPrim |= PER_NORMAL;
                    break;
                case G_PER_COMPONENT:
                    _perComp |= PER_NORMAL;
                    break;
            }

            bPerPrimNormal=((_perPrim & PER_NORMAL)!=0);

            if (TexCoordBinding == G_PER_VERTEX)
                _perVertex |= PER_TEXCOORD;
        }

        size_t vertex_size = draw_prim_setup(geom);

        // Note: draw_prim_setup could unset color flags if global color is set, so must
        //       recheck this flag here!
        bPerPrimColor=(_perPrim & PER_COLOR)!=0;

        #ifdef _DEBUG
          // is it Ok not to recompute bUseTexCoordOnlyLoop even if draw_prim_setup unsets color flags?
          // add this check to make sure.  texcoordonly needs input that with unchanging color, except per-prim
           bool bNewUseTexCoordOnlyLoop = ((((_perComp|_perVertex) & PER_COLOR)==0x0) &&
                                           ((_CurFVFType & D3DFVF_NORMAL)==0x0) &&
                                           ((_CurFVFType & D3DFVF_TEX1)!=0x0));

           if(bUseTexCoordOnlyLoop && (!bNewUseTexCoordOnlyLoop)) {
               // ok for bUseTexCoordOnlyLoop to be false, and bNew to be true.
               // draw_prim_setup can sometimes turn off the _perComp color for
               // G_OVERALL and scene-graph-color cases, which causes bNew to be true,
               // while the original bUseTexCoordOnly is still false.
               // the case we want to prevent is accidently using the texcoordloop
               // instead of the general one, using the general one should always work.

               DebugBreak();
               assert(0);
           }

        #endif

        // iterate through the triangle primitives

        int nVerts;
        if(pLengthArr==NULL) {
           // we've been called by draw_quad, which has no lengths array
           nVerts=4;
        }

        for (uint i = 0; i < nPrims; i++) {

            if(pLengthArr!=NULL) {
              nVerts = *(pLengthArr++);
            }

            if(bPerPrimColor) {  // remember color might be G_OVERALL too!
                GET_NEXT_COLOR();
            }

#ifdef _DEBUG
            nassertv(nVerts >= 3);
            nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
            nassertv(nVerts * vertex_size < VERT_BUFFER_SIZE);
#endif
            _pCurFvfBufPtr = _pFvfBufBasePtr;            // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

            if(_perComp==0x0) {
                 if(bUseTexCoordOnlyLoop) {
                    draw_prim_inner_loop_coordtexonly(nVerts, geom);
                 } else {
                     if (bPerPrimNormal)
                         p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.

                     draw_prim_inner_loop(nVerts, geom, _perVertex);
                 }
            } else {
                if(bPerPrimNormal)
                    p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.

                if(bIsTriList) {
                   // in flat shade mode, D3D strips color using the 1st vertex.
                   // (note: differs from OGL, which always uses last vtx for strips&fans

                    // Store all but last 2 verts
                    draw_prim_inner_loop(nVerts-2, geom, _perVertex | _perComp);

                    // _perComp attribs should not be fetched for last 2 verts
                    draw_prim_inner_loop(2, geom, _perVertex);
                } else {
                   // in flat shade mode, D3D fans color using the 2nd vertex.
                   // (note: differs from OGL, which always uses last vtx for strips&fans
                   // _perComp attribs should not be fetched for first & last verts, they will
                   // be associated with middle n-2 verts

                    draw_prim_inner_loop(1, geom, _perVertex);
                    draw_prim_inner_loop(nVerts-2, geom, _perVertex | _perComp);
                    draw_prim_inner_loop(1, geom, _perVertex);
                }
            }

            assert((nVerts*vertex_size) == (_pCurFvfBufPtr-_pFvfBufBasePtr));
            DWORD numTris=nVerts-2;

            hr = _pD3DDevice->DrawPrimitiveUP(trilisttype, numTris, _pFvfBufBasePtr, vertex_size);
            TestDrawPrimFailure(DrawPrim,hr,_pD3DDevice,nVerts,numTris);

            _pCurFvfBufPtr = NULL;
        }
    }

}

//-----------------------------------------------------------------------------
// Name: GenerateSphere()
// Desc: Makes vertex and index data for ellipsoid w/scaling factors sx,sy,sz
//       tries to match gluSphere behavior
//-----------------------------------------------------------------------------

// probably want to replace this with D3DX9 call

void DXGraphicsStateGuardian9::
GenerateSphere(void *pVertexSpace,DWORD dwVertSpaceByteSize,
               void *pIndexSpace,DWORD dwIndexSpaceByteSize,
               D3DXVECTOR3 *pCenter, float fRadius,
               DWORD wNumRings, DWORD wNumSections, float sx, float sy, float sz,
               DWORD *pNumVertices,DWORD *pNumTris,DWORD fvfFlags,DWORD dwVertSize) {
    float x, y, z, rsintheta;
    D3DXVECTOR3 vPoint;

//#define DBG_GENSPHERE
#define M_PI 3.1415926f   // probably should get this from mathNumbers.h instead

    nassertv(wNumRings>=2 && wNumSections>=2);
    wNumRings--;  // wNumRings indicates number of vertex rings (not tri-rings).
                  // gluSphere 'stacks' arg for 1 vert ring is 2, so convert to our '1'.
    wNumSections++;  // to make us equiv to gluSphere

    //Figure out needed space for the triangles and vertices.
    DWORD dwNumVertices,dwNumIndices,dwNumTriangles;

#define DO_SPHERE_TEXTURING (fvfFlags & D3DFVF_TEXCOUNT_MASK)
#define DO_SPHERE_NORMAL    (fvfFlags & D3DFVF_NORMAL)
#define DO_SPHERE_COLOR     (fvfFlags & D3DFVF_DIFFUSE)

    if (DO_SPHERE_TEXTURING) {
        // if texturing, we need full rings of identical position verts at poles to hold diff texture coords
        wNumRings+=2;
        dwNumVertices = *pNumVertices = wNumRings * wNumSections;
        dwNumTriangles = (wNumRings-1) * wNumSections * 2;
    } else {
        dwNumVertices = *pNumVertices = wNumRings * wNumSections + 2;
        dwNumTriangles = wNumRings*wNumSections*2;
    }

    dwNumIndices = dwNumTriangles*3;
    *pNumTris = dwNumTriangles;

//    D3DVERTEX* pvVertices = (D3DVERTEX*) pVertexSpace;
    WORD *pwIndices = (WORD *) pIndexSpace;

    nassertv(dwNumVertices*dwVertSize < VERT_BUFFER_SIZE);
    nassertv(dwNumIndices < PANDA_MAXNUMVERTS );

    // Generate vertex at the top point
    D3DXVECTOR3 vTopPoint  = *pCenter;
    D3DXVECTOR3 vBotPoint  = *pCenter;
    float yRadius=sy*fRadius;
    vTopPoint.y+=yRadius;
    vBotPoint.y-=yRadius;
    D3DXVECTOR3 vNormal = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
    float texCoords[2];

    nassertv(pVertexSpace==_pCurFvfBufPtr);  // add_to_FVFBuf requires this

#define ADD_GENSPHERE_VERTEX_TO_BUFFER(VERT)                      \
    add_to_FVFBuf((void *)&(VERT), 3*sizeof(float));            \
    if(fvfFlags & D3DFVF_NORMAL)                                  \
        add_to_FVFBuf((void *)&vNormal, 3*sizeof(float));       \
    if(fvfFlags & D3DFVF_DIFFUSE)                                 \
        add_DWORD_to_FVFBuf(_curD3Dcolor);                        \
    if(fvfFlags & D3DFVF_TEXCOUNT_MASK)                           \
        add_to_FVFBuf((void *)texCoords, sizeof(TexCoordf));

#ifdef DBG_GENSPHERE
    int nvs_written=0;
    memset(pVertexSpace,0xFF,dwNumVertices*dwVertSize);
#endif

    if (! DO_SPHERE_TEXTURING) {
        ADD_GENSPHERE_VERTEX_TO_BUFFER(vTopPoint);
#ifdef DBG_GENSPHERE
        nvs_written++;
#endif
    }

    // Generate vertex points for rings
    float inv_radius = 1.0f/fRadius;
    const float reciprocal_PI=1.0f/M_PI;
    const float reciprocal_2PI=1.0f/(2.0*M_PI);
    DWORD i;
    float theta,dtheta;

    if (DO_SPHERE_TEXTURING) {
        // numRings already includes 1st and last rings for this case
        dtheta = (float)(M_PI / (wNumRings-1));     //Angle between each ring (ignore 2 fake rings)
        theta = 0.0f;
    } else {
        dtheta = (float)(M_PI / (wNumRings + 1));   //Angle between each ring
        theta = dtheta;
    }
    float phi,dphi   = (float)(2*M_PI / (wNumSections-1)); //Angle between each section

    for (i = 0; i < wNumRings; i++) {
        float costheta,sintheta,cosphi,sinphi;
        phi =   0.0f;

        if (DO_SPHERE_TEXTURING) {
            texCoords[1] = theta * reciprocal_PI;  // v is the same for each ring
        }

        // could optimize all this sin/cos stuff w/tables
        csincos(theta,&sintheta,&costheta);
        y = fRadius * costheta;     // y is the same for each ring

        rsintheta = fRadius * sintheta;

        for (DWORD j = 0; j < wNumSections; j++) {
            csincos(phi,&sinphi,&cosphi);
            x = rsintheta * sinphi;
            z = rsintheta * cosphi;

#ifdef DBG_GENSPHERE
            nvs_written++;
#endif
            vPoint.x = pCenter->x + sx*x;
            vPoint.y = pCenter->y + sy*y;
            vPoint.z = pCenter->z + sz*z;

            add_to_FVFBuf((void *)&vPoint, 3*sizeof(float));

            if (DO_SPHERE_NORMAL) {
                // bugbug: this is wrong normal for the non-spherical case (i think you need to multiply by 1/scale factor per component)
                D3DXVECTOR3 vVec = D3DXVECTOR3( x*inv_radius, y*inv_radius, z*inv_radius );
                D3DXVec3Normalize(&vNormal,&vVec);
                add_to_FVFBuf((float *)&vNormal, 3*sizeof(float));
            }

            if (DO_SPHERE_COLOR)
                add_DWORD_to_FVFBuf(_curD3Dcolor);

            if (DO_SPHERE_TEXTURING) {
                texCoords[0] = 1.0f - phi*reciprocal_2PI;
                add_to_FVFBuf((void *)texCoords, sizeof(TexCoordf));
            }

            phi += dphi;
        }
        theta += dtheta;
    }

    if (! DO_SPHERE_TEXTURING) {
        // Generate bottom vertex
        vNormal = D3DXVECTOR3( 0.0f, -1.0f, 0.0f );
        ADD_GENSPHERE_VERTEX_TO_BUFFER(vBotPoint);
#ifdef DBG_GENSPHERE
        nvs_written++;
#endif
    }

#ifdef DBG_GENSPHERE
    assert(nvs_written == dwNumVertices);
#endif


#ifdef DBG_GENSPHERE
    memset(pwIndices,0xFF,dwNumIndices*sizeof(WORD));
#endif

    // inited for textured case
    DWORD cur_vertring_startidx=0;    // first vertex in current ring
    DWORD CurFinalTriIndex = 0;       // index of next tri to be written

    if (! DO_SPHERE_TEXTURING) {
        // Generate caps using unique the bot/top vert
        // for non-textured case, could render the caps as indexed trifans,
        // but should be no perf difference b/w indexed trilists and indexed trifans
        // and this has advantage of being aggregable into 1 big DPrim call for whole sphere

        for (i = 0; i < wNumSections; i++) {
            DWORD TopCapTriIndex=3*i;
            DWORD BotCapTriIndex=3*(dwNumTriangles - wNumSections + i);
            DWORD i_incd = ((i + 1) % wNumSections);

            pwIndices[TopCapTriIndex++] = 0;
            pwIndices[TopCapTriIndex++] = i + 1;
            pwIndices[TopCapTriIndex] =  i_incd + 1;

            pwIndices[BotCapTriIndex++] = (WORD)( dwNumVertices - 1 );
            pwIndices[BotCapTriIndex++] = (WORD)( dwNumVertices - 2 - i );
            pwIndices[BotCapTriIndex] = (WORD)( dwNumVertices - 2 - i_incd);
        }

        cur_vertring_startidx = 1;          // first vertex in current ring (skip top vert)
        CurFinalTriIndex = wNumSections;    // index of tri to be written, wNumSections to skip the top cap row
    }

    DWORD j_incd,base_index;

    // technically we could break into a strip for every row (or 1 big strip connected w/degenerate tris)
    // but indexed trilists should actually be just as fast on HW

    // Generate triangles for the rings
    for (i = 0; i < wNumRings-1; i++) {
        for (DWORD j = 0; j < wNumSections; j++) {

            base_index=3*CurFinalTriIndex;  // final vert index is 3*finaltriindex
            j_incd=(j+1) % wNumSections;

            DWORD v1_row1_idx,v2_row1_idx,v1_row2_idx,v2_row2_idx;

            v1_row1_idx = cur_vertring_startidx + j;
            v2_row1_idx = cur_vertring_startidx + j_incd;
            v1_row2_idx = v1_row1_idx + wNumSections;
            v2_row2_idx = v2_row1_idx + wNumSections;

#ifdef DBG_GENSPHERE
            assert(v2_row2_idx<dwNumVertices);
            assert(v1_row2_idx<dwNumVertices);
            assert(v2_row1_idx<dwNumVertices);
            assert(v1_row1_idx<dwNumVertices);
#endif

            pwIndices[base_index++] = v1_row1_idx;
            pwIndices[base_index++] = v1_row2_idx;
            pwIndices[base_index++] = v2_row2_idx;

            pwIndices[base_index++] = v1_row1_idx;
            pwIndices[base_index++] = v2_row2_idx;
            pwIndices[base_index++] = v2_row1_idx;

            CurFinalTriIndex += 2;  // we wrote 2 tris, add 2 to finaltriindex
        }
        cur_vertring_startidx += wNumSections;
    }

#ifdef DBG_GENSPHERE
    if (DO_SPHERE_TEXTURING) {
        assert(CurFinalTriIndex == dwNumTriangles);
        assert(base_index == dwNumIndices);
    } else {
        assert(CurFinalTriIndex == dwNumTriangles-wNumSections);
        assert(base_index == dwNumIndices-wNumSections*3);
    }

    for (i = 0; i < dwNumIndices; i++)
        assert(pwIndices[i] <dwNumVertices);
#endif
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::draw_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
draw_sphere(GeomSphere *geom, GeomContext *gc) {

#define SPHERE_NUMSLICES 16
#define SPHERE_NUMSTACKS 10

#ifdef GSG_VERBOSE
    dxgsg9_cat.debug() << "draw_sphere()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    int nprims = geom->get_num_prims();

    if (nprims==0) {
        dxgsg9_cat.warning() << "draw_sphere() called with ZERO vertices!!" << endl;
        return;
    }

    Geom::VertexIterator vi = geom->make_vertex_iterator();
    Geom::ColorIterator ci;
    bool bPerPrimColor = (geom->get_binding(G_COLOR) == G_PER_PRIM);
    if (bPerPrimColor)
        ci = geom->make_color_iterator();

    for (int i = 0; i < nprims; i++) {

        DWORD nVerts,nTris;
        Vertexf center = geom->get_next_vertex(vi);
        Vertexf edge = geom->get_next_vertex(vi);
        LVector3f v = edge - center;
        float fRadius = sqrt(dot(v, v));

        size_t vertex_size = draw_prim_setup(geom);

        _pCurFvfBufPtr = _pFvfBufBasePtr;

        if (bPerPrimColor) {
            GET_NEXT_COLOR();
        }

        GenerateSphere(_pCurFvfBufPtr, VERT_BUFFER_SIZE,
                       _index_buf, PANDA_MAXNUMVERTS*2,
                       (D3DXVECTOR3 *)&center, fRadius,
                       SPHERE_NUMSTACKS, SPHERE_NUMSLICES,
                       1.0f, 1.0f, 1.0f,  // no scaling factors, do a sphere not ellipsoid
                       &nVerts,&nTris,_CurFVFType,vertex_size);

        // possible optimization: make DP 1 for all spheres call here, since trilist is independent tris.
        // indexes couldnt start w/0 tho, need to pass offset to gensph
        HRESULT hr = _pD3DDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0,  // start index in array
                                                         nVerts, nTris, _index_buf, D3DFMT_INDEX16,
                                                         _pFvfBufBasePtr, vertex_size);
        TestDrawPrimFailure(DrawIndexedPrim,hr,_pD3DDevice,nVerts,nTris);
    }

    _pCurFvfBufPtr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::prepare_texture
//       Access: Public, Virtual
//  Description: Creates a new retained-mode representation of the
//               given texture, and returns a newly-allocated
//               TextureContext pointer to reference it.  It is the
//               responsibility of the calling function to later
//               call release_texture() with this same pointer (which
//               will also delete the pointer).
////////////////////////////////////////////////////////////////////
TextureContext *DXGraphicsStateGuardian9::
prepare_texture(Texture *tex) {
    DXTextureContext9 *dtc = new DXTextureContext9(tex);
    if (dtc->CreateTexture(*_pScrn) == NULL) {
        delete dtc;
        return NULL;
    }
    return dtc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::apply_texture
//       Access: Public, Virtual
//  Description: Makes the texture the currently available texture for
//               rendering.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
apply_texture(TextureContext *tc) {
  if (tc==NULL) {
    // The texture wasn't bound properly or something, so ensure
    // texturing is disabled and just return.
    enable_texturing(false);
    return;
  }
  
#ifdef DO_PSTATS
  add_to_texture_record(tc);
#endif
  
  // Note: if this code changes, make sure to change initialization
  // SetTSS code in dx_init as well so DX TSS renderstate matches
  // dxgsg state
  
  DXTextureContext9 *dtc = DCAST(DXTextureContext9, tc);
  
  int dirty = dtc->get_dirty_flags();
  
  if (dirty) {
    // If the texture image has changed, or if its use of mipmaps has
    // changed, we need to re-create the image.  Ignore other types of
    // changes, which arent significant for dx
    
    if((dirty & (Texture::DF_image | Texture::DF_mipmap)) != 0) {
      // If this is *only* because of a mipmap change, issue a
      // warning--it is likely that this change is the result of an
      // error or oversight.
      if ((dirty & Texture::DF_image) == 0) {
        dxgsg9_cat.warning()
          << "Texture " << *dtc->_texture << " has changed mipmap state.\n";
      }
      
      dtc->DeleteTexture();
      if (dtc->CreateTexture(*_pScrn) == NULL) {
        
        // Oops, we can't re-create the texture for some reason.
        dxgsg9_cat.error() << "Unable to re-create texture " << *dtc->_texture << endl;
        
        enable_texturing(false);
        return;
      }
    }
    dtc->clear_dirty_flags();
  } else {
    if(_pCurTexContext == dtc) {
      enable_texturing(true);
      return;
    }
  }
  
  Texture *tex = tc->_texture;
  Texture::WrapMode wrapU,wrapV;
  wrapU=tex->get_wrap_u();
  wrapV=tex->get_wrap_v();
  
  if (wrapU!=_CurTexWrapModeU) {
    _pD3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,get_texture_wrap_mode(wrapU));
    _CurTexWrapModeU = wrapU;
  }
  if (wrapV!=_CurTexWrapModeV) {
    _pD3DDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,get_texture_wrap_mode(wrapV));
    _CurTexWrapModeV = wrapV;
  }
  
  uint aniso_degree=tex->get_anisotropic_degree();
  Texture::FilterType ft=tex->get_magfilter();
  
  if(_CurTexAnisoDegree != aniso_degree) {
    _pD3DDevice->SetSamplerState(0,D3DSAMP_MAXANISOTROPY,aniso_degree);
    _CurTexAnisoDegree = aniso_degree;
  }
  
  D3DTEXTUREFILTERTYPE newMagFilter;
  if (aniso_degree<=1) {
    newMagFilter=((ft!=Texture::FT_nearest) ? D3DTEXF_LINEAR : D3DTEXF_POINT);
    
#ifdef _DEBUG
    if((ft!=Texture::FT_linear)&&(ft!=Texture::FT_nearest)) {
      dxgsg9_cat.error() << "MipMap filter type setting for texture magfilter makes no sense,  texture: " << tex->get_name() << "\n";
    }
#endif
  } else {
    newMagFilter=D3DTEXF_ANISOTROPIC;
  }
  
  if(_CurTexMagFilter!=newMagFilter) {
    _CurTexMagFilter=newMagFilter;
    _pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, newMagFilter);
  }
  
#ifdef _DEBUG
  assert(Texture::FT_linear_mipmap_linear < 8);
#endif
  /*
    enum FilterType {
    FT_nearest,FT_linear,FT_nearest_mipmap_nearest,FT_linear_mipmap_nearest,
    FT_nearest_mipmap_linear, FT_linear_mipmap_linear, };
  */
  // map Panda composite min+mip filter types to d3d's separate min & mip filter types
  static D3DTEXTUREFILTERTYPE PandaToD3DMinType[8] =
    {D3DTEXF_POINT,D3DTEXF_LINEAR,D3DTEXF_POINT,D3DTEXF_LINEAR,D3DTEXF_POINT,D3DTEXF_LINEAR};
  static D3DTEXTUREFILTERTYPE PandaToD3DMipType[8] =
    {D3DTEXF_NONE,D3DTEXF_NONE,D3DTEXF_POINT,D3DTEXF_POINT,D3DTEXF_LINEAR,D3DTEXF_LINEAR};
  
  ft=tex->get_minfilter();
  
#ifdef _DEBUG
  if(ft > Texture::FT_linear_mipmap_linear) {
    dxgsg9_cat.error() << "Unknown tex filter type for tex: " << tex->get_name() << "  filter: "<<(DWORD)ft<<"\n";
    return;
  }
#endif
  
  D3DTEXTUREFILTERTYPE newMipFilter = PandaToD3DMipType[(DWORD)ft];
  
  if (!tex->might_have_ram_image()) {
    // If the texture is completely dynamic, don't try to issue
    // mipmaps--pandadx doesn't support auto-generated mipmaps at this
    // point.
    newMipFilter = D3DTEXF_NONE;
  }
  
#ifndef NDEBUG
  // sanity check
  extern char *PandaFilterNameStrs[];
  if((!(dtc->_bHasMipMaps))&&(newMipFilter!=D3DTEXF_NONE)) {
    dxgsg9_cat.error() << "Trying to set mipmap filtering for texture with no generated mipmaps!! texname[" << tex->get_name() << "], filter("<<PandaFilterNameStrs[ft]<<")\n";
        newMipFilter=D3DTEXF_NONE;
    }
#endif

  
  D3DTEXTUREFILTERTYPE newMinFilter = PandaToD3DMinType[(DWORD)ft];
  
  if(aniso_degree>=2) {
    newMinFilter=D3DTEXF_ANISOTROPIC;
  }
  
  if(newMinFilter!=_CurTexMinFilter) {
    _CurTexMinFilter = newMinFilter;
    _pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, newMinFilter);
  }
  
  if(newMipFilter!=_CurTexMipFilter) {
    _CurTexMipFilter = newMipFilter;
    _pD3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, newMipFilter);
  }
  
  // bugbug:  does this handle the case of untextured geometry?
  //          we dont see this bug cause we never mix textured/untextured
  _pD3DDevice->SetTexture(0,dtc->_pD3DTexture9);
  
#if 0
  if (dtc!=NULL) {
    dxgsg9_cat.spam() << "Setting active DX texture: " << dtc->_tex->get_name() << "\n";
  }
#endif
  
  _pCurTexContext = dtc;   // enable_texturing needs this
  enable_texturing(true);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
release_texture(TextureContext *tc) {
    DXTextureContext9 *gtc = DCAST(DXTextureContext9, tc);
    gtc->DeleteTexture();
    delete gtc;
}

// copies current display region in framebuffer to the texture
// usually its more efficient to do SetRenderTgt
void DXGraphicsStateGuardian9::
framebuffer_copy_to_texture(Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb) {
  set_read_buffer(rb);

  HRESULT hr;
  int xo, yo, w, h;
  dr->get_region_pixels_i(xo, yo, w, h);

  tex->set_x_size(w);
  tex->set_y_size(h);

  TextureContext *tc = tex->prepare_now(get_prepared_objects(), this);
  if (tc == (TextureContext *)NULL) {
    return;
  }
  DXTextureContext9 *dtc = DCAST(DXTextureContext9, tc);

  IDirect3DSurface9 *pTexSurfaceLev0,*pCurRenderTarget;
  hr = dtc->_pD3DTexture9->GetSurfaceLevel(0,&pTexSurfaceLev0);
  if(FAILED(hr)) {
    dxgsg9_cat.error() << "GetSurfaceLev failed in copy_texture" << D3DERRORSTRING(hr);
    return;
  }

  hr = _pD3DDevice->GetRenderTarget(0, &pCurRenderTarget);
  if(FAILED(hr)) {
    dxgsg9_cat.error() << "GetRenderTgt failed in copy_texture" << D3DERRORSTRING(hr);
    SAFE_RELEASE(pTexSurfaceLev0);
    return;
  }

  RECT SrcRect;

  SrcRect.left = xo;
  SrcRect.right = xo+w;
  SrcRect.top = yo;
  SrcRect.bottom = yo+h;

  // now copy from fb to tex
  //hr = _pD3DDevice->UpdateSurface(pCurRenderTarget,&SrcRect,pTexSurfaceLev0,0);
  // the following call does what we want. Interesting though, why Dx9 took out the 
  // functionality of copying from VRAM to VRAM and put it in D3DX library. Perhaps
  // to promote D3DX!?
  hr = D3DXLoadSurfaceFromSurface(pTexSurfaceLev0, NULL, NULL, pCurRenderTarget, NULL, &SrcRect, D3DX_FILTER_NONE, 0);
  
  if(FAILED(hr)) {
    dxgsg9_cat.error() 
      << "UpdateSurface failed in copy_texture" << D3DERRORSTRING(hr);
  }

  SAFE_RELEASE(pCurRenderTarget);
  SAFE_RELEASE(pTexSurfaceLev0);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::framebuffer_copy_to_ram
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian9::
framebuffer_copy_to_ram(Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb) {
  set_read_buffer(rb);

  RECT SrcCopyRect;
  nassertr(tex != NULL && dr != NULL, false);
  
  int xo, yo, w, h;
  dr->get_region_pixels_i(xo, yo, w, h);

  tex->setup_2d_texture(w, h, Texture::T_unsigned_byte, Texture::F_rgb);

  SrcCopyRect.top = yo;
  SrcCopyRect.left = xo;
  SrcCopyRect.right = xo + w;
  SrcCopyRect.bottom = yo + h;
  
  IDirect3DSurface9 *pD3DSurf;
  HRESULT hr;
  
  if(_cur_read_pixel_buffer & RenderBuffer::T_back) {
    hr=_pD3DDevice->GetBackBuffer(0, 0,D3DBACKBUFFER_TYPE_MONO,&pD3DSurf);
    
    if(FAILED(hr)) {
      dxgsg9_cat.error() << "GetBackBuffer failed" << D3DERRORSTRING(hr);
      return false;
    }
    
    // note if you try to grab the backbuffer and full-screen anti-aliasing is on,
    // the backbuffer might be larger than the window size.  for screenshots its safer to get the front buffer.
    
  } else if(_cur_read_pixel_buffer & RenderBuffer::T_front) {
    // must create a A8R8G8B8 sysmem surface for GetFrontBuffer to copy to
    
    DWORD TmpSurfXsize,TmpSurfYsize;
    
    if(_pScrn->PresParams.Windowed) {
      // GetFrontBuffer retrieves the entire desktop for a monitor, so
      // need space for that
      
      MONITORINFO minfo;
      minfo.cbSize = sizeof(MONITORINFO);
      GetMonitorInfo(_pScrn->hMon, &minfo);   // have to use GetMonitorInfo, since this gsg may not be for primary monitor
      
      TmpSurfXsize = RECT_XSIZE(minfo.rcMonitor);
      TmpSurfYsize = RECT_YSIZE(minfo.rcMonitor);

      // set SrcCopyRect to client area of window in scrn coords
      ClientToScreen( _pScrn->hWnd, (POINT*)&SrcCopyRect.left );
      ClientToScreen( _pScrn->hWnd, (POINT*)&SrcCopyRect.right );

    } else {
      RECT WindRect;
      GetWindowRect(_pScrn->hWnd,&WindRect);
      TmpSurfXsize = RECT_XSIZE(WindRect);
      TmpSurfYsize = RECT_YSIZE(WindRect);
    }
    
    hr=_pD3DDevice->CreateOffscreenPlainSurface(TmpSurfXsize,TmpSurfYsize,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM, &pD3DSurf, NULL);
    if(FAILED(hr)) {
      dxgsg9_cat.error() << "CreateImageSurface failed in copy_pixel_buffer()" << D3DERRORSTRING(hr);
      return false;
    }
    
    hr=_pD3DDevice->GetFrontBufferData(0, pD3DSurf);
    
    if(hr==D3DERR_DEVICELOST) {
      pD3DSurf->Release();
      dxgsg9_cat.error() << "copy_pixel_buffer failed: device lost\n";
      return false;
    }

  } else {
    dxgsg9_cat.error() << "copy_pixel_buffer: unhandled current_read_pixel_buffer type\n";
    return false;
  }
  
  (void) ConvertD3DSurftoPixBuf(SrcCopyRect,pD3DSurf,tex);
  
  RELEASE(pD3DSurf,dxgsg9,"pD3DSurf",RELEASE_ONCE);
  
  nassertr(tex->has_ram_image(), false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::apply_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::apply_material( const Material* material ) {
    D3DMATERIAL9 cur_material;
    cur_material.Diffuse = *(D3DCOLORVALUE *)(material->get_diffuse().get_data());
    cur_material.Ambient = *(D3DCOLORVALUE *)(material->get_ambient().get_data());
    cur_material.Specular = *(D3DCOLORVALUE *)(material->get_specular().get_data());
    cur_material.Emissive = *(D3DCOLORVALUE *)(material->get_emission().get_data());
    cur_material.Power   =  material->get_shininess();
    _pD3DDevice->SetMaterial(&cur_material);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::apply_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
apply_fog(Fog *fog) {

    if(_doFogType==None)
      return;

    Fog::Mode panda_fogmode = fog->get_mode();
    D3DFOGMODE d3dfogmode = get_fog_mode_type(panda_fogmode);


    // should probably avoid doing redundant SetRenderStates, but whatever
    _pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)_doFogType, d3dfogmode);

    const Colorf &fog_colr = fog->get_color();
    _pD3DDevice->SetRenderState(D3DRS_FOGCOLOR,
                   MY_D3DRGBA(fog_colr[0], fog_colr[1], fog_colr[2], 0.0f));  // Alpha bits are not used

    // do we need to adjust fog start/end values based on D3DPRASTERCAPS_WFOG/D3DPRASTERCAPS_ZFOG ?
    // if not WFOG, then docs say we need to adjust values to range [0,1]

    switch (panda_fogmode) {
        case Fog::M_linear:
            {
                float onset, opaque;
                fog->get_linear_range(onset, opaque);

                _pD3DDevice->SetRenderState( D3DRS_FOGSTART,
                                            *((LPDWORD) (&onset)) );
                _pD3DDevice->SetRenderState( D3DRS_FOGEND,
                                            *((LPDWORD) (&opaque)) );
            }
            break;
        case Fog::M_exponential:
        case Fog::M_exponential_squared:
            {
                // Exponential fog is always camera-relative.
                float fog_density = fog->get_exp_density();
                _pD3DDevice->SetRenderState( D3DRS_FOGDENSITY,
                            *((LPDWORD) (&fog_density)) );
            }
            break;
    }
}

void DXGraphicsStateGuardian9::SetTextureBlendMode(TextureApplyAttrib::Mode TexBlendMode,bool bCanJustEnable) {

/*class TextureApplyAttrib {
  enum Mode {
    M_modulate,M_decal,M_blend,M_replace,M_add};
*/
    static D3DTEXTUREOP TexBlendColorOp1[/* TextureApplyAttrib::Mode maxval*/ 10] =
    {D3DTOP_MODULATE,D3DTOP_BLENDTEXTUREALPHA,D3DTOP_MODULATE,D3DTOP_SELECTARG1,D3DTOP_ADD};

    //if bCanJustEnable, then we only need to make sure ColorOp is turned on and set properly
    if (bCanJustEnable && (TexBlendMode==_CurTexBlendMode)) {
        // just reset COLOROP 0 to enable pipeline, rest is already set properly
        _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, TexBlendColorOp1[TexBlendMode] );
        return;
    }

    _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, TexBlendColorOp1[TexBlendMode] );

    switch (TexBlendMode) {

        case TextureApplyAttrib::M_modulate:
            // emulates GL_MODULATE glTexEnv mode
            // want to multiply tex-color*pixel color to emulate GL modulate blend (see glTexEnv)
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

            break;
        case TextureApplyAttrib::M_decal:
            // emulates GL_DECAL glTexEnv mode
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

            break;
        case TextureApplyAttrib::M_replace:
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            break;
        case TextureApplyAttrib::M_add:
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

            // since I'm making up 'add' mode, use modulate.  "adding" alpha never makes sense right?
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

            break;
        case TextureApplyAttrib::M_blend:
            dxgsg9_cat.error()
            << "Impossible to emulate GL_BLEND in DX exactly " << (int) TexBlendMode << endl;
/*
           // emulate GL_BLEND glTexEnv

           GL requires 2 independent operations on 3 input vars for this mode
           DX texture pipeline requires re-using input of last stage on each new op, so I dont think
           exact emulation is possible
           _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
           _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT );
           _pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

           _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
           _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
           _pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

           need to SetTexture(1,tex) also
           _pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE ); wrong
           _pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
           _pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TFACTOR );

           _pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
           _pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
*/


            break;
        default:
            dxgsg9_cat.error() << "Unknown texture blend mode " << (int) TexBlendMode << endl;
            break;
    }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_transform
//       Access: Public, Virtual
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_transform(const TransformState *transform) {
  DO_PSTATS_STUFF(_transform_state_pcollector.add_level(1));

  // if we're using ONLY vertex shaders, could get avoid calling SetTrans
  D3DMATRIX *pMat = (D3DMATRIX*)transform->get_mat().get_data();
  _pD3DDevice->SetTransform(D3DTS_WORLD,pMat);

  _transform = transform;
  if (_auto_rescale_normal) {
    do_auto_rescale_normal();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_tex_matrix
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_tex_matrix(const TexMatrixAttrib *attrib) {
  const LMatrix4f &m = attrib->get_mat();

  if (!attrib->has_stage(TextureStage::get_default())) {
    _pD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                      D3DTTFF_DISABLE);
    // For some reason, "disabling" texture coordinate transforms
    // doesn't seem to be sufficient.  We'll load an identity matrix
    // to underscore the point.
    _pD3DDevice->SetTransform(D3DTS_TEXTURE0, &matIdentity);

  } else {
    // We have to reorder the elements of the matrix for some reason.
    LMatrix4f dm(m(0, 0), m(0, 1), m(0, 3), 0.0f,
                 m(1, 0), m(1, 1), m(1, 3), 0.0f,
                 m(3, 0), m(3, 1), m(3, 3), 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
    _pD3DDevice->SetTransform(D3DTS_TEXTURE0, (D3DMATRIX *)dm.get_data());
    _pD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                      D3DTTFF_COUNT2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_tex_gen
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_tex_gen(const TexGenAttrib *attrib) {
  /*
   * Automatically generate texture coordinates for stage 0.
   * Use the wrap mode from the texture coordinate set at index 1.
   */
  DO_PSTATS_STUFF(_texture_state_pcollector.add_level(1));
  if (attrib->is_empty()) {

    //enable_texturing(false);
    // reset the texcoordindex lookup to 0
    //_pD3DDevice->SetTransform(D3DTS_TEXTURE0, (D3DMATRIX *)dm.get_data());
    _pD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
    _pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);

  } else if (attrib->get_mode(TextureStage::get_default()) == TexGenAttrib::M_eye_sphere_map) {

#if 0
    // best reflection on a sphere is achieved by camera space normals in directx
    _pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX,
                                       D3DTSS_TCI_CAMERASPACENORMAL);
    // We have set up the texture matrix to scale and translate the
    // texture coordinates to get from camera space (-1, +1) to
    // texture space (0,1)
    LMatrix4f dm(0.5f, 0.0f, 0.0f, 0.0f,
                 0.0f, 0.5f, 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 0.5f, 0.5f, 0.0f, 1.0f);
#else
    // since this is a reflection map, we want the camera space
    // reflection vector. A close approximation of the asin(theta)/pi
    // + 0.5 is achieved by the following matrix
    _pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX,
                                       D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
    LMatrix4f dm(0.33f, 0.0f, 0.0f, 0.0f,
                 0.0f, 0.33f, 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 0.5f, 0.5f, 0.0f, 1.0f);
#endif
    _pD3DDevice->SetTransform(D3DTS_TEXTURE0, (D3DMATRIX *)dm.get_data());
    _pD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                          D3DTTFF_COUNT2);
    //_pD3DDevice->SetRenderState(D3DRS_LOCALVIEWER, false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_texture(const TextureAttrib *attrib) {
  DO_PSTATS_STUFF(_texture_state_pcollector.add_level(1));
  if (attrib->is_off()) {
    enable_texturing(false);
  } else {
    Texture *tex = attrib->get_texture();
    nassertv(tex != (Texture *)NULL);

    TextureContext *tc = tex->prepare_now(_prepared_objects, this);
    apply_texture(tc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_material(const MaterialAttrib *attrib) {
  const Material *material = attrib->get_material();
  if (material != (const Material *)NULL) {
    apply_material(material);
  } else {
    // Apply a default material when materials are turned off.
    Material empty;
    apply_material(&empty);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_render_mode
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_render_mode(const RenderModeAttrib *attrib) {
  RenderModeAttrib::Mode mode = attrib->get_mode();

  switch (mode) {
  case RenderModeAttrib::M_unchanged:
  case RenderModeAttrib::M_filled:
    _pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    break;

  case RenderModeAttrib::M_wireframe:
    _pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    break;

  case RenderModeAttrib::M_point:
    _pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
    break;

  default:
    dxgsg9_cat.error()
      << "Unknown render mode " << (int)mode << endl;
  }

  _current_fill_mode = mode;
}
 
////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_rescale_normal
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_rescale_normal(const RescaleNormalAttrib *attrib) {
  RescaleNormalAttrib::Mode mode = attrib->get_mode();

  _auto_rescale_normal = false;

  switch (mode) {
  case RescaleNormalAttrib::M_none:
    _pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, false);
    break;

  case RescaleNormalAttrib::M_rescale:
  case RescaleNormalAttrib::M_normalize:
    _pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, true);
    break;

  case RescaleNormalAttrib::M_auto:
    _auto_rescale_normal = true;
    do_auto_rescale_normal();
    break;

  default:
    dxgsg9_cat.error()
      << "Unknown rescale_normal mode " << (int)mode << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_texture_apply
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_texture_apply(const TextureApplyAttrib *attrib) {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_depth_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_depth_test(const DepthTestAttrib *attrib) {
  DepthTestAttrib::PandaCompareFunc mode = attrib->get_mode();
  if (mode == DepthTestAttrib::M_none) {
    _depth_test_enabled = false;
    _pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
  } else {
    _depth_test_enabled = true;
    _pD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    _pD3DDevice->SetRenderState(D3DRS_ZFUNC, (D3DCMPFUNC) mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_alpha_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_alpha_test(const AlphaTestAttrib *attrib) {
  AlphaTestAttrib::PandaCompareFunc mode = attrib->get_mode();
  if (mode == AlphaTestAttrib::M_none) {
    enable_alpha_test(false);
  } else {
    //  AlphaTestAttrib::PandaCompareFunc === D3DCMPFUNC
    call_dxAlphaFunc((D3DCMPFUNC)mode, attrib->get_reference_alpha());
    enable_alpha_test(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_depth_write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_depth_write(const DepthWriteAttrib *attrib) {
  enable_zwritemask(attrib->get_mode() == DepthWriteAttrib::M_on);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_cull_face
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_cull_face(const CullFaceAttrib *attrib) {
  CullFaceAttrib::Mode mode = attrib->get_effective_mode();

  switch (mode) {
  case CullFaceAttrib::M_cull_none:
    _pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    break;
  case CullFaceAttrib::M_cull_clockwise:
    _pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
    _pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    break;
  default:
    dxgsg9_cat.error()
      << "invalid cull face mode " << (int)mode << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_fog(const FogAttrib *attrib) {
  if (!attrib->is_off()) {
    enable_fog(true);
    Fog *fog = attrib->get_fog();
    nassertv(fog != (Fog *)NULL);
    apply_fog(fog);
  } else {
    enable_fog(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::issue_depth_offset
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
issue_depth_offset(const DepthOffsetAttrib *attrib) {
  int offset = attrib->get_offset();
  _pD3DDevice->SetRenderState(D3DRS_DEPTHBIAS, offset);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
bind_light(PointLight *light, int light_id) {
  // Get the light in "world coordinates".  This means the light in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  NodePath light_np(light);
  const LMatrix4f &light_mat = light_np.get_mat(_scene_setup->get_camera_path());
  LMatrix4f rel_mat = light_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  LPoint3f pos = light->get_point() * rel_mat;

  D3DCOLORVALUE black;
  black.r = black.g = black.b = black.a = 0.0f;
  D3DLIGHT9 alight;
  alight.Type =  D3DLIGHT_POINT;
  alight.Diffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
  alight.Ambient  =  black ;
  alight.Specular = *(D3DCOLORVALUE *)(light->get_specular_color().get_data());

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  alight.Position = *(D3DVECTOR *)pos.get_data();

  alight.Range =  __D3DLIGHT_RANGE_MAX;
  alight.Falloff =  1.0f;

  const LVecBase3f &att = light->get_attenuation();
  alight.Attenuation0 = att[0];
  alight.Attenuation1 = att[1];
  alight.Attenuation2 = att[2];

  HRESULT res = _pD3DDevice->SetLight(light_id, &alight);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
bind_light(DirectionalLight *light, int light_id) {
  // Get the light in "world coordinates".  This means the light in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  NodePath light_np(light);
  const LMatrix4f &light_mat = light_np.get_mat(_scene_setup->get_camera_path());
  LMatrix4f rel_mat = light_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  LVector3f dir = light->get_direction() * rel_mat;

  D3DCOLORVALUE black;
  black.r = black.g = black.b = black.a = 0.0f;

  D3DLIGHT9 alight;
  ZeroMemory(&alight, sizeof(D3DLIGHT9));

  alight.Type =  D3DLIGHT_DIRECTIONAL;
  alight.Diffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
  alight.Ambient  =  black ;
  alight.Specular = *(D3DCOLORVALUE *)(light->get_specular_color().get_data());

  alight.Direction = *(D3DVECTOR *)dir.get_data();

  alight.Range =  __D3DLIGHT_RANGE_MAX;
  alight.Falloff =  1.0f;

  alight.Attenuation0 = 1.0f;       // constant
  alight.Attenuation1 = 0.0f;       // linear
  alight.Attenuation2 = 0.0f;       // quadratic

  HRESULT res = _pD3DDevice->SetLight(light_id, &alight);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
bind_light(Spotlight *light, int light_id) {
  Lens *lens = light->get_lens();
  nassertv(lens != (Lens *)NULL);

  // Get the light in "world coordinates".  This means the light in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  NodePath light_np(light);
  const LMatrix4f &light_mat = light_np.get_mat(_scene_setup->get_camera_path());
  LMatrix4f rel_mat = light_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  LPoint3f pos = lens->get_nodal_point() * rel_mat;
  LVector3f dir = lens->get_view_vector() * rel_mat;

  D3DCOLORVALUE black;
  black.r = black.g = black.b = black.a = 0.0f;

  D3DLIGHT9  alight;
  ZeroMemory(&alight, sizeof(D3DLIGHT9));

  alight.Type =  D3DLIGHT_SPOT;
  alight.Ambient  =  black ;
  alight.Diffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
  alight.Specular = *(D3DCOLORVALUE *)(light->get_specular_color().get_data());

  alight.Position = *(D3DVECTOR *)pos.get_data();

  alight.Direction = *(D3DVECTOR *)dir.get_data();

  alight.Range =  __D3DLIGHT_RANGE_MAX;
  alight.Falloff =  1.0f;
  alight.Theta =  0.0f;
  alight.Phi =  lens->get_hfov();

  const LVecBase3f &att = light->get_attenuation();
  alight.Attenuation0 = att[0];
  alight.Attenuation1 = att[1];
  alight.Attenuation2 = att[2];

  HRESULT res = _pD3DDevice->SetLight(light_id, &alight);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::begin_frame
//       Access: Public, Virtual
//  Description: Called before each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup before
//               beginning the frame.
//
//               The return value is true if successful (in which case
//               the frame will be drawn and end_frame() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_frame() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian9::
begin_frame() {
  return GraphicsStateGuardian::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::begin_scene
//       Access: Public, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the beginning of drawing commands for a "scene"
//               (usually a particular DisplayRegion) within a frame.
//               All 3-D drawing commands, except the clear operation,
//               must be enclosed within begin_scene() .. end_scene().
//
//               The return value is true if successful (in which case
//               the scene will be drawn and end_scene() will be
//               called later), or false if unsuccessful (in which
//               case nothing will be drawn and end_scene() will not
//               be called).
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian9::
begin_scene() {
  if (!GraphicsStateGuardian::begin_scene()) {
    return false;
  }

  HRESULT hr = _pD3DDevice->BeginScene();

  if (FAILED(hr)) {
    if (hr == D3DERR_DEVICELOST) {
      if (dxgsg9_cat.is_debug()) {
        dxgsg9_cat.debug()
          << "BeginScene returns D3DERR_DEVICELOST" << endl;
      }
      
      CheckCooperativeLevel();

    } else {
      dxgsg9_cat.error()
        << "BeginScene failed, unhandled error hr == "
        << D3DERRORSTRING(hr) << endl;
      exit(1);
    }
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::end_scene
//       Access: Public, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the end of drawing commands for a "scene" (usually a
//               particular DisplayRegion) within a frame.  All 3-D
//               drawing commands, except the clear operation, must be
//               enclosed within begin_scene() .. end_scene().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
end_scene() {
  HRESULT hr = _pD3DDevice->EndScene();
  
  if (FAILED(hr)) {

    if (hr == D3DERR_DEVICELOST) {
      if(dxgsg9_cat.is_debug()) {
        dxgsg9_cat.debug()
          << "EndScene returns DeviceLost\n";
      }
      CheckCooperativeLevel();

    } else {
      dxgsg9_cat.error()
        << "EndScene failed, unhandled error hr == " << D3DERRORSTRING(hr);
      exit(1);
    }
    return;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsStateGuardian::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
end_frame() {
#ifdef COUNT_DRAWPRIMS
    {
        #define FRAMES_PER_DPINFO 90
        static DWORD LastDPInfoFrame=0;
        static DWORD LastTickCount=0;
        const float one_thousandth = 1.0f/1000.0f;

        if (_cur_frame_count-LastDPInfoFrame > FRAMES_PER_DPINFO) {
            DWORD CurTickCount=GetTickCount();
            float delta_secs=(CurTickCount-LastTickCount)*one_thousandth;

            float numframes=_cur_frame_count-LastDPInfoFrame;
            float verts_per_frame = cVertcount/numframes;
            float tris_per_frame = cTricount/numframes;
            float DPs_per_frame = cDPcount/numframes;
            float DPs_notexchange_per_frame = cDP_noTexChangeCount/numframes;
            float verts_per_DP = cVertcount/(float)cDPcount;
            float verts_per_sec = cVertcount/delta_secs;
            float tris_per_sec = cTricount/delta_secs;
            float Geoms_per_frame = cGeomcount/numframes;
            float DrawPrims_per_Geom = cDPcount/(float)cGeomcount;
            float verts_per_Geom = cVertcount/(float)cGeomcount;

            dxgsg9_cat.debug() << "==================================="
                << "\n Avg Verts/sec:\t\t" << verts_per_sec
                << "\n Avg Tris/sec:\t\t" << tris_per_sec
                << "\n Avg Verts/frame:\t" << verts_per_frame
                << "\n Avg Tris/frame:\t" << tris_per_frame
                << "\n Avg DrawPrims/frm:\t" << DPs_per_frame
                << "\n Avg Verts/DrawPrim:\t" << verts_per_DP
                << "\n Avg DrawPrims w/no Texture Change from prev DrawPrim/frm:\t" << DPs_notexchange_per_frame
                << "\n Avg Geoms/frm:\t" << Geoms_per_frame
                << "\n Avg DrawPrims/Geom:\t" << DrawPrims_per_Geom
                << "\n Avg Verts/Geom:\t" << verts_per_Geom
                << endl;

            LastDPInfoFrame=_cur_frame_count;
            cDPcount = cVertcount=cTricount=cDP_noTexChangeCount=cGeomcount=0;
            LastTickCount=CurTickCount;
        }
    }
#endif

#if defined(DO_PSTATS)||defined(PRINT_RESOURCESTATS)
#ifndef PRINT_RESOURCESTATS
  if (_texmgrmem_total_pcollector.is_active())
#endif
  {
      #define TICKS_PER_GETTEXINFO (2.5*1000)   // 2.5 second interval
      static DWORD LastTickCount=0;
      DWORD CurTickCount=GetTickCount();

      if (CurTickCount-LastTickCount > TICKS_PER_GETTEXINFO) {
          LastTickCount=CurTickCount;
          report_texmgr_stats();
      }
  }
#endif

  // Note: regular GraphicsWindow::end_frame is being called,
  // but we override gsg::end_frame, so need to explicitly call it here
  // (currently it's an empty fn)
  GraphicsStateGuardian::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::set_draw_buffer
//       Access: Protected
//  Description: Sets up the glDrawBuffer to render into the buffer
//               indicated by the RenderBuffer object.  This only sets
//               up the color bits; it does not affect the depth,
//               stencil, accum layers.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
set_draw_buffer(const RenderBuffer &rb) {
    dxgsg9_cat.fatal() << "DX set_draw_buffer unimplemented!!!";
    return;

#ifdef WBD_GL_MODE
    switch (rb._buffer_type & RenderBuffer::T_color) {
        case RenderBuffer::T_front:
            call_glDrawBuffer(GL_FRONT);
            break;

        case RenderBuffer::T_back:
            call_glDrawBuffer(GL_BACK);
            break;

        case RenderBuffer::T_right:
            call_glDrawBuffer(GL_RIGHT);
            break;

        case RenderBuffer::T_left:
            call_glDrawBuffer(GL_LEFT);
            break;

        case RenderBuffer::T_front_right:
            call_glDrawBuffer(GL_FRONT_RIGHT);
            break;

        case RenderBuffer::T_front_left:
            call_glDrawBuffer(GL_FRONT_LEFT);
            break;

        case RenderBuffer::T_back_right:
            call_glDrawBuffer(GL_BACK_RIGHT);
            break;

        case RenderBuffer::T_back_left:
            call_glDrawBuffer(GL_BACK_LEFT);
            break;

        default:
            call_glDrawBuffer(GL_FRONT_AND_BACK);
    }
#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::set_read_buffer
//       Access: Protected
//  Description: Vestigial analog of glReadBuffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
set_read_buffer(const RenderBuffer &rb) {

    if(rb._buffer_type & RenderBuffer::T_front) {
            _cur_read_pixel_buffer=RenderBuffer::T_front;
    } else  if(rb._buffer_type & RenderBuffer::T_back) {
            _cur_read_pixel_buffer=RenderBuffer::T_back;
    } else {
            dxgsg9_cat.error() << "Invalid or unimplemented Argument to set_read_buffer!\n";
    }
    return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::do_auto_rescale_normal
//       Access: Protected
//  Description: Issues the appropriate GL commands to either rescale
//               or normalize the normals according to the current
//               transform.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
do_auto_rescale_normal() {
  if (_transform->has_uniform_scale() &&
      IS_NEARLY_EQUAL(_transform->get_uniform_scale(), 1.0f)) {
    // If there's no scale, don't normalize anything.
    _pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, false);

  } else {
    // If there is a scale, turn on normalization.
    _pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::enable_lighting
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of lighting overall.  This
//               is called by issue_light() according to whether any
//               lights are in use or not.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
enable_lighting(bool enable) {
  _pD3DDevice->SetRenderState(D3DRS_LIGHTING, (DWORD)enable);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::set_ambient_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               indicate the color of the ambient light that should
//               be in effect.  This is called by issue_light() after
//               all other lights have been enabled or disabled.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
set_ambient_light(const Colorf &color) {
  _pD3DDevice->SetRenderState(D3DRS_AMBIENT,
                                  Colorf_to_D3DCOLOR(color));
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::enable_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated light id.  A specific Light will
//               already have been bound to this id via bind_light().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
enable_light(int light_id, bool enable) {
  HRESULT res = _pD3DDevice->LightEnable(light_id, enable);

#ifdef GSG_VERBOSE
  dxgsg9_cat.debug()
    << "LightEnable(" << light_id << "=" << enable << ")" << endl;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::slot_new_clip_plane
//       Access: Protected, Virtual
//  Description: This will be called by the base class before a
//               particular clip plane id will be used for the first
//               time.  It is intended to allow the derived class to
//               reserve any additional resources, if required, for
//               the new clip plane; and also to indicate whether the
//               hardware supports this many simultaneous clipping
//               planes.
//
//               The return value should be true if the additional
//               plane is supported, or false if it is not.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian9::
slot_new_clip_plane(int plane_id) {
  return (plane_id < D3DMAXUSERCLIPPLANES);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::bind_clip_plane
//       Access: Protected, Virtual
//  Description: Called the first time a particular clip_plane has been
//               bound to a given id within a frame, this should set
//               up the associated hardware clip_plane with the clip_plane's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
bind_clip_plane(PlaneNode *plane, int plane_id) {
  // Get the plane in "world coordinates".  This means the plane in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  NodePath plane_np(plane);
  const LMatrix4f &plane_mat = plane_np.get_mat(_scene_setup->get_camera_path());
  LMatrix4f rel_mat = plane_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  Planef world_plane = plane->get_plane() * rel_mat;

  _pD3DDevice->SetClipPlane(plane_id, world_plane.get_data());
}

void DXGraphicsStateGuardian9::
issue_color_write(const ColorWriteAttrib *attrib) {
  _color_write_mode = attrib->get_mode();
  set_color_writemask((_color_write_mode ==ColorWriteAttrib::M_on) ? 0xFFFFFFFF : 0x0);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::set_blend_mode
//       Access: Protected, Virtual
//  Description: Called after any of the things that might change
//               blending state have changed, this function is
//               responsible for setting the appropriate color
//               blending mode based on the current properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
set_blend_mode() {

  if((_color_write_mode == ColorWriteAttrib::M_off) && !_pScrn->bCanDirectDisableColorWrites) {
    // need !_pScrn->bCanDirectDisableColorWrites guard because other issue_colorblend,issue_transp
    // will come this way, and they should ignore the colorwriteattrib value since it's been
    // handled separately in set_color_writemask
    enable_blend(true);
    _pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    call_dxBlendFunc(D3DBLEND_ZERO, D3DBLEND_ONE);
    return;
  }

  // Is there a color blend set?
  if (_color_blend_mode != ColorBlendAttrib::M_none) {
    enable_blend(true);

    switch (_color_blend_mode) {
    case ColorBlendAttrib::M_add:
      _pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
      break;

    case ColorBlendAttrib::M_subtract:
      _pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT);
      break;

    case ColorBlendAttrib::M_inv_subtract:
      _pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
      break;

    case ColorBlendAttrib::M_min:
      _pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MIN);
      break;

    case ColorBlendAttrib::M_max:
      _pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX);
      break;
    }

    call_dxBlendFunc(get_blend_func(_color_blend->get_operand_a()),
                     get_blend_func(_color_blend->get_operand_b()));
    return;
  }

  // No color blend; is there a transparency set?
  switch (_transparency_mode) {
  case TransparencyAttrib::M_none:
  case TransparencyAttrib::M_binary:
    break;

  case TransparencyAttrib::M_alpha:
  case TransparencyAttrib::M_multisample:
  case TransparencyAttrib::M_multisample_mask:
  case TransparencyAttrib::M_dual:
    enable_blend(true);
    _pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    call_dxBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
    return;

  default:
    dxgsg9_cat.error()
      << "invalid transparency mode " << (int)_transparency_mode << endl;
    break;
  }

  // Nothing's set, so disable blending.
  enable_blend(false);
}

TypeHandle DXGraphicsStateGuardian9::get_type(void) const {
    return get_class_type();
}

TypeHandle DXGraphicsStateGuardian9::get_class_type(void) {
    return _type_handle;
}

void DXGraphicsStateGuardian9::init_type(void) {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "DXGraphicsStateGuardian9",
                  GraphicsStateGuardian::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: dx_cleanup
//  Description: Clean up the DirectX environment, accounting for exit()
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::
dx_cleanup(bool bRestoreDisplayMode,bool bAtExitFnCalled) {
  static bool bAtExitFnEverCalled=false;

    if(dxgsg9_cat.is_spam()) {
        dxgsg9_cat.spam() << "dx_cleanup called, bAtExitFnCalled=" << bAtExitFnCalled << ", bAtExitFnEverCalled=" << bAtExitFnEverCalled << endl;
    }

    bAtExitFnEverCalled = (bAtExitFnEverCalled || bAtExitFnCalled);

    // for now, I can't trust any of the ddraw/d3d releases during atexit(),
    // so just return directly.  maybe revisit this later, if have problems
    // restarting d3d/ddraw after one of these uncleaned-up exits
    //    if(bAtExitFnEverCalled)
    //      return;

    if (!_pD3DDevice)
      return;

    // unsafe to do the D3D releases after exit() called, since DLL_PROCESS_DETACH
    // msg already delivered to d3d.dll and it's unloaded itself

    wdxdisplay9_cat.debug() << "called dx_cleanup\n";
    free_nondx_resources();

    wdxdisplay9_cat.debug() << "device : " << _pD3DDevice << endl;
    PRINT_REFCNT(dxgsg9,_pD3DDevice);

    PRINT_REFCNT(dxgsg9,_pD3DDevice);

    // Do a safe check for releasing the D3DDEVICE. RefCount should be zero.
    // if we're called from exit(), _pD3DDevice may already have been released
    if (_pD3DDevice!=NULL) {
        for(int i=0;i<D3D_MAXTEXTURESTAGES;i++)
           _pD3DDevice->SetTexture(i,NULL);  // d3d should release this stuff internally anyway, but whatever
        RELEASE(_pD3DDevice,dxgsg9,"d3dDevice",RELEASE_DOWN_TO_ZERO);
        _pScrn->pD3DDevice = NULL;
    }

    // Releasing pD3D is now the responsibility of the GraphicsPipe destructor
}

void DXGraphicsStateGuardian9::  
set_context(DXScreenData *pNewContextData) {
  // dont do copy from window since dx_init sets fields too.
  // simpler to keep all of it in one place, so use ptr to window struct
  
  assert(pNewContextData!=NULL);
  _pScrn = pNewContextData;
  _pD3DDevice = _pScrn->pD3DDevice;   //copy this one field for speed of deref
  _pSwapChain = _pScrn->pSwapChain;   //copy this one field for speed of deref
  
  //wdxdisplay9_cat.debug() << "SwapChain = "<< _pSwapChain << "\n";
}

bool DXGraphicsStateGuardian9::  
create_swap_chain(DXScreenData *pNewContextData) {
  // Instead of creating a device and rendering as d3ddevice->present()
  // we should render using SwapChain->present(). This is done to support
  // multiple windows rendering. For that purpose, we need to set additional
  // swap chains here.
  
  HRESULT hr;
  hr = pNewContextData->pD3DDevice->CreateAdditionalSwapChain(&pNewContextData->PresParams, &pNewContextData->pSwapChain);
  if (FAILED(hr)) {
    wdxdisplay9_cat.debug() << "Swapchain creation failed :"<<D3DERRORSTRING(hr)<<"\n";
    return false;
  }
  return true;
}

bool DXGraphicsStateGuardian9::  
release_swap_chain(DXScreenData *pNewContextData) {
  // Release the swap chain on this DXScreenData
  HRESULT hr;
  if (pNewContextData->pSwapChain) {
    hr = pNewContextData->pSwapChain->Release();
    if (FAILED(hr)) {
      wdxdisplay9_cat.debug() << "Swapchain release failed:" << D3DERRORSTRING(hr) << "\n";
      return false;
    }
  }
  return true;
}

bool refill_tex_callback(TextureContext *tc,void *void_dxgsg_ptr) {
     DXTextureContext9 *dtc = DCAST(DXTextureContext9, tc);
//   DXGraphicsStateGuardian9 *dxgsg = (DXGraphicsStateGuardian9 *)void_dxgsg_ptr; not needed?

     // Re-fill the contents of textures and vertex buffers
     // which just got restored now.
     HRESULT hr=dtc->FillDDSurfTexturePixels();
     return hr==S_OK;
}

bool delete_tex_callback(TextureContext *tc,void *void_dxgsg_ptr) {
     DXTextureContext9 *dtc = DCAST(DXTextureContext9, tc);

     // release DDSurf (but not the texture context)
     dtc->DeleteTexture();
     return true;
}

bool recreate_tex_callback(TextureContext *tc,void *void_dxgsg_ptr) {
     DXTextureContext9 *dtc = DCAST(DXTextureContext9, tc);
     DXGraphicsStateGuardian9 *dxgsg = (DXGraphicsStateGuardian9 *)void_dxgsg_ptr;

     // Re-fill the contents of textures and vertex buffers
     // which just got restored now.

     IDirect3DTexture9 *ddtex = dtc->CreateTexture(*dxgsg->_pScrn);
     return ddtex!=NULL;
}

// release all textures and vertex/index buffers
HRESULT DXGraphicsStateGuardian9::DeleteAllDeviceObjects(void) {
  // BUGBUG: need to release any vertexbuffers here

  // cant access template in libpanda.dll directly due to vc++ limitations, use traverser to get around it

  // dont call release_all_textures() because we want the panda tex obj around so it can reload its texture

  // bugbug:  do I still need to delete all the textures since they are all D3DPOOL_MANAGED now?
  traverse_prepared_textures(delete_tex_callback,this);

  if(dxgsg9_cat.is_debug())
      dxgsg9_cat.debug() << "release of all textures complete\n";

  assert(_pD3DDevice);

  return S_OK;
}

// recreate all textures and vertex/index buffers
HRESULT DXGraphicsStateGuardian9::RecreateAllDeviceObjects(void) {
  // BUGBUG: need to handle vertexbuffer handling here

  // cant access template in libpanda.dll directly due to vc++ limitations, use traverser to get around it
  traverse_prepared_textures(recreate_tex_callback,this);

  if(dxgsg9_cat.is_debug())
      dxgsg9_cat.debug() << "recreation of all textures complete\n";
  return S_OK;
}

HRESULT DXGraphicsStateGuardian9::ReleaseAllDeviceObjects(void) {
    // release any D3DPOOL_DEFAULT objects here (currently none)
    return S_OK;
}

#if 0
////////////////////////////////////////////////////////////////////
//     Function: show_frame
//       Access:
//       Description:   redraw primary buffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::show_frame(bool bNoNewFrameDrawn) {
  if(_pD3DDevice==NULL)
    return;

  //  DO_PSTATS_STUFF(PStatTimer timer(_win->_swap_pcollector));  // this times just the flip, so it must go here in dxgsg, instead of wdxdisplay, which would time the whole frame
  HRESULT hr;

  if(bNoNewFrameDrawn) {
      // a new frame has not been rendered, we just want to display the last thing
      // that was drawn into backbuf, if backbuf is valid
      if(_pScrn->PresParams.SwapEffect==D3DSWAPEFFECT_DISCARD) {
          // in DISCARD mode, old backbufs are not guaranteed to have valid pixels,
          // so we cant copy back->front here.  just give up.
          return;
      } else if(_pScrn->PresParams.SwapEffect==D3DSWAPEFFECT_FLIP) {
         /* bugbug:  here we should use CopyRects here to copy backbuf to front (except in
                     the case of frames 1 and 2 where we have no valid data in the backbuffer yet,
                     for those cases give up and return).
                     not implemented yet since right now we always do discard mode for fullscrn Present()
                     for speed.
          */
          return;
      }

      // otherwise we have D3DSWAPEFFECT_COPY, so fall-thru to normal Present()
      // may work ok as long as backbuf hasnt been touched
  }

  hr = _pD3DDevice->Present((CONST RECT*)NULL,(CONST RECT*)NULL,(HWND)NULL,NULL);
  if(FAILED(hr)) {
    if(hr == D3DERR_DEVICELOST) {
        CheckCooperativeLevel();
    } else {
      dxgsg9_cat.error() << "show_frame() - Present() failed" << D3DERRORSTRING(hr);
      exit(1);
    }
  }
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: show_frame
//       Access:
//       Description:   redraw primary buffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::show_frame(bool bNoNewFrameDrawn) {
  if(_pD3DDevice==NULL)
    return;

  //  DO_PSTATS_STUFF(PStatTimer timer(_win->_swap_pcollector));  // this times just the flip, so it must go here in dxgsg, instead of wdxdisplay, which would time the whole frame
  HRESULT hr;

  if(bNoNewFrameDrawn) {
      // a new frame has not been rendered, we just want to display the last thing
      // that was drawn into backbuf, if backbuf is valid
      if(_pScrn->PresParams.SwapEffect==D3DSWAPEFFECT_DISCARD) {
          // in DISCARD mode, old backbufs are not guaranteed to have valid pixels,
          // so we cant copy back->front here.  just give up.
          return;
      } else if(_pScrn->PresParams.SwapEffect==D3DSWAPEFFECT_FLIP) {
         /* bugbug:  here we should use CopyRects here to copy backbuf to front (except in
                     the case of frames 1 and 2 where we have no valid data in the backbuffer yet,
                     for those cases give up and return).
                     not implemented yet since right now we always do discard mode for fullscrn Present()
                     for speed.
          */
          return;
      }

      // otherwise we have D3DSWAPEFFECT_COPY, so fall-thru to normal Present()
      // may work ok as long as backbuf hasnt been touched
  }

  if (_pSwapChain)
    hr = _pSwapChain->Present((CONST RECT*)NULL,(CONST RECT*)NULL,(HWND)NULL,NULL, 0);
  else
    hr = _pD3DDevice->Present((CONST RECT*)NULL,(CONST RECT*)NULL,(HWND)NULL,NULL);

  if(FAILED(hr)) {
    if(hr == D3DERR_DEVICELOST) {
        CheckCooperativeLevel();
    } else {
      dxgsg9_cat.error() << "show_frame() - Present() failed" << D3DERRORSTRING(hr);
      exit(1);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: set_render_target
//       Access:
//       Description: Set render target to the backbuffer of
//                    current swap chain.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::set_render_target() {
  LPDIRECT3DSURFACE9 pBack=NULL, pStencil=NULL;

  if (!_pSwapChain)  //maybe fullscreen mode or main/single window
    _pD3DDevice->GetBackBuffer(0, 0,D3DBACKBUFFER_TYPE_MONO,&pBack);
  else
    _pSwapChain->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&pBack);

  //wdxdisplay9_cat.debug() << "swapchain is " << _pSwapChain << "\n";
  //wdxdisplay9_cat.debug() << "back buffer is " << pBack << "\n";

  _pD3DDevice->GetDepthStencilSurface(&pStencil);
  _pD3DDevice->SetDepthStencilSurface(pStencil);
  _pD3DDevice->SetRenderTarget(0, pBack);
  if (pBack)
    pBack->Release();
  if (pStencil)
    pStencil->Release();
}

////////////////////////////////////////////////////////////////////
//     Function: copy_pres_reset
//       Access:
//       Description: copies the PresReset from passed DXScreenData
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian9::copy_pres_reset(DXScreenData *pScrn) {
  memcpy(&_PresReset, &_pScrn->PresParams,sizeof(D3DPRESENT_PARAMETERS));
}

/////////////////////////////////////////////////////////////////////////////////////
//  Function: reset_d3d_device
//  Access:
//  Description: This function checks current device's framebuffer dimension against
//               passed pPresParams backbuffer dimension to determine a device reset
//               if there is only one window or it is the main window or fullscreen
//               mode then, it resets the device. Finally it returns the new
//               DXScreenData through parameter pScrn
/////////////////////////////////////////////////////////////////////////////////////
HRESULT DXGraphicsStateGuardian9::
reset_d3d_device(D3DPRESENT_PARAMETERS *pPresParams, DXScreenData **pScrn) {
  HRESULT hr;

  assert(IS_VALID_PTR(pPresParams));
  assert(IS_VALID_PTR(_pScrn->pD3D9));
  assert(IS_VALID_PTR(_pD3DDevice));

  ReleaseAllDeviceObjects();

  // for windowed mode make sure our format matches the desktop fmt,
  // in case the desktop mode has been changed
  
  _pScrn->pD3D9->GetAdapterDisplayMode(_pScrn->CardIDNum, &_pScrn->DisplayMode);
  pPresParams->BackBufferFormat = _pScrn->DisplayMode.Format;

  // here we have to look at the _PresReset frame buffer dimension
  // if current window's dimension is bigger than _PresReset 
  // we have to reset the device before creating new swapchain.
  // inorder to reset properly, we need to release all swapchains

  if ( !(_pScrn->pSwapChain)
       || (_PresReset.BackBufferWidth < pPresParams->BackBufferWidth)
       || (_PresReset.BackBufferHeight < pPresParams->BackBufferHeight) ) {

    wdxdisplay9_cat.debug() << "Swpachain = " << _pScrn->pSwapChain << " _PresReset = "
                            << _PresReset.BackBufferWidth << "x" << _PresReset.BackBufferHeight << "pPresParams = "
                            << pPresParams->BackBufferWidth << "x" << pPresParams->BackBufferHeight << "\n";

    get_engine()->reset_all_windows(false);// reset old swapchain by releasing

    if (_pScrn->pSwapChain) {  //other windows might be using bigger buffers
      _PresReset.BackBufferWidth = max(_PresReset.BackBufferWidth, pPresParams->BackBufferWidth);
      _PresReset.BackBufferHeight = max(_PresReset.BackBufferHeight, pPresParams->BackBufferHeight);
    }
    else {  // single window, must reset to the new pPresParams dimension
      _PresReset.BackBufferWidth = pPresParams->BackBufferWidth;
      _PresReset.BackBufferHeight = pPresParams->BackBufferHeight;
    }

    hr=_pD3DDevice->Reset(&_PresReset);
    if (FAILED(hr)) {
      return hr;
    }

    get_engine()->reset_all_windows(true);// reset with new swapchains by creating

    if (pScrn)
      *pScrn = NULL;
    if(pPresParams!=&_pScrn->PresParams)
      memcpy(&_pScrn->PresParams,pPresParams,sizeof(D3DPRESENT_PARAMETERS));
    return hr;
  }

  // release the old swapchain and create a new one
  if (_pScrn && _pScrn->pSwapChain) {
    _pScrn->pSwapChain->Release();
    wdxdisplay9_cat.debug() << "SwapChain " << _pScrn->pSwapChain << " is released\n";
    _pScrn->pSwapChain = NULL;
    hr=_pD3DDevice->CreateAdditionalSwapChain(pPresParams,&_pScrn->pSwapChain);
  }
  if(SUCCEEDED(hr)) {
     if(pPresParams!=&_pScrn->PresParams)
         memcpy(&_pScrn->PresParams,pPresParams,sizeof(D3DPRESENT_PARAMETERS));
     if (pScrn) 
       *pScrn = _pScrn;
  }
  return hr;
}

bool DXGraphicsStateGuardian9::
CheckCooperativeLevel(bool bDoReactivateWindow) {
  HRESULT hr = _pD3DDevice->TestCooperativeLevel();

  if(SUCCEEDED(hr)) {
    assert(SUCCEEDED(_last_testcooplevel_result));
    return true;
  }

  switch(hr) {
      case D3DERR_DEVICENOTRESET:
        _bDXisReady = false;
        hr=reset_d3d_device(&_pScrn->PresParams);
        if (FAILED(hr)) {
          // I think this shouldnt fail unless I've screwed up the PresParams from the original working ones somehow
          dxgsg9_cat.error()
            << "CheckCooperativeLevel Reset() failed, hr = " << D3DERRORSTRING(hr);
          // drose is commenting out this exit() call; it's getting
          // triggered on some actual client hardware (with
          // DRIVERINTERNALERROR) but maybe that's ok.
          //exit(1);
        }
    
        // BUGBUG: is taking this out wrong??
        /*
        if(bDoReactivateWindow) {
           _win->reactivate_window();  //must reactivate window before you can restore surfaces (otherwise you are in WRONGVIDEOMODE, and DDraw RestoreAllSurfaces fails)
        }
        */

        hr = _pD3DDevice->TestCooperativeLevel();
        if(FAILED(hr)) {
          // internal chk, shouldnt fail
          dxgsg9_cat.error()
            << "TestCooperativeLevel following Reset() failed, hr = " << D3DERRORSTRING(hr);

          // drose is commenting out this exit() call; maybe it's ok if the above fails.
          //exit(1);
        }
    
        _bDXisReady = TRUE;
        break;
    
      case D3DERR_DEVICELOST:
        if(SUCCEEDED(_last_testcooplevel_result)) {
          if(_bDXisReady) {
            //                   _win->deactivate_window();
            _bDXisReady = false;
            if(dxgsg9_cat.is_debug())
              dxgsg9_cat.debug() << "D3D Device was Lost, waiting...\n";
          }
        }
  }

  _last_testcooplevel_result = hr;
  return SUCCEEDED(hr);
}

HRESULT CreateDX9Cursor(LPDIRECT3DDEVICE9 pd3dDevice, HCURSOR hCursor,BOOL bAddWatermark) {
// copied directly from dxsdk SetDeviceCursor
    HRESULT hr = E_FAIL;
    ICONINFO iconinfo;
    LPDIRECT3DSURFACE9 pCursorBitmap = NULL;
    HDC hdcColor = NULL;
    HDC hdcMask = NULL;
    HDC hdcScreen = NULL;
    BITMAP bm;
    DWORD dwWidth,dwHeightSrc,dwHeightDest;
    COLORREF crColor,crMask;
    UINT x,y;
    BITMAPINFO bmi;
    COLORREF* pcrArrayColor = NULL;
    COLORREF* pcrArrayMask = NULL;
    DWORD* pBitmap;
    HGDIOBJ hgdiobjOld;
    bool bBWCursor;

    ZeroMemory( &iconinfo, sizeof(iconinfo) );
    if( !GetIconInfo( hCursor, &iconinfo ) )
        goto End;

    if (0 == GetObject((HGDIOBJ)iconinfo.hbmMask, sizeof(BITMAP), (LPVOID)&bm))
        goto End;
    dwWidth = bm.bmWidth;
    dwHeightSrc = bm.bmHeight;

    if( iconinfo.hbmColor == NULL ) {
        bBWCursor = true;
        dwHeightDest = dwHeightSrc / 2;
    } else {
        bBWCursor = false;
        dwHeightDest = dwHeightSrc;
    }

    // Create a surface for the cursor
    if( FAILED( hr = pd3dDevice->CreateOffscreenPlainSurface( dwWidth, dwHeightDest,
        D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pCursorBitmap, NULL ) ) ) {
        goto End;
    }

    pcrArrayMask = new DWORD[dwWidth * dwHeightSrc];

    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = dwWidth;
    bmi.bmiHeader.biHeight = dwHeightSrc;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hdcScreen = GetDC( NULL );
    hdcMask = CreateCompatibleDC( hdcScreen );
    if( hdcMask == NULL )
    {
        hr = E_FAIL;
        goto End;
    }
    hgdiobjOld = SelectObject(hdcMask, iconinfo.hbmMask);
    GetDIBits(hdcMask, iconinfo.hbmMask, 0, dwHeightSrc,
        pcrArrayMask, &bmi, DIB_RGB_COLORS);
    SelectObject(hdcMask, hgdiobjOld);

    if (!bBWCursor)
    {
        pcrArrayColor = new DWORD[dwWidth * dwHeightDest];
        hdcColor = CreateCompatibleDC( GetDC( NULL ) );
        if( hdcColor == NULL )
        {
            hr = E_FAIL;
            goto End;
        }
        SelectObject(hdcColor, iconinfo.hbmColor);
        GetDIBits(hdcColor, iconinfo.hbmColor, 0, dwHeightDest,
            pcrArrayColor, &bmi, DIB_RGB_COLORS);
    }

    // Transfer cursor image into the surface
    D3DLOCKED_RECT lr;
    pCursorBitmap->LockRect( &lr, NULL, 0 );
    pBitmap = (DWORD*)lr.pBits;
    for( y = 0; y < dwHeightDest; y++ )
    {
        for( x = 0; x < dwWidth; x++ )
        {
            if (bBWCursor)
            {
                crColor = pcrArrayMask[dwWidth*(dwHeightDest-1-y) + x];
                crMask = pcrArrayMask[dwWidth*(dwHeightSrc-1-y) + x];
            }
            else
            {
                crColor = pcrArrayColor[dwWidth*(dwHeightDest-1-y) + x];
                crMask = pcrArrayMask[dwWidth*(dwHeightDest-1-y) + x];
            }
            if (crMask == 0)
                pBitmap[dwWidth*y + x] = 0xff000000 | crColor;
            else
                pBitmap[dwWidth*y + x] = 0x00000000;

            // It may be helpful to make the D3D cursor look slightly
            // different from the Windows cursor so you can distinguish
            // between the two when developing/testing code.  When
            // bAddWatermark is TRUE, the following code adds some
            // small grey "D3D" characters to the upper-left corner of
            // the D3D cursor image.
            if( bAddWatermark && x < 12 && y < 5 )
            {
                // 11.. 11.. 11.. .... CCC0
                // 1.1. ..1. 1.1. .... A2A0
                // 1.1. .1.. 1.1. .... A4A0
                // 1.1. ..1. 1.1. .... A2A0
                // 11.. 11.. 11.. .... CCC0

                const WORD wMask[5] = { 0xccc0, 0xa2a0, 0xa4a0, 0xa2a0, 0xccc0 };
                if( wMask[y] & (1 << (15 - x)) )
                {
                    pBitmap[dwWidth*y + x] |= 0xff808080;
                }
            }
        }
    }
    pCursorBitmap->UnlockRect();

    // Set the device cursor
    if( FAILED( hr = pd3dDevice->SetCursorProperties( iconinfo.xHotspot,
        iconinfo.yHotspot, pCursorBitmap ) ) )
    {
        goto End;
    }

    hr = S_OK;

End:
    if( iconinfo.hbmMask != NULL )
        DeleteObject( iconinfo.hbmMask );
    if( iconinfo.hbmColor != NULL )
        DeleteObject( iconinfo.hbmColor );
    if( hdcScreen != NULL )
        ReleaseDC( NULL, hdcScreen );
    if( hdcColor != NULL )
        DeleteDC( hdcColor );
    if( hdcMask != NULL )
        DeleteDC( hdcMask );

    SAFE_DELETE_ARRAY( pcrArrayColor );
    SAFE_DELETE_ARRAY( pcrArrayMask );
    RELEASE(pCursorBitmap,dxgsg9,"pCursorBitmap",RELEASE_ONCE);
    return hr;
}

#ifdef _DEBUG
// defns for print formatting in debugger
typedef struct {
  float x,y,z;
  float nx,ny,nz;
  D3DCOLOR diffuse;
  float u,v;
} POS_NORM_COLOR_TEX_VERTEX;

typedef struct {
  float x,y,z;
  D3DCOLOR diffuse;
  float u,v;
} POS_COLOR_TEX_VERTEX;

typedef struct {
  float x,y,z;
  float u,v;
} POS_TEX_VERTEX;

// define junk vars so symbols are included in dbginfo
POS_TEX_VERTEX junk11;
POS_COLOR_TEX_VERTEX junk22;
POS_NORM_COLOR_TEX_VERTEX junk33;
#endif

