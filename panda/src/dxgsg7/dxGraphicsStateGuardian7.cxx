// Filename: dxGraphicsStateGuardian7.cxx
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

#include "dxGraphicsStateGuardian7.h"
#include "config_dxgsg7.h"

#include "displayRegion.h"
#include "renderBuffer.h"
#include "geom.h"
#include "geomSphere.h"
#include "geomIssuer.h"
#include "graphicsWindow.h"
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
#include "fogAttrib.h"
#include "depthOffsetAttrib.h"
#include "fog.h"

#include "throw_event.h"

#ifdef DO_PSTATS
#include "pStatTimer.h"
#include "pStatCollector.h"
#endif

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

TypeHandle DXGraphicsStateGuardian7::_type_handle;

// bit masks used for drawing primitives
// bitmask type: normal=0x1,color=0x2,texcoord=0x4
typedef enum { NothingSet=0,NormalOnly,ColorOnly,Normal_Color,TexCoordOnly,
               Normal_TexCoord,Color_TexCoord,Normal_Color_TexCoord
} DrawLoopFlags;

#define PER_NORMAL   NormalOnly
#define PER_COLOR    ColorOnly
#define PER_TEXCOORD TexCoordOnly

static D3DMATRIX matIdentity;

#ifdef COUNT_DRAWPRIMS
// you should just use Intel GPT instead of this stuff

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

    if(_pCurDeviceTexture==pLastTexture) {
        cDP_noTexChangeCount++;
    } else pLastTexture = _pCurDeviceTexture;
}
#else
#define CountDPs(nv,nt)
#endif

#define MY_D3DRGBA(r,g,b,a) ((D3DCOLOR) D3DRGBA(r,g,b,a))

#if defined(DO_PSTATS) || defined(PRINT_TEXSTATS)
static bool bTexStatsRetrievalImpossible=false;
#endif

//#define Colorf_to_D3DCOLOR(out_color) (MY_D3DRGBA((out_color)[0], (out_color)[1], (out_color)[2], (out_color)[3]))

INLINE DWORD
Colorf_to_D3DCOLOR(const Colorf &cColorf) {
// MS VC defines _M_IX86 for x86.  gcc should define _X86_
#if defined(_M_IX86) || defined(_X86_)
    DWORD d3dcolor,tempcolorval=255;

    // note the default FPU rounding mode will give 255*0.5f=0x80, not 0x7F as VC would force it to by resetting rounding mode
    // dont think this makes much difference

    __asm {
        push ebx   ; want to save this in case this fn is inlined
        push ecx
        mov ecx, cColorf
        fild tempcolorval
        fld DWORD PTR [ecx]
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
    }

   //   dxgsg7_cat.debug() << (void*)d3dcolor << endl;
   return d3dcolor;
#else //!_X86_
   return MY_D3DRGBA(cColorf[0], cColorf[1], cColorf[2], cColorf[3]);
#endif //!_X86_
}
#ifdef _DEBUG
void
dbgPrintVidMem(LPDIRECTDRAW7 pDD, LPDDSCAPS2 lpddsCaps,const char *pMsg) {
  DWORD dwTotal,dwFree;
  HRESULT hr;

 //  These Caps bits arent allowed to be specified when calling GetAvailVidMem.
 //  They don't affect surface allocation in a vram heap.

#define AVAILVIDMEM_BADCAPS  (DDSCAPS_BACKBUFFER   | \
                              DDSCAPS_FRONTBUFFER  | \
                              DDSCAPS_COMPLEX      | \
                              DDSCAPS_FLIP         | \
                              DDSCAPS_OWNDC        | \
                              DDSCAPS_PALETTE      | \
                              DDSCAPS_SYSTEMMEMORY | \
                              DDSCAPS_VISIBLE      | \
                              DDSCAPS_WRITEONLY)

  DDSCAPS2 ddsCaps = *lpddsCaps;
  ddsCaps.dwCaps &= ~(AVAILVIDMEM_BADCAPS);  // turn off the bad caps
  //  ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY; done internally by DX anyway
  
  if(FAILED(  hr = pDD->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
    wdxdisplay7_cat.debug() << "GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
    exit(1);
  }

  // Write a debug message to the console reporting the texture memory.
  char tmpstr[100],tmpstr2[100];
  sprintf(tmpstr,"%.4g",dwTotal/1000000.0);
  sprintf(tmpstr2,"%.4g",dwFree/1000000.0);
  if(wdxdisplay7_cat.is_debug())
    wdxdisplay7_cat.debug() << "AvailableVidMem before creating "<< pMsg << ",(megs) total: " << tmpstr << "  free:" << tmpstr2 <<endl;
}
#endif

void DXGraphicsStateGuardian7::
set_color_clear_value(const Colorf& value) {
  _color_clear_value = value;
  _d3dcolor_clear_value =  Colorf_to_D3DCOLOR(value);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian7::
DXGraphicsStateGuardian7(const FrameBufferProperties &properties) :
  GraphicsStateGuardian(properties, CS_yup_left) 
{
    // allocate local buffers used during rendering

    GraphicsStateGuardian::reset();

    _pScrn = NULL;
    _pCurFvfBufPtr = NULL;
    _pFvfBufBasePtr = new BYTE[VERT_BUFFER_SIZE];  // allocate storage for vertex info.
    _index_buf = new WORD[D3DMAXNUMVERTICES];  // allocate storage for vertex index info.
    _dx_ready = false;
    _overlay_windows_supported = false;

//    _pScrn->pddsPrimary = _pScrn->pddsZBuf = _pScrn->pddsBack = NULL;
//    _pDD = NULL;
//    _pScrn->pD3DDevice = NULL;

    // non-dx obj values inited here should not change if resize is
    // called and dx objects need to be recreated (otherwise they
    // belong in dx_init, with other renderstate

    ZeroMemory(&matIdentity,sizeof(D3DMATRIX));
    matIdentity._11 = matIdentity._22 = matIdentity._33 = matIdentity._44 = 1.0f;

    // All implementations have the following buffers.
    _buffer_mask = (RenderBuffer::T_color |
                    RenderBuffer::T_depth |
                    RenderBuffer::T_back
//                  RenderBuffer::T_stencil |
//                  RenderBuffer::T_accum
                    );

    //   this is incorrect for general mono displays, need both right and left flags set.
    //   stereo has not been handled yet for dx
    //    _buffer_mask &= ~RenderBuffer::T_right;

    _cur_read_pixel_buffer=RenderBuffer::T_front;

    set_color_clear_value(_color_clear_value);

    // DirectX drivers seem to consistently invert the texture when
    // they copy framebuffer-to-texture.  Ok.
    _copy_texture_inverted = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian7::
~DXGraphicsStateGuardian7() {
/*   
    if(IS_VALID_PTR(_pScrn)) {
        assert((_pScrn->pD3DDevice==NULL) || IS_VALID_PTR(_pScrn->pD3DDevice));
        _pScrn->pD3DDevice->SetTexture(0, NULL);  // this frees reference to the old texture
    }
*/
    _pCurTexContext = NULL;

    // free_dxgsg_objects() ????????????

    free_pointers();
    SAFE_DELETE_ARRAY(_pFvfBufBasePtr)
    SAFE_DELETE_ARRAY(_index_buf);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.  The GraphicsWindow pointer represents a
//               typical window that might be used for this context;
//               it may be required to set up the frame buffer
//               properly the first time.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
reset() {
    GraphicsStateGuardian::reset();
    dxgsg7_cat.error() << "DXGSG reset() not implemented properly yet!\n";
    // delete all the objs too, right?
    //dx_init();
}

void DXGraphicsStateGuardian7::  
set_context(DXScreenData *pNewContextData) {
    // dont do copy from window since dx_init sets fields too.
    // simpler to keep all of it in one place, so use ptr to window struct

    assert(pNewContextData!=NULL);
    _pScrn = pNewContextData;
    _pD3DDevice = _pScrn->pD3DDevice;   //copy this one field for speed of deref
}

// recreate dx objects without modifying gsg state, other than clearing state cache
void DXGraphicsStateGuardian7::
free_dxgsg_objects(void) {
    ULONG refcnt;

    free_pointers();

    // dont want a full reset of gsg, just a state clear
    set_state(RenderState::make_empty());
    // want gsg to pass all state settings through
    // we need to reset our internal state guards right here because dx_init() should be called after this,
    // which resets all of them to our defaults, and syncs them with the D3DRENDERSTATE

    _dx_ready = false;

    if (_pScrn->pD3DDevice!=NULL) {
        _pScrn->pD3DDevice->SetTexture(0,NULL);  // should release this stuff internally anyway
        RELEASE(_pScrn->pD3DDevice,dxgsg7,"d3dDevice",RELEASE_DOWN_TO_ZERO);
    }

    DeleteAllVideoSurfaces();

    // Release the DDraw and D3D objects used by the app
    RELEASE(_pScrn->pddsZBuf,dxgsg7,"zbuffer",false);
    RELEASE(_pScrn->pddsBack,dxgsg7,"backbuffer",false);
    RELEASE(_pScrn->pddsPrimary,dxgsg7,"primary surface",false);
}

HRESULT CALLBACK EnumTexFmtsCallback( LPDDPIXELFORMAT pddpf, VOID* param ) {
    // wont build if its a member fn, so had to do this stuff
#ifdef USE_TEXFMTVEC
    DDPixelFormatVec *pPixFmtVec =  (DDPixelFormatVec *) param;
    pPixFmtVec->push_back(*pddpf);
#else
    DXGraphicsStateGuardian7 *mystate = (DXGraphicsStateGuardian7 *) param;
    assert(mystate->_cNumTexPixFmts < MAX_DX_TEXPIXFMTS);
    memcpy( &(mystate->_pTexPixFmts[mystate->_cNumTexPixFmts]), pddpf, sizeof(DDPIXELFORMAT) );
    mystate->_cNumTexPixFmts++;
#endif
    return DDENUMRET_OK;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::reset
//       Access: Public, Virtual
//  Description: Handles initialization which assumes that DX has already been
//               set up.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
dx_init( void) {
/*
         LPDIRECTDRAW7     context,
          LPDIRECTDRAWSURFACE7  pri,
          LPDIRECTDRAWSURFACE7  back,
          LPDIRECTDRAWSURFACE7  zbuf,
          LPDIRECT3D7          pD3D,
          LPDIRECT3DDEVICE7    pDevice,
          RECT viewrect) */
    assert(_pScrn->pDD!=NULL);
    assert(_pScrn->pD3D!=NULL);
    assert(_pScrn->pD3DDevice!=NULL);
    assert(_pScrn->pddsPrimary!=NULL);
    assert(_pScrn->pddsBack!=NULL);

//    _pDD=_pScrn->pDD;  // save for speed of access
//    _pCurD3DDevice = _pScrn->pD3DDevice;

/*    _pDD = context;
    _pScrn->pddsPrimary = pri;
    _pScrn->pddsBack = back;
    _pScrn->pddsZBuf = zbuf;
    _pScrn->pD3D = pD3D;
    _pScrn->pD3DDevice = pDevice;
    _view_rect = viewrect;
*/

    ZeroMemory(&_lmodel_ambient,sizeof(Colorf));
    _pScrn->pD3DDevice->SetRenderState( D3DRENDERSTATE_AMBIENT, 0x0);

    _clip_plane_bits = 0;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE , 0x0);

    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPING, true);
    _clipping_enabled = true;

    _CurShadeMode =  D3DSHADE_FLAT;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, _CurShadeMode);

    _depth_write_enabled = true;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, _depth_write_enabled);

    // need to free these properly
#ifndef USE_TEXFMTVEC
    _cNumTexPixFmts = 0;
    _pTexPixFmts = NULL;
#endif
    _pCurTexContext = NULL;

    // none of these are implemented
    //_multisample_enabled = false;
    //_point_smooth_enabled = false;

    _line_smooth_enabled = false;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, false);

    _color_material_enabled = false;

    _depth_test_enabled = D3DZB_FALSE;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);

    _blend_enabled = false;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, (DWORD)_blend_enabled);

    _pScrn->pD3DDevice->GetRenderState(D3DRENDERSTATE_SRCBLEND, (DWORD*)&_blend_source_func);
    _pScrn->pD3DDevice->GetRenderState(D3DRENDERSTATE_DESTBLEND, (DWORD*)&_blend_dest_func);

    _fog_enabled = false;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, _fog_enabled);

    _current_projection_mat = LMatrix4f::ident_mat();
    _projection_mat_stack_count = 0;
    _has_scene_graph_color = false;

//  GL stuff that hasnt been translated to DX
//    _scissor_enabled = false;
//    _multisample_alpha_one_enabled = false;
//    _multisample_alpha_mask_enabled = false;
//    _line_width = 1.0f;
//    _point_size = 1.0f;

    assert(_pScrn->pddsBack!=NULL);  // dxgsg7 is always double-buffered right now

#ifdef COUNT_DRAWPRIMS
     global_pD3DDevice = pDevice;
#endif
    _bDrawPrimDoSetupVertexBuffer = false;

    _last_testcooplevel_result = S_OK;

    HRESULT hr;

#ifdef USE_TEXFMTVEC
    assert(_pScrn->TexPixFmts.size()==0);

    if(FAILED(hr=_pScrn->pD3DDevice->EnumTextureFormats(EnumTexFmtsCallback, &_pScrn->TexPixFmts))) {
#else
    _pTexPixFmts = new DDPIXELFORMAT[MAX_DX_TEXPIXFMTS];
    _cNumTexPixFmts = 0;
    assert(_pTexPixFmts!=NULL);

    if(FAILED(hr=_pScrn->pD3DDevice->EnumTextureFormats(EnumTexFmtsCallback, this))) {
#endif
        if(hr==D3DERR_TEXTURE_NO_SUPPORT) {
            dxgsg7_cat.error() << "EnumTextureFormats indicates No Texturing Support on this HW!, exiting...\n";
            exit(1);
        } else {
            dxgsg7_cat.error() << "EnumTextureFormats failed! hr = " << ConvD3DErrorToString(hr) << endl;
        }
    }

    DX_DECLARE_CLEAN(DDCAPS,ddCaps);
    if (FAILED(hr = _pScrn->pDD->GetCaps(&ddCaps,NULL))) {
        dxgsg7_cat.fatal() << "GetCaps failed on DDraw! hr = " << ConvD3DErrorToString(hr) << "\n";
        exit(1);
    }

    // s3 virge drivers sometimes give crap values for these
    if(_pScrn->D3DDevDesc.dwMaxTextureWidth==0)
       _pScrn->D3DDevDesc.dwMaxTextureWidth=256;

    if(_pScrn->D3DDevDesc.dwMaxTextureHeight==0)
       _pScrn->D3DDevDesc.dwMaxTextureHeight=256;

//  shouldve already been set
//    sc_bIsTNLDevice = (IsEqualGUID(_pScrn->D3DDevDesc.deviceGUID,IID_IDirect3DTnLHalDevice)!=0);

    if ((dx_decal_type==GDT_offset) && !(_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBIAS)) {
#ifdef _DEBUG
        // dx7 doesnt support PLANEMASK renderstate
#if(DIRECT3D_VERSION < 0x700)
        dxgsg7_cat.debug() << "dx-decal-type 'offset' not supported by hardware, switching to decal masking\n";
#else
        dxgsg7_cat.debug() << "dx-decal-type 'offset' not supported by hardware, switching to decal double-draw blend-based masking\n";
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
    dxgsg7_cat.spam() << "polygon-offset decaling disabled in dxgsg7, switching to double-draw decaling\n";
#endif

#if(DIRECT3D_VERSION < 0x700)
    dx_decal_type = GDT_mask;
#else
    dx_decal_type = GDT_blend;
#endif
#endif

    if ((dx_decal_type==GDT_mask) && !(_pScrn->D3DDevDesc.dpcTriCaps.dwMiscCaps & D3DPMISCCAPS_MASKPLANES)) {
#ifdef _DEBUG
        dxgsg7_cat.debug() << "No hardware support for colorwrite disabling, switching to dx-decal-type 'mask' to 'blend'\n";
#endif
        dx_decal_type = GDT_blend;
    }

    if (((dx_decal_type==GDT_blend)||(dx_decal_type==GDT_mask)) && !(_pScrn->D3DDevDesc.dpcTriCaps.dwMiscCaps & D3DPMISCCAPS_MASKZ)) {
        dxgsg7_cat.error() << "dx-decal-type mask impossible to implement, no hardware support for Z-masking, decals will not appear correctly!\n";
    }

//#define REQUIRED_BLENDCAPS (D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE|D3DPBLENDCAPS_SRCCOLOR|D3DPBLENDCAPS_INVSRCCOLOR| \
//                            D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_INVSRCALPHA | D3DPBLENDCAPS_DESTALPHA|D3DPBLENDCAPS_INVDESTALPHA|D3DPBLENDCAPS_DESTCOLOR|D3DPBLENDCAPS_INVDESTCOLOR)
// voodoo3 doesnt support commented out ones, & we dont need them now

#define REQUIRED_BLENDCAPS (D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE|  /*D3DPBLENDCAPS_SRCCOLOR|D3DPBLENDCAPS_INVSRCCOLOR| */ \
                            D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_INVSRCALPHA /* | D3DPBLENDCAPS_DESTALPHA|D3DPBLENDCAPS_INVDESTALPHA|D3DPBLENDCAPS_DESTCOLOR|D3DPBLENDCAPS_INVDESTCOLOR*/)

    if (((_pScrn->D3DDevDesc.dpcTriCaps.dwSrcBlendCaps & REQUIRED_BLENDCAPS)!=REQUIRED_BLENDCAPS) ||
        ((_pScrn->D3DDevDesc.dpcTriCaps.dwDestBlendCaps & REQUIRED_BLENDCAPS)!=REQUIRED_BLENDCAPS)) {
        dxgsg7_cat.error() << "device is missing alpha blending capabilities, blending may not work correctly: SrcBlendCaps: 0x"<< (void*) _pScrn->D3DDevDesc.dpcTriCaps.dwSrcBlendCaps << "  DestBlendCaps: "<< (void*) _pScrn->D3DDevDesc.dpcTriCaps.dwDestBlendCaps << endl;
    }

    if (!(_pScrn->D3DDevDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY)) {
        dxgsg7_cat.error() << "device is missing texture transparency capability, transparency may not work correctly!  TextureCaps: 0x"<< (void*) _pScrn->D3DDevDesc.dpcTriCaps.dwTextureCaps << endl;
    }

    // just require trilinear.  if it can do that, it can probably do all the lesser point-sampling variations too
#define REQUIRED_TEXFILTERCAPS (D3DPTFILTERCAPS_MAGFLINEAR |  D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_LINEAR)
    if ((_pScrn->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps & REQUIRED_TEXFILTERCAPS)!=REQUIRED_TEXFILTERCAPS) {
        dxgsg7_cat.error() << "device is missing texture bilinear filtering capability, textures may appear blocky!  TextureFilterCaps: 0x"<< (void*) _pScrn->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
    }
#define REQUIRED_MIPMAP_TEXFILTERCAPS (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_LINEARMIPLINEAR)

    if (!(ddCaps.ddsCaps.dwCaps & DDSCAPS_MIPMAP)) {
        dxgsg7_cat.debug() << "device does not have mipmap texturing filtering capability!   TextureFilterCaps: 0x"<< (void*) _pScrn->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
        dx_ignore_mipmaps = TRUE;
    } else if ((_pScrn->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps & REQUIRED_MIPMAP_TEXFILTERCAPS)!=REQUIRED_MIPMAP_TEXFILTERCAPS) {
        dxgsg7_cat.debug() << "device is missing tri-linear mipmap filtering capability, texture mipmaps may not supported! TextureFilterCaps: 0x"<< (void*) _pScrn->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
    }

#define REQUIRED_TEXBLENDCAPS (D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2)
    if ((_pScrn->D3DDevDesc.dwTextureOpCaps & REQUIRED_TEXBLENDCAPS)!=REQUIRED_TEXBLENDCAPS) {
        dxgsg7_cat.error() << "device is missing some required texture blending capabilities, texture blending may not work properly! TextureOpCaps: 0x"<< (void*) _pScrn->D3DDevDesc.dwTextureOpCaps << endl;
    }

    if(_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE) {
        // watch out for drivers that emulate per-pixel fog with per-vertex fog (Riva128, Matrox Millen G200)
        // some of these require gouraud-shading to be set to work, as if you were using vertex fog
        _doFogType=PerPixelFog;
    } else {
        // every card is going to have vertex fog, since it's implemented in d3d runtime
        assert((_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX )!=0);

        // vtx fog may look crappy if you have large polygons in the foreground and they get clipped,
        // so you may want to disable it

        if(dx_no_vertex_fog) {
            _doFogType = None;
        } else {
            _doFogType = PerVertexFog;

            // range-based fog only works with vertex fog in dx7/8
            if(dx_use_rangebased_fog && (_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGRANGE))
                _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_RANGEFOGENABLE, true);
        }
    }

    SetRect(&_pScrn->clip_rect, 0,0,0,0);  // no clip rect set

    // Lighting, let's turn it off by default
    _lighting_enabled = false;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, _lighting_enabled);

    // turn on dithering if the rendertarget is < 8bits/color channel
    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_back);
    _pScrn->pddsBack->GetSurfaceDesc(&ddsd_back);
    _dither_enabled = (!dx_no_dithering) && ((ddsd_back.ddpfPixelFormat.dwRGBBitCount < 24) &&
                       (_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER));
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, _dither_enabled);

    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPING,true);

    // Stencil test is off by default
    _stencil_test_enabled = false;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, _stencil_test_enabled);

    // Antialiasing.
    enable_line_smooth(false);
//  enable_multisample(true);

    _current_fill_mode = RenderModeAttrib::M_filled;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);

    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);

    if(dx_auto_normalize_lighting)
         _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, true);

    // initial clip rect
    SetRect(&_pScrn->clip_rect, 0,0,0,0);     // no clip rect set

    // must do SetTSS here because redundant states are filtered out by our code based on current values above, so
    // initial conditions must be correct

    _CurTexBlendMode = TextureApplyAttrib::M_modulate;
    SetTextureBlendMode(_CurTexBlendMode,FALSE);
    _texturing_enabled = false;
    _pScrn->pD3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);  // disables texturing

    // Init more Texture State
    _CurTexMagFilter=(D3DTEXTUREMAGFILTER) 0x0;
    _CurTexMinFilter=(D3DTEXTUREMINFILTER) 0x0;
    _CurTexMipFilter=(D3DTEXTUREMIPFILTER) 0x0;
    _CurTexWrapModeU=_CurTexWrapModeV=Texture::WM_clamp;
    _CurTexAnisoDegree=1;

    // this code must match apply_texture() code for states above
    // so DX TSS renderstate matches dxgsg7 state

    _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
    _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_POINT);
    _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
    _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_MAXANISOTROPY,_CurTexAnisoDegree);
    _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSU,get_texture_wrap_mode(_CurTexWrapModeU));
    _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSV,get_texture_wrap_mode(_CurTexWrapModeV));

#ifdef _DEBUG
    if ((_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS) &&
        (dx_global_miplevel_bias!=0.0f)) {
        _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&dx_global_miplevel_bias)) );
    }
#endif

    if (dx_full_screen_antialiasing) {
      if(_pScrn->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT) {
        _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS,D3DANTIALIAS_SORTINDEPENDENT);
        if(dxgsg7_cat.is_debug())
            dxgsg7_cat.debug() << "enabling full-screen anti-aliasing\n";
      } else {
        if(dxgsg7_cat.is_debug())
            dxgsg7_cat.debug() << "device doesnt support full-screen anti-aliasing\n";
      }
    }

#ifndef NDEBUG
    if(dx_force_backface_culling!=0) {
      if((dx_force_backface_culling > 0) &&
         (dx_force_backface_culling < D3DCULL_FORCE_DWORD)) {
             _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, dx_force_backface_culling);
      } else {
          dx_force_backface_culling=0;
          if(dxgsg7_cat.is_debug())
              dxgsg7_cat.debug() << "error, invalid value for dx-force-backface-culling\n";
      }
    }
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, dx_force_backface_culling);
#else
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
#endif

    _alpha_func = D3DCMP_ALWAYS;
    _alpha_func_refval = 1.0f;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, _alpha_func);
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (_alpha_func_refval*255.0f));
    _alpha_test_enabled = false;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, _alpha_test_enabled);

    // Make sure the DX state matches all of our initial attribute states.
    CPT(RenderAttrib) dta = DepthTestAttrib::make(DepthTestAttrib::M_less);
    CPT(RenderAttrib) dwa = DepthWriteAttrib::make(DepthWriteAttrib::M_on);
    CPT(RenderAttrib) cfa = CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise);

    dta->issue(this);
    dwa->issue(this);
    cfa->issue(this);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_clear(const RenderBuffer &buffer) {
  //    DO_PSTATS_STUFF(PStatTimer timer(_win->_clear_pcollector));

    nassertv(buffer._gsg == this);
    int buffer_type = buffer._buffer_type;

    int flags = 0;

    if (buffer_type & RenderBuffer::T_depth) {
        flags |=  D3DCLEAR_ZBUFFER;
        assert(_pScrn->pddsZBuf!=NULL);
    }
    if (buffer_type & RenderBuffer::T_back)       //set appropriate flags
        flags |=  D3DCLEAR_TARGET;
    if (buffer_type & RenderBuffer::T_stencil)
        flags |=  D3DCLEAR_STENCIL;

    HRESULT  hr = _pScrn->pD3DDevice->Clear(0, NULL, flags, _d3dcolor_clear_value,
                                    (D3DVALUE) _depth_clear_value, (DWORD)_stencil_clear_value);
    if (hr != DD_OK) {
        dxgsg7_cat.error() << "clear_buffer failed:  Clear returned " << ConvD3DErrorToString(hr) << endl;
        throw_event("panda3d-render-error");
    }
    /*  The following line will cause the background to always clear to a medium red
    _color_clear_value[0] = .5;
    /*  The following lines will cause the background color to cycle from black to red.
    _color_clear_value[0] += .001;
     if (_color_clear_value[0] > 1.0f) _color_clear_value[0] = 0.0f;
     */
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::prepare_display_region
//       Access: Public, Virtual
//  Description: Prepare a display region for rendering (set up
//       scissor region and viewport)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
prepare_display_region() {
  if (_current_display_region == (DisplayRegion*)0L) {
    dxgsg7_cat.error()
      << "Invalid NULL display region in prepare_display_region()\n";

  } else if (_current_display_region != _actual_display_region) {
    _actual_display_region = _current_display_region;

    int l, u, w, h;
    _actual_display_region->get_region_pixels_i(l, u, w, h);

    // Create the viewport
    D3DVIEWPORT7 vp = { l, u, w, h, 0.0f, 1.0f };
    HRESULT hr = _pScrn->pD3DDevice->SetViewport(&vp);
    if (FAILED(hr)) {
      dxgsg7_cat.error()
        << "SetViewport(" << l << ", " << u << ", " << w << ", " << h
        << ") failed : result = " << ConvD3DErrorToString(hr)
        << endl;
      throw_event("panda3d-render-error"); 
    }
    
    // Note: for DX9, also change scissor clipping state here
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::prepare_lens
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
bool DXGraphicsStateGuardian7::
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
    hr = _pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,
                                   (LPD3DMATRIX)new_projection_mat.get_data());
  return SUCCEEDED(hr);
}

#ifndef NO_MULTIPLE_DISPLAY_REGIONS
////////////////////////////////////////////////////////////////////
//     Function: set_clipper
//       Access:
//  Description: Useless in DX at the present time
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::set_clipper(RECT cliprect) {

    LPDIRECTDRAWCLIPPER Clipper;
    HRESULT result;

    // For windowed mode, the clip region is associated with the window,
    // and DirectX does not allow you to create clip regions.
    if (_win->is_fullscreen()) return;

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

    if (_pScrn->pddsPrimary->GetClipper(&Clipper) != DD_OK) {
        result = _pScrn->pDD->CreateClipper(0, &Clipper, NULL);
        result = Clipper->SetClipList(rgn_data, 0);
        result = _pScrn->pddsPrimary->SetClipper(Clipper);
    } else {
        result = Clipper->SetClipList(rgn_data, 0 );
        if (result == DDERR_CLIPPERISUSINGHWND) {
            result = _pScrn->pddsPrimary->SetClipper(NULL);
            result = _pScrn->pDD->CreateClipper(0, &Clipper, NULL);
            result = Clipper->SetClipList(rgn_data, 0 ) ;
            result = _pScrn->pddsPrimary->SetClipper(Clipper);
        }
    }
    free(rgn_data);
    DeleteObject(hrgn);
}
#endif

#if defined(_DEBUG) || defined(COUNT_DRAWPRIMS)
typedef enum {DrawPrim,DrawIndexedPrim,DrawPrimStrided} DP_Type;
static const char *DP_Type_Strs[3] = {"DrawPrimitive","DrawIndexedPrimitive","DrawPrimitiveStrided"};

void INLINE TestDrawPrimFailure(DP_Type dptype,HRESULT hr,LPDIRECTDRAW7 pDD,DWORD nVerts,DWORD nTris) {
        if(FAILED(hr)) {
            // loss of exclusive mode is not a real DrawPrim problem, ignore it
            HRESULT testcooplvl_hr = pDD->TestCooperativeLevel();
            if((testcooplvl_hr != DDERR_NOEXCLUSIVEMODE)||(testcooplvl_hr != DDERR_EXCLUSIVEMODEALREADYSET)) {
                dxgsg7_cat.fatal() << DP_Type_Strs[dptype] << "() failed: result = " << ConvD3DErrorToString(hr) << endl;
                exit(1);
            }
        }

        CountDPs(nVerts,nTris);
}
#else
#define TestDrawPrimFailure(a,b,c,nVerts,nTris) CountDPs(nVerts,nTris);
#endif

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::get_blend_func
//       Access: Protected, Static
//  Description: Maps from ColorBlendAttrib::Operand to D3DBLEND
//               value.
////////////////////////////////////////////////////////////////////
D3DBLEND DXGraphicsStateGuardian7::
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

  dxgsg7_cat.error()
    << "Unknown color blend operand " << (int)operand << endl;
  return D3DBLEND_ZERO;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::report_texmgr_stats
//       Access: Protected
//  Description: Reports the DX texture manager's activity to PStats.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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
      if(FAILED(  hr = _pScrn->pDD->GetAvailableVidMem(&ddsCaps,&dwVidTotal,&dwVidFree))) {
            dxgsg7_cat.debug() << "report_texmgr GetAvailableVidMem for VIDMEM failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
      }

      ddsCaps.dwCaps = DDSCAPS_TEXTURE;
      if(FAILED(  hr = _pScrn->pDD->GetAvailableVidMem(&ddsCaps,&dwTexTotal,&dwTexFree))) {
            dxgsg7_cat.debug() << "report_texmgr GetAvailableVidMem for TEXTURE failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
      }
  }

  D3DDEVINFO_TEXTUREMANAGER tminfo;
  ZeroMemory(&tminfo,sizeof(D3DDEVINFO_TEXTUREMANAGER));

  if(!bTexStatsRetrievalImpossible) {
      hr = _pScrn->pD3DDevice->GetInfo(D3DDEVINFOID_TEXTUREMANAGER,&tminfo,sizeof(D3DDEVINFO_TEXTUREMANAGER));
      if (hr!=D3D_OK) {
          if (hr==S_FALSE) {
              static int PrintedMsg=2;
              if(PrintedMsg>0) {
                  if(dxgsg7_cat.is_debug())
                    dxgsg7_cat.debug() << " ************ texstats GetInfo() requires debug DX DLLs to be installed!!  ***********\n";
                  ZeroMemory(&tminfo,sizeof(D3DDEVINFO_TEXTUREMANAGER));
                  bTexStatsRetrievalImpossible=true;
              }
          } else {
              dxgsg7_cat.error() << "d3ddev->GetInfo(TEXTUREMANAGER) failed to get tex stats: result = " << ConvD3DErrorToString(hr) << endl;
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
    dxgsg7_cat.debug() << "\nAvailableVidMem for RenderSurfs: (megs) total: " << tmpstr1 << "  free: " << tmpstr2
                      << "\nAvailableVidMem for Textures:    (megs) total: " << tmpstr3 << "  free: " << tmpstr4 << endl;

   if(!bTexStatsRetrievalImpossible) {
            dxgsg7_cat.spam()
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
            hr = _pScrn->pD3DDevice->GetInfo(D3DDEVINFOID_TEXTURING,&texappinfo,sizeof(D3DDEVINFO_TEXTURING));
            if (hr!=D3D_OK) {
                dxgsg7_cat.error() << "GetInfo(TEXTURING) failed : result = " << ConvD3DErrorToString(hr) << endl;
                return;
            } else {
                dxgsg7_cat.spam()
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
//     Function: DXGraphicsStateGuardian7::add_to_FVFBuf
//       Access: Private
//  Description: This adds data to the flexible vertex format
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
add_to_FVFBuf(void *data,  size_t bytes) {
    memcpy(_pCurFvfBufPtr, data, bytes);
    _pCurFvfBufPtr += bytes;
}

// generates slightly fewer instrs
#define add_DWORD_to_FVFBuf(data) { *((DWORD *)_pCurFvfBufPtr) = (DWORD) data;  _pCurFvfBufPtr += sizeof(DWORD);}

typedef enum {
    FlatVerts,IndexedVerts,MixedFmtVerts
} GeomVertFormat;

INLINE void DXGraphicsStateGuardian7::
transform_color(Colorf &InColor,D3DCOLOR &OutRGBAColor) {
  Colorf transformed
    ((InColor[0] * _current_color_scale[0]) + _current_color_offset[0],
     (InColor[1] * _current_color_scale[1]) + _current_color_offset[1],
     (InColor[2] * _current_color_scale[2]) + _current_color_offset[2],
     (InColor[3] * _current_color_scale[3]) + _current_color_offset[3]);
  OutRGBAColor = Colorf_to_D3DCOLOR(transformed);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_prim_setup
//       Access: Private
//  Description: This adds data to the flexible vertex format
////////////////////////////////////////////////////////////////////
size_t DXGraphicsStateGuardian7::
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
    if(_color_transform_enabled == 0) {                                                 \
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
   _curFVFflags = D3DFVF_XYZ;
   size_t vertex_size = sizeof(D3DVALUE) * 3;

   GeomBindType ColorBinding=geom->get_binding(G_COLOR);
   bool bDoColor=(ColorBinding != G_OFF);

   if (bDoColor || _has_scene_graph_color) {
        ci = geom->make_color_iterator();
        _curFVFflags |= D3DFVF_DIFFUSE;
        vertex_size += sizeof(D3DCOLOR);

        if (_has_scene_graph_color) {
            if (_scene_graph_color_stale) {
              // Compute the D3DCOLOR for the scene graph override color.
              if(_color_transform_enabled == 0) {
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
        _curFVFflags |= D3DFVF_NORMAL;
        vertex_size += sizeof(D3DVALUE) * 3;

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
       _curFVFflags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
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

   D3DSHADEMODE needed_shademode =
       (((_perVertex & (PER_COLOR | (wants_normals() ? PER_NORMAL : 0))) || _fog_enabled) ?
        D3DSHADE_GOURAUD : D3DSHADE_FLAT);

   set_shademode(needed_shademode);

   return vertex_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_prim_inner_loop
//       Access: Private
//  Description: This adds data to the flexible vertex format with a check
//               for component normals and color
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_prim_inner_loop(int nVerts, const Geom *geom, ushort perFlags) {
    Vertexf NextVert;

    for(;nVerts > 0;nVerts--) {
         // coord info will always be _perVertex
        GET_NEXT_VERTEX(NextVert);     // need to optimize these
        add_to_FVFBuf((void *)&NextVert, sizeof(D3DVECTOR));

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

        if (_curFVFflags & D3DFVF_NORMAL)
            add_to_FVFBuf((void *)&p_normal, sizeof(D3DVECTOR));
        if (_curFVFflags & D3DFVF_DIFFUSE)
            add_DWORD_to_FVFBuf(_curD3Dcolor);
        if (_curFVFflags & D3DFVF_TEXCOUNT_MASK)
            add_to_FVFBuf((void *)&p_texcoord, sizeof(TexCoordf));
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_prim_inner_loop_coordtexonly
//       Access: Private
//  Description: FastPath loop used by animated character data
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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
           memcpy(pLocalFvfBufPtr,(void*)&_coord_array[*pCurCoordIndex],sizeof(D3DVECTOR));
           pCurCoordIndex++;
        } else {
           memcpy(pLocalFvfBufPtr,(void*)pCurCoord,sizeof(D3DVECTOR));
           pCurCoord++;
        }

        pLocalFvfBufPtr+=sizeof(D3DVECTOR);

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
//     Function: DXGraphicsStateGuardian7::draw_point
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_point(GeomPoint *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "draw_point()" << endl;
#endif

    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    // The DX Way

    int nPrims = geom->get_num_prims();

    if (nPrims==0) {
        dxgsg7_cat.warning() << "draw_point() called with ZERO vertices!!" << endl;
        return;
    }

#ifdef _DEBUG
    static BOOL bPrintedMsg=FALSE;

    if (!bPrintedMsg && (geom->get_size()!=1.0f)) {
        bPrintedMsg=TRUE;
        dxgsg7_cat.warning() << "D3D does not support drawing points of non-unit size, setting point size to 1.0f!\n";
    }
#endif

    nassertv(nPrims < D3DMAXNUMVERTICES );

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

    _perVertex = 0x0;
    _perPrim = 0x0;
    if (geom->get_binding(G_NORMAL) == G_PER_VERTEX)
        _perVertex |= PER_NORMAL;
    if (geom->get_binding(G_COLOR) == G_PER_VERTEX)
        _perVertex |= PER_COLOR;

    // for Indexed Prims and mixed indexed/non-indexed prims, we will use old pipeline for now
    // need to add code to handle fully indexed mode (and handle cases with index arrays of different lengths,
    // values (may only be possible to handle certain cases without reverting to old pipeline)
    if (GeomVrtFmt!=FlatVerts) {
        size_t vertex_size = draw_prim_setup(geom);

        nassertv(_pCurFvfBufPtr == NULL);    // make sure the storage pointer is clean.
        nassertv(nPrims * vertex_size < VERT_BUFFER_SIZE);
        _pCurFvfBufPtr = _pFvfBufBasePtr;          // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

        // iterate through the point
        draw_prim_inner_loop(nPrims, geom, _perVertex | _perPrim);

        nassertv((nPrims*vertex_size) == (_pCurFvfBufPtr-_pFvfBufBasePtr));

        HRESULT hr = _pScrn->pD3DDevice->DrawPrimitive(D3DPT_POINTLIST, _curFVFflags, _pFvfBufBasePtr, nPrims, NULL);
        TestDrawPrimFailure(DrawPrim,hr,_pScrn->pDD,nPrims,0);
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

            if(_color_transform_enabled == 0) {
              for (int i=0;i<nPrims;i++) {
                Colorf out_color=colors[i];
                add_DWORD_to_FVFBuf(Colorf_to_D3DCOLOR(out_color));
              }
            } else {
              for (int i=0;i<nPrims;i++) {
                D3DCOLOR RGBA_color;
                transform_color(colors[i],RGBA_color);
                add_DWORD_to_FVFBuf(RGBA_color);
              }
            }
        }

        if (_curFVFflags & D3DFVF_TEXCOUNT_MASK) {
            dps_data.textureCoords[0].lpvData = (VOID*)texcoords;
            dps_data.textureCoords[0].dwStride = sizeof(TexCoordf);
        }

        HRESULT hr = _pScrn->pD3DDevice->DrawPrimitiveStrided(D3DPT_POINTLIST, _curFVFflags, &dps_data, nPrims, NULL);
        TestDrawPrimFailure(DrawPrimStrided,hr,_pScrn->pDD,nPrims,0);
    }

    _pCurFvfBufPtr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_line(GeomLine* geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "draw_line()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

#ifdef _DEBUG
    static BOOL bPrintedMsg=FALSE;

    // note: need to implement approximation of non-1.0 width lines with quads

    if (!bPrintedMsg && (geom->get_width()!=1.0f)) {
        bPrintedMsg=TRUE;
        if(dxgsg7_cat.is_debug())
            dxgsg7_cat.debug() << "DX does not support drawing lines with a non-1.0f pixel width, setting width to 1.0f!\n";
    }
#endif

    int nPrims = geom->get_num_prims();

    if (nPrims==0) {
        if(dxgsg7_cat.is_debug())
           dxgsg7_cat.debug() << "draw_line() called with ZERO vertices!!" << endl;
        return;
    }

    _perVertex = 0x0;
    _perPrim = 0x0;
    _perComp = 0x0;

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
      hr = _pScrn->pD3DDevice->DrawPrimitive(D3DPT_LINELIST, _curFVFflags, _pFvfBufBasePtr, nVerts, NULL);
    } else {
      nassertv((nVerts*vertex_size) == (_pCurFvfBufPtr-_tmp_fvfOverrunBuf));
      hr = _pScrn->pD3DDevice->DrawPrimitive(D3DPT_LINELIST, _curFVFflags, _tmp_fvfOverrunBuf, nVerts, NULL);
      delete [] _tmp_fvfOverrunBuf;
    }
    TestDrawPrimFailure(DrawPrim,hr,_pScrn->pDD,nVerts,0);

    _pCurFvfBufPtr = NULL;
}

void DXGraphicsStateGuardian7::
draw_linestrip(GeomLinestrip* geom, GeomContext *gc) {

#ifdef _DEBUG
    static BOOL bPrintedMsg=FALSE;

    if (!bPrintedMsg && (geom->get_width()!=1.0f)) {
        bPrintedMsg=TRUE;
        if(dxgsg7_cat.is_debug())
            dxgsg7_cat.debug() << "DX does not support drawing lines with a non-1.0f pixel width, setting width to 1.0f!\n";
    }
#endif

  draw_linestrip_base(geom,gc,false);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_linestrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_linestrip_base(Geom* geom, GeomContext *gc, bool bConnectEnds) {
// Note draw_linestrip_base() may be called from non-line draw_fns to support wireframe mode

#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "draw_linestrip()" << endl;
#endif

    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    int nPrims = geom->get_num_prims();
    const int *pLengthArr = geom->get_lengths();

    if(nPrims==0) {
        if(dxgsg7_cat.is_debug())
            dxgsg7_cat.debug() << "draw_linestrip() called with ZERO vertices!!" << endl;
        return;
    }

    _perVertex = 0x0;
    _perPrim = 0x0;
    _perComp = 0x0;

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

        HRESULT hr = _pScrn->pD3DDevice->DrawPrimitive(D3DPT_LINESTRIP, _curFVFflags, _pFvfBufBasePtr, nVerts, NULL);
        TestDrawPrimFailure(DrawPrim,hr,_pScrn->pDD,nVerts,0);

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
//     Function: DXGraphicsStateGuardian7::draw_sprite
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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
    dxgsg7_cat.debug() << "draw_sprite()" << endl;
#endif
    // get the array traversal set up.
    int nprims = geom->get_num_prims();

    if (nprims==0) {
        return;
    }

    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));

    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(nprims));

    bool bReEnableDither=false;

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
      tex_xsize = tex->_pbuffer->get_xsize();
      tex_ysize = tex->_pbuffer->get_ysize();
    }

    // save the modelview matrix
    const LMatrix4f &modelview_mat = _transform->get_mat();

    // We don't need to mess with the aspect ratio, since we are now
    // using the default projection matrix, which has the right aspect
    // ratio built in.

    // null the world xform, so sprites are orthog to scrn
    _pScrn->pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matIdentity);
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

    WrappedSprite *SpriteArray = new WrappedSprite[nprims];

    //BUGBUG: could we use _fvfbuf for this to avoid perframe alloc?
    // alternately, alloc once when retained mode becomes available

    if (SpriteArray==NULL) {
        dxgsg7_cat.fatal() << "draw_sprite() out of memory!!" << endl;
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

    #if 0
        //   not going to attempt this bDoColor optimization to use default white color in flat-shaded
        //   mode anymore,  it just make the logic confusing below.  from now on, always have color in FVF

        _curFVFflags = D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) ;
        DWORD vertex_size = sizeof(float) * 2 + sizeof(D3DVALUE) * 3;

        bool bDoColor=true;

        if (color_overall) {
            GET_NEXT_COLOR();
            CurColor = _curD3Dcolor;
            bDoColor = (_curD3Dcolor != ~0);  // dont need to add color if it's all white
        }

        if (bDoColor) {
            _curFVFflags |= D3DFVF_DIFFUSE;
            vertex_size+=sizeof(D3DCOLOR);
        }
    #else
      _curFVFflags = D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) | D3DFVF_DIFFUSE;
      DWORD vertex_size = sizeof(float) * 2 + sizeof(D3DVALUE) * 3 + sizeof(D3DCOLOR);

      if (color_overall) {
        GET_NEXT_COLOR();
        CurColor = _curD3Dcolor;
      }
    #endif

    // see note on fog in draw_prim_setup
    bool bUseGouraudShadedColor=_fog_enabled;

    set_shademode(!_fog_enabled ? D3DSHADE_FLAT: D3DSHADE_GOURAUD);

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

        add_to_FVFBuf((void *)ll.get_data(), sizeof(D3DVECTOR));
        if (!color_overall)  // otherwise its already been set globally
           CurColor = pSpr->_c;
        add_DWORD_to_FVFBuf(CurColor); // only need to cpy color on 1st vert, others are just empty ignored space
        add_to_FVFBuf((void *)TexCrdSets[0], sizeof(float)*2);

        /*********  LR vertex  **********/

        add_to_FVFBuf((void *)lr.get_data(), sizeof(D3DVECTOR));

        // if flat shading, dont need to write color for middle vtx, just incr ptr
        if(bUseGouraudShadedColor)
            *((DWORD *)_pCurFvfBufPtr) = (DWORD) CurColor;
        _pCurFvfBufPtr += sizeof(D3DCOLOR);

        add_to_FVFBuf((void *)TexCrdSets[1], sizeof(float)*2);

        /*********  UL vertex  **********/

        add_to_FVFBuf((void *)ul.get_data(), sizeof(D3DVECTOR));
        // if flat shading, dont need to write color for middle vtx, just incr ptr
        if(bUseGouraudShadedColor)
            *((DWORD *)_pCurFvfBufPtr) = (DWORD) CurColor;
        _pCurFvfBufPtr += sizeof(D3DCOLOR);
        add_to_FVFBuf((void *)TexCrdSets[2], sizeof(float)*2);

        /*********  UR vertex  **********/

        add_to_FVFBuf((void *)ur.get_data(), sizeof(D3DVECTOR));
        add_DWORD_to_FVFBuf(CurColor);
        add_to_FVFBuf((void *)TexCrdSets[3], sizeof(float)*2);

        for (int ii=0;ii<QUADVERTLISTLEN;ii++) {
            _index_buf[CurDPIndexArrLength+ii]=QuadVertIndexList[ii]+CurVertCount;
        }
        CurDPIndexArrLength+=QUADVERTLISTLEN;
        CurVertCount+=4;
    }

    nassertv(((4*nprims)*vertex_size) == (_pCurFvfBufPtr-_pFvfBufBasePtr));

    // cant do tristrip/fan since it would require 1 call want to make 1 call for multiple quads which arent connected
    // best we can do is indexed primitive, which sends 2 redundant indices instead of sending 2 redundant full verts
    HRESULT hr = _pScrn->pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, _curFVFflags, _pFvfBufBasePtr, 4*nprims, _index_buf,QUADVERTLISTLEN*nprims,NULL);
    TestDrawPrimFailure(DrawIndexedPrim,hr,_pScrn->pDD,QUADVERTLISTLEN*nprims,nprims);

    _pCurFvfBufPtr = NULL;
    delete [] SpriteArray;

    // restore the matrices
    _pScrn->pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,
                                  (LPD3DMATRIX)modelview_mat.get_data());
    if(bReEnableDither)
        enable_dither(true);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_polygon
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_polygon(GeomPolygon *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
   dxgsg7_cat.debug() << "draw_polygon()" << endl;
#endif
   DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
   DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

   // wireframe polygon will be drawn as linestrip, otherwise draw as multi-tri trifan
   DWORD rstate;
   _pScrn->pD3DDevice->GetRenderState(D3DRENDERSTATE_FILLMODE, &rstate);
   if(rstate!=D3DFILL_WIREFRAME) {
       draw_multitri(geom, D3DPT_TRIANGLEFAN);
   } else {
       draw_linestrip_base(geom,gc,true);
   }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_quad
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_quad(GeomQuad *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "draw_quad()" << endl;
#endif
   DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
   DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

   // wireframe quad will be drawn as linestrip, otherwise draw as multi-tri trifan
   DWORD rstate;
   _pScrn->pD3DDevice->GetRenderState(D3DRENDERSTATE_FILLMODE, &rstate);
   if(rstate!=D3DFILL_WIREFRAME) {
       draw_multitri(geom, D3DPT_TRIANGLEFAN);
   } else {
       draw_linestrip_base(geom,gc,true);
   }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_tri
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_tri(GeomTri *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "draw_tri()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_tri_pcollector.add_level(geom->get_num_vertices()));

#if 0
    if (_pCurTexContext!=NULL) {
        dxgsg7_cat.spam() << "Cur active DX texture: " << _pCurTexContext->_tex->get_name() << "\n";
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

/*
   for now, always use complex path, since DPstrided path never gave speedup
    GeomVertFormat GeomVrtFmt=FlatVerts;

    // first determine if we're indexed or non-indexed


    if ((vindexes!=NULL)&&(cindexes!=NULL)&&(tindexes!=NULL)&&(nindexes!=NULL)) {
        GeomVrtFmt=IndexedVerts;
        //make sure array sizes are consistent, we can only pass 1 size to DrawIPrm
//      nassertv(coords.size==norms.size);      nassertv(coords.size==colors.size);     nassertv(coords.size==texcoords.size);  need to assert only if we have this w/same binding
        // indexed mode requires all used norms,colors,texcoords,coords array be the same
        // length, or 0 or 1 (dwStride==0), also requires all elements to use the same index array
    }

    else if (!((vindexes==NULL)&&(cindexes==NULL)&&(tindexes==NULL)&&(nindexes==NULL)))
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
    if (GeomVrtFmt!=FlatVerts)
*/

     {
        // this is the old geom setup, it reformats every vtx into an output array passed to d3d

        _perVertex = 0x0;
        _perPrim = 0x0;

        bool bUseTexCoordOnlyLoop = ((ColorBinding != G_PER_VERTEX) &&
                                     (NormalBinding == G_OFF) &&
                                     (TexCoordBinding != G_OFF));
        bool bPerPrimNormal=false;

        if(bUseTexCoordOnlyLoop) {
           _perVertex |= PER_TEXCOORD;  // TexCoords are either G_OFF or G_PER_VERTEX
        } else {
            if(NormalBinding == G_PER_VERTEX)
                _perVertex |= PER_NORMAL;
            else if(NormalBinding == G_PER_PRIM) {
                    _perPrim |= PER_NORMAL;
                    bPerPrimNormal=true;
            }

            if(TexCoordBinding == G_PER_VERTEX)
               _perVertex |= PER_TEXCOORD;
        }

        bool bPerPrimColor=(ColorBinding == G_PER_PRIM);
        if(bPerPrimColor)
           _perPrim |= PER_COLOR;
          else if(ColorBinding == G_PER_VERTEX)
                 _perVertex |= PER_COLOR;

        size_t vertex_size = draw_prim_setup(geom);

        // Note: draw_prim_setup could unset color flags if global color is set, so must
        //       recheck this flag here!
        bPerPrimColor=(_perPrim & PER_COLOR)!=0x0;

        #ifdef _DEBUG
          // is it Ok not to recompute bUseTexCoordOnlyLoop even if draw_prim_setup unsets color flags?
          // add this check to make sure
           bool bNewUseTexCoordOnlyLoop = (((_perVertex & PER_COLOR)==0x0) &&
                                           ((_curFVFflags & D3DFVF_NORMAL)==0x0) &&
                                           ((_curFVFflags & D3DFVF_TEX1)!=0x0));

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

        hr = _pScrn->pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, _curFVFflags, _pFvfBufBasePtr, nVerts, NULL);
        TestDrawPrimFailure(DrawPrim,hr,_pScrn->pDD,nVerts,nPrims);

        _pCurFvfBufPtr = NULL;
    }

/*
    else {

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

        // see fog comment in draw_prim_setup
        D3DSHADEMODE NeededShadeMode = (_fog_enabled) ? D3DSHADE_GOURAUD : D3DSHADE_FLAT;

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

                for (uint i=0;i<nPrims;i++,pInVec++,pOutVec+=dwVertsperPrim) {
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

        // We should issue geometry colors only if the scene graph color is off.
        bool bDoGlobalSceneGraphColor = FALSE;
        bool bDoColor = (_vertex_colors_enabled && ColorBinding != G_OFF);
        if (_has_scene_graph_color) {
          bDoColor = TRUE;
          bDoGlobalSceneGraphColor = TRUE;
          ColorBinding = G_OVERALL;
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

                    if(_color_transform_enabled == 0) {
                        for (uint i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsperPrim) {
                            D3DCOLOR newcolr = Colorf_to_D3DCOLOR(*pInColor);
                            *pOutColor     = newcolr;
                            *(pOutColor+1) = newcolr;
                            *(pOutColor+2) = newcolr;
                        }
                     } else {
                        for (uint i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsperPrim) {
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

                    if(_color_transform_enabled == 0) {
                        for (uint i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsperPrim) {
                            *pOutColor = Colorf_to_D3DCOLOR(*pInColor);
                        }
                     } else {
                        for (uint i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsperPrim) {
                            transform_color(*pInColor,*pOutColor);
                        }
                    }
                }
            } else if (ColorBinding==G_PER_VERTEX) {
                NeededShadeMode = D3DSHADE_GOURAUD;

                // want to do this conversion once in retained mode
                DWORD cNumColors=nPrims*dwVertsperPrim;

                    if(_color_transform_enabled == 0) {
                        for (uint i=0;i<cNumColors;i++,pInColor++,pOutColor++) {
                            *pOutColor = Colorf_to_D3DCOLOR(*pInColor);
                        }
                     } else {
                        for (uint i=0;i<cNumColors;i++,pInColor++,pOutColor++) {
                            transform_color(*pInColor,*pOutColor);
                        }
                    }
            } else {
#ifdef _DEBUG
                nassertv(ColorBinding==G_OVERALL);
#endif
                // copy the one global color in, set stride to 0

                if(_color_transform_enabled == 0) {
                    if (bDoGlobalSceneGraphColor) {
                        Colorf colr = _scene_graph_color;
                        *pConvertedColorArray = Colorf_to_D3DCOLOR(colr);
                    } else {
                        *pConvertedColorArray = Colorf_to_D3DCOLOR(*pInColor);
                    }
                } else {
                    if (bDoGlobalSceneGraphColor) {
                        Colorf colr = _scene_graph_color;
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

        hr = _pScrn->pD3DDevice->DrawPrimitiveStrided(primtype, fvf_flags, &dps_data, nVerts, NULL);
        TestDrawPrimFailure(DrawPrimStrided,hr,_pScrn->pDD,nVerts,nPrims);

        _pCurFvfBufPtr = NULL;
    }
*/
///////////////////////////

/*
#if 0
    // test triangle for me to dbg experiments only
    float vert_buf[15] = {
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        33.0, 0.0f, 0.0f,  0.0f, 2.0,
        0.0f, 0.0f, 33.0,  2.0, 0.0f
    };

    _pScrn->pD3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_BORDER);
    _pScrn->pD3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_BORDER);
    _pScrn->pD3DDevice->SetTextureStageState(0,D3DTSS_BORDERCOLOR,MY_D3DRGBA(0,0,0,0));

    _curFVFflags =  D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) ;
    HRESULT hr = _pScrn->pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST,  _curFVFflags, vert_buf, nPrims*3, NULL);
    TestDrawPrimFailure(DrawPrim,hr,_pScrn->pDD,nPrims*3,nPrims);
#endif
*/
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_tristrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_tristrip(GeomTristrip *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
  dxgsg7_cat.debug() << "draw_tristrip()" << endl;
#endif
  DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
  DO_PSTATS_STUFF(_vertices_tristrip_pcollector.add_level(geom->get_num_vertices()));

  draw_multitri(geom, D3DPT_TRIANGLESTRIP);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_trifan
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_trifan(GeomTrifan *geom, GeomContext *gc) {

#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "draw_trifan()" << endl;
#endif
  DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
  DO_PSTATS_STUFF(_vertices_trifan_pcollector.add_level(geom->get_num_vertices()));

  draw_multitri(geom, D3DPT_TRIANGLEFAN);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_multitri
//       Access: Public, Virtual
//  Description: handles trifans and tristrips
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_multitri(Geom *geom, D3DPRIMITIVETYPE trilisttype) {

    DWORD nPrims = geom->get_num_prims();
    const uint *pLengthArr = (const uint *) ((const int *)geom->get_lengths());
    HRESULT hr;

    if(nPrims==0) {
        #ifdef _DEBUG
          dxgsg7_cat.warning() << "draw_multitri() called with ZERO vertices!!" << endl;
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
/*
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
    if (GeomVrtFmt!=FlatVerts)
*/

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

            switch (ColorBinding) {
                case G_PER_PRIM:
                    _perPrim |= PER_COLOR;
                    break;
                case G_PER_COMPONENT:
                    _perComp |= PER_COLOR;
                    break;
                case G_PER_VERTEX:
                    _perVertex |= PER_COLOR;
                    break;
            }
        }

        // draw_prim_setup() REQUIRES _perVertex, etc flags setup properly prior to call
        size_t vertex_size = draw_prim_setup(geom);

        // Note: draw_prim_setup could unset color flags if global color is set, so must
        //       recheck this flag here!
        bPerPrimColor=(_perPrim & PER_COLOR)!=0;

        #ifdef _DEBUG
          // is it Ok not to recompute bUseTexCoordOnlyLoop even if draw_prim_setup unsets color flags?
          // add this check to make sure.  texcoordonly needs input that with unchanging color, except per-prim
           bool bNewUseTexCoordOnlyLoop = ((((_perComp|_perVertex) & PER_COLOR)==0x0) &&
                                           ((_curFVFflags & D3DFVF_NORMAL)==0x0) &&
                                           ((_curFVFflags & D3DFVF_TEX1)!=0x0));

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

            if(bPerPrimColor) {
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

            hr = _pScrn->pD3DDevice->DrawPrimitive(trilisttype,  _curFVFflags, _pFvfBufBasePtr, nVerts, NULL);
            TestDrawPrimFailure(DrawPrim,hr,_pScrn->pDD,nVerts,nVerts-2);

            _pCurFvfBufPtr = NULL;
        }
    }

#if 0
    else {

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

        for (uint i=0;i<nPrims;i++) {
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
                const uint *pLengths=pLengthArr;

                nassertv(norms.size()>=nPrims);

                for (uint i=0;i<nPrims;i++,pInVec++,pLengths++) {
                    for (uint j=0;j<(*pLengths);j++,pOutVec++) {
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

        // We should issue geometry colors only if the scene graph color is off.
        bool bDoGlobalSceneGraphColor = FALSE;
        bool bDoColor = (_vertex_colors_enabled && ColorBinding != G_OFF);
        if (_has_scene_graph_color) {
          bDoColor = TRUE;
          bDoGlobalSceneGraphColor = TRUE;
          ColorBinding = G_OVERALL;
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

                if(_color_transform_enabled == 0) {
                    for (uint i=0;i<cTotalVerts;i++,pInColor++,pOutColor++) {
                        *pOutColor = Colorf_to_D3DCOLOR(*pInColor);
                    }
                } else {
                    for (uint i=0;i<cTotalVerts;i++,pInColor++,pOutColor++) {
                        transform_color(*pInColor,*pOutColor);
                    }
                }
            } else if (ColorBinding==G_PER_PRIM) {
                // must use tmp array to expand per-prim info to per-vert info
                // eventually want to do this conversion once in retained mode
                // have one color per strip, need 1 color per vert

                // could save 2 clr writes per strip/fan in flat shade mode but not going to bother here

                if(_color_transform_enabled == 0) {
                    for (uint j=0;j<nPrims;j++,pInColor++) {
                        D3DCOLOR lastcolr = Colorf_to_D3DCOLOR(*pInColor);
                        DWORD cStripLength=pLengthArr[j];
                        for (uint i=0;i<cStripLength;i++,pOutColor++) {
                            *pOutColor = lastcolr;
                        }
                    }
                } else {
                    for (uint j=0;j<nPrims;j++,pInColor++) {
                        D3DCOLOR lastcolr;
                        transform_color(*pInColor,lastcolr);
                        DWORD cStripLength=pLengthArr[j];
                        for (uint i=0;i<cStripLength;i++,pOutColor++) {
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
                    for (uint i=0;i<cCurStripColorCnt;i++,pInColor++,pOutColor++)

                #define COLOR_CONVERT_COPY_STMT  {*pOutColor = Colorf_to_D3DCOLOR(*pInColor);}
                #define COLOR_CONVERT_XFORM_STMT {transform_color(*pInColor,*pOutColor);}

                #define COMPONENT_COLOR_COPY_LOOPS(COLOR_COPYSTMT)  {                        \
                    if (NeededShadeMode == D3DSHADE_FLAT) {                                  \
                        /* FLAT shade mode.  for tristrips, skip writing last 2 verts.  */   \
                        /* for trifans, skip first and last verts                       */   \
                        if (trilisttype==D3DPT_TRIANGLESTRIP) {                              \
                            for (uint j=0;j<nPrims;j++) {                                    \
                                MULTITRI_COLORCOPY_LOOP {                                    \
                                   COLOR_COPYSTMT;                                           \
                                }                                                            \
                                pOutColor+=cNumMoreVertsthanTris;                            \
                            }                                                                \
                        } else {  /* trifan */                                               \
                            for (uint j=0;j<nPrims;j++) {                                    \
                                pOutColor++;                                                 \
                                MULTITRI_COLORCOPY_LOOP {                                    \
                                   COLOR_COPYSTMT;                                           \
                                }                                                            \
                                pOutColor++;                                                 \
                            }                                                                \
                        }                                                                    \
                    } else {  /* GOURAUD shademode (due to presence of normals) */           \
                        if (trilisttype==D3DPT_TRIANGLESTRIP) {                              \
                            for (uint j=0;j<nPrims;j++) {                                    \
                                MULTITRI_COLORCOPY_LOOP {                                    \
                                   COLOR_COPYSTMT;                                           \
                                }                                                            \
                                DWORD lastcolr = *(pOutColor-1);                             \
                                *pOutColor++ = lastcolr;                                     \
                                *pOutColor++ = lastcolr;                                     \
                            }                                                                \
                        } else {  /* trifan */                                               \
                            for (uint j=0;j<nPrims;j++) {                                    \
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

                if(_color_transform_enabled == 0) {
                  COMPONENT_COLOR_COPY_LOOPS(COLOR_CONVERT_COPY_STMT);
                } else {
                  COMPONENT_COLOR_COPY_LOOPS(COLOR_CONVERT_XFORM_STMT);
                }
            } else {
#ifdef _DEBUG
                nassertv(ColorBinding==G_OVERALL);
#endif
                // copy the one global color in, set stride to 0

                if(_color_transform_enabled == 0) {
                    if (bDoGlobalSceneGraphColor) {
                        Colorf colr = _scene_graph_color();
                        *pConvertedColorArray = Colorf_to_D3DCOLOR(colr);
                    } else {
                        *pConvertedColorArray = Colorf_to_D3DCOLOR(*pInColor);
                    }
                } else {
                    if (bDoGlobalSceneGraphColor) {
                        Colorf colr = _scene_graph_color();
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

        for (uint j=0;j<nPrims;j++) {
            const uint cCurNumStripVerts = pLengthArr[j];

            hr = _pScrn->pD3DDevice->DrawPrimitiveStrided(trilisttype, fvf_flags, &dps_data, cCurNumStripVerts, NULL);
            TestDrawPrimFailure(DrawPrimStrided,hr,_pScrn->pDD,cCurNumStripVerts,cCurNumStripVerts-2);

            dps_data.position.lpvData = (VOID*)(((char*) dps_data.position.lpvData) + cCurNumStripVerts*dps_data.position.dwStride);
            dps_data.diffuse.lpvData = (VOID*)(((char*) dps_data.diffuse.lpvData) + cCurNumStripVerts*dps_data.diffuse.dwStride);
            dps_data.normal.lpvData = (VOID*)(((char*) dps_data.normal.lpvData) + cCurNumStripVerts*dps_data.normal.dwStride);
            dps_data.textureCoords[0].lpvData = (VOID*)(((char*) dps_data.textureCoords[0].lpvData) + cCurNumStripVerts*dps_data.textureCoords[0].dwStride);
        }

        nassertv(_pCurFvfBufPtr == NULL);
    }
#endif
}

//-----------------------------------------------------------------------------
// Name: GenerateSphere()
// Desc: Makes vertex and index data for ellipsoid w/scaling factors sx,sy,sz
//       tries to match gluSphere behavior
//-----------------------------------------------------------------------------

void DXGraphicsStateGuardian7::
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
//     Function: DXGraphicsStateGuardian7::draw_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_sphere(GeomSphere *geom, GeomContext *gc) {

#define SPHERE_NUMSLICES 16
#define SPHERE_NUMSTACKS 10

#ifdef GSG_VERBOSE
    dxgsg7_cat.debug() << "draw_sphere()" << endl;
#endif
    DO_PSTATS_STUFF(PStatTimer timer(_draw_primitive_pcollector));
    DO_PSTATS_STUFF(_vertices_other_pcollector.add_level(geom->get_num_vertices()));

    int nprims = geom->get_num_prims();

    if (nprims==0) {
        dxgsg7_cat.warning() << "draw_sphere() called with ZERO vertices!!" << endl;
        return;
    }

    Geom::VertexIterator vi = geom->make_vertex_iterator();
    Geom::ColorIterator ci;
    bool bperPrimColor = (geom->get_binding(G_COLOR) == G_PER_PRIM);
    if (bperPrimColor)
        ci = geom->make_color_iterator();

    _perVertex = 0x0;
    _perPrim = 0x0;
    _perComp = 0x0;

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
        HRESULT hr = _pScrn->pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,  _curFVFflags, _pFvfBufBasePtr, nVerts, _index_buf,nIndices,NULL);
        TestDrawPrimFailure(DrawIndexedPrim,hr,_pScrn->pDD,nVerts,(nIndices>>2));
    }

    _pCurFvfBufPtr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::prepare_texture
//       Access: Public, Virtual
//  Description: Creates a new retained-mode representation of the
//               given texture, and returns a newly-allocated
//               TextureContext pointer to reference it.  It is the
//               responsibility of the calling function to later
//               call release_texture() with this same pointer (which
//               will also delete the pointer).
////////////////////////////////////////////////////////////////////
TextureContext *DXGraphicsStateGuardian7::
prepare_texture(Texture *tex) {

    DXTextureContext7 *dtc = new DXTextureContext7(tex);
#ifdef USE_TEXFMTVEC
    if (dtc->CreateTexture(_pScrn->pD3DDevice,_pScrn->TexPixFmts,&_pScrn->D3DDevDesc) == NULL) {
#else
    if (dtc->CreateTexture(_pScrn->pD3DDevice,_cNumTexPixFmts,_pTexPixFmts,&_pScrn->D3DDevDesc) == NULL) {
#endif
        delete dtc;
        return NULL;
    }

    return dtc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::apply_texture
//       Access: Public, Virtual
//  Description: Makes the texture the currently available texture for
//               rendering.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
apply_texture(TextureContext *tc) {
    if (tc==NULL) {
        return;  // use enable_texturing to disable/enable
    }
    #ifdef DO_PSTATS
       add_to_texture_record(tc);
    #endif

//  bind_texture(tc);

//  specify_texture(tc->_texture);
    // Note: if this code changes, make sure to change initialization SetTSS code in dx_init as well
    // so DX TSS renderstate matches dxgsg7 state

    DXTextureContext7 *dtc = DCAST(DXTextureContext7, tc);

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
            dxgsg7_cat.warning()
              << "Texture " << *dtc->_texture << " has changed mipmap state.\n";
          }

          dtc->DeleteTexture();
#ifdef USE_TEXFMTVEC
          if (dtc->CreateTexture(_pScrn->pD3DDevice,_pScrn->TexPixFmts,&_pScrn->D3DDevDesc) == NULL) {
#else
          if (dtc->CreateTexture(_pScrn->pD3DDevice,_cNumTexPixFmts,_pTexPixFmts,&_pScrn->D3DDevDesc) == NULL) {
#endif
            // Oops, we can't re-create the texture for some reason.
            dxgsg7_cat.error() << "Unable to re-create texture " << *dtc->_texture << endl;

            release_texture(dtc);
            enable_texturing(false);
            return;
          }
      }
      dtc->clear_dirty_flags();
    } else {
       if(_pCurTexContext == dtc) {
          return;  // tex already set (and possible problem in state-sorting?)
       }
    }

    Texture *tex = tc->_texture;
    Texture::WrapMode wrapU,wrapV;
    wrapU=tex->get_wrapu();
    wrapV=tex->get_wrapv();

    if (wrapU!=_CurTexWrapModeU) {
        _pScrn->pD3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,get_texture_wrap_mode(wrapU));
        _CurTexWrapModeU = wrapU;
    }
    if (wrapV!=_CurTexWrapModeV) {
        _pScrn->pD3DDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,get_texture_wrap_mode(wrapV));
        _CurTexWrapModeV = wrapV;
    }

    uint aniso_degree=tex->get_anisotropic_degree();
    if(_CurTexAnisoDegree != aniso_degree) {
        _pScrn->pD3DDevice->SetTextureStageState(0,D3DTSS_MAXANISOTROPY,aniso_degree);
        _CurTexAnisoDegree = aniso_degree;
    }

    Texture::FilterType ft=tex->get_magfilter();

    D3DTEXTUREMAGFILTER newMagFilter;
    if (aniso_degree<=1) {
        newMagFilter=((ft!=Texture::FT_nearest) ? D3DTFG_LINEAR : D3DTFG_POINT);

        #ifdef _DEBUG
        if((ft!=Texture::FT_linear)&&(ft!=Texture::FT_nearest)) {
             dxgsg7_cat.error() << "MipMap filter type setting for texture magfilter makes no sense,  texture: " << tex->get_name() << "\n";
        }
        #endif
    } else {
        newMagFilter=D3DTFG_ANISOTROPIC;
    }

    if(_CurTexMagFilter!=newMagFilter) {
        _CurTexMagFilter=newMagFilter;
        _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, newMagFilter);
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

    D3DTEXTUREMIPFILTER newMipFilter = PandaToD3DMipType[(DWORD)ft];

    #ifndef NDEBUG
       // sanity check
       extern char *PandaFilterNameStrs[];
       if((!(dtc->_bHasMipMaps))&&(newMipFilter!=D3DTFP_NONE)) {
                dxgsg7_cat.error() << "Trying to set mipmap filtering for texture with no generated mipmaps!! texname[" << tex->get_name() << "], filter("<<PandaFilterNameStrs[ft]<<")\n";
                newMipFilter=D3DTFP_NONE;
       }
    #endif


    D3DTEXTUREMINFILTER newMinFilter = PandaToD3DMinType[(DWORD)ft];

    if(aniso_degree>=2) {
        newMinFilter=D3DTFN_ANISOTROPIC;
    }

    if(newMinFilter!=_CurTexMinFilter) {
        _CurTexMinFilter = newMinFilter;
        _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, newMinFilter);
    }

    if(newMipFilter!=_CurTexMipFilter) {
        _CurTexMipFilter = newMipFilter;
        _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, newMipFilter);
    }

    // bugbug:  does this handle the case of untextured geometry?
    //          we dont see this bug cause we never mix textured/untextured
    _pScrn->pD3DDevice->SetTexture(0,dtc->_surface);

#if 0
    if (dtc!=NULL) {
        dxgsg7_cat.spam() << "Setting active DX texture: " << dtc->_tex->get_name() << "\n";
    }
#endif

    _pCurTexContext = dtc;   // enable_texturing needs this
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
release_texture(TextureContext *tc) {
    DXTextureContext7 *gtc = DCAST(DXTextureContext7, tc);
    gtc->DeleteTexture();
    delete gtc;
}

void DXGraphicsStateGuardian7::
copy_texture(Texture *tex, const DisplayRegion *dr) {
    dxgsg7_cat.error() << "DX copy_texture unimplemented!!!";
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::copy_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
copy_texture(Texture *tex, const DisplayRegion *dr, const RenderBuffer &rb) {
    dxgsg7_cat.error() << "DX copy_texture unimplemented!!!";
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::texture_to_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb) {
    dxgsg7_cat.error()
      << "texture_to_pixel_buffer unimplemented!\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::texture_to_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
texture_to_pixel_buffer(TextureContext *tc, PixelBuffer *pb,
                        const DisplayRegion *dr) {
    dxgsg7_cat.error()
      << "texture_to_pixel_buffer unimplemented!\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian7::
copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr) {

    extern HRESULT ConvertDDSurftoPixBuf(PixelBuffer *pixbuf,LPDIRECTDRAWSURFACE7 pDDSurf);

    nassertr(pb != NULL && dr != NULL, false);

    int xo, yo, w, h;
    dr->get_region_pixels_i(xo, yo, w, h);

    // only handled simple case
    nassertr(xo == 0 && yo==0 && w == pb->get_xsize() && h == pb->get_ysize(), false);

/*
    set_pack_alignment(1);
    glReadPixels( pb->get_xorg() + xo, pb->get_yorg() + yo,
                  pb->get_xsize(), pb->get_ysize(),
                  get_external_image_format(pb->get_format()),
                  get_image_type(pb->get_image_type()),
                  pb->_image.p() );
*/


    (void) ConvertDDSurftoPixBuf(pb,((_cur_read_pixel_buffer & RenderBuffer::T_back) ? _pScrn->pddsBack : _pScrn->pddsPrimary));

    nassertr(!pb->_image.empty(), false);
    return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian7::
copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr,
                  const RenderBuffer &rb) {
    set_read_buffer(rb);
    return copy_pixel_buffer(pb, dr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::apply_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::apply_material( const Material* material ) {
    D3DMATERIAL7 cur_material;
    cur_material.dcvDiffuse = *(D3DCOLORVALUE *)(material->get_diffuse().get_data());
    cur_material.dcvAmbient = *(D3DCOLORVALUE *)(material->get_ambient().get_data());
    cur_material.dcvSpecular = *(D3DCOLORVALUE *)(material->get_specular().get_data());
    cur_material.dcvEmissive = *(D3DCOLORVALUE *)(material->get_emission().get_data());
    cur_material.dvPower   =  material->get_shininess();
    _pScrn->pD3DDevice->SetMaterial(&cur_material);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::apply_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
apply_fog(Fog *fog) {
  if(_doFogType==None)
    return;

  Fog::Mode panda_fogmode = fog->get_mode();
  D3DFOGMODE d3dfogmode = get_fog_mode_type(panda_fogmode);


  // should probably avoid doing redundant SetRenderStates, but whatever
  _pScrn->pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)_doFogType, d3dfogmode);

  const Colorf &fog_colr = fog->get_color();
  _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR,
                                  MY_D3DRGBA(fog_colr[0], fog_colr[1], fog_colr[2], 0.0f));  // Alpha bits are not used

  // do we need to adjust fog start/end values based on D3DPRASTERCAPS_WFOG/D3DPRASTERCAPS_ZFOG ?
  // if not WFOG, then docs say we need to adjust values to range [0,1]

  switch (panda_fogmode) {
  case Fog::M_linear:
    {
      float onset, opaque;
      fog->get_linear_range(onset, opaque);

      _pScrn->pD3DDevice->SetRenderState( D3DRENDERSTATE_FOGSTART,
                                       *((LPDWORD) (&onset)) );
      _pScrn->pD3DDevice->SetRenderState( D3DRENDERSTATE_FOGEND,
                                       *((LPDWORD) (&opaque)) );
    }
    break;
  case Fog::M_exponential:
  case Fog::M_exponential_squared:
    {
      // Exponential fog is always camera-relative.
      float fog_density = fog->get_exp_density();
      _pScrn->pD3DDevice->SetRenderState( D3DRENDERSTATE_FOGDENSITY,
                                       *((LPDWORD) (&fog_density)) );
    }
    break;
  }
}

void DXGraphicsStateGuardian7::SetTextureBlendMode(TextureApplyAttrib::Mode TexBlendMode,bool bCanJustEnable) {

/*class TextureApplyAttrib {
  enum Mode {
    M_modulate,M_decal,M_blend,M_replace,M_add};
*/
    static D3DTEXTUREOP TexBlendColorOp1[/* TextureApplyAttrib::Mode maxval*/ 10] =
    {D3DTOP_MODULATE,D3DTOP_BLENDTEXTUREALPHA,D3DTOP_MODULATE,D3DTOP_SELECTARG1,D3DTOP_ADD};

    //if bCanJustEnable, then we only need to make sure ColorOp is turned on and set properly
    if (bCanJustEnable && (TexBlendMode==_CurTexBlendMode)) {
        // just reset COLOROP 0 to enable pipeline, rest is already set properly
        _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, TexBlendColorOp1[TexBlendMode] );
        return;
    }

    _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, TexBlendColorOp1[TexBlendMode] );

    switch (TexBlendMode) {

        case TextureApplyAttrib::M_modulate:
            // emulates GL_MODULATE glTexEnv mode
            // want to multiply tex-color*pixel color to emulate GL modulate blend (see glTexEnv)
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

            break;
        case TextureApplyAttrib::M_decal:
            // emulates GL_DECAL glTexEnv mode
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

            break;
        case TextureApplyAttrib::M_replace:
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            break;
        case TextureApplyAttrib::M_add:
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

            // since I'm making up 'add' mode, use modulate.  "adding" alpha never makes sense right?
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
            _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

            break;
        case TextureApplyAttrib::M_blend:
            dxgsg7_cat.error()
            << "Impossible to emulate GL_BLEND in DX exactly " << (int) TexBlendMode << endl;
/*
           // emulate GL_BLEND glTexEnv

           GL requires 2 independent operations on 3 input vars for this mode
           DX texture pipeline requires re-using input of last stage on each new op, so I dont think
           exact emulation is possible
           _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
           _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT );
           _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

           _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
           _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
           _pScrn->pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

           need to SetTexture(1,tex) also
           _pScrn->pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE ); wrong
           _pScrn->pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
           _pScrn->pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TFACTOR );

           _pScrn->pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
           _pScrn->pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
*/


            break;
        default:
            dxgsg7_cat.error() << "Unknown texture blend mode " << (int) TexBlendMode << endl;
            break;
    }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_texturing
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_texturing(bool val) {
//  if (_texturing_enabled == val) {  // this check is mostly for internal gsg calls, panda already screens out redundant state changes
//        return;
//  }

    _texturing_enabled = val;

//  assert(_pCurTexContext!=NULL);  we're definitely called with it NULL for both true and false
//  I'm going to allow enabling texturing even if no tex has been set yet, seems to cause no probs

    if (val == FALSE) {
        _pScrn->pD3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);
    } else {
          SetTextureBlendMode(_CurTexBlendMode,TRUE);
    }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_transform
//       Access: Public, Virtual
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_transform(const TransformState *transform) {
  DO_PSTATS_STUFF(_transform_state_pcollector.add_level(1));

  _pScrn->pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,
                                (LPD3DMATRIX)transform->get_mat().get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_tex_matrix
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_tex_matrix(const TexMatrixAttrib *attrib) {
  const LMatrix4f &m = attrib->get_mat();

  // This is ugly.  Need to make this a simple boolean test instead of
  // a matrix compare.
  if (m == LMatrix4f::ident_mat()) {
    _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                             D3DTTFF_DISABLE);
  } else {
    // We have to reorder the elements of the matrix for some reason.
    LMatrix4f dm(m(0, 0), m(0, 1), m(0, 3), 0.0f,
                 m(1, 0), m(1, 1), m(1, 3), 0.0f,
                 m(3, 0), m(3, 1), m(3, 3), 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
    _pScrn->pD3DDevice->SetTransform(D3DTRANSFORMSTATE_TEXTURE0,
                                     (D3DMATRIX *)dm.get_data());
    _pScrn->pD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                             D3DTTFF_COUNT2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_texture(const TextureAttrib *attrib) {
  DO_PSTATS_STUFF(_texture_state_pcollector.add_level(1));
  if (attrib->is_off()) {
    enable_texturing(false);
  } else {
    enable_texturing(true);
    Texture *tex = attrib->get_texture();
    nassertv(tex != (Texture *)NULL);

    TextureContext *tc = tex->prepare_now(_prepared_objects, this);
    apply_texture(tc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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
//     Function: DXGraphicsStateGuardian7::issue_render_mode
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_render_mode(const RenderModeAttrib *attrib) {
  RenderModeAttrib::Mode mode = attrib->get_mode();

  switch (mode) {
  case RenderModeAttrib::M_filled:
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
    break;

  case RenderModeAttrib::M_wireframe:
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
    break;

  default:
    dxgsg7_cat.error()
      << "Unknown render mode " << (int)mode << endl;
  }

  _current_fill_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_texture_apply
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_texture_apply(const TextureApplyAttrib *attrib) {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_depth_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_depth_test(const DepthTestAttrib *attrib) {
  DepthTestAttrib::PandaCompareFunc mode = attrib->get_mode();
  if (mode == DepthTestAttrib::M_none) {
    _depth_test_enabled = false;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
  } else {
    _depth_test_enabled = true;
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, (D3DCMPFUNC) mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_alpha_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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
//     Function: DXGraphicsStateGuardian7::issue_depth_write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_depth_write(const DepthWriteAttrib *attrib) {
  enable_zwritemask(attrib->get_mode() == DepthWriteAttrib::M_on);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_cull_face
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_cull_face(const CullFaceAttrib *attrib) {
  CullFaceAttrib::Mode mode = attrib->get_effective_mode();

  switch (mode) {
  case CullFaceAttrib::M_cull_none:
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    break;
  case CullFaceAttrib::M_cull_clockwise:
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
    _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
    break;
  default:
    dxgsg7_cat.error()
      << "invalid cull face mode " << (int)mode << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::issue_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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
//     Function: DXGraphicsStateGuardian7::issue_depth_offset
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
issue_depth_offset(const DepthOffsetAttrib *attrib) {
  int offset = attrib->get_offset();
  _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, offset);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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
  D3DLIGHT7  alight;
  alight.dltType =  D3DLIGHT_POINT;
  alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
  alight.dcvAmbient  =  black ;
  alight.dcvSpecular = *(D3DCOLORVALUE *)(light->get_specular_color().get_data());

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  alight.dvPosition = *(D3DVECTOR *)pos.get_data();

  alight.dvRange =  D3DLIGHT_RANGE_MAX;
  alight.dvFalloff =  1.0f;

  const LVecBase3f &att = light->get_attenuation();
  alight.dvAttenuation0 = (D3DVALUE)att[0];
  alight.dvAttenuation1 = (D3DVALUE)att[1];
  alight.dvAttenuation2 = (D3DVALUE)att[2];

  HRESULT res = _pScrn->pD3DDevice->SetLight(light_id, &alight);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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

  D3DLIGHT7  alight;
  ZeroMemory(&alight, sizeof(D3DLIGHT7));

  alight.dltType =  D3DLIGHT_DIRECTIONAL;
  alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
  alight.dcvAmbient  =  black ;
  alight.dcvSpecular = *(D3DCOLORVALUE *)(light->get_specular_color().get_data());

  alight.dvDirection = *(D3DVECTOR *)dir.get_data();

  alight.dvRange =  D3DLIGHT_RANGE_MAX;
  alight.dvFalloff =  1.0f;

  alight.dvAttenuation0 = 1.0f;       // constant
  alight.dvAttenuation1 = 0.0f;       // linear
  alight.dvAttenuation2 = 0.0f;       // quadratic

  HRESULT res = _pScrn->pD3DDevice->SetLight(light_id, &alight);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
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

  D3DLIGHT7  alight;
  ZeroMemory(&alight, sizeof(D3DLIGHT7));

  alight.dltType =  D3DLIGHT_SPOT;
  alight.dcvAmbient  =  black ;
  alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
  alight.dcvSpecular = *(D3DCOLORVALUE *)(light->get_specular_color().get_data());

  alight.dvPosition = *(D3DVECTOR *)pos.get_data();

  alight.dvDirection = *(D3DVECTOR *)dir.get_data();

  alight.dvRange =  D3DLIGHT_RANGE_MAX;
  alight.dvFalloff =  1.0f;
  alight.dvTheta =  0.0f;
  alight.dvPhi =  lens->get_hfov();

  const LVecBase3f &att = light->get_attenuation();
  alight.dvAttenuation0 = (D3DVALUE)att[0];
  alight.dvAttenuation1 = (D3DVALUE)att[1];
  alight.dvAttenuation2 = (D3DVALUE)att[2];

  HRESULT res = _pScrn->pD3DDevice->SetLight(light_id, &alight);
}

#if 0
////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::begin_frame
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
bool DXGraphicsStateGuardian7::
begin_frame() {
  return GraphicsStateGuardian::begin_frame();
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::begin_scene
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
bool DXGraphicsStateGuardian7::
begin_scene() {
  if (!GraphicsStateGuardian::begin_scene()) {
    return false;
  }

  HRESULT hr = _pScrn->pD3DDevice->BeginScene();

  if (FAILED(hr)) {
    if ((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      if (dxgsg7_cat.is_debug()) {
        dxgsg7_cat.debug()
          << "BeginScene returns " << ConvD3DErrorToString(hr) << endl;
      }
      
      CheckCooperativeLevel();

    } else {
      dxgsg7_cat.error()
        << "BeginScene failed, unhandled error hr == "
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::end_scene
//       Access: Public, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the end of drawing commands for a "scene" (usually a
//               particular DisplayRegion) within a frame.  All 3-D
//               drawing commands, except the clear operation, must be
//               enclosed within begin_scene() .. end_scene().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
end_scene() {
  HRESULT hr = _pScrn->pD3DDevice->EndScene();

  if (FAILED(hr)) {
    if ((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      if (dxgsg7_cat.is_debug()) {
        dxgsg7_cat.debug()
          << "EndScene returns " << ConvD3DErrorToString(hr) << endl;
      }

      CheckCooperativeLevel();

    } else {
      dxgsg7_cat.error()
        << "EndScene failed, unhandled error hr == " 
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }

  GraphicsStateGuardian::end_scene();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
end_frame() {
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

      dxgsg7_cat.debug() << "==================================="
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

  // Note: regular GraphicsWindow::end_frame is being called,
  // but we override gsg::end_frame, so need to explicitly call it here
  // (currently it's an empty fn)
  GraphicsStateGuardian::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::wants_texcoords
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool DXGraphicsStateGuardian7::
wants_texcoords() const {
    return _texturing_enabled;
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::depth_offset_decals
//       Access: Public, Virtual
//  Description: Returns true if this GSG can implement decals using a
//               DepthOffsetAttrib, or false if that is unreliable
//               and the three-step rendering process should be used
//               instead.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian7::
depth_offset_decals() {
  return dx_depth_offset_decals;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::compute_distance_to
//       Access: Public, Virtual
//  Description: This function may only be called during a render
//               traversal; it will compute the distance to the
//               indicated point, assumed to be in modelview
//               coordinates, from the camera plane.
////////////////////////////////////////////////////////////////////
INLINE float DXGraphicsStateGuardian7::
compute_distance_to(const LPoint3f &point) const {
    // In the case of a DXGraphicsStateGuardian7, we know that the
    // modelview matrix already includes the relative transform from the
    // camera, as well as a to-y-up conversion.  Thus, the distance to
    // the camera plane is simply the +z distance.  (negative of gl compute_distance_to,
    // since d3d uses left-hand coords)

    return point[2];
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_draw_buffer
//       Access: Protected
//  Description: Sets up the glDrawBuffer to render into the buffer
//               indicated by the RenderBuffer object.  This only sets
//               up the color bits; it does not affect the depth,
//               stencil, accum layers.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_draw_buffer(const RenderBuffer &rb) {
    dxgsg7_cat.fatal() << "DX set_draw_buffer unimplemented!!!";
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
//     Function: DXGraphicsStateGuardian7::set_read_buffer
//       Access: Protected
//  Description: Vestigial analog of glReadBuffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_read_buffer(const RenderBuffer &rb) {

    if(rb._buffer_type & RenderBuffer::T_front) {
            _cur_read_pixel_buffer=RenderBuffer::T_front;
    } else  if(rb._buffer_type & RenderBuffer::T_back) {
            _cur_read_pixel_buffer=RenderBuffer::T_back;
    } else {
            dxgsg7_cat.error() << "Invalid or unimplemented Argument to set_read_buffer!\n";
    }
    return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::get_texture_wrap_mode
//       Access: Protected
//  Description: Maps from the Texture's internal wrap mode symbols to
//               GL's.
////////////////////////////////////////////////////////////////////
INLINE D3DTEXTUREADDRESS DXGraphicsStateGuardian7::
get_texture_wrap_mode(Texture::WrapMode wm) const {

    if (wm == Texture::WM_clamp)
        return D3DTADDRESS_CLAMP;
    else if (wm != Texture::WM_repeat) {
#ifdef _DEBUG
        dxgsg7_cat.error() << "Invalid or Unimplemented Texture::WrapMode value!\n";
#endif
    }

    return D3DTADDRESS_WRAP;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::get_fog_mode_type
//       Access: Protected
//  Description: Maps from the fog types to gl version
////////////////////////////////////////////////////////////////////
INLINE D3DFOGMODE DXGraphicsStateGuardian7::
get_fog_mode_type(Fog::Mode m) const {
  switch (m) {
  case Fog::M_linear:
    return D3DFOG_LINEAR;
  case Fog::M_exponential:
    return D3DFOG_EXP;
  case Fog::M_exponential_squared:
    return D3DFOG_EXP2;
  }
  dxgsg7_cat.error() << "Invalid Fog::Mode value" << endl;
  return D3DFOG_EXP;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_lighting
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of lighting overall.  This
//               is called by issue_light() according to whether any
//               lights are in use or not.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
enable_lighting(bool enable) {
  _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, (DWORD)enable);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_ambient_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               indicate the color of the ambient light that should
//               be in effect.  This is called by issue_light() after
//               all other lights have been enabled or disabled.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_ambient_light(const Colorf &color) {
  _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENT,
                                  Colorf_to_D3DCOLOR(color));
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated light id.  A specific Light will
//               already have been bound to this id via bind_light().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
enable_light(int light_id, bool enable) {
  HRESULT res = _pScrn->pD3DDevice->LightEnable(light_id, enable);

#ifdef GSG_VERBOSE
  dxgsg7_cat.debug()
    << "LightEnable(" << light_id << "=" << enable << ")" << endl;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::slot_new_clip_plane
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
bool DXGraphicsStateGuardian7::
slot_new_clip_plane(int plane_id) {
  return (plane_id < D3DMAXUSERCLIPPLANES);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_clip_plane
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated clip_plane id.  A specific
//               PlaneNode will already have been bound to this id via
//               bind_clip_plane().
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian7::
enable_clip_plane(int plane_id, bool enable) {
  assert(plane_id < D3DMAXUSERCLIPPLANES);

  DWORD bitflag = ((DWORD)1 << plane_id);
  if (enable) {
    _clip_plane_bits |= bitflag;
  } else {
    _clip_plane_bits &= ~bitflag;
  }

  _pScrn->pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE, _clip_plane_bits);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::bind_clip_plane
//       Access: Protected, Virtual
//  Description: Called the first time a particular clip_plane has been
//               bound to a given id within a frame, this should set
//               up the associated hardware clip_plane with the clip_plane's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
bind_clip_plane(PlaneNode *plane, int plane_id) {
  // Get the plane in "world coordinates".  This means the plane in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  NodePath plane_np(plane);
  const LMatrix4f &plane_mat = plane_np.get_mat(_scene_setup->get_camera_path());
  LMatrix4f rel_mat = plane_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  Planef world_plane = plane->get_plane() * rel_mat;

  _pScrn->pD3DDevice->SetClipPlane(plane_id, (float *)world_plane.get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_blend_mode
//       Access: Protected, Virtual
//  Description: Called after any of these three blending states have
//               changed; this function is responsible for setting the
//               appropriate color blending mode based on the given
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_blend_mode(ColorWriteAttrib::Mode color_write_mode,
               ColorBlendAttrib::Mode color_blend_mode,
               TransparencyAttrib::Mode transparency_mode) {
  // If color_write_mode is off, we disable writing to the color using
  // blending.  I don't know if it is possible in DX to disable color
  // outside of a blend mode.
  if (color_write_mode == ColorWriteAttrib::M_off) {
    enable_blend(true);
    call_dxBlendFunc(D3DBLEND_ZERO, D3DBLEND_ONE);
    return;
  }
  
  // Is there a color blend set?
  if (color_blend_mode != ColorBlendAttrib::M_none) {
    enable_blend(true);

    // DX7 supports only ColorBlendAttrib::M_add.  Assume that's what
    // we've got; if the user asked for anything else, give him M_add
    // instead.
  
    call_dxBlendFunc(get_blend_func(_color_blend->get_operand_a()),
                     get_blend_func(_color_blend->get_operand_b()));
    return;
  }

  // No color blend; is there a transparency set?
  switch (transparency_mode) {
  case TransparencyAttrib::M_none:
  case TransparencyAttrib::M_binary:
    break;

  case TransparencyAttrib::M_alpha:
  case TransparencyAttrib::M_multisample:
  case TransparencyAttrib::M_multisample_mask:
  case TransparencyAttrib::M_dual:
    enable_blend(true);
    call_dxBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
    return;

  default:
    dxgsg7_cat.error()
      << "invalid transparency mode " << (int)transparency_mode << endl;
    break;
  }

  // Nothing's set, so disable blending.
  enable_blend(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::free_pointers
//       Access: Public
//  Description: Frees some memory that was explicitly allocated
//               within the dxgsg7.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
free_pointers() {
#ifdef USE_TEXFMTVEC
    _pScrn->TexPixFmts.clear();
#else
    SAFE_DELETE_ARRAY(_pTexPixFmts);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::save_frame_buffer
//       Access: Public
//  Description: Saves the indicated planes of the frame buffer
//               (within the indicated display region) and returns it
//               in some meaningful form that can be restored later
//               via restore_frame_buffer().  This is a helper
//               function for push_frame_buffer() and
//               pop_frame_buffer().
////////////////////////////////////////////////////////////////////
PT(SavedFrameBuffer) DXGraphicsStateGuardian7::
save_frame_buffer(const RenderBuffer &buffer,
                  CPT(DisplayRegion) dr) {

    dxgsg7_cat.error() << "save_frame_buffer unimplemented!!\n";
    return NULL;

#if 0
    DXSavedFrameBuffer7 *sfb = new DXSavedFrameBuffer7(buffer, dr);

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
//     Function: DXGraphicsStateGuardian7::restore_frame_buffer
//       Access: Public
//  Description: Restores the frame buffer that was previously saved.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
restore_frame_buffer(SavedFrameBuffer *frame_buffer) {
    dxgsg7_cat.error() << "restore_frame_buffer unimplemented!!\n";
    return;
}

TypeHandle DXGraphicsStateGuardian7::get_type(void) const {
    return get_class_type();
}

TypeHandle DXGraphicsStateGuardian7::get_class_type(void) {
    return _type_handle;
}

void DXGraphicsStateGuardian7::init_type(void) {
    GraphicsStateGuardian::init_type();
    register_type(_type_handle, "DXGraphicsStateGuardian7",
                  GraphicsStateGuardian::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: dx_cleanup
//  Description: Clean up the DirectX environment.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
dx_cleanup(bool bRestoreDisplayMode,bool bAtExitFnCalled) {
  static bool bAtExitFnEverCalled=false;

    if(dxgsg7_cat.is_spam()) {
        dxgsg7_cat.spam() << "dx_cleanup called, bAtExitFnCalled=" << bAtExitFnCalled << ", bAtExitFnEverCalled=" << bAtExitFnEverCalled << endl;
    }

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

        PRINTREFCNT(_pScrn->pDD,"exit start IDirectDraw7");

        // Do a safe check for releasing the D3DDEVICE. RefCount should be zero.
        // if we're called from exit(), _pScrn->pD3DDevice may already have been released
        if (_pScrn->pD3DDevice!=NULL) {
            _pScrn->pD3DDevice->SetTexture(0,NULL);  // should release this stuff internally anyway
            RELEASE(_pScrn->pD3DDevice,dxgsg7,"d3dDevice",RELEASE_DOWN_TO_ZERO);
        }

        PRINTREFCNT(_pScrn->pDD,"after d3ddevice release IDirectDraw7");

        if((_pScrn->pddsBack!=NULL)&&(_pScrn->pddsZBuf!=NULL))
            _pScrn->pddsBack->DeleteAttachedSurface(0x0,_pScrn->pddsZBuf);

        // Release the DDraw and D3D objects used by the app
        RELEASE(_pScrn->pddsZBuf,dxgsg7,"zbuffer",false);

        PRINTREFCNT(_pScrn->pDD,"before releasing d3d obj, IDirectDraw7");
        RELEASE(_pScrn->pD3D,dxgsg7,"IDirect3D7 _pScrn->pD3D",false); //RELEASE_DOWN_TO_ZERO);
        PRINTREFCNT(_pScrn->pDD,"after releasing d3d obj, IDirectDraw7");

        // is it wrong to explictly release _pScrn->pddsBack if it is part of complex surface chain (as in full_pScrn->mode)?
        RELEASE(_pScrn->pddsBack,dxgsg7,"backbuffer",false);
        RELEASE(_pScrn->pddsPrimary,dxgsg7,"primary surface",false);

        PRINTREFCNT(_pScrn->pDD,"after releasing all surfs, IDirectDraw7");
    }

    // for some reason, DLL_PROCESS_DETACH has not yet been sent to ddraw, so we can still call its fns

    // Do a safe check for releasing DDRAW. RefCount should be zero.
    if (_pScrn->pDD!=NULL) {
        if(bRestoreDisplayMode) {
          HRESULT hr = _pScrn->pDD->RestoreDisplayMode();
          if(dxgsg7_cat.is_spam())
                dxgsg7_cat.spam() << "dx_cleanup -  Restoring original desktop DisplayMode\n";
          if(FAILED(hr)) {
                dxgsg7_cat.error() << "dx_cleanup -  RestoreDisplayMode failed, hr = " << ConvD3DErrorToString(hr) << endl;
          }
        }

        if(bAtExitFnCalled) {
           // if exit() called, there is definitely no more need for the IDDraw object,
           // so we can make sure it's fully released
           // note currently this is never called
           RELEASE(_pScrn->pDD,dxgsg7,"IDirectDraw7 _pScrn->pDD", RELEASE_DOWN_TO_ZERO);
        } else {
           // seems wrong to release to zero, since it might be being used somewhere else?

           RELEASE(_pScrn->pDD,dxgsg7,"IDirectDraw7 _pScrn->pDD", false);
           if(refcnt>0) {
              if(dxgsg7_cat.is_spam())
                dxgsg7_cat.debug() << "dx_cleanup -  warning IDDraw7 refcnt = " << refcnt << ", should be zero!\n";
           }
        }
    }
}

////////////////////////////////////////////////////////////////////
//     Function: dx_setup_after_resize
//  Description: Recreate the back buffer and zbuffers at the new size
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
dx_setup_after_resize(RECT *pViewRect) {
    if (_pScrn->pddsBack == NULL) // nothing created yet
        return;

    // for safety, need some better error-cleanup here
    assert((_pScrn->pddsPrimary!=NULL) && (_pScrn->pddsBack!=NULL) && (_pScrn->pddsZBuf!=NULL));

    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_back);
    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd_zbuf);

    _pScrn->pddsBack->GetSurfaceDesc(&ddsd_back);
    _pScrn->pddsZBuf->GetSurfaceDesc(&ddsd_zbuf);

    ULONG refcnt;

    if((_pScrn->pddsBack!=NULL)&&(_pScrn->pddsZBuf!=NULL))
        _pScrn->pddsBack->DeleteAttachedSurface(0x0,_pScrn->pddsZBuf);

    RELEASE(_pScrn->pddsZBuf,dxgsg7,"zbuffer",false);
    RELEASE(_pScrn->pddsBack,dxgsg7,"backbuffer",false);
    RELEASE(_pScrn->pddsPrimary,dxgsg7,"primary surface",false);

    assert((_pScrn->pddsPrimary == NULL) && (_pScrn->pddsBack == NULL) && (_pScrn->pddsZBuf == NULL));
    _pScrn->view_rect = *pViewRect;

    DWORD renderWid = _pScrn->view_rect.right - _pScrn->view_rect.left;
    DWORD renderHt = _pScrn->view_rect.bottom - _pScrn->view_rect.top;

    ddsd_back.dwWidth  = ddsd_zbuf.dwWidth = renderWid;
    ddsd_back.dwHeight = ddsd_zbuf.dwHeight = renderHt;

    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);

    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    ddsd.dwFlags        = DDSD_CAPS;

    PRINTVIDMEM(_pScrn->pDD,&ddsd.ddsCaps,"resize primary surf");
    HRESULT hr;

    if (FAILED(hr = _pScrn->pDD->CreateSurface( &ddsd, &_pScrn->pddsPrimary, NULL ))) {
        dxgsg7_cat.fatal() << "resize() - CreateSurface failed for primary : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    if (!_pScrn->bIsFullScreen) {
        // Create a clipper object which handles all our clipping for cases when
        // our window is partially obscured by other windows.
        LPDIRECTDRAWCLIPPER Clipper;

        if (FAILED(hr = _pScrn->pDD->CreateClipper( 0, &Clipper, NULL ))) {
            dxgsg7_cat.fatal()
            << "CreateClipper after resize failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        // Associate the clipper with our window. Note that, afterwards, the
        // clipper is internally referenced by the primary surface, so it is safe
        // to release our local reference to it.
        Clipper->SetHWnd( 0, _pScrn->hWnd );
        _pScrn->pddsPrimary->SetClipper( Clipper );
        Clipper->Release();
    }

    // Recreate the backbuffer. (might want to handle failure due to
    // running out of video memory)

    ddsd_back.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;  // just to make sure
    ddsd_back.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

    PRINTVIDMEM(_pScrn->pDD,&ddsd_back.ddsCaps,"resize backbuffer surf");

    if (FAILED(hr = _pScrn->pDD->CreateSurface( &ddsd_back, &_pScrn->pddsBack, NULL ))) {
        dxgsg7_cat.fatal() << "resize() - CreateSurface failed for backbuffer : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    PRINTVIDMEM(_pScrn->pDD,&ddsd_back.ddsCaps,"resize zbuffer surf");

    // Recreate and attach a z-buffer.
    if (FAILED(hr = _pScrn->pDD->CreateSurface( &ddsd_zbuf, &_pScrn->pddsZBuf, NULL ))) {
        dxgsg7_cat.fatal() << "resize() - CreateSurface failed for Z buffer: result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    // Attach the z-buffer to the back buffer.
    if ((hr = _pScrn->pddsBack->AddAttachedSurface( _pScrn->pddsZBuf ) ) != DD_OK) {
        dxgsg7_cat.fatal() << "resize() - AddAttachedSurface failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    if ((hr = _pScrn->pD3DDevice->SetRenderTarget(_pScrn->pddsBack,0x0) ) != DD_OK) {
        dxgsg7_cat.fatal() << "resize() - SetRenderTarget failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    // It doesn't seem necessary to create a new viewport for the
    // window, since Panda creates a new viewport before rendering
    // into each DisplayRegion.  Creating this redundant viewport may
    // just lead to confusion.
    /*
    // Create the viewport
    D3DVIEWPORT7 vp = { 
      0, 0,
      renderWid, renderHt, 
      0.0f, 1.0f
    };
    hr = _pScrn->pD3DDevice->SetViewport( &vp );
    if (hr != DD_OK) {
        dxgsg7_cat.fatal()
        << "SetViewport failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }
    */

//    _dxgsg->set_context(&_wcontext); 
}

bool refill_tex_callback(TextureContext *tc,void *void_dxgsg7_ptr) {
     DXTextureContext7 *dtc = DCAST(DXTextureContext7, tc);
//   DXGraphicsStateGuardian7 *dxgsg7 = (DXGraphicsStateGuardian7 *)void_dxgsg7_ptr; not needed?

     // Re-fill the contents of textures and vertex buffers
     // which just got restored now.
     HRESULT hr=dtc->FillDDSurfTexturePixels();
     return hr==S_OK;
}

bool delete_tex_callback(TextureContext *tc,void *void_dxgsg7_ptr) {
     DXTextureContext7 *dtc = DCAST(DXTextureContext7, tc);

     // release DDSurf (but not the texture context)
     dtc->DeleteTexture();
     return true;
}

bool recreate_tex_callback(TextureContext *tc,void *void_dxgsg7_ptr) {
     DXTextureContext7 *dtc = DCAST(DXTextureContext7, tc);
     DXGraphicsStateGuardian7 *dxgsg7 = (DXGraphicsStateGuardian7 *)void_dxgsg7_ptr;

     // Re-fill the contents of textures and vertex buffers
     // which just got restored now.

     LPDIRECTDRAWSURFACE7 ddtex =
#ifdef USE_TEXFMTVEC
        dtc->CreateTexture(dxgsg7->_pScrn->pD3DDevice,_pScrn->TexPixFmts,&dxgsg7->_pScrn->D3DDevDesc);
#else
        dtc->CreateTexture(dxgsg7->_pScrn->pD3DDevice,dxgsg7->_cNumTexPixFmts,dxgsg7->_pTexPixFmts,&dxgsg7->_pScrn->D3DDevDesc);
#endif
     return ddtex!=NULL;
}

// release all textures and vertex/index buffers
HRESULT DXGraphicsStateGuardian7::DeleteAllVideoSurfaces(void) {
  // BUGBUG: need to handle vertexbuffer handling here

  // cant access template in libpanda.dll directly due to vc++ limitations, use traverser to get around it
  traverse_prepared_textures(delete_tex_callback,this);

  if(dxgsg7_cat.is_debug())
      dxgsg7_cat.debug() << "release of all textures complete\n";
  return S_OK;
}

// recreate all textures and vertex/index buffers
HRESULT DXGraphicsStateGuardian7::RecreateAllVideoSurfaces(void) {
  // BUGBUG: need to handle vertexbuffer handling here

  // cant access template in libpanda.dll directly due to vc++ limitations, use traverser to get around it
  traverse_prepared_textures(recreate_tex_callback,this);

  if(dxgsg7_cat.is_debug())
      dxgsg7_cat.debug() << "recreation of all textures complete\n";
  return S_OK;
}

HRESULT DXGraphicsStateGuardian7::RestoreAllVideoSurfaces(void) {
  // BUGBUG: this should also restore vertex buffer contents when they are implemented
  // You will need to destroy and recreate
  // optimized vertex buffers however, restoring is not enough.

  HRESULT hr;

  // note: could go through and just restore surfs that return IsLost() true
  // apparently that isnt as reliable w/some drivers tho
  if (FAILED(hr = _pScrn->pDD->RestoreAllSurfaces() )) {
        dxgsg7_cat.fatal() << "RestoreAllSurfs failed : result = " << ConvD3DErrorToString(hr) << endl;
    exit(1);
  }

  // cant access template in libpanda.dll directly due to vc++ limitations, use traverser to get around it
  traverse_prepared_textures(refill_tex_callback,this);

  if(dxgsg7_cat.is_debug())
      dxgsg7_cat.debug() << "restore and refill of video surfaces complete...\n";
  return S_OK;
}


////////////////////////////////////////////////////////////////////
//     Function: show_frame
//       Access:
//       Description:   Repaint primary buffer from back buffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::show_frame(void) {
  if(_pScrn->pddsPrimary==NULL)
    return;

  //  DO_PSTATS_STUFF(PStatTimer timer(_win->_swap_pcollector));  // this times just the flip, so it must go here in dxgsg7, instead of wdxdisplay, which would time the whole frame

  if (_pScrn->bIsFullScreen) {
    show_full_screen_frame();
  } else {
    show_windowed_frame();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: dxGraphicsStateGuardian7::support_overlay_window
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
void DXGraphicsStateGuardian7::
support_overlay_window(bool flag) {
  if (!flag) {
    // Disable support for overlay windows.
    _overlay_windows_supported = false;

    if (_pScrn != (DXScreenData *)NULL && _pScrn->bIsFullScreen) {
      _pScrn->pddsPrimary->SetClipper(NULL);
    }

  } else {
    // Enable support for overlay windows.
    _overlay_windows_supported = true;

    if (_pScrn != (DXScreenData *)NULL && _pScrn->bIsFullScreen) {
      // Create a Clipper object to blt the whole screen.
      LPDIRECTDRAWCLIPPER Clipper;

      if (_pScrn->pDD->CreateClipper(0, &Clipper, NULL) == DD_OK) {
        Clipper->SetHWnd(0, _pScrn->hWnd);
        _pScrn->pddsPrimary->SetClipper(Clipper);
      }
      _pScrn->pDD->FlipToGDISurface();
      Clipper->Release();
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: show_full_screen_frame
//       Access:
//       Description:   Repaint primary buffer from back buffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::show_full_screen_frame(void) {
  HRESULT hr;

  // Flip the front and back buffers, to make what we just rendered
  // visible.

  if(!_overlay_windows_supported) {
    // Normally, we can just do the fast flip operation.
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
    hr = _pScrn->pddsPrimary->Flip( NULL, dwFlipFlags);
  } else {
      // If we're asking for overlay windows, we have to blt instead of
      // flip, so we don't lose the window.
      hr = _pScrn->pddsPrimary->Blt( NULL, _pScrn->pddsBack,  NULL, DDBLT_WAIT, NULL );
  }

  if(FAILED(hr)) {
      if((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
        CheckCooperativeLevel();
      } else {
        dxgsg7_cat.error() << "show_frame() - Flip failed w/unexpected error code: " << ConvD3DErrorToString(hr) << endl;
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
void DXGraphicsStateGuardian7::show_windowed_frame(void) {
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

  hr = _pScrn->pddsPrimary->Blt( &_pScrn->view_rect, _pScrn->pddsBack,  NULL, DDBLT_DDFX | DDBLT_WAIT, &bltfx );

  if (dx_sync_video) {
    HRESULT hr = _pScrn->pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
    if(hr != DD_OK) {
      dxgsg7_cat.error() << "WaitForVerticalBlank() failed : " << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }

  if(FAILED(hr)) {
    if((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      CheckCooperativeLevel();
    } else {
      dxgsg7_cat.error() << "show_frame() - Blt failed : " << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }

}

bool DXGraphicsStateGuardian7::
CheckCooperativeLevel(bool bDoReactivateWindow) {
  HRESULT hr = _pScrn->pDD->TestCooperativeLevel();

  if (SUCCEEDED(_last_testcooplevel_result)) {
    if (SUCCEEDED(hr)) {
      // this means this was just a safety check, dont need to restore surfs
      return true;
    }

    // otherwise something just went wrong
    if (hr == DDERR_NOEXCLUSIVEMODE || hr == DDERR_EXCLUSIVEMODEALREADYSET) {
      // This means that mode changes had taken place, surfaces
      // were lost but still we are in the original mode, so we
      // simply restore all surfaces and keep going.

      if (dxgsg7_cat.is_debug()) {
        dxgsg7_cat.debug()
          << "Another app has DDRAW exclusive mode, waiting...\n";
      }

      // This doesn't seem to be necessary.
      /*
      if (_dx_ready) {
        _win->deactivate_window();
        _dx_ready = FALSE;
      }
      */

    } else if (hr == DDERR_WRONGMODE) {
      // This means that the desktop mode has changed.  need to
      // destroy all of dx stuff and recreate everything back again,
      // which is a big hit
      dxgsg7_cat.error()
        << "detected display mode change in TestCoopLevel, must recreate all DDraw surfaces, D3D devices, this is not handled yet. hr == " << ConvD3DErrorToString(hr) << endl;
      exit(1);
    } else if (FAILED(hr)) {
      dxgsg7_cat.error()
        << "unexpected return code from TestCoopLevel: " 
        << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }

  } else {
    // testcooplvl was failing, handle case where it now succeeds

    if (SUCCEEDED(hr)) {
      if (_last_testcooplevel_result == DDERR_EXCLUSIVEMODEALREADYSET ||
          _last_testcooplevel_result == DDERR_NOEXCLUSIVEMODE) {
        if (dxgsg7_cat.is_debug()) {
          dxgsg7_cat.debug()
            << "other app relinquished exclusive mode, refilling surfs...\n";
        }
      }

      // This isn't necessary either.
      /*
      if (bDoReactivateWindow) {
        _win->reactivate_window();
      }
      */

      RestoreAllVideoSurfaces();

      //      _dx_ready = TRUE;

    } else if (hr == DDERR_WRONGMODE) {
      // This means that the desktop mode has changed.  need to
      // destroy all of dx stuff and recreate everything back again,
      // which is a big hit
      dxgsg7_cat.error()
        << "detected desktop display mode change in TestCoopLevel, must recreate all DDraw surfaces & D3D devices, this is not handled yet.  " << ConvD3DErrorToString(hr) << endl;
      //_win->close_window();
      exit(1);
    } else if ((hr!=DDERR_NOEXCLUSIVEMODE) && (hr!=DDERR_EXCLUSIVEMODEALREADYSET)) {
      dxgsg7_cat.error() 
        << "unexpected return code from TestCoopLevel: " << ConvD3DErrorToString(hr) << endl;
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
void DXGraphicsStateGuardian7::adjust_view_rect(int x, int y) {
    if (_pScrn->view_rect.left != x || _pScrn->view_rect.top != y) {
        _pScrn->view_rect.right = x + _pScrn->view_rect.right - _pScrn->view_rect.left;
        _pScrn->view_rect.left = x;
        _pScrn->view_rect.bottom = y + _pScrn->view_rect.bottom - _pScrn->view_rect.top;
        _pScrn->view_rect.top = y;

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
void DXGraphicsStateGuardian7::read_mipmap_images(Texture *tex) {
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

