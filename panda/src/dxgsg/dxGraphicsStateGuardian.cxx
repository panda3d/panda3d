// Filename: dxGraphicsStateGuardian.cxx
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

#include <pandabase.h>
#include <directRenderTraverser.h>
#include <cullTraverser.h>
#include <displayRegion.h>
#include <projectionNode.h>
#include <camera.h>
#include <renderBuffer.h>
#include <geom.h>
#include <geomSphere.h>
#include <geomIssuer.h>
#include <graphicsWindow.h>
#include <graphicsChannel.h>
#include <projection.h>
#include <get_rel_pos.h>
#include <perspectiveProjection.h>
#include <ambientLight.h>
#include <directionalLight.h>
#include <pointLight.h>
#include <spotlight.h>
#include <projectionNode.h>
#include <transformTransition.h>
#include <colorTransition.h>
#include <lightTransition.h>
#include <textureTransition.h>
#include <renderModeTransition.h>
#include <materialTransition.h>
#include <colorBlendTransition.h>
#include <colorMaskTransition.h>
#include <texMatrixTransition.h>
#include <texGenTransition.h>
#include <textureApplyTransition.h>
#include <clipPlaneTransition.h>
#include <transparencyTransition.h>
#include <fogTransition.h>
#include <linesmoothTransition.h>
#include <depthTestTransition.h>
#include <depthWriteTransition.h>
#include <cullFaceTransition.h>
#include <stencilTransition.h>

#ifdef DO_PSTATS
#include <pStatTimer.h>
#include <pStatCollector.h>
#endif

#include "config_dxgsg.h"
#include "dxGraphicsStateGuardian.h"

#include <mmsystem.h>

// print out simple drawprim stats every few secs
//#define COUNT_DRAWPRIMS

//#define PRINT_TEXSTATS

//#define DISABLE_DECALING
#define DISABLE_POLYGON_OFFSET_DECALING
// currently doesnt work well enough in toontown models for us to use
// prob is when viewer gets close to decals, they disappear into wall poly, need to investigate

// test non-optimized general geom pipe for all models
// apparently DPStrided faults for some color G_OVERALL cases, so comment out for now
// not clear that it is actually faster in practice, it may even be slightly slower
#define DONT_USE_DRAWPRIMSTRIDED

//const int VERT_BUFFER_SIZE = (8*1024L);
// For sparkle particles, we can have 4 vertices per sparkle, and a
// particle pool size of 1024 particles

// for sprites, 1000 prims, 6 verts/prim, 24 bytes/vert
const int VERT_BUFFER_SIZE = (32*6*1024L);

// if defined, pandadx only handles 1 panda display region
// note multiple region code doesnt work now (see prepare_display_region,set_clipper)
#define NO_MULTIPLE_DISPLAY_REGIONS

TypeHandle DXGraphicsStateGuardian::_type_handle;

// bit masks used for drawing primitives
#define PER_TEXCOORD 0x8
#define PER_COLOR    0x4
#define PER_NORMAL   0x2
#define PER_COORD    0x1

// technically DX7's front-end has no limit on the number of lights, but it's simpler for
// this implementation to set a small GL-like limit to make the light array traversals short
// and so I dont have to write code that reallocs light arrays
#define DXGSG_MAX_LIGHTS 8

static D3DMATRIX matIdentity;

#ifdef COUNT_DRAWPRIMS

static DWORD cDPcount=0;
static DWORD cVertcount=0;
static DWORD cTricount=0;
static DWORD cGeomcount=0;

static LPDIRECTDRAWSURFACE7 pLastTexture=NULL;
static DWORD cDP_noTexChangeCount=0;
static LPDIRECT3DDEVICE7 global_pD3DDevice = NULL;

static void CountDPs(DWORD nVerts,DWORD nTris) {
    cDPcount++;
    cVertcount+=nVerts;
    cTricount+=nTris;

    LPDIRECTDRAWSURFACE7 pCurTexture;
    global_pD3DDevice->GetTexture(0,&pCurTexture);
    if(pCurTexture==pLastTexture) {
        cDP_noTexChangeCount++;
    } else pLastTexture = pCurTexture;
}
#else
#define CountDPs(nv,nt)
#endif

#if defined(DO_PSTATS) || defined(PRINT_TEXSTATS)
static bool bTexStatsRetrievalImpossible=false;
#endif

//#define Colorf_to_D3DCOLOR(out_color) (D3DRGBA((out_color)[0], (out_color)[1], (out_color)[2], (out_color)[3]))

INLINE DWORD
Colorf_to_D3DCOLOR(Colorf &cColorf) {
// MS VC defines _M_IX86 for x86.  gcc should define _X86_
#if defined(_M_IX86) || defined(_X86_)
    DWORD d3dcolor,tempcolorval=255;
//    DWORD *Colorf_addr=(DWORD*)&cColorf;

    // note the default FPU rounding mode will give 255*0.5f=0x80, not 0x7F as VC would force it to by resetting rounding mode
    // dont think this makes much difference

    __asm {
;        push eax
        push ebx   ; want to save this in case this fn is inlined
        push ecx
;        mov ecx, Colorf_addr  ; must be a better way to do this w/o using ecx
        mov ecx, cColorf
        fild tempcolorval
        fld [ecx]
        fmul ST(0),ST(1)
        fistp tempcolorval  ; no way to store directly to int register
        mov eax, tempcolorval
        shl eax, 16

        fld DWORD PTR [ecx+4]  ;grn
        fmul ST(0),ST(1)
        fistp tempcolorval   
        mov ebx,tempcolorval
        shl ebx, 8
        or eax,ebx

        fld DWORD PTR [ecx+8]  ;blue
        fmul ST(0),ST(1)
        fistp tempcolorval
        or eax,tempcolorval

        fld DWORD PTR [ecx+12] ;alpha
        fmul ST(0),ST(1)
        fistp tempcolorval
        ; simulate pop 255.0 off FP stack w/o store, mark top as empty and increment stk ptr
        ffree ST(0)
        fincstp
        mov ebx,tempcolorval
        shl ebx, 24
        or eax,ebx
        mov d3dcolor,eax
        pop ecx
        pop ebx
;        pop eax
    }

//   dxgsg_cat.debug() << (void*)d3dcolor << endl;
   return d3dcolor;
#else //!_X86_
   return D3DRGBA(cColorf[0], cColorf[1], cColorf[2], cColorf[3]);
#endif //!_X86_
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian::
DXGraphicsStateGuardian(GraphicsWindow *win) : GraphicsStateGuardian(win) {
    _light_enabled = (bool *)NULL;
    _cur_light_enabled = (bool *)NULL;
    _clip_plane_enabled = (bool *)NULL;
    _cur_clip_plane_enabled = (bool *)NULL;
    _pCurFvfBufPtr = NULL;
    _pFvfBufBasePtr = new char[VERT_BUFFER_SIZE];  // allocate storage for vertex info.
    _index_buf = new WORD[D3DMAXNUMVERTICES];  // allocate storage for vertex index info.

    _dx_ready = false;

    _CurShadeMode =  D3DSHADE_FLAT;

    _pri = _zbuf = _back = NULL;
    _pDD = NULL;
    _d3dDevice = NULL;

    _cur_read_pixel_buffer=RenderBuffer::T_front;

    ZeroMemory(&matIdentity,sizeof(D3DMATRIX));
    matIdentity._11 = matIdentity._22 = matIdentity._33 = matIdentity._44 = 1.0f;

    _cNumTexPixFmts = 0;
    _pTexPixFmts = NULL;
    _pCurTexContext = NULL;

    // Create a default RenderTraverser.
    if (dx_cull_traversal) {
        _render_traverser = new CullTraverser(this, RenderRelation::get_class_type());
    } else {
        _render_traverser = new DirectRenderTraverser(this, RenderRelation::get_class_type());
    }

    //Color and alpha transform variables
    _color_transform_enabled = false;
    _alpha_transform_enabled = false;
    _current_color_mat = LMatrix4f::ident_mat();
    _current_alpha_offset = 0;
    _current_alpha_scale = 1;

    reset();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian::
~DXGraphicsStateGuardian() {
    if (_d3dDevice != NULL)
        _d3dDevice->SetTexture(0, NULL);  // this frees reference to the old texture
    _pCurTexContext = NULL;

    free_pointers();
    delete [] _pFvfBufBasePtr;
    delete [] _index_buf;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
reset() {
    free_pointers();
    GraphicsStateGuardian::reset();
    _buffer_mask = 0;

    // All implementations have the following buffers.
    _buffer_mask = (RenderBuffer::T_color |
                    RenderBuffer::T_depth |
                    RenderBuffer::T_back
//                  RenderBuffer::T_stencil |
//                  RenderBuffer::T_accum
                    );

    _current_projection_mat = LMatrix4f::ident_mat();
    _projection_mat_stack_count = 0;

    _issued_color_enabled = false;
    _enable_all_color = true;

//   this is incorrect for general mono displays, need both right and left flags set.
//   stereo has not been handled yet for dx
//    _buffer_mask &= ~RenderBuffer::T_right;

    // Set up our clear values to invalid values, so the glClear* calls
    // will be made initially.
    _clear_color_red = -1.0f;
    _clear_color_green = -1.0f;
    _clear_color_blue = -1.0f;
    _clear_color_alpha = -1.0f;
    _clear_depth = -1.0f;
    _clear_stencil = -1;
    _clear_accum_red = -1.0f;
    _clear_accum_green = -1.0f;
    _clear_accum_blue = -1.0f;
    _clear_accum_alpha = -1.0f;
    _line_width = 1.0f;
    _point_size = 1.0f;
    _depth_mask = false;
    _fog_mode = D3DFOG_EXP;
    _alpha_func = D3DCMP_ALWAYS;
    _alpha_func_ref = 0;
//  _polygon_mode = GL_FILL;

//  _pack_alignment = 4;
//  _unpack_alignment = 4;

    // Set up all the enabled/disabled flags to GL's known initial
    // values: everything off.
    _multisample_enabled = false;
    _line_smooth_enabled = false;
    _point_smooth_enabled = false;
    _color_material_enabled = false;
//  _scissor_enabled = false;
    _lighting_enabled = false;
    _normals_enabled = false;
    _texturing_enabled = false;
    _multisample_alpha_one_enabled = false;
    _multisample_alpha_mask_enabled = false;
    _blend_enabled = false;
    _depth_test_enabled = false;
    _fog_enabled = false;
    _alpha_test_enabled = false;
    _decal_level = 0;

    _dx_ready = false;
}

HRESULT CALLBACK EnumTexFmtsCallback( LPDDPIXELFORMAT pddpf, VOID* param ) {
    // wont build if its a member fn, so had to do this stuff
    DXGraphicsStateGuardian *mystate = (DXGraphicsStateGuardian *) param;
    assert(mystate->_cNumTexPixFmts < MAX_DX_TEXPIXFMTS);
    memcpy( &(mystate->_pTexPixFmts[mystate->_cNumTexPixFmts]), pddpf, sizeof(DDPIXELFORMAT) );
    mystate->_cNumTexPixFmts++;
    return DDENUMRET_OK;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Handles initialization which assumes that DX has already been
//               set up.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
init_dx(  LPDIRECTDRAW7     context,
          LPDIRECTDRAWSURFACE7  pri,
          LPDIRECTDRAWSURFACE7  back,
          LPDIRECTDRAWSURFACE7  zbuf,
          LPDIRECT3D7          pD3D,
          LPDIRECT3DDEVICE7    pDevice,
          RECT viewrect) {
    _pDD = context;
    _pri = pri;
    _back = back;
    _zbuf = zbuf;
    _d3d = pD3D;
    _d3dDevice = pDevice;
    _view_rect = viewrect;

#ifdef COUNT_DRAWPRIMS
     global_pD3DDevice = pDevice;
#endif
    _pCurrentGeomContext = NULL;
    _bDrawPrimDoSetupVertexBuffer = false; 

    _last_testcooplevel_result = S_OK;

    // only 1 channel on dx currently
    _panda_gfx_channel = _win->get_channel(0);

    HRESULT hr;

    if(dx_show_fps_meter) {
        _start_time = timeGetTime();
        _current_fps = 0.0f;
        _start_frame_count = _cur_frame_count = 0;
    }

    _pTexPixFmts = new DDPIXELFORMAT[MAX_DX_TEXPIXFMTS];

    assert(_pTexPixFmts!=NULL);

    if (pDevice->EnumTextureFormats(EnumTexFmtsCallback, this) != S_OK) {
        dxgsg_cat.error() << "EnumTextureFormats failed!!\n";
    }

    if (FAILED(hr = pDevice->GetCaps(&_D3DDevDesc))) {
        dxgsg_cat.fatal() << "GetCaps failed on D3D Device\n";
        exit(1);
    }

    DX_DECLARE_CLEAN(DDCAPS,ddCaps);
    if (FAILED(hr = _pDD->GetCaps(&ddCaps,NULL))) {
        dxgsg_cat.fatal() << "GetCaps failed on DDraw\n";
        exit(1);
    }

    _bIsTNLDevice = (IsEqualGUID(_D3DDevDesc.deviceGUID,IID_IDirect3DTnLHalDevice)!=0);

    if ((dx_decal_type==GDT_offset) && !(_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBIAS)) {
#ifdef _DEBUG

        // dx7 doesnt support PLANEMASK renderstate
#if(DIRECT3D_VERSION < 0x700)
        dxgsg_cat.error() << "dx-decal-type 'offset' not supported by hardware, switching to decal masking\n";
#else
        dxgsg_cat.error() << "dx-decal-type 'offset' not supported by hardware, switching to decal double-draw blend-based masking\n";
#endif
#endif
#if(DIRECT3D_VERSION < 0x700)
        dx_decal_type = GDT_mask;
#else
        dx_decal_type = GDT_blend;
#endif
    }

#ifdef DISABLE_POLYGON_OFFSET_DECALING
#ifdef _DEBUG
    dxgsg_cat.spam() << "polygon-offset decaling disabled in dxgsg, switching to double-draw decaling\n";
#endif

#if(DIRECT3D_VERSION < 0x700)
    dx_decal_type = GDT_mask;
#else
    dx_decal_type = GDT_blend;
#endif
#endif

    if ((dx_decal_type==GDT_mask) && !(_D3DDevDesc.dpcTriCaps.dwMiscCaps & D3DPMISCCAPS_MASKPLANES)) {
#ifdef _DEBUG
        dxgsg_cat.error() << "No hardware support for colorwrite disabling, switching to dx-decal-type 'mask' to 'blend'\n";
#endif
        dx_decal_type = GDT_blend;
    }

    if (((dx_decal_type==GDT_blend)||(dx_decal_type==GDT_mask)) && !(_D3DDevDesc.dpcTriCaps.dwMiscCaps & D3DPMISCCAPS_MASKZ)) {
        dxgsg_cat.error() << "dx-decal-type mask impossible to implement, no hardware support for Z-masking, decals will not appear correctly\n";
    }

//#define REQUIRED_BLENDCAPS (D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE|D3DPBLENDCAPS_SRCCOLOR|D3DPBLENDCAPS_INVSRCCOLOR| \
//                            D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_INVSRCALPHA | D3DPBLENDCAPS_DESTALPHA|D3DPBLENDCAPS_INVDESTALPHA|D3DPBLENDCAPS_DESTCOLOR|D3DPBLENDCAPS_INVDESTCOLOR)
// voodoo3 doesnt support commented out ones, & we dont need them now

#define REQUIRED_BLENDCAPS (D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE|  /*D3DPBLENDCAPS_SRCCOLOR|D3DPBLENDCAPS_INVSRCCOLOR| */ \
                            D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_INVSRCALPHA /* | D3DPBLENDCAPS_DESTALPHA|D3DPBLENDCAPS_INVDESTALPHA|D3DPBLENDCAPS_DESTCOLOR|D3DPBLENDCAPS_INVDESTCOLOR*/)

    if (((_D3DDevDesc.dpcTriCaps.dwSrcBlendCaps & REQUIRED_BLENDCAPS)!=REQUIRED_BLENDCAPS) ||
        ((_D3DDevDesc.dpcTriCaps.dwDestBlendCaps & REQUIRED_BLENDCAPS)!=REQUIRED_BLENDCAPS)) {
        dxgsg_cat.error() << "device is missing alpha blending capabilities, blending may not work correctly: SrcBlendCaps: 0x"<< (void*) _D3DDevDesc.dpcTriCaps.dwSrcBlendCaps << "  DestBlendCaps: "<< (void*) _D3DDevDesc.dpcTriCaps.dwDestBlendCaps << endl;
    }

    if (!(_D3DDevDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY)) {
        dxgsg_cat.error() << "device is missing texture transparency capability, transparency may not work correctly!  TextureCaps: 0x"<< (void*) _D3DDevDesc.dpcTriCaps.dwTextureCaps << endl;
    }

    // just require trilinear.  if it can do that, it can probably do all the lesser point-sampling variations too
#define REQUIRED_TEXFILTERCAPS (D3DPTFILTERCAPS_MAGFLINEAR |  D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_LINEAR)
    if ((_D3DDevDesc.dpcTriCaps.dwTextureFilterCaps & REQUIRED_TEXFILTERCAPS)!=REQUIRED_TEXFILTERCAPS) {
        dxgsg_cat.error() << "device is missing texture bilinear filtering capability, textures may appear blocky!  TextureFilterCaps: 0x"<< (void*) _D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
    }
#define REQUIRED_MIPMAP_TEXFILTERCAPS (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_LINEARMIPLINEAR)

    if (!(ddCaps.ddsCaps.dwCaps & DDSCAPS_MIPMAP)) {
        dxgsg_cat.debug() << "device does not have mipmap texturing filtering capability!   TextureFilterCaps: 0x"<< (void*) _D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
        dx_ignore_mipmaps = TRUE;
    } else if ((_D3DDevDesc.dpcTriCaps.dwTextureFilterCaps & REQUIRED_MIPMAP_TEXFILTERCAPS)!=REQUIRED_MIPMAP_TEXFILTERCAPS) {
        dxgsg_cat.debug() << "device is missing tri-linear mipmap filtering capability, texture mipmaps may not supported! TextureFilterCaps: 0x"<< (void*) _D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
    }

#define REQUIRED_TEXBLENDCAPS (D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2)
    if ((_D3DDevDesc.dwTextureOpCaps & REQUIRED_TEXBLENDCAPS)!=REQUIRED_TEXBLENDCAPS) {
        dxgsg_cat.error() << "device is missing some required texture blending capabilities, texture blending may not work properly! TextureOpCaps: 0x"<< (void*) _D3DDevDesc.dwTextureOpCaps << endl;
    }

    if(_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE ) {
        // watch out for drivers that emulate per-pixel fog with per-vertex fog (Riva128, Matrox Millen G200)
        // some of these require gouraud-shading to be set to work, as if you were using vertex fog
        _doFogType=PerPixelFog;
    } else {
        // every card is going to have vertex fog, since it's implemented in d3d runtime
        assert((_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX )!=0);

        // vtx fog may look crappy if you have large polygons in the foreground and they get clipped,
        // so you may want to disable it

        if(dx_no_vertex_fog) {
            _doFogType = None;
        } else {
            _doFogType = PerVertexFog;

            // range-based fog only works with vertex fog in dx7/8
            if(dx_use_rangebased_fog && (_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGRANGE))
                _d3dDevice->SetRenderState(D3DRENDERSTATE_RANGEFOGENABLE, true);
        }
    }

    SetRect(&clip_rect, 0,0,0,0);     // no clip rect set

    // Lighting, let's turn it off by default
    _lighting_enabled = true;
    enable_lighting(false);

    // turn on dithering if the rendertarget is < 8bits/color channel
    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_back);
    _back->GetSurfaceDesc(&ddsd_back);
    _dither_enabled = ((ddsd_back.ddpfPixelFormat.dwRGBBitCount < 24) &&
                       (_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER));
   _d3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, _dither_enabled);

   _d3dDevice->SetRenderState(D3DRENDERSTATE_CLIPPING,true);

    // Stencil test is off by default
    _stencil_test_enabled = false;
//  _stencil_func = D3DCMP_NOTEQUAL;
//  _stencil_op = D3DSTENCILOP_REPLACE;

    // Antialiasing.
    enable_line_smooth(false);
//  enable_multisample(true);

    _d3dDevice->SetRenderState(D3DRENDERSTATE_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);

    // technically DX7's front-end has no limit on the number of lights, but it's simpler for
    // this implementation to set a small GL-like limit to make the light array traversals short
    // and so I dont have to write code that reallocs light arrays
    assert((_D3DDevDesc.dwMaxActiveLights==0) ||  // 0 means infinite lights
           (DXGSG_MAX_LIGHTS <= _D3DDevDesc.dwMaxActiveLights));

    _max_lights = DXGSG_MAX_LIGHTS;

    _available_light_ids = PTA(Light*)(_max_lights);
    _light_enabled = new bool[_max_lights];
    _cur_light_enabled = new bool[_max_lights];

    int i;
    for (i = 0; i < _max_lights; i++) {
        _available_light_ids[i] = NULL;
        _light_enabled[i] = false;
    }

    if(dx_auto_normalize_lighting)
         _d3dDevice->SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, true);

    // Set up the clip plane id map
    _max_clip_planes = D3DMAXUSERCLIPPLANES;
    _available_clip_plane_ids = PTA(PlaneNode*)(_max_clip_planes);
    _clip_plane_enabled = new bool[_max_clip_planes];
    _cur_clip_plane_enabled = new bool[_max_clip_planes];
    for (i = 0; i < _max_clip_planes; i++) {
        _available_clip_plane_ids[i] = NULL;
        _clip_plane_enabled[i] = false;
    }

    // initial clip rect
    SetRect(&clip_rect, 0,0,0,0);     // no clip rect set

    // Make sure the GL state matches all of our initial attribute
    // states.
    PT(DepthTestTransition) dta = new DepthTestTransition;
    PT(DepthWriteTransition) dwa = new DepthWriteTransition;
    PT(CullFaceTransition) cfa = new CullFaceTransition;
    PT(LightTransition) la = new LightTransition;
    PT(TextureTransition) ta = new TextureTransition;

    dta->issue(this);
    dwa->issue(this);
    cfa->issue(this);
    la->issue(this);

    // must do SetTSS here because redundant states are filtered out by our code based on current values above, so
    // initial conditions must be correct

    _CurTexBlendMode = TextureApplyProperty::M_modulate;
    SetTextureBlendMode(_CurTexBlendMode,FALSE);
    _d3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);  // disables texturing

    // Init more Texture State
    _CurTexMagFilter=_CurTexMinFilter=Texture::FT_nearest;
    _CurTexWrapModeU=_CurTexWrapModeV=Texture::WM_clamp;
    _CurTexAnisoDegree=1;

    // this code must match apply_texture() code for states above
    // so DX TSS renderstate matches dxgsg state

    _d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
    _d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_POINT);
    _d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
    _d3dDevice->SetTextureStageState(0, D3DTSS_MAXANISOTROPY,_CurTexAnisoDegree);
    _d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU,get_texture_wrap_mode(_CurTexWrapModeU));
    _d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV,get_texture_wrap_mode(_CurTexWrapModeV));

    ta->issue(this); // no curtextcontext, this does nothing.  dx should already be properly inited above anyway

#ifdef _DEBUG
    if ((_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS) &&
        (dx_global_miplevel_bias!=0.0f)) {
        _d3dDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&dx_global_miplevel_bias)) );
    }
#endif

    _d3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, _CurShadeMode);

    if(dx_full_screen_antialiasing) {
      if(_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT) {
        _d3dDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS,D3DANTIALIAS_SORTINDEPENDENT);
        if(dxgsg_cat.is_debug()) 
            dxgsg_cat.debug() << "enabling full-screen anti-aliasing\n";
      } else {
        if(dxgsg_cat.is_debug()) 
            dxgsg_cat.debug() << "device doesnt support full-screen anti-aliasing\n";
      }
    }

#ifndef NDEBUG
    if(dx_force_backface_culling!=0) {
      if((dx_force_backface_culling > 0) &&
         (dx_force_backface_culling < D3DCULL_FORCE_DWORD)) {
             _d3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, dx_force_backface_culling);
      } else {
          dx_force_backface_culling=0;
          if(dxgsg_cat.is_debug()) 
              dxgsg_cat.debug() << "error, invalid value for dx-force-backface-culling\n";
      }
    }
#endif

}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
clear(const RenderBuffer &buffer) {
    DO_PSTATS_STUFF(PStatTimer timer(_win->_clear_pcollector));

    nassertv(buffer._gsg == this);
    int buffer_type = buffer._buffer_type;

    int flags = 0;

    if (buffer_type & RenderBuffer::T_depth)
        flags |=  D3DCLEAR_ZBUFFER;
    if (buffer_type & RenderBuffer::T_back)       //set appropriate flags
        flags |=  D3DCLEAR_TARGET;
    if (buffer_type & RenderBuffer::T_stencil)
        flags |=  D3DCLEAR_STENCIL;

    D3DCOLOR  clear_colr = Colorf_to_D3DCOLOR(_color_clear_value);

    HRESULT  hr = _d3dDevice->Clear(0, NULL, flags, clear_colr,
                                    (D3DVALUE) _depth_clear_value, (DWORD)_stencil_clear_value);
    if (hr != DD_OK)
        dxgsg_cat.error() << "clear_buffer failed:  Clear returned " << ConvD3DErrorToString(hr) << endl;
    /*  The following line will cause the background to always clear to a medium red
    _color_clear_value[0] = .5;
    /*  The following lines will cause the background color to cycle from black to red.
    _color_clear_value[0] += .001;
     if (_color_clear_value[0] > 1.0f) _color_clear_value[0] = 0.0f;
     */
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
clear(const RenderBuffer &buffer, const DisplayRegion *region) {
    DisplayRegionStack old_dr = push_display_region(region);
    prepare_display_region();
    clear(buffer);
    pop_display_region(old_dr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_light
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian::
enable_light(int light, bool val) {
    if (_light_enabled[light] != val) {
        _light_enabled[light] = val;
        HRESULT res = _d3dDevice->LightEnable( light, val  );

#ifdef GSG_VERBOSE
        dxgsg_cat.debug()
        << "LightEnable(" << light << "=" << val << ")" << endl;
#endif

        return(res == DD_OK);
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::prepare_display_region
//       Access: Public, Virtual
//  Description: Prepare a display region for rendering (set up
//       scissor region and viewport)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
prepare_display_region() {
    if (_current_display_region == (DisplayRegion*)0L) {
        dxgsg_cat.error()
        << "Invalid NULL display region in prepare_display_region()\n";
    } else if (_current_display_region != _actual_display_region) {
        _actual_display_region = _current_display_region;

#ifndef NO_MULTIPLE_DISPLAY_REGIONS
    int l, b, w, h;
    _actual_display_region->get_region_pixels(l, b, w, h);
    GLint x = GLint(l);
    GLint y = GLint(b);
    GLsizei width = GLsizei(w);
    GLsizei height = GLsizei(h);
#ifdef WBD_GL_MODE
//    call_glScissor( x, y, width, height );
//    call_glViewport( x, y, width, height );
#else
    if ( _scissor_x != x || _scissor_y != y ||
            _scissor_width != width || _scissor_height != height )
        {
        _scissor_x = x; _scissor_y = y;
        _scissor_width = width; _scissor_height = height;
        RECT cliprect;
        SetRect(&cliprect, x, y, x+width, y+height );
        set_clipper(cliprect);
        }
#endif   //WBD_GL_MODE
#endif
    }
}

#ifndef NO_MULTIPLE_DISPLAY_REGIONS
////////////////////////////////////////////////////////////////////
//     Function: set_clipper
//       Access:
//  Description: Useless in DX at the present time
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::set_clipper(RECT cliprect) {

    LPDIRECTDRAWCLIPPER Clipper;
    HRESULT result;

    // For windowed mode, the clip region is associated with the window,
    // and DirectX does not allow you to create clip regions.
    if (dx_full_screen) return;

    /* The cliprect we receive is normalized so that (0,0) means the upper left of
       the client portion of the window.
        At least, I think that's true, and the following code assumes that.
        So we must adjust the clip region by offsetting it to the origin of the
        view rectangle.
    */
    clip_rect = cliprect;       // store the normalized clip rect
    cliprect.left += _view_rect.left;
    cliprect.right += _view_rect.left;
    cliprect.top += _view_rect.top;
    cliprect.bottom += _view_rect.top;
    RGNDATA *rgn_data = (RGNDATA *)malloc(sizeof(RGNDATAHEADER) + sizeof(RECT));
    HRGN hrgn = CreateRectRgn(cliprect.left, cliprect.top, cliprect.right, cliprect.bottom);
    GetRegionData(hrgn, sizeof(RGNDATAHEADER) + sizeof(RECT), rgn_data);

    if (_pri->GetClipper(&Clipper) != DD_OK) {
        result = _pDD->CreateClipper(0, &Clipper, NULL);
        result = Clipper->SetClipList(rgn_data, 0);
        result = _pri->SetClipper(Clipper);
    } else {
        result = Clipper->SetClipList(rgn_data, 0 );
        if (result == DDERR_CLIPPERISUSINGHWND) {
            result = _pri->SetClipper(NULL);
            result = _pDD->CreateClipper(0, &Clipper, NULL);
            result = Clipper->SetClipList(rgn_data, 0 ) ;
            result = _pri->SetClipper(Clipper);
        }
    }
    free(rgn_data);
    DeleteObject(hrgn);
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::render_frame
//       Access: Public, Virtual
//  Description: Renders an entire frame, including all display
//               regions within the frame, and includes any necessary
//               pre- and post-processing like swapping buffers.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
render_frame() {
  if (!_dx_ready) 
    return;

  _win->begin_frame();
  _lighting_enabled_this_frame = false;

#ifdef DO_PSTATS
  // For Pstats to track our current texture memory usage, we have to
  // reset the set of current textures each frame.
  init_frame_pstats();

  // But since we don't get sent a new issue_texture() unless our
  // texture state has changed, we have to be sure to clear the
  // current texture state now.  A bit unfortunate, but probably not
  // measurably expensive.
  NodeTransitions state;
  state.set_transition(new TextureTransition);
  modify_state(state);
#endif

  HRESULT hr = _d3dDevice->BeginScene();

  if(FAILED(hr)) {
    if((hr==DDERR_SURFACELOST)||(hr==DDERR_SURFACEBUSY)) {
          if(dxgsg_cat.is_debug())
              dxgsg_cat.debug() << "BeginScene returns " << ConvD3DErrorToString(hr) << endl;

          CheckCooperativeLevel();
    } else {
        dxgsg_cat.error() << "BeginScene failed, unhandled error hr == " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }
    return;
  }

/* _d3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matIdentity); */

#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "begin frame --------------------------------------------" << endl;
#endif

    _decal_level = 0;

    if (_clear_buffer_type != 0) {
      // First, clear the entire window.
      #ifndef NO_MULTIPLE_DISPLAY_REGIONS
        PT(DisplayRegion) win_dr = _win->make_scratch_display_region(_win->get_width(), _win->get_height());
        clear(get_render_buffer(_clear_buffer_type), win_dr);
      #else
        clear(get_render_buffer(_clear_buffer_type));     
      #endif
    }

#if 0
    int max_channel_index = _win->get_max_channel_index();
    for (int c = 0; c < max_channel_index; c++) {
        if (_win->is_channel_defined(c)) {
#endif

    // only 1 channel on dx currently
    assert(_win->get_max_channel_index()==1);
    assert(_win->is_channel_defined(0));

            if(_panda_gfx_channel->is_active()) {
                // Now render each of our layers in order.

                int num_layers = _panda_gfx_channel->get_num_layers();
                for (int l = 0; l < num_layers; l++) {
                    GraphicsLayer *layer = _panda_gfx_channel->get_layer(l);
                    if (layer->is_active()) {
                        int num_drs = layer->get_num_drs();
                        for (int d = 0; d < num_drs; d++) {
                            DisplayRegion *dr = layer->get_dr(d);
                            Camera *cam = dr->get_camera();

                            // For each display region, render from the camera's view.
                            if (dr->is_active() && cam != (Camera *)NULL &&
                                cam->is_active() && cam->get_scene() != (Node *)NULL) {
                                DisplayRegionStack old_dr = push_display_region(dr);
                                prepare_display_region();
                                render_scene(cam->get_scene(), cam);
                                pop_display_region(old_dr);
                            }
                        }
                    }       //      if (layer->is_active())
                }
            }       //      if (chan->is_active())
#if 0
        }
    }   //  for (int c = 0; c < max_channel_index; c++)
#endif

    // Now we're done with the frame processing.  Clean up.

  hr = _d3dDevice->EndScene();  // FPS meter drawing MUST occur after EndScene, since it uses GDI

  if (_lighting_enabled_this_frame) {
        // Let's turn off all the lights we had on, and clear the light
        // cache--to force the lights to be reissued next frame, in case
        // their parameters or positions have changed between frames.

        for (int i = 0; i < _max_lights; i++) {
            enable_light(i, false);
            _available_light_ids[i] = NULL;
        }

        // Also force the lighting state to unlit, so that issue_light()
        // will be guaranteed to be called next frame even if we have the
        // same set of light pointers we had this frame.
        NodeTransitions state;
        state.set_transition(new LightTransition);
        modify_state(state);

        // All this work to undo the lighting state each frame doesn't seem
        // ideal--there may be a better way.  Maybe if the lights were just
        // more aware of whether their parameters or positions have changed
        // at all?
   }

  if(FAILED(hr)) {
    if((hr==DDERR_SURFACELOST)||(hr==DDERR_SURFACEBUSY)) {
          if(dxgsg_cat.is_debug())
              dxgsg_cat.debug() << "EndScene returns " << ConvD3DErrorToString(hr) << endl;

          CheckCooperativeLevel();
    } else {
       dxgsg_cat.error() << "EndScene failed, unhandled error hr == " << ConvD3DErrorToString(hr) << endl;
       exit(1);
    }
    return;
  }

   if(dx_show_fps_meter) {
         DO_PSTATS_STUFF(PStatTimer timer(_win->_show_fps_pcollector));

         DWORD now = timeGetTime();  // this is win32 fn

         float time_delta = (now - _start_time) * 0.001f;

         if(time_delta > dx_fps_meter_update_interval) {
             // didnt use global clock object, it wasnt working properly when I tried,
             // its probably slower due to cache faults, and I can easily track all the
             // info I need in dxgsg
             DWORD num_frames = _cur_frame_count - _start_frame_count;

             _current_fps = num_frames / time_delta;
             _start_time = now;
             _start_frame_count = _cur_frame_count;
         }

         HDC hDC;

         if(SUCCEEDED(_back->GetDC(&hDC))) {
             char fps_msg[15];
             sprintf(fps_msg, "%7.02f fps", _current_fps);
             SetTextColor(hDC, RGB(255,255,0) );
             SetBkMode(hDC, TRANSPARENT );

             // 75,8 seem to work well w/default font
             TextOutA(hDC, (_view_rect.right-_view_rect.left)-75, 8, fps_msg, strlen(fps_msg));
             _back->ReleaseDC(hDC);
         }

         _cur_frame_count++;  // only used by fps meter right now
    }

    _win->end_frame();

    show_frame();

#ifdef COUNT_DRAWPRIMS
    {
        #define FRAMES_PER_DPINFO 90
        static DWORD LastDPInfoFrame=0;
        static DWORD LastTickCount=0;

        if (_cur_frame_count-LastDPInfoFrame > FRAMES_PER_DPINFO) {
            DWORD CurTickCount=GetTickCount();
            float delta_secs=(CurTickCount-LastTickCount)/1000.0f;

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

            dxgsg_cat.debug() << "==================================="
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

#if defined(DO_PSTATS)||defined(PRINT_TEXSTATS)
#ifndef PRINT_TEXSTATS
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

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "end frame ----------------------------------------------" << endl;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::report_texmgr_stats
//       Access: Protected
//  Description: Reports the DX texture manager's activity to PStats.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
report_texmgr_stats() {

#if defined(DO_PSTATS)||defined(PRINT_TEXSTATS)

  HRESULT hr;
  DWORD dwTexTotal,dwTexFree,dwVidTotal,dwVidFree;

#ifndef PRINT_TEXSTATS
  if (_total_texmem_pcollector.is_active())
#endif
  {
      DDSCAPS2 ddsCaps;
    
      ZeroMemory(&ddsCaps,sizeof(ddsCaps));
    
      ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
      if(FAILED(  hr = _pDD->GetAvailableVidMem(&ddsCaps,&dwVidTotal,&dwVidFree))) {
            dxgsg_cat.debug() << "report_texmgr GetAvailableVidMem for VIDMEM failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
      }
    
      ddsCaps.dwCaps = DDSCAPS_TEXTURE;
      if(FAILED(  hr = _pDD->GetAvailableVidMem(&ddsCaps,&dwTexTotal,&dwTexFree))) {
            dxgsg_cat.debug() << "report_texmgr GetAvailableVidMem for TEXTURE failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
      }
  }

  D3DDEVINFO_TEXTUREMANAGER tminfo;
  ZeroMemory(&tminfo,sizeof(D3DDEVINFO_TEXTUREMANAGER));

  if(!bTexStatsRetrievalImpossible) {
      hr = _d3dDevice->GetInfo(D3DDEVINFOID_TEXTUREMANAGER,&tminfo,sizeof(D3DDEVINFO_TEXTUREMANAGER));
      if (hr!=D3D_OK) {
          if (hr==S_FALSE) {
              static int PrintedMsg=2;
              if(PrintedMsg>0) {
                  if(dxgsg_cat.is_debug())
                    dxgsg_cat.debug() << " ************ texstats GetInfo() requires debug DX DLLs to be installed!!  ***********\n";
                  ZeroMemory(&tminfo,sizeof(D3DDEVINFO_TEXTUREMANAGER));
                  bTexStatsRetrievalImpossible=true;
              }
          } else {
              dxgsg_cat.error() << "d3ddev->GetInfo(TEXTUREMANAGER) failed to get tex stats: result = " << ConvD3DErrorToString(hr) << endl;
              return;
          }
      }
  }

#ifdef PRINT_TEXSTATS
    char tmpstr1[50],tmpstr2[50],tmpstr3[50],tmpstr4[50];
    sprintf(tmpstr1,"%.4g",dwVidTotal/1000000.0);
    sprintf(tmpstr2,"%.4g",dwVidFree/1000000.0);
    sprintf(tmpstr3,"%.4g",dwTexTotal/1000000.0);
    sprintf(tmpstr4,"%.4g",dwTexFree/1000000.0);
    dxgsg_cat.debug() << "\nAvailableVidMem for RenderSurfs: (megs) total: " << tmpstr1 << "  free: " << tmpstr2
                      << "\nAvailableVidMem for Textures:    (megs) total: " << tmpstr3 << "  free: " << tmpstr4 << endl;

   if(!bTexStatsRetrievalImpossible) {
            dxgsg_cat.spam()
                << "\n bThrashing:\t" << tminfo.bThrashing
                << "\n NumEvicts:\t" << tminfo.dwNumEvicts
                << "\n NumVidCreates:\t" << tminfo.dwNumVidCreates
                << "\n NumTexturesUsed:\t" << tminfo.dwNumTexturesUsed
                << "\n NumUsedTexInVid:\t" << tminfo.dwNumUsedTexInVid
                << "\n WorkingSet:\t" << tminfo.dwWorkingSet
                << "\n WorkingSetBytes:\t" << tminfo.dwWorkingSetBytes
                << "\n TotalManaged:\t" << tminfo.dwTotalManaged
                << "\n TotalBytes:\t" << tminfo.dwTotalBytes
                << "\n LastPri:\t" << tminfo.dwLastPri << endl;

            D3DDEVINFO_TEXTURING texappinfo;
            ZeroMemory(&texappinfo,sizeof(D3DDEVINFO_TEXTURING));
            hr = _d3dDevice->GetInfo(D3DDEVINFOID_TEXTURING,&texappinfo,sizeof(D3DDEVINFO_TEXTURING));
            if (hr!=D3D_OK) {
                dxgsg_cat.error() << "GetInfo(TEXTURING) failed : result = " << ConvD3DErrorToString(hr) << endl;
                return;
            } else {
                dxgsg_cat.spam()
                << "\n NumTexLoads:\t" << texappinfo.dwNumLoads
                << "\n ApproxBytesLoaded:\t" << texappinfo.dwApproxBytesLoaded
                << "\n NumPreLoads:\t" << texappinfo.dwNumPreLoads
                << "\n NumSet:\t" << texappinfo.dwNumSet
                << "\n NumCreates:\t" << texappinfo.dwNumCreates
                << "\n NumDestroys:\t" << texappinfo.dwNumDestroys
                << "\n NumSetPriorities:\t" << texappinfo.dwNumSetPriorities
                << "\n NumSetLODs:\t" << texappinfo.dwNumSetLODs
                << "\n NumLocks:\t" << texappinfo.dwNumLocks
                << "\n NumGetDCs:\t" << texappinfo.dwNumGetDCs << endl;
            }
    }
#endif

#ifdef DO_PSTATS
  // Tell PStats about the state of the texture memory.

  if (_texmgrmem_total_pcollector.is_active()) {
      // report zero if no debug dlls, to signal this info is invalid
      _texmgrmem_total_pcollector.set_level(tminfo.dwTotalBytes);
      _texmgrmem_resident_pcollector.set_level(tminfo.dwWorkingSetBytes);
  }
    
  if (_total_texmem_pcollector.is_active()) {
    _total_texmem_pcollector.set_level(dwTexTotal);
    _used_texmem_pcollector.set_level(dwTexTotal - dwTexFree);
  }
#endif

#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::render_scene
//       Access: Public, Virtual
//  Description: Renders an entire scene, from the root node of the
//               scene graph, as seen from a particular ProjectionNode
//               and with a given initial state.  This initial state
//               may be modified during rendering.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
render_scene(Node *root, ProjectionNode *projnode) {
#ifdef GSG_VERBOSE
    _pass_number = 0;
    dxgsg_cat.debug()
    << "begin scene - - - - - - - - - - - - - - - - - - - - - - - - -"
    << endl;
#endif
    _current_root_node = root;

    render_subgraph(_render_traverser, root, projnode,
                    AllTransitionsWrapper());

#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "done scene  - - - - - - - - - - - - - - - - - - - - - - - - -"
    << endl;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::render_subgraph
//       Access: Public, Virtual
//  Description: Renders a subgraph of the scene graph as seen from a
//               given projection node, and with a particular initial
//               state.  This state may be modified by the render
//               process.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
render_subgraph(RenderTraverser *traverser,
                Node *subgraph, ProjectionNode *projnode,
                const AllTransitionsWrapper &net_trans) {
    ProjectionNode *old_projection_node = _current_projection_node;
    _current_projection_node = projnode;
    LMatrix4f old_projection_mat = _current_projection_mat;

    // d3d is left-handed coord system
    LMatrix4f projection_mat =
    projnode->get_projection()->get_projection_mat(CS_yup_left);

#if 0
    dxgsg_cat.spam() << "cur projection matrix: " << projection_mat <<"\n";
#endif

    _current_projection_mat = projection_mat;
    _projection_mat_stack_count++;

#ifdef _DEBUG
   {
      static bool bPrintedMsg=false;

      if((!bPrintedMsg) && !IS_NEARLY_EQUAL(projection_mat(2,3),1.0f)) {
         bPrintedMsg=true;
         dxgsg_cat.debug() << "non w-compliant render_subgraph projection matrix [2][3] should be 1.0, instead is: " << projection_mat(2,3) << endl;
         dxgsg_cat.debug() << "cur projection matrix: " << projection_mat << endl;
      }

      // note: a projection matrix that does not have a [3][4] value of 1.0f is
      //       not w-compliant and could cause problems with fog

    }
#endif

    HRESULT hr;

    // We load the projection matrix directly.
    hr = _d3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,
                                 (LPD3DMATRIX) _current_projection_mat.get_data());

    // We infer the modelview matrix by doing a wrt on the projection
    // node.
    LMatrix4f modelview_mat;
    get_rel_mat(subgraph, _current_projection_node, modelview_mat);  //needs reversal from glgsg, probably due D3D LH coordsys
//  get_rel_mat(_current_projection_node, subgraph, modelview_mat);

    if (_coordinate_system != CS_yup_left) {
        // Now we build the coordinate system conversion into the
        // modelview matrix (as described in the paragraph above).
        modelview_mat = modelview_mat *
                        LMatrix4f::convert_mat(_coordinate_system, CS_yup_left);
    }

    // The modelview matrix will be loaded as each geometry is
    // encountered.  So we set the supplied modelview matrix as an
    // initial value instead of loading it now.
    AllTransitionsWrapper sub_trans = net_trans;
    sub_trans.set_transition(new TransformTransition(modelview_mat));

    render_subgraph(traverser, subgraph, sub_trans);

    _current_projection_node = old_projection_node;
    _current_projection_mat = old_projection_mat;
    _projection_mat_stack_count--;


    // We must now restore the projection matrix from before.  We could
    // do a push/pop matrix if we were using D3DX
    if (_projection_mat_stack_count > 0)
        hr =_d3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,
                                    (LPD3DMATRIX) _current_projection_mat.get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::render_subgraph
//       Access: Public, Virtual
//  Description: Renders a subgraph of the scene graph as seen from the
//               current projection node, and with a particular
//               initial state.  This state may be modified during the
//               render process.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
render_subgraph(RenderTraverser *traverser, Node *subgraph,
                const AllTransitionsWrapper &net_trans) {
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "begin subgraph (pass " << ++_pass_number
    << ") - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
#endif

    nassertv(traverser != (RenderTraverser *)NULL);
    traverser->traverse(subgraph, net_trans);

#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "end subgraph (pass " << _pass_number
    << ") - - - - - - - - - - - - - - - - - - - - - - - - -"
    << endl;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::add_to_FVFBuf
//       Access: Private
//  Description: This adds data to the flexible vertex format
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
add_to_FVFBuf(void *data,  size_t bytes) {
    memcpy(_pCurFvfBufPtr, data, bytes);
    _pCurFvfBufPtr += bytes;
}

// generates slightly fewer instrs
#define add_DWORD_to_FVFBuf(data) { *((DWORD *)_pCurFvfBufPtr) = (DWORD) data;  _pCurFvfBufPtr += sizeof(DWORD);}

typedef enum {
    FlatVerts,IndexedVerts,MixedFmtVerts
} GeomVertFormat;

#if defined(_DEBUG) || defined(COUNT_DRAWPRIMS)
typedef enum {DrawPrim,DrawIndexedPrim,DrawPrimStrided} DP_Type;
static const char *DP_Type_Strs[3] = {"DrawPrimitive","DrawIndexedPrimitive","DrawPrimitiveStrided"};

void INLINE TestDrawPrimFailure(DP_Type dptype,HRESULT hr,LPDIRECTDRAW7 pDD,DWORD nVerts,DWORD nTris) {
        if(FAILED(hr)) {
            // loss of exclusive mode is not a real DrawPrim problem, ignore it
            HRESULT testcooplvl_hr = pDD->TestCooperativeLevel();
            if((testcooplvl_hr != DDERR_NOEXCLUSIVEMODE)||(testcooplvl_hr != DDERR_EXCLUSIVEMODEALREADYSET)) {
                dxgsg_cat.fatal() << DP_Type_Strs[dptype] << "() failed: result = " << ConvD3DErrorToString(hr) << endl;
                exit(1);
            }
        }

        CountDPs(nVerts,nTris);
}
#else
#define TestDrawPrimFailure(a,b,c,nVerts,nTris) CountDPs(nVerts,nTris);
#endif

#define COPYVERTDATA_2_VERTEXBUFFER(PrimType,NumVertices) {                                     \
            DWORD numVertBytes=_pCurFvfBufPtr-_pFvfBufBasePtr;                                  \
            memcpy(_pCurrentGeomContext->_pEndofVertData,_pFvfBufBasePtr,numVertBytes);         \
            DPInfo dpInfo;                                                                      \
            dpInfo.nVerts=NumVertices;                                                          \
            dpInfo.primtype=PrimType;                                                           \
            _pCurrentGeomContext->_PrimInfo.push_back(dpInfo);                                  \
            _pCurrentGeomContext->_num_verts+=dpInfo.nVerts;                                    \
            _pCurrentGeomContext->_pEndofVertData+=numVertBytes; }
            

INLINE void DXGraphicsStateGuardian::
transform_color(Colorf &InColor,D3DCOLOR &OutRGBAColor) {
  // To be truly general, we really need a 5x5 matrix to transform a
  // 4-component color.  Rather than messing with that, we instead
  // treat the color as a 3-component RGB, which can be transformed by
  // the ordinary 4x4 matrix, and a separate alpha value, which can be
  // scaled and offsetted.

    LPoint4f temp_pnt(InColor[0], InColor[1], InColor[2], 1.0f);
    Colorf out_color = temp_pnt * _current_color_mat;  // maybe expand this out for efficiency
    out_color[3] = (InColor[3] * _current_alpha_scale) + _current_alpha_offset;
    OutRGBAColor = Colorf_to_D3DCOLOR(out_color);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_prim_setup
//       Access: Private
//  Description: This adds data to the flexible vertex format
////////////////////////////////////////////////////////////////////
size_t DXGraphicsStateGuardian::
draw_prim_setup(const Geom *geom) {
    //  Set the flags for the flexible vertex format and compute the bytes
    //  required to store a single vertex.

    #ifdef _DEBUG
      assert(geom->get_binding(G_COORD) != G_OFF);
    #endif

#define GET_NEXT_VERTEX() { p_vertex = geom->get_next_vertex(vi); }
#define GET_NEXT_NORMAL() { p_normal = geom->get_next_normal(ni); }
#define GET_NEXT_TEXCOORD() { p_texcoord = geom->get_next_texcoord(ti); }
#define GET_NEXT_COLOR() {                                                           \
    if(_color_transform_enabled || _alpha_transform_enabled) {                       \
        Colorf tempcolor = geom->get_next_color(ci);                                 \
        transform_color(tempcolor,_curD3Dcolor);                                           \
    } else {                                                                         \
        p_color = geom->get_next_color(ci);                                          \
        _curD3Dcolor = Colorf_to_D3DCOLOR(p_color);                                        \
    }}

////////

   vi = geom->make_vertex_iterator();
   _curFVFflags = D3DFVF_XYZ;
   size_t vertex_size = sizeof(D3DVALUE) * 3;

   GeomBindType ColorBinding=geom->get_binding(G_COLOR);
   bool bDoColor=(ColorBinding != G_OFF);

   if (_enable_all_color && (bDoColor || _issued_color_enabled)) {
        ci = geom->make_color_iterator();
        _curFVFflags |= D3DFVF_DIFFUSE;
        vertex_size += sizeof(D3DCOLOR);

        if (_issued_color_enabled) {
            _curD3Dcolor = _issued_color_D3DCOLOR;  // set primitive color if there is one.

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
        _curFVFflags |= D3DFVF_NORMAL;
        vertex_size += sizeof(D3DVALUE) * 3;

        if (geom->get_binding(G_NORMAL) == G_OVERALL)
            p_normal = geom->get_next_normal(ni);    // set overall normal if there is one
   }

   if (geom->get_binding(G_TEXCOORD) != G_OFF) {
        ti = geom->make_texcoord_iterator();
        _curFVFflags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
        vertex_size += sizeof(float) * 2;
   }

    // If we have per-vertex colors or normals, we need smooth shading.
    // Otherwise we want flat shading for performance reasons.

   // Note on fogging:
   // the fogging expression should really be (_doFogType==PerVertexFog)
   // since GOURAUD shading should not be required for PerPixel fog, but the
   // problem is some cards (Riva128,Matrox G200) emulate pixel fog with table fog
   // but dont force the shading mode to gouraud internally, so you end up with flat-shaded fog colors
   // (note, TNT does the right thing tho).  So I guess we must do gouraud shading for all fog rendering for now

   if ((_perVertex & (PER_COLOR | (wants_normals() ? PER_NORMAL : 0))) || (_doFogType!=None)) 
        set_shademode(D3DSHADE_GOURAUD);
   else set_shademode(D3DSHADE_FLAT);

   return vertex_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::wants_colors
//       Access: Public, Virtual
//  Description: Returns true if the GSG should issue geometry color
//               commands, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DXGraphicsStateGuardian::
wants_colors() const {
    // If we have scene graph color enabled, return false to indicate we
    // shouldn't bother issuing geometry color commands.

    const ColorTransition *catt;
    if (!get_attribute_into(catt, this)) {
        // No scene graph color at all.
        return true;
    }

    // We should issue geometry colors only if the scene graph color is off.
    if (catt->is_off() || (!catt->is_real()))
        return true;

    return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_prim_inner_loop
//       Access: Private
//  Description: This adds data to the flexible vertex format with a check
//               for component normals and color
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_prim_inner_loop(int nVerts, const Geom *geom, DWORD perFlags) {
    perFlags &= ~PER_COORD;  // should always be set anyway

    for(;nVerts > 0;nVerts--) {

        GET_NEXT_VERTEX();   // coord info will always be _perVertex

        if(perFlags==0xC) {
            // break out the common case first
            GET_NEXT_TEXCOORD();
            GET_NEXT_COLOR();
        } else {
            switch (perFlags) {
                case 0x4:
                    GET_NEXT_COLOR();
                    break;
                case 0x6:
                    GET_NEXT_COLOR();
                case 0x2:
                    GET_NEXT_NORMAL();
                    break;
                case 0xE:
                    GET_NEXT_COLOR();
                case 0xA:
                    GET_NEXT_NORMAL();
                case 0x8:
                    GET_NEXT_TEXCOORD();
                    break;
            }
        }

        add_to_FVFBuf((void *)&p_vertex, sizeof(D3DVECTOR));

        if (_curFVFflags & D3DFVF_NORMAL)
            add_to_FVFBuf((void *)&p_normal, sizeof(D3DVECTOR));
        if (_curFVFflags & D3DFVF_DIFFUSE)
            add_DWORD_to_FVFBuf(_curD3Dcolor);
        if (_curFVFflags & D3DFVF_TEXCOUNT_MASK)
            add_to_FVFBuf((void *)&p_texcoord, sizeof(TexCoordf));
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_point
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_point(GeomPoint *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "draw_point()" << endl;
#endif
  
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));
  
    // The DX Way

    int nPrims = geom->get_num_prims();

    if (nPrims==0) {
        dxgsg_cat.warning() << "draw_point() called with ZERO vertices!!" << endl;
        return;
    }

#ifdef _DEBUG
    static BOOL bPrintedMsg=FALSE;

    if (!bPrintedMsg && (geom->get_size()!=1.0f)) {
        bPrintedMsg=TRUE;
        dxgsg_cat.warning() << "D3D does not support drawing points of non-unit size, setting point size to 1.0f!\n";
    }
#endif

    nassertv(nPrims < D3DMAXNUMVERTICES );

    PTA_Vertexf coords;
    PTA_Normalf norms;
    PTA_Colorf colors;
    PTA_TexCoordf texcoords;
    GeomBindType bind;
    PTA_ushort vindexes,nindexes,tindexes,cindexes;

    geom->get_coords(coords,bind,vindexes);
    geom->get_normals(norms,bind,nindexes);
    geom->get_colors(colors,bind,cindexes);
    geom->get_texcoords(texcoords,bind,tindexes);

    GeomVertFormat GeomVrtFmt=FlatVerts;

    // first determine if we're indexed or non-indexed

    if ((vindexes!=NULL)&&(cindexes!=NULL)&&(tindexes!=NULL)&&(nindexes!=NULL)) {
        GeomVrtFmt=IndexedVerts;
    } else if (!((vindexes==NULL)&&(cindexes==NULL)&&(tindexes==NULL)&&(nindexes==NULL)))
        GeomVrtFmt=MixedFmtVerts;

#ifdef DONT_USE_DRAWPRIMSTRIDED
    GeomVrtFmt=MixedFmtVerts;
#else
    if(_bDrawPrimDoSetupVertexBuffer) {
      GeomVrtFmt=MixedFmtVerts;
    }
#endif

    // for Indexed Prims and mixed indexed/non-indexed prims, we will use old pipeline for now
    // need to add code to handle fully indexed mode (and handle cases with index arrays of different lengths,
    // values (may only be possible to handle certain cases without reverting to old pipeline)
    if (GeomVrtFmt!=FlatVerts) {

        _perVertex = PER_COORD;
        _perPrim = 0;
        if (geom->get_binding(G_COORD) == G_PER_VERTEX)  _perVertex |= PER_COORD;
        if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) _perVertex |= PER_NORMAL;
        if (geom->get_binding(G_COLOR) == G_PER_VERTEX) _perVertex |= PER_COLOR;

        size_t vertex_size = draw_prim_setup(geom);

        nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
        nassertv(nPrims * vertex_size < VERT_BUFFER_SIZE);
        _pCurFvfBufPtr = _pFvfBufBasePtr;          // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

        // iterate through the point
        draw_prim_inner_loop(nPrims, geom, _perVertex | _perPrim);

        if(!_bDrawPrimDoSetupVertexBuffer) {
           HRESULT hr = _d3dDevice->DrawPrimitive(D3DPT_POINTLIST, _curFVFflags, _pFvfBufBasePtr, nPrims, NULL);   
           TestDrawPrimFailure(DrawPrim,hr,_pDD,nPrims,0);
        } else {
            COPYVERTDATA_2_VERTEXBUFFER(D3DPT_POINTLIST,nPrims);
        }
    } else {  // setup for strided

        size_t vertex_size = draw_prim_setup(geom);

        // new code only handles non-indexed pointlists (is this enough?)
        nassertv((vindexes==NULL)&&(cindexes==NULL)&&(tindexes==NULL)&&(nindexes==NULL));

        D3DDRAWPRIMITIVESTRIDEDDATA dps_data;
        memset(&dps_data,0,sizeof(D3DDRAWPRIMITIVESTRIDEDDATA));

        dps_data.position.lpvData = (VOID*)coords;
        dps_data.position.dwStride = sizeof(D3DVECTOR);

        if (_curFVFflags & D3DFVF_NORMAL) {
            dps_data.normal.lpvData = (VOID*)norms;
            dps_data.normal.dwStride = sizeof(D3DVECTOR);
        }

        if (_curFVFflags & D3DFVF_DIFFUSE) {
            _pCurFvfBufPtr=_pFvfBufBasePtr;

            dps_data.diffuse.lpvData = (VOID*)_pFvfBufBasePtr;
            dps_data.diffuse.dwStride = sizeof(D3DCOLOR);

            // Geom nodes store floats for colors, drawprim requires ARGB dwords
            // BUGBUG: eventually this hack every-frame all-colors conversion needs
            // to be done only once as part of a vertex buffer

            if(_color_transform_enabled || _alpha_transform_enabled) {
                for (int i=0;i<nPrims;i++) {
                    D3DCOLOR RGBA_color;
                    transform_color(colors[i],RGBA_color);
                    add_DWORD_to_FVFBuf(RGBA_color);
                }
            } else
             for (int i=0;i<nPrims;i++) {
                Colorf out_color=colors[i];
                add_DWORD_to_FVFBuf(Colorf_to_D3DCOLOR(out_color));
             }
        }

        if (_curFVFflags & D3DFVF_TEXCOUNT_MASK) {
            dps_data.textureCoords[0].lpvData = (VOID*)texcoords;
            dps_data.textureCoords[0].dwStride = sizeof(TexCoordf);
        }

        HRESULT hr = _d3dDevice->DrawPrimitiveStrided(D3DPT_POINTLIST, _curFVFflags, &dps_data, nPrims, NULL);
        TestDrawPrimFailure(DrawPrimStrided,hr,_pDD,nPrims,0);
    }

    _pCurFvfBufPtr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_line(GeomLine* geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "draw_line()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

#ifdef _DEBUG
    static BOOL bPrintedMsg=FALSE;

    // note: need to implement approximation of non-1.0 width lines with quads

    if (!bPrintedMsg && (geom->get_width()!=1.0f)) {
        bPrintedMsg=TRUE;
        dxgsg_cat.warning() << "DX does not support drawing lines with a non-1.0f pixel width, setting width to 1.0f!\n";
    }
#endif

    int nPrims = geom->get_num_prims();

    if (nPrims==0) {
        dxgsg_cat.warning() << "draw_line() called with ZERO vertices!!" << endl;
        return;
    }

    assert(geom->get_binding(G_COORD) == G_PER_VERTEX);
    _perVertex = PER_COORD;

    _perPrim = _perComp = 0;
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

    size_t vertex_size = draw_prim_setup(geom);

    char *_tmp_fvf = NULL;
    nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
//  nassertv(nPrims * 2 * vertex_size < VERT_BUFFER_SIZE);

    if (nPrims * 2 * vertex_size > VERT_BUFFER_SIZE) {
        // bugbug: need cleaner way to handle tmp buffer size overruns (malloc/realloc?)
        _pCurFvfBufPtr = _tmp_fvf = new char[nPrims * 2 * vertex_size];
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

    DWORD vertCount = nPrims<<1;

    if(!_bDrawPrimDoSetupVertexBuffer) {
        if (_tmp_fvf == NULL)
            hr = _d3dDevice->DrawPrimitive(D3DPT_LINELIST, _curFVFflags, _pFvfBufBasePtr, vertCount, NULL);
        else {
            hr = _d3dDevice->DrawPrimitive(D3DPT_LINELIST, _curFVFflags, _tmp_fvf, vertCount, NULL);
            delete [] _tmp_fvf;
        }
        TestDrawPrimFailure(DrawPrim,hr,_pDD,vertCount,0);
    } else {
        COPYVERTDATA_2_VERTEXBUFFER(D3DPT_LINELIST,vertCount);
    }

    _pCurFvfBufPtr = NULL;
}

void DXGraphicsStateGuardian::
draw_linestrip(GeomLinestrip* geom, GeomContext *gc) {

#ifdef _DEBUG
    static BOOL bPrintedMsg=FALSE;

    if (!bPrintedMsg && (geom->get_width()!=1.0f)) {
        bPrintedMsg=TRUE;
        dxgsg_cat.warning() << "DX does not support drawing lines with a non-1.0f pixel width, setting width to 1.0f!\n";
    }
#endif

  draw_linestrip_base(geom,gc,false);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_linestrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_linestrip_base(Geom* geom, GeomContext *gc, bool bConnectEnds) {

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "draw_linestrip()" << endl;
#endif

    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    int nPrims = geom->get_num_prims();
    const int *plen = geom->get_lengths();

    if (nPrims==0) {
        dxgsg_cat.warning() << "draw_linestrip() called with ZERO vertices!!" << endl;
        return;
    }

    assert(geom->get_binding(G_COORD) == G_PER_VERTEX);
    _perVertex = PER_COORD;

    _perPrim = _perComp = 0;
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

    size_t vertex_size = draw_prim_setup(geom);

    for (int i = 0; i < nPrims; i++) {
        if (_perPrim & PER_COLOR) {
            GET_NEXT_COLOR();
        }

        int nVerts;

        if(plen==NULL) {
            nVerts=4;  // we've been called by draw_quad, which has no lengths array
        } else {
            nVerts= *(plen++);
            nassertv(nVerts >= 2);
        }

        nassertv(_pCurFvfBufPtr == NULL);   // make sure the storage pointer is clean.
        nassertv(nVerts * vertex_size < VERT_BUFFER_SIZE);
        _pCurFvfBufPtr = _pFvfBufBasePtr;   // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

        DWORD perFlags = _perVertex | _perComp;

        draw_prim_inner_loop(nVerts, geom, perFlags);

        if(bConnectEnds) {
             // append first vertex to end
             memcpy(_pCurFvfBufPtr,_pFvfBufBasePtr,vertex_size);
             _pCurFvfBufPtr+=vertex_size;
             nVerts++;
        }

        if(!_bDrawPrimDoSetupVertexBuffer) {
            HRESULT hr = _d3dDevice->DrawPrimitive(D3DPT_LINESTRIP, _curFVFflags, _pFvfBufBasePtr, nVerts, NULL);
            TestDrawPrimFailure(DrawPrim,hr,_pDD,nVerts,0);
        } else {
            COPYVERTDATA_2_VERTEXBUFFER(D3DPT_LINESTRIP,nVerts);
        }           

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
//     Function: DXGraphicsStateGuardian::draw_sprite
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
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


    // Note: for DX8, try to use the PointSprite primitive instead of doing all the stuff below

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "draw_sprite()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));

    // get the array traversal set up.
    int nprims = geom->get_num_prims();

    if (nprims==0) {
        return;
    }

    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(nprims));

    Texture *tex = geom->get_texture();
    nassertv(tex != (Texture *) NULL);

    D3DMATRIX OldD3DWorldMatrix;
    _d3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &OldD3DWorldMatrix);

    // ATI sez:  most applications ignore the fact that since alpha blended primitives
    // combine the data in the frame buffer with the data in the current pixel, pixels 
    // can be dithered multiple times and accentuate the dither pattern. This is particularly
    // true in particle systems which rely on the cumulative visual effect of many overlapping
    // alpha blended primitives.

    bool bReEnableDither=_dither_enabled;
    if(bReEnableDither) {
        enable_dither(false);
    }

    _d3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &OldD3DWorldMatrix);

    Geom::VertexIterator vi = geom->make_vertex_iterator();
    Geom::ColorIterator ci = geom->make_color_iterator();

    // save the modelview matrix
    LMatrix4f modelview_mat;

    const TransformTransition *ctatt;
    if (!get_attribute_into(ctatt, this))
        modelview_mat = LMatrix4f::ident_mat();
    else
        modelview_mat = ctatt->get_matrix();

    // get the camera information
    float aspect_ratio;
    aspect_ratio = _actual_display_region->get_camera()->get_aspect();

    // null the world xform, so sprites are orthog to scrn  (but not necessarily camera pnt unless they lie along z-axis)
    _d3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matIdentity);
    // only need to change _WORLD xform, _VIEW xform is Identity

    // precomputation stuff
    float tex_left = geom->get_ll_uv()[0];
    float tex_right = geom->get_ur_uv()[0];
    float tex_bottom = geom->get_ll_uv()[1];
    float tex_top = geom->get_ur_uv()[1];
    
    float half_width = 0.5f * (float) tex->_pbuffer->get_xsize() * fabs(tex_right - tex_left);
    float half_height = 0.5f * (float) tex->_pbuffer->get_ysize() * fabs(tex_top - tex_bottom);
    float scaled_width, scaled_height;

    // set up the texture-rendering state
    NodeTransitions state;

    // this sets up texturing.  Could just set the renderstates directly, but this is a little cleaner
    TextureTransition *ta = new TextureTransition(tex);
    state.set_transition(ta);

    TextureApplyTransition *taa = new TextureApplyTransition(TextureApplyProperty::M_modulate);
    state.set_transition(taa);

    modify_state(state);

    // the user can override alpha sorting if they want
    bool alpha = false;

    if (!geom->get_alpha_disable()) {
        // figure out if alpha's enabled (if not, no reason to sort)
        const TransparencyTransition *ctratt;
        if (get_attribute_into(ctratt, this))
          alpha = (ctratt->get_mode() != TransparencyProperty::M_none);
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
        scaled_height = geom->_y_texel_ratio[0] * half_height * aspect_ratio;
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

    WrappedSprite *SpriteArray = new WrappedSprite[nprims];

    //BUGBUG: could we use _fvfbuf for this to avoid perframe alloc?
    // alternately, alloc once when retained mode becomes available

    if (SpriteArray==NULL) {
        dxgsg_cat.fatal() << "draw_sprite() out of memory!!" << endl;
        return;
    }

    // the state is set, start running the prims

    WrappedSprite *pSpr;

    for (pSpr=SpriteArray,i = 0; i < nprims; i++,pSpr++) {

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
        sorted_sprite_vector.reserve(nprims);   //pre-alloc space for nprims

        for (pSpr=SpriteArray,i = 0; i < nprims; i++,pSpr++) {   // build STL-sortable array
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
    }

    Vertexf ul, ur, ll, lr;

    ////////////////////////////////////////////////////////////////////////////
    // INNER LOOP PART 2 STARTS HERE
    // Now we run through the cameraspace vector and compute the geometry for each
    // tristrip.  This includes scaling as per the ratio arrays, as well as
    // rotating in the z.
    ////////////////////////////////////////////////////////////////////////////

    _curFVFflags = D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) ;
    DWORD vertex_size = sizeof(float) * 2 + sizeof(D3DVALUE) * 3;

    D3DCOLOR CurColor;
    bool bDoColor=TRUE;
    if (color_overall) {
        GET_NEXT_COLOR();
        CurColor = _curD3Dcolor;
        bDoColor = (_curD3Dcolor != ~0);
    }

    if (bDoColor) {
        _curFVFflags |= D3DFVF_DIFFUSE;
        vertex_size+=sizeof(D3DCOLOR);
    }

    #ifdef _DEBUG
     nassertv(_pCurFvfBufPtr == NULL);   // make sure the storage pointer is clean.
     nassertv(nprims * 4 * vertex_size < VERT_BUFFER_SIZE);
     nassertv(nprims * 6 < D3DMAXNUMVERTICES );
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

    for (pSpr=SpriteArray,i = 0; i < nprims; i++,pSpr++) {   // build STL-sortable array

        if (alpha) {
            pSpr = sorted_vec_iter->pSpr;
            sorted_vec_iter++;
        }

        // if not G_OVERALL, calculate the scale factors    //huh??
        if (!x_overall)
            scaled_width = pSpr->_x_ratio * half_width;

        if (!y_overall)
            scaled_height = pSpr->_y_ratio * half_height * aspect_ratio;

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

        add_to_FVFBuf((void *)ll.get_data(), sizeof(D3DVECTOR));
        if (bDoColor) {
            if (!color_overall)  // otherwise its already been set globally
                CurColor = pSpr->_c;
            add_DWORD_to_FVFBuf(CurColor); // only need to cpy color on 1st vert, others are just empty ignored space
        }
        add_to_FVFBuf((void *)TexCrdSets[0], sizeof(float)*2);

        add_to_FVFBuf((void *)lr.get_data(), sizeof(D3DVECTOR));
        if (bDoColor)
            _pCurFvfBufPtr += sizeof(D3DCOLOR);  // flat shading, dont need to write color, just incr ptr
        add_to_FVFBuf((void *)TexCrdSets[1], sizeof(float)*2);

        add_to_FVFBuf((void *)ul.get_data(), sizeof(D3DVECTOR));
        if (bDoColor)
            _pCurFvfBufPtr += sizeof(D3DCOLOR);  // flat shading, dont need to write color, just incr ptr
        add_to_FVFBuf((void *)TexCrdSets[2], sizeof(float)*2);

        add_to_FVFBuf((void *)ur.get_data(), sizeof(D3DVECTOR));
        if (bDoColor)
            add_DWORD_to_FVFBuf(CurColor);
        add_to_FVFBuf((void *)TexCrdSets[3], sizeof(float)*2);

        for (int ii=0;ii<QUADVERTLISTLEN;ii++) {
            _index_buf[CurDPIndexArrLength+ii]=QuadVertIndexList[ii]+CurVertCount;
        }
        CurDPIndexArrLength+=QUADVERTLISTLEN;
        CurVertCount+=4;
    }

    // cant do tristrip/fan since it would require 1 call want to make 1 call for multiple quads which arent connected
    // best we can do is indexed primitive, which sends 2 redundant indices instead of sending 2 redundant full verts
    HRESULT hr = _d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, _curFVFflags, _pFvfBufBasePtr, 4*nprims, _index_buf,QUADVERTLISTLEN*nprims,NULL);
    TestDrawPrimFailure(DrawIndexedPrim,hr,_pDD,QUADVERTLISTLEN*nprims,nprims);

    _pCurFvfBufPtr = NULL;
    delete [] SpriteArray;

    // restore the matrices
    _d3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &OldD3DWorldMatrix);

    if(bReEnableDither)
        enable_dither(true);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_polygon
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_polygon(GeomPolygon *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
   dxgsg_cat.debug() << "draw_polygon()" << endl;
#endif
   DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
   DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

   // wireframe polygon will be drawn as linestrip, otherwise draw as multi-tri trifan
   DWORD rstate;
   _d3dDevice->GetRenderState(D3DRENDERSTATE_FILLMODE, &rstate);
   if(rstate==D3DFILL_WIREFRAME) {
       draw_linestrip_base(geom,gc,true);
   } else {
       draw_multitri(geom, D3DPT_TRIANGLEFAN);
   }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_quad
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_quad(GeomQuad *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "draw_quad()" << endl;
#endif
   DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
   DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

   // wireframe quad will be drawn as linestrip, otherwise draw as multi-tri trifan
   DWORD rstate;
   _d3dDevice->GetRenderState(D3DRENDERSTATE_FILLMODE, &rstate);
   if(rstate==D3DFILL_WIREFRAME) {
       draw_linestrip_base(geom,gc,true);
   } else {
       draw_multitri(geom, D3DPT_TRIANGLEFAN);
   }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_tri
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_tri(GeomTri *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "draw_tri()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_tri_pcollector.add_level(geom->get_num_vertices()));

#ifdef _DEBUG
    if (_pCurTexContext!=NULL) {
//    dxgsg_cat.spam() << "Cur active DX texture: " << _pCurTexContext->_tex->get_name() << "\n";
    }
#endif

#ifdef COUNT_DRAWPRIMS
    cGeomcount++;
#endif

    int nPrims = geom->get_num_prims();
    HRESULT hr;

    PTA_Vertexf coords;
    PTA_Normalf norms;
    PTA_Colorf colors;
    PTA_TexCoordf texcoords;
    GeomBindType TexCoordBinding,ColorBinding,NormalBinding,junk1;
    PTA_ushort vindexes,nindexes,tindexes,cindexes;

    geom->get_coords(coords,junk1,vindexes);
    geom->get_normals(norms,NormalBinding,nindexes);
    geom->get_colors(colors,ColorBinding,cindexes);
    geom->get_texcoords(texcoords,TexCoordBinding,tindexes);

    GeomVertFormat GeomVrtFmt=FlatVerts;

    // first determine if we're indexed or non-indexed

    if ((vindexes!=NULL)&&(cindexes!=NULL)&&(tindexes!=NULL)&&(nindexes!=NULL)) {
        GeomVrtFmt=IndexedVerts;
        //make sure array sizes are consistent, we can only pass 1 size to DrawIPrm
//      nassertv(coords.size==norms.size);      nassertv(coords.size==colors.size);     nassertv(coords.size==texcoords.size);  need to assert only if we have this w/same binding
        // indexed mode requires all used norms,colors,texcoords,coords array be the same
        // length, or 0 or 1 (dwStride==0), also requires all elements to use the same index array
    } else if (!((vindexes==NULL)&&(cindexes==NULL)&&(tindexes==NULL)&&(nindexes==NULL)))
        GeomVrtFmt=MixedFmtVerts;

#ifdef DONT_USE_DRAWPRIMSTRIDED
    GeomVrtFmt=MixedFmtVerts;
#else
    if(_bDrawPrimDoSetupVertexBuffer) {
      GeomVrtFmt=MixedFmtVerts;
    }
#endif

    // for Indexed Prims and mixed indexed/non-indexed prims, we will use old pipeline for now
    // need to add code to handle fully indexed mode (and handle cases with index arrays of different lengths,
    // values (may only be possible to handle certain cases without reverting to old pipeline)
    if (GeomVrtFmt!=FlatVerts) {
        // this is the old geom setup, it reformats every vtx into an output array passed to d3d

        _perVertex = PER_COORD;

        if (NormalBinding == G_PER_VERTEX)   _perVertex |= PER_NORMAL;
        if (ColorBinding == G_PER_VERTEX)    _perVertex |= PER_COLOR;
        if (TexCoordBinding == G_PER_VERTEX) _perVertex |= PER_TEXCOORD;

        _perPrim = 0;
        if (NormalBinding == G_PER_PRIM) _perPrim |= PER_NORMAL;
        if (ColorBinding == G_PER_PRIM)  _perPrim |= PER_COLOR;

        size_t vertex_size = draw_prim_setup(geom);

        nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
        nassertv(nPrims * 3 * vertex_size < VERT_BUFFER_SIZE);
        _pCurFvfBufPtr = _pFvfBufBasePtr;          // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

        // iterate through the triangle primitive

        for (int i = 0; i < nPrims; i++) {
            if (_perPrim & PER_COLOR) {
                GET_NEXT_COLOR();
            }

            if (_perPrim & PER_NORMAL)
                p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.

            draw_prim_inner_loop(3, geom, _perVertex);
        }

        DWORD nVerts=nPrims*3;

        if(!_bDrawPrimDoSetupVertexBuffer) {
            hr = _d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, _curFVFflags, _pFvfBufBasePtr, nVerts, NULL);
            TestDrawPrimFailure(DrawPrim,hr,_pDD,nVerts,nPrims);
        } else {
            COPYVERTDATA_2_VERTEXBUFFER(D3DPT_TRIANGLELIST,nVerts);
        }

        _pCurFvfBufPtr = NULL;
    } else {

        // new geom setup that uses strided DP calls to avoid making an extra pass over the data

        D3DDRAWPRIMITIVESTRIDEDDATA dps_data;
        memset(&dps_data,0,sizeof(D3DDRAWPRIMITIVESTRIDEDDATA));

#ifdef _DEBUG
        nassertv(!geom->uses_components());  // code ignores lengths array
        nassertv(geom->get_binding(G_COORD) == G_PER_VERTEX);
#endif

        D3DPRIMITIVETYPE primtype=D3DPT_TRIANGLELIST;

        DWORD fvf_flags = D3DFVF_XYZ;
        dps_data.position.lpvData = (VOID*)coords;
        dps_data.position.dwStride = sizeof(D3DVECTOR);

        D3DSHADEMODE NeededShadeMode = (_doFogType!=None) ? D3DSHADE_GOURAUD : D3DSHADE_FLAT;

        const DWORD dwVertsperPrim=3;

        if ((NormalBinding != G_OFF) && wants_normals()) {

            dps_data.normal.lpvData = (VOID*)norms;
            dps_data.normal.dwStride = sizeof(D3DVECTOR);

#ifdef _DEBUG
            nassertv(geom->get_num_vertices_per_prim()==3);
            nassertv( nPrims*dwVertsperPrim*sizeof(D3DVECTOR) <= D3DMAXNUMVERTICES*sizeof(WORD));
            if (NormalBinding==G_PER_VERTEX)
                nassertv(norms.size()>=nPrims*dwVertsperPrim);
#endif

            fvf_flags |= D3DFVF_NORMAL;
            NeededShadeMode = D3DSHADE_GOURAUD;

            Normalf *pExpandedNormalArray = (Normalf *)_index_buf;  // BUGBUG:  need to use real permanent buffers for this conversion
            if (NormalBinding==G_PER_PRIM) {
                // must use tmp array to duplicate-expand per-prim norms to per-vert norms
                Normalf *pOutVec = pExpandedNormalArray;
                Normalf *pInVec=norms;

                nassertv(norms.size()>=nPrims);

                for (int i=0;i<nPrims;i++,pInVec++,pOutVec+=dwVertsperPrim) {
                    *pOutVec     = *pInVec;
                    *(pOutVec+1) = *pInVec;
                    *(pOutVec+2) = *pInVec;
                }

                dps_data.normal.lpvData = (VOID*)pExpandedNormalArray;

            } else if (NormalBinding==G_OVERALL) {
                // copy the one global color in, set stride to 0
                *pExpandedNormalArray=norms[0];
                dps_data.normal.lpvData = (VOID*)pExpandedNormalArray;
                dps_data.normal.dwStride = 0;
            }
        }

        ColorTransition *catt=NULL;
        bool bDoGlobalSceneGraphColor=FALSE,bDoColor=(ColorBinding != G_OFF);

        // We should issue geometry colors only if the scene graph color is off.
        if (get_attribute_into(catt, this)) {
            if (!catt->is_real())
                bDoColor=FALSE;  // this turns off any Geom colors
            else {
                ColorBinding=G_OVERALL;
                bDoGlobalSceneGraphColor=TRUE;
            }
        }

        if (bDoColor || bDoGlobalSceneGraphColor) {
            D3DCOLOR *pOutColor,*pConvertedColorArray;
            Colorf *pInColor=colors;
            pOutColor = pConvertedColorArray = (D3DCOLOR *)_pFvfBufBasePtr;

#ifdef _DEBUG
            nassertv( nPrims*dwVertsperPrim*sizeof(D3DCOLOR) <= VERT_BUFFER_SIZE);
#endif

            fvf_flags |= D3DFVF_DIFFUSE;

            dps_data.diffuse.lpvData = (VOID*)pConvertedColorArray;
            dps_data.diffuse.dwStride = sizeof(D3DCOLOR);

            if (ColorBinding==G_PER_PRIM) {
                // must use tmp array to expand per-prim info to per-vert info

                // Geom nodes store floats for colors, drawprim requires ARGB dwords
                // BUGBUG: eventually this hack every-frame all-colors conversion needs
                // to be done only once as part of a vertex buffer

                if (NeededShadeMode!=D3DSHADE_FLAT) {
                    // but if lighting enabled, we need to color every vert since shading will be GOURAUD

                    if(!(_color_transform_enabled || _alpha_transform_enabled)) {
                        for (int i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsperPrim) {
                            D3DCOLOR newcolr = Colorf_to_D3DCOLOR(*pInColor);
                            *pOutColor     = newcolr;
                            *(pOutColor+1) = newcolr;
                            *(pOutColor+2) = newcolr;
                        }
                     } else {
                        for (int i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsperPrim) {
                            D3DCOLOR newcolr;
                            transform_color(*pInColor,newcolr);

                            *pOutColor     = newcolr;
                            *(pOutColor+1) = newcolr;
                            *(pOutColor+2) = newcolr;
                        }
                    }
                } else {
                    // dont write 2nd,3rd colors in output buffer, these are not used in flat shading
                    // MAKE SURE ShadeMode never set to GOURAUD after this!

                    if(!(_color_transform_enabled || _alpha_transform_enabled)) {
                        for (int i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsperPrim) {
                            *pOutColor = Colorf_to_D3DCOLOR(*pInColor);
                        }
                     } else {
                        for (int i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsperPrim) {
                            transform_color(*pInColor,*pOutColor);
                        }
                    }
                }
            } else if (ColorBinding==G_PER_VERTEX) {
                NeededShadeMode = D3DSHADE_GOURAUD;

                // want to do this conversion once in retained mode
                DWORD cNumColors=nPrims*dwVertsperPrim;

                    if(!(_color_transform_enabled || _alpha_transform_enabled)) {
                        for (int i=0;i<cNumColors;i++,pInColor++,pOutColor++) {
                            *pOutColor = Colorf_to_D3DCOLOR(*pInColor);
                        }
                     } else {
                        for (int i=0;i<cNumColors;i++,pInColor++,pOutColor++) {
                            transform_color(*pInColor,*pOutColor);
                        }
                    }
            } else {
#ifdef _DEBUG
                nassertv(ColorBinding==G_OVERALL);
#endif
                // copy the one global color in, set stride to 0

                if(!(_color_transform_enabled || _alpha_transform_enabled)) {
                    if (bDoGlobalSceneGraphColor) {
                        Colorf colr = catt->get_color();
                        *pConvertedColorArray = Colorf_to_D3DCOLOR(colr);
                    } else {
                        *pConvertedColorArray = Colorf_to_D3DCOLOR(*pInColor);
                    }
                } else {
                    if (bDoGlobalSceneGraphColor) {
                        Colorf colr = catt->get_color();
                        transform_color(colr,*pConvertedColorArray);
                    } else {
                        transform_color(*pInColor,*pConvertedColorArray);
                    }
                }

                dps_data.diffuse.dwStride = 0;
            }
        }

        if ((TexCoordBinding != G_OFF) && _texturing_enabled) {

#ifdef _DEBUG
            nassertv(TexCoordBinding == G_PER_VERTEX);  // only sensible choice for a tri
#endif

            dps_data.textureCoords[0].lpvData = (VOID*)texcoords;
            dps_data.textureCoords[0].dwStride = sizeof(TexCoordf);
            fvf_flags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
        }

        set_shademode(NeededShadeMode);

        DWORD nVerts = nPrims*dwVertsperPrim;

        hr = _d3dDevice->DrawPrimitiveStrided(primtype, fvf_flags, &dps_data, nVerts, NULL);
        TestDrawPrimFailure(DrawPrimStrided,hr,_pDD,nVerts,nPrims);

        _pCurFvfBufPtr = NULL;
    }

///////////////////////////
#if 0
    // test triangle for me to dbg experiments only
    float vert_buf[15] = {
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        33.0, 0.0f, 0.0f,  0.0f, 2.0,
        0.0f, 0.0f, 33.0,  2.0, 0.0f
    };

    _d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_BORDER);
    _d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_BORDER);
    _d3dDevice->SetTextureStageState(0,D3DTSS_BORDERCOLOR,D3DRGBA(0,0,0,0));

    _curFVFflags =  D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) ;
    HRESULT hr = _d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,  _curFVFflags, vert_buf, nPrims*3, NULL);
    TestDrawPrimFailure(DrawPrim,hr,_pDD,nPrims*3,nPrims);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_tristrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_tristrip(GeomTristrip *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
  dxgsg_cat.debug() << "draw_tristrip()" << endl;
#endif
  DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
  DO_PSTATS_STUFF(_vertices_tristrip_pcollector.add_level(geom->get_num_vertices()));

  draw_multitri(geom, D3DPT_TRIANGLESTRIP);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_trifan
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_trifan(GeomTrifan *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "draw_trifan()" << endl;
#endif
  DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
  DO_PSTATS_STUFF(_vertices_trifan_pcollector.add_level(geom->get_num_vertices()));

  draw_multitri(geom, D3DPT_TRIANGLEFAN);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_multitri
//       Access: Public, Virtual
//  Description: handles trifans and tristrips
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_multitri(Geom *geom, D3DPRIMITIVETYPE trilisttype) {

    int nPrims = geom->get_num_prims();
    const int *pLengthArr = geom->get_lengths();
    HRESULT hr;

    if(nPrims==0) {
        #ifdef _DEBUG
          dxgsg_cat.warning() << "draw_multitri() called with ZERO vertices!!" << endl;
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
    GeomBindType TexCoordBinding,ColorBinding,NormalBinding,junk1;
    PTA_ushort vindexes,nindexes,tindexes,cindexes;

    geom->get_coords(coords,junk1,vindexes);
    geom->get_normals(norms,NormalBinding,nindexes);
    geom->get_colors(colors,ColorBinding,cindexes);
    geom->get_texcoords(texcoords,TexCoordBinding,tindexes);

    GeomVertFormat GeomVrtFmt;

#ifdef DONT_USE_DRAWPRIMSTRIDED
    GeomVrtFmt=MixedFmtVerts;
#else
    GeomVrtFmt=FlatVerts;

    if(!geom->uses_components()) {
       GeomVrtFmt=MixedFmtVerts; // dont need efficiency here, just use simpler codepath
    } else {
        // first determine if we're indexed or non-indexed
        if((vindexes!=NULL)&&(cindexes!=NULL)&&(tindexes!=NULL)&&(nindexes!=NULL)) {
            GeomVrtFmt=IndexedVerts;
            //make sure array sizes are consistent, we can only pass 1 size to DrawIPrm
            //      nassertv(coords.size==norms.size);      nassertv(coords.size==colors.size);     nassertv(coords.size==texcoords.size);  need to assert only if we have this w/same binding
            // indexed mode requires all used norms,colors,texcoords,coords array be the same
            // length, or 0 or 1 (dwStride==0), also requires all elements to use the same index array
        } else if (!((vindexes==NULL)&&(cindexes==NULL)&&(tindexes==NULL)&&(nindexes==NULL)))
            GeomVrtFmt=MixedFmtVerts;
    }

    if(_bDrawPrimDoSetupVertexBuffer) {
      GeomVrtFmt=MixedFmtVerts;
    }
#endif

    // for Indexed Prims and mixed indexed/non-indexed prims, we will use old pipeline
    // cant handle indexed prims because usually have different index arrays for different components,
    // and DrIdxPrmStrd only accepts 1 index array for all components
    if (GeomVrtFmt!=FlatVerts) {

        // this is the old geom setup, it reformats every vtx into an output array passed to d3d
        _perVertex = PER_COORD;
        _perPrim = _perComp = 0;

        switch (geom->get_binding(G_NORMAL)) {
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

        switch (geom->get_binding(G_COLOR)) {
            case G_PER_VERTEX:
                _perVertex |= PER_COLOR;
                break;
            case G_PER_PRIM:
                _perPrim |= PER_COLOR;
                break;
            case G_PER_COMPONENT:
                _perComp |= PER_COLOR;
                break;
        }

        if (geom->get_binding(G_TEXCOORD) == G_PER_VERTEX)
            _perVertex |= PER_TEXCOORD;

        size_t vertex_size = draw_prim_setup(geom);

        // iterate through the triangle primitives

        for (int i = 0; i < nPrims; i++) {

            if (_perPrim & PER_COLOR) {
                GET_NEXT_COLOR();
            }
            if (_perPrim & PER_NORMAL)
                p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.
            
            int nVerts;

            if(pLengthArr==NULL) {
                // we've been called by draw_quad, which has no lengths array
                nVerts=4;
            } else {
              nVerts = *(pLengthArr++);
            }

#ifdef _DEBUG
            nassertv(nVerts >= 3);
            nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
            nassertv(nVerts * vertex_size < VERT_BUFFER_SIZE);
#endif
            _pCurFvfBufPtr = _pFvfBufBasePtr;            // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

            if(_perComp==0x0) {
                    draw_prim_inner_loop(nVerts, geom, _perVertex);
            } else {
                    if(trilisttype==D3DPT_TRIANGLESTRIP) {
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

            if(!_bDrawPrimDoSetupVertexBuffer) {
                HRESULT hr = _d3dDevice->DrawPrimitive(trilisttype,  _curFVFflags, _pFvfBufBasePtr, nVerts, NULL);
                TestDrawPrimFailure(DrawPrim,hr,_pDD,nVerts,nVerts-2);
            } else {
                COPYVERTDATA_2_VERTEXBUFFER(trilisttype,nVerts);
            }                       

            _pCurFvfBufPtr = NULL;
        }
    } else {

        // new geom setup that uses strided DP calls to avoid making an extra pass over the data

        D3DDRAWPRIMITIVESTRIDEDDATA dps_data;
        memset(&dps_data,0,sizeof(D3DDRAWPRIMITIVESTRIDEDDATA));

#ifdef _DEBUG
        nassertv(geom->uses_components());
        nassertv(geom->get_binding(G_COORD) == G_PER_VERTEX);
#endif

        DWORD fvf_flags = D3DFVF_XYZ;
        dps_data.position.lpvData = (VOID*)coords;
        dps_data.position.dwStride = sizeof(D3DVECTOR);

        D3DSHADEMODE NeededShadeMode = D3DSHADE_FLAT;

        DWORD cTotalVerts=0;

        for (int i=0;i<nPrims;i++) {
            cTotalVerts+= pLengthArr[i];
        }

        const DWORD cNumMoreVertsthanTris=2;

        if((NormalBinding != G_OFF) && wants_normals()) {

            dps_data.normal.lpvData = (VOID*)norms;
            dps_data.normal.dwStride = sizeof(D3DVECTOR);

#ifdef _DEBUG
            nassertv(geom->get_num_more_vertices_than_components()==2);
            nassertv(NormalBinding!=G_PER_COMPONENT); // makes no sense, unimplementable for strips since normals always shared across tris
            nassertv( cTotalVerts*sizeof(D3DVECTOR) <= D3DMAXNUMVERTICES*sizeof(WORD));
            if(NormalBinding==G_PER_VERTEX)
                nassertv(norms.size()>=cTotalVerts);
#endif
            fvf_flags |= D3DFVF_NORMAL;
            NeededShadeMode = D3DSHADE_GOURAUD;

            Normalf *pExpandedNormalArray = (Normalf *)_index_buf;  // BUGBUG:  need to use real permanent buffers instead of _indexbuf hack

            if(NormalBinding==G_PER_PRIM) {
                // we have 1 normal per strip
                // must use tmp array to duplicate-expand per-prim norms to per-vert norms
                Normalf *pOutVec = pExpandedNormalArray;
                Normalf *pInVec=norms;
                const int *pLengths=pLengthArr;

                nassertv(norms.size()>=nPrims);

                for (int i=0;i<nPrims;i++,pInVec++,pLengths++) {
                    for (int j=0;j<(*pLengths);j++,pOutVec++) {
                        *pOutVec = *pInVec;
                    }
                }

                dps_data.normal.lpvData = (VOID*)pExpandedNormalArray;

            } else if(NormalBinding==G_OVERALL) {
                // copy the one global color in, set stride to 0
                *pExpandedNormalArray=norms[0];
                dps_data.normal.lpvData = (VOID*)pExpandedNormalArray;
                dps_data.normal.dwStride = 0;
            }
        }

        ColorTransition *catt=NULL;
        bool bDoGlobalSceneGraphColor=FALSE,bDoColor=(ColorBinding != G_OFF);

        // We should issue geometry colors only if the scene graph color is off.
        if (get_attribute_into(catt, this)) {
            if (!catt->is_real())
                bDoColor=FALSE;  // this turns off any Geom colors
            else {
                ColorBinding=G_OVERALL;
                bDoGlobalSceneGraphColor=TRUE;
            }
        }

        if (bDoColor || bDoGlobalSceneGraphColor) {
            D3DCOLOR *pOutColor,*pConvertedColorArray;
            Colorf *pInColor=colors;
            pOutColor = pConvertedColorArray = (D3DCOLOR *)_pFvfBufBasePtr;

#ifdef _DEBUG
            nassertv( cTotalVerts*sizeof(D3DCOLOR) <= VERT_BUFFER_SIZE);
#endif

            fvf_flags |= D3DFVF_DIFFUSE;

            dps_data.diffuse.lpvData = (VOID*)pConvertedColorArray;
            dps_data.diffuse.dwStride = sizeof(D3DCOLOR);

            if (ColorBinding==G_PER_VERTEX) {
                NeededShadeMode = D3DSHADE_GOURAUD;

                if(!(_color_transform_enabled || _alpha_transform_enabled)) {
                    for (int i=0;i<cTotalVerts;i++,pInColor++,pOutColor++) {
                        *pOutColor = Colorf_to_D3DCOLOR(*pInColor);
                    }
                } else {
                    for (int i=0;i<cTotalVerts;i++,pInColor++,pOutColor++) {
                        transform_color(*pInColor,*pOutColor);
                    }
                }
            } else if (ColorBinding==G_PER_PRIM) {
                // must use tmp array to expand per-prim info to per-vert info
                // eventually want to do this conversion once in retained mode
                // have one color per strip, need 1 color per vert

                // could save 2 clr writes per strip/fan in flat shade mode but not going to bother here

                if(!(_color_transform_enabled || _alpha_transform_enabled)) {
                    for (int j=0;j<nPrims;j++,pInColor++) {
                        D3DCOLOR lastcolr = Colorf_to_D3DCOLOR(*pInColor);
                        DWORD cStripLength=pLengthArr[j];
                        for (int i=0;i<cStripLength;i++,pOutColor++) {
                            *pOutColor = lastcolr;
                        }
                    }
                } else {
                    for (int j=0;j<nPrims;j++,pInColor++) {
                        D3DCOLOR lastcolr;
                        transform_color(*pInColor,lastcolr);
                        DWORD cStripLength=pLengthArr[j];
                        for (int i=0;i<cStripLength;i++,pOutColor++) {
                            *pOutColor = lastcolr;
                        }
                    }
                }
            } else if (ColorBinding==G_PER_COMPONENT) {
                // have a color per tri, need a color per vert (2 more than #tris)
                // want to do this conversion once in retained mode
                nassertv(colors.size() >= cTotalVerts-nPrims*cNumMoreVertsthanTris);

                #define MULTITRI_COLORCOPY_LOOP                                       \
                    DWORD cCurStripColorCnt=pLengthArr[j]-cNumMoreVertsthanTris;      \
                    for (int i=0;i<cCurStripColorCnt;i++,pInColor++,pOutColor++)

                #define COLOR_CONVERT_COPY_STMT  {*pOutColor = Colorf_to_D3DCOLOR(*pInColor);}
                #define COLOR_CONVERT_XFORM_STMT {transform_color(*pInColor,*pOutColor);}

                #define COMPONENT_COLOR_COPY_LOOPS(COLOR_COPYSTMT)  {                        \
                    if (NeededShadeMode == D3DSHADE_FLAT) {                                  \
                        /* FLAT shade mode.  for tristrips, skip writing last 2 verts.  */   \
                        /* for trifans, skip first and last verts                       */   \
                        if (trilisttype==D3DPT_TRIANGLESTRIP) {                              \
                            for (int j=0;j<nPrims;j++) {                                     \
                                MULTITRI_COLORCOPY_LOOP {                                    \
                                   COLOR_COPYSTMT;                                           \
                                }                                                            \
                                pOutColor+=cNumMoreVertsthanTris;                            \
                            }                                                                \
                        } else {  /* trifan */                                               \
                            for (int j=0;j<nPrims;j++) {                                     \
                                pOutColor++;                                                 \
                                MULTITRI_COLORCOPY_LOOP {                                    \
                                   COLOR_COPYSTMT;                                           \
                                }                                                            \
                                pOutColor++;                                                 \
                            }                                                                \
                        }                                                                    \
                    } else {  /* GOURAUD shademode (due to presence of normals) */           \
                        if (trilisttype==D3DPT_TRIANGLESTRIP) {                              \
                            for (int j=0;j<nPrims;j++) {                                     \
                                MULTITRI_COLORCOPY_LOOP {                                    \
                                   COLOR_COPYSTMT;                                           \
                                }                                                            \
                                DWORD lastcolr = *(pOutColor-1);                             \
                                *pOutColor++ = lastcolr;                                     \
                                *pOutColor++ = lastcolr;                                     \
                            }                                                                \
                        } else {  /* trifan */                                               \
                            for (int j=0;j<nPrims;j++) {                                     \
                                COLOR_COPYSTMT;                                              \
                                pOutColor++;                                                 \
                                MULTITRI_COLORCOPY_LOOP {                                    \
                                   COLOR_COPYSTMT;                                           \
                                }                                                            \
                                *pOutColor++ = *(pOutColor-1);                               \
                            }                                                                \
                        }                                                                    \
                    }                                                                        \
                  }

                if(!(_color_transform_enabled || _alpha_transform_enabled)) {
                  COMPONENT_COLOR_COPY_LOOPS(COLOR_CONVERT_COPY_STMT);
                } else {
                  COMPONENT_COLOR_COPY_LOOPS(COLOR_CONVERT_XFORM_STMT);
                }
            } else {
#ifdef _DEBUG
                nassertv(ColorBinding==G_OVERALL);
#endif
                // copy the one global color in, set stride to 0

                if(!(_color_transform_enabled || _alpha_transform_enabled)) {
                    if (bDoGlobalSceneGraphColor) {
                        Colorf colr = catt->get_color();
                        *pConvertedColorArray = Colorf_to_D3DCOLOR(colr);
                    } else {
                        *pConvertedColorArray = Colorf_to_D3DCOLOR(*pInColor);
                    }
                } else {
                    if (bDoGlobalSceneGraphColor) {
                        Colorf colr = catt->get_color();
                        transform_color(colr,*pConvertedColorArray);
                    } else {
                        transform_color(*pInColor,*pConvertedColorArray);
                    }
                }

                dps_data.diffuse.dwStride = 0;
            }
        }

        if ((TexCoordBinding != G_OFF) && _texturing_enabled) {

#ifdef _DEBUG
            nassertv(TexCoordBinding == G_PER_VERTEX);  // only sensible choice for a tri
#endif

            dps_data.textureCoords[0].lpvData = (VOID*)texcoords;
            dps_data.textureCoords[0].dwStride = sizeof(TexCoordf);
            fvf_flags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
        }

        set_shademode(NeededShadeMode);

        for (int j=0;j<nPrims;j++) {
            const int cCurNumStripVerts = pLengthArr[j];

            hr = _d3dDevice->DrawPrimitiveStrided(trilisttype, fvf_flags, &dps_data, cCurNumStripVerts, NULL);
            TestDrawPrimFailure(DrawPrimStrided,hr,_pDD,cCurNumStripVerts,cCurNumStripVerts-2);

            dps_data.position.lpvData = (VOID*)(((char*) dps_data.position.lpvData) + cCurNumStripVerts*dps_data.position.dwStride);
            dps_data.diffuse.lpvData = (VOID*)(((char*) dps_data.diffuse.lpvData) + cCurNumStripVerts*dps_data.diffuse.dwStride);
            dps_data.normal.lpvData = (VOID*)(((char*) dps_data.normal.lpvData) + cCurNumStripVerts*dps_data.normal.dwStride);
            dps_data.textureCoords[0].lpvData = (VOID*)(((char*) dps_data.textureCoords[0].lpvData) + cCurNumStripVerts*dps_data.textureCoords[0].dwStride);
        }

        nassertv(_pCurFvfBufPtr == NULL);
    }
}

//-----------------------------------------------------------------------------
// Name: GenerateSphere()
// Desc: Makes vertex and index data for ellipsoid w/scaling factors sx,sy,sz
//       tries to match gluSphere behavior
//-----------------------------------------------------------------------------

void DXGraphicsStateGuardian::
GenerateSphere(void *pVertexSpace,DWORD dwVertSpaceByteSize,
               void *pIndexSpace,DWORD dwIndexSpaceByteSize,
               D3DVECTOR *pCenter, float fRadius,
               DWORD wNumRings, DWORD wNumSections, float sx, float sy, float sz,
               DWORD *pNumVertices,DWORD *pNumIndices,DWORD fvfFlags,DWORD dwVertSize) {

    float x, y, z, rsintheta;
    D3DVECTOR vPoint;

//#define DBG_GENSPHERE
#define M_PI 3.1415926f   // probably should get this from mathNumbers.h instead

    nassertv(wNumRings>=2 && wNumSections>=2);
    wNumRings--;  // wNumRings indicates number of vertex rings (not tri-rings).
                  // gluSphere 'stacks' arg for 1 vert ring is 2, so convert to our '1'.
    wNumSections++;  // to make us equiv to gluSphere

    //Figure out needed space for the triangles and vertices.
    DWORD dwNumVertices,dwNumIndices,dwNumTriangles;

#define DOTEXTURING (fvfFlags & D3DFVF_TEXCOUNT_MASK)
#define DONORMAL (fvfFlags & D3DFVF_NORMAL)
#define DOCOLOR (fvfFlags & D3DFVF_DIFFUSE)

    if (DOTEXTURING) {
        // if texturing, we need full rings of identical position verts at poles to hold diff texture coords
        wNumRings+=2;
        dwNumVertices = *pNumVertices = wNumRings * wNumSections;
        dwNumTriangles = (wNumRings-1) * wNumSections * 2;
    } else {
        dwNumVertices = *pNumVertices = wNumRings * wNumSections + 2;
        dwNumTriangles = wNumRings*wNumSections*2;
    }

    dwNumIndices = *pNumIndices = dwNumTriangles*3;

    D3DVERTEX* pvVertices = (D3DVERTEX*) pVertexSpace;
    WORD *pwIndices = (WORD *) pIndexSpace;

    nassertv(dwNumVertices*dwVertSize < VERT_BUFFER_SIZE);
    nassertv(dwNumIndices < D3DMAXNUMVERTICES );

    // Generate vertex at the top point
    D3DVECTOR vTopPoint  = *pCenter + D3DVECTOR( 0.0f, +sy*fRadius, 0.0f);
    D3DVECTOR vBotPoint  = *pCenter + D3DVECTOR( 0.0f, -sy*fRadius, 0.0f);
    D3DVECTOR vNormal = D3DVECTOR( 0.0f, 1.0f, 0.0f );
    float texCoords[2];

    nassertv(pVertexSpace==_pCurFvfBufPtr);  // add_to_FVFBuf requires this

#define ADD_GENSPHERE_VERTEX_TO_BUFFER(VERT)                      \
    add_to_FVFBuf((void *)&(VERT), sizeof(D3DVECTOR));            \
    if(fvfFlags & D3DFVF_NORMAL)                                  \
        add_to_FVFBuf((void *)&vNormal, sizeof(D3DVECTOR));       \
    if(fvfFlags & D3DFVF_DIFFUSE)                                 \
        add_DWORD_to_FVFBuf(_curD3Dcolor);                              \
    if(fvfFlags & D3DFVF_TEXCOUNT_MASK)                           \
        add_to_FVFBuf((void *)texCoords, sizeof(TexCoordf));

#ifdef DBG_GENSPHERE
    int nvs_written=0;
    memset(pVertexSpace,0xFF,dwNumVertices*dwVertSize);
#endif

    if (! DOTEXTURING) {
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

    if (DOTEXTURING) {
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

        if (DOTEXTURING) {
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
            vPoint = *pCenter + D3DVECTOR( sx*x, sy*y, sz*z );

            add_to_FVFBuf((void *)&vPoint, sizeof(D3DVECTOR));

            if (DONORMAL) {
                // this is wrong normal for the non-spherical case (i think you need to multiply by 1/scale factor per component)
                vNormal = Normalize(D3DVECTOR( x*inv_radius, y*inv_radius, z*inv_radius ));
                add_to_FVFBuf((void *)&vNormal, sizeof(D3DVECTOR));
            }

            if (DOCOLOR)
                add_DWORD_to_FVFBuf(_curD3Dcolor);

            if (DOTEXTURING) {
                texCoords[0] = 1.0f - phi*reciprocal_2PI;
                add_to_FVFBuf((void *)texCoords, sizeof(TexCoordf));
            }

            phi += dphi;
        }
        theta += dtheta;
    }

    if (! DOTEXTURING) {
        // Generate bottom vertex
        vNormal = D3DVECTOR( 0.0f, -1.0f, 0.0f );
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

    if (! DOTEXTURING) {
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
    if (DOTEXTURING) {
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
//     Function: DXGraphicsStateGuardian::draw_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_sphere(GeomSphere *geom, GeomContext *gc) {

#define SPHERE_NUMSLICES 16
#define SPHERE_NUMSTACKS 10

#ifdef GSG_VERBOSE
    dxgsg_cat.debug() << "draw_sphere()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    int nprims = geom->get_num_prims();

    if (nprims==0) {
        dxgsg_cat.warning() << "draw_sphere() called with ZERO vertices!!" << endl;
        return;
    }

    Geom::VertexIterator vi = geom->make_vertex_iterator();
    Geom::ColorIterator ci;
    bool bperPrimColor = (geom->get_binding(G_COLOR) == G_PER_PRIM);
    if (bperPrimColor)
        ci = geom->make_color_iterator();

    for (int i = 0; i < nprims; i++) {

        DWORD nVerts,nIndices;
        Vertexf center = geom->get_next_vertex(vi);
        Vertexf edge = geom->get_next_vertex(vi);
        LVector3f v = edge - center;
        float fRadius = sqrt(dot(v, v));

        size_t vertex_size = draw_prim_setup(geom);

        _pCurFvfBufPtr = _pFvfBufBasePtr;

        if (bperPrimColor) {
            GET_NEXT_COLOR();
        }

        GenerateSphere(_pCurFvfBufPtr, VERT_BUFFER_SIZE,
                       _index_buf, D3DMAXNUMVERTICES,
                       (D3DVECTOR *)&center, fRadius,
                       SPHERE_NUMSTACKS, SPHERE_NUMSLICES,
                       1.0f, 1.0f, 1.0f,  // no scaling factors, do a sphere not ellipsoid
                       &nVerts,&nIndices,_curFVFflags,vertex_size);

        // possible optimization: make DP 1 for all spheres call here, since trilist is independent tris.
        // indexes couldnt start w/0 tho, need to pass offset to gensph
        HRESULT hr = _d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,  _curFVFflags, _pFvfBufBasePtr, nVerts, _index_buf,nIndices,NULL);
        TestDrawPrimFailure(DrawIndexedPrim,hr,_pDD,nVerts,(nIndices>>2));
    }

    _pCurFvfBufPtr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::prepare_texture
//       Access: Public, Virtual
//  Description: Creates a new retained-mode representation of the
//               given texture, and returns a newly-allocated
//               TextureContext pointer to reference it.  It is the
//               responsibility of the calling function to later
//               call release_texture() with this same pointer (which
//               will also delete the pointer).
////////////////////////////////////////////////////////////////////
TextureContext *DXGraphicsStateGuardian::
prepare_texture(Texture *tex) {

    DXTextureContext *gtc = new DXTextureContext(tex);
#ifdef WBD_GL_MODE
    glGenTextures(1, &gtc->_index);

    bind_texture(gtc);
    glPrioritizeTextures(1, &gtc->_index, &gtc->_priority);
    specify_texture(tex);
    apply_texture_immediate(tex);
#else
    if (gtc->CreateTexture(_d3dDevice,_cNumTexPixFmts,_pTexPixFmts) == NULL) {
        delete gtc;
        return NULL;
    }
#endif              // WBD_GL_MODE

    bool inserted = mark_prepared_texture(gtc);

    // If this assertion fails, the same texture was prepared twice,
    // which shouldn't be possible, since the texture itself should
    // detect this.
    nassertr(inserted, NULL);

    return gtc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_texture
//       Access: Public, Virtual
//  Description: Makes the texture the currently available texture for
//               rendering.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
apply_texture(TextureContext *tc) {
    if (tc==NULL) {
        return;  // use enable_texturing to disable/enable
    }
    #ifdef DO_PSTATS
       add_to_texture_record(tc);
    #endif

//  bind_texture(tc);

//  specify_texture(tc->_texture);
    // Note: if this code changes, make sure to change initialization SetTSS code in init_dx as well
    // so DX TSS renderstate matches dxgsg state

    DXTextureContext *dtc = DCAST(DXTextureContext, tc);

    if( _pCurTexContext == dtc) {
        return;  // tex already set (and possible problem in state-sorting?)
    }

    Texture *tex = tc->_texture;
    Texture::WrapMode wrapU,wrapV;
    wrapU=tex->get_wrapu();
    wrapV=tex->get_wrapv();

    if (wrapU!=_CurTexWrapModeU) {
        _d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,get_texture_wrap_mode(wrapU));
        _CurTexWrapModeU = wrapU;
    }
    if (wrapV!=_CurTexWrapModeV) {
        _d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,get_texture_wrap_mode(wrapV));
        _CurTexWrapModeV = wrapV;
    }

/*
#ifdef _DEBUG
    Texture::WrapMode wrapval;
    _d3dDevice->GetTextureStageState(0,D3DTSS_ADDRESSU,(DWORD*)&wrapval);
    assert(get_texture_wrap_mode(wrapU) == wrapval);
    _d3dDevice->GetTextureStageState(0,D3DTSS_ADDRESSV,(DWORD*)&wrapval);
    assert(get_texture_wrap_mode(wrapV) == wrapval);
#endif
*/

    int aniso_degree=tex->get_anisotropic_degree();
    Texture::FilterType ft=tex->get_magfilter();

    if (aniso_degree>1) {
        if (aniso_degree!=_CurTexAnisoDegree) {
            _CurTexAnisoDegree = aniso_degree;
            _d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_ANISOTROPIC );
            _d3dDevice->SetTextureStageState(0, D3DTSS_MAXANISOTROPY,aniso_degree);
        }
    } else {
        if (_CurTexMagFilter!=ft) {

            _CurTexMagFilter = ft;
            _d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER,(ft==Texture::FT_nearest)? D3DTFG_POINT : D3DTFG_LINEAR);
#ifdef _DEBUG
            if((ft!=Texture::FT_linear)&&(ft!=Texture::FT_nearest)) {
                dxgsg_cat.error() << "MipMap filter type setting for texture magfilter makes no sense,  texture: " << tex->get_name() << "\n";
            }
#endif
        }
    }

#ifdef _DEBUG
    assert(Texture::FT_linear_mipmap_linear < 8);
#endif
/*
 enum FilterType {
    FT_nearest,FT_linear,FT_nearest_mipmap_nearest,FT_linear_mipmap_nearest,
    FT_nearest_mipmap_linear, FT_linear_mipmap_linear, };
*/
 static D3DTEXTUREMINFILTER PandaToD3DMinType[8] =
    {D3DTFN_POINT,D3DTFN_LINEAR,D3DTFN_POINT,D3DTFN_LINEAR,D3DTFN_POINT,D3DTFN_LINEAR};
 static D3DTEXTUREMIPFILTER PandaToD3DMipType[8] =
    {D3DTFP_NONE,D3DTFP_NONE,D3DTFP_POINT,D3DTFP_POINT,D3DTFP_LINEAR,D3DTFP_LINEAR};

    ft=tex->get_minfilter();

    if ((ft!=_CurTexMinFilter)||(aniso_degree!=_CurTexAnisoDegree)) {

#ifdef _DEBUG
        if(ft > Texture::FT_linear_mipmap_linear) {
                dxgsg_cat.error() << "Unknown tex filter type for tex: " << tex->get_name() << "  filter: "<<(DWORD)ft<<"\n";
                return;
        }
#endif

        D3DTEXTUREMINFILTER minfilter = PandaToD3DMinType[(DWORD)ft];
        D3DTEXTUREMIPFILTER mipfilter = PandaToD3DMipType[(DWORD)ft];
/*
        switch (ft) {
            case Texture::FT_nearest:
                minfilter = D3DTFN_POINT;
                mipfilter = D3DTFP_NONE;
                break;
            case Texture::FT_linear:
                minfilter = D3DTFN_LINEAR;
                mipfilter = D3DTFP_NONE;
                break;
            case Texture::FT_nearest_mipmap_nearest:
                minfilter = D3DTFN_POINT;
                mipfilter = D3DTFP_POINT;
                break;
            case Texture::FT_linear_mipmap_nearest:
                minfilter = D3DTFN_LINEAR;
                mipfilter = D3DTFP_POINT;
                break;
            case Texture::FT_nearest_mipmap_linear:
                minfilter = D3DTFN_POINT;
                mipfilter = D3DTFP_LINEAR;
                break;
            case Texture::FT_linear_mipmap_linear:
                minfilter = D3DTFN_LINEAR;
                mipfilter = D3DTFP_LINEAR;
                break;
            default:
                dxgsg_cat.error() << "Unknown tex filter type for tex: " << tex->get_name() << "  filter: "<<(DWORD)ft<<"\n";
        }
*/

        #ifndef NDEBUG
            extern char *PandaFilterNameStrs[];
            if((!(dtc->_bHasMipMaps))&&(mipfilter!=D3DTFP_NONE)) {
                dxgsg_cat.error() << "Trying to set mipmap filtering for texture with no generated mipmaps!! texname[" << tex->get_name() << "], filter("<<PandaFilterNameStrs[ft]<<")\n";
                mipfilter=D3DTFP_NONE;
            }
        #endif

        if (aniso_degree>1) {
            minfilter=D3DTFN_ANISOTROPIC;
        }

        _d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, minfilter);
        _d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, mipfilter);

        _CurTexMinFilter = ft;
        _CurTexAnisoDegree = aniso_degree;
    }

    // bugbug:  does this handle the case of untextured geometry?
    //          we dont see this bug cause we never mix textured/untextured

    _d3dDevice->SetTexture(0, dtc->_surface );

#if 0
    if (dtc!=NULL) {
        dxgsg_cat.spam() << "Setting active DX texture: " << dtc->_tex->get_name() << "\n";
    }
#endif

    _pCurTexContext = dtc;   // enable_texturing needs this
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
release_texture(TextureContext *tc) {
    DXTextureContext *gtc = DCAST(DXTextureContext, tc);
    Texture *tex = tc->_texture;

    gtc->DeleteTexture();
    bool erased = unmark_prepared_texture(gtc);

    // If this assertion fails, a texture was released that hadn't been
    // prepared (or a texture was released twice).
    nassertv(erased);

    tex->clear_gsg(this);

    delete gtc;
}

#if 1

void DXGraphicsStateGuardian::
copy_texture(TextureContext *tc, const DisplayRegion *dr) {
    dxgsg_cat.fatal() << "DX copy_texture unimplemented!!!";
}

#else
static int logs[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
    4096, 0};

// This function returns the smallest power of two greater than or
// equal to x.
static int binary_log_cap(const int x) {
    int i = 0;
    for (; (x > logs[i]) && (logs[i] != 0); ++i);
    if (logs[i] == 0)
        return 4096;
    return logs[i];
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::copy_texture
//       Access: Public, Virtual
//  Description: Copy the pixel region indicated by the display
//       region from the framebuffer into texture memory
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
copy_texture(TextureContext *tc, const DisplayRegion *dr) {

    nassertv(tc != NULL && dr != NULL);

    Texture *tex = tc->_texture;

    // Determine the size of the grab from the given display region
    // If the requested region is not a power of two, grab a region that is
    // a power of two that contains the requested region
    int xo, yo, req_w, req_h;
    dr->get_region_pixels(xo, yo, req_w, req_h);
    int w = binary_log_cap(req_w);
    int h = binary_log_cap(req_h);
    if (w != req_w || h != req_h) {
        tex->_requested_w = req_w;
        tex->_requested_h = req_h;
        tex->_has_requested_size = true;
    }

    PixelBuffer *pb = tex->_pbuffer;

    pb->set_xorg(xo);
    pb->set_yorg(yo);
    pb->set_xsize(w);
    pb->set_ysize(h);


    bind_texture(tc);
    glCopyTexImage2D( GL_TEXTURE_2D, tex->get_level(),
                      get_internal_image_format(pb->get_format()),
                      pb->get_xorg(), pb->get_yorg(),
                      pb->get_xsize(), pb->get_ysize(), pb->get_border() );
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::copy_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
copy_texture(TextureContext *tc, const DisplayRegion *dr, const RenderBuffer &rb) {
    dxgsg_cat.fatal() << "DX copy_texture unimplemented!!!";
    return;

    set_read_buffer(rb);
    copy_texture(tc, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_texture(TextureContext *tc, const DisplayRegion *dr) {

    dxgsg_cat.fatal() << "DXGSG draw_texture unimplemented!!!";
    return;

    nassertv(tc != NULL && dr != NULL);

#ifdef WBD_GL_MODE
    Texture *tex = tc->_texture;

    DisplayRegionStack old_dr = push_display_region(dr);
    prepare_display_region();

    NodeTransitions state;
    CullFaceTransition *cfa = new CullFaceTransition(CullFaceProperty::M_cull_none);
    DepthTestTransition *dta = new DepthTestTransition(DepthTestProperty::M_none);
    DepthWriteTransition *dwa = new DepthWriteTransition(DepthWriteTransition::off());
    TextureTransition *ta = new TextureTransition(tex);
    TextureApplyTransition *taa = new TextureApplyTransition(TextureApplyProperty::M_decal);

    state.set_transition(new LightTransition);
    state.set_transition(new ColorMaskTransition);
    state.set_transition(new RenderModeTransition);
    state.set_transition(new TexMatrixTransition);
    state.set_transition(new TransformTransition);
    state.set_transition(new ColorBlendTransition);
    state.set_transition(cfa);
    state.set_transition(dta);
    state.set_transition(dwa);
    state.set_transition(ta);
    state.set_transition(taa);
    modify_state(state);

    // We set up an orthographic projection that defines our entire
    // viewport to the range [0..1] in both dimensions.  Then, when we
    // create a unit square polygon below, it will exactly fill the
    // viewport (and thus exactly fill the display region).
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);

    float txl, txr, tyt, tyb;
    txl = tyb = 0.;
    if (tex->_has_requested_size) {
        txr = ((float)(tex->_requested_w)) / ((float)(tex->_pbuffer->get_xsize()));
        tyt = ((float)(tex->_requested_h)) / ((float)(tex->_pbuffer->get_ysize()));
    } else {
        txr = tyt = 1.;
    }

    // This two-triangle strip is actually a quad.  But it's usually
    // better to render quads as tristrips anyway.
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(txl, tyb);   glVertex2i(0, 0);
    glTexCoord2f(txr, tyb);   glVertex2i(1, 0);
    glTexCoord2f(txl, tyt);   glVertex2i(0, 1);
    glTexCoord2f(txr, tyt);   glVertex2i(1, 1);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    pop_display_region(old_dr);
#endif              // WBD_GL_MODE

}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_texture(TextureContext *tc, const DisplayRegion *dr, const RenderBuffer &rb) {
    dxgsg_cat.fatal() << "DXGSG draw_texture unimplemented!!!";
    return;

    set_draw_buffer(rb);
    draw_texture(tc, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::texture_to_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb) {
    nassertv(tc != NULL && pb != NULL);

    Texture *tex = tc->_texture;

    int w = tex->_pbuffer->get_xsize();
    int h = tex->_pbuffer->get_ysize();

    PT(DisplayRegion) dr = _win->make_scratch_display_region(w, h);

    FrameBufferStack old_fb = push_frame_buffer
                              (get_render_buffer(RenderBuffer::T_back | RenderBuffer::T_depth),
                               dr);

    texture_to_pixel_buffer(tc, pb, dr);

    pop_frame_buffer(old_fb);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::texture_to_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb,
                        const DisplayRegion *dr) {
    nassertv(tc != NULL && pb != NULL && dr != NULL);

    Texture *tex = tc->_texture;

    // Do a deep copy to initialize the pixel buffer
    pb->copy(tex->_pbuffer);

    // If the image was empty, we need to render the texture into the frame
    // buffer and then copy the results into the pixel buffer's image
    if (pb->_image.empty()) {
        int w = pb->get_xsize();
        int h = pb->get_ysize();
        draw_texture(tc, dr);
        pb->_image = PTA_uchar(w * h * pb->get_num_components());
        copy_pixel_buffer(pb, dr);
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr) {

    extern HRESULT ConvertDDSurftoPixBuf(PixelBuffer *pixbuf,LPDIRECTDRAWSURFACE7 pDDSurf);

    nassertv(pb != NULL && dr != NULL);

    int xo, yo, w, h;
    dr->get_region_pixels(xo, yo, w, h);

    // only handled simple case
    nassertv(xo==0);
    nassertv(yo==0);
    nassertv(w==pb->get_xsize());
    nassertv(h==pb->get_ysize());

/*
    set_pack_alignment(1);
    glReadPixels( pb->get_xorg() + xo, pb->get_yorg() + yo,
                  pb->get_xsize(), pb->get_ysize(),
                  get_external_image_format(pb->get_format()),
                  get_image_type(pb->get_image_type()),
                  pb->_image.p() );
*/


    (void) ConvertDDSurftoPixBuf(pb,((_cur_read_pixel_buffer & RenderBuffer::T_back) ? _back : _pri));

    nassertv(!pb->_image.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                  const RenderBuffer &rb) {
    set_read_buffer(rb);
    copy_pixel_buffer(pb, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                  const NodeTransitions& na) {

    dxgsg_cat.fatal() << "DXGSG draw_pixel_buffer unimplemented!!!";
    return;

#ifdef WBD_GL_MODE
    nassertv(pb != NULL && dr != NULL);
    nassertv(!pb->_image.empty());

    DisplayRegionStack old_dr = push_display_region(dr);
    prepare_display_region();

    NodeTransitions state(na);
    state.set_transition(new LightTransition);
    state.set_transition(new TextureTransition);
    state.set_transition(new TransformTransition);
    state.set_transition(new ColorBlendTransition);
    state.set_transition(new StencilTransition);


    switch (pb->get_format()) {
        case PixelBuffer::F_depth_component:
            {
                ColorMaskTransition *cma = new ColorMaskTransition(0);
                DepthTestTransition *dta = new DepthTestTransition(DepthTestProperty::M_always);
                DepthWriteTransition *dwa = new DepthWriteTransition(DepthWriteTransition::off());
                state.set_transition(cma);
                state.set_transition(dta);
                state.set_transition(dwa);
            }
            break;

        case PixelBuffer::F_rgb:
        case PixelBuffer::F_rgb5:
        case PixelBuffer::F_rgb8:
        case PixelBuffer::F_rgb12:
        case PixelBuffer::F_rgba:
        case PixelBuffer::F_rgba4:
        case PixelBuffer::F_rgba8:
        case PixelBuffer::F_rgba12:
            {
                ColorMaskTransition *cma = new ColorMaskTransition;
                DepthTestTransition *dta = new DepthTestTransition(DepthTestProperty::M_none);
                DepthWriteTransition *dwa = new DepthWriteTransition(DepthWriteTransition::off());
                state.set_transition(cma);
                state.set_transition(dta);
                state.set_transition(dwa);
            }
            break;
        default:
            dxgsg_cat.error()
            << "draw_pixel_buffer(): unknown buffer format" << endl;
            break;
    }

    modify_state(state);

    enable_color_material(false);
    set_unpack_alignment(1);

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, _win->get_width(),
               0, _win->get_height());

#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "glDrawPixels(" << pb->get_xsize() << ", " << pb->get_ysize()
    << ", ";
    switch (get_external_image_format(pb->get_format())) {
        case GL_DEPTH_COMPONENT:
            dxgsg_cat.debug(false) << "GL_DEPTH_COMPONENT, ";
            break;
        case GL_RGB:
            dxgsg_cat.debug(false) << "GL_RGB, ";
            break;
        case GL_RGBA:
            dxgsg_cat.debug(false) << "GL_RGBA, ";
            break;
        default:
            dxgsg_cat.debug(false) << "unknown, ";
            break;
    }
    switch (get_image_type(pb->get_image_type())) {
        case GL_UNSIGNED_BYTE:
            dxgsg_cat.debug(false) << "GL_UNSIGNED_BYTE, ";
            break;
        case GL_float:
            dxgsg_cat.debug(false) << "GL_float, ";
            break;
        default:
            dxgsg_cat.debug(false) << "unknown, ";
            break;
    }
    dxgsg_cat.debug(false)
    << (void *)pb->_image.p() << ")" << endl;
#endif

    glRasterPos2i( pb->get_xorg(), pb->get_yorg() );
    glDrawPixels( pb->get_xsize(), pb->get_ysize(),
                  get_external_image_format(pb->get_format()),
                  get_image_type(pb->get_image_type()),
                  pb->_image.p() );

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    pop_display_region(old_dr);
#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                  const RenderBuffer &rb, const NodeTransitions& na) {
    set_read_buffer(rb);
    draw_pixel_buffer(pb, dr, na);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::apply_material( const Material* material ) {
    D3DMATERIAL7 cur_material;
    cur_material.dcvDiffuse = *(D3DCOLORVALUE *)(material->get_diffuse().get_data());
    cur_material.dcvAmbient = *(D3DCOLORVALUE *)(material->get_ambient().get_data());
    cur_material.dcvSpecular = *(D3DCOLORVALUE *)(material->get_specular().get_data());
    cur_material.dcvEmissive = *(D3DCOLORVALUE *)(material->get_emission().get_data());
    cur_material.dvPower   =  material->get_shininess();
    _d3dDevice->SetMaterial(&cur_material);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
apply_fog(Fog *fog) {

    if(_doFogType==None)
      return;

    Fog::Mode panda_fogmode = fog->get_mode();
    D3DFOGMODE d3dfogmode = get_fog_mode_type(panda_fogmode);

    // should probably avoid doing redundant SetRenderStates, but whatever
    _d3dDevice->SetRenderState((D3DRENDERSTATETYPE)_doFogType, d3dfogmode);

    Colorf  fog_colr = fog->get_color();
    _d3dDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,
                   D3DRGBA(fog_colr[0], fog_colr[1], fog_colr[2], 0.0f));  // Alpha bits are not used

    // do we need to adjust fog start/end values based on D3DPRASTERCAPS_WFOG/D3DPRASTERCAPS_ZFOG ?
    // if not WFOG, then docs say we need to adjust values to range [0,1]

    switch (panda_fogmode) {
        case Fog::M_linear:
            {
                float fog_start,fog_end;
                fog->get_range(fog_start,fog_end);

                _d3dDevice->SetRenderState( D3DRENDERSTATE_FOGSTART,
                            *((LPDWORD) (&fog_start)) );
                _d3dDevice->SetRenderState( D3DRENDERSTATE_FOGEND,
                            *((LPDWORD) (&fog_end)) );
            }
            break;
        case Fog::M_exponential:
        case Fog::M_exponential_squared:
            {
                float fog_density = fog->get_density();
                _d3dDevice->SetRenderState( D3DRENDERSTATE_FOGDENSITY,
                            *((LPDWORD) (&fog_density)) );
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::apply_light( PointLight* light ) {
    // The light position will be relative to the current matrix, so
    // we have to know what the current matrix is.  Find a better
    // solution later.
#ifdef WBD_GL_MODE
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "glMatrixMode(GL_MODELVIEW)" << endl;
    dxgsg_cat.debug()
    << "glPushMatrix()" << endl;
    dxgsg_cat.debug()
    << "glLoadIdentity()" << endl;
#endif
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glLoadMatrixf(LMatrix4f::convert_mat(_coordinate_system, CS_yup_left)
                  .get_data());

    GLenum id = get_light_id( _cur_light_id );
    Colorf black(0, 0, 0, 1);
    glLightfv(id, GL_AMBIENT, black.get_data());
    glLightfv(id, GL_DIFFUSE, light->get_color().get_data());
    glLightfv(id, GL_SPECULAR, light->get_specular().get_data());

    // Position needs to specify x, y, z, and w
    // w == 1 implies non-infinite position
    LPoint3f pos = get_rel_pos( light, _current_projection_node );
    LPoint4f fpos( pos[0], pos[1], pos[2], 1 );
    glLightfv( id, GL_POSITION, fpos.get_data() );

    // GL_SPOT_DIRECTION is not significant when cutoff == 180

    // Exponent == 0 implies uniform light distribution
    glLightf( id, GL_SPOT_EXPONENT, 0 );

    // Cutoff == 180 means uniform point light source
    glLightf( id, GL_SPOT_CUTOFF, 180.0 );

    glLightf( id, GL_CONSTANT_ATTENUATION,
              light->get_constant_attenuation() );
    glLightf( id, GL_LINEAR_ATTENUATION,
              light->get_linear_attenuation() );
    glLightf( id, GL_QUADRATIC_ATTENUATION,
              light->get_quadratic_attenuation() );

    glPopMatrix();

#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "glPopMatrix()" << endl;
#endif

#else
    D3DCOLORVALUE black;
    black.r = black.g = black.b = black.a = 0.0f;
    D3DLIGHT7  alight;
    alight.dltType =  D3DLIGHT_POINT;
    alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
    alight.dcvAmbient  =  black ;
    alight.dcvSpecular = *(D3DCOLORVALUE *)(light->get_specular().get_data());

    // Position needs to specify x, y, z, and w
    // w == 1 implies non-infinite position
    alight.dvPosition = *(D3DVECTOR *)(get_rel_pos( light, _current_root_node ).get_data());

    alight.dvRange =  D3DLIGHT_RANGE_MAX;
    alight.dvFalloff =  1.0f;

    alight.dvAttenuation0 = (D3DVALUE)light->get_constant_attenuation();
    alight.dvAttenuation1 = (D3DVALUE)light->get_linear_attenuation();
    alight.dvAttenuation2 = (D3DVALUE)light->get_quadratic_attenuation();

    HRESULT res = _d3dDevice->SetLight(_cur_light_id, &alight);

#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::apply_light( DirectionalLight* light ) {
    // The light position will be relative to the current matrix, so
    // we have to know what the current matrix is.  Find a better
    // solution later.
#ifdef WBD_GL_MODE
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "glMatrixMode(GL_MODELVIEW)" << endl;
    dxgsg_cat.debug()
    << "glPushMatrix()" << endl;
    dxgsg_cat.debug()
    << "glLoadIdentity()" << endl;
#endif
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(LMatrix4f::convert_mat(_coordinate_system, CS_yup_left)
                  .get_data());

    GLenum id = get_light_id( _cur_light_id );
    Colorf black(0, 0, 0, 1);
    glLightfv(id, GL_AMBIENT, black.get_data());
    glLightfv(id, GL_DIFFUSE, light->get_color().get_data());
    glLightfv(id, GL_SPECULAR, light->get_specular().get_data());

    // Position needs to specify x, y, z, and w
    // w == 0 implies light is at infinity
    LPoint3f dir = get_rel_forward( light, _current_root_node,
                                    _coordinate_system );
    LPoint4f pos( -dir[0], -dir[1], -dir[2], 0 );
    glLightfv( id, GL_POSITION, pos.get_data() );

    // GL_SPOT_DIRECTION is not significant when cutoff == 180
    // In this case, position x, y, z specifies direction

    // Exponent == 0 implies uniform light distribution
    glLightf( id, GL_SPOT_EXPONENT, 0 );

    // Cutoff == 180 means uniform point light source
    glLightf( id, GL_SPOT_CUTOFF, 180.0 );

    // Default attenuation values (only spotlight can modify these)
    glLightf( id, GL_CONSTANT_ATTENUATION, 1 );
    glLightf( id, GL_LINEAR_ATTENUATION, 0 );
    glLightf( id, GL_QUADRATIC_ATTENUATION, 0 );

    glPopMatrix();
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "glPopMatrix()" << endl;
#endif
#else           // DX Directional light
    D3DCOLORVALUE black;
    black.r = black.g = black.b = black.a = 0.0f;

    D3DLIGHT7  alight;
    ZeroMemory(&alight, sizeof(D3DLIGHT7));

    alight.dltType =  D3DLIGHT_DIRECTIONAL;
    alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
    alight.dcvAmbient  =  black ;
    alight.dcvSpecular = *(D3DCOLORVALUE *)(light->get_specular().get_data());

    alight.dvDirection = *(D3DVECTOR *)
                         (get_rel_forward( light, _current_projection_node, CS_yup_left).get_data());

    alight.dvRange =  D3DLIGHT_RANGE_MAX;
    alight.dvFalloff =  1.0f;

    alight.dvAttenuation0 = 1.0f;       // constant
    alight.dvAttenuation1 = 0.0f;       // linear
    alight.dvAttenuation2 = 0.0f;       // quadratic

    HRESULT res = _d3dDevice->SetLight(_cur_light_id, &alight);
//    _d3dDevice->LightEnable( _cur_light_id, TRUE );
//    _d3dDevice->SetRenderState( D3DRENDERSTATE_LIGHTING, TRUE );

#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::apply_light( Spotlight* light ) {
    // The light position will be relative to the current matrix, so
    // we have to know what the current matrix is.  Find a better
    // solution later.
#ifdef WBD_GL_MODE
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "glMatrixMode(GL_MODELVIEW)" << endl;
    dxgsg_cat.debug()
    << "glPushMatrix()" << endl;
    dxgsg_cat.debug()
    << "glLoadIdentity()" << endl;
#endif
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(LMatrix4f::convert_mat(_coordinate_system, CS_yup_left)
                  .get_data());

    GLenum id = get_light_id( _cur_light_id );
    Colorf black(0, 0, 0, 1);
    glLightfv(id, GL_AMBIENT, black.get_data());
    glLightfv(id, GL_DIFFUSE, light->get_color().get_data());
    glLightfv(id, GL_SPECULAR, light->get_specular().get_data());

    // Position needs to specify x, y, z, and w
    // w == 1 implies non-infinite position
    LPoint3f pos = get_rel_pos( light, _current_projection_node );
    LPoint4f fpos( pos[0], pos[1], pos[2], 1 );
    glLightfv( id, GL_POSITION, fpos.get_data() );

    glLightfv( id, GL_SPOT_DIRECTION,
               get_rel_forward( light, _current_projection_node,
                                _coordinate_system ).get_data() );
    glLightf( id, GL_SPOT_EXPONENT, light->get_exponent() );
    glLightf( id, GL_SPOT_CUTOFF,
              light->get_cutoff_angle() );
    glLightf( id, GL_CONSTANT_ATTENUATION,
              light->get_constant_attenuation() );
    glLightf( id, GL_LINEAR_ATTENUATION,
              light->get_linear_attenuation() );
    glLightf( id, GL_QUADRATIC_ATTENUATION,
              light->get_quadratic_attenuation() );

    glPopMatrix();
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "glPopMatrix()" << endl;
#endif
#else       // DX Spotlight
    D3DCOLORVALUE black;
    black.r = black.g = black.b = black.a = 0.0f;

    D3DLIGHT7  alight;
    ZeroMemory(&alight, sizeof(D3DLIGHT7));

    alight.dltType =  D3DLIGHT_SPOT;
    alight.dcvAmbient  =  black ;
    alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
    alight.dcvSpecular = *(D3DCOLORVALUE *)(light->get_specular().get_data());

    alight.dvPosition = *(D3DVECTOR *)
                        (get_rel_pos( light, _current_root_node ).get_data());

    alight.dvDirection = *(D3DVECTOR *)
                         (get_rel_forward( light, _current_root_node, _coordinate_system).get_data());

    alight.dvRange =  D3DLIGHT_RANGE_MAX;
    alight.dvFalloff =  1.0f;
    alight.dvTheta =  0.0f;
    alight.dvPhi =  light->get_cutoff_angle();

    alight.dvAttenuation0 = (D3DVALUE)light->get_constant_attenuation(); // constant
    alight.dvAttenuation1 = (D3DVALUE)light->get_linear_attenuation();   // linear
    alight.dvAttenuation2 = (D3DVALUE)light->get_quadratic_attenuation();// quadratic

    HRESULT res = _d3dDevice->SetLight(_cur_light_id, &alight);

#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::apply_light( AmbientLight* light ) {
    _cur_ambient_light = _cur_ambient_light + light->get_color();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_transform
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_transform(const TransformTransition *attrib) {
#ifndef NDEBUG
    if (dx_show_transforms) {

        bool lighting_was_enabled = _lighting_enabled;
        bool texturing_was_enabled = _texturing_enabled;
        enable_lighting(false);
        enable_texturing(false);

        typedef struct {
            D3DVALUE x,y,z;   // position
            D3DVALUE nx,ny,nz; // normal
            D3DCOLOR  diff;   // diffuse color
        }        VERTFORMAT;

        VERTFORMAT vert_buf[] = {
            {0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  D3DRGBA(1.0f, 0.0f, 0.0f, 1.0f)},      // red
            {3.0, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  D3DRGBA(1.0f, 0.0f, 0.0f, 1.0f)},       // red
            {0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  D3DRGBA(0.0f, 1.0f, 0.0f, 1.0f)},      // grn
            {0.0f, 3.0, 0.0f,  0.0f, -1.0f, 0.0f,  D3DRGBA(0.0f, 1.0f, 0.0f, 1.0f)},       // grn
            {0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f,  D3DRGBA(0.0f, 0.0f, 1.0f, 1.0f)},      // blu
            {0.0f, 0.0f, 3.0,  0.0f, -1.0f, 0.0f,  D3DRGBA(0.0f, 0.0f, 1.0f, 1.0f)},       // blu
        };

        HRESULT hr = _d3dDevice->DrawPrimitive(D3DPT_LINELIST, D3DFVF_DIFFUSE | D3DFVF_XYZ | D3DFVF_NORMAL,
                                  vert_buf, 6, NULL);
        TestDrawPrimFailure(DrawPrim,hr,_pDD,6,0);

        enable_lighting(lighting_was_enabled);
        enable_texturing(texturing_was_enabled);
    }
#endif

    _d3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD/*VIEW*/,
                             (LPD3DMATRIX) attrib->get_matrix().get_data());
    _bTransformIssued = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_matrix
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_tex_matrix(const TexMatrixTransition *attrib) {
    dxgsg_cat.fatal() << "DXGSG issue_tex_matrix unimplemented!!!";
    return;

#ifdef WBD_GL_MODE
#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
    << "glLoadMatrix(GL_TEXTURE): " << attrib->get_matrix() << endl;
#endif
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(attrib->get_matrix().get_data());
#else
    _d3dDevice->SetTransform( D3DTRANSFORMSTATE_TEXTURE0,
                              (LPD3DMATRIX)attrib->get_matrix().get_data());
#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_color
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_color(const ColorTransition *attrib) {
    
    bool bAttribOn=attrib->is_on();
    bool bIsReal = (bAttribOn ? attrib->is_real() : false);

    _issued_color_enabled = bAttribOn;

    // if an active unreal color is set, disable all color
    _enable_all_color = !(bAttribOn && (!bIsReal));  

    if(bAttribOn && bIsReal) {
        _issued_color = attrib->get_color();
        if(_color_transform_enabled || _alpha_transform_enabled) {
            transform_color(_issued_color, _issued_color_D3DCOLOR);
        } else {
            _issued_color_D3DCOLOR = Colorf_to_D3DCOLOR(_issued_color);
        }
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_color_transform
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_color_transform(const ColorMatrixTransition *attrib) {
    _current_color_mat = attrib->get_matrix();

    // couldnt we compare a single ptr instead of doing full comparison?
    // bugbug: the ColorMatrixTransition needs to be an On/Off transition 
    // so we dont have to do this comparison
    if (_current_color_mat == LMatrix4f::ident_mat()) { 
        _color_transform_enabled = false;
        if(_issued_color_enabled) {
             _issued_color_D3DCOLOR = Colorf_to_D3DCOLOR(_issued_color);
        }
    } else {
        _color_transform_enabled = true;
        if(_issued_color_enabled) {
             transform_color(_issued_color, _issued_color_D3DCOLOR);
        }
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_alpha_transform
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_alpha_transform(const AlphaTransformTransition *attrib) {
    _current_alpha_offset = attrib->get_offset();
    _current_alpha_scale = attrib->get_scale();

    if ((_current_alpha_offset == 0.0f) && (_current_alpha_scale == 1.0f)) {
        _alpha_transform_enabled = false;
        if(_issued_color_enabled) {
             _issued_color_D3DCOLOR = Colorf_to_D3DCOLOR(_issued_color);
        }
    } else {
        _alpha_transform_enabled = true;
        if(_issued_color_enabled) {
             transform_color(_issued_color, _issued_color_D3DCOLOR);
        }
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_texture(const TextureTransition *attrib) {
    if (attrib->is_on()) {
        enable_texturing(true);
        Texture *tex = attrib->get_texture();
        nassertv(tex != (Texture *)NULL);
        tex->apply(this);
    } else {
        enable_texturing(false);
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_tex_gen
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_tex_gen(const TexGenTransition *attrib) {
    dxgsg_cat.fatal() << "DXGSG issue_tex_gen unimplemented!!!";
    return;
#ifdef WBD_GL_MODE
    TexGenProperty::Mode mode = attrib->get_mode();

    switch (mode) {
        case TexGenProperty::M_none:
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_Q);
            glDisable(GL_TEXTURE_GEN_R);
            break;

        case TexGenProperty::M_texture_projector:
            {
                glEnable(GL_TEXTURE_GEN_S);
                glEnable(GL_TEXTURE_GEN_T);
                glEnable(GL_TEXTURE_GEN_Q);
                glEnable(GL_TEXTURE_GEN_R);
                const LMatrix4f &plane = attrib->get_plane();
                glTexGenfv(GL_S, GL_OBJECT_PLANE, plane.get_row(0).get_data());
                glTexGenfv(GL_T, GL_OBJECT_PLANE, plane.get_row(1).get_data());
                glTexGenfv(GL_R, GL_OBJECT_PLANE, plane.get_row(2).get_data());
                glTexGenfv(GL_Q, GL_OBJECT_PLANE, plane.get_row(3).get_data());
                glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
                glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
                glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
                glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            }
            break;

        case TexGenProperty::M_sphere_map:
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_GEN_Q);
            glDisable(GL_TEXTURE_GEN_R);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            break;

        default:
            dxgsg_cat.error()
            << "Unknown texgen mode " << (int)mode << endl;
            break;
    }
#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_material(const MaterialTransition *attrib) {
    if (attrib->is_on()) {
        const Material *material = attrib->get_material();
        nassertv(material != (const Material *)NULL);
        apply_material(material);
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_fog(const FogTransition *attrib) {

    if (attrib->is_on()) {
        enable_fog(true);
        Fog *fog = attrib->get_fog();
        nassertv(fog != (Fog *)NULL);
        fog->apply(this);
    } else {
        enable_fog(false);
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_render_mode
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_render_mode(const RenderModeTransition *attrib) {

    RenderModeProperty::Mode mode = attrib->get_mode();

    switch (mode) {
        case RenderModeProperty::M_filled:
            _d3dDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
            break;

        case RenderModeProperty::M_wireframe:
            _d3dDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
//    call_glLineWidth(attrib->get_line_width());
            break;

        default:
            dxgsg_cat.error()
            << "Unknown render mode " << (int)mode << endl;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::issue_light(const LightTransition *attrib ) {
  nassertv(attrib->get_default_dir() != TD_on);

  // Initialize the current ambient light total and currently enabled
  // light list
  _cur_ambient_light.set(0, 0, 0, 1);
  int i;
  for (i = 0; i < _max_lights; i++)
    _cur_light_enabled[i] = false;

  int num_enabled = 0;
  LightTransition::const_iterator li;
  for (li = attrib->begin(); li != attrib->end(); ++li) {
    Light *light = (*li).first;
    nassertv(light != (Light *)NULL);
    TransitionDirection dir = (*li).second;
 
    if (dir == TD_on) {
      num_enabled++;
      enable_lighting(true);

      _cur_light_id = -1;
            
      // Ambient lights don't require specific light ids
      // Simply add in the ambient contribution to the current total
      if (light->get_light_type() == AmbientLight::get_class_type()) {
        light->apply(this);
        // We need to indicate that no light id is necessary because
        // it's an ambient light
        _cur_light_id = -2;
      }

      // Check to see if this light has already been bound to an id
      for (i = 0; i < _max_lights; i++) {
        if (_available_light_ids[i] == light) {
          // Light has already been bound to an id, we only need
          // to enable the light, not apply it
          _cur_light_id = -2;
          if (enable_light(i, true))  _cur_light_enabled[i] = true;
          break;
        }
      }

      // See if there are any unbound light ids
      if (_cur_light_id == -1) {
        for (i = 0; i < _max_lights; i++) {
          if (_available_light_ids[i] == NULL) {
            _available_light_ids[i] = light;
            _cur_light_id = i;
            break;
          }
        }
      }

      // If there were no unbound light ids, see if we can replace
      // a currently unused but previously bound id
      if (_cur_light_id == -1) {
        for (i = 0; i < _max_lights; i++) {
          if (attrib->is_off(_available_light_ids[i])) {
            _available_light_ids[i] = light;
            _cur_light_id = i;
            break;
          }
        }
      }

      if (_cur_light_id >= 0) {
        if (enable_light(_cur_light_id, true))  _cur_light_enabled[_cur_light_id] = true;

        // We need to do something different for each type of light
        light->apply(this);
      } else if (_cur_light_id == -1) {
        dxgsg_cat.error()
          << "issue_light() - failed to bind light to id" << endl;
      }
    }
  }

  // Disable all unused lights
  for (i = 0; i < _max_lights; i++) {
    if (!_cur_light_enabled[i])
      enable_light(i, false);
  }
  
  // If no lights were enabled, disable lighting
  if (num_enabled == 0) {
    enable_lighting(false);
    enable_color_material(false);
  } else {
    call_dxLightModelAmbient(_cur_ambient_light);
    enable_color_material(true);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::reset_ambient
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
reset_ambient() {
    _lmodel_ambient += 2.0f;
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_color_blend       //
//       Access: Public, Virtual                                  //
//  Description:                                                  //
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_color_blend(const ColorBlendTransition *attrib) {
    
    ColorBlendProperty::Mode mode = attrib->get_mode();

    switch (mode) {
        case ColorBlendProperty::M_none:
            enable_blend(false);
            break;
        case ColorBlendProperty::M_multiply:
            enable_blend(true);
            call_dxBlendFunc(D3DBLEND_DESTCOLOR, D3DBLEND_ZERO);
            break;
        case ColorBlendProperty::M_add:
            enable_blend(true);
            call_dxBlendFunc(D3DBLEND_ONE, D3DBLEND_ONE);
            break;
        case ColorBlendProperty::M_multiply_add:
            enable_blend(true);
            call_dxBlendFunc(D3DBLEND_DESTCOLOR, D3DBLEND_ONE);
            break;
        case ColorBlendProperty::M_alpha:
            enable_blend(true);
            call_dxBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
            break;
        default:
            dxgsg_cat.error()
            << "Unknown color blend mode " << (int)mode << endl;
            break;
    }
}

void DXGraphicsStateGuardian::SetTextureBlendMode(TextureApplyProperty::Mode TexBlendMode,bool bCanJustEnable) {

/*class TextureApplyProperty {
  enum Mode {
    M_modulate,M_decal,M_blend,M_replace,M_add};
*/
    static D3DTEXTUREOP TexBlendColorOp1[/* TextureApplyProperty::Mode maxval*/ 10] =
    {D3DTOP_MODULATE,D3DTOP_BLENDTEXTUREALPHA,D3DTOP_MODULATE,D3DTOP_SELECTARG1,D3DTOP_ADD};

    //if bCanJustEnable, then we only need to make sure ColorOp is turned on and set properly
    if (bCanJustEnable && (TexBlendMode==_CurTexBlendMode)) {
        // just reset COLOROP 0 to enable pipeline, rest is already set properly
        _d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, TexBlendColorOp1[TexBlendMode] );
        return;
    }

    _d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, TexBlendColorOp1[TexBlendMode] );

    switch (TexBlendMode) {

        case TextureApplyProperty::M_modulate:
            // emulates GL_MODULATE glTexEnv mode
            // want to multiply tex-color*pixel color to emulate GL modulate blend (see glTexEnv)
            _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

            break;
        case TextureApplyProperty::M_decal:
            // emulates GL_DECAL glTexEnv mode
            _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

            break;
        case TextureApplyProperty::M_replace:
            _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            break;
        case TextureApplyProperty::M_add:
            _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

            // since I'm making up 'add' mode, use modulate.  "adding" alpha never makes sense right?
            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

            break;
        case TextureApplyProperty::M_blend:
            dxgsg_cat.error()
            << "Impossible to emulate GL_BLEND in DX exactly " << (int) TexBlendMode << endl;
/*
           // emulate GL_BLEND glTexEnv

           GL requires 2 independent operations on 3 input vars for this mode
           DX texture pipeline requires re-using input of last stage on each new op, so I dont think
           exact emulation is possible
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

           need to SetTexture(1,tex) also
           _d3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE ); wrong
           _d3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
           _d3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TFACTOR );

           _d3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
           _d3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
*/


            break;
        default:
            dxgsg_cat.error() << "Unknown texture blend mode " << (int) TexBlendMode << endl;
            break;
    }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::enable_texturing
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
enable_texturing(bool val) {
//  if (_texturing_enabled == val) {  // this check is mostly for internal gsg calls, panda already screens out redundant state changes
//        return;
//  }

    _texturing_enabled = val;

//  assert(_pCurTexContext!=NULL);  we're definitely called with it NULL for both true and false
//  I'm going to allow enabling texturing even if no tex has been set yet, seems to cause no probs

    if (val == FALSE) {
        _d3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);
    } else {
          SetTextureBlendMode(_CurTexBlendMode,TRUE);
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_texture_apply
//       Access: Public, Virtual
//  Description: handles texture transition (i.e. filter modes, etc) changes
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_texture_apply(const TextureApplyTransition *attrib) {

    _CurTexBlendMode = attrib->get_mode();

    if (!_texturing_enabled) {
        return;
    }

    SetTextureBlendMode(_CurTexBlendMode,FALSE);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_color_mask
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_color_mask(const ColorMaskTransition *attrib) {
    dxgsg_cat.fatal() << "DXGSG issue_color_mask unimplemented (not implementable on DX7)!!!";
    return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_depth_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_depth_test(const DepthTestTransition *attrib) {
    

    DepthTestProperty::Mode mode = attrib->get_mode();

    if (mode == DepthTestProperty::M_none) {
        _depth_test_enabled = false;
        _d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
    } else {
        _depth_test_enabled = true;
        _d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
        _d3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, get_depth_func_type(mode));
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_depth_write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_depth_write(const DepthWriteTransition *attrib) {
    _d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, attrib->is_on());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_stencil
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_stencil(const StencilTransition *attrib) {

  StencilProperty::Mode mode = attrib->get_mode();

#if 1
  if (mode != StencilProperty::M_none) {
    dxgsg_cat.error() << "stencil buffering unimplemented for DX GSG renderer!!!\n";
    // to implement stenciling, need to change wdxGraphicsWindow to create a stencil
    // z-buffer or maybe do a SetRenderTarget on a new zbuffer
  }

#else

    if (mode == StencilProperty::M_none) {
        enable_stencil_test(false);

    } else {
        enable_stencil_test(true);
        _d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFUNC, get_stencil_func_type(mode));
        _d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILPASS, get_stencil_action_type(attrib->get_action()));
        _d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFAIL, get_stencil_action_type(attrib->get_action()));
        _d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILZFAIL, get_stencil_action_type(attrib->get_action()));
    }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_cull_transition
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_cull_face(const CullFaceTransition *attrib) {
    
#ifndef NDEBUG
    if(dx_force_backface_culling!=0) {
        return;  // leave as initially set
    }
#endif

    CullFaceProperty::Mode mode = attrib->get_mode();

    switch (mode) {
        case CullFaceProperty::M_cull_none:
            _d3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
            break;
        case CullFaceProperty::M_cull_clockwise:
            _d3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
            break;
        case CullFaceProperty::M_cull_counter_clockwise:
            _d3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
            break;
        case CullFaceProperty::M_cull_all:
            dxgsg_cat.warning() << "M_cull_all is invalid for DX GSG renderer, using CULL_CCW \n";
            _d3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
            break;
        default:
            dxgsg_cat.error()
            << "invalid cull face mode " << (int)mode << endl;
            break;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_clip_plane
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_clip_plane(const ClipPlaneTransition *attrib) {
  nassertv(attrib->get_default_dir() != TD_on);
    
  // Initialize the currently enabled clip plane list
  int i;
  for (i = 0; i < _max_clip_planes; i++)
    _cur_clip_plane_enabled[i] = false;
  
  int num_enabled = 0;
  ClipPlaneTransition::const_iterator pi;
  for (pi = attrib->begin(); pi != attrib->end(); ++pi) {
    PlaneNode *plane_node;
    DCAST_INTO_V(plane_node, (*pi).first);
    nassertv(plane_node != (PlaneNode *)NULL);
    TransitionDirection dir = (*pi).second;

    if (dir == TD_on) {
      _cur_clip_plane_id = -1;
      num_enabled++;
      
      // Check to see if this clip plane has already been bound to an id
      for (i = 0; i < _max_clip_planes; i++) {
        if (_available_clip_plane_ids[i] == plane_node) {
          // Clip plane has already been bound to an id, we only need
          // to enable the clip plane, not apply it
          _cur_clip_plane_id = -2;
          enable_clip_plane(i, true);
          _cur_clip_plane_enabled[i] = true;
          break;
        }
      }
      
      // See if there are any unbound clip plane ids
      if (_cur_clip_plane_id == -1) {
        for (i = 0; i < _max_clip_planes; i++) {
          if (_available_clip_plane_ids[i] == NULL) {
            _available_clip_plane_ids[i] = plane_node;
            _cur_clip_plane_id = i;
            break;
          }
        }
      }
      
      // If there were no unbound clip plane ids, see if we can replace
      // a currently unused but previously bound id
      if (_cur_clip_plane_id == -1) {
        for (i = 0; i < _max_clip_planes; i++) {
          if (attrib->is_off(_available_clip_plane_ids[i])) {
            _available_clip_plane_ids[i] = plane_node;
            _cur_clip_plane_id = i;
            break;
          }
        }
      }
      
      if (_cur_clip_plane_id >= 0) {
        enable_clip_plane(_cur_clip_plane_id, true);
        _cur_clip_plane_enabled[_cur_clip_plane_id] = true;
        const Planef clip_plane = plane_node->get_plane();
        
        D3DVALUE equation[4];
        equation[0] = clip_plane._a;
        equation[1] = clip_plane._b;
        equation[2] = clip_plane._c;
        equation[3] = clip_plane._d;
        _d3dDevice->SetClipPlane(_cur_clip_plane_id, equation);
        
      } else if (_cur_clip_plane_id == -1) {
        dxgsg_cat.error()
          << "issue_clip_plane() - failed to bind clip plane to id" << endl;
      }
    }
  }

  // Disable all unused clip planes
  for (i = 0; i < _max_clip_planes; i++) {
    if (!_cur_clip_plane_enabled[i])
      enable_clip_plane(i, false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_transparency
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_transparency(const TransparencyTransition *attrib ) {

    TransparencyProperty::Mode mode = attrib->get_mode();

    switch (mode) {
        case TransparencyProperty::M_none:
//          enable_multisample_alpha_one(false);
//          enable_multisample_alpha_mask(false);
            enable_blend(false);
            enable_alpha_test(false);
            break;
        case TransparencyProperty::M_alpha:
        case TransparencyProperty::M_alpha_sorted:
            // Should we really have an "alpha" and an "alpha_sorted" mode,
            // like Performer does?  (The difference is that "alpha" is with
            // the write to the depth buffer disabled.)  Or should we just use
            // the separate depth write transition to control this?  Doing it
            // implicitly requires a bit more logic here and in the state
            // management; for now we require the user to explicitly turn off
            // the depth write.
//          enable_multisample_alpha_one(false);
//          enable_multisample_alpha_mask(false);
            enable_blend(true);
            enable_alpha_test(false);
            call_dxBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
            break;
        case TransparencyProperty::M_multisample:
//          enable_multisample_alpha_one(true);
//          enable_multisample_alpha_mask(true);
            enable_blend(false);
            enable_alpha_test(false);
            break;
        case TransparencyProperty::M_multisample_mask:
//          enable_multisample_alpha_one(false);
//          enable_multisample_alpha_mask(true);
            enable_blend(false);
            enable_alpha_test(false);
            break;
        case TransparencyProperty::M_binary:
//          enable_multisample_alpha_one(false);
//          enable_multisample_alpha_mask(false);
            enable_blend(false);
            enable_alpha_test(true);
            call_dxAlphaFunc(D3DCMP_EQUAL, 1);
            break;
        default:
            dxgsg_cat.error()
            << "invalid transparency mode " << (int)mode << endl;
            break;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_linesmooth
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_linesmooth(const LinesmoothTransition *attrib) {
    enable_line_smooth(attrib->is_on());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::wants_normals
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool DXGraphicsStateGuardian::
wants_normals() const {
    return (_lighting_enabled || _normals_enabled);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::wants_texcoords
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool DXGraphicsStateGuardian::
wants_texcoords() const {
    return _texturing_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::begin_decal
//       Access: Public, Virtual
//  Description: This will be called to initiate decaling mode.  It is
//               passed the pointer to the GeomNode that will be the
//               destination of the decals, which it is expected that
//               the GSG will render normally; subsequent geometry
//               rendered up until the next call of end_decal() should
//               be rendered as decals of the base_geom.
//
//               The transitions wrapper is the current state as of the
//               base geometry node.  It may or may not be modified by
//               the GSG to reflect whatever rendering state is
//               necessary to render the decals properly.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
begin_decal(GeomNode *base_geom, AllTransitionsWrapper &attrib) {
    nassertv(base_geom != (GeomNode *)NULL);

    _decal_level++;

#ifndef DISABLE_DECALING

 #ifndef DISABLE_POLYGON_OFFSET_DECALING
    if (dx_decal_type == GDT_offset) {
 #define POLYGON_OFFSET_MULTIPLIER 2

        // note: zbias seems inconsitently supported.  may be possible to fake it by
        //       adding (delta * element[4,3]) to element [3,3] of a regular matrix
        //       need to test this and see if to offers perf improvement over blend-based decaling

        nassertv(POLYGON_OFFSET_MULTIPLIER*_decal_level < 16);  // 16 is the max allowed zbias

        // Just draw the base geometry normally.
        base_geom->draw(this);
        _d3dDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, POLYGON_OFFSET_MULTIPLIER * _decal_level); // _decal_level better not be higher than 8!
    } else
 #endif

    {
        if (_decal_level > 1) {
            base_geom->draw(this);  // If we're already decaling, just draw the geometry.
        } else {
            // need to save current xform matrix in case it is changed during subrendering, so subsequent decal draws use same xform
            _bTransformIssued = false;
            _d3dDevice->GetTransform( D3DTRANSFORMSTATE_WORLD, &_SavedTransform);

            // First turn off writing the depth buffer to render the base geometry.
            _d3dDevice->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, (DWORD *)&_depth_write_enabled);  //save cur val
            _d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

            // It is also important to update the current state to
            // indicate the depth buffer write is off, so that future
            // geometry will render correctly.
            DepthWriteTransition *dwa = new DepthWriteTransition(DepthWriteTransition::off());
            attrib.set_transition(dwa);

            // Now render the base geometry.
            base_geom->draw(this);

            // Render all of the decal geometry, too.  We'll keep the depth
            // buffer write off during this.
        }
    }
#else
            base_geom->draw(this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::end_decal
//       Access: Public, Virtual
//  Description: This will be called to terminate decaling mode.  It
//               is passed the same base_geom that was passed to
//               begin_decal().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
end_decal(GeomNode *base_geom) {
    _decal_level--;

#ifdef DISABLE_DECALING
    return;
#endif

    nassertv(base_geom != (GeomNode *)NULL);
//  nassertv(_decal_level >= 1);

#ifndef DISABLE_POLYGON_OFFSET_DECALING
    if (dx_decal_type == GDT_offset) {
        // Restore the Zbias offset.
        _d3dDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, POLYGON_OFFSET_MULTIPLIER * _decal_level); // _decal_level better not be higher than 8!
    } else
#endif
    {  // for GDT_mask
        if (_decal_level == 0) {
            // Now we need to re-render the base geometry with the depth write
            // on and the color mask off, so we update the depth buffer
            // properly.
            bool was_textured = _texturing_enabled;
            bool was_blend = _blend_enabled;
            D3DBLEND old_blend_source_func = _blend_source_func;
            D3DBLEND old_blend_dest_func = _blend_dest_func;

            // Enable the writing to the depth buffer.
            _d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);


            // Disable the writing to the color buffer, however we have to
            // do this.  (I don't think this is possible in DX without blending.)
            if (dx_decal_type == GDT_blend) {
                // Expensive.
                enable_blend(true);
                call_dxBlendFunc(D3DBLEND_ZERO, D3DBLEND_ONE);
            }

#if(DIRECT3D_VERSION < 0x700)
            else {  // dx7 doesn't support planemask rstate
                // note: not saving current planemask val, assumes this is always all 1's.  should be ok
                _d3dDevice->SetRenderState(D3DRENDERSTATE_PLANEMASK,0x0);  // note PLANEMASK is supposedly obsolete for DX7
            }
#endif
// Note: For DX8, use D3DRS_COLORWRITEENABLE  (check D3DPMISCCAPS_COLORWRITEENABLE first)

            // No need to have texturing on for this.
            enable_texturing(false);

            // if current xform has changed, reset to saved xform
            if(_bTransformIssued) 
                _d3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &_SavedTransform);

            base_geom->draw(this);

            // Finally, restore the depth write and color mask states to the
            // way they're supposed to be.

            if (dx_decal_type == GDT_blend) {
                enable_blend(was_blend);
                if (was_blend)
                    call_dxBlendFunc(old_blend_source_func, old_blend_dest_func);
            }
#if(DIRECT3D_VERSION < 0x700)
            else {
                _d3dDevice->SetRenderState(D3DRENDERSTATE_PLANEMASK,0xFFFFFFFF);  // this is unlikely to work due to poor driver support
            }
#endif

            enable_texturing(was_textured);
            _d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, _depth_write_enabled);
        }
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::compute_distance_to
//       Access: Public, Virtual
//  Description: This function may only be called during a render
//               traversal; it will compute the distance to the
//               indicated point, assumed to be in modelview
//               coordinates, from the camera plane.
////////////////////////////////////////////////////////////////////
INLINE float DXGraphicsStateGuardian::
compute_distance_to(const LPoint3f &point) const {
    // In the case of a DXGraphicsStateGuardian, we know that the
    // modelview matrix already includes the relative transform from the
    // camera, as well as a to-y-up conversion.  Thus, the distance to
    // the camera plane is simply the +z distance.  (negative of gl compute_distance_to,
    // since d3d uses left-hand coords)

    return point[2];
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::set_draw_buffer
//       Access: Protected
//  Description: Sets up the glDrawBuffer to render into the buffer
//               indicated by the RenderBuffer object.  This only sets
//               up the color bits; it does not affect the depth,
//               stencil, accum layers.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
set_draw_buffer(const RenderBuffer &rb) {
    dxgsg_cat.fatal() << "DX set_draw_buffer unimplemented!!!";
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
//     Function: DXGraphicsStateGuardian::set_read_buffer
//       Access: Protected
//  Description: Vestigial analog of glReadBuffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
set_read_buffer(const RenderBuffer &rb) {

    if(rb._buffer_type & RenderBuffer::T_front) {
            _cur_read_pixel_buffer=RenderBuffer::T_front;
    } else  if(rb._buffer_type & RenderBuffer::T_back) {
            _cur_read_pixel_buffer=RenderBuffer::T_back;
    } else {
            dxgsg_cat.error() << "Invalid or unimplemented Argument to set_read_buffer!\n";
    }
    return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_texture_wrap_mode
//       Access: Protected
//  Description: Maps from the Texture's internal wrap mode symbols to
//               GL's.
////////////////////////////////////////////////////////////////////
INLINE D3DTEXTUREADDRESS DXGraphicsStateGuardian::
get_texture_wrap_mode(Texture::WrapMode wm) const {

    if (wm == Texture::WM_clamp)
        return D3DTADDRESS_CLAMP;
    else if (wm != Texture::WM_repeat) {
#ifdef _DEBUG
        dxgsg_cat.error() << "Invalid or Unimplemented Texture::WrapMode value!\n";
#endif
    }

    return D3DTADDRESS_WRAP;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_depth_func_type
//       Access: Protected
//  Description: Maps from the depth func modes to gl version
////////////////////////////////////////////////////////////////////
INLINE D3DCMPFUNC DXGraphicsStateGuardian::
get_depth_func_type(DepthTestProperty::Mode m) const {
    switch (m) {
        case DepthTestProperty::M_never: return D3DCMP_NEVER;
        case DepthTestProperty::M_less: return D3DCMP_LESS;
        case DepthTestProperty::M_equal: return D3DCMP_EQUAL;
        case DepthTestProperty::M_less_equal: return D3DCMP_LESSEQUAL;
        case DepthTestProperty::M_greater: return D3DCMP_GREATER;
        case DepthTestProperty::M_not_equal: return D3DCMP_NOTEQUAL;
        case DepthTestProperty::M_greater_equal: return D3DCMP_GREATEREQUAL;
        case DepthTestProperty::M_always: return D3DCMP_ALWAYS;
    }
    dxgsg_cat.error()
    << "Invalid DepthTestProperty::Mode value" << endl;
    return D3DCMP_LESS;
}

#if 0
// ifdef out until stencil zbuf creation works in wdxGraphicsWindow.c

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_stencil_func_type
//       Access: Protected
//  Description: Maps from the stencil func modes to dx version
////////////////////////////////////////////////////////////////////
INLINE D3DCMPFUNC DXGraphicsStateGuardian::
get_stencil_func_type(StencilProperty::Mode m) const {
    switch (m) {
        case StencilProperty::M_never: return D3DCMP_NEVER;
        case StencilProperty::M_less: return D3DCMP_LESS;
        case StencilProperty::M_equal: return D3DCMP_EQUAL;
        case StencilProperty::M_less_equal: return D3DCMP_LESSEQUAL;
        case StencilProperty::M_greater: return D3DCMP_GREATER;
        case StencilProperty::M_not_equal: return D3DCMP_NOTEQUAL;
        case StencilProperty::M_greater_equal: return D3DCMP_GREATEREQUAL;
        case StencilProperty::M_always: return D3DCMP_ALWAYS;
    }
    dxgsg_cat.error()
    << "Invalid StencilProperty::Mode value" << endl;
    return D3DCMP_LESS;
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_stencil_action_type
//       Access: Protected
//  Description: Maps from the stencil action modes to dx version
////////////////////////////////////////////////////////////////////
INLINE D3DSTENCILOP DXGraphicsStateGuardian::
get_stencil_action_type(StencilProperty::Action a) const {
    switch (a) {
        case StencilProperty::A_keep: return D3DSTENCILOP_KEEP;
        case StencilProperty::A_zero: return D3DSTENCILOP_ZERO;
        case StencilProperty::A_replace: return D3DSTENCILOP_REPLACE;
        case StencilProperty::A_increment: return D3DSTENCILOP_INCR;
        case StencilProperty::A_decrement: return D3DSTENCILOP_DECR;
        case StencilProperty::A_invert: return D3DSTENCILOP_INVERT;
    }
    dxgsg_cat.error()
    << "Invalid StencilProperty::Action value" << endl;
    return D3DSTENCILOP_KEEP;
}

#endif

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_fog_mode_type
//       Access: Protected
//  Description: Maps from the fog types to gl version
////////////////////////////////////////////////////////////////////
INLINE D3DFOGMODE DXGraphicsStateGuardian::
get_fog_mode_type(Fog::Mode m) const {
    switch (m) {
        case Fog::M_linear: return D3DFOG_LINEAR;
        case Fog::M_exponential: return D3DFOG_EXP;
        case Fog::M_exponential_squared: return D3DFOG_EXP2;
    }
    dxgsg_cat.error() << "Invalid Fog::Mode value" << endl;
    return D3DFOG_EXP;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::free_pointers
//       Access: Public
//  Description: Frees some memory that was explicitly allocated
//               within the dxgsg.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
free_pointers() {
    if (_light_enabled != (bool *)NULL) {
        delete[] _light_enabled;
        _light_enabled = (bool *)NULL;
    }
    if (_cur_light_enabled != (bool *)NULL) {
        delete[] _cur_light_enabled;
        _cur_light_enabled = (bool *)NULL;
    }
    if (_clip_plane_enabled != (bool *)NULL) {
        delete[] _clip_plane_enabled;
        _clip_plane_enabled = (bool *)NULL;
    }
    if (_cur_clip_plane_enabled != (bool *)NULL) {
        delete[] _cur_clip_plane_enabled;
        _cur_clip_plane_enabled = (bool *)NULL;
    }

    if (_pTexPixFmts != NULL) {
        delete [] _pTexPixFmts;
        _pTexPixFmts = NULL;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::save_frame_buffer
//       Access: Public
//  Description: Saves the indicated planes of the frame buffer
//               (within the indicated display region) and returns it
//               in some meaningful form that can be restored later
//               via restore_frame_buffer().  This is a helper
//               function for push_frame_buffer() and
//               pop_frame_buffer().
////////////////////////////////////////////////////////////////////
PT(SavedFrameBuffer) DXGraphicsStateGuardian::
save_frame_buffer(const RenderBuffer &buffer,
                  CPT(DisplayRegion) dr) {

    dxgsg_cat.error() << "save_frame_buffer unimplemented!!\n";
    return NULL;

#if 0
    DXSavedFrameBuffer *sfb = new DXSavedFrameBuffer(buffer, dr);

    if (buffer._buffer_type & RenderBuffer::T_depth) {
        // Save the depth buffer.
        sfb->_depth =
        new PixelBuffer(PixelBuffer::depth_buffer(dr->get_pixel_width(),
                                                  dr->get_pixel_height()));
        copy_pixel_buffer(sfb->_depth, dr, buffer);
    }

    if (buffer._buffer_type & RenderBuffer::T_back) {
        // Save the color buffer.
        sfb->_back_rgba = new Texture;
        copy_texture(sfb->_back_rgba->prepare(this), dr, buffer);
    }

    return sfb;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::restore_frame_buffer
//       Access: Public
//  Description: Restores the frame buffer that was previously saved.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
restore_frame_buffer(SavedFrameBuffer *frame_buffer) {

    dxgsg_cat.error() << "restore_frame_buffer unimplemented!!\n";
    return;

#if 0
    DXSavedFrameBuffer *sfb = DCAST(DXSavedFrameBuffer, frame_buffer);

    if (sfb->_back_rgba != (Texture *)NULL &&
        (sfb->_buffer._buffer_type & RenderBuffer::T_back) != 0) {
        // Restore the color buffer.
        draw_texture(sfb->_back_rgba->prepare(this),
                     sfb->_display_region, sfb->_buffer);
    }

    if (sfb->_depth != (PixelBuffer *)NULL &&
        (sfb->_buffer._buffer_type & RenderBuffer::T_depth) != 0) {
        // Restore the depth buffer.
        draw_pixel_buffer(sfb->_depth, sfb->_display_region, sfb->_buffer);
    }
#endif
}

// factory and type stuff

GraphicsStateGuardian *DXGraphicsStateGuardian::
make_DXGraphicsStateGuardian(const FactoryParams &params) {
    GraphicsStateGuardian::GsgWindow *win_param;
    if (!get_param_into(win_param, params)) {
        dxgsg_cat.error()
        << "No window specified for gsg creation!" << endl;
        return NULL;
    }

    GraphicsWindow *win = win_param->get_window();
    return new DXGraphicsStateGuardian(win);
}

TypeHandle DXGraphicsStateGuardian::get_type(void) const {
    return get_class_type();
}

TypeHandle DXGraphicsStateGuardian::get_class_type(void) {
    return _type_handle;
}

void DXGraphicsStateGuardian::init_type(void) {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "DXGraphicsStateGuardian",
                  GraphicsStateGuardian::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: dx_cleanup
//  Description: Clean up the DirectX environment.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
dx_cleanup(bool bRestoreDisplayMode,bool bAtExitFnCalled) {
  static bool bAtExitFnEverCalled=false;

    bAtExitFnEverCalled = (bAtExitFnEverCalled || bAtExitFnCalled);

    // for now, I can't trust any of the ddraw/d3d releases during atexit(),
    // so just return directly.  maybe revisit this later, if have problems
    // restarting d3d/ddraw after one of these uncleaned-up exits
    if(bAtExitFnEverCalled)
      return;

    ULONG refcnt;

    // unsafe to do the D3D releases after exit() called, since DLL_PROCESS_DETACH
    // msg already delivered to d3d.dll and it's unloaded itself
    if(!bAtExitFnEverCalled) {

        // these 2 calls release ddraw surfaces and vbuffers.  unsafe unless not on exit
        release_all_textures();
        release_all_geoms();


        // Do a safe check for releasing the D3DDEVICE. RefCount should be zero.
        // if we're called from exit(), _d3dDevice may already have been released
        if (_d3dDevice!=NULL) {
            if (0 < (refcnt =  _d3dDevice->Release())) {
                dxgsg_cat.error() << "dx_cleanup - d3dDevice reference count = " <<  refcnt << ", should be zero!\n";
            }
        }
    
        _d3dDevice = NULL;  // clear the pointer in the Gsg
    
        // Release the DDraw and D3D objects used by the app
        RELEASE(_zbuf);
        RELEASE(_back);
        RELEASE(_pri);
    
        RELEASE(_d3d);
    }

    // for some reason, DLL_PROCESS_DETACH has not yet been sent to ddraw, so we can still call its fns

    // Do a safe check for releasing DDRAW. RefCount should be zero.
    if (_pDD!=NULL) {
        if(bRestoreDisplayMode) {
          HRESULT hr = _pDD->RestoreDisplayMode(); 
          if(dxgsg_cat.is_spam())
                dxgsg_cat.spam() << "dx_cleanup -  Restoring original desktop DisplayMode\n";
          if(FAILED(hr)) {
                dxgsg_cat.error() << "dx_cleanup -  RestoreDisplayMode failed, hr = " << ConvD3DErrorToString(hr) << endl;
          }
        }

        refcnt = _pDD->Release();

        if(bAtExitFnCalled) {
            // refcnt may be > 1

           while (refcnt>1) {
               refcnt = _pDD->Release();
           }
        } else {
            if(refcnt>0)
                dxgsg_cat.error() << "dx_cleanup - IDDraw Obj reference count = " << refcnt << ", should be zero!\n";
        }

        _pDD  = NULL;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: dx_setup_after_resize
//  Description: Recreate the back buffer and zbuffers at the new size
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
dx_setup_after_resize(RECT viewrect, HWND mwindow) {
    if (_back == NULL) // nothing created yet
        return;

    // for safety, need some better error-cleanup here
    assert((_pri!=NULL) && (_back!=NULL) && (_zbuf!=NULL));

    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_back);
    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_zbuf);

    _back->GetSurfaceDesc(&ddsd_back);
    _zbuf->GetSurfaceDesc(&ddsd_zbuf);

#if 0
    DWORD refcnt;
    refcnt = _zbuf->Release();  _zbuf = NULL;
    dxgsg_cat.error() << "zbuf refcnt= "<<refcnt <<endl;
    refcnt = _back->Release();  _back = NULL;
    dxgsg_cat.error() << "back refcnt= "<<refcnt <<endl;
    refcnt = _pri->Release();  _pri = NULL;
    dxgsg_cat.error() << "pri refcnt= "<<refcnt <<endl;
#endif

    RELEASE(_zbuf);
    RELEASE(_back);
    RELEASE(_pri);

    assert((_pri == NULL) && (_back == NULL) && (_zbuf == NULL));
    _view_rect = viewrect;

    DWORD renderWid = _view_rect.right - _view_rect.left;
    DWORD renderHt = _view_rect.bottom - _view_rect.top;

    ddsd_back.dwWidth  = ddsd_zbuf.dwWidth = renderWid;
    ddsd_back.dwHeight = ddsd_zbuf.dwHeight = renderHt;

    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);

    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    ddsd.dwFlags        = DDSD_CAPS;

    PRINTVIDMEM(_pDD,&ddsd.ddsCaps,"resize primary surf");
    HRESULT hr;

    if (FAILED(hr = _pDD->CreateSurface( &ddsd, &_pri, NULL ))) {
        dxgsg_cat.fatal() << "resize() - CreateSurface failed for primary : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    if(!dx_full_screen) {
        // Create a clipper object which handles all our clipping for cases when
        // our window is partially obscured by other windows.
        LPDIRECTDRAWCLIPPER Clipper;
    
        if (FAILED(hr = _pDD->CreateClipper( 0, &Clipper, NULL ))) {
            dxgsg_cat.fatal()
            << "CreateClipper after resize failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        // Associate the clipper with our window. Note that, afterwards, the
        // clipper is internally referenced by the primary surface, so it is safe
        // to release our local reference to it.
        Clipper->SetHWnd( 0, mwindow );
        _pri->SetClipper( Clipper );
        Clipper->Release();
    }

    // Recreate the backbuffer. (might want to handle failure due to running out of video memory)

    ddsd_back.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;  // just to make sure
    ddsd_back.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

    PRINTVIDMEM(_pDD,&ddsd_back.ddsCaps,"resize backbuffer surf");

    if (FAILED(hr = _pDD->CreateSurface( &ddsd_back, &_back, NULL ))) {
        dxgsg_cat.fatal() << "resize() - CreateSurface failed for backbuffer : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    PRINTVIDMEM(_pDD,&ddsd_back.ddsCaps,"resize zbuffer surf");

    // Recreate and attach a z-buffer.
    if (FAILED(hr = _pDD->CreateSurface( &ddsd_zbuf, &_zbuf, NULL ))) {
        dxgsg_cat.fatal() << "resize() - CreateSurface failed for Z buffer: result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    // Attach the z-buffer to the back buffer.
    if ((hr = _back->AddAttachedSurface( _zbuf ) ) != DD_OK) {
        dxgsg_cat.fatal()
        << "resize() - AddAttachedSurface failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    if ((hr = _d3dDevice->SetRenderTarget(_back,0x0) ) != DD_OK) {
        dxgsg_cat.fatal()
        << "resize() - SetRenderTarget failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    // Create the viewport
    D3DVIEWPORT7 vp = { 0, 0, renderWid, renderHt, 0.0f, 1.0f};
    hr = _d3dDevice->SetViewport( &vp );
    if (hr != DD_OK) {
        dxgsg_cat.fatal()
        << "SetViewport failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }
}

bool refill_tex_callback(TextureContext *tc,void *void_dxgsg_ptr) {
     DXTextureContext *dtc = DCAST(DXTextureContext, tc);
//   DXGraphicsStateGuardian *dxgsg = (DXGraphicsStateGuardian *)void_dxgsg_ptr; not needed?

     // Re-fill the contents of textures and vertex buffers
     // which just got restored now.
     HRESULT hr=dtc->FillDDSurfTexturePixels();
     return hr==S_OK;
}

HRESULT DXGraphicsStateGuardian::RestoreAllVideoSurfaces(void) {
  // BUGBUG: this should also restore vertex buffer contents when they are implemented
  // You will need to destroy and recreate
  // optimized vertex buffers however, restoring is not enough.

  HRESULT hr;

  // note: could go through and just restore surfs that return IsLost() true
  // apparently that isnt as reliable w/some drivers tho
  if (FAILED(hr = _pDD->RestoreAllSurfaces() )) {
        dxgsg_cat.fatal() << "RestoreAllSurfs failed : result = " << ConvD3DErrorToString(hr) << endl;
    exit(1);
  }

  // cant access template in libpanda.dll directly due to vc++ limitations, use traverser to get around it
  traverse_prepared_textures(refill_tex_callback,this);

  if(dxgsg_cat.is_debug())
      dxgsg_cat.debug() << "restore and refill of video surfaces complete...\n";
  return S_OK;
}


////////////////////////////////////////////////////////////////////
//     Function: show_frame
//       Access:
//       Description:   Repaint primary buffer from back buffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::show_frame(void) {
  if(_pri==NULL)
    return;

  DO_PSTATS_STUFF(PStatTimer timer(_win->_swap_pcollector));  // this times just the flip, so it must go here in dxgsg, instead of wdxdisplay, which would time the whole frame

  if(dx_full_screen) {
    show_full_screen_frame();
  } else {
    show_windowed_frame();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: show_full_screen_frame
//       Access:
//       Description:   Repaint primary buffer from back buffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::show_full_screen_frame(void) {
  HRESULT hr;
  
  // Flip the front and back buffers, to make what we just rendered
  // visible.

  DWORD dwFlipFlags = DDFLIP_WAIT;

  if (!dx_sync_video) {
    // If the user indicated via Config that we shouldn't wait for
    // video sync, then don't wait (if the hardware supports this).
    // This will introduce visible artifacts like tearing, and may
    // cause the frame rate to grow excessively (and pointlessly)
    // high, starving out other processes.
    dwFlipFlags |= DDFLIP_NOVSYNC;
//  dwFlipFlags = DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC;
  }

  // bugbug: dont we want triple buffering instead of wasting time
  // waiting for vsync?
  hr = _pri->Flip( NULL, dwFlipFlags);

  if(FAILED(hr)) {
    if((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      CheckCooperativeLevel();
    } else {
      dxgsg_cat.error() << "show_frame() - Flip failed w/unexpected error code: " << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }

}

////////////////////////////////////////////////////////////////////
//     Function: show_windowed_frame
//       Access: Public
//  Description: Repaint primary buffer from back buffer (windowed
//               mode only)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::show_windowed_frame(void) {
  HRESULT hr;

  DX_DECLARE_CLEAN(DDBLTFX, bltfx);

  if (dx_sync_video) {
    // Wait for the video refresh *before* we blt the rendered image
    // onto the window.  This will (a) prevent the "tearing" of the
    // image that would occur if only part of the image happened to be
    // copied into the window before the video refresh occurred, and
    // (b) prevent our frame rate from going excessively (and
    // pointlessly) higher than our video refresh rate, starving out
    // other processes.

    // Unfortunately, when the system is even lightly loaded, this
    // wait call sometimes appears to wait through multiple frames
    // before returning, causing our frame rate to be unreasonably low
    // and erratic.  There doesn't appear to be any way to prevent
    // this behavior; thus, we allow the user to avoid this wait,
    // based on the Config settings.

    bltfx.dwDDFX |= DDBLTFX_NOTEARING;  // hmm, does any driver actually recognize this flag?
  }

  hr = _pri->Blt( &_view_rect, _back,  NULL, DDBLT_DDFX | DDBLT_WAIT, &bltfx );

  if (dx_sync_video) {
    HRESULT hr = _pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
    if(hr != DD_OK) {
      dxgsg_cat.error() << "WaitForVerticalBlank() failed : " << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }

  if(FAILED(hr)) {
    if((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      CheckCooperativeLevel();
    } else {
      dxgsg_cat.error() << "show_frame() - Blt failed : " << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }

}

bool DXGraphicsStateGuardian::CheckCooperativeLevel(bool bDoReactivateWindow) {

    HRESULT hr = _pDD->TestCooperativeLevel();

    if(SUCCEEDED(_last_testcooplevel_result)) {
        if(SUCCEEDED(hr))  // this means this was just a safety check, dont need to restore surfs
            return true;

        // otherwise something just went wrong

        HRESULT hr;
    
        // TestCooperativeLevel returns DD_OK: If the current mode is same as the one which the App set.
        // The following error is returned only for exclusivemode apps.
        // DDERR_NOEXCLUSIVEMODE: Some other app took exclusive mode.

        hr = _pDD->TestCooperativeLevel();
    
        HRESULT expected_error = (dx_full_screen ? DDERR_NOEXCLUSIVEMODE : DDERR_EXCLUSIVEMODEALREADYSET);

        if(hr == expected_error) {
          // This means that mode changes had taken place, surfaces
          // were lost but still we are in the original mode, so we
          // simply restore all surfaces and keep going.
    
          if(dxgsg_cat.is_debug()) {
             if(dx_full_screen)
                dxgsg_cat.debug() << "Lost access to DDRAW exclusive mode, waiting to regain it...\n";
              else dxgsg_cat.debug() << "Another app has DDRAW exclusive mode, waiting...\n";
          }

          if(_dx_ready) {
             _win->deactivate_window();
             _dx_ready = FALSE;
          }
        } else if(hr==DDERR_WRONGMODE) {
            // This means that the desktop mode has changed
            // need to destroy all of dx stuff and recreate everything
            // back again, which is a big hit
            dxgsg_cat.error() << "detected display mode change in TestCoopLevel, must recreate all DDraw surfaces, D3D devices, this is not handled yet. hr == " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        } else if(FAILED(hr)) {
            dxgsg_cat.error() << "unexpected return code from TestCoopLevel: " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
    } else {
        // testcooplvl was failing, handle case where it now succeeds

        if(SUCCEEDED(hr)) {
          if(_last_testcooplevel_result == DDERR_EXCLUSIVEMODEALREADYSET) {
              if(dxgsg_cat.is_debug())
                  dxgsg_cat.debug() << "other app relinquished exclusive mode, refilling surfs...\n";
          } else if(_last_testcooplevel_result == DDERR_NOEXCLUSIVEMODE) {
                      if(dxgsg_cat.is_debug())
                          dxgsg_cat.debug() << "regained exclusive mode, refilling surfs...\n";
          }
              
          if(bDoReactivateWindow)
              _win->reactivate_window();  //must reactivate window before you can restore surfaces (otherwise you are in WRONGVIDEOMODE, and DDraw RestoreAllSurfaces fails)

          RestoreAllVideoSurfaces();  

          _dx_ready = TRUE;

        } else if(hr==DDERR_WRONGMODE) {
            // This means that the desktop mode has changed
            // need to destroy all of dx stuff and recreate everything
            // back again, which is a big hit
            dxgsg_cat.error() << "detected desktop display mode change in TestCoopLevel, must recreate all DDraw surfaces & D3D devices, this is not handled yet.  " << ConvD3DErrorToString(hr) << endl;
            _win->close_window();
            exit(1);
          } else if((hr!=DDERR_NOEXCLUSIVEMODE) && (hr!=DDERR_EXCLUSIVEMODEALREADYSET)) {
                      dxgsg_cat.error() << "unexpected return code from TestCoopLevel: " << ConvD3DErrorToString(hr) << endl;
                      exit(1);
                  }
    }

    _last_testcooplevel_result = hr;
    return SUCCEEDED(hr);
}

////////////////////////////////////////////////////////////////////
//     Function: handle_window_move
//       Access:
//  Description: we receive the new x and y position of the client
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::adjust_view_rect(int x, int y) {
    if (_view_rect.left != x || _view_rect.top != y) {
        _view_rect.right = x + _view_rect.right - _view_rect.left;
        _view_rect.left = x;
        _view_rect.bottom = y + _view_rect.bottom - _view_rect.top;
        _view_rect.top = y;

//  set_clipper(clip_rect);
    }
}

#if 0

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::save_mipmap_images
//       Access: Protected
//  Description: Saves out each mipmap level of the indicated texture
//               (which must also be the currently active texture in
//               the GL state) as a separate image file to disk.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::read_mipmap_images(Texture *tex) {
   Filename filename = tex->get_name();
   string name;
   if (filename.empty()) {
     static index = 0;
     name = "texture" + format_string(index);
     index++;
   } else {
     name = filename.get_basename_wo_extension();
   }

   PixelBuffer *pb = tex->get_ram_image();
   nassertv(pb != (PixelBuffer *)NULL);

   GLenum external_format = get_external_image_format(pb->get_format());
   GLenum type = get_image_type(pb->get_image_type());

   int xsize = pb->get_xsize();
   int ysize = pb->get_ysize();

   // Specify byte-alignment for the pixels on output.
   glPixelStorei(GL_PACK_ALIGNMENT, 1);

   int mipmap_level = 0;
   do {
     xsize = max(xsize, 1);
     ysize = max(ysize, 1);

     PT(PixelBuffer) mpb =
       new PixelBuffer(xsize, ysize, pb->get_num_components(),
                       pb->get_component_width(), pb->get_image_type(),
                       pb->get_format());
     glGetTexImage(GL_TEXTURE_2D, mipmap_level, external_format,
                   type, mpb->_image);
     Filename mipmap_filename = name + "_" + format_string(mipmap_level) + ".pnm";
     nout << "Writing mipmap level " << mipmap_level
          << " (" << xsize << " by " << ysize << ") "
          << mipmap_filename << "\n";
     mpb->write(mipmap_filename);

     xsize >>= 1;
     ysize >>= 1;
     mipmap_level++;
   } while (xsize > 0 && ysize > 0);
}
#endif


#if 0
//-----------------------------------------------------------------------------
// Name: SetViewMatrix()
// Desc: Given an eye point, a lookat point, and an up vector, this
//       function builds a 4x4 view matrix.
//-----------------------------------------------------------------------------
HRESULT SetViewMatrix( D3DMATRIX& mat, D3DVECTOR& vFrom, D3DVECTOR& vAt,
                       D3DVECTOR& vWorldUp ) {
    // Get the z basis vector, which points straight ahead. This is the
    // difference from the eyepoint to the lookat point.
    D3DVECTOR vView = vAt - vFrom;

    float fLength = Magnitude( vView );
    if (fLength < 1e-6f)
        return E_INVALIDARG;

    // Normalize the z basis vector
    vView /= fLength;

    // Get the dot product, and calculate the projection of the z basis
    // vector onto the up vector. The projection is the y basis vector.
    float fDotProduct = DotProduct( vWorldUp, vView );

    D3DVECTOR vUp = vWorldUp - fDotProduct * vView;

    // If this vector has near-zero length because the input specified a
    // bogus up vector, let's try a default up vector
    if (1e-6f > ( fLength = Magnitude( vUp ) )) {
        vUp = D3DVECTOR( 0.0f, 1.0f, 0.0f ) - vView.y * vView;

        // If we still have near-zero length, resort to a different axis.
        if (1e-6f > ( fLength = Magnitude( vUp ) )) {
            vUp = D3DVECTOR( 0.0f, 0.0f, 1.0f ) - vView.z * vView;

            if (1e-6f > ( fLength = Magnitude( vUp ) ))
                return E_INVALIDARG;
        }
    }

    // Normalize the y basis vector
    vUp /= fLength;

    // The x basis vector is found simply with the cross product of the y
    // and z basis vectors
    D3DVECTOR vRight = CrossProduct( vUp, vView );

    // Start building the matrix. The first three rows contains the basis
    // vectors used to rotate the view to point at the lookat point
    mat._11 = vRight.x;  mat._12 = vUp.x;  mat._13 = vView.x;  mat._14 = 0.0f;
    mat._21 = vRight.y;  mat._22 = vUp.y;  mat._23 = vView.y;  mat._24 = 0.0f;
    mat._31 = vRight.z;  mat._32 = vUp.z;  mat._33 = vView.z;  mat._34 = 0.0f;

    // Do the translation values (rotations are still about the eyepoint)
    mat._41 = - DotProduct( vFrom, vRight );
    mat._42 = - DotProduct( vFrom, vUp );
    mat._43 = - DotProduct( vFrom, vView );
    mat._44 = 1.0f;

    return S_OK;
}

#endif


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::prepare_geom_node
//       Access: Public, Virtual
//  Description: Prepares the indicated GeomNode for retained-mode
//               rendering.  If this function returns non-NULL, the
//               value returned will be passed back to a future call
//               to draw_geom_node(), which is expected to draw the
//               contents of the node.
////////////////////////////////////////////////////////////////////
GeomNodeContext *DXGraphicsStateGuardian::
prepare_geom_node(GeomNode *node) {


 if(link_tristrips) {
  for(int iGeom=0;iGeom<node->get_num_geoms();iGeom++) {
    dDrawable *drawable1 = node->get_geom(iGeom);
    
    if(!drawable1->is_of_type(Geom::get_class_type()))
      continue;
    
    Geom *geomptr=DCAST(Geom, drawable1);
    assert(geomptr!=NULL);
    
    if(!geomptr->is_of_type(GeomTristrip::get_class_type()))
      continue;
    
    GeomTristrip *me = DCAST(GeomTristrip, geomptr);
    assert(me!=NULL);
    
    int nPrims=me->get_num_prims();
    if(nPrims==1)
      continue;
    
    if(dxgsg_cat.is_debug()) {
       static bool bPrintedMsg=false;
       if(!bPrintedMsg) {
           dxgsg_cat.debug() << "linking tristrips together with degenerate tris\n";
           bPrintedMsg=true;
       }
    }

    bool bStripReversalNeeded;
    unsigned int nOrigTotalVerts=0;
    PTA_int new_lengths;
    int p;

    // sum things up so can reserve space for new vecs
    for(p=0;p<nPrims;p++) {
      nOrigTotalVerts+=me->get_length(p);
    }

    // could compute it exactly if I wanted to reproduce all the cTotalVertsOutputSoFar logic in the loop below
    // might save on memory reallocations. try to overestimate using *3.
    int cEstimatedTotalVerts=nOrigTotalVerts+nPrims*3-2;

    #define INIT_ATTRVARS(ATTRNAME,PTA_TYPENAME)                                                 \
       PTA_##PTA_TYPENAME old_##ATTRNAME##s,new_##ATTRNAME##s;                                   \
       PTA_ushort old_##ATTRNAME##_indices,new_##ATTRNAME##_indices;                             \
       GeomBindType ATTRNAME##binding;                                                           \
       me->get_##ATTRNAME##s(old_##ATTRNAME##s, ATTRNAME##binding, old_##ATTRNAME##_indices);    \
                                                                                                 \
       PTA_##PTA_TYPENAME::iterator old_##ATTRNAME##_iter=old_##ATTRNAME##s.begin();             \
       PTA_ushort::iterator old_##ATTRNAME##index_iter;                                          \
                                                                                                 \
       if((ATTRNAME##binding!=G_OFF)&&(ATTRNAME##binding!=G_OVERALL)) {                          \
         if(old_##ATTRNAME##_indices!=NULL) {                                                    \
             old_##ATTRNAME##index_iter=old_##ATTRNAME##_indices.begin();                        \
             new_##ATTRNAME##_indices.reserve(cEstimatedTotalVerts);                             \
         } else {                                                                                \
             new_##ATTRNAME##s.reserve(cEstimatedTotalVerts);                                    \
         }                                                                                       \
       }

    INIT_ATTRVARS(coord,Vertexf);
    INIT_ATTRVARS(color,Colorf);
    INIT_ATTRVARS(normal,Normalf);
    INIT_ATTRVARS(texcoord,TexCoordf);

#define IsOdd(X) (((X) & 0x1)!=0)

    int cTotalVertsOutputSoFar=0;
    int nVerts;
    bool bAddExtraStartVert;

    for(p=0;p<nPrims;p++) {
      nVerts=me->get_length(p);

         // if bStripStateStartsBackfacing, then if the current strip
         // ends frontfacing, we can fix the problem by just reversing
         // the current strip order.  But if the current strip ends
         // backfacing, this will not work, since last tri is encoded for backfacing slot
         // so it will be incorrectly backfacing when you put it in a backfacing
         // slot AND reverse the vtx order.
      
         // insert an extra pad vertex at the beginning to force the
         // strip backface-state parity to change (more expensive, since
         // we make 1 more degen tri).  We always want the first tri
         // to start in a front-facing slot (unless it's a reversed end

         bStripReversalNeeded = false;      
         bAddExtraStartVert=false;

         if(p==0) {
           cTotalVertsOutputSoFar+=nVerts+1;
         } else {
            if(!IsOdd(cTotalVertsOutputSoFar)) {
               // we're starting on a backfacing slot
               if(IsOdd(nVerts)) {
                  bStripReversalNeeded = true;
               } else {
                bAddExtraStartVert=true;
                cTotalVertsOutputSoFar++;
               }
            }

            cTotalVertsOutputSoFar+=nVerts+2;
            if(p==nPrims-1)
              cTotalVertsOutputSoFar--;
         }
                
#define PERVERTEX_ATTRLOOP(OLDVERT_ITERATOR,NEWVERT_VECTOR,VECLENGTH,NUMSTARTPADDINGATTRS,NUMENDPADDINGATTRS)    \
          if(bStripReversalNeeded) {                                                        \
              /* to preserve normal-direction property for backface-cull purposes, */       \
              /* vtx order must be reversed*/                                               \
                                                                                            \
              OLDVERT_ITERATOR+=((VECLENGTH)-1);  /* start at last vert, and go back*/      \
              /*dxgsg_cat.debug() << "doing reversal on strip " << p << " of length " << nVerts << endl;*/ \
                                                                                            \
              if(p!=0) {                                                                    \
                 /* copy first vert twice to link with last strip*/                         \
                 for(int i=0;i<NUMSTARTPADDINGATTRS;i++)                                    \
                      NEWVERT_VECTOR.push_back(*OLDVERT_ITERATOR);                          \
              }                                                                             \
                                                                                            \
              for(int v=0;v<(VECLENGTH);v++,OLDVERT_ITERATOR--) {                           \
                  NEWVERT_VECTOR.push_back(*OLDVERT_ITERATOR);                              \
              }                                                                             \
                                                                                            \
              OLDVERT_ITERATOR++;                                                           \
                                                                                            \
              if(p!=(nPrims-1)) {                                                           \
                  /* copy last vert twice to link to next strip  */                         \
                 for(int i=0;i<NUMENDPADDINGATTRS;i++)                                      \
                    NEWVERT_VECTOR.push_back(*OLDVERT_ITERATOR);                            \
              }                                                                             \
                                                                                            \
              OLDVERT_ITERATOR+=(VECLENGTH);                                                \
                                                                                            \
          } else {                                                                          \
              if(p!=0) {                                                                    \
                 /* copy first vert twice to link with last strip*/                         \
                 for(int i=0;i<NUMSTARTPADDINGATTRS;i++)                                    \
                      NEWVERT_VECTOR.push_back(*OLDVERT_ITERATOR);                          \
              }                                                                             \
                                                                                            \
              for(int v=0;v<(VECLENGTH);v++,OLDVERT_ITERATOR++)                             \
                  NEWVERT_VECTOR.push_back(*OLDVERT_ITERATOR);                              \
                                                                                            \
              if(p!=(nPrims-1)) {                                                           \
                  /* copy last vert twice to link to next strip  */                         \
                 for(int i=0;i<NUMENDPADDINGATTRS;i++)                                      \
                    NEWVERT_VECTOR.push_back(*(OLDVERT_ITERATOR-1));                        \
              }                                                                             \
          }


#define CONVERT_ATTR_VECTOR(ATTRNAME)                                                          \
      switch(ATTRNAME##binding) {                                                              \
            case G_OFF:                                                                        \
            case G_OVERALL:                                                                    \
                break;                                                                         \
                                                                                               \
            case G_PER_PRIM: {                                                                 \
              /* must convert to per-component*/                                               \
              /* nPrims*2+origTotalVerts-2 components  */                                      \
                int veclength=nVerts+2;                                                        \
                if((p==0)||(p==(nPrims-1)))                                                    \
                    veclength--;                                                               \
                if(bAddExtraStartVert)                                                         \
                   veclength++;                                                                \
                                                                                               \
                if(old_##ATTRNAME##_indices!=NULL) {                                           \
                    for(int v=0;v<veclength;v++)                                               \
                       new_##ATTRNAME##_indices.push_back(*old_##ATTRNAME##index_iter);        \
                                                                                               \
                    old_##ATTRNAME##index_iter++; /* move on to val for next strip*/           \
                 } else {                                                                      \
                      for(int v=0;v<veclength;v++)                                             \
                         new_##ATTRNAME##s.push_back(*old_##ATTRNAME##_iter);                  \
                                                                                               \
                      old_##ATTRNAME##_iter++; /* move on to val for next strip*/              \
                 }                                                                             \
                 break;                                                                        \
             }                                                                                 \
                                                                                               \
            case G_PER_COMPONENT:                                                              \
            case G_PER_VERTEX: {                                                               \
                int veclength,numstartcopies,numendcopies;                                     \
                                                                                               \
                if(ATTRNAME##binding==G_PER_VERTEX) {                                          \
                    veclength=nVerts;                                                          \
                    numstartcopies=numendcopies=1;                                             \
                } else {                                                                       \
                    veclength=nVerts-2;                                                        \
                    numstartcopies=numendcopies=2;                                             \
                }                                                                              \
                if(bAddExtraStartVert)                                                         \
                   numstartcopies++;                                                           \
                                                                                               \
                if(old_##ATTRNAME##_indices!=NULL) {                                           \
                    PERVERTEX_ATTRLOOP(old_##ATTRNAME##index_iter,new_##ATTRNAME##_indices,veclength,numstartcopies,numendcopies); \
                } else {                                                                       \
                    /* non-indexed case */                                                     \
                    PERVERTEX_ATTRLOOP(old_##ATTRNAME##_iter,new_##ATTRNAME##s,veclength,numstartcopies,numendcopies); \
                }                                                                              \
            }                                                                                  \
            break;                                                                             \
      }                                                                                        \

      
      CONVERT_ATTR_VECTOR(coord);

      #ifdef _DEBUG
         if(old_coord_indices==NULL)
           assert(cTotalVertsOutputSoFar==new_coords.size());
         else 
           assert(cTotalVertsOutputSoFar==new_coord_indices.size());
      #endif

      CONVERT_ATTR_VECTOR(color);
      CONVERT_ATTR_VECTOR(normal);
      CONVERT_ATTR_VECTOR(texcoord);

    }  // end per-Prim (strip) loop

    if(old_coord_indices!=NULL)  {
        me->set_coords(old_coords, coordbinding, new_coord_indices);
        new_lengths.push_back(new_coord_indices.size());
    } else {
        me->set_coords(new_coords, coordbinding, NULL);
        new_lengths.push_back(new_coords.size());
    }

    me->set_lengths(new_lengths);
    me->set_num_prims(1);

    #define SET_NEW_ATTRIBS(ATTRNAME)                                                                 \
    if((ATTRNAME##binding!=G_OFF) && (ATTRNAME##binding!=G_OVERALL)) {                                \
        if(ATTRNAME##binding==G_PER_PRIM)                                                             \
           ATTRNAME##binding=G_PER_COMPONENT;                                                         \
        if(old_##ATTRNAME##_indices==NULL) {                                                          \
           me->set_##ATTRNAME##s(new_##ATTRNAME##s, ATTRNAME##binding, NULL);                         \
        } else {                                                                                      \
           me->set_##ATTRNAME##s(old_##ATTRNAME##s, ATTRNAME##binding, new_##ATTRNAME##_indices);     \
        }                                                                                             \
    }
    /*    
    int ii;
    for( ii=0;ii<old_coords.size();ii++) 
        dxgsg_cat.debug() << "old coord[" << ii <<"] " << old_coords[ii] << endl;
    dxgsg_cat.debug() << "=================\n";
    for(ii=0;ii<new_coords.size();ii++) 
        dxgsg_cat.debug() << "new coord[" << ii <<"] " << new_coords[ii] << endl;
    dxgsg_cat.debug() << "=================\n";
    
    for( ii=0;ii<old_normals.size();ii++) 
        dxgsg_cat.debug() << "old norm[" << ii <<"] " << old_normals[ii] << endl;
    dxgsg_cat.debug() << "=================\n";
    for(ii=0;ii<new_normals.size();ii++) 
        dxgsg_cat.debug() << "new norm[" << ii <<"] " << new_normals[ii] << endl;
    
    if(old_color_indices!=NULL) {
    
    for( ii=0;ii<old_color_indices.size();ii++) 
        dxgsg_cat.debug() << "old colorindex[" << ii <<"] " << old_color_indices[ii] << endl;
    dxgsg_cat.debug() << "=================\n";
    for( ii=0;ii<new_color_indices.size();ii++) 
        dxgsg_cat.debug() << "new colorindex[" << ii <<"] " << new_color_indices[ii] << endl;
    }
    */

    SET_NEW_ATTRIBS(color);
    SET_NEW_ATTRIBS(normal);
    SET_NEW_ATTRIBS(texcoord);

    me->make_dirty();
  }
 } // if(link_tristrips)

  // for now, only using vertexbufs for static Geom, so
  // Make sure we have at least some static Geoms in the GeomNode;
  int cNumVerts=0,i,num_geoms = node->get_num_geoms();

  // need to always put in space for color because we might have some scene-graph color we need to add?
  // will that even work?  I think we'd have to overwrite all the VB colors dynamically, and then restore
  // them from somewhere if the global color goes away

  DWORD fvfFlags=D3DFVF_XYZ;// | D3DFVF_DIFFUSE;

  for (i = 0; (i < num_geoms); i++) {
    dDrawable *drawable1 = node->get_geom(i);
    if(!drawable1->is_of_type(Geom::get_class_type()))
      continue;
    
    Geom *geom=DCAST(Geom, drawable1);
    assert(geom!=NULL);

    DWORD new_fvfFlags=D3DFVF_XYZ;

    if(!geom->is_dynamic()) {
        cNumVerts+=geom->get_num_vertices();
        if(geom->get_binding(G_COLOR) != G_OFF)
            new_fvfFlags |= D3DFVF_DIFFUSE;
        if(geom->get_binding(G_NORMAL) != G_OFF)
            new_fvfFlags |= D3DFVF_NORMAL;
        if(geom->get_binding(G_TEXCOORD) != G_OFF) 
            new_fvfFlags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
    }
    if(i!=0) {
       if(fvfFlags!=new_fvfFlags) {
           // all primitives within a geom must use the same FVF type for the DrawPrim api
           dxgsg_cat.error() << "error creating vertex buffer, geoms within geomnode require differing vertex data types!!\n";
           exit(1);
       }
    } else fvfFlags=new_fvfFlags;
  }

  if(cNumVerts==0) {
    // Never mind.
    return (GeomNodeContext *)NULL;
  }

  // Ok, we've got something; use it.
  DXGeomNodeContext *dx_gnc = new DXGeomNodeContext(node);

  assert(dx_gnc!=NULL);

  // right now there is a 1-1 correspondence b/w vertbufs and geomnodecontexts.
  // later multiple geomnodecontexts will use the same vertbuf

  HRESULT hr;
  LPDIRECT3D7 pD3D;

  assert(_d3dDevice!=NULL);
  hr=_d3dDevice->GetDirect3D(&pD3D);
  assert(!FAILED(hr));
  LPDIRECT3DVERTEXBUFFER7 pD3DVertexBuffer;
  DX_DECLARE_CLEAN(D3DVERTEXBUFFERDESC, VBdesc);

  VBdesc.dwCaps = D3DVBCAPS_WRITEONLY;
  VBdesc.dwCaps |= _bIsTNLDevice ? 0x0 : D3DVBCAPS_SYSTEMMEMORY;
  VBdesc.dwFVF=fvfFlags;
  VBdesc.dwNumVertices=cNumVerts;

  hr=pD3D->CreateVertexBuffer(&VBdesc,&pD3DVertexBuffer,0x0);
  if(FAILED(hr)) {
    dxgsg_cat.error() << "error creating vertex buffer: " << ConvD3DErrorToString(hr) << endl;
    delete dx_gnc;
    exit(1);
  }

  dx_gnc->_pVB = pD3DVertexBuffer;

  if(!_bIsTNLDevice) {
      // create VB for ProcessVerts to xform to

      fvfFlags&=~D3DFVF_XYZ;    // switch to xformed vert type
      fvfFlags&=~D3DFVF_NORMAL; // xformed verts are also lighted, so no normals allowed
      fvfFlags|=D3DFVF_XYZRHW; 
      VBdesc.dwFVF=fvfFlags;

      hr=pD3D->CreateVertexBuffer(&VBdesc,&pD3DVertexBuffer,0x0);
      if(FAILED(hr)) {
          dxgsg_cat.error() << "error creating xformed vertex buffer: " << ConvD3DErrorToString(hr) << endl;
          delete dx_gnc;
          exit(1);
      }

      dx_gnc->_pXformed_VB = pD3DVertexBuffer;
  }

  pD3D->Release();

  dx_gnc->_num_verts=cNumVerts;
  dx_gnc->_start_index=0;   

  LPVOID pVertData=NULL;
  DWORD dwVBFlags = DDLOCK_NOOVERWRITE | DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR |
                    DDLOCK_WAIT | DDLOCK_DISCARDCONTENTS;

  hr=dx_gnc->_pVB->Lock(dwVBFlags,&pVertData,NULL);
  if(FAILED(hr)) {
        dxgsg_cat.error() << "error locking vertex buffer: " << ConvD3DErrorToString(hr) << endl;
        delete dx_gnc;
        exit(1);
  }

  assert(pVertData!=NULL);
  _pCurrentGeomContext->_pEndofVertData=(BYTE*)pVertData;
  _bDrawPrimDoSetupVertexBuffer = true;
  _pCurrentGeomContext = dx_gnc;

  for (i = 0; (i < num_geoms); i++) {
    dDrawable *drawable1 = node->get_geom(i);
    if(!drawable1->is_of_type(Geom::get_class_type()))
      continue;
    
    Geom *geom=DCAST(Geom, drawable1);
    assert(geom!=NULL);

    if(geom->is_dynamic()) {
      dx_gnc->_other_geoms.push_back(geom);
    } else {
       node->get_geom(i)->draw(this);
    }
  }

  _bDrawPrimDoSetupVertexBuffer = false;
  _pCurrentGeomContext = NULL;
  _pCurrentGeomContext->_pEndofVertData=NULL;  

  hr=dx_gnc->_pVB->Unlock();
  if(FAILED(hr)) {
      dxgsg_cat.error() << "error unlocking vertex buffer: " << ConvD3DErrorToString(hr) << endl;
      delete dx_gnc;
      exit(1);
  }

  hr=dx_gnc->_pVB->Optimize(_d3dDevice,0x0);
  if(FAILED(hr)) {
      dxgsg_cat.error() << "error optimizing vertex buffer: " << ConvD3DErrorToString(hr) << endl;
      delete dx_gnc;
      exit(1);
  }

#if 0  //DO_PSTATS
  float num_verts_after = 
    _vertices_tristrip_pcollector.get_level() +
    _vertices_trifan_pcollector.get_level() +
    _vertices_tri_pcollector.get_level() +
    _vertices_other_pcollector.get_level();
  float num_verts = num_verts_after - num_verts_before;
  ggnc->_num_verts = (int)(num_verts + 0.5);
#endif

  bool inserted = mark_prepared_geom_node(dx_gnc);

  // If this assertion fails, the same GeomNode was prepared twice,
  // which shouldn't be possible, since the GeomNode itself should
  // detect this.
  nassertr(inserted, NULL);

  return dx_gnc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_geom_node
//       Access: Public, Virtual
//  Description: Draws a GeomNode previously indicated by a call to
//               prepare_geom_node().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_geom_node(GeomNode *node, GeomNodeContext *gnc) {

  int i,num_geoms = node->get_num_geoms();

  if (gnc == (GeomNodeContext *)NULL) {

    // We don't have a saved context; just draw the GeomNode in
    // immediate mode.
    for (i = 0; i < num_geoms; i++) {
        node->get_geom(i)->draw(this);
    }
    return;
  }

  // We do have a saved context; use it.
  add_to_geom_node_record(gnc);
  DXGeomNodeContext *dx_gnc = DCAST(DXGeomNodeContext, gnc);

  #ifdef _DEBUG
     assert(dx_gnc->_pVB!=NULL);
     if(_bIsTNLDevice) 
        assert(dx_gnc->_pXformed_VB!=NULL);
  #endif
  
  if(!_bIsTNLDevice) {
      HRESULT hr;
    
      DWORD PVOp=D3DVOP_CLIP | D3DVOP_TRANSFORM | D3DVOP_EXTENTS;
    
      D3DVERTEXBUFFERDESC VBdesc;
      VBdesc.dwSize=sizeof(VBdesc);
      hr=dx_gnc->_pVB->GetVertexBufferDesc(&VBdesc);   // would be useful to keep fvf in vertbuf struct to avoid having to do this
      if(FAILED(hr)) {
        dxgsg_cat.error() << "error in getvbdesc: " << ConvD3DErrorToString(hr) << endl;
        exit(1);
      }
    
      if(VBdesc.dwFVF & D3DFVF_NORMAL) {
          PVOp|=D3DVOP_LIGHT;
      }
    
      hr=dx_gnc->_pXformed_VB->ProcessVertices(PVOp,0,dx_gnc->_num_verts,dx_gnc->_pVB,0,_d3dDevice,0x0);
      if(FAILED(hr)) {
        dxgsg_cat.error() << "error in ProcessVertices: " << ConvD3DErrorToString(hr) << endl;
        exit(1);
      }
  }


  for (i = 0; i < num_geoms; i++) {
      node->get_geom(i)->draw(this);
  }

#if 0 //def DO_PSTATS
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    _vertices_display_list_pcollector.add_level(dx_gnc->_num_verts);
#endif


  // Also draw all the dynamic Geoms.
  for (i = 0; i < num_geoms; i++) {
      dx_gnc->_other_geoms[i]->draw(this);
  }

}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::release_geom_node
//       Access: Public, Virtual
//  Description: Frees the resources previously allocated via a call
//               to prepare_geom_node(), including deleting the
//               GeomNodeContext itself, if necessary.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
release_geom_node(GeomNodeContext *gnc) {
  if (gnc != (GeomNodeContext *)NULL) {
    DXGeomNodeContext *dx_gnc = DCAST(DXGeomNodeContext, gnc);

    bool erased = unmark_prepared_geom_node(dx_gnc);

    // If this assertion fails, a GeomNode was released that hadn't
    // been prepared (or a GeomNode was released twice).
    nassertv(erased);
    
    dx_gnc->_node->clear_gsg(this);
    delete dx_gnc;  // should release vertex buffer
  }
}
