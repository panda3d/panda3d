// Filename: dxGraphicsStateGuardian.cxx
// Created by:  mike (02Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "dxSavedFrameBuffer.h"
#include "config_dxgsg.h"
#include "dxGraphicsStateGuardian.h"
#include <pStatTimer.h>
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

#include <pandabase.h>

TypeHandle DXGraphicsStateGuardian::_type_handle;

// bit masks used for drawing primitives
#define PerTexcoord  8
#define PerColor  4
#define PerNormal 2
#define PerCoord  1

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
  _fvf_buf = NULL;
  _sav_fvf = new char[VERT_BUFFER_SIZE];  // allocate storage for vertex info.
  _dx_ready = false;

  _pri = _zbuf = _back = NULL;
  _pDD = NULL;
  _d3dDevice = NULL;

  _cNumTexPixFmts = 0;
  _pTexPixFmts = NULL;
  _pCurTexContext = NULL;
  
  // Create a default RenderTraverser.
  if (dx_cull_traversal) {
    _render_traverser = 
      new CullTraverser(this, RenderRelation::get_class_type());
  } else {
    _render_traverser = 
      new DirectRenderTraverser(this, RenderRelation::get_class_type());
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
  delete [] _sav_fvf;
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

  // All implementations have the following buffers. (?)
  _buffer_mask = (RenderBuffer::T_color |
		  RenderBuffer::T_depth |
		  RenderBuffer::T_stencil |
		  RenderBuffer::T_accum );

  // WBD  for now, let's assume a back buffer too);
  _buffer_mask |= RenderBuffer::T_back;

  _current_projection_mat = LMatrix4f::ident_mat();
  _projection_mat_stack_count = 0;

#ifdef WBD_GL_MODE

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
  ta->issue(this);


  // Check to see if we have double-buffering.
  GLboolean has_back;
  glGetBooleanv(GL_DOUBLEBUFFER, &has_back);
  if (!has_back) {
    _buffer_mask &= ~RenderBuffer::T_back;
  }

  // Check to see if we have stereo (and therefore a right buffer).
  GLboolean has_stereo;
  glGetBooleanv(GL_STEREO, &has_stereo);
  if (!has_stereo) {
    _buffer_mask &= ~RenderBuffer::T_right;
  }

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

  // Set up the specific state values to GL's known initial values.
  _draw_buffer_mode = (has_back) ? GL_BACK : GL_FRONT;
  _read_buffer_mode = (has_back) ? GL_BACK : GL_FRONT;
  _shade_model_mode = GL_SMOOTH;
  glFrontFace(GL_CCW);

  _line_width = 1.0;
  _point_size = 1.0;
  _depth_mask = false;
  _fog_mode = GL_EXP;
  _alpha_func = GL_ALWAYS;
  _alpha_func_ref = 0;

  _pack_alignment = 4;
  _unpack_alignment = 4;

  // Set up all the enabled/disabled flags to GL's known initial
  // values: everything off.
  _multisample_enabled = false;
  _line_smooth_enabled = false;
  _point_smooth_enabled = false;
  _color_material_enabled = false;
  _scissor_enabled = false;
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

  _scissor_x = _scissor_y = _scissor_height = _scissor_width = 0;

  // Dither is on by default in GL, let's turn it off
  _dither_enabled = true;
  enable_dither(false);

  // Stencil test is off by default
  _stencil_test_enabled = false;
  _stencil_func = GL_NOTEQUAL;
  _stencil_op = GL_REPLACE;

  // Antialiasing.
  enable_line_smooth(false);
  enable_multisample(true);

  // Set up the light id map
  glGetIntegerv( GL_MAX_LIGHTS, &_max_lights );
  _available_light_ids = PTA(Light*)(_max_lights);
  _light_enabled = new bool[_max_lights];
  _cur_light_enabled = new bool[_max_lights];
  int i;
  for (i = 0; i < _max_lights; i++ ) {
    _available_light_ids[i] = NULL;
    _light_enabled[i] = false;
  }

  // Set up the clip plane id map
  glGetIntegerv(GL_MAX_CLIP_PLANES, &_max_clip_planes);
  _available_clip_plane_ids = PTA(PlaneNode*)(_max_clip_planes);
  _clip_plane_enabled = new bool[_max_clip_planes];
  _cur_clip_plane_enabled = new bool[_max_clip_planes];
  for (i = 0; i < _max_clip_planes; i++) {
    _available_clip_plane_ids[i] = NULL;
    _clip_plane_enabled[i] = false;
  }

  if (dx_cheap_textures) {
    dxgsg_cat.info()
      << "Setting glHint() for fastest textures.\n";
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	}


#else
  _issued_color_enabled = false;

  _buffer_mask &= ~RenderBuffer::T_right;  // test for these later

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

  _pack_alignment = 4;
  _unpack_alignment = 4;

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

#endif   // WBD_GL_MODE
}

HRESULT CALLBACK EnumTexFmtsCallback( LPDDPIXELFORMAT pddpf, VOID* param )  
{
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
init_dx(  LPDIRECTDRAW7		context,
		  LPDIRECTDRAWSURFACE7  pri,
		  LPDIRECTDRAWSURFACE7  back,
		  LPDIRECTDRAWSURFACE7  zbuf,
		  LPDIRECT3D7          pD3D,
		  LPDIRECT3DDEVICE7    pDevice,
		  RECT viewrect)
{
  _pDD = context;
  _pri = pri;
  _back = back;
  _zbuf = zbuf;
  _d3d = pD3D;
  _d3dDevice = pDevice;
  _view_rect = viewrect;
  HRESULT hr;

  _pTexPixFmts = new DDPIXELFORMAT[MAX_DX_TEXPIXFMTS];

  assert(_pTexPixFmts!=NULL);

  if (pDevice->EnumTextureFormats(EnumTexFmtsCallback, this) != S_OK) {
      dxgsg_cat.error() << "EnumTextureFormats failed!!\n";
  }

  D3DDEVICEDESC7 D3DDevDesc;

  if(FAILED(hr = pDevice->GetCaps(&D3DDevDesc))) {
    dxgsg_cat.fatal() << "GetCaps failed on Device\n";
    exit(1);
  }

  if((dx_decal_type==GDT_offset) && !(D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBIAS)) {
#ifdef _DEBUG
          dxgsg_cat.error() << "dx-decal-type 'offset' not supported by hardware, switching to decal masking\n";
#endif
          dx_decal_type = GDT_mask;
  } 

  if((dx_decal_type==GDT_mask) && !(D3DDevDesc.dpcTriCaps.dwMiscCaps & D3DPMISCCAPS_MASKPLANES)) {
#ifdef _DEBUG
          dxgsg_cat.error() << "No hardware support for colorwrite disabling, switching to dx-decal-type 'mask' to 'blend'\n";
#endif
          dx_decal_type = GDT_blend;
  }

  if(((dx_decal_type==GDT_blend)||(dx_decal_type==GDT_mask)) && !(D3DDevDesc.dpcTriCaps.dwMiscCaps & D3DPMISCCAPS_MASKZ))
         dxgsg_cat.error() << "dx-decal-type mask impossible to implement, no hardware support for Z-masking, decals will not appear correctly\n";

#define REQUIRED_BLENDCAPS (D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE|D3DPBLENDCAPS_SRCCOLOR|D3DPBLENDCAPS_INVSRCCOLOR| \
                            D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_INVSRCALPHA | D3DPBLENDCAPS_DESTALPHA|D3DPBLENDCAPS_INVDESTALPHA|D3DPBLENDCAPS_DESTCOLOR|D3DPBLENDCAPS_INVDESTCOLOR)

  if(((D3DDevDesc.dpcTriCaps.dwSrcBlendCaps & REQUIRED_BLENDCAPS)!=REQUIRED_BLENDCAPS) ||
     ((D3DDevDesc.dpcTriCaps.dwDestBlendCaps & REQUIRED_BLENDCAPS)!=REQUIRED_BLENDCAPS)) {
         dxgsg_cat.error() << "device is missing texture blending capabilities, blending may not work correctly\n";
  }

  if(!(D3DDevDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY)) {
         dxgsg_cat.error() << "device is missing texture transparency capability, transparency will work correctly!!!\n";
  }

  // just require trilinear.  if it can do that, it can probably do all the lesser point-sampling variations too
#define REQUIRED_TEXFILTERCAPS (D3DPTFILTERCAPS_MAGFLINEAR |  D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_LINEAR)
  if((D3DDevDesc.dpcTriCaps.dwTextureFilterCaps & REQUIRED_TEXFILTERCAPS)!=REQUIRED_TEXFILTERCAPS) {
         dxgsg_cat.error() << "device is missing texture bilinear filtering capability, textures may appear blocky!!!\n";
  }

#define REQUIRED_MIPMAP_TEXFILTERCAPS (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_LINEARMIPLINEAR)
  if((D3DDevDesc.dpcTriCaps.dwTextureFilterCaps & REQUIRED_MIPMAP_TEXFILTERCAPS)!=REQUIRED_MIPMAP_TEXFILTERCAPS) {
         dxgsg_cat.error() << "device is missing tri-linear mipmap filtering capability, texture mipmaps may not supported!!!\n";
  }

  SetRect(&clip_rect, 0,0,0,0);		// no clip rect set

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
  enable_multisample(true);

  _max_lights = 16;		// assume for now.  This is totally arbitrary
  _available_light_ids = PTA(Light*)(_max_lights);
  _light_enabled = new bool[_max_lights];
  _cur_light_enabled = new bool[_max_lights];
  int i;
  for (i = 0; i < _max_lights; i++ ) {
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
  SetRect(&clip_rect, 0,0,0,0);		// no clip rect set

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

  _d3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);  // disables texturing
  ta->issue(this); // no curtextcontext, this does nothing.  dx should already be properly inited above anyway

}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
clear(const RenderBuffer &buffer) {

  activate();

  nassertv(buffer._gsg == this);
  int buffer_type = buffer._buffer_type;

  int   flags = 0;

  if (buffer_type & RenderBuffer::T_depth)
	  flags |= 	D3DCLEAR_ZBUFFER;
  if (buffer_type & RenderBuffer::T_back)		//set appropriate flags
	  flags |= 	D3DCLEAR_TARGET;
  if (buffer_type & RenderBuffer::T_stencil)
	  flags |= 	D3DCLEAR_STENCIL;

  D3DCOLOR  clear_colr = D3DRGBA(_color_clear_value[0],_color_clear_value[1],
	  _color_clear_value[2], /*1.0*/_color_clear_value[3]);

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
enable_light(int light, bool val)
{
  if ( _light_enabled[light] != val )
    {
	_light_enabled[light] = val;
    HRESULT res = _d3dDevice->LightEnable( light, val  );

#ifdef GSG_VERBOSE
    dxgsg_cat.debug()
	    << "LightEnable(" << light << "=" << val << ")" << endl;
#endif

	return (res == DD_OK);
    }
  return TRUE;
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::prepare_display_region
//       Access: Public, Virtual
//  Description: Prepare a display region for rendering (set up
//		 scissor region and viewport)
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
	clip_rect = cliprect;		// store the normalized clip rect
	cliprect.left += _view_rect.left;
	cliprect.right += _view_rect.left;
	cliprect.top += _view_rect.top;
	cliprect.bottom += _view_rect.top;
	RGNDATA *rgn_data = (RGNDATA *)malloc(sizeof(RGNDATAHEADER) + sizeof(RECT));
    HRGN hrgn = CreateRectRgn(cliprect.left, cliprect.top, cliprect.right, cliprect.bottom);
    GetRegionData(hrgn, sizeof(RGNDATAHEADER) + sizeof(RECT), rgn_data);

	if (_pri->GetClipper(&Clipper) != DD_OK)
		{
		result = _pDD->CreateClipper(0, &Clipper, NULL);
		result = Clipper->SetClipList(rgn_data, 0);
		result = _pri->SetClipper(Clipper);
		}
	else {
		result = Clipper->SetClipList(rgn_data, 0 );
		if (result == DDERR_CLIPPERISUSINGHWND)
			{
			result = _pri->SetClipper(NULL);
			result = _pDD->CreateClipper(0, &Clipper, NULL);
			result = Clipper->SetClipList(rgn_data, 0 ) ;
			result = _pri->SetClipper(Clipper);
			}
		}

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
   if (!_dx_ready) return;

  _win->begin_frame();
  _d3dDevice->BeginScene();

/*    D3DMATRIX ident;
    ident._11 = ident._32 = ident._44 = 1.0f;
    ident._23 = -1.0f;
    ident._12 = ident._13 = ident._14 = ident._41 = 0.0f;
    ident._21 = ident._22 = ident._24 = ident._42 = 0.0f;
    ident._31 = ident._33 = ident._34 = ident._43 = 0.0f;

    _d3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &ident);
 */
#ifdef GSG_VERBOSE
  dxgsg_cat.debug()
    << "begin frame --------------------------------------------" << endl;
#endif

  _decal_level = 0;

  // First, clear the entire window.
  PT(DisplayRegion) win_dr = 
    _win->make_scratch_display_region(_win->get_width(), _win->get_height());
  clear(get_render_buffer(RenderBuffer::T_back | RenderBuffer::T_depth), win_dr);

  // Now render each of our layers in order.
  int max_channel_index = _win->get_max_channel_index();
  for (int c = 0; c < max_channel_index; c++) 
	{
    if (_win->is_channel_defined(c)) 
		{
		GraphicsChannel *chan = _win->get_channel(c);
		if (chan->is_active()) 
			{
			int num_layers = chan->get_num_layers();
			for (int l = 0; l < num_layers; l++) 
				{
				GraphicsLayer *layer = chan->get_layer(l);
				if (layer->is_active()) 
					{
				    int num_drs = layer->get_num_drs();
				    for (int d = 0; d < num_drs; d++) 
						{
						DisplayRegion *dr = layer->get_dr(d);
						Camera *cam = dr->get_camera();
			    
				  // For each display region, render from the camera's view.
						if (dr->is_active() && cam != (Camera *)NULL && 
						  cam->is_active() && cam->get_scene() != (Node *)NULL) 
							{
						 	DisplayRegionStack old_dr = push_display_region(dr);
						 	prepare_display_region();
						 	render_scene(cam->get_scene(), cam, initial_state);
						 	pop_display_region(old_dr);
							}
						}
					}		//		if (layer->is_active()) 
				}
		    }		//		if (chan->is_active()) 
		}
	}	//  for (int c = 0; c < max_channel_index; c++) 

  // Now we're done with the frame processing.  Clean up.

  // Let's turn off all the lights we had on, and clear the light
  // cache--to force the lights to be reissued next frame, in case
  // their parameters or positions have changed between frames.
  
  for (int i = 0; i < _max_lights; i++) 
	{
	enable_light(i, false);
	_available_light_ids[i] = NULL;
	}

  // Also force the lighting state to unlit, so that issue_light()
  // will be guaranteed to be called next frame even if we have the
  // same set of light pointers we had this frame.
  NodeAttributes state;
  state.set_attribute(LightTransition::get_class_type(), new LightAttribute);
  state.set_attribute(TextureTransition::get_class_type(), new TextureAttribute);
  set_state(state, false);

  // All this work to undo the lighting state each frame doesn't seem
  // ideal--there may be a better way.  Maybe if the lights were just
  // more aware of whether their parameters or positions have changed
  // at all?

  _d3dDevice->EndScene();
  
  _win->end_frame();  
  show_frame();

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
render_scene(Node *root, const ProjectionNode *projnode,
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
		Node *subgraph, const ProjectionNode *projnode,
		const AllAttributesWrapper &initial_state,
		const AllTransitionsWrapper &net_trans) {
  activate();
  const ProjectionNode *old_projection_node = _current_projection_node;
  _current_projection_node = projnode;
  LMatrix4f old_projection_mat = _current_projection_mat;

  // The projection matrix must always be right-handed Y-up, even if
  // our coordinate system of choice is otherwise, because certain GL
  // calls (specifically glTexGen(GL_SPHERE_MAP)) assume this kind of
  // a coordinate system.  Sigh.  In order to implement a Z-up
  // coordinate system, we'll store the Z-up conversion in the
  // modelview matrix.
  LMatrix4f projection_mat = 
    projnode->get_projection()->get_projection_mat(CS_yup_left);

  _current_projection_mat = projection_mat;
  _projection_mat_stack_count++;

  // We load the projection matrix directly.
  HRESULT res = 
	 _d3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,
					(LPD3DMATRIX) _current_projection_mat.get_data());

  // We infer the modelview matrix by doing a wrt on the projection
  // node.
  LMatrix4f modelview_mat;
  get_rel_mat(subgraph, _current_projection_node, modelview_mat);
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
  // do a push/pop matrix, but OpenGL doesn't promise more than 2
  // levels in the projection matrix stack, so we'd better do it in
  // the CPU.
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

#ifdef WBD_GL_MODE
  call_glPointSize(geom->get_size());

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

  // Points don't make a distinction between flat and smooth shading.
  // We'll leave the shade model alone.

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
  issuer.issue_normal(G_OVERALL, ni); 
  
  glBegin(GL_POINTS);
  
  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);
    
    // Draw per vertex, same thing.
    issuer.issue_color(G_PER_VERTEX, ci);
    issuer.issue_normal(G_PER_VERTEX, ni);

    issuer.issue_vertex(G_PER_VERTEX, vi);
  }

  glEnd();
#else		  // The DX Way
  int nPrims = geom->get_num_prims();

  perVertex = 0;
  if (geom->get_binding(G_COORD) == G_PER_VERTEX)  perVertex |= PerCoord;
  if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) perVertex |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_VERTEX) perVertex |= PerColor;

  perPrim = 0;
  if (geom->get_binding(G_NORMAL) == G_PER_PRIM) perPrim |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_PRIM) perPrim |= PerColor;

  size_t vertex_size = draw_prim_setup(geom);

  nassertv(_fvf_buf == NULL);    // make sure the storage pointer is clean.
  nassertv(nPrims * vertex_size < VERT_BUFFER_SIZE);
  _fvf_buf = _sav_fvf;			// _fvf_buf changes,  sav_fvf doesn't

	// iterate through the point
  draw_prim_inner_loop2(nPrims, geom, perPrim);

  _d3dDevice->DrawPrimitive(D3DPT_POINTLIST, p_flags, _sav_fvf, nPrims, NULL);

  _fvf_buf = NULL;

#endif				// WBD_GL_MODE
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
#else		 // the DX way

  int nPrims = geom->get_num_prims();

  perVertex = 0;
  if (geom->get_binding(G_COORD) == G_PER_VERTEX)  perVertex |= PerCoord;
  if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) perVertex |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_VERTEX) perVertex |= PerColor;

  perPrim = 0;
  if (geom->get_binding(G_NORMAL) == G_PER_PRIM) perPrim |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_PRIM) perPrim |= PerColor;

  size_t vertex_size = draw_prim_setup(geom);

  void *_tmp_fvf = NULL;
  nassertv(_fvf_buf == NULL);    // make sure the storage pointer is clean.
//  nassertv(nPrims * 2 * vertex_size < VERT_BUFFER_SIZE);
  
  if (nPrims * 2 * vertex_size > VERT_BUFFER_SIZE)
	  {
	  _fvf_buf = _tmp_fvf = new char[nPrims * 2 * vertex_size];
	  }
  else  _fvf_buf = _sav_fvf;			// _fvf_buf changes,  sav_fvf doesn't
  
  for (int i = 0; i < nPrims; i++) 
	{
	if (perPrim & PerColor)
		{
	    p_color = geom->get_next_color(ci);		// set primitive color if there is one.
	    p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
	    }
	if (perPrim & PerNormal)
		 p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.
	draw_prim_inner_loop(2, geom);
	}

  if (_tmp_fvf == NULL)
	  _d3dDevice->DrawPrimitive(D3DPT_LINELIST, p_flags, _sav_fvf, nPrims*2, NULL);
  else {
	  _d3dDevice->DrawPrimitive(D3DPT_LINELIST, p_flags, _tmp_fvf, nPrims*2, NULL);
	  delete [] _tmp_fvf;
	  }

  _fvf_buf = NULL;

#endif				// WBD_GL_MODE
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

  perVertex = 0;
  if (geom->get_binding(G_COORD) == G_PER_VERTEX)  perVertex |= PerCoord;
  if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) perVertex |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_VERTEX) perVertex |= PerColor;

  perComp = 0;
  if (geom->get_binding(G_NORMAL) == G_PER_COMPONENT) perComp |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_COMPONENT) perComp |= PerColor;

  perPrim = 0;
  if (geom->get_binding(G_NORMAL) == G_PER_PRIM) perPrim |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_PRIM) perPrim |= PerColor;

  size_t vertex_size = draw_prim_setup(geom);

  for (int i = 0; i < nPrims; i++) 
	{
	if (perPrim & PerColor)
		{
	    p_color = geom->get_next_color(ci);		// set primitive color if there is one.
	    p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
	    }

    int nVerts = *(plen++);
    nassertv(nVerts >= 2);

	nassertv(_fvf_buf == NULL);    // make sure the storage pointer is clean.
    nassertv(nVerts * vertex_size < VERT_BUFFER_SIZE);
//	void *sav_fvf = new char[nVerts * vertex_size];  // allocate storage for vertex info.
	_fvf_buf = _sav_fvf;			// _fvf_buf changes,  sav_fvf doesn't
	
	draw_prim_inner_loop2(nVerts, geom, perComp);

	_d3dDevice->DrawPrimitive(D3DPT_LINESTRIP,  p_flags, _sav_fvf, nVerts, NULL);

	_fvf_buf = NULL;
	}


#endif				// WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_sprite
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_sprite(const GeomSprite *geom) {
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
#endif				// WBD_GL_MODE
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_prim_setup
//       Access: Private
//  Description: This adds data to the flexible vertex format
////////////////////////////////////////////////////////////////////
size_t DXGraphicsStateGuardian::
draw_prim_setup(const Geom *geom) 
{

  // set up iterators
  vi = geom->make_vertex_iterator();
  ni = geom->make_normal_iterator();
  ti = geom->make_texcoord_iterator();
  ci = geom->make_color_iterator();

  //  Set the flags for the flexible vertex format and compute the bytes
  //  required to store a single vertex.
  p_flags = 0;
  size_t  vertex_size = 0;
  if (geom->get_binding(G_COLOR) != G_OFF || _issued_color_enabled)
	  {  p_flags |= D3DFVF_DIFFUSE;   vertex_size += sizeof(D3DCOLOR);  }
  if (geom->get_binding(G_COORD) != G_OFF)
	  {  p_flags |= D3DFVF_XYZ;   vertex_size += sizeof(D3DVALUE) * 3;  }
  if (geom->get_binding(G_NORMAL) != G_OFF)
	  {  p_flags |= D3DFVF_NORMAL;  vertex_size += sizeof(D3DVALUE) * 3;  }
  if (geom->get_binding(G_TEXCOORD) != G_OFF)
	  {  p_flags |= D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0);
		 vertex_size += sizeof(float) * 2;
	  }


  if (geom->get_binding(G_COLOR) == G_OVERALL)
	  {
	  p_color = geom->get_next_color(ci);		// set overall color if there is one
  	  p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
	  }

  if (geom->get_binding(G_NORMAL) == G_OVERALL)
		 p_normal = geom->get_next_normal(ni);    // set overall normal if there is one

  if (_issued_color_enabled)
	  {
	  p_colr = _issued_color;		// set primitive color if there is one.
	  perVertex &= ~PerColor;
	  perPrim &= ~PerColor;
	  perComp &= ~PerColor;
	  }

  // If we have per-vertex colors or normals, we need smooth shading.
  // Otherwise we want flat shading for performance reasons.
  if (perVertex & ((wants_colors() ? PerColor : 0) | 
		   (wants_normals() ? PerNormal : 0)))
		  _d3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
  else
		  _d3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT);

  return vertex_size;
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_prim_inner_loop
//       Access: Private
//  Description: This adds data to the flexible vertex format
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_prim_inner_loop(int loops, const Geom *geom) 
{
	while (--loops >= 0)
		{
		switch(perVertex)
			{
			case 3:
				p_normal = geom->get_next_normal(ni);
			case 1:
				p_vertex = geom->get_next_vertex(vi);
				break;
			case 5:
				p_vertex = geom->get_next_vertex(vi);
			case 4:
				p_color = geom->get_next_color(ci);
				p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
				break;
			case 7:
				p_vertex = geom->get_next_vertex(vi);
			case 6:
				p_color = geom->get_next_color(ci);
				p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
			case 2:
				p_normal = geom->get_next_normal(ni);
				break;
			case 9:
				p_vertex = geom->get_next_vertex(vi);
			case 8:
				p_texcoord = geom->get_next_texcoord(ti);
				break;
			case 11:
				p_vertex = geom->get_next_vertex(vi);
			case 10:
				p_normal = geom->get_next_normal(ni);
				p_texcoord = geom->get_next_texcoord(ti);
				break;
			case 13:
				p_vertex = geom->get_next_vertex(vi);
			case 12:
				p_color = geom->get_next_color(ci);
				p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
				p_texcoord = geom->get_next_texcoord(ti);
				break;
			case 15:
				p_vertex = geom->get_next_vertex(vi);
			case 14:
				p_normal = geom->get_next_normal(ni);
				p_color = geom->get_next_color(ci);
				p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
				p_texcoord = geom->get_next_texcoord(ti);
				break;
			}
		if (p_flags & D3DFVF_XYZ)
			add_to_FVF((void *)&p_vertex, sizeof(D3DVECTOR));
		if (p_flags & D3DFVF_NORMAL)
			add_to_FVF((void *)&p_normal, sizeof(D3DVECTOR));
		if (p_flags & D3DFVF_DIFFUSE)
			add_to_FVF((void *)&p_colr, sizeof(D3DCOLOR));
		if (p_flags & D3DFVF_TEXCOUNT_MASK)
			add_to_FVF((void *)&p_texcoord, sizeof(TexCoordf));
		}
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_prim_inner_loop2
//       Access: Private
//  Description: This adds data to the flexible vertex format with a check
//               for component normals and color
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_prim_inner_loop2(int loops, const Geom *geom, short& per ) 
{
	while (--loops >= 0)
		{
		if (per & PerColor)
		  {
		  p_color = geom->get_next_color(ci);		// set overall color if there is one
		  p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
		  }
		if (per & PerNormal)
			p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.

		switch(perVertex)
			{
			case 3:
				p_normal = geom->get_next_normal(ni);
			case 1:
				p_vertex = geom->get_next_vertex(vi);
				break;
			case 5:
				p_vertex = geom->get_next_vertex(vi);
			case 4:
				p_color = geom->get_next_color(ci);
				p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
				break;
			case 7:
				p_vertex = geom->get_next_vertex(vi);
			case 6:
				p_color = geom->get_next_color(ci);
				p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
			case 2:
				p_normal = geom->get_next_normal(ni);
				break;
			case 9:
				p_vertex = geom->get_next_vertex(vi);
			case 8:
				p_texcoord = geom->get_next_texcoord(ti);
				break;
			case 11:
				p_vertex = geom->get_next_vertex(vi);
			case 10:
				p_normal = geom->get_next_normal(ni);
				p_texcoord = geom->get_next_texcoord(ti);
				break;
			case 13:
				p_vertex = geom->get_next_vertex(vi);
			case 12:
				p_color = geom->get_next_color(ci);
				p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
				p_texcoord = geom->get_next_texcoord(ti);
				break;
			case 15:
				p_vertex = geom->get_next_vertex(vi);
			case 14:
				p_normal = geom->get_next_normal(ni);
				p_color = geom->get_next_color(ci);
				p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
				p_texcoord = geom->get_next_texcoord(ti);
				break;
			}
		if (p_flags & D3DFVF_XYZ)
			add_to_FVF((void *)&p_vertex, sizeof(D3DVECTOR));
		if (p_flags & D3DFVF_NORMAL)
			add_to_FVF((void *)&p_normal, sizeof(D3DVECTOR));
		if (p_flags & D3DFVF_DIFFUSE)
			add_to_FVF((void *)&p_colr, sizeof(D3DCOLOR));
		if (p_flags & D3DFVF_TEXCOUNT_MASK)
			add_to_FVF((void *)&p_texcoord, sizeof(TexCoordf));
		}
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::add_to_FVF
//       Access: Private
//  Description: This adds data to the flexible vertex format
////////////////////////////////////////////////////////////////////
INLINE void DXGraphicsStateGuardian::
add_to_FVF(void *data,  size_t bytes) 
{
	memcpy(_fvf_buf, data, bytes);
	_fvf_buf = (char *)_fvf_buf + bytes;

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
 if(_pCurTexContext!=NULL) {
     dxgsg_cat.spam() << "Cur active DX texture: " << _pCurTexContext->_tex->get_name() << "\n";
    } 
#endif

#ifdef WBD_GL_MODE
  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::NormalIterator ni = geom->make_normal_iterator();
  Geom::TexCoordIterator ti = geom->make_texcoord_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer::IssueColor *issue_color;

    if (!_color_transform_enabled && !_alpha_transform_enabled) {
      issue_color = issue_color_gl;
    }
    else {
      issue_color = issue_transformed_color_gl;
    }

    GeomIssuer issuer(geom, this,
              issue_vertex_gl,
              issue_normal_gl,
              issue_texcoord_gl,
              issue_color);

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
  
  glBegin(GL_TRIANGLES);
  
  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);
    issuer.issue_normal(G_PER_PRIM, ni);
    
    for (int j = 0; j < 3; j++) {
      // Draw per vertex
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
  }

  glEnd();
#else      // the DX way
  int nPrims = geom->get_num_prims();

  perVertex = 0;
  if (geom->get_binding(G_COORD) == G_PER_VERTEX)  perVertex |= PerCoord;
  if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) perVertex |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_VERTEX) perVertex |= PerColor;
  if (geom->get_binding(G_TEXCOORD) == G_PER_VERTEX) perVertex |= PerTexcoord;

  perPrim = 0;
  if (geom->get_binding(G_NORMAL) == G_PER_PRIM) perPrim |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_PRIM) perPrim |= PerColor;

  size_t vertex_size = draw_prim_setup(geom);

  nassertv(_fvf_buf == NULL);    // make sure the storage pointer is clean.
  nassertv(nPrims * 3 * vertex_size < VERT_BUFFER_SIZE);
  _fvf_buf = _sav_fvf;			// _fvf_buf changes,  sav_fvf doesn't

	// iterate through the triangle primitive

  for (int i = 0; i < nPrims; i++) 
	{
	if (perPrim & PerColor)
		{
	    p_color = geom->get_next_color(ci);		// set primitive color if there is one.
	    p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
	    }

	if (perPrim & PerNormal)
		 p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.

	draw_prim_inner_loop(3, geom);
	}

  _d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,  p_flags, _sav_fvf, nPrims*3, NULL);

  _fvf_buf = NULL;

#endif				// WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_quad
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_quad(const GeomQuad *geom) {
  activate();

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
#endif				// WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_tristrip
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_tristrip(const GeomTristrip *geom) {
	activate();

#ifdef GSG_VERBOSE
  dxgsg_cat.debug() << "draw_tristrip()" << endl;
#endif

#ifdef WBD_GL_MODE

  int nPrims = geom->get_num_prims();
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

    glBegin(GL_TRIANGLE_STRIP);

    // Per-component attributes for the first triangle?
    issuer.issue_color(G_PER_COMPONENT, ci);
    issuer.issue_normal(G_PER_COMPONENT, ni);
    
    // Draw the first three vertices.
    int v;
    for (v = 0; v < 3; v++) {
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
    
    // Now draw each of the remaining vertices.  Each vertex from
    // this point on defines a new triangle.
    for (v = 3; v < num_verts; v++) {
      // Per-component attributes?
      issuer.issue_color(G_PER_COMPONENT, ci);
      issuer.issue_normal(G_PER_COMPONENT, ni);
      
      // Per-vertex attributes.
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
    glEnd();
  }
#else

  draw_multitri(geom, D3DPT_TRIANGLESTRIP);

#endif				// WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_trifan
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_trifan(const GeomTrifan *geom) {
  activate();

#ifdef GSG_VERBOSE
  dxgsg_cat.debug() << "draw_trifan()" << endl;
#endif

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

    glBegin(GL_TRIANGLE_FAN);

    // Per-component attributes for the first triangle?
    issuer.issue_color(G_PER_COMPONENT, ci);
    issuer.issue_normal(G_PER_COMPONENT, ni);
   
    // Draw the first three vertices.
    int v;
    for (v = 0; v < 3; v++) {
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }

    // Now draw each of the remaining vertices.  Each vertex from
    // this point on defines a new triangle.
    for (v = 3; v < num_verts; v++) {
      // Per-component attributes?
      issuer.issue_color(G_PER_COMPONENT, ci);
      issuer.issue_normal(G_PER_COMPONENT, ni);

      // Per-vertex attributes.
      issuer.issue_color(G_PER_VERTEX, ci);
      issuer.issue_normal(G_PER_VERTEX, ni);
      issuer.issue_texcoord(G_PER_VERTEX, ti);
      issuer.issue_vertex(G_PER_VERTEX, vi);
    }
    glEnd();
  }
#else      // the DX way

	draw_multitri(geom, D3DPT_TRIANGLEFAN);

#endif				// WBD_GL_MODE
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_multitri
//       Access: Public, Virtual
//  Description: handles trifans and tristrips
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_multitri(const Geom *geom, D3DPRIMITIVETYPE tri_id) 
{

  int nPrims = geom->get_num_prims();
  const int *plen = geom->get_lengths();

  perVertex = 0;
  if (geom->get_binding(G_COORD) == G_PER_VERTEX)  perVertex |= PerCoord;
  if (geom->get_binding(G_NORMAL) == G_PER_VERTEX) perVertex |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_VERTEX) perVertex |= PerColor;
  if (geom->get_binding(G_TEXCOORD) == G_PER_VERTEX) perVertex |= PerTexcoord;

  perPrim = 0;
  if (geom->get_binding(G_NORMAL) == G_PER_PRIM) perPrim |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_PRIM) perPrim |= PerColor;

  perComp = 0;
  if (geom->get_binding(G_NORMAL) == G_PER_COMPONENT) perComp |= PerNormal;
  if (geom->get_binding(G_COLOR) == G_PER_COMPONENT) perComp |= PerColor;

  size_t vertex_size = draw_prim_setup(geom);

  // iterate through the triangle primitives

  for (int i = 0; i < nPrims; i++) 
	{
	if (perPrim & PerColor)
		{
	    p_color = geom->get_next_color(ci);		// set primitive color if there is one.
	    p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
	    }

	if (perPrim & PerNormal)
		 p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.

    int nVerts = *(plen++);
    nassertv(nVerts >= 3);

	nassertv(_fvf_buf == NULL);    // make sure the storage pointer is clean.
    nassertv(nVerts * vertex_size < VERT_BUFFER_SIZE);
	_fvf_buf = _sav_fvf;			// _fvf_buf changes,  sav_fvf doesn't
	
	if (perComp & PerColor)
	  {
	  p_color = geom->get_next_color(ci);		// set overall color if there is one
	  p_colr = D3DRGBA(p_color[0], p_color[1], p_color[2], p_color[3]);
	  }
	if (perComp & PerNormal)
		p_normal = geom->get_next_normal(ni);   // set primitive normal if there is one.

	// Store first triangle
	draw_prim_inner_loop(3, geom);

	// Store remaining vertices
	draw_prim_inner_loop2(nVerts-3, geom, perComp);

	_d3dDevice->DrawPrimitive(tri_id,  p_flags, _sav_fvf, nVerts, NULL);

	_fvf_buf = NULL;
	}
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::draw_sphere
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
draw_sphere(const GeomSphere *geom) {
  activate();
 
#ifdef GSG_VERBOSE
  dxgsg_cat.debug() << "draw_sphere()" << endl;
#endif
 
#ifdef WBD_GL_MODE
  int nprims = geom->get_num_prims();
  Geom::VertexIterator vi = geom->make_vertex_iterator();
  Geom::ColorIterator ci = geom->make_color_iterator();

  GeomIssuer issuer(geom, this,
                    issue_vertex_gl,
                    issue_normal_gl,
                    issue_texcoord_gl,
                    issue_color_gl);

  if (wants_normals()) {
    call_glShadeModel(GL_SMOOTH);
  } else {
    call_glShadeModel(GL_FLAT);
  }

  // Draw overall
  issuer.issue_color(G_OVERALL, ci);
 
  GLUquadricObj *sph = gluNewQuadric();
  gluQuadricNormals(sph, wants_normals() ? (GLenum)GLU_SMOOTH : (GLenum)GLU_NONE);
  gluQuadricTexture(sph, wants_texcoords() ? (GLenum)GL_TRUE : (GLenum)GL_FALSE);
  gluQuadricOrientation(sph, (GLenum)GLU_OUTSIDE);
  gluQuadricDrawStyle(sph, (GLenum)GLU_FILL);
  //gluQuadricDrawStyle(sph, (GLenum)GLU_LINE);

  for (int i = 0; i < nprims; i++) {
    // Draw per primitive
    issuer.issue_color(G_PER_PRIM, ci);

    for (int j = 0; j < 2; j++) {
      // Draw per vertex
      issuer.issue_color(G_PER_VERTEX, ci);
    }
    Vertexf center = geom->get_next_vertex(vi);
    Vertexf edge = geom->get_next_vertex(vi);
    LVector3f v = edge - center;
    float r = sqrt(dot(v, v));

    // Since gluSphere doesn't have a center parameter, we have to use
    // a matrix transform.

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(LMatrix4f::translate_mat(center).get_data());

    // Now render the sphere using GLU calls.
    gluSphere(sph, r, 16, 10);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  gluDeleteQuadric(sph);
#endif				// WBD_GL_MODE
}


////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::issue_color_transform
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_color_transform(const ColorMatrixAttribute *attrib) {
  _current_color_mat = attrib->get_matrix();

  if (_current_color_mat == LMatrix4f::ident_mat()) {
    _color_transform_enabled = false;
  }
  else {
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

  if (_current_alpha_offset == 0 && _current_alpha_scale == 1) {
    _alpha_transform_enabled = false;
  }
  else {
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
  if(gtc->CreateTexture(_hdc, _d3dDevice,_cNumTexPixFmts,_pTexPixFmts) == NULL) {
      delete gtc;
      return NULL;
  }
#endif				// WBD_GL_MODE

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
  if(tc==NULL) {
     return;
  }

//  activate();  inactive

#ifdef WBD_GL_MODE
  bind_texture(tc);
#else
  specify_texture(tc->_texture);
  apply_texture_immediate(DCAST(DXTextureContext, tc));
#endif
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
release_texture(TextureContext *tc) {
  activate();
  DXTextureContext *gtc = DCAST(DXTextureContext, tc);
  Texture *tex = tc->_texture;

#ifdef WBD_GL_MODE
  glDeleteTextures(1, &gtc->_index);
  gtc->_index = 0;
#else
  gtc->DeleteTexture();
#endif				// WBD_GL_MODE
  bool erased = unmark_prepared_texture(gtc);

  // If this assertion fails, a texture was released that hadn't been
  // prepared (or a texture was released twice).
  nassertv(erased);

  tex->clear_gsg(this);

  delete gtc;
}

static int logs[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
                      4096, 0 };

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
//		 region from the framebuffer into texture memory
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
copy_texture(TextureContext *tc, const DisplayRegion *dr) {
 dxgsg_cat.fatal() << "DX copy_texture unimplemented!!!";
 return;

#if 0
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


//#ifdef WBD_GL_MODE
  bind_texture(tc);
  glCopyTexImage2D( GL_TEXTURE_2D, tex->get_level(), 
		    get_internal_image_format(pb->get_format()),
		    pb->get_xorg(), pb->get_yorg(),
		    pb->get_xsize(), pb->get_ysize(), pb->get_border() );
#endif				// WBD_GL_MODE
}

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
#endif				// WBD_GL_MODE

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

  dxgsg_cat.fatal() << "DXGSG copy_pixel_buffer unimplemented!!!";
  return;

#ifdef WBD_GL_MODE
  nassertv(pb != NULL && dr != NULL);
  activate();
  set_pack_alignment(1);

  NodeAttributes state;

  // Bug fix for RE, RE2, and VTX - need to disable texturing in order
  // for glReadPixels() to work
  // NOTE: reading the depth buffer is *much* slower than reading the
  // color buffer
  state.set_attribute(TextureTransition::get_class_type(), 
		      new TextureAttribute);
  set_state(state, false);

  int xo, yo, w, h;
  dr->get_region_pixels(xo, yo, w, h);

#ifdef GSG_VERBOSE
  dxgsg_cat.debug()
    << "glReadPixels(" << pb->get_xorg() << ", " << pb->get_yorg()
    << ", " << pb->get_xsize() << ", " << pb->get_ysize()
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
    case GL_FLOAT: 
      dxgsg_cat.debug(false) << "GL_FLOAT, "; 
      break;
  default: 
    dxgsg_cat.debug(false) << "unknown, "; 
    break;
  }
  dxgsg_cat.debug(false)
    << (void *)pb->_image.p() << ")" << endl;
#endif

  glReadPixels( pb->get_xorg() + xo, pb->get_yorg() + yo, 
		pb->get_xsize(), pb->get_ysize(), 
		get_external_image_format(pb->get_format()),
		get_image_type(pb->get_image_type()),
		pb->_image.p() );

  nassertv(!pb->_image.empty());
#endif				// WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::copy_pixel_buffer
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
copy_pixel_buffer(PixelBuffer *pb, const DisplayRegion *dr, 
		  const RenderBuffer &rb) {
  activate();
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
  case GL_FLOAT:
    dxgsg_cat.debug(false) << "GL_FLOAT, ";
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
#endif				// WBD_GL_MODE
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
void DXGraphicsStateGuardian::apply_material( Material* material )
{
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
  _d3dDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, 
							get_fog_mode_type(fog->get_mode()));
  _d3dDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, 
							get_fog_mode_type(fog->get_mode()));

  switch(fog->get_mode()) 
	{
    case Fog::M_linear:
		{
   	    float fog_start = fog->get_start();	  
	    float fog_end = fog->get_end();	  
   	    _d3dDevice->SetRenderState( D3DRENDERSTATE_FOGSTART, 
			  *((LPDWORD) (&fog_start)) );
   	    _d3dDevice->SetRenderState( D3DRENDERSTATE_FOGEND, 
			  *((LPDWORD) (&fog_end)) );
		}
        break;
    case Fog::M_exponential:
    case Fog::M_super_exponential:
		{
	    float fog_density = fog->get_density();	  
   	    _d3dDevice->SetRenderState( D3DRENDERSTATE_FOGDENSITY, 
			  *((LPDWORD) (&fog_density)) );
		}
        break;
    case Fog::M_spline:
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
void DXGraphicsStateGuardian::apply_light( PointLight* light )
{
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
	alight.dcvAmbient  =  black	;
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

#endif				// WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::apply_light( DirectionalLight* light )
{
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
#else			// DX Directional light
	D3DCOLORVALUE black;
	black.r = black.g = black.b = black.a = 0.0f;
	
	D3DLIGHT7  alight;
	ZeroMemory(&alight, sizeof(D3DLIGHT7));

	alight.dltType =  D3DLIGHT_DIRECTIONAL;
	alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light->get_color().get_data());
	alight.dcvAmbient  =  black	;
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
						  
#endif				// WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::apply_light( Spotlight* light )
{
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
#else		// DX Spotlight
	D3DCOLORVALUE black;
	black.r = black.g = black.b = black.a = 0.0f;
	
	D3DLIGHT7  alight;
	ZeroMemory(&alight, sizeof(D3DLIGHT7));

	alight.dltType =  D3DLIGHT_SPOT;
	alight.dcvAmbient  =  black	;
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

#endif				// WBD_GL_MODE 
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_light
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::apply_light( AmbientLight* light )
{
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
	  D3DVALUE x,y,z;	// position
	  D3DVALUE nx,ny,nz; // normal
	  D3DCOLOR  diff;	// diffuse color
		}		 VERTFORMAT;

	VERTFORMAT vert_buf[] = {
	  {0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(1.0, 0.0, 0.0, 1.0)},    // red
	  {3.0, 0.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(1.0, 0.0, 0.0, 1.0)},    // red
	  {0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(0.0, 1.0, 0.0, 1.0)},    // grn
	  {0.0, 3.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(0.0, 1.0, 0.0, 1.0)},    // grn
	  {0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  D3DRGBA(0.0, 0.0, 1.0, 1.0)},    // blu
	  {0.0, 0.0, 3.0,  0.0, -1.0, 0.0,  D3DRGBA(0.0, 0.0, 1.0, 1.0)},    // blu
	  };

	_d3dDevice->DrawPrimitive(D3DPT_LINELIST, D3DFVF_DIFFUSE | D3DFVF_XYZ | D3DFVF_NORMAL,
		  vert_buf, 6, NULL);

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
#endif				// WBD_GL_MODE
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_color
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_color(const ColorAttribute *attrib) {
  activate();
  if (attrib->is_on()&& attrib->is_real()) 
	 {
	 _issued_color_enabled = true;
     Colorf c = attrib->get_color();
	 _issued_color = D3DRGBA(c[0], c[1], c[2], c[3]);
	 }
  else _issued_color_enabled = false;
}

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
#endif				// WBD_GL_MODE
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
    Material *material = attrib->get_material();
    nassertv(material != (Material *)NULL);
    material->apply(this);
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


  switch(mode)
	  {
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
void DXGraphicsStateGuardian::issue_light(const LightAttribute *attrib )
{
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
      if (enable_light(_cur_light_id, true))  _cur_light_enabled[_cur_light_id] = true;
      
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
reset_ambient() 
{
	_lmodel_ambient += 2.0f;
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_color_blend		  //
//       Access: Public, Virtual								  //
//  Description:												  //
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

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_texture_apply
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_texture_apply(const TextureApplyAttribute *attrib) {

   switch(attrib->get_mode()) {
       case TextureApplyProperty::M_modulate: 
           // emulates GL_MODULATE glTexEnv mode
           // want to multiply tex-color*pixel color to emulate GL modulate blend (see glTexEnv)
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

           break;
       case TextureApplyProperty::M_decal:
           // emulates GL_DECAL glTexEnv mode
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_BLENDTEXTUREALPHA );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

           break;           
       case TextureApplyProperty::M_replace: 
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
           break;           
       case TextureApplyProperty::M_add: 
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_ADD );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

           // since I'm making up 'add' mode, use modulate.  "adding" alpha never makes sense right?
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
           _d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

           break;           
       case TextureApplyProperty::M_blend: 
           dxgsg_cat.error()
             << "Impossible to emulate GL_BLEND in DX exactly " << (int) attrib->get_mode() << endl;
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
            dxgsg_cat.error() << "Unknown texture blend mode " << (int) attrib->get_mode() << endl;
            break;
   }
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

  if (mode == DepthTestProperty::M_none) 
	{
    _depth_test_enabled = false;
 	_d3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
	}
  else {
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
  activate();
  _d3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, attrib->is_on());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::issue_stencil
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
issue_stencil(const StencilAttribute *attrib) {
  activate();

  StencilProperty::Mode mode = attrib->get_mode();
  if (mode == StencilProperty::M_none) {
    enable_stencil_test(false);

  } else {
    enable_stencil_test(true);
	_d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFUNC, get_stencil_func_type(mode));
	_d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILPASS, get_stencil_action_type(attrib->get_action()));
	_d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFAIL, get_stencil_action_type(attrib->get_action()));
	_d3dDevice->SetRenderState(D3DRENDERSTATE_STENCILZFAIL, get_stencil_action_type(attrib->get_action()));
  }
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
issue_clip_plane(const ClipPlaneAttribute *attrib)
{
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
issue_transparency(const TransparencyAttribute *attrib )
{
  activate();

  TransparencyProperty::Mode mode = attrib->get_mode();

  switch (mode) {
  case TransparencyProperty::M_none:
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
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
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
    enable_blend(true);
    enable_alpha_test(false);
    call_dxBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
	break;
  case TransparencyProperty::M_multisample:
    enable_multisample_alpha_one(true);
    enable_multisample_alpha_mask(true);
    enable_blend(false);
    enable_alpha_test(false);
    break;
  case TransparencyProperty::M_multisample_mask:
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(true);
    enable_blend(false);
    enable_alpha_test(false);
    break;
  case TransparencyProperty::M_binary:
    enable_multisample_alpha_one(false);
    enable_multisample_alpha_mask(false);
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
  activate();
  enable_line_smooth(attrib->is_on());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::wants_normals
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian::
wants_normals() const {
  return (_lighting_enabled || _normals_enabled);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::wants_texcoords
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian::
wants_texcoords() const {
  return _texturing_enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::wants_colors
//       Access: Public, Virtual
//  Description: Returns true if the GSG should issue geometry color
//               commands, false otherwise.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian::
wants_colors() const { 
  // If we have scene graph color enabled, return false to indicate we
  // shouldn't bother issuing geometry color commands.

  const ColorAttribute *catt;
  if (!get_attribute_into(catt, _state, ColorTransition::get_class_type())) {
    // No scene graph color at all.
    return true;
  }

  // We should issue geometry colors only if the scene graph color is
  // off.
  return catt->is_off();
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

  if (dx_decal_type == GDT_offset) {

#define POLYGON_OFFSET_MULTIPLIER 2
    // Just draw the base geometry normally.
    base_geom->draw(this);
	_d3dDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, POLYGON_OFFSET_MULTIPLIER * _decal_level); // _decal_level better not be higher than 4!
  } else {
    if (_decal_level > 1) 
	      base_geom->draw(this);  // If we're already decaling, just draw the geometry.
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

  if (dx_decal_type == GDT_offset) {
    // Restore the Zbias offset.
	_d3dDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, POLYGON_OFFSET_MULTIPLIER * _decal_level); // _decal_level better not be higher than 8!
  } else {  // for GDT_mask
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
 	  else {
        // note: not saving current planemask val, assumes this is always all 1's.  should be ok
        _d3dDevice->SetRenderState(D3DRENDERSTATE_PLANEMASK,0x0);  // note PLANEMASK is supposedly obsolete for DX7
      }

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
       } else {
          _d3dDevice->SetRenderState(D3DRENDERSTATE_PLANEMASK,0xFFFFFFFF);
       }

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
float DXGraphicsStateGuardian::
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
#endif				// WBD_GL_MODE
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::set_read_buffer
//       Access: Protected
//  Description: Sets up the glReadBuffer to render into the buffer
//               indicated by the RenderBuffer object.  This only sets
//               up the color bits; it does not affect the depth,
//               stencil, accum layers.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
set_read_buffer(const RenderBuffer &rb) {
 dxgsg_cat.fatal() << "DX set_read_buffer unimplemented!!!";
 return;

#ifdef WBD_GL_MODE
  switch (rb._buffer_type & RenderBuffer::T_color) {
  case RenderBuffer::T_front:
    call_glReadBuffer(GL_FRONT);
    break;
    
  case RenderBuffer::T_back:
    call_glReadBuffer(GL_BACK);
    break;
    
  case RenderBuffer::T_right:
    call_glReadBuffer(GL_RIGHT);
    break;
    
  case RenderBuffer::T_left:
    call_glReadBuffer(GL_LEFT);
    break;
    
  case RenderBuffer::T_front_right:
    call_glReadBuffer(GL_FRONT_RIGHT);
    break;
    
  case RenderBuffer::T_front_left:
    call_glReadBuffer(GL_FRONT_LEFT);
    break;
    
  case RenderBuffer::T_back_right:
    call_glReadBuffer(GL_BACK_RIGHT);
    break;
    
  case RenderBuffer::T_back_left:
    call_glReadBuffer(GL_BACK_LEFT);
    break;
    
  default:
    call_glReadBuffer(GL_FRONT_AND_BACK);
  }
#endif	// WBD_GL_MODE
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::specify_texture
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
specify_texture(Texture *tex) {

  _d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSU,
				  get_texture_wrap_mode(tex->get_wrapu()));
  _d3dDevice->SetTextureStageState(0,D3DTSS_ADDRESSV,
				  get_texture_wrap_mode(tex->get_wrapv()));

  Texture::FilterType ft=tex->get_magfilter();

  switch(ft) {
      case Texture::FT_nearest:
          _d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
          break;
      case Texture::FT_linear:
          _d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
          break;
      default:
          dxgsg_cat.error() << "MipMap filter type setting for texture magfilter makes no sense,  texture: " << tex->get_name() << "\n";       
          break;
  }
  
  ft=tex->get_minfilter();

  switch(ft) {
      case Texture::FT_nearest:
          _d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
          _d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
          break;
      case Texture::FT_linear:
          _d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
          _d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
          break;
      case Texture::FT_nearest_mipmap_nearest:
          _d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
          _d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_POINT);
          break;
      case Texture::FT_linear_mipmap_nearest:
          _d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
          _d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_POINT);
          break;
      case Texture::FT_nearest_mipmap_linear:
          _d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
          _d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
          break;
      case Texture::FT_linear_mipmap_linear:
          _d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
          _d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
          break;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::apply_texture_immediate
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
apply_texture_immediate(DXTextureContext *tc) {

    // bugbug:  does this handle the case of untextured geometry? 
    //          we dont see this bug cause we never mix textured/untextured

	_d3dDevice->SetTexture(0, tc->_surface );
#ifdef _DEBUG
    if(tc!=NULL) {
       dxgsg_cat.spam() << "Setting active DX texture: " << tc->_tex->get_name() << "\n";
    } 
#endif

    _pCurTexContext = tc;   // enable_texturing needs this

  /*   FOR REFERENCE>>>  PixelBuffer::Type and PixelBuffer::Format
       use pb->get_format()  and pb->get_image_type()
  enum Type {
    T_unsigned_byte,
    T_unsigned_short,
    T_unsigned_byte_332,
    T_float,
  };
 
  enum Format {
    F_color_index,
    F_stencil_index,
    F_depth_component,
    F_red,
    F_green,
    F_blue,
    F_alpha,
    F_rgb,
    F_rgb5,    // specifically, 5 bits per R,G,B channel
    F_rgb8,    // 8 bits per R,G,B channel
    F_rgb12,   // 12 bits per R,G,B channel
    F_rgb332,  // 3 bits per R & G, 2 bits for B
    F_rgba,
    F_rgba4,   // 4 bits per R,G,B,A channel
    F_rgba8,   // 8 bits per R,G,B,A channel
    F_rgba12,  // 12 bits per R,G,B,A channel
    F_luminance,
    F_luminance_alpha,
  };

  */
}



////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_texture_wrap_mode
//       Access: Protected
//  Description: Maps from the Texture's internal wrap mode symbols to
//               GL's.
////////////////////////////////////////////////////////////////////
D3DTEXTUREADDRESS DXGraphicsStateGuardian::
get_texture_wrap_mode(Texture::WrapMode wm) {
  switch (wm) {
  case Texture::WM_clamp:
    return D3DTADDRESS_CLAMP;
  case Texture::WM_repeat:
    return D3DTADDRESS_WRAP;
  }
  dxgsg_cat.error() << "Invalid Texture::WrapMode value!\n";
  return D3DTADDRESS_WRAP;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_depth_func_type
//       Access: Protected
//  Description: Maps from the depth func modes to gl version 
////////////////////////////////////////////////////////////////////
D3DCMPFUNC DXGraphicsStateGuardian::
get_depth_func_type(DepthTestProperty::Mode m) const
{
    switch(m)
    {
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

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_stencil_func_type
//       Access: Protected
//  Description: Maps from the stencil func modes to dx version
////////////////////////////////////////////////////////////////////
D3DCMPFUNC DXGraphicsStateGuardian::
get_stencil_func_type(StencilProperty::Mode m) const
{
    switch(m) {
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
D3DSTENCILOP DXGraphicsStateGuardian::
get_stencil_action_type(StencilProperty::Action a) const
{
    switch(a) {
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


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_fog_mode_type
//       Access: Protected
//  Description: Maps from the fog types to gl version
////////////////////////////////////////////////////////////////////
D3DFOGMODE DXGraphicsStateGuardian::
get_fog_mode_type(Fog::Mode m) const {
  switch(m) {
    case Fog::M_linear: return D3DFOG_LINEAR;
    case Fog::M_exponential: return D3DFOG_EXP;
    case Fog::M_super_exponential: return D3DFOG_EXP2;
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

  if( _pTexPixFmts != NULL) {
      delete _pTexPixFmts;
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
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::restore_frame_buffer
//       Access: Public
//  Description: Restores the frame buffer that was previously saved.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
restore_frame_buffer(SavedFrameBuffer *frame_buffer) {
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



#ifdef WBD_GL_MODE


/* All of the following routines return GL-specific constants based
   on Panda defined constants.  For DX, just use 
   the Panda defined constants to decide what to do.

	get_image_type(PixelBuffer::Type type) {
	get_external_image_format(PixelBuffer::Format format) {
	get_internal_image_format(PixelBuffer::Format format) {
	get_texture_apply_mode_type( TextureApplyProperty::Mode am ) const
*/

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_image_type
//       Access: Protected
//  Description: Maps from the PixelBuffer's internal Type symbols
//               to GL's.
////////////////////////////////////////////////////////////////////
GLenum DXGraphicsStateGuardian::
get_image_type(PixelBuffer::Type type) {
  switch (type) {
  case PixelBuffer::T_unsigned_byte:
    return GL_UNSIGNED_BYTE;
  case PixelBuffer::T_unsigned_short:
    return GL_UNSIGNED_SHORT;
#ifdef GL_UNSIGNED_BYTE_3_3_2_EXT
  case PixelBuffer::T_unsigned_byte_332:
    return GL_UNSIGNED_BYTE_3_3_2_EXT;
#endif
  case PixelBuffer::T_float:
    return GL_FLOAT;
  }
  dxgsg_cat.error() << "Invalid PixelBuffer::Type value!\n";
  return GL_UNSIGNED_BYTE;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_external_image_format
//       Access: Protected
//  Description: Maps from the PixelBuffer's Format symbols
//               to GL's.
////////////////////////////////////////////////////////////////////
GLenum DXGraphicsStateGuardian::
get_external_image_format(PixelBuffer::Format format) {
  switch (format) {
  case PixelBuffer::F_color_index:
    return GL_COLOR_INDEX;
  case PixelBuffer::F_stencil_index:
    return GL_STENCIL_INDEX;
  case PixelBuffer::F_depth_component:
    return GL_DEPTH_COMPONENT;
  case PixelBuffer::F_red:
    return GL_RED;
  case PixelBuffer::F_green:
    return GL_GREEN;
  case PixelBuffer::F_blue:
    return GL_BLUE;
  case PixelBuffer::F_alpha:
    return GL_ALPHA;
  case PixelBuffer::F_rgb:
  case PixelBuffer::F_rgb5:
  case PixelBuffer::F_rgb8:
  case PixelBuffer::F_rgb12:
  case PixelBuffer::F_rgb332:
    return GL_RGB;
  case PixelBuffer::F_rgba:
  case PixelBuffer::F_rgba4:
  case PixelBuffer::F_rgba8:
  case PixelBuffer::F_rgba12:
    return GL_RGBA;
  case PixelBuffer::F_luminance:
    return GL_LUMINANCE;
  case PixelBuffer::F_luminance_alpha:
    return GL_LUMINANCE_ALPHA;
  }
  dxgsg_cat.error()
    << "Invalid PixelBuffer::Format value in get_external_image_format(): "
    << (int)format << "\n";
  return GL_RGB;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_internal_image_format
//       Access: Protected
//  Description: Maps from the PixelBuffer's Format symbols to a
//               suitable internal format for GL textures.
////////////////////////////////////////////////////////////////////
GLenum DXGraphicsStateGuardian::
get_internal_image_format(PixelBuffer::Format format) {
  switch (format) {
  case PixelBuffer::F_rgba:
    return GL_RGBA;
  case PixelBuffer::F_rgba4:
    return GL_RGBA4;
  case PixelBuffer::F_rgba8:
    return GL_RGBA8;
  case PixelBuffer::F_rgba12:
    return GL_RGBA12;

  case PixelBuffer::F_rgb:
    return GL_RGB;
  case PixelBuffer::F_rgb5:
  case PixelBuffer::F_rgb8:
    return GL_RGB8;
  case PixelBuffer::F_rgb12:
    return GL_RGB12;
  case PixelBuffer::F_rgb332:
    return GL_R3_G3_B2;

  case PixelBuffer::F_luminance_alpha:
    return GL_LUMINANCE_ALPHA;

  case PixelBuffer::F_alpha:
    return GL_ALPHA;

  case PixelBuffer::F_red:
  case PixelBuffer::F_green:
  case PixelBuffer::F_blue:
  case PixelBuffer::F_luminance:
    return GL_LUMINANCE;
  }

  dxgsg_cat.error()
    << "Invalid image format in get_internal_image_format(): "
    << (int)format << "\n";
  return GL_RGB;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian::get_texture_apply_mode_type
//       Access: Protected
//  Description: Maps from the texture environment's mode types
//		 to the corresponding OpenGL ids
////////////////////////////////////////////////////////////////////
GLint DXGraphicsStateGuardian::
get_texture_apply_mode_type( TextureApplyProperty::Mode am ) const
{
    switch( am )
    {
	case TextureApplyProperty::M_modulate: return GL_MODULATE;
	case TextureApplyProperty::M_decal: return GL_DECAL;
 	case TextureApplyProperty::M_blend: return GL_BLEND;
	case TextureApplyProperty::M_replace: return GL_REPLACE;
	case TextureApplyProperty::M_add: return GL_ADD;
    }
    dxgsg_cat.error()
      << "Invalid TextureApplyProperty::Mode value" << endl;
    return GL_MODULATE;
}
#endif	// WBD_GL_MODE


////////////////////////////////////////////////////////////////////
//     Function: dx_cleanup
//  Description: Clean up the DirectX environment.  
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
dx_cleanup() {

	release_all_textures();

	// Release the DDraw and D3D objects used by the app
    RELEASE(_zbuf);
    RELEASE(_back);
    RELEASE(_pri);

    // Do a safe check for releasing the D3DDEVICE. RefCount should be zero.
    if( _d3dDevice!=NULL ) {
        if( 0 < _d3dDevice->Release() ) {
			  dxgsg_cat.error() << "DXGraphicsStateGuardian::destructor - d3dDevice reference count > 0" << endl;
        } 
        _d3dDevice = NULL;	// clear the pointer in the Gsg
    }

    RELEASE(_d3d);

	// Do a safe check for releasing DDRAW. RefCount should be zero.
    if( _pDD!=NULL ) {
		int val;
        if( 0 < (val = _pDD->Release()) ) {
			  dxgsg_cat.error()
		      << "DXGraphicsStateGuardian::destructor - context reference count = " << val << endl;
        }
        _pDD  = NULL;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: dx_setup_after_resize
//  Description: Recreate the back buffer and zbuffers at the new size
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::
dx_setup_after_resize(RECT viewrect, HWND mwindow) 
{
    if(_back == NULL) // nothing created yet
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

     if(FAILED(hr = _pDD->CreateSurface( &ddsd, &_pri, NULL ))) {
       dxgsg_cat.fatal() << "DXGraphicsStateGuardian::resize() - CreateSurface failed for primary : result = " << ConvD3DErrorToString(hr) << endl;
       exit(1);
     }

     // Create a clipper object which handles all our clipping for cases when
     // our window is partially obscured by other windows. 
     LPDIRECTDRAWCLIPPER Clipper;

     if(FAILED(hr = _pDD->CreateClipper( 0, &Clipper, NULL ))) {
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

     ddsd_back.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;  // just to make sure
     ddsd_back.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    
     PRINTVIDMEM(_pDD,&ddsd_back.ddsCaps,"resize backbuffer surf");
    
     if(FAILED(hr = _pDD->CreateSurface( &ddsd_back, &_back, NULL ))) {
    	  dxgsg_cat.fatal() << "DXGraphicsStateGuardian::resize() - CreateSurface failed for backbuffer : result = " << ConvD3DErrorToString(hr) << endl;
    	  exit(1);
     }
    
     PRINTVIDMEM(_pDD,&ddsd_back.ddsCaps,"resize zbuffer surf");
    
     // Recreate and attach a z-buffer. 
     if(FAILED(hr = _pDD->CreateSurface( &ddsd_zbuf, &_zbuf, NULL ))) {
    	  dxgsg_cat.fatal() << "DXGraphicsStateGuardian::resize() - CreateSurface failed for Z buffer: result = " << ConvD3DErrorToString(hr) << endl;
    	  exit(1);
     }

	// Attach the z-buffer to the back buffer.
    if( (hr = _back->AddAttachedSurface( _zbuf ) ) != DD_OK) {
	  dxgsg_cat.fatal()
      << "DXGraphicsStateGuardian::resize() - AddAttachedSurface failed : result = " << ConvD3DErrorToString(hr) << endl;
	  exit(1);
    }

    if( (hr = _d3dDevice->SetRenderTarget(_back,0x0) ) != DD_OK) {
	  dxgsg_cat.fatal()
      << "DXGraphicsStateGuardian::resize() - SetRenderTarget failed : result = " << ConvD3DErrorToString(hr) << endl;
	  exit(1);
    }

    // Create the viewport
	D3DVIEWPORT7 vp = { 0, 0, renderWid, renderHt, 0.0f, 1.0f };
    hr = _d3dDevice->SetViewport( &vp );
    if (hr != DD_OK) {
	  dxgsg_cat.fatal()
      << "DXGraphicsStateGuardian::config() - SetViewport failed : result = " << ConvD3DErrorToString(hr) << endl;
	  exit(1);
    }
}


////////////////////////////////////////////////////////////////////
//     Function: show_frame 
//       Access:
//       Description:   Repaint primary buffer from back buffer  (windowed mode only)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::show_frame(void) {

    PStatTimer timer(_win->_swap_pcollector);  // this times just the flip, so it must go here in dxgsg, instead of wdxdisplay, which would time the whole frame

	if (_pri!=NULL) {	
	    if( dx_full_screen ) {

            HRESULT hr = _pri->Flip( NULL, DDFLIP_WAIT );  // bugbug:  dont we want triple buffering instead of wasting time waiting for vsync?
            if (hr == DDERR_SURFACELOST) {
               // Check/restore the primary surface
               if(( _pri!=NULL ) && _pri->IsLost())
                     _pri->Restore();
               // Check/restore the back buffer
               if(( _back!=NULL ) &&  _back->IsLost())
                    _back->Restore();
               // Check/restore the z-buffer
               if(( _zbuf!=NULL ) &&  _zbuf->IsLost())
                    _zbuf->Restore();
               return;
            }

			if (hr != DD_OK) {
			      dxgsg_cat.error() << "DXGraphicsStateGuardian::show_frame() - Flip failed : " << ConvD3DErrorToString(hr) << endl;
                  exit(1);
            }
		
        } else {
		    HRESULT hr = _pri->Blt( &_view_rect, _back,  NULL, DDBLT_WAIT, NULL );
            if(hr!=DD_OK) {
    			if (hr == DDERR_SURFACELOST) {
                  // Check/restore the primary surface
                   if(( _pri!=NULL ) && _pri->IsLost())
                         _pri->Restore();
                   // Check/restore the back buffer
                   if(( _back!=NULL ) &&  _back->IsLost())
                        _back->Restore();
                   // Check/restore the z-buffer
                   if(( _zbuf!=NULL ) &&  _zbuf->IsLost())
                        _zbuf->Restore();
                   return;
                } else {
    			      dxgsg_cat.error() << "DXGraphicsStateGuardian::show_frame() - Blt failed : " << ConvD3DErrorToString(hr) << endl;
                      exit(1);
                }
            }
		}
    }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_window_move
//       Access:
//  Description: we receive the new x and y position of the client
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian::adjust_view_rect(int x, int y)
{
  if (_view_rect.left != x || _view_rect.top != y)
	{
	_view_rect.right = x + _view_rect.right - _view_rect.left;
	_view_rect.left = x;
	_view_rect.bottom = y + _view_rect.bottom - _view_rect.top;
	_view_rect.top = y;

//	set_clipper(clip_rect);
	}
}


