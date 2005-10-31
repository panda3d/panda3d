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
#include "graphicsWindow.h"
#include "graphicsEngine.h"
#include "lens.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"
#include "textureAttrib.h"
#include "texGenAttrib.h"
#include "shadeModelAttrib.h"
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
#include "geomVertexFormat.h"
#include "geomVertexData.h"
#include "geomTriangles.h"
#include "geomTristrips.h"
#include "geomTrifans.h"
#include "geomLines.h"
#include "geomLinestrips.h"
#include "geomPoints.h"
#include "GeomVertexReader.h"
#include "dxGeomMunger7.h"
#include "config_gobj.h"
#include "pStatTimer.h"
#include "pStatCollector.h"

#include <d3d.h>
#include <mmsystem.h>

TypeHandle DXGraphicsStateGuardian7::_type_handle;

D3DMATRIX DXGraphicsStateGuardian7::_d3d_ident_mat;

#define MY_D3DRGBA(r, g, b, a) ((D3DCOLOR) D3DCOLOR_COLORVALUE(r, g, b, a))

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian7::
DXGraphicsStateGuardian7(const FrameBufferProperties &properties) :
  GraphicsStateGuardian(properties, CS_yup_left)
{
  _screen = NULL;
  _d3d_device = NULL;

  _dx_is_ready = false;
  _vertex_blending_enabled = false;
  _tex_stats_retrieval_impossible = false;

  _active_vbuffer = NULL;
  _active_ibuffer = NULL;

  // This is a static member, but we initialize it here in the
  // constructor anyway.  It won't hurt if it gets repeatedly
  // initalized.
  ZeroMemory(&_d3d_ident_mat, sizeof(D3DMATRIX));
  _d3d_ident_mat._11 = _d3d_ident_mat._22 = _d3d_ident_mat._33 = _d3d_ident_mat._44 = 1.0f;

  _cur_read_pixel_buffer = RenderBuffer::T_front;
  set_color_clear_value(_color_clear_value);

  // DirectX drivers seem to consistently invert the texture when
  // they copy framebuffer-to-texture.  Ok.
  _copy_texture_inverted = true;

  _supported_geom_rendering = 
    Geom::GR_point | Geom::GR_indexed_other | Geom::GR_flat_first_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian7::
~DXGraphicsStateGuardian7() {
  if (IS_VALID_PTR(_d3d_device)) {
    _d3d_device->SetTexture(0, NULL);  // this frees reference to the old texture
  }
  free_nondx_resources();
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
//
//               This function should not be called directly to
//               prepare a texture.  Instead, call Texture::prepare().
////////////////////////////////////////////////////////////////////
TextureContext *DXGraphicsStateGuardian7::
prepare_texture(Texture *tex) {
  DXTextureContext7 *dtc = new DXTextureContext7(tex);
  if (dtc->create_texture(_d3d_device, _num_tex_formats, _tex_formats,
			  &_screen->D3DDevDesc) == NULL) {
    delete dtc;
    return NULL;
  }

  return dtc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::apply_texture
//       Access: Public
//  Description: Makes the texture the currently available texture for
//               rendering on the ith stage.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
apply_texture(int i, TextureContext *tc) {
  if (tc == (TextureContext *)NULL) {
    // The texture wasn't bound properly or something, so ensure
    // texturing is disabled and just return.
    _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
    return;
  }

#ifdef DO_PSTATS
  add_to_texture_record(tc);
#endif

  DXTextureContext7 *dtc = DCAST(DXTextureContext7, tc);

  int dirty = dtc->get_dirty_flags();

  // If the texture image has changed, or if its use of mipmaps has
  // changed, we need to re-create the image.  Ignore other types of
  // changes, which aren't significant for DX.

  if ((dirty & (Texture::DF_image | Texture::DF_mipmap)) != 0) {
    // If this is *only* because of a mipmap change, issue a
    // warning--it is likely that this change is the result of an
    // error or oversight.
    if ((dirty & Texture::DF_image) == 0) {
      dxgsg7_cat.warning()
	<< "Texture " << *dtc->_texture << " has changed mipmap state.\n";
    }
    
    if (!dtc->create_texture(_d3d_device, _num_tex_formats, _tex_formats,
			     &_screen->D3DDevDesc)) {
      // Oops, we can't re-create the texture for some reason.
      dxgsg7_cat.error()
	<< "Unable to re-create texture " << *dtc->_texture << endl;
      _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
      return;
    }
  }

  Texture *tex = tc->_texture;
  Texture::WrapMode wrap_u, wrap_v, wrap_w;
  wrap_u = tex->get_wrap_u();
  wrap_v = tex->get_wrap_v();
  wrap_w = tex->get_wrap_w();

  _d3d_device->SetTextureStageState(i, D3DTSS_ADDRESSU, get_texture_wrap_mode(wrap_u));
  _d3d_device->SetTextureStageState(i, D3DTSS_ADDRESSV, get_texture_wrap_mode(wrap_v));

  _d3d_device->SetTextureStageState(i, D3DTSS_BORDERCOLOR, 
				    Colorf_to_D3DCOLOR(tex->get_border_color()));

  uint aniso_degree = tex->get_anisotropic_degree();
  Texture::FilterType ft = tex->get_magfilter();

  _d3d_device->SetTextureStageState(i, D3DTSS_MAXANISOTROPY, aniso_degree);

  D3DTEXTUREMAGFILTER new_mag_filter;
  new_mag_filter = ((ft != Texture::FT_nearest) ? D3DTFG_LINEAR : D3DTFG_POINT);

  _d3d_device->SetTextureStageState(i, D3DTSS_MAGFILTER, new_mag_filter);

  // map Panda composite min+mip filter types to d3d's separate min & mip filter types
  D3DTEXTUREMINFILTER new_min_filter = get_d3d_min_type(tex->get_minfilter());
  D3DTEXTUREMIPFILTER new_mip_filter = get_d3d_mip_type(tex->get_minfilter());

  if (!tex->might_have_ram_image()) {
    // If the texture is completely dynamic, don't try to issue
    // mipmaps--pandadx doesn't support auto-generated mipmaps at this
    // point.
    new_mip_filter = D3DTFP_NONE;
  }

#ifndef NDEBUG
  // sanity check
  if ((!dtc->has_mipmaps()) && (new_mip_filter != D3DTFP_NONE)) {
    dxgsg7_cat.error()
      << "Trying to set mipmap filtering for texture with no generated mipmaps!! texname["
      << tex->get_name() << "], filter("
      << tex->get_minfilter() << ")\n";
    new_mip_filter = D3DTFP_NONE;
  }
#endif

  _d3d_device->SetTextureStageState(i, D3DTSS_MINFILTER, new_min_filter);
  _d3d_device->SetTextureStageState(i, D3DTSS_MIPFILTER, new_mip_filter);

  _d3d_device->SetTexture(i, dtc->_surface);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
release_texture(TextureContext *tc) {
  DXTextureContext7 *dtc = DCAST(DXTextureContext7, tc);
  delete dtc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::make_geom_munger
//       Access: Public, Virtual
//  Description: Creates a new GeomMunger object to munge vertices
//               appropriate to this GSG for the indicated state.
////////////////////////////////////////////////////////////////////
PT(GeomMunger) DXGraphicsStateGuardian7::
make_geom_munger(const RenderState *state) {
  PT(DXGeomMunger7) munger = new DXGeomMunger7(this, state);
  return GeomMunger::register_munger(munger);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_color_clear_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_color_clear_value(const Colorf& value) {
  _color_clear_value = value;
  _d3dcolor_clear_value =  Colorf_to_D3DCOLOR(value);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_clear(const RenderBuffer &buffer) {
  nassertv(buffer._gsg == this);
  int buffer_type = buffer._buffer_type;
  
  int flags = 0;
  
  if (buffer_type & RenderBuffer::T_depth) {
    flags |=  D3DCLEAR_ZBUFFER;
    nassertv(_screen->pddsZBuf!=NULL);
  }
  if (buffer_type & RenderBuffer::T_back) {
    flags |=  D3DCLEAR_TARGET;
  }
  
  HRESULT  hr = _d3d_device->Clear(0, NULL, flags, _d3dcolor_clear_value,
				   (D3DVALUE) _depth_clear_value, 0);
  if (hr != DD_OK) {
    dxgsg7_cat.error()
      << "clear_buffer failed:  Clear returned " << ConvD3DErrorToString(hr)
      << endl;
    throw_event("panda3d-render-error");
  }
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
    HRESULT hr = _screen->pD3DDevice->SetViewport(&vp);
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
  const LMatrix4f &lens_mat = _current_lens->get_projection_mat();

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

  _projection_mat = convert_mat * lens_mat * rescale_mat;

  if (_scene_setup->get_inverted()) {
    // If the scene is supposed to be inverted, then invert the
    // projection matrix.
    static LMatrix4f invert_mat = LMatrix4f::scale_mat(1.0f, -1.0f, 1.0f);
    _projection_mat *= invert_mat;
  }

  HRESULT hr = 
    _d3d_device->SetTransform(D3DTRANSFORMSTATE_PROJECTION,
                              (D3DMATRIX*)_projection_mat.get_data());
  return SUCCEEDED(hr);
}

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

  HRESULT hr = _screen->pD3DDevice->BeginScene();

  if (FAILED(hr)) {
    if ((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      if (dxgsg7_cat.is_debug()) {
        dxgsg7_cat.debug()
          << "BeginScene returns " << ConvD3DErrorToString(hr) << endl;
      }
      
      check_cooperative_level();

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
  HRESULT hr = _screen->pD3DDevice->EndScene();

  if (FAILED(hr)) {
    if ((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      if (dxgsg7_cat.is_debug()) {
        dxgsg7_cat.debug()
          << "EndScene returns " << ConvD3DErrorToString(hr) << endl;
      }

      check_cooperative_level();

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
//     Function: GraphicsStateGuardian::end_frame
//       Access: Public, Virtual
//  Description: Called after each frame is rendered, to allow the
//               GSG a chance to do any internal cleanup after
//               rendering the frame, and before the window flips.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
end_frame() {

#if defined(DO_PSTATS)
  if (_texmgrmem_total_pcollector.is_active()) {
#define TICKS_PER_GETTEXINFO (2.5*1000)   // 2.5 second interval
    static DWORD last_tick_count = 0;
    DWORD cur_tick_count = GetTickCount();

    if (cur_tick_count - last_tick_count > TICKS_PER_GETTEXINFO) {
      last_tick_count = cur_tick_count;
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
//     Function: DXGraphicsStateGuardian7::begin_draw_primitives
//       Access: Public, Virtual
//  Description: Called before a sequence of draw_primitive()
//               functions are called, this should prepare the vertex
//               data for rendering.  It returns true if the vertices
//               are ok, false to abort this group of primitives.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian7::
begin_draw_primitives(const Geom *geom, const GeomMunger *munger,
                      const GeomVertexData *vertex_data) {
  if (!GraphicsStateGuardian::begin_draw_primitives(geom, munger, vertex_data)) {
    return false;
  }
  nassertr(_vertex_data != (GeomVertexData *)NULL, false);
  determine_fvf();

  if (_transform_stale && !_vertex_data->is_vertex_transformed()) {
    D3DMATRIX *d3d_mat = (D3DMATRIX *)(const D3DMATRIX *)_internal_transform->get_mat().get_data();
    _screen->pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, d3d_mat);
    _transform_stale = false;
  }

  if (_vertex_data->is_vertex_transformed()) {
    // If the vertex data claims to be already transformed into clip
    // coordinates, wipe out the current projection and modelview
    // matrix (so we don't attempt to transform it again).

    // It's tempting just to use the D3DFVF_XYZRHW specification on
    // these vertices, but that turns out to be a bigger hammer than
    // we want: that also prevents lighting calculations and user clip
    // planes.
    _screen->pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &_d3d_ident_mat);
    static const LMatrix4f rescale_mat
      (1, 0, 0, 0,
       0, 1, 0, 0,
       0, 0, 0.5, 0,
       0, 0, 0.5, 1);
    _transform_stale = true;

    _screen->pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)rescale_mat.get_data());
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::determine_fvf
//       Access: Public
//  Description: Computes the FVF code for the current vertex data.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
determine_fvf() {
  // The munger should have put the FVF data in the first array.
  const GeomVertexArrayData *data = _vertex_data->get_array(0);

  const GeomVertexArrayFormat *array_format = data->get_array_format();

  // We have to start with the vertex data, and work up from there in
  // order, since that's the way the FVF is defined.
  int n = 0;
  int num_columns = array_format->get_num_columns();

  _fvf = 0;
  
  if (n < num_columns && 
      array_format->get_column(n)->get_name() == InternalName::get_vertex()) {
    ++n;
  }

  if (n < num_columns && 
      array_format->get_column(n)->get_name() == InternalName::get_normal()) {
    _fvf |= D3DFVF_NORMAL;
    ++n;
  }
  if (n < num_columns && 
      array_format->get_column(n)->get_name() == InternalName::get_color()) {
    _fvf |= D3DFVF_DIFFUSE;
    ++n;
  }

  // Now look for all of the texcoord names and enable them in the
  // same order they appear in the array.
  int texcoord_index = 0;
  while (n < num_columns && 
         array_format->get_column(n)->get_contents() == Geom::C_texcoord) {
    const GeomVertexColumn *column = array_format->get_column(n);
    switch (column->get_num_values()) {
    case 1:
      _fvf |= D3DFVF_TEXCOORDSIZE1(texcoord_index);
      ++n;
      break;
    case 2:
      _fvf |= D3DFVF_TEXCOORDSIZE2(texcoord_index);
      ++n;
      break;
    case 3:
      _fvf |= D3DFVF_TEXCOORDSIZE3(texcoord_index);
      ++n;
      break;
    case 4:
      _fvf |= D3DFVF_TEXCOORDSIZE4(texcoord_index);
      ++n;
      break;
    }
    ++texcoord_index;
  }

  switch (texcoord_index) {
  case 0:
    break;
  case 1:
    _fvf |= D3DFVF_TEX1;
    break;
  case 2:
    _fvf |= D3DFVF_TEX2;
    break;
  case 3:
    _fvf |= D3DFVF_TEX3;
    break;
  case 4:
    _fvf |= D3DFVF_TEX4;
    break;
  case 5:
    _fvf |= D3DFVF_TEX5;
    break;
  case 6:
    _fvf |= D3DFVF_TEX6;
    break;
  case 7:
    _fvf |= D3DFVF_TEX7;
    break;
  case 8:
    _fvf |= D3DFVF_TEX8;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_triangles
//       Access: Public, Virtual
//  Description: Draws a series of disconnected triangles.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_triangles(const GeomTriangles *primitive) {
  _vertices_tri_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_tri_pcollector.add_level(1);

  if (primitive->is_indexed()) {
    // Indexed, client arrays.
    _d3d_device->DrawIndexedPrimitive
      (D3DPT_TRIANGLELIST, _fvf,
       (LPVOID)_vertex_data->get_array(0)->get_data().p(),
       primitive->get_max_vertex() + 1,
       (LPWORD)primitive->get_data().p(),
       primitive->get_num_vertices(),
       NULL);

  } else {
    // Nonindexed, client arrays.
    int stride = _vertex_data->get_format()->get_array(0)->get_stride();
    unsigned int first_vertex = primitive->get_first_vertex();

    // Interestingly, my ATI driver seems to fail to draw anything in
    // this call if the address range of the buffer supplied crosses
    // over a multiple of 0x10000.  I refuse to hack around this lame
    // driver bug.
    _screen->pD3DDevice->DrawPrimitive
      (D3DPT_TRIANGLELIST, _fvf,
       (LPVOID)(_vertex_data->get_array(0)->get_data() + stride * first_vertex),
       primitive->get_num_vertices(), NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_lines
//       Access: Public, Virtual
//  Description: Draws a series of disconnected line segments.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_lines(const GeomLines *primitive) {
  _vertices_other_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_other_pcollector.add_level(1);

  if (primitive->is_indexed()) {
    // Indexed, client arrays.
    _d3d_device->DrawIndexedPrimitive
      (D3DPT_LINELIST, _fvf,
       (LPVOID)_vertex_data->get_array(0)->get_data().p(),
       primitive->get_max_vertex() + 1,
       (LPWORD)primitive->get_data().p(),
       primitive->get_num_vertices(),
       NULL);

  } else {
    // Nonindexed, client arrays.
    int stride = _vertex_data->get_format()->get_array(0)->get_stride();
    unsigned int first_vertex = primitive->get_first_vertex();
    _screen->pD3DDevice->DrawPrimitive
      (D3DPT_LINELIST, _fvf,
       (LPVOID)(_vertex_data->get_array(0)->get_data() + stride * first_vertex),
       primitive->get_num_vertices(), NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::draw_points
//       Access: Public, Virtual
//  Description: Draws a series of disconnected points.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
draw_points(const GeomPoints *primitive) {
  _vertices_other_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_other_pcollector.add_level(1);

  // The munger should have protected us from indexed points--DirectX
  // doesn't support them.
  nassertv(!primitive->is_indexed());

  // Nonindexed, client arrays.
  int stride = _vertex_data->get_format()->get_array(0)->get_stride();
  unsigned int first_vertex = primitive->get_first_vertex();
  _screen->pD3DDevice->DrawPrimitive
    (D3DPT_POINTLIST, _fvf,
     (LPVOID)(_vertex_data->get_array(0)->get_data() + stride * first_vertex),
     primitive->get_num_vertices(), NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::end_draw_primitives()
//       Access: Public, Virtual
//  Description: Called after a sequence of draw_primitive()
//               functions are called, this should do whatever cleanup
//               is appropriate.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
end_draw_primitives() {
  if (_vertex_data->is_vertex_transformed()) {
    // Restore the projection matrix that we wiped out above.
    prepare_lens();
  }

  GraphicsStateGuardian::end_draw_primitives();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::framebuffer_copy_to_texture
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display
//               region from the framebuffer into texture memory.
//
//               If z > -1, it is the cube map index into which to
//               copy.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
framebuffer_copy_to_texture(Texture *tex, int z, const DisplayRegion *dr,
                            const RenderBuffer &rb) {
  // For a DX7, go the slow route through RAM, just 'cause no one
  // really cares about DX7 anyway.
  framebuffer_copy_to_ram(tex, z, dr, rb);
}


////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian9::framebuffer_copy_to_ram
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display region
//               from the framebuffer into system memory, not texture
//               memory.  Returns true on success, false on failure.
//
//               This completely redefines the ram image of the
//               indicated texture.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian7::
framebuffer_copy_to_ram(Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb) {
  set_read_buffer(rb);

  nassertr(tex != NULL && dr != NULL, false);

  int xo, yo, w, h;
  dr->get_region_pixels_i(xo, yo, w, h);

  Texture::Format format = tex->get_format();
  Texture::ComponentType component_type = tex->get_component_type();

  switch (format) {
  case Texture::F_depth_component:
  case Texture::F_stencil_index:
    // Sorry, not (yet?) supported in pandadx.
    return false;

  default:
    format = Texture::F_rgb;
    component_type = Texture::T_unsigned_byte;
  }

  Texture::TextureType texture_type = Texture::TT_2d_texture;

  if (tex->get_x_size() != w || tex->get_y_size() != h ||
      tex->get_component_type() != component_type ||
      tex->get_format() != format ||
      tex->get_texture_type() != texture_type) {
    // Re-setup the texture; its properties have changed.
    tex->setup_texture(texture_type, w, h, tex->get_z_size(), 
                       component_type, format);
  }

  extern HRESULT ConvertDDSurftoPixBuf(Texture *pixbuf,LPDIRECTDRAWSURFACE7 pDDSurf);
  ConvertDDSurftoPixBuf(tex,((_cur_read_pixel_buffer & RenderBuffer::T_back) ? _screen->pddsBack : _screen->pddsPrimary));
  
  nassertr(tex->has_ram_image(), false);
  return true;
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

  _auto_rescale_normal = false;

  // overwrite gsg defaults with these values

  // Lamely assume we have both a color and a depth buffer.
  _buffer_mask = RenderBuffer::T_color | RenderBuffer::T_depth;

  HRESULT hr;

  // make sure gsg passes all current state down to us
  // set_state_and_transform(RenderState::make_empty(),
  // TransformState::make_identity());
  // want gsg to pass all state settings down so any non-matching defaults we set here get overwritten

  assert(_screen->pDD!=NULL);
  assert(_screen->pD3D!=NULL);
  assert(_screen->pD3DDevice!=NULL);
  assert(_screen->pddsPrimary!=NULL);
  assert(_screen->pddsBack!=NULL);

  _screen->pD3DDevice->SetRenderState( D3DRENDERSTATE_AMBIENT, 0x0);

  _clip_plane_bits = 0;
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE , 0x0);
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_EDGEANTIALIAS, false);
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, false);
  
  assert(_screen->pddsBack!=NULL);  // dxgsg7 is always double-buffered right now

  _last_testcooplevel_result = S_OK;

  DX_DECLARE_CLEAN(DDCAPS,ddCaps);
  if (FAILED(hr = _screen->pDD->GetCaps(&ddCaps,NULL))) {
    dxgsg7_cat.fatal() << "GetCaps failed on DDraw! hr = " << ConvD3DErrorToString(hr) << "\n";
    exit(1);
  }

  // s3 virge drivers sometimes give crap values for these
  if(_screen->D3DDevDesc.dwMaxTextureWidth==0)
    _screen->D3DDevDesc.dwMaxTextureWidth=256;
  
  if(_screen->D3DDevDesc.dwMaxTextureHeight==0)
    _screen->D3DDevDesc.dwMaxTextureHeight=256;

#define REQUIRED_BLENDCAPS (D3DPBLENDCAPS_ZERO|D3DPBLENDCAPS_ONE| \
                            D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_INVSRCALPHA)

  if (((_screen->D3DDevDesc.dpcTriCaps.dwSrcBlendCaps & REQUIRED_BLENDCAPS)!=REQUIRED_BLENDCAPS) ||
      ((_screen->D3DDevDesc.dpcTriCaps.dwDestBlendCaps & REQUIRED_BLENDCAPS)!=REQUIRED_BLENDCAPS)) {
    dxgsg7_cat.error() << "device is missing alpha blending capabilities, blending may not work correctly: SrcBlendCaps: 0x"<< (void*) _screen->D3DDevDesc.dpcTriCaps.dwSrcBlendCaps << "  DestBlendCaps: "<< (void*) _screen->D3DDevDesc.dpcTriCaps.dwDestBlendCaps << endl;
  }

  if (!(_screen->D3DDevDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY)) {
    dxgsg7_cat.error() << "device is missing texture transparency capability, transparency may not work correctly!  TextureCaps: 0x"<< (void*) _screen->D3DDevDesc.dpcTriCaps.dwTextureCaps << endl;
  }
  
  // just require trilinear.  if it can do that, it can probably do all the lesser point-sampling variations too
#define REQUIRED_TEXFILTERCAPS (D3DPTFILTERCAPS_MAGFLINEAR |  D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_LINEAR)
  if ((_screen->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps & REQUIRED_TEXFILTERCAPS)!=REQUIRED_TEXFILTERCAPS) {
    dxgsg7_cat.error() << "device is missing texture bilinear filtering capability, textures may appear blocky!  TextureFilterCaps: 0x"<< (void*) _screen->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
  }
#define REQUIRED_MIPMAP_TEXFILTERCAPS (D3DPTFILTERCAPS_MIPFLINEAR | D3DPTFILTERCAPS_LINEARMIPLINEAR)
  
  if (!(ddCaps.ddsCaps.dwCaps & DDSCAPS_MIPMAP)) {
    dxgsg7_cat.debug() << "device does not have mipmap texturing filtering capability!   TextureFilterCaps: 0x"<< (void*) _screen->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
    dx_ignore_mipmaps = TRUE;
  } else if ((_screen->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps & REQUIRED_MIPMAP_TEXFILTERCAPS)!=REQUIRED_MIPMAP_TEXFILTERCAPS) {
    dxgsg7_cat.debug() << "device is missing tri-linear mipmap filtering capability, texture mipmaps may not supported! TextureFilterCaps: 0x"<< (void*) _screen->D3DDevDesc.dpcTriCaps.dwTextureFilterCaps << endl;
  }
  
#define REQUIRED_TEXBLENDCAPS (D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2)
  if ((_screen->D3DDevDesc.dwTextureOpCaps & REQUIRED_TEXBLENDCAPS)!=REQUIRED_TEXBLENDCAPS) {
    dxgsg7_cat.error() << "device is missing some required texture blending capabilities, texture blending may not work properly! TextureOpCaps: 0x"<< (void*) _screen->D3DDevDesc.dwTextureOpCaps << endl;
  }
  
  if(_screen->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE) {
    // watch out for drivers that emulate per-pixel fog with per-vertex fog (Riva128, Matrox Millen G200)
    // some of these require gouraud-shading to be set to work, as if you were using vertex fog
    _do_fog_type=PerPixelFog;
  } else {
    // every card is going to have vertex fog, since it's implemented in d3d runtime
    assert((_screen->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX )!=0);
    
    // vtx fog may look crappy if you have large polygons in the foreground and they get clipped,
    // so you may want to disable it
    
    if(dx_no_vertex_fog) {
      _do_fog_type = None;
    } else {
      _do_fog_type = PerVertexFog;
      
      // range-based fog only works with vertex fog in dx7/8
      if(dx_use_rangebased_fog && (_screen->D3DDevDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGRANGE))
	_screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_RANGEFOGENABLE, true);
    }
  }

  SetRect(&_screen->clip_rect, 0,0,0,0);  // no clip rect set

  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, false);
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPING,true);
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, false);
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
  
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
  
  // initial clip rect
  SetRect(&_screen->clip_rect, 0,0,0,0);     // no clip rect set
  
  _screen->pD3DDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_DISABLE);  // disables texturing
  
  // this code must match apply_texture() code for states above
  // so DX TSS renderstate matches dxgsg7 state
  
  _screen->pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
  _screen->pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_POINT);
  _screen->pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);
  _screen->pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);

  _max_vertices_per_array = 65536;
  _max_vertices_per_primitive = 1024;
  _max_texture_stages = 1;
  _max_texture_dimension = 
    min(_screen->D3DDevDesc.dwMaxTextureWidth,
	_screen->D3DDevDesc.dwMaxTextureHeight);

  _supports_texture_combine = false;
  _supports_texture_saved_result = false;
  _supports_texture_dot3 = false;
  _supports_3d_texture = false;
  _supports_cube_map = false;

  _max_lights = 8;
  _max_clip_planes = 8;
  _max_vertex_transforms = 0;
  _max_vertex_transform_indices = 0;

  _projection_mat = LMatrix4f::ident_mat();
  _has_scene_graph_color = false;

  _last_testcooplevel_result = D3D_OK;


  _tex_formats = new DDPIXELFORMAT[MAX_DX_TEXPIXFMTS];
  _num_tex_formats = 0;

  _d3d_device->EnumTextureFormats(enum_tex_formats_callback, this);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::apply_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
apply_fog(Fog *fog) {
  if (_do_fog_type == None)
    return;

  Fog::Mode panda_fogmode = fog->get_mode();
  D3DFOGMODE d3dfogmode = get_fog_mode_type(panda_fogmode);

  _d3d_device->SetRenderState((D3DRENDERSTATETYPE)_do_fog_type, d3dfogmode);

  const Colorf &fog_colr = fog->get_color();
  _d3d_device->SetRenderState(D3DRENDERSTATE_FOGCOLOR,
                              D3DRGBA(fog_colr[0], fog_colr[1], fog_colr[2], 0.0f));  // Alpha bits are not used

  // do we need to adjust fog start/end values based on D3DPRASTERCAPS_WFOG/D3DPRASTERCAPS_ZFOG ?
  // if not WFOG, then docs say we need to adjust values to range [0, 1]

  switch (panda_fogmode) {
  case Fog::M_linear:
    {
      float onset, opaque;
      fog->get_linear_range(onset, opaque);

      _d3d_device->SetRenderState(D3DRENDERSTATE_FOGSTART,
                                   *((LPDWORD) (&onset)));
      _d3d_device->SetRenderState(D3DRENDERSTATE_FOGEND,
                                   *((LPDWORD) (&opaque)));
    }
    break;
  case Fog::M_exponential:
  case Fog::M_exponential_squared:
    {
      // Exponential fog is always camera-relative.
      float fog_density = fog->get_exp_density();
      _d3d_device->SetRenderState(D3DRENDERSTATE_FOGDENSITY,
                                   *((LPDWORD) (&fog_density)));
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_transform
//       Access: Protected
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
//
//               This transform is the internal_transform, already
//               converted into the GSG's internal coordinate system.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_transform() {
  const TransformState *transform = _internal_transform;
  DO_PSTATS_STUFF(_transform_state_pcollector.add_level(1));

  const D3DMATRIX *d3d_mat = (const D3DMATRIX *)transform->get_mat().get_data();
  _d3d_device->SetTransform(D3DTRANSFORMSTATE_WORLD, (LPD3DMATRIX)d3d_mat);
  _transform_stale = false;

  if (_auto_rescale_normal) {
    do_auto_rescale_normal();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_alpha_test
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_alpha_test() {
  const AlphaTestAttrib *attrib = _target._alpha_test;
  AlphaTestAttrib::PandaCompareFunc mode = attrib->get_mode();
  if (mode == AlphaTestAttrib::M_none) {
    _d3d_device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

  } else {
    //  AlphaTestAttrib::PandaCompareFunc === D3DCMPFUNC
    _d3d_device->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, (D3DCMPFUNC)mode);
    _d3d_device->SetRenderState(D3DRENDERSTATE_ALPHAREF, (UINT) (attrib->get_reference_alpha()*255.0f));  //d3d uses 0x0-0xFF, not a float
    _d3d_device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_render_mode
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_render_mode() {
  const RenderModeAttrib *attrib = _target._render_mode;
  RenderModeAttrib::Mode mode = attrib->get_mode();

  switch (mode) {
  case RenderModeAttrib::M_unchanged:
  case RenderModeAttrib::M_filled:
    _d3d_device->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
    break;

  case RenderModeAttrib::M_wireframe:
    _d3d_device->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
    break;

  case RenderModeAttrib::M_point:
    _d3d_device->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_POINT);
    break;

  default:
    dxgsg7_cat.error()
      << "Unknown render mode " << (int)mode << endl;
  }

  _current_fill_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_rescale_normal
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_rescale_normal() {
  const RescaleNormalAttrib *attrib = _target._rescale_normal;
  RescaleNormalAttrib::Mode mode = attrib->get_mode();

  _auto_rescale_normal = false;

  switch (mode) {
  case RescaleNormalAttrib::M_none:
    _d3d_device->SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, false);
    break;

  case RescaleNormalAttrib::M_rescale:
  case RescaleNormalAttrib::M_normalize:
    _d3d_device->SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, true);
    break;

  case RescaleNormalAttrib::M_auto:
    _auto_rescale_normal = true;
    do_auto_rescale_normal();
    break;

  default:
    dxgsg7_cat.error()
      << "Unknown rescale_normal mode " << (int)mode << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_depth_test
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_depth_test() {
  const DepthTestAttrib *attrib = _target._depth_test;
  DepthTestAttrib::PandaCompareFunc mode = attrib->get_mode();
  if (mode == DepthTestAttrib::M_none) {
    _d3d_device->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
  } else {
    _d3d_device->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
    _d3d_device->SetRenderState(D3DRENDERSTATE_ZFUNC, (D3DCMPFUNC) mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_depth_write
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_depth_write() {
  const DepthWriteAttrib *attrib = _target._depth_write;
  if (attrib->get_mode() == DepthWriteAttrib::M_on) {
    _d3d_device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
  } else {
    _d3d_device->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_cull_face
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_cull_face() {
  const CullFaceAttrib *attrib = _target._cull_face;
  _cull_face_mode = attrib->get_effective_mode();

  switch (_cull_face_mode) {
  case CullFaceAttrib::M_cull_none:
    _d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    break;
  case CullFaceAttrib::M_cull_clockwise:
    _d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
    _d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
    break;
  default:
    dxgsg7_cat.error()
      << "invalid cull face mode " << (int)_cull_face_mode << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_fog
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_fog() {
  const FogAttrib *attrib = _target._fog;
  if (!attrib->is_off()) {
    _d3d_device->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
    Fog *fog = attrib->get_fog();
    nassertv(fog != (Fog *)NULL);
    apply_fog(fog);
  } else {
    _d3d_device->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_depth_offset
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_depth_offset() {
  const DepthOffsetAttrib *attrib = _target._depth_offset;
  int offset = attrib->get_offset();
  _d3d_device->SetRenderState(D3DRENDERSTATE_ZBIAS, offset);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_shade_model
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_shade_model() {
  const ShadeModelAttrib *attrib = _target._shade_model;
  switch (attrib->get_mode()) {
  case ShadeModelAttrib::M_smooth:
    _d3d_device->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
    break;

  case ShadeModelAttrib::M_flat:
    _d3d_device->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_state_and_transform
//       Access: Public, Virtual
//  Description: Simultaneously resets the render state and the
//               transform state.
//
//               This transform specified is the "external" net
//               transform, expressed in the external coordinate
//               space; internally, it will be pretransformed by
//               get_cs_transform() to express it in the GSG's
//               internal coordinate space.
//
//               Special case: if (state==NULL), then the target
//               state is already stored in _target.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_state_and_transform(const RenderState *target,
                        const TransformState *transform) {
#ifndef NDEBUG
  if (gsg_cat.is_spam()) {
    gsg_cat.spam() << "Setting GSG state to " << (void *)target << ":\n";
    target->write(gsg_cat.spam(false), 2);
  }
#endif
  _state_pcollector.add_level(1);

  if (transform != _external_transform) {
    _state_pcollector.add_level(1);
    _external_transform = transform;
    _internal_transform = _cs_transform->compose(transform);
    do_issue_transform();
  }
  
  if (target == _state_rs) {
    return;
  }
  _target_rs = target;
  _target.clear_to_defaults();
  target->store_into_slots(&_target);
  _state_rs = 0;

  // There might be some physical limits to the actual target
  // attributes we issue.  Impose them now.
  _target._texture = _target._texture->filter_to_max(_max_texture_stages);
  
  if (_target._alpha_test != _state._alpha_test) {
    do_issue_alpha_test();
    _state._alpha_test = _target._alpha_test;
  }
  
  if (_target._antialias != _state._antialias) {
    // Antialias not implemented under DX7
    _state._antialias = _target._antialias;
  }
  
  if (_target._clip_plane != _state._clip_plane) {
    do_issue_clip_plane();
    _state._clip_plane = _target._clip_plane;
  }
  
  if (_target._color != _state._color) {
    do_issue_color();
    _state._color = _target._color;
  }
  
  if (_target._color_scale != _state._color_scale) {
    do_issue_color_scale();
    _state._color_scale = _target._color_scale;
  }
  
  if (_target._cull_face != _state._cull_face) {
    do_issue_cull_face();
    _state._cull_face = _target._cull_face;
  }
  
  if (_target._depth_offset != _state._depth_offset) {
    do_issue_depth_offset();
    _state._depth_offset = _target._depth_offset;
  }
  
  if (_target._depth_test != _state._depth_test) {
    do_issue_depth_test();
    _state._depth_test = _target._depth_test;
  }
  
  if (_target._depth_write != _state._depth_write) {
    do_issue_depth_write();
    _state._depth_write = _target._depth_write;
  }
  
  if (_target._fog != _state._fog) {
    do_issue_fog();
    _state._fog = _target._fog;
  }
  
  if (_target._render_mode != _state._render_mode) {
    do_issue_render_mode();
    _state._render_mode = _target._render_mode;
  }
  
  if (_target._rescale_normal != _state._rescale_normal) {
    do_issue_rescale_normal();
    _state._rescale_normal = _target._rescale_normal;
  }
  
  if (_target._shade_model != _state._shade_model) {
    do_issue_shade_model();
    _state._shade_model = _target._shade_model;
  }
  
  // Shaders not implemented under DX7
  if (_target._shader != _state._shader) {
    _state._shader = _target._shader;
  }
  
  if (_target._tex_gen != _state._tex_gen) {
    _state._texture = 0;
    _state._tex_gen = _target._tex_gen;
  }
  
  if (_target._tex_matrix != _state._tex_matrix) {
    _state._tex_matrix = _target._tex_matrix;
  }
  
  if ((_target._transparency != _state._transparency)||
      (_target._color_write != _state._color_write)||
      (_target._color_blend != _state._color_blend)) {
    do_issue_blending();
    _state._transparency = _target._transparency;
    _state._color_write = _target._color_write;
    _state._color_blend = _target._color_blend;
  }
  
  if (_target._texture != _state._texture) {
    do_issue_texture();
    _state._texture = _target._texture;
  }

  if (_target._material != _state._material) {
    do_issue_material();
    _state._material = _target._material;
  }
  
  if (_target._light != _state._light) {
    do_issue_light();
    _state._light = _target._light;
  }

  _state_rs = _target_rs;
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
bind_light(PointLight *light_obj, const NodePath &light, int light_id) {
  // Get the light in "world coordinates".  This means the light in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  CPT(TransformState) transform = light.get_transform(_scene_setup->get_camera_path());
  const LMatrix4f &light_mat = transform->get_mat();
  LMatrix4f rel_mat = light_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  LPoint3f pos = light_obj->get_point() * rel_mat;

  D3DCOLORVALUE black;
  black.r = black.g = black.b = black.a = 0.0f;
  D3DLIGHT7  alight;
  alight.dltType =  D3DLIGHT_POINT;
  alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light_obj->get_color().get_data());
  alight.dcvAmbient  =  black ;
  alight.dcvSpecular = *(D3DCOLORVALUE *)(light_obj->get_specular_color().get_data());

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  alight.dvPosition = *(D3DVECTOR *)pos.get_data();

  alight.dvRange =  D3DLIGHT_RANGE_MAX;
  alight.dvFalloff =  1.0f;

  const LVecBase3f &att = light_obj->get_attenuation();
  alight.dvAttenuation0 = (D3DVALUE)att[0];
  alight.dvAttenuation1 = (D3DVALUE)att[1];
  alight.dvAttenuation2 = (D3DVALUE)att[2];

  HRESULT res = _screen->pD3DDevice->SetLight(light_id, &alight);
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
bind_light(DirectionalLight *light_obj, const NodePath &light, int light_id) {
  // Get the light in "world coordinates".  This means the light in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  CPT(TransformState) transform = light.get_transform(_scene_setup->get_camera_path());
  const LMatrix4f &light_mat = transform->get_mat();
  LMatrix4f rel_mat = light_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  LVector3f dir = light_obj->get_direction() * rel_mat;

  D3DCOLORVALUE black;
  black.r = black.g = black.b = black.a = 0.0f;

  D3DLIGHT7  alight;
  ZeroMemory(&alight, sizeof(D3DLIGHT7));

  alight.dltType =  D3DLIGHT_DIRECTIONAL;
  alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light_obj->get_color().get_data());
  alight.dcvAmbient  =  black ;
  alight.dcvSpecular = *(D3DCOLORVALUE *)(light_obj->get_specular_color().get_data());

  alight.dvDirection = *(D3DVECTOR *)dir.get_data();

  alight.dvRange =  D3DLIGHT_RANGE_MAX;
  alight.dvFalloff =  1.0f;

  alight.dvAttenuation0 = 1.0f;       // constant
  alight.dvAttenuation1 = 0.0f;       // linear
  alight.dvAttenuation2 = 0.0f;       // quadratic

  HRESULT res = _screen->pD3DDevice->SetLight(light_id, &alight);
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
bind_light(Spotlight *light_obj, const NodePath &light, int light_id) {
  Lens *lens = light_obj->get_lens();
  nassertv(lens != (Lens *)NULL);

  // Get the light in "world coordinates".  This means the light in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  CPT(TransformState) transform = light.get_transform(_scene_setup->get_camera_path());
  const LMatrix4f &light_mat = transform->get_mat();
  LMatrix4f rel_mat = light_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  LPoint3f pos = lens->get_nodal_point() * rel_mat;
  LVector3f dir = lens->get_view_vector() * rel_mat;

  D3DCOLORVALUE black;
  black.r = black.g = black.b = black.a = 0.0f;

  D3DLIGHT7  alight;
  ZeroMemory(&alight, sizeof(D3DLIGHT7));

  alight.dltType =  D3DLIGHT_SPOT;
  alight.dcvAmbient  =  black ;
  alight.dcvDiffuse  = *(D3DCOLORVALUE *)(light_obj->get_color().get_data());
  alight.dcvSpecular = *(D3DCOLORVALUE *)(light_obj->get_specular_color().get_data());

  alight.dvPosition = *(D3DVECTOR *)pos.get_data();

  alight.dvDirection = *(D3DVECTOR *)dir.get_data();

  alight.dvRange =  D3DLIGHT_RANGE_MAX;
  alight.dvFalloff =  1.0f;
  alight.dvTheta =  0.0f;
  alight.dvPhi = deg_2_rad(lens->get_hfov());

  const LVecBase3f &att = light_obj->get_attenuation();
  alight.dvAttenuation0 = (D3DVALUE)att[0];
  alight.dvAttenuation1 = (D3DVALUE)att[1];
  alight.dvAttenuation2 = (D3DVALUE)att[2];

  HRESULT hr = _screen->pD3DDevice->SetLight(light_id, &alight);
  if (FAILED(hr)) {
    wdxdisplay7_cat.warning() 
      << "Could not set light properties for " << light 
      << " to id " << light_id << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_material() {
  static Material empty;
  const Material *material;
  if (_target._material == (MaterialAttrib *)NULL ||
      _target._material->is_off()) {
    material = &empty;
  } else {
    material = _target._material->get_material();
  }

  D3DMATERIAL7 cur_material;
  cur_material.dcvDiffuse = *(D3DCOLORVALUE *)(material->get_diffuse().get_data());
  cur_material.dcvAmbient = *(D3DCOLORVALUE *)(material->get_ambient().get_data());
  cur_material.dcvSpecular = *(D3DCOLORVALUE *)(material->get_specular().get_data());
  cur_material.dcvEmissive = *(D3DCOLORVALUE *)(material->get_emission().get_data());
  cur_material.dvPower = material->get_shininess();

  if (material->has_diffuse()) {
    // If the material specifies an diffuse color, use it.
    _d3d_device->SetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
  } else {
    // Otherwise, the diffuse color comes from the object color.
    if (_has_material_force_color) {
      cur_material.dcvDiffuse = *(D3DCOLORVALUE *)_material_force_color.get_data();
      _d3d_device->SetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
    } else {
      _d3d_device->SetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    }
  }
  if (material->has_ambient()) {
    // If the material specifies an ambient color, use it.
    _d3d_device->SetRenderState(D3DRENDERSTATE_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
  } else {
    // Otherwise, the ambient color comes from the object color.
    if (_has_material_force_color) {
      cur_material.dcvAmbient = *(D3DCOLORVALUE *)_material_force_color.get_data();
      _d3d_device->SetRenderState(D3DRENDERSTATE_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
    } else {
      _d3d_device->SetRenderState(D3DRENDERSTATE_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    }
  }

  if (material->has_specular()) {
    _d3d_device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);
  } else {
    _d3d_device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
  }

  if (material->get_local()) {
    _d3d_device->SetRenderState(D3DRENDERSTATE_LOCALVIEWER, TRUE);
  } else {
    _d3d_device->SetRenderState(D3DRENDERSTATE_LOCALVIEWER, FALSE);
  }

  _d3d_device->SetMaterial(&cur_material);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_texture() {
  DO_PSTATS_STUFF(_texture_state_pcollector.add_level(1));

  int num_stages = _target._texture->get_num_on_stages();
  int num_old_stages = _max_texture_stages;
  if (_state._texture != (TextureAttrib *)NULL) {
    num_old_stages = _state._texture->get_num_on_stages();
  }

  nassertv(num_stages <= _max_texture_stages &&
           num_old_stages <= _max_texture_stages);

  _texture_involves_color_scale = false;

  // We have to match up the texcoord stage index to the order written
  // out by the DXGeomMunger.  This means the texcoord names are
  // written in the order they are referenced by the TextureAttrib,
  // except that if a name is repeated its index number is reused from
  // the first time.
  typedef pmap<const InternalName *, int> UsedTexcoordIndex;
  UsedTexcoordIndex used_texcoord_index;

  int i;
  for (i = 0; i < num_stages; i++) {
    TextureStage *stage = _target._texture->get_on_stage(i);
    Texture *texture = _target._texture->get_on_texture(stage);
    nassertv(texture != (Texture *)NULL);

    const InternalName *name = stage->get_texcoord_name();

    // This pair of lines will get the next consecutive texcoord index
    // number if this is the first time we have referenced this
    // particular texcoord name; otherwise, it will return the same
    // index number it returned before.
    UsedTexcoordIndex::iterator ti = used_texcoord_index.insert(UsedTexcoordIndex::value_type(name, (int)used_texcoord_index.size())).first;
    int texcoord_index = (*ti).second;

    // We always reissue every stage in DX, just in case the texcoord
    // index or texgen mode or some other property has changed.
    TextureContext *tc = texture->prepare_now(_prepared_objects, this);
    apply_texture(i, tc);
    set_texture_blend_mode(i, stage);

    int texcoord_dimensions = 0;

    CPT(TransformState) tex_mat = TransformState::make_identity();
    if (_state._tex_matrix->has_stage(stage)) {
      tex_mat = _state._tex_matrix->get_transform(stage);
    }
    
    // Issue the texgen mode.
    TexGenAttrib::Mode mode = _state._tex_gen->get_mode(stage);
    bool any_point_sprite = false;

    switch (mode) {
    case TexGenAttrib::M_off:
    case TexGenAttrib::M_light_vector:
      _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, texcoord_index);
      break;
      
    case TexGenAttrib::M_eye_sphere_map:
      {
        _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX,
                                          texcoord_index | D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
        // This texture matrix, applied on top of the texcoord
        // computed by D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR,
        // approximates the effect produced by OpenGL's GL_SPHERE_MAP.
        static CPT(TransformState) sphere_map =
          TransformState::make_mat(LMatrix4f(0.33f, 0.0f, 0.0f, 0.0f,
                                             0.0f, 0.33f, 0.0f, 0.0f,
                                             0.0f, 0.0f, 1.0f, 0.0f,
                                             0.5f, 0.5f, 0.0f, 1.0f));
        tex_mat = tex_mat->compose(sphere_map);
        texcoord_dimensions = 3;
      }
      break;

    case TexGenAttrib::M_world_cube_map:
      // To achieve world reflection vector, we must transform camera
      // coordinates to world coordinates; i.e. apply the camera
      // transform.  In the case of a vector, we should not apply the
      // pos component of the transform.
      {
        _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 
                                          texcoord_index | D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
        texcoord_dimensions = 3;
        CPT(TransformState) camera_transform = _scene_setup->get_camera_transform()->compose(_inv_cs_transform);
        tex_mat = tex_mat->compose(camera_transform->set_pos(LVecBase3f::zero()));
      }
      break;

    case TexGenAttrib::M_eye_cube_map:
      _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 
                                        texcoord_index | D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
      tex_mat = tex_mat->compose(_inv_cs_transform);
      texcoord_dimensions = 3;
      break;

    case TexGenAttrib::M_world_normal:
      // To achieve world normal, we must transform camera coordinates
      // to world coordinates; i.e. apply the camera transform.  In
      // the case of a normal, we should not apply the pos component
      // of the transform.
      {
        _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 
                                          texcoord_index | D3DTSS_TCI_CAMERASPACENORMAL);
        texcoord_dimensions = 3;
        CPT(TransformState) camera_transform = _scene_setup->get_camera_transform()->compose(_inv_cs_transform);
        tex_mat = tex_mat->compose(camera_transform->set_pos(LVecBase3f::zero()));
      }
      break;

    case TexGenAttrib::M_eye_normal:
      _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 
                                        texcoord_index | D3DTSS_TCI_CAMERASPACENORMAL);
      texcoord_dimensions = 3;
      tex_mat = tex_mat->compose(_inv_cs_transform);
      break;

    case TexGenAttrib::M_world_position:
      // To achieve world position, we must transform camera
      // coordinates to world coordinates; i.e. apply the
      // camera transform.
      {
        _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 
                                          texcoord_index | D3DTSS_TCI_CAMERASPACEPOSITION);
        texcoord_dimensions = 3;
        CPT(TransformState) camera_transform = _scene_setup->get_camera_transform()->compose(_inv_cs_transform);
        tex_mat = tex_mat->compose(camera_transform);
      }
      break;

    case TexGenAttrib::M_eye_position:
      _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 
                                        texcoord_index | D3DTSS_TCI_CAMERASPACEPOSITION);
      texcoord_dimensions = 3;
      tex_mat = tex_mat->compose(_inv_cs_transform);
      break;
      
    case TexGenAttrib::M_point_sprite:
      _d3d_device->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, texcoord_index);
      any_point_sprite = true;
      break;
    }
    // Dx7 doesn't support point sprites anyway.
    nassertv(!any_point_sprite);

    if (!tex_mat->is_identity()) {
      if (tex_mat->is_2d() && texcoord_dimensions <= 2) {
        // For 2-d texture coordinates, we have to reorder the matrix.
        LMatrix4f m = tex_mat->get_mat();
        m.set(m(0, 0), m(0, 1), m(0, 3), 0.0f,
              m(1, 0), m(1, 1), m(1, 3), 0.0f,
              m(3, 0), m(3, 1), m(3, 3), 0.0f,
              0.0f, 0.0f, 0.0f, 1.0f);
        _d3d_device->SetTransform(get_tex_mat_sym(i), (D3DMATRIX *)m.get_data());
        _d3d_device->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS,
                                          D3DTTFF_COUNT2);
      } else {
        LMatrix4f m = tex_mat->get_mat();
        _d3d_device->SetTransform(get_tex_mat_sym(i), (D3DMATRIX *)m.get_data());
	DWORD transform_flags = texcoord_dimensions;
	if (m.get_col(3) != LVecBase4f(0.0f, 0.0f, 0.0f, 1.0f)) {
	  // If we have a projected texture matrix, we also need to
	  // set D3DTTFF_COUNT4.
	  transform_flags = D3DTTFF_COUNT4 | D3DTTFF_PROJECTED;
	}
        _d3d_device->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS,
                                          transform_flags);
      }

    } else {
      _d3d_device->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS,
                                        D3DTTFF_DISABLE);
      // For some reason, "disabling" texture coordinate transforms
      // doesn't seem to be sufficient.  We'll load an identity matrix
      // to underscore the point.
      _d3d_device->SetTransform(get_tex_mat_sym(i), &_d3d_ident_mat);
    }
  }

  // Disable the texture stages that are no longer used.
  for (i = num_stages; i < num_old_stages; i++) {
    _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
  }
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
  _d3d_device->SetRenderState(D3DRENDERSTATE_LIGHTING, (DWORD)enable);
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
  Colorf c = color;
  c.set(c[0] * _light_color_scale[0],
        c[1] * _light_color_scale[1],
        c[2] * _light_color_scale[2],
        c[3] * _light_color_scale[3]);

  _d3d_device->SetRenderState(D3DRENDERSTATE_AMBIENT, Colorf_to_D3DCOLOR(c));
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
  HRESULT hr = _d3d_device->LightEnable(light_id, enable);

  if (FAILED(hr)) {
    wdxdisplay7_cat.warning()
      << "Could not enable light " << light_id << ": "
      << D3DERRORSTRING(hr) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enable_clip_plane
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated clip_plane id.  A specific
//               PlaneNode will already have been bound to this id via
//               bind_clip_plane().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
enable_clip_plane(int plane_id, bool enable) {
  if (enable) {
    _clip_plane_bits |= ((DWORD)1 << plane_id);
  } else {
    _clip_plane_bits &= ~((DWORD)1 << plane_id);
  }
  _d3d_device->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE, _clip_plane_bits);
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
bind_clip_plane(const NodePath &plane, int plane_id) {
  // Get the plane in "world coordinates".  This means the plane in
  // the coordinate space of the camera, converted to DX's coordinate
  // system.
  CPT(TransformState) transform = plane.get_transform(_scene_setup->get_camera_path());
  const LMatrix4f &plane_mat = transform->get_mat();
  LMatrix4f rel_mat = plane_mat * LMatrix4f::convert_mat(CS_yup_left, CS_default);
  const PlaneNode *plane_node;
  DCAST_INTO_V(plane_node, plane.node());
  Planef world_plane = plane_node->get_plane() * rel_mat;

  HRESULT hr = _d3d_device->SetClipPlane(plane_id, (D3DVALUE*)world_plane.get_data());
  if (FAILED(hr)) {
    wdxdisplay7_cat.warning()
      << "Could not set clip plane for " << plane
      << " to id " << plane_id << ": " << D3DERRORSTRING(hr) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_blend_mode
//       Access: Protected, Virtual
//  Description: Called after any of the things that might change
//               blending state have changed, this function is
//               responsible for setting the appropriate color
//               blending mode based on the current properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_issue_blending() {

  // Handle the color_write attrib.  If color_write is off, then
  // all the other blending-related stuff doesn't matter.  If the
  // device doesn't support color-write, we use blending tricks
  // to effectively disable color write.
  if (_target._color_write->get_channels() == ColorWriteAttrib::C_off) {
    if (_target._color_write != _state._color_write) {
      _d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
      _d3d_device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
      _d3d_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
    }
    return;
  }

  CPT(ColorBlendAttrib) color_blend = _target._color_blend;
  ColorBlendAttrib::Mode color_blend_mode = _target._color_blend->get_mode();
  TransparencyAttrib::Mode transparency_mode = _target._transparency->get_mode();

  // Is there a color blend set?
  if (color_blend_mode != ColorBlendAttrib::M_none) {
    _d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

    // DX7 supports only ColorBlendAttrib::M_add.  Assume that's what
    // we've got; if the user asked for anything else, give him M_add
    // instead.
    _d3d_device->SetRenderState(D3DRENDERSTATE_SRCBLEND, get_blend_func(color_blend->get_operand_a()));
    _d3d_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, get_blend_func(color_blend->get_operand_b()));
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
    _d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
    _d3d_device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
    _d3d_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
    return;

  default:
    dxgsg7_cat.error()
      << "invalid transparency mode " << (int)transparency_mode << endl;
    break;
  }

  // Nothing's set, so disable blending.
  _d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::free_nondx_resources
//       Access: Public
//  Description: Frees some memory that was explicitly allocated
//               within the dxgsg.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
free_nondx_resources() {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::free_d3d_device
//       Access: Public
//  Description: setup for re-calling dx_init(), this is not the final
//               exit cleanup routine (see dx_cleanup)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
free_d3d_device() {
  // dont want a full reset of gsg, just a state clear
  _state_rs = 0;
  _state.clear_to_zero();
  // want gsg to pass all state settings through

  _dx_is_ready = false;

  if (_d3d_device != NULL)
    _d3d_device->SetTexture(0, NULL);  // d3d should release this stuff internally anyway, but whatever

  release_all();

  if (_d3d_device != NULL)
    RELEASE(_d3d_device, dxgsg7, "d3dDevice", RELEASE_DOWN_TO_ZERO);

  free_nondx_resources();

  // obviously we dont release ID3D7, just ID3DDevice7
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
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_read_buffer
//       Access: Protected
//  Description: Vestigial analog of glReadBuffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_read_buffer(const RenderBuffer &rb) {
  if (rb._buffer_type & RenderBuffer::T_front) {
    _cur_read_pixel_buffer = RenderBuffer::T_front;
  } else  if (rb._buffer_type & RenderBuffer::T_back) {
    _cur_read_pixel_buffer = RenderBuffer::T_back;
  } else {
    dxgsg7_cat.error() << "Invalid or unimplemented Argument to set_read_buffer!\n";
  }
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::do_auto_rescale_normal
//       Access: Protected
//  Description: Issues the appropriate GL commands to either rescale
//               or normalize the normals according to the current
//               transform.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
do_auto_rescale_normal() {
  if (_external_transform->has_identity_scale()) {
    // If there's no scale, don't normalize anything.
    _d3d_device->SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, false);

  } else {
    // If there is a scale, turn on normalization.
    _d3d_device->SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLGraphicsStateGuardian::get_light_color
//       Access: Public
//  Description: Returns the array of four floats that should be
//               issued as the light's color, as scaled by the current
//               value of _light_color_scale, in the case of
//               color_scale_via_lighting.
////////////////////////////////////////////////////////////////////
const D3DCOLORVALUE &DXGraphicsStateGuardian7::
get_light_color(Light *light) const {
  static Colorf c;
  c = light->get_color();
  c.set(c[0] * _light_color_scale[0],
        c[1] * _light_color_scale[1],
        c[2] * _light_color_scale[2],
        c[3] * _light_color_scale[3]);
  return *(D3DCOLORVALUE *)c.get_data();
}

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

#ifdef DO_PSTATS
#ifdef TEXMGRSTATS_USES_GETAVAILVIDMEM
  DWORD dwTexTotal, dwTexFree, dwVidTotal, dwVidFree;

  if (_total_texmem_pcollector.is_active()) {
    DDSCAPS2 ddsCaps;

    ZeroMemory(&ddsCaps, sizeof(ddsCaps));

    ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
    if (FAILED( hr = _d3d_device->GetAvailableVidMem(&ddsCaps, &dwVidTotal, &dwVidFree))) {
      dxgsg7_cat.fatal() << "report_texmgr GetAvailableVidMem for VIDMEM failed : result = " << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
      return;
    }

    ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    if (FAILED( hr = _d3d_device->GetAvailableVidMem(&ddsCaps, &dwTexTotal, &dwTexFree))) {
      dxgsg7_cat.fatal() << "report_texmgr GetAvailableVidMem for TEXTURE failed : result = " << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
      return;
    }
  }
#endif  // TEXMGRSTATS_USES_GETAVAILVIDMEM
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_context
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_context(DXScreenData *new_context) {
  nassertv(new_context != NULL);
  _screen = new_context;
  _d3d_device = _screen->pD3DDevice;   //copy this one field for speed of deref
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_render_target
//       Access: Protected
//  Description: Set render target to the backbuffer of current swap
//               chain.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_render_target() {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::set_texture_blend_mode
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
set_texture_blend_mode(int i, const TextureStage *stage) {
  _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE);
  _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
  _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::dx_cleanup
//       Access: Protected
//  Description: Clean up the DirectX environment, accounting for exit()
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
dx_cleanup() {
  if (!_d3d_device) {
    return;
  }

  free_nondx_resources();

  // Do a safe check for releasing the D3DDEVICE. RefCount should be zero.
  // if we're called from exit(), _d3d_device may already have been released
  RELEASE(_d3d_device, dxgsg7, "d3dDevice", RELEASE_DOWN_TO_ZERO);
  _d3d_device = NULL;
  _screen->pD3DDevice = NULL;

  // Releasing pD3D is now the responsibility of the GraphicsPipe destructor
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::check_cooperative_level
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian7::
check_cooperative_level() {
  HRESULT hr = _screen->pDD->TestCooperativeLevel();

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

      release_all();

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
//     Function: DXGraphicsStateGuardian7::show_frame
//       Access: Protected
//  Description: redraw primary buffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
show_frame() {
  if(_screen->pddsPrimary==NULL)
    return;

  //  DO_PSTATS_STUFF(PStatTimer timer(_win->_swap_pcollector));  // this times just the flip, so it must go here in dxgsg7, instead of wdxdisplay, which would time the whole frame

  if (_screen->bIsFullScreen) {
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
void DXGraphicsStateGuardian7::show_full_screen_frame() {
  HRESULT hr;

  // Flip the front and back buffers, to make what we just rendered
  // visible.

  DWORD dwFlipFlags = DDFLIP_WAIT;
  hr = _screen->pddsPrimary->Flip( NULL, dwFlipFlags);

  if(FAILED(hr)) {
    if((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      check_cooperative_level();
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
void DXGraphicsStateGuardian7::show_windowed_frame() {
  HRESULT hr;

  hr = _screen->pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
  if(hr != DD_OK) {
    dxgsg7_cat.error() 
      << "WaitForVerticalBlank() failed : " << ConvD3DErrorToString(hr)
      << endl;
    throw_event("panda3d-render-error");
  }

  DX_DECLARE_CLEAN(DDBLTFX, bltfx);

  bltfx.dwDDFX |= DDBLTFX_NOTEARING;  // hmm, does any driver actually recognize this flag?
  hr = _screen->pddsPrimary->Blt( &_screen->view_rect, _screen->pddsBack,  NULL, DDBLT_DDFX | DDBLT_WAIT, &bltfx );

  if(FAILED(hr)) {
    if((hr == DDERR_SURFACELOST) || (hr == DDERR_SURFACEBUSY)) {
      check_cooperative_level();
    } else {
      dxgsg7_cat.error() << "show_frame() - Blt failed : " << ConvD3DErrorToString(hr) << endl;
      exit(1);
    }
  }

}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::copy_pres_reset
//       Access: Protected
//  Description: copies the PresReset from passed DXScreenData
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian7::
copy_pres_reset(DXScreenData *screen) {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::get_d3d_min_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
D3DTEXTUREMINFILTER DXGraphicsStateGuardian7::
get_d3d_min_type(Texture::FilterType filter_type) {
  switch (filter_type) {
  case Texture::FT_nearest:
    return D3DTFN_POINT;

  case Texture::FT_linear:
    return D3DTFN_LINEAR;

  case Texture::FT_nearest_mipmap_nearest:
    return D3DTFN_POINT;

  case Texture::FT_linear_mipmap_nearest:
    return D3DTFN_LINEAR;

  case Texture::FT_nearest_mipmap_linear:
    return D3DTFN_POINT;

  case Texture::FT_linear_mipmap_linear:
    return D3DTFN_LINEAR;
  }

  dxgsg7_cat.error()
    << "Invalid FilterType value (" << (int)filter_type << ")\n";
  return D3DTFN_POINT;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::get_d3d_mip_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
D3DTEXTUREMIPFILTER DXGraphicsStateGuardian7::
get_d3d_mip_type(Texture::FilterType filter_type) {
  switch (filter_type) {
  case Texture::FT_nearest:
    return D3DTFP_NONE;

  case Texture::FT_linear:
    return D3DTFP_NONE;

  case Texture::FT_nearest_mipmap_nearest:
    return D3DTFP_POINT;

  case Texture::FT_linear_mipmap_nearest:
    return D3DTFP_POINT;

  case Texture::FT_nearest_mipmap_linear:
    return D3DTFP_LINEAR;

  case Texture::FT_linear_mipmap_linear:
    return D3DTFP_LINEAR;
  }

  dxgsg7_cat.error()
    << "Invalid FilterType value (" << (int)filter_type << ")\n";
  return D3DTFP_NONE;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::get_texture_operation
//       Access: Public, Static
//  Description: Returns the D3DTEXTUREOP value corresponding to the
//               indicated TextureStage::CombineMode enumerated type.
////////////////////////////////////////////////////////////////////
D3DTEXTUREOP DXGraphicsStateGuardian7::
get_texture_operation(TextureStage::CombineMode mode, int scale) {
  return D3DTOP_SELECTARG1;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::get_texture_argument
//       Access: Public, Static
//  Description: Returns the D3DTA value corresponding to the
//               indicated TextureStage::CombineSource and
//               TextureStage::CombineOperand enumerated types.
////////////////////////////////////////////////////////////////////
DWORD DXGraphicsStateGuardian7::
get_texture_argument(TextureStage::CombineSource source,
                     TextureStage::CombineOperand operand) {
  return D3DTA_CURRENT;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::get_texture_argument_modifier
//       Access: Public, Static
//  Description: Returns the extra bits that modify the D3DTA
//               argument, according to the indicated
//               TextureStage::CombineOperand enumerated type.
////////////////////////////////////////////////////////////////////
DWORD DXGraphicsStateGuardian7::
get_texture_argument_modifier(TextureStage::CombineOperand operand) {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian7::enum_tex_formats_callback
//       Access: Public, Static
//  Description: This is attached as a callback to the
//               EnumTextureFormats() call, which gets all of the
//               available texture formats for the display device.
////////////////////////////////////////////////////////////////////
HRESULT CALLBACK DXGraphicsStateGuardian7::
enum_tex_formats_callback(LPDDPIXELFORMAT pddpf, void *param) {
  DXGraphicsStateGuardian7 *gsg = (DXGraphicsStateGuardian7 *)param;
  nassertr(gsg->_num_tex_formats < MAX_DX_TEXPIXFMTS, E_FAIL);
  memcpy(&(gsg->_tex_formats[gsg->_num_tex_formats]), pddpf, 
	 sizeof(DDPIXELFORMAT));
  gsg->_num_tex_formats++;
  
  return DDENUMRET_OK;
}
