// Filename: dxGraphicsStateGuardian.cxx
// Created by:  mike (02Feb99)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include "dxSavedFrameBuffer.h"
#include "config_dxgsg.h"
#include "dxGraphicsStateGuardian.h"
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
#include <transformAttribute.h>
#include <transformTransition.h>
#include <colorAttribute.h>
#include <colorTransition.h>
#include <lightAttribute.h>
#include <lightTransition.h>
#include <textureAttribute.h>
#include <textureTransition.h>
#include <renderModeAttribute.h>
#include <renderModeTransition.h>
#include <materialAttribute.h>
#include <materialTransition.h>
#include <colorBlendAttribute.h>
#include <colorBlendTransition.h>
#include <colorMaskAttribute.h>
#include <colorMaskTransition.h>
#include <texMatrixAttribute.h>
#include <texMatrixTransition.h>
#include <texGenAttribute.h>
#include <texGenTransition.h>
#include <textureApplyAttribute.h>
#include <textureApplyTransition.h>
#include <clipPlaneAttribute.h>
#include <clipPlaneTransition.h>
#include <transparencyAttribute.h>
#include <transparencyTransition.h>
#include <fogAttribute.h>
#include <fogTransition.h>
#include <linesmoothAttribute.h>
#include <linesmoothTransition.h>
#include <depthTestAttribute.h>
#include <depthTestTransition.h>
#include <depthWriteAttribute.h>
#include <depthWriteTransition.h>
#include <cullFaceAttribute.h>
#include <cullFaceTransition.h>
#include <stencilAttribute.h>
#include <stencilTransition.h>
#include <throw_event.h>
#include <mmsystem.h>
#include <pStatTimer.h>

#define DISABLE_POLYGON_OFFSET_DECALING
// currently doesnt work well enough in toontown models for us to use
// prob is when viewer gets close to decals, they disappear into wall poly, need to investigate

//#define PRINT_TEXSTATS

//const int VERT_BUFFER_SIZE = (8*1024L);
// For sparkle particles, we can have 4 vertices per sparkle, and a 
// particle pool size of 1024 particles

const int VERT_BUFFER_SIZE = (32*6*1024L);

// for sprites, 1000 prims, 6 verts/prim, 24 bytes/vert

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

// debugging-only flag to test non-optimized general geom pipe for all models
//#define DONT_USE_DRAWPRIMSTRIDED

static D3DMATRIX matIdentity;

#define Colorf_to_D3DCOLOR(out_color) (D3DRGBA((out_color)[0], (out_color)[1], (out_color)[2], (out_color)[3]))

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
	activate();
	GraphicsStateGuardian::reset();
	_buffer_mask = 0;

	// All implementations have the following buffers. 
	_buffer_mask = (RenderBuffer::T_color |
					RenderBuffer::T_depth |
					RenderBuffer::T_back  
//					RenderBuffer::T_stencil |
//					RenderBuffer::T_accum 
					);

	_current_projection_mat = LMatrix4f::ident_mat();
	_projection_mat_stack_count = 0;

	_issued_color_enabled = false;

	_buffer_mask &= ~RenderBuffer::T_right;	 // test for these later

	// Set up our clear values to invalid values, so the glClear* calls
	// will be made initially.
	_clear_color_red = -1.0; 
	_clear_color_green = -1.0;
	_clear_color_blue = -1.0; 
	_clear_color_alpha = -1.0;
	_clear_depth = -1.0;
	_clear_stencil = -1;
	_clear_accum_red = -1.0;
	_clear_accum_green = -1.0;
	_clear_accum_blue = -1.0; 
	_clear_accum_alpha = -1.0;
	_line_width = 1.0;
	_point_size = 1.0;
	_depth_mask = false;
	_fog_mode = D3DFOG_EXP;
	_alpha_func = D3DCMP_ALWAYS;
	_alpha_func_ref = 0;
//  _polygon_mode = GL_FILL;

//	_pack_alignment = 4;
//	_unpack_alignment = 4;

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
	HRESULT hr;

	if(dx_show_fps_meter) {
		_start_time = timeGetTime();
		_current_fps = 0.0;
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

	SetRect(&clip_rect, 0,0,0,0);	  // no clip rect set

	_d3dDevice->SetRenderState(D3DRENDERSTATE_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);

	// Lighting, let's turn it off by default
	_lighting_enabled = true;
	enable_lighting(false);

	// Dither, let's turn it off
	_dither_enabled = true;
	enable_dither(false);

	// Stencil test is off by default
	_stencil_test_enabled = false;
//  _stencil_func = D3DCMP_NOTEQUAL;
//  _stencil_op = D3DSTENCILOP_REPLACE;

	// Antialiasing.
	enable_line_smooth(false);
//	enable_multisample(true);

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
	SetRect(&clip_rect, 0,0,0,0);	  // no clip rect set

	// Make sure the GL state matches all of our initial attribute
	// states.
	PT(DepthTestAttribute) dta = new DepthTestAttribute;
	PT(DepthWriteAttribute) dwa = new DepthWriteAttribute;
	PT(CullFaceAttribute) cfa = new CullFaceAttribute;
	PT(LightAttribute) la = new LightAttribute;
	PT(TextureAttribute) ta = new TextureAttribute;

	dta->issue(this);
	dwa->issue(this);
	cfa->issue(this);
	la->issue(this);

	// must do SetTSS here because redundant states are filtered out by our code based on current values above, so
	// initial conditions must be correct

	_CurTexBlendMode = TextureApplyProperty::M_modulate;
	SetTextureBlendMode(_CurTexBlendMode,FALSE);
	_d3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);	// disables texturing

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
	_d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,get_texture_wrap_mode(_CurTexWrapModeU));
	_d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,get_texture_wrap_mode(_CurTexWrapModeV));

	ta->issue(this); // no curtextcontext, this does nothing.  dx should already be properly inited above anyway

#ifdef _DEBUG
	if ((_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS) &&
		(dx_global_miplevel_bias!=0.0)) {
		_d3dDevice->SetTextureStageState(0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&dx_global_miplevel_bias)) );
	}
#endif

	_d3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, _CurShadeMode);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
clear(const RenderBuffer &buffer) {
  //	PStatTimer timer(_win->_clear_pcollector);

	activate();

	nassertv(buffer._gsg == this);
	int buffer_type = buffer._buffer_type;

	int flags = 0;

	if (buffer_type & RenderBuffer::T_depth)
		flags |=  D3DCLEAR_ZBUFFER;
	if (buffer_type & RenderBuffer::T_back)		  //set appropriate flags
		flags |=  D3DCLEAR_TARGET;
	if (buffer_type & RenderBuffer::T_stencil)
		flags |=  D3DCLEAR_STENCIL;

	D3DCOLOR  clear_colr = Colorf_to_D3DCOLOR(_color_clear_value);

	HRESULT  hr = _d3dDevice->Clear(0, NULL, flags, clear_colr,
									(D3DVALUE) _depth_clear_value, (DWORD)_stencil_clear_value);
	if (hr != DD_OK)
		dxgsg_cat.error()
		<< "dxGSG::clear_buffer failed:  Clear returned " << ConvD3DErrorToString(hr) << endl;
	/*  The following line will cause the background to always clear to a medium red
	_color_clear_value[0] = .5;           
	/*  The following lines will cause the background color to cycle from black to red.
	_color_clear_value[0] += .001;
	 if (_color_clear_value[0] > 1.0) _color_clear_value[0] = 0.0;
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
/*
	int l, b, w, h;
	_actual_display_region->get_region_pixels(l, b, w, h);
	GLint x = GLint(l);
	GLint y = GLint(b);
	GLsizei width = GLsizei(w);
	GLsizei height = GLsizei(h);
#ifdef WBD_GL_MODE
	call_glScissor( x, y, width, height );    
	call_glViewport( x, y, width, height );
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
*/
	}
}



////////////////////////////////////////////////////////////////////
//     Function: set_clipper 
//       Access:
//  Description: Useless in DX at the present time
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::set_clipper(RECT cliprect) {
#if 0
	LPDIRECTDRAWCLIPPER Clipper;
	HRESULT result;

	// For windowed mode, the clip region is associated with the window,
	// and DirectX does not allow you to create clip regions.
	if (dx_full_screen)	return;

	/* The cliprect we receive is normalized so that (0,0) means the upper left of
	   the client portion of the window.
		At least, I think that's true, and the following code assumes that.
		So we must adjust the clip region by offsetting it to the origin of the 
		view rectangle.
	*/
	clip_rect = cliprect;		// store the normalized clip rect
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
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::render_frame
//       Access: Public, Virtual
//  Description: Renders an entire frame, including all display
//               regions within the frame, and includes any necessary
//               pre- and post-processing like swapping buffers.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
render_frame(const AllAttributesWrapper &initial_state) {
	if (!_dx_ready)	return;

	_win->begin_frame();
	_d3dDevice->BeginScene();

/* _d3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matIdentity); */

#ifdef GSG_VERBOSE
	dxgsg_cat.debug()
	<< "begin frame --------------------------------------------" << endl;
#endif

	_decal_level = 0;

	if (_clear_buffer_type != 0) {
	  // First, clear the entire window.
	  PT(DisplayRegion) win_dr = 
	    _win->make_scratch_display_region(_win->get_width(), _win->get_height());
	  clear(get_render_buffer(_clear_buffer_type), win_dr);
	}
	  
	// Now render each of our layers in order.
	int max_channel_index = _win->get_max_channel_index();
	for (int c = 0; c < max_channel_index; c++) {
		if (_win->is_channel_defined(c)) {
			GraphicsChannel *chan = _win->get_channel(c);
			if (chan->is_active()) {
				int num_layers = chan->get_num_layers();
				for (int l = 0; l < num_layers; l++) {
					GraphicsLayer *layer = chan->get_layer(l);
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
								render_scene(cam->get_scene(), cam, initial_state);
								pop_display_region(old_dr);
							}
						}
					}		//      if (layer->is_active()) 
				}
			}		//      if (chan->is_active()) 
		}
	}	//  for (int c = 0; c < max_channel_index; c++) 

	// Now we're done with the frame processing.  Clean up.

	_d3dDevice->EndScene();  // FPS meter drawing MUST occur after EndScene, since it uses GDI

	if(_lighting_enabled) { 
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
		NodeAttributes state;
		state.set_attribute(LightTransition::get_class_type(), new LightAttribute);
		set_state(state, false);
	
		// All this work to undo the lighting state each frame doesn't seem
		// ideal--there may be a better way.  Maybe if the lights were just
		// more aware of whether their parameters or positions have changed
		// at all?
	} 

	if(dx_show_fps_meter) {
		 PStatTimer timer(_win->_show_fps_pcollector);

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

#ifdef PRINT_TEXSTATS
	{

#define TICKSPERINFO (3*1000)
		static DWORD LastTickCount=0;

		DWORD CurTickCount=GetTickCount();

		if (CurTickCount-LastTickCount > TICKSPERINFO) {
			LastTickCount=CurTickCount;
			HRESULT hr;

			D3DDEVINFO_TEXTUREMANAGER tminfo;
			ZeroMemory(&tminfo,sizeof(  D3DDEVINFO_TEXTUREMANAGER));
			hr = _d3dDevice->GetInfo(D3DDEVINFOID_TEXTUREMANAGER,&tminfo,sizeof(D3DDEVINFO_TEXTUREMANAGER));
			if (hr!=D3D_OK) {
				if (hr==S_FALSE)
					dxgsg_cat.error() << "GetInfo requires debug DX7 DLLs to be installed!!\n";
				else dxgsg_cat.error() << "GetInfo appinfo failed : result = " << ConvD3DErrorToString(hr) << endl;
			} else
				dxgsg_cat.spam()
				<< "\n bThrashing:\t" << tminfo.bThrashing
				<< "\n NumEvicts:\t" <<	tminfo.dwNumEvicts
				<< "\n NumVidCreates:\t" <<	tminfo.dwNumVidCreates
				<< "\n NumTexturesUsed:\t" << tminfo.dwNumTexturesUsed
				<< "\n NumUsedTexInVid:\t" << tminfo.dwNumUsedTexInVid
				<< "\n WorkingSet:\t" << tminfo.dwWorkingSet
				<< "\n WorkingSetBytes:\t" << tminfo.dwWorkingSetBytes
				<< "\n TotalManaged:\t"	<< tminfo.dwTotalManaged
				<< "\n TotalBytes:\t" << tminfo.dwTotalBytes
				<< "\n LastPri:\t" << tminfo.dwLastPri		 <<	endl;


			D3DDEVINFO_TEXTURING texappinfo;
			ZeroMemory(&texappinfo,sizeof(  D3DDEVINFO_TEXTURING));
			hr = _d3dDevice->GetInfo(D3DDEVINFOID_TEXTURING,&texappinfo,sizeof(D3DDEVINFO_TEXTURING));
			if (hr!=D3D_OK) {
				if (hr==S_FALSE)
					dxgsg_cat.error() << "GetInfo requires debug DX7 DLLs to be installed!!\n";
				else dxgsg_cat.error() << "GetInfo appinfo failed : result = " << ConvD3DErrorToString(hr) << endl;
			} else
				dxgsg_cat.spam()
				<< "\n NumLoads:\t"	<< texappinfo.dwNumLoads
				<< "\n ApproxBytesLoaded:\t" <<	texappinfo.dwApproxBytesLoaded
				<< "\n NumPreLoads:\t" << texappinfo.dwNumPreLoads
				<< "\n NumSet:\t" << texappinfo.dwNumSet
				<< "\n NumCreates:\t" << texappinfo.dwNumCreates
				<< "\n NumDestroys:\t" << texappinfo.dwNumDestroys
				<< "\n NumSetPriorities:\t"	<< texappinfo.dwNumSetPriorities
				<< "\n NumSetLODs:\t" << texappinfo.dwNumSetLODs
				<< "\n NumLocks:\t"	<< texappinfo.dwNumLocks
				<< "\n NumGetDCs:\t" <<	texappinfo.dwNumGetDCs << endl;
		}

	}
#endif

#ifdef GSG_VERBOSE
	dxgsg_cat.debug()
	<< "end frame ----------------------------------------------" << endl;
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
render_scene(Node *root, ProjectionNode *projnode,
			 const AllAttributesWrapper &initial_state) {
#ifdef GSG_VERBOSE
	_pass_number = 0;
	dxgsg_cat.debug()
	<< "begin scene - - - - - - - - - - - - - - - - - - - - - - - - -" 
	<< endl;
#endif
	_current_root_node = root;

	render_subgraph(_render_traverser, root, projnode, initial_state,
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
				const AllAttributesWrapper &initial_state,
				const AllTransitionsWrapper &net_trans) {
//  activate();  doesnt do anything right now
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
   	     dxgsg_cat.info() << "non w-compliant render_subgraph projection matrix [2][3]: " << projection_mat(2,3) << endl;
  	     dxgsg_cat.info() << "cur projection matrix: " << projection_mat << endl;
	  }

	  // note: a projection matrix that does not have a [3][4] value of 1.0 is
	  //       not w-compliant and could cause problems with fog

	}
#endif

	// We load the projection matrix directly.
	HRESULT res = 
	_d3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,
							 (LPD3DMATRIX) _current_projection_mat.get_data());

	// We infer the modelview matrix by doing a wrt on the projection
	// node.
	LMatrix4f modelview_mat;
	get_rel_mat(subgraph, _current_projection_node, modelview_mat);	 // reversed from GL
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

	render_subgraph(traverser, subgraph, initial_state, sub_trans);

	_current_projection_node = old_projection_node;
	_current_projection_mat = old_projection_mat;
	_projection_mat_stack_count--;


	// We must now restore the projection matrix from before.  We could
	// do a push/pop matrix if we were using D3DX
	if (_projection_mat_stack_count > 0)
		_d3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,
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
				const AllAttributesWrapper &initial_state,
				const AllTransitionsWrapper &net_trans) {
#ifdef GSG_VERBOSE
	dxgsg_cat.debug()
	<< "begin subgraph (pass " << ++_pass_number 
	<< ") - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
#endif
	activate();

	nassertv(traverser != (RenderTraverser *)NULL);
	traverser->traverse(subgraph, initial_state, net_trans);

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

#ifdef _DEBUG
typedef enum {DrawPrim,DrawPrimStrided} DP_Type;

void INLINE TestDrawPrimFailure(DP_Type dptype,HRESULT hr,LPDIRECTDRAW7 pDD) {
		if(FAILED(hr)) {
			// loss of exclusive mode is not a real DrawPrim problem
			HRESULT testcooplvl_hr = pDD->TestCooperativeLevel();
			if((testcooplvl_hr != DDERR_NOEXCLUSIVEMODE)||(testcooplvl_hr != DDERR_EXCLUSIVEMODEALREADYSET)) {
				dxgsg_cat.fatal() << ((dptype==DrawPrimStrided) ? "DrawPrimStrided" : "DrawPrim") << "failed: result = " << ConvD3DErrorToString(hr) << endl;
			}
			exit(1);
		}
}
#else
#define TestDrawPrimFailure(a,b,c)
#endif

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
	    Colorf tempcolor = geom->get_next_color(ci);		                         \
	    transform_color(tempcolor,p_colr);		                                     \
	} else {                                                                         \
		p_color = geom->get_next_color(ci);                                          \
		p_colr = Colorf_to_D3DCOLOR(p_color);                                        \
	}}

////////

	vi = geom->make_vertex_iterator();  
	p_flags = D3DFVF_XYZ;   
	size_t vertex_size = sizeof(D3DVALUE) * 3;  

	if ((geom->get_binding(G_COLOR) != G_OFF) || _issued_color_enabled) {
		ci = geom->make_color_iterator();
		p_flags |= D3DFVF_DIFFUSE;   
		vertex_size += sizeof(D3DCOLOR);  

		if (geom->get_binding(G_COLOR) == G_OVERALL) {
			GET_NEXT_COLOR();
		}

		if (_issued_color_enabled) {
			p_colr = _issued_color;		  // set primitive color if there is one.
			perVertex &= ~PER_COLOR;
			perPrim &= ~PER_COLOR;
			perComp &= ~PER_COLOR;
		}
	}

	if (geom->get_binding(G_NORMAL) != G_OFF) {
		ni = geom->make_normal_iterator();
		p_flags |= D3DFVF_NORMAL;  
		vertex_size += sizeof(D3DVALUE) * 3;  

		if (geom->get_binding(G_NORMAL) == G_OVERALL)
			p_normal = geom->get_next_normal(ni);	 // set overall normal if there is one
	}

	if (geom->get_binding(G_TEXCOORD) != G_OFF) {
		ti = geom->make_texcoord_iterator();
		p_flags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
		vertex_size += sizeof(float) * 2;
	}

	// If we have per-vertex colors or normals, we need smooth shading.
	// Otherwise we want flat shading for performance reasons.
	if (perVertex & ((wants_colors() ? PER_COLOR : 0) | (wants_normals() ? PER_NORMAL : 0)))
		set_shademode(D3DSHADE_GOURAUD);
	else set_shademode(D3DSHADE_FLAT);

	return vertex_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_prim_inner_loop
//       Access: Private
//  Description: This adds data to the flexible vertex format with a check
//               for component normals and color
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_prim_inner_loop(int nVerts, const Geom *geom, DWORD perFlags) {

	for(;nVerts > 0;nVerts--) {

		GET_NEXT_VERTEX();	 // coord info will always be perVertex

		switch (perFlags) {
			case 0x3:
				GET_NEXT_NORMAL();
			case 0x1:
				break;
			case 0x5:
			case 0x4:
				GET_NEXT_COLOR();
				break;
			case 0x7:
			case 0x6:
				GET_NEXT_COLOR();
			case 0x2:
				GET_NEXT_NORMAL();
				break;
			case 0x9:
			case 0x8:
				GET_NEXT_TEXCOORD();
				break;
			case 0xB:
			case 0xA:
				GET_NEXT_NORMAL();
				GET_NEXT_TEXCOORD();
				break;
			case 0xD:
			case 0xC:
				GET_NEXT_COLOR();
				GET_NEXT_TEXCOORD();
				break;
			case 0xF:
			case 0xE:
				GET_NEXT_NORMAL();
				GET_NEXT_COLOR();
				GET_NEXT_TEXCOORD();
				break;
		}

		add_to_FVFBuf((void *)&p_vertex, sizeof(D3DVECTOR));

		if (p_flags & D3DFVF_NORMAL)
			add_to_FVFBuf((void *)&p_normal, sizeof(D3DVECTOR));
		if (p_flags & D3DFVF_DIFFUSE)
			add_DWORD_to_FVFBuf(p_colr);
		if (p_flags & D3DFVF_TEXCOUNT_MASK)
			add_to_FVFBuf((void *)&p_texcoord, sizeof(TexCoordf));
	}
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_point
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_point(const GeomPoint *geom) {
	activate();

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_point()" << endl;
#endif

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
		dxgsg_cat.warning() << "D3D does not support drawing points of non-unit size, setting point size to 1.0!\n";
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
#endif

	// for Indexed Prims and mixed indexed/non-indexed prims, we will use old pipeline for now
	// need to add code to handle fully indexed mode (and handle cases with index arrays of different lengths,
	// values (may only be possible to handle certain cases without reverting to old pipeline)
	if (GeomVrtFmt!=FlatVerts) {

		perVertex = PER_COORD;
		perPrim = 0;
		if (geom->get_binding(G_COORD) == G_PER_VERTEX)	 perVertex |= PER_COORD;
		if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) perVertex |= PER_NORMAL;
		if (geom->get_binding(G_COLOR) == G_PER_VERTEX)	perVertex |= PER_COLOR;

		size_t vertex_size = draw_prim_setup(geom);

		nassertv(_pCurFvfBufPtr == NULL);	 // make sure the storage pointer is clean.
		nassertv(nPrims * vertex_size < VERT_BUFFER_SIZE);
		_pCurFvfBufPtr = _pFvfBufBasePtr;		   // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

		// iterate through the point
		draw_prim_inner_loop(nPrims, geom, perVertex | perPrim);

		HRESULT hr = _d3dDevice->DrawPrimitive(D3DPT_POINTLIST, p_flags, _pFvfBufBasePtr, nPrims, NULL);

	  #ifdef _DEBUG
		if(FAILED(hr)) {
			// loss of exclusive mode is not a real DrawPrim problem
			HRESULT testcooplvl_hr = _pDD->TestCooperativeLevel();
			if((testcooplvl_hr != DDERR_NOEXCLUSIVEMODE)||(testcooplvl_hr != DDERR_EXCLUSIVEMODEALREADYSET)) {
				dxgsg_cat.fatal() << "DrawPrim failed: result = " << ConvD3DErrorToString(hr) << endl;
			}
			exit(1);
		}
	  #endif
	} else {  // setup for strided

		size_t vertex_size = draw_prim_setup(geom);

		// new code only handles non-indexed pointlists (is this enough?)
		nassertv((vindexes==NULL)&&(cindexes==NULL)&&(tindexes==NULL)&&(nindexes==NULL));

		D3DDRAWPRIMITIVESTRIDEDDATA dps_data;
		memset(&dps_data,0,sizeof(D3DDRAWPRIMITIVESTRIDEDDATA));

		dps_data.position.lpvData = (VOID*)coords;
		dps_data.position.dwStride = sizeof(D3DVECTOR);

		if (p_flags & D3DFVF_NORMAL) {
			dps_data.normal.lpvData = (VOID*)norms;
			dps_data.normal.dwStride = sizeof(D3DVECTOR);
		}

		if (p_flags & D3DFVF_DIFFUSE) {
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

		if (p_flags & D3DFVF_TEXCOUNT_MASK) {
			dps_data.textureCoords[0].lpvData = (VOID*)texcoords;
			dps_data.textureCoords[0].dwStride = sizeof(TexCoordf);
		}

		HRESULT hr = _d3dDevice->DrawPrimitiveStrided(D3DPT_POINTLIST, p_flags, &dps_data, nPrims, NULL);
	    TestDrawPrimFailure(DrawPrimStrided,hr,_pDD);
	}

	_pCurFvfBufPtr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_line
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_line(const GeomLine* geom) {
	activate();

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_line()" << endl;
#endif

#ifdef _DEBUG
	static BOOL bPrintedMsg=FALSE;

	if (!bPrintedMsg && (geom->get_width()!=1.0f)) {
		bPrintedMsg=TRUE;
		dxgsg_cat.warning() << "DX does not support drawing lines with a non-1.0 pixel width, setting width to 1.0!\n";
	}
#endif

#ifdef WBD_GL_MODE
	call_glLineWidth(geom->get_width());

	int nprims = geom->get_num_prims();
	Geom::VertexIterator vi = geom->make_vertex_iterator();
	Geom::ColorIterator ci = geom->make_color_iterator();

	GeomIssuer issuer(geom, this,
					  issue_vertex_gl,
					  issue_normal_gl,
					  issue_texcoord_gl,
					  issue_color_gl);

	if (geom->get_binding(G_COLOR) == G_PER_VERTEX) {
		call_glShadeModel(GL_SMOOTH);
	} else {
		call_glShadeModel(GL_FLAT);
	}

	// Draw overall
	issuer.issue_color(G_OVERALL, ci);

	glBegin(GL_LINES);

	for (int i = 0; i < nprims; i++) {
		// Draw per primitive
		issuer.issue_color(G_PER_PRIM, ci);

		for (int j = 0; j < 2; j++) {
			// Draw per vertex
			issuer.issue_color(G_PER_VERTEX, ci);
			issuer.issue_vertex(G_PER_VERTEX, vi);
		}
	}

	glEnd();
#else        // the DX way

	int nPrims = geom->get_num_prims();

	if (nPrims==0) {
		dxgsg_cat.warning() << "draw_line() called with ZERO vertices!!" << endl;
		return;
	}

	perVertex = 0;
	if (geom->get_binding(G_COORD) == G_PER_VERTEX)	 perVertex |= PER_COORD;
	if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) perVertex |= PER_NORMAL;
	if (geom->get_binding(G_COLOR) == G_PER_VERTEX)	perVertex |= PER_COLOR;

	perPrim = 0;
	if (geom->get_binding(G_NORMAL) == G_PER_PRIM) perPrim |= PER_NORMAL;
	if (geom->get_binding(G_COLOR) == G_PER_PRIM) perPrim |= PER_COLOR;

	size_t vertex_size = draw_prim_setup(geom);

	char *_tmp_fvf = NULL;
	nassertv(_pCurFvfBufPtr == NULL);	 // make sure the storage pointer is clean.
//  nassertv(nPrims * 2 * vertex_size < VERT_BUFFER_SIZE);

	if (nPrims * 2 * vertex_size > VERT_BUFFER_SIZE) {
		_pCurFvfBufPtr = _tmp_fvf = new char[nPrims * 2 * vertex_size];
	} else	_pCurFvfBufPtr = _pFvfBufBasePtr;			 // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

	for (int i = 0; i < nPrims; i++) {
		if (perPrim & PER_COLOR) {
			GET_NEXT_COLOR();
		}
		if (perPrim & PER_NORMAL)
			p_normal = geom->get_next_normal(ni);	// set primitive normal if there is one.
		draw_prim_inner_loop(2, geom, perVertex);
	}

	HRESULT hr;

	if (_tmp_fvf == NULL) 
		hr = _d3dDevice->DrawPrimitive(D3DPT_LINELIST, p_flags, _pFvfBufBasePtr, nPrims*2, NULL);
	else {
		hr = _d3dDevice->DrawPrimitive(D3DPT_LINELIST, p_flags, _tmp_fvf, nPrims*2, NULL);
		delete [] _tmp_fvf;
	}
	TestDrawPrimFailure(DrawPrim,hr,_pDD);

	_pCurFvfBufPtr = NULL;

#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_linestrip
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_linestrip(const GeomLinestrip* geom) {
	activate();

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_linestrip()" << endl;
#endif

#ifdef _DEBUG
	static BOOL bPrintedMsg=FALSE;

	if (!bPrintedMsg && (geom->get_width()!=1.0f)) {
		bPrintedMsg=TRUE;
		dxgsg_cat.warning() << "DX does not support drawing lines with a non-1.0 pixel width, setting width to 1.0!\n";
	}
#endif

#ifdef WBD_GL_MODE
	call_glLineWidth(geom->get_width());

	int nprims = geom->get_num_prims();
	const int *plen = geom->get_lengths();
	Geom::VertexIterator vi = geom->make_vertex_iterator();
	Geom::ColorIterator ci = geom->make_color_iterator();

	GeomIssuer issuer(geom, this,
					  issue_vertex_gl,
					  issue_normal_gl,
					  issue_texcoord_gl,
					  issue_color_gl);

	if (geom->get_binding(G_COLOR) == G_PER_VERTEX) {
		call_glShadeModel(GL_SMOOTH);
	} else {
		call_glShadeModel(GL_FLAT);
	}

	// Draw overall
	issuer.issue_color(G_OVERALL, ci);

	for (int i = 0; i < nprims; i++) {
		// Draw per primitive
		issuer.issue_color(G_PER_PRIM, ci);

		int num_verts = *(plen++);
		nassertv(num_verts >= 2);

		glBegin(GL_LINE_STRIP);

		// Per-component attributes for the first line segment?
		issuer.issue_color(G_PER_COMPONENT, ci);

		// Draw the first 2 vertices
		int v;
		for (v = 0; v < 2; v++) {
			issuer.issue_color(G_PER_VERTEX, ci);
			issuer.issue_vertex(G_PER_VERTEX, vi);
		}

		// Now draw each of the remaining vertices.  Each vertex from
		// this point on defines a new line segment.
		for (v = 2; v < num_verts; v++) {
			// Per-component attributes?
			issuer.issue_color(G_PER_COMPONENT, ci);

			// Per-vertex attributes
			issuer.issue_color(G_PER_VERTEX, ci);
			issuer.issue_vertex(G_PER_VERTEX, vi);
		}
		glEnd();
	}
#else
	int nPrims = geom->get_num_prims();
	const int *plen = geom->get_lengths();

	if (nPrims==0) {
		dxgsg_cat.warning() << "draw_linestrip() called with ZERO vertices!!" << endl;
		return;
	}

	perVertex = 0;
	if (geom->get_binding(G_COORD) == G_PER_VERTEX)	 perVertex |= PER_COORD;
	if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) perVertex |= PER_NORMAL;
	if (geom->get_binding(G_COLOR) == G_PER_VERTEX)	perVertex |= PER_COLOR;

	perComp = 0;
	if (geom->get_binding(G_NORMAL) == G_PER_COMPONENT)	perComp |= PER_NORMAL;
	if (geom->get_binding(G_COLOR) == G_PER_COMPONENT) perComp |= PER_COLOR;

	perPrim = 0;
	if (geom->get_binding(G_NORMAL) == G_PER_PRIM) perPrim |= PER_NORMAL;
	if (geom->get_binding(G_COLOR) == G_PER_PRIM) perPrim |= PER_COLOR;

	size_t vertex_size = draw_prim_setup(geom);

	for (int i = 0; i < nPrims; i++) {
		if (perPrim & PER_COLOR) {
			GET_NEXT_COLOR();
		}

		int nVerts = *(plen++);
		nassertv(nVerts >= 2);

		nassertv(_pCurFvfBufPtr == NULL);	 // make sure the storage pointer is clean.
		nassertv(nVerts * vertex_size < VERT_BUFFER_SIZE);
		_pCurFvfBufPtr = _pFvfBufBasePtr;			 // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

		draw_prim_inner_loop(nVerts, geom, perVertex | perComp);

		HRESULT hr = _d3dDevice->DrawPrimitive(D3DPT_LINESTRIP,  p_flags, _pFvfBufBasePtr, nVerts, NULL);
	    TestDrawPrimFailure(DrawPrim,hr,_pDD);

		_pCurFvfBufPtr = NULL;
	}


#endif              // WBD_GL_MODE
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
		return v0.z > v1.z;	// reversed from gl
	}
};
/*
struct draw_sprite_vertex_less {
  INLINE bool operator ()(const WrappedSprite& v0, 
			  const WrappedSprite& v1) const {
	return v0._v[2] > v1._v[2]; // reversed from gl
  }
};
*/

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_sprite
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_sprite(const GeomSprite *geom) {

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

	Texture *tex = geom->get_texture();
	nassertv(tex != (Texture *) NULL);

	// get the array traversal set up.
	int nprims = geom->get_num_prims();

	if (nprims==0) {
		dxgsg_cat.warning() << "draw_sprite() called with ZERO vertices!!" << endl;
		return;
	}

	D3DMATRIX OldD3DWorldMatrix;
	_d3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &OldD3DWorldMatrix);

	Geom::VertexIterator vi = geom->make_vertex_iterator();
	Geom::ColorIterator ci = geom->make_color_iterator();

	// save the modelview matrix
	LMatrix4f modelview_mat;

	const TransformAttribute *ctatt;
	if (!get_attribute_into(ctatt, _state, TransformTransition::get_class_type()))
		modelview_mat = LMatrix4f::ident_mat();
	else
		modelview_mat = ctatt->get_matrix();

	// get the camera information
	float aspect_ratio;
	aspect_ratio = _actual_display_region->get_camera()->get_aspect();

	// null the world xform, so sprites are orthog to scrn  (but not necessarily camera pntm unless they lie along z-axis)
	_d3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matIdentity);
	// only need to change _WORLD xform, _VIEW xform is Identity

	// precomputation stuff
	float half_width = 0.5f * (float) tex->_pbuffer->get_xsize();
	float half_height = 0.5f * (float) tex->_pbuffer->get_ysize();
	float scaled_width, scaled_height;

	// set up the texture-rendering state
	NodeAttributes state;

	// this sets up texturing.  Could just set the renderstates directly, but this is a little cleaner
	TextureAttribute *ta = new TextureAttribute;
	ta->set_on(tex);
	state.set_attribute(TextureTransition::get_class_type(), ta);

	TextureApplyAttribute *taa = new TextureApplyAttribute;
	taa->set_mode(TextureApplyProperty::M_modulate);
	state.set_attribute(TextureApplyTransition::get_class_type(), taa);

	set_state(state, false);

	// the user can override alpha sorting if they want
	bool alpha = false;

	if (geom->get_alpha_disable() == false) {
		// figure out if alpha's enabled (if not, no reason to sort)
		const TransparencyAttribute *ctratt;
		if (get_attribute_into(ctratt, _state, TransparencyTransition::get_class_type()))
			alpha = true;
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
	if (x_overall == true)
		scaled_width = geom->_x_texel_ratio[0] * half_width;
	else {
		nassertv(((int)geom->_x_texel_ratio.size() >= geom->get_num_prims()));
		x_walk = &geom->_x_texel_ratio[0];
	}

	// y direction
	if (y_overall == true)
		scaled_height = geom->_y_texel_ratio[0] * half_height * aspect_ratio;
	else {
		nassertv(((int)geom->_y_texel_ratio.size() >= geom->get_num_prims()));
		y_walk = &geom->_y_texel_ratio[0];
	}

	// theta
	if (theta_on) {
		if (theta_overall == true)
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
	vector< WrappedSpriteSortPtr > sorted_sprite_vector;
	vector< WrappedSpriteSortPtr >::iterator sorted_vec_iter;

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

		if (color_overall == false) {
			GET_NEXT_COLOR();
			pSpr->_c = p_colr;
		}
		if (x_overall == false)
			pSpr->_x_ratio = *x_walk++;
		if (y_overall == false)
			pSpr->_y_ratio = *y_walk++;	   // go along array of ratio values stored in geom
		if (theta_on && (theta_overall == false))
			pSpr->_theta = *theta_walk++;
	}

	if (alpha) {
		sorted_sprite_vector.reserve(nprims);	//pre-alloc space for nprims	  

		for (pSpr=SpriteArray,i = 0; i < nprims; i++,pSpr++) {	 // build STL-sortable array
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

	p_flags = D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) ;
	DWORD vertex_size = sizeof(float) * 2 + sizeof(D3DVALUE) * 3;

	D3DCOLOR CurColor;
	bool bDoColor=TRUE;
	if (color_overall == true) {
		GET_NEXT_COLOR();
		CurColor = p_colr;
		bDoColor = (p_colr != ~0); 
	}

	if (bDoColor) {
		p_flags |= D3DFVF_DIFFUSE;
		vertex_size+=sizeof(D3DCOLOR);
	}

	#ifdef _DEBUG
  	 nassertv(_pCurFvfBufPtr == NULL);	 // make sure the storage pointer is clean.
     nassertv(nprims * 4 * vertex_size < VERT_BUFFER_SIZE);
	 nassertv(nprims * 6 < D3DMAXNUMVERTICES );
	#endif

	_pCurFvfBufPtr = _pFvfBufBasePtr;		   // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't
	const float TexCrdSets[4][2] = {{0.0,0.0},{1.0,0.0},{0.0,1.0},{1.0,1.0}};

#define QUADVERTLISTLEN 6

	DWORD QuadVertIndexList[QUADVERTLISTLEN] = { 0, 1, 2, 3, 2, 1};
	DWORD CurDPIndexArrLength=0,CurVertCount=0;

	for (pSpr=SpriteArray,i = 0; i < nprims; i++,pSpr++) {	 // build STL-sortable array

		if (alpha) {
			pSpr = sorted_vec_iter->pSpr;
			sorted_vec_iter++;
		}

		// if not G_OVERALL, calculate the scale factors    //huh??
		if (x_overall == false)
			scaled_width = pSpr->_x_ratio * half_width;

		if (y_overall == false)
			scaled_height = pSpr->_y_ratio * half_height * aspect_ratio;

		// if not G_OVERALL, do some trig for this z rotate   //what is the theta angle??
		if (theta_on) {
			if (!theta_overall)
				theta = pSpr->_theta;

			// create the rotated points.  BUGBUG: this matmult will be slow if we dont get inlining
			// rotate_mat calls sin() on an unbounded val, possible to make it faster with lookup table (modulate to 0-360 range?)
			LMatrix3f xform_mat = LMatrix3f::rotate_mat(theta) *   
								  LMatrix3f::scale_mat(scaled_width, scaled_height);

			ur = (LVector3f( 1,  1, 0) * xform_mat) + pSpr->_v;
			ul = (LVector3f(-1,  1, 0) * xform_mat) + pSpr->_v;
			lr = (LVector3f( 1, -1, 0) * xform_mat) + pSpr->_v;
			ll = (LVector3f(-1, -1, 0) * xform_mat) + pSpr->_v;
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
			if (!color_overall)	 // otherwise its already been set globally
				CurColor = pSpr->_c;
			add_DWORD_to_FVFBuf(CurColor); // only need to cpy color on 1st vert, others are just empty ignored space
		}
		add_to_FVFBuf((void *)TexCrdSets[0], sizeof(float)*2);

		add_to_FVFBuf((void *)lr.get_data(), sizeof(D3DVECTOR));
		if (bDoColor)
			_pCurFvfBufPtr += sizeof(D3DCOLOR);	 // flat shading, dont need to write color, just incr ptr
		add_to_FVFBuf((void *)TexCrdSets[1], sizeof(float)*2);

		add_to_FVFBuf((void *)ul.get_data(), sizeof(D3DVECTOR));
		if (bDoColor)
			_pCurFvfBufPtr += sizeof(D3DCOLOR);	 // flat shading, dont need to write color, just incr ptr
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
	_d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, p_flags, _pFvfBufBasePtr, 4*nprims, _index_buf,QUADVERTLISTLEN*nprims,NULL);

	_pCurFvfBufPtr = NULL;
	delete [] SpriteArray;

	// restore the matrices
	_d3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &OldD3DWorldMatrix);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_polygon
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_polygon(const GeomPolygon *geom) {
	activate();

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_polygon()" << endl;
#endif

/*  wireframe polygon will be drawn as multi-tri trifan until I get this casting issue straightened out
   DWORD rstate;
   _d3dDevice->GetRenderState(D3DRENDERSTATE_FILLMODE, &rstate);
   if(rstate!=D3DFILL_WIREFRAME) {
	   draw_multitri(geom, D3DPT_TRIANGLEFAN);
   } else {
	   Geom *gp=dynamic_cast<Geom *>(geom);  doesnt work
	   draw_linestrip(gp);
   }
*/   

	draw_multitri(geom, D3DPT_TRIANGLEFAN);

#ifdef WBD_GL_MODE
	int nprims = geom->get_num_prims();
	const int *plen = geom->get_lengths();
	Geom::VertexIterator vi = geom->make_vertex_iterator();
	Geom::NormalIterator ni = geom->make_normal_iterator();
	Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
	Geom::ColorIterator ci = geom->make_color_iterator();


	GeomIssuer issuer(geom, this,
					  issue_vertex_gl,
					  issue_normal_gl,
					  issue_texcoord_gl,
					  issue_color_gl);

	// If we have per-vertex colors or normals, we need smooth shading.
	// Otherwise we want flat shading for performance reasons.
	if (geom->get_binding(G_COLOR) == G_PER_VERTEX || 
		geom->get_binding(G_NORMAL) == G_PER_VERTEX) {
		call_glShadeModel(GL_SMOOTH);
	} else {
		call_glShadeModel(GL_FLAT);
	}

	// Draw overall
	issuer.issue_color(G_OVERALL, ci);
	issuer.issue_normal(G_OVERALL, ni); 

	for (int i = 0; i < nprims; i++) {
		// Draw per primitive
		issuer.issue_color(G_PER_PRIM, ci);
		issuer.issue_normal(G_PER_PRIM, ni);

		int num_verts = *(plen++);
		nassertv(num_verts >= 3);

		glBegin(GL_POLYGON);

		// Draw the vertices.
		int v;
		for (v = 0; v < num_verts; v++) {
			// Per-vertex attributes.
			issuer.issue_color(G_PER_VERTEX, ci);
			issuer.issue_normal(G_PER_VERTEX, ni);
			issuer.issue_texcoord(G_PER_VERTEX, ti);
			issuer.issue_vertex(G_PER_VERTEX, vi);
		}
		glEnd();
	}
#endif              // WBD_GL_MODE
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_tri
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_tri(const GeomTri *geom) {
	// activate();

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_tri()" << endl;
#endif

#ifdef _DEBUG
	if (_pCurTexContext!=NULL) {
//    dxgsg_cat.spam() << "Cur active DX texture: " << _pCurTexContext->_tex->get_name() << "\n";
	}
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
#endif

	// for Indexed Prims and mixed indexed/non-indexed prims, we will use old pipeline for now
	// need to add code to handle fully indexed mode (and handle cases with index arrays of different lengths,
	// values (may only be possible to handle certain cases without reverting to old pipeline)
	if (GeomVrtFmt!=FlatVerts) {
		// this is the old geom setup, it reformats every vtx into an output array passed to d3d

		perVertex = PER_COORD;

		if (NormalBinding == G_PER_VERTEX)	 perVertex |= PER_NORMAL;
		if (ColorBinding == G_PER_VERTEX)	 perVertex |= PER_COLOR;
		if (TexCoordBinding == G_PER_VERTEX) perVertex |= PER_TEXCOORD;

		perPrim = 0;
		if (NormalBinding == G_PER_PRIM) perPrim |= PER_NORMAL;
		if (ColorBinding == G_PER_PRIM)	 perPrim |= PER_COLOR;

		size_t vertex_size = draw_prim_setup(geom);

		nassertv(_pCurFvfBufPtr == NULL);	 // make sure the storage pointer is clean.
		nassertv(nPrims * 3 * vertex_size < VERT_BUFFER_SIZE);
		_pCurFvfBufPtr = _pFvfBufBasePtr;		   // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

		// iterate through the triangle primitive

		for (int i = 0; i < nPrims; i++) {
			if (perPrim & PER_COLOR) {
				GET_NEXT_COLOR();
			}

			if (perPrim & PER_NORMAL)
				p_normal = geom->get_next_normal(ni);	// set primitive normal if there is one.

			draw_prim_inner_loop(3, geom, perVertex);
		}

		hr = _d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, p_flags, _pFvfBufBasePtr, nPrims*3, NULL);
	    TestDrawPrimFailure(DrawPrim,hr,_pDD);

		_pCurFvfBufPtr = NULL;
	} else {

		// new geom setup that uses strided DP calls to avoid making an extra pass over the data

		D3DDRAWPRIMITIVESTRIDEDDATA dps_data;
		memset(&dps_data,0,sizeof(D3DDRAWPRIMITIVESTRIDEDDATA));

#ifdef _DEBUG
		nassertv(!geom->uses_components());	 // code ignores lengths array
		nassertv(geom->get_binding(G_COORD) == G_PER_VERTEX);
#endif

		D3DPRIMITIVETYPE primtype=D3DPT_TRIANGLELIST;

		DWORD fvf_flags = D3DFVF_XYZ;
		dps_data.position.lpvData = (VOID*)coords;
		dps_data.position.dwStride = sizeof(D3DVECTOR);

		D3DSHADEMODE NeededShadeMode = D3DSHADE_FLAT;

		const DWORD dwVertsPerPrim=3;
	
		if ((NormalBinding != G_OFF) && wants_normals()) {

			dps_data.normal.lpvData = (VOID*)norms;
			dps_data.normal.dwStride = sizeof(D3DVECTOR);

#ifdef _DEBUG
			nassertv(geom->get_num_vertices_per_prim()==3);
			nassertv( nPrims*dwVertsPerPrim*sizeof(D3DVECTOR) <= D3DMAXNUMVERTICES*sizeof(WORD)); 
			if (NormalBinding==G_PER_VERTEX)
				nassertv(norms.size()>=nPrims*dwVertsPerPrim);
#endif

			fvf_flags |= D3DFVF_NORMAL;

			NeededShadeMode = D3DSHADE_GOURAUD;

			Normalf *pExpandedNormalArray = (Normalf *)_index_buf;	// BUGBUG:  need to use real permanent buffers for this conversion
			if (NormalBinding==G_PER_PRIM) {
				// must use tmp array to duplicate-expand per-prim norms to per-vert norms
				Normalf *pOutVec = pExpandedNormalArray;
				Normalf *pInVec=norms;

				nassertv(norms.size()>=nPrims);

				for (int i=0;i<nPrims;i++,pInVec++,pOutVec+=dwVertsPerPrim) {
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

		ColorAttribute *catt=NULL;
		bool bDoGlobalSceneGraphColor=FALSE,bDoColor=(ColorBinding != G_OFF);

		// We should issue geometry colors only if the scene graph color is off.
		if (get_attribute_into(catt, _state, ColorTransition::get_class_type()) && !catt->is_off()) {
			if (!catt->is_real())
				bDoColor=FALSE;	 // this turns off any Geom colors
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
			nassertv( nPrims*dwVertsPerPrim*sizeof(D3DCOLOR) <= VERT_BUFFER_SIZE);
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
						for (int i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsPerPrim) {
							D3DCOLOR newcolr = Colorf_to_D3DCOLOR(*pInColor);
							*pOutColor     = newcolr;
							*(pOutColor+1) = newcolr;
							*(pOutColor+2) = newcolr;
						}
					 } else {
						for (int i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsPerPrim) {
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
						for (int i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsPerPrim) {
							*pOutColor = Colorf_to_D3DCOLOR(*pInColor);
						}
					 } else {
						for (int i=0;i<nPrims;i++,pInColor++,pOutColor+=dwVertsPerPrim) {
							transform_color(*pInColor,*pOutColor);
						}
					} 
				}
			} else if (ColorBinding==G_PER_VERTEX) {
				NeededShadeMode = D3DSHADE_GOURAUD;

				// want to do this conversion once in retained mode
				DWORD cNumColors=nPrims*dwVertsPerPrim;

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
			nassertv(TexCoordBinding == G_PER_VERTEX);	// only sensible choice for a tri
#endif

			dps_data.textureCoords[0].lpvData = (VOID*)texcoords;
			dps_data.textureCoords[0].dwStride = sizeof(TexCoordf);
			fvf_flags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
		}

		set_shademode(NeededShadeMode);

		hr = _d3dDevice->DrawPrimitiveStrided(primtype, fvf_flags, &dps_data, nPrims*dwVertsPerPrim, NULL);
	    TestDrawPrimFailure(DrawPrimStrided,hr,_pDD);

		_pCurFvfBufPtr = NULL;
	}

///////////////////////////
#if 0
	// test triangle for me to dbg experiments only
	float vert_buf[15] = {
		0.0, 0.0, 0.0,  0.0, 0.0, 
		33.0, 0.0, 0.0,  0.0, 2.0, 
		0.0, 0.0, 33.0,  2.0, 0.0
	};

	_d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,D3DTADDRESS_BORDER);
	_d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,D3DTADDRESS_BORDER);
	_d3dDevice->SetTextureStageState(0,D3DTSS_BORDERCOLOR,D3DRGBA(0,0,0,0));

	p_flags =  D3DFVF_XYZ | (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0)) ;
	HRESULT hr = _d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,  p_flags, vert_buf, nPrims*3, NULL);
    TestDrawPrimFailure(DrawPrim,hr,_pDD);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_quad
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_quad(const GeomQuad *geom) {
	activate();

#if 1
	static BOOL bPrintedMsg=FALSE;

	if (!bPrintedMsg) {
		bPrintedMsg=TRUE;
		dxgsg_cat.error() << "dxgsg draw_quad drawing not implemented yet!\n";
	}
#endif

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_quad()" << endl;
#endif

#ifdef WBD_GL_MODE
	int nprims = geom->get_num_prims();
	Geom::VertexIterator vi = geom->make_vertex_iterator();
	Geom::NormalIterator ni = geom->make_normal_iterator();
	Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
	Geom::ColorIterator ci = geom->make_color_iterator();

	GeomIssuer issuer(geom, this,
					  issue_vertex_gl,
					  issue_normal_gl,
					  issue_texcoord_gl,
					  issue_color_gl);

	// If we have per-vertex colors or normals, we need smooth shading.
	// Otherwise we want flat shading for performance reasons.
	if (geom->get_binding(G_COLOR) == G_PER_VERTEX || 
		geom->get_binding(G_NORMAL) == G_PER_VERTEX) {
		call_glShadeModel(GL_SMOOTH);
	} else {
		call_glShadeModel(GL_FLAT);
	}

	// Draw overall
	issuer.issue_color(G_OVERALL, ci);
	issuer.issue_normal(G_OVERALL, ni); 

	glBegin(GL_QUADS);

	for (int i = 0; i < nprims; i++) {
		// Draw per primitive
		issuer.issue_color(G_PER_PRIM, ci);
		issuer.issue_normal(G_PER_PRIM, ni);

		for (int j = 0; j < 4; j++) {
			// Draw per vertex
			issuer.issue_color(G_PER_VERTEX, ci);
			issuer.issue_normal(G_PER_VERTEX, ni);
			issuer.issue_texcoord(G_PER_VERTEX, ti);
			issuer.issue_vertex(G_PER_VERTEX, vi);
		}
	}

	glEnd();
#endif              // WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_tristrip
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_tristrip(const GeomTristrip *geom) {

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_tristrip()" << endl;
#endif

	draw_multitri(geom, D3DPT_TRIANGLESTRIP);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_trifan
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_trifan(const GeomTrifan *geom) {

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_trifan()" << endl;
#endif

	draw_multitri(geom, D3DPT_TRIANGLEFAN);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_multitri
//       Access: Public, Virtual
//  Description: handles trifans and tristrips
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_multitri(const Geom *geom, D3DPRIMITIVETYPE trilisttype) {

	int nPrims = geom->get_num_prims();
	const int *pLengthArr = geom->get_lengths();
	HRESULT hr;

	if (nPrims==0) {
		#ifdef _DEBUG
 		  dxgsg_cat.warning() << "draw_multitri() called with ZERO vertices!!" << endl;
		#endif
		return;
	}

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
#endif

	// for Indexed Prims and mixed indexed/non-indexed prims, we will use old pipeline
	// cant handle indexed prims because usually have different index arrays for different components,
	// and DrIdxPrmStrd only accepts 1 index array for all components
	if (GeomVrtFmt!=FlatVerts) {

		// this is the old geom setup, it reformats every vtx into an output array passed to d3d
		perVertex = PER_COORD;
		perPrim = perComp = 0;

		switch (geom->get_binding(G_NORMAL)) {
			case G_PER_VERTEX:
				perVertex |= PER_NORMAL;
				break;
			case G_PER_PRIM:
				perPrim |= PER_NORMAL;
				break;
			case G_PER_COMPONENT:
				perComp |= PER_NORMAL;
				break;
		}

		switch (geom->get_binding(G_COLOR)) {
			case G_PER_VERTEX:
				perVertex |= PER_COLOR;
				break;
			case G_PER_PRIM:
				perPrim |= PER_COLOR;
				break;
			case G_PER_COMPONENT:
				perComp |= PER_COLOR;
				break;
		}

		if (geom->get_binding(G_TEXCOORD) == G_PER_VERTEX)
			perVertex |= PER_TEXCOORD;

		size_t vertex_size = draw_prim_setup(geom);

		// iterate through the triangle primitives

		for (int i = 0; i < nPrims; i++) {

			if (perPrim & PER_COLOR) {
				GET_NEXT_COLOR();
			}
			if (perPrim & PER_NORMAL)
				p_normal = geom->get_next_normal(ni);	// set primitive normal if there is one.

			int nVerts = *(pLengthArr++);
#ifdef _DEBUG
			nassertv(nVerts >= 3);
			nassertv(_pCurFvfBufPtr == NULL);	 // make sure the storage pointer is clean.
			nassertv(nVerts * vertex_size < VERT_BUFFER_SIZE);
#endif
			_pCurFvfBufPtr = _pFvfBufBasePtr;			 // _pCurFvfBufPtr changes,  _pFvfBufBasePtr doesn't

			// handles first triangle strip/fan
			if (perComp & PER_COLOR) {
				GET_NEXT_COLOR();
			} 
			if (perComp & PER_NORMAL)
				p_normal = geom->get_next_normal(ni);	// set primitive normal if there is one.

			// Store first triangle
			draw_prim_inner_loop(3, geom, perVertex);

			// Store remaining vertices
			draw_prim_inner_loop(nVerts-3, geom, perVertex | perComp);

			HRESULT hr = _d3dDevice->DrawPrimitive(trilisttype,  p_flags, _pFvfBufBasePtr, nVerts, NULL);
			TestDrawPrimFailure(DrawPrim,hr,_pDD);

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

			Normalf *pExpandedNormalArray = (Normalf *)_index_buf;	// BUGBUG:  need to use real permanent buffers instead of _indexbuf hack

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

		ColorAttribute *catt=NULL;
		bool bDoGlobalSceneGraphColor=FALSE,bDoColor=(ColorBinding != G_OFF);

		// We should issue geometry colors only if the scene graph color is off.
		if (get_attribute_into(catt, _state, ColorTransition::get_class_type()) && !catt->is_off()) {
			if (!catt->is_real())
				bDoColor=FALSE;	 // this turns off any Geom colors
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
						} else {  /* trifan */                								 \
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
			nassertv(TexCoordBinding == G_PER_VERTEX);	// only sensible choice for a tri
#endif

			dps_data.textureCoords[0].lpvData = (VOID*)texcoords;
			dps_data.textureCoords[0].dwStride = sizeof(TexCoordf);
			fvf_flags |= (D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
		}

		set_shademode(NeededShadeMode);

		for (int j=0;j<nPrims;j++) {
			const int cCurNumStripVerts = pLengthArr[j];

			hr = _d3dDevice->DrawPrimitiveStrided(trilisttype, fvf_flags, &dps_data, cCurNumStripVerts, NULL);
			TestDrawPrimFailure(DrawPrimStrided,hr,_pDD);

			dps_data.position.lpvData = (VOID*)(((char*) dps_data.position.lpvData) + cCurNumStripVerts*dps_data.position.dwStride); 
			dps_data.diffuse.lpvData = (VOID*)(((char*) dps_data.diffuse.lpvData) + cCurNumStripVerts*dps_data.diffuse.dwStride);
			dps_data.normal.lpvData = (VOID*)(((char*) dps_data.normal.lpvData) + cCurNumStripVerts*dps_data.normal.dwStride); 
			dps_data.textureCoords[0].lpvData = (VOID*)(((char*) dps_data.textureCoords[0].lpvData) + cCurNumStripVerts*dps_data.textureCoords[0].dwStride); 
		}

		_pCurFvfBufPtr = NULL;
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
	wNumSections++;	 // to make us equiv to gluSphere 

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

	nassertv(pVertexSpace==_pCurFvfBufPtr);	 // add_to_FVFBuf requires this

#define ADD_GENSPHERE_VERTEX_TO_BUFFER(VERT)                      \
    add_to_FVFBuf((void *)&(VERT), sizeof(D3DVECTOR));            \
    if(fvfFlags & D3DFVF_NORMAL)                                  \
		add_to_FVFBuf((void *)&vNormal, sizeof(D3DVECTOR));       \
    if(fvfFlags & D3DFVF_DIFFUSE)                                 \
		add_DWORD_to_FVFBuf(p_colr);                              \
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
		dtheta = (float)(M_PI / (wNumRings-1));		//Angle between each ring (ignore 2 fake rings)  
		theta = 0.0;
	} else {
		dtheta = (float)(M_PI / (wNumRings + 1));	//Angle between each ring		 
		theta = dtheta;
	}
	float phi,dphi   = (float)(2*M_PI / (wNumSections-1)); //Angle between each section

	for (i = 0; i < wNumRings; i++) {
		float costheta,sintheta,cosphi,sinphi;
		phi =   0.0;

		if (DOTEXTURING) {
			texCoords[1] = theta * reciprocal_PI;  // v is the same for each ring
		}

		// could optimize all this sin/cos stuff w/tables
		csincos(theta,&sintheta,&costheta);
		y = fRadius * costheta;		// y is the same for each ring

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
				add_DWORD_to_FVFBuf(p_colr);

			if (DOTEXTURING) {
				texCoords[0] = 1.0 - phi*reciprocal_2PI;
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
	DWORD cur_vertring_startidx=0;	  // first vertex in current ring
	DWORD CurFinalTriIndex = 0;		  // index of next tri to be written

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

		cur_vertring_startidx = 1;			// first vertex in current ring (skip top vert)
		CurFinalTriIndex = wNumSections;	// index of tri to be written, wNumSections to skip the top cap row
	}

	DWORD j_incd,base_index;

	// technically we could break into a strip for every row (or 1 big strip connected w/degenerate tris)
	// but indexed trilists should actually be just as fast on HW

	// Generate triangles for the rings
	for (i = 0; i < wNumRings-1; i++) {
		for (DWORD j = 0; j < wNumSections; j++) {

			base_index=3*CurFinalTriIndex;	// final vert index is 3*finaltriindex
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

			CurFinalTriIndex += 2;	// we wrote 2 tris, add 2 to finaltriindex
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
draw_sphere(const GeomSphere *geom) {

#define SPHERE_NUMSLICES 16
#define SPHERE_NUMSTACKS 10

#ifdef GSG_VERBOSE
	dxgsg_cat.debug() << "draw_sphere()" << endl;
#endif

	int nprims = geom->get_num_prims();

	if (nprims==0) {
		dxgsg_cat.warning() << "draw_sphere() called with ZERO vertices!!" << endl;
		return;
	}

	Geom::VertexIterator vi = geom->make_vertex_iterator();
	Geom::ColorIterator ci;
	bool bPerPrimColor = (geom->get_binding(G_COLOR) == G_PER_PRIM);
	if (bPerPrimColor)
		ci = geom->make_color_iterator();

	for (int i = 0; i < nprims; i++) {

		DWORD nVerts,nIndices;
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
					   _index_buf, D3DMAXNUMVERTICES,
					   (D3DVECTOR *)&center, fRadius, 
					   SPHERE_NUMSTACKS, SPHERE_NUMSLICES,
					   1.0f, 1.0f, 1.0f,  // no scaling factors, do a sphere not ellipsoid
					   &nVerts,&nIndices,p_flags,vertex_size);

		// possible optimization: make DP 1 for all spheres call here, since trilist is independent tris.
		// indexes couldnt start w/0 tho, need to pass offset to gensph
		_d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,  p_flags, _pFvfBufBasePtr, nVerts, _index_buf,nIndices,NULL);
	}

	_pCurFvfBufPtr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::issue_color_transform
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_color_transform(const ColorMatrixAttribute *attrib) {
	_current_color_mat = attrib->get_matrix();

	if (_current_color_mat == LMatrix4f::ident_mat()) {  // couldnt we a single ptr instead of doing full comparison
		_color_transform_enabled = false;
	} else {
		_color_transform_enabled = true;
	}
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::issue_alpha_transform
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_alpha_transform(const AlphaTransformAttribute *attrib) {
	_current_alpha_offset= attrib->get_offset();
	_current_alpha_scale = attrib->get_scale();

	if ((_current_alpha_offset == 0.0) && (_current_alpha_scale == 1.0)) {
		_alpha_transform_enabled = false;
	} else {
		_alpha_transform_enabled = true;
	}
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
	activate();

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
		return;	 // use enable_texturing to disable/enable 
	}

//  activate();  inactive
//	bind_texture(tc);

//  specify_texture(tc->_texture);
	// Note: if this code changes, make sure to change initialization SetTSS code in init_dx as well
	// so DX TSS renderstate matches dxgsg state

	DXTextureContext *dtc = DCAST(DXTextureContext, tc);  

	if(	_pCurTexContext == dtc) {
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

		#ifdef _DEBUG
			if((!(dtc->_bHasMipMaps))&&(mipfilter!=D3DTFP_NONE)) {
				dxgsg_cat.error() << "Trying to set mipmap filtering for texture with no generated mipmaps!! texname:" << tex->get_name() << "  filter: "<<(DWORD)ft<<"\n";
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

	_pCurTexContext = dtc;	 // enable_texturing needs this
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
	activate();

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

	activate();
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
	activate();

#ifdef WBD_GL_MODE
	Texture *tex = tc->_texture;

	DisplayRegionStack old_dr = push_display_region(dr);
	prepare_display_region();

	NodeAttributes state;
	CullFaceAttribute *cfa = new CullFaceAttribute;
	cfa->set_mode(CullFaceProperty::M_cull_none);
	DepthTestAttribute *dta = new DepthTestAttribute;
	dta->set_mode(DepthTestProperty::M_none);
	DepthWriteAttribute *dwa = new DepthWriteAttribute;
	dwa->set_off();
	TextureAttribute *ta = new TextureAttribute;
	ta->set_on(tex);
	TextureApplyAttribute *taa = new TextureApplyAttribute;
	taa->set_mode(TextureApplyProperty::M_decal);

	state.set_attribute(LightTransition::get_class_type(), 
						new LightAttribute);
	state.set_attribute(ColorMaskTransition::get_class_type(),
						new ColorMaskAttribute);
	state.set_attribute(RenderModeTransition::get_class_type(),
						new RenderModeAttribute);
	state.set_attribute(TexMatrixTransition::get_class_type(),
						new TexMatrixAttribute);
	state.set_attribute(TransformTransition::get_class_type(),
						new TransformAttribute);
	state.set_attribute(ColorBlendTransition::get_class_type(),
						new ColorBlendAttribute);
	state.set_attribute(CullFaceTransition::get_class_type(), cfa);
	state.set_attribute(DepthTestTransition::get_class_type(), dta);
	state.set_attribute(DepthWriteTransition::get_class_type(), dwa);
	state.set_attribute(TextureTransition::get_class_type(), ta);
	state.set_attribute(TextureApplyTransition::get_class_type(), taa);
	set_state(state, false);

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

	activate();
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
	activate();

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
	activate();

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
				  const NodeAttributes& na) {

	dxgsg_cat.fatal() << "DXGSG draw_pixel_buffer unimplemented!!!";
	return;

#ifdef WBD_GL_MODE
	nassertv(pb != NULL && dr != NULL);
	nassertv(!pb->_image.empty());
	activate();

	DisplayRegionStack old_dr = push_display_region(dr);
	prepare_display_region();

	NodeAttributes state(na);
	state.set_attribute(LightTransition::get_class_type(), 
						new LightAttribute);
	state.set_attribute(TextureTransition::get_class_type(), 
						new TextureAttribute);
	state.set_attribute(TransformTransition::get_class_type(), 
						new TransformAttribute);
	state.set_attribute(ColorBlendTransition::get_class_type(), 
						new ColorBlendAttribute);
	state.set_attribute(StencilTransition::get_class_type(), 
						new StencilAttribute);


	switch (pb->get_format()) {
		case PixelBuffer::F_depth_component: 
			{
				ColorMaskAttribute *cma = new ColorMaskAttribute;
				cma->set_mask(0);
				DepthTestAttribute *dta = new DepthTestAttribute;
				dta->set_mode(DepthTestProperty::M_always);
				DepthWriteAttribute *dwa = new DepthWriteAttribute;
				dwa->set_off();
				state.set_attribute(ColorMaskTransition::get_class_type(), cma);
				state.set_attribute(DepthTestTransition::get_class_type(), dta);
				state.set_attribute(DepthWriteTransition::get_class_type(), dwa);
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
				ColorMaskAttribute *cma = new ColorMaskAttribute;
				DepthTestAttribute *dta = new DepthTestAttribute;
				dta->set_mode(DepthTestProperty::M_none);
				DepthWriteAttribute *dwa = new DepthWriteAttribute;
				dwa->set_off();
				state.set_attribute(ColorMaskTransition::get_class_type(), cma);
				state.set_attribute(DepthTestTransition::get_class_type(), dta);
				state.set_attribute(DepthWriteTransition::get_class_type(), dwa);
			}
			break;
		default:
			dxgsg_cat.error()
			<< "draw_pixel_buffer(): unknown buffer format" << endl;
			break;
	}

	set_state(state, false);

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
				  const RenderBuffer &rb, const NodeAttributes& na) {
	activate();
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

	Fog::Mode panda_fogmode = fog->get_mode();
	D3DFOGMODE d3dfogmode = get_fog_mode_type(panda_fogmode);

	if(_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE ) {
  	   _d3dDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, d3dfogmode);
	} else {

	  // vtx fog looks crappy if you have large polygons in the foreground
	  if(dx_no_vertex_fog)
		  return;

		//if(_D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX )
		// every card is going to have vertex fog, since it's implemented in d3d runtime

      _d3dDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, d3dfogmode);
	}

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

	Colorf  fog_colr = fog->get_color();
	_d3dDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, 
				   D3DRGBA(fog_colr[0], fog_colr[1], fog_colr[2], 0.0));
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

	alight.dvAttenuation0 = 1.0f;		// constant
	alight.dvAttenuation1 = 0.0f;		// linear
	alight.dvAttenuation2 = 0.0f;		// quadratic

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
	alight.dvAttenuation1 = (D3DVALUE)light->get_linear_attenuation();	 // linear
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
issue_transform(const TransformAttribute *attrib) {
	activate();
#ifndef NDEBUG
	if (dx_show_transforms) {

		bool lighting_was_enabled = _lighting_enabled;
		bool texturing_was_enabled = _texturing_enabled;
		enable_lighting(false);
		enable_texturing(false);

		typedef struct {
			D3DVALUE x,y,z;	  // position
			D3DVALUE nx,ny,nz; // normal
			D3DCOLOR  diff;	  // diffuse color
		}        VERTFORMAT;

		VERTFORMAT vert_buf[] = {
			{0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(1.0, 0.0, 0.0, 1.0)},	   // red
			{3.0, 0.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(1.0, 0.0, 0.0, 1.0)},	   // red
			{0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(0.0, 1.0, 0.0, 1.0)},	   // grn
			{0.0, 3.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(0.0, 1.0, 0.0, 1.0)},	   // grn
			{0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(0.0, 0.0, 1.0, 1.0)},	   // blu
			{0.0, 0.0, 3.0,  0.0, -1.0, 0.0,  D3DRGBA(0.0, 0.0, 1.0, 1.0)},	   // blu
		};

		HRESULT hr = _d3dDevice->DrawPrimitive(D3DPT_LINELIST, D3DFVF_DIFFUSE | D3DFVF_XYZ | D3DFVF_NORMAL,
								  vert_buf, 6, NULL);
		TestDrawPrimFailure(DrawPrim,hr,_pDD);

		enable_lighting(lighting_was_enabled);
		enable_texturing(texturing_was_enabled);
	}
#endif

	_d3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD/*VIEW*/,
							 (LPD3DMATRIX) attrib->get_matrix().get_data());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_matrix
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_tex_matrix(const TexMatrixAttribute *attrib) {
	dxgsg_cat.fatal() << "DXGSG issue_tex_matrix unimplemented!!!";
	return;

	activate();
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

#if 1

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_color
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_color(const ColorAttribute *attrib) {
	activate();
	if (attrib->is_on()&& attrib->is_real()) {
		_issued_color_enabled = true;
		Colorf c = attrib->get_color();
		_issued_color = Colorf_to_D3DCOLOR(c);
	} else _issued_color_enabled = false;
}

#endif

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_texture
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_texture(const TextureAttribute *attrib) {

	activate();

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
issue_tex_gen(const TexGenAttribute *attrib) {
	dxgsg_cat.fatal() << "DXGSG issue_tex_gen unimplemented!!!";
	return;
	activate();
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
issue_material(const MaterialAttribute *attrib) {
	activate();
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
issue_fog(const FogAttribute *attrib) {
	activate();

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
issue_render_mode(const RenderModeAttribute *attrib) {
	activate();

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
void DXGraphicsStateGuardian::issue_light(const LightAttribute *attrib ) {
	nassertv(attrib->get_properties_is_on());
	activate();

	// Initialize the current ambient light total and currently enabled
	// light list
	_cur_ambient_light.set(0, 0, 0, 1);
	int i;
	for (i = 0; i < _max_lights; i++)
		_cur_light_enabled[i] = false;

	int num_enabled = 0;
	LightAttribute::const_iterator li;
	for (li = attrib->begin(); li != attrib->end(); ++li) {
		_cur_light_id = -1;
		num_enabled++;
		enable_lighting(true);
		Light *light = (*li);
		nassertv(light != (Light *)NULL);

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
				if (enable_light(i, true))	_cur_light_enabled[i] = true;
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
			if (enable_light(_cur_light_id, true))	_cur_light_enabled[_cur_light_id] = true;

			// We need to do something different for each type of light
			light->apply(this);
		} else if (_cur_light_id == -1) {
			dxgsg_cat.error()
			<< "issue_light() - failed to bind light to id" << endl;
		}
	}

	// Disable all unused lights
	for (i = 0; i < _max_lights; i++) {
		if (_cur_light_enabled[i] == false)
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
issue_color_blend(const ColorBlendAttribute *attrib) {
	activate();
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
//	if (_texturing_enabled == val) {  // this check is mostly for internal gsg calls, panda already screens out redundant state changes
//        return;        
//	}

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
//  Description: handles texture attribute (i.e. filter modes, etc) changes
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_texture_apply(const TextureApplyAttribute *attrib) {

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
issue_color_mask(const ColorMaskAttribute *attrib) {
	dxgsg_cat.fatal() << "DXGSG issue_color_mask unimplemented (not implementable on DX7)!!!";
	return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_depth_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_depth_test(const DepthTestAttribute *attrib) {
	activate();

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
issue_depth_write(const DepthWriteAttribute *attrib) {
	_d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, attrib->is_on());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_stencil
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_stencil(const StencilAttribute *attrib) {
  
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
//     Function: DXGraphicsStateGuardian::issue_cull_attribute
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_cull_face(const CullFaceAttribute *attrib) {
	activate();

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
issue_clip_plane(const ClipPlaneAttribute *attrib) {
	activate();

	// Initialize the currently enabled clip plane list
	int i;
	for (i = 0; i < _max_clip_planes; i++)
		_cur_clip_plane_enabled[i] = false;

	int num_enabled = 0;
	ClipPlaneAttribute::const_iterator pi;
	for (pi = attrib->begin(); pi != attrib->end(); ++pi) {
		PlaneNode *plane_node;
		DCAST_INTO_V(plane_node, (*pi));
		nassertv(plane_node != (PlaneNode *)NULL);

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

	// Disable all unused clip planes 
	for (i = 0; i < _max_clip_planes; i++) {
		if (_cur_clip_plane_enabled[i] == false)
			enable_clip_plane(i, false);
	}
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_transparency
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_transparency(const TransparencyAttribute *attrib ) {
	activate();

	TransparencyProperty::Mode mode = attrib->get_mode();

	switch (mode) {
		case TransparencyProperty::M_none:
//			enable_multisample_alpha_one(false);
//			enable_multisample_alpha_mask(false);
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
//			enable_multisample_alpha_one(false);
//			enable_multisample_alpha_mask(false);
			enable_blend(true);
			enable_alpha_test(false);
			call_dxBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
			break;
		case TransparencyProperty::M_multisample:
//			enable_multisample_alpha_one(true);
//			enable_multisample_alpha_mask(true);
			enable_blend(false);
			enable_alpha_test(false);
			break;
		case TransparencyProperty::M_multisample_mask:
//			enable_multisample_alpha_one(false);
//			enable_multisample_alpha_mask(true);
			enable_blend(false);
			enable_alpha_test(false);
			break;
		case TransparencyProperty::M_binary:
//			enable_multisample_alpha_one(false);
//			enable_multisample_alpha_mask(false);
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
issue_linesmooth(const LinesmoothAttribute *attrib) {
	enable_line_smooth(attrib->is_on());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::wants_normals
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool DXGraphicsStateGuardian::
wants_normals() const {
	return(_lighting_enabled || _normals_enabled);
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
//     Function: DXGraphicsStateGuardian::wants_colors
//       Access: Public, Virtual
//  Description: Returns true if the GSG should issue geometry color
//               commands, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DXGraphicsStateGuardian::
wants_colors() const {
	// If we have scene graph color enabled, return false to indicate we
	// shouldn't bother issuing geometry color commands.

	const ColorAttribute *catt;
	if (!get_attribute_into(catt, _state, ColorTransition::get_class_type())) {
		// No scene graph color at all.
		return true;
	}

	// We should issue geometry colors only if the scene graph color is off.
	if (catt->is_off() || (!catt->is_real()))
		return true;

	return false;
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
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
begin_decal(GeomNode *base_geom) {
	nassertv(base_geom != (GeomNode *)NULL);

	_decal_level++;
	nassertv(4*_decal_level < 16);

#ifndef DISABLE_POLYGON_OFFSET_DECALING
	if (dx_decal_type == GDT_offset) {

#define POLYGON_OFFSET_MULTIPLIER 2

		// Just draw the base geometry normally.
		base_geom->draw(this);
		_d3dDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, POLYGON_OFFSET_MULTIPLIER * _decal_level);	// _decal_level better not be higher than 8!
	} else
#endif  
	{
		if (_decal_level > 1)
			base_geom->draw(this);	// If we're already decaling, just draw the geometry.
		else {
			// First turn off writing the depth buffer to render the base geometry.
			_d3dDevice->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, (unsigned long *)&_depth_write_enabled);  //save cur val
			_d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

			// Now render the base geometry.
			base_geom->draw(this);

			// Render all of the decal geometry, too.  We'll keep the depth
			// buffer write off during this.
		}
	}
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
	nassertv(base_geom != (GeomNode *)NULL);

	_decal_level--;
//  nassertv(_decal_level >= 1);

#ifndef DISABLE_POLYGON_OFFSET_DECALING
	if (dx_decal_type == GDT_offset) {
		// Restore the Zbias offset.
		_d3dDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, POLYGON_OFFSET_MULTIPLIER * _decal_level);	// _decal_level better not be higher than 8!
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
			else {	// dx7 doesn't support planemask rstate
				// note: not saving current planemask val, assumes this is always all 1's.  should be ok
				_d3dDevice->SetRenderState(D3DRENDERSTATE_PLANEMASK,0x0);  // note PLANEMASK is supposedly obsolete for DX7
			}
#endif
// Note: For DX8, use D3DRS_COLORWRITEENABLE  (check D3DPMISCCAPS_COLORWRITEENABLE first)


			// No need to have texturing on for this.
			enable_texturing(false);

			base_geom->draw(this);

			// Finally, restore the depth write and color mask states to the
			// way they're supposed to be.
/*
	  DepthWriteAttribute *depth_write;
	  if (get_attribute_into(depth_write, _state,
				 DepthWriteTransition::get_class_type())) 
			issue_depth_write(depth_write);

	 ColorMaskAttribute *color_mask;
	if (get_attribute_into(color_mask, _state,
				   ColorMaskTransition::get_class_type())) {
	  issue_color_mask(color_mask);
	} else {

	  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}            
*/            

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
	} else 	if(rb._buffer_type & RenderBuffer::T_back) {
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
dx_cleanup() {

	release_all_textures();

	// Do a safe check for releasing the D3DDEVICE. RefCount should be zero.
	if (_d3dDevice!=NULL) {
		if (0 < _d3dDevice->Release()) {
			dxgsg_cat.error() << "DXGraphicsStateGuardian::destructor - d3dDevice reference count > 0" << endl;
		}
		_d3dDevice = NULL;	// clear the pointer in the Gsg
	}

	// Release the DDraw and D3D objects used by the app
	RELEASE(_zbuf);
	RELEASE(_back);
	RELEASE(_pri);

	RELEASE(_d3d);

	// Do a safe check for releasing DDRAW. RefCount should be zero.
	if (_pDD!=NULL) {
		int val;
		if (0 < (val = _pDD->Release())) {
			dxgsg_cat.error()
			<< "DXGraphicsStateGuardian::destructor -  IDDraw Obj reference count = " << val << ", should be zero!\n";
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
		dxgsg_cat.fatal() << "DXGraphicsStateGuardian::resize() - CreateSurface failed for primary : result = " << ConvD3DErrorToString(hr) << endl;
		exit(1);
	}

	// Create a clipper object which handles all our clipping for cases when
	// our window is partially obscured by other windows. 
	LPDIRECTDRAWCLIPPER Clipper;

	if (FAILED(hr = _pDD->CreateClipper( 0, &Clipper, NULL ))) {
		dxgsg_cat.fatal()
		<< "dxgsg - CreateClipper after resize failed : result = " << ConvD3DErrorToString(hr) << endl;
		exit(1);
	}
	// Associate the clipper with our window. Note that, afterwards, the
	// clipper is internally referenced by the primary surface, so it is safe
	// to release our local reference to it.
	Clipper->SetHWnd( 0, mwindow );
	_pri->SetClipper( Clipper );
	Clipper->Release();

	// Recreate the backbuffer. (might want to handle failure due to running out of video memory)

	ddsd_back.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;	// just to make sure
	ddsd_back.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

	PRINTVIDMEM(_pDD,&ddsd_back.ddsCaps,"resize backbuffer surf");

	if (FAILED(hr = _pDD->CreateSurface( &ddsd_back, &_back, NULL ))) {
		dxgsg_cat.fatal() << "DXGraphicsStateGuardian::resize() - CreateSurface failed for backbuffer : result = " << ConvD3DErrorToString(hr) << endl;
		exit(1);
	}

	PRINTVIDMEM(_pDD,&ddsd_back.ddsCaps,"resize zbuffer surf");

	// Recreate and attach a z-buffer. 
	if (FAILED(hr = _pDD->CreateSurface( &ddsd_zbuf, &_zbuf, NULL ))) {
		dxgsg_cat.fatal() << "DXGraphicsStateGuardian::resize() - CreateSurface failed for Z buffer: result = " << ConvD3DErrorToString(hr) << endl;
		exit(1);
	}

	// Attach the z-buffer to the back buffer.
	if ((hr = _back->AddAttachedSurface( _zbuf ) ) != DD_OK) {
		dxgsg_cat.fatal()
		<< "DXGraphicsStateGuardian::resize() - AddAttachedSurface failed : result = " << ConvD3DErrorToString(hr) << endl;
		exit(1);
	}

	if ((hr = _d3dDevice->SetRenderTarget(_back,0x0) ) != DD_OK) {
		dxgsg_cat.fatal()
		<< "DXGraphicsStateGuardian::resize() - SetRenderTarget failed : result = " << ConvD3DErrorToString(hr) << endl;
		exit(1);
	}

	// Create the viewport
	D3DVIEWPORT7 vp = { 0, 0, renderWid, renderHt, 0.0f, 1.0f};
	hr = _d3dDevice->SetViewport( &vp );
	if (hr != DD_OK) {
		dxgsg_cat.fatal()
		<< "DXGraphicsStateGuardian:: SetViewport failed : result = " << ConvD3DErrorToString(hr) << endl;
		exit(1);
	}
}

bool refill_tex_callback(TextureContext *tc,void *void_dxgsg_ptr) {
	 DXTextureContext *dtc = DCAST(DXTextureContext, tc);  
//	 DXGraphicsStateGuardian *dxgsg = (DXGraphicsStateGuardian *)void_dxgsg_ptr;

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
		dxgsg_cat.fatal()
		<< "DXGraphicsStateGuardian:: RestoreAllSurfs failed : result = " << ConvD3DErrorToString(hr) << endl;
	return hr;
  }

  // cant access template in libpanda.dll directly due to vc++ limitations, use traverser to get around it
  traverse_prepared_textures(refill_tex_callback,this);
  return S_OK;
}


////////////////////////////////////////////////////////////////////
//     Function: show_frame 
//       Access:
//       Description:   Repaint primary buffer from back buffer  (windowed mode only)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::show_frame(void) {
	PStatTimer timer(_win->_swap_pcollector);  // this times just the flip, so it must go here in dxgsg, instead of wdxdisplay, which would time the whole frame

	if(_pri==NULL)
		return;

	if(dx_full_screen) {
		HRESULT hr = _pri->Flip( NULL, DDFLIP_WAIT );  // bugbug:  dont we want triple buffering instead of wasting time waiting for vsync?

		if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY) {
			//full screen app has been switched away
			HRESULT hr;

			// TestCooperativeLevel returns DD_OK: If the current mode is same as the one which the App set.
			// The following error is returned only for exclusivemode apps.
			// DDERR_NOEXCLUSIVEMODE: Some other app took exclusive mode.
			hr = _pDD->TestCooperativeLevel();

			while(hr == DDERR_NOEXCLUSIVEMODE) {
						// This means that mode changes had taken place, surfaces
						// were lost but still we are in the original mode, so we
						// simply restore all surfaces and keep going.
				_dx_ready = FALSE;

				#ifdef _DEBUG
				  dxgsg_cat.spam() << "DXGraphicsStateGuardian:: no exclusive mode, waiting...\n";
				#endif
						
				Sleep( 500 );	// Dont consume CPU.
				throw_event("PandaPaused");   // throw panda event to invoke network-only processing

				_win->process_events();
				hr = _pDD->TestCooperativeLevel();
			}

			if(FAILED(hr)) {
				dxgsg_cat.error() << "DXGraphicsStateGuardian::unexpected return code from TestCoopLevel: " << ConvD3DErrorToString(hr) << endl;   
				return;
			}

			#ifdef _DEBUG
			   dxgsg_cat.debug() << "DXGraphicsStateGuardian:: regained exclusive mode, refilling surfs...\n";
			#endif
			
			RestoreAllVideoSurfaces();

			#ifdef _DEBUG
			   dxgsg_cat.debug() << "DXGraphicsStateGuardian:: refill done...\n";			
			#endif
			
			_dx_ready = TRUE;
			
			return;	 // need to re-render scene before we can display it
		}

		if(hr != DD_OK) {
			dxgsg_cat.error() << "DXGraphicsStateGuardian::show_frame() - Flip failed w/unexpected error code: " << ConvD3DErrorToString(hr) << endl;
			exit(1);
		}

	} else {
		DX_DECLARE_CLEAN(DDBLTFX, bltfx);

		bltfx.dwDDFX |= DDBLTFX_NOTEARING;
		HRESULT hr = _pri->Blt( &_view_rect, _back,  NULL, DDBLT_DDFX | DDBLT_WAIT, &bltfx );

		if(FAILED(hr)) {
			if(hr == DDERR_SURFACELOST || hr == DDERR_SURFACEBUSY) {

				HRESULT hr;

			// TestCooperativeLevel returns DD_OK: If the current mode is same as the one which the App set.
			// The following two errors are returned to NORMALMODE (windowed)apps only.
			//
			// DDERR_WRONGMODE: If the App is a windowed app and the current mode is
			//                  not the same as the one in which the app was created.
			// DDERR_EXCLUSIVEMODEALREADYSET: If another app took exclusivemode access
				hr = _pDD->TestCooperativeLevel();
				while(hr == DDERR_EXCLUSIVEMODEALREADYSET) {
				// This means that mode changes had taken place, surfaces
				// were lost but still we are in the original mode, so we
				// simply restore all surfaces and keep going.

					_dx_ready = FALSE;
	
				#ifdef _DEBUG
					dxgsg_cat.spam() << "DXGraphicsStateGuardian:: another app has exclusive mode, waiting...\n";
				#endif
							
					Sleep( 500 );	// Dont consume CPU.
					throw_event("PandaPaused");   // throw panda event to invoke network-only processing

					hr = _pDD->TestCooperativeLevel();
				}

				if(hr==DDERR_WRONGMODE) {
				// This means that the desktop mode has changed
				// need to destroy all of dx stuff and recreate everything back again, which is a big hit
					dxgsg_cat.error() << "DXGraphicsStateGuardian:: detected display mode change in TestCoopLevel, must recreate all DDraw surfaces, D3D devices, this is not handled yet.  " << ConvD3DErrorToString(hr) << endl;   
					exit(1);
					return;
				}

				if(FAILED(hr)) {
					dxgsg_cat.error() << "DXGraphicsStateGuardian::unexpected return code from TestCoopLevel: " << ConvD3DErrorToString(hr) << endl;   
					return;
				}

			#ifdef _DEBUG
			    dxgsg_cat.debug() << "DXGraphicsStateGuardian:: other app relinquished exclusive mode, refilling surfs...\n";
			#endif
				RestoreAllVideoSurfaces();  
			#ifdef _DEBUG
			    dxgsg_cat.debug() << "DXGraphicsStateGuardian:: refill done...\n";			
			#endif

				_dx_ready = TRUE;
				return;	 // need to re-render scene before we can display it
			} else {
				dxgsg_cat.error() << "DXGraphicsStateGuardian::show_frame() - Blt failed : " << ConvD3DErrorToString(hr) << endl;
				exit(1);
			}
		}

		// right now, we force sync to v-blank  (time from now up to vblank is wasted)
		// this keeps calling processes from trying to render more frames than the refresh
		// rate since (as implemented right now) they expect this call to block
		hr =  _pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		if(hr != DD_OK) {
			dxgsg_cat.error() << "DXGraphicsStateGuardian::WaitForVerticalBlank() failed : " << ConvD3DErrorToString(hr) << endl;
			exit(1);
		}
	}
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
