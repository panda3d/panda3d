// Filename: dxGraphicsStateGuardian8.cxx
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

#include "dxGraphicsStateGuardian8.h"
#include "config_dxgsg8.h"
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
#include "dxGeomMunger8.h"
#include "config_gobj.h"
#include "dxVertexBufferContext8.h"
#include "dxIndexBufferContext8.h"
#include "pStatTimer.h"
#include "pStatCollector.h"

#include <d3dx8.h>
#include <mmsystem.h>

TypeHandle DXGraphicsStateGuardian8::_type_handle;

D3DMATRIX DXGraphicsStateGuardian8::_d3d_ident_mat;


#define __D3DLIGHT_RANGE_MAX ((float)sqrt(FLT_MAX))  //for some reason this is missing in dx8 hdrs

#define MY_D3DRGBA(r, g, b, a) ((D3DCOLOR) D3DCOLOR_COLORVALUE(r, g, b, a))

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian8::
DXGraphicsStateGuardian8(const FrameBufferProperties &properties) :
  GraphicsStateGuardian(properties, CS_yup_left)
{
  _screen = NULL;
  _d3d_device = NULL;

  _dx_is_ready = false;
  _vertex_blending_enabled = false;
  _overlay_windows_supported = false;
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

  // D3DRS_POINTSPRITEENABLE doesn't seem to support remapping the
  // texture coordinates via a texture matrix, so we don't advertise
  // GR_point_sprite_tex_matrix.
  _supported_geom_rendering = 
    Geom::GR_point | Geom::GR_point_uniform_size |
    Geom::GR_point_perspective | Geom::GR_point_sprite |
    Geom::GR_triangle_strip | Geom::GR_triangle_fan |
    Geom::GR_flat_first_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian8::
~DXGraphicsStateGuardian8() {
  if (IS_VALID_PTR(_d3d_device)) {
    _d3d_device->SetTexture(0, NULL);  // this frees reference to the old texture
  }
  free_nondx_resources();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::prepare_texture
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
TextureContext *DXGraphicsStateGuardian8::
prepare_texture(Texture *tex) {
  DXTextureContext8 *dtc = new DXTextureContext8(tex);
  if (!dtc->create_texture(*_screen)) {
    delete dtc;
    return NULL;
  }

  return dtc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::apply_texture
//       Access: Public
//  Description: Makes the texture the currently available texture for
//               rendering on the ith stage.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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

  DXTextureContext8 *dtc = DCAST(DXTextureContext8, tc);

  int dirty = dtc->get_dirty_flags();

  if (dirty) {
    // If the texture image has changed, or if its use of mipmaps has
    // changed, we need to re-create the image.  Ignore other types of
    // changes, which aren't significant for DX.

    if ((dirty & (Texture::DF_image | Texture::DF_mipmap)) != 0) {
      // If this is *only* because of a mipmap change, issue a
      // warning--it is likely that this change is the result of an
      // error or oversight.
      if ((dirty & Texture::DF_image) == 0) {
        dxgsg8_cat.warning()
          << "Texture " << *dtc->_texture << " has changed mipmap state.\n";
      }

      if (!dtc->create_texture(*_screen)) {
        // Oops, we can't re-create the texture for some reason.
        dxgsg8_cat.error()
          << "Unable to re-create texture " << *dtc->_texture << endl;
        _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        return;
      }
    }
  }

  Texture *tex = tc->_texture;
  Texture::WrapMode wrap_u, wrap_v, wrap_w;
  wrap_u = tex->get_wrap_u();
  wrap_v = tex->get_wrap_v();
  wrap_w = tex->get_wrap_w();

  _d3d_device->SetTextureStageState(i, D3DTSS_ADDRESSU, get_texture_wrap_mode(wrap_u));
  _d3d_device->SetTextureStageState(i, D3DTSS_ADDRESSV, get_texture_wrap_mode(wrap_v));
  _d3d_device->SetTextureStageState(i, D3DTSS_ADDRESSW, get_texture_wrap_mode(wrap_w));

  _d3d_device->SetTextureStageState(i, D3DTSS_BORDERCOLOR, 
				    Colorf_to_D3DCOLOR(tex->get_border_color()));

  uint aniso_degree = tex->get_anisotropic_degree();
  Texture::FilterType ft = tex->get_magfilter();

  _d3d_device->SetTextureStageState(i, D3DTSS_MAXANISOTROPY, aniso_degree);

  D3DTEXTUREFILTERTYPE new_mag_filter;
  if (aniso_degree <= 1) {
    new_mag_filter = ((ft != Texture::FT_nearest) ? D3DTEXF_LINEAR : D3DTEXF_POINT);
  } else {
    new_mag_filter = D3DTEXF_ANISOTROPIC;
  }

  _d3d_device->SetTextureStageState(i, D3DTSS_MAGFILTER, new_mag_filter);

  // map Panda composite min+mip filter types to d3d's separate min & mip filter types
  D3DTEXTUREFILTERTYPE new_min_filter = get_d3d_min_type(tex->get_minfilter());
  D3DTEXTUREFILTERTYPE new_mip_filter = get_d3d_mip_type(tex->get_minfilter());

  if (!tex->might_have_ram_image()) {
    // If the texture is completely dynamic, don't try to issue
    // mipmaps--pandadx doesn't support auto-generated mipmaps at this
    // point.
    new_mip_filter = D3DTEXF_NONE;
  }

#ifndef NDEBUG
  // sanity check
  if ((!dtc->has_mipmaps()) && (new_mip_filter != D3DTEXF_NONE)) {
    dxgsg8_cat.error()
      << "Trying to set mipmap filtering for texture with no generated mipmaps!! texname["
      << tex->get_name() << "], filter("
      << tex->get_minfilter() << ")\n";
    new_mip_filter = D3DTEXF_NONE;
  }
#endif

  if (aniso_degree >= 2) {
    new_min_filter = D3DTEXF_ANISOTROPIC;
  }

  _d3d_device->SetTextureStageState(i, D3DTSS_MINFILTER, new_min_filter);
  _d3d_device->SetTextureStageState(i, D3DTSS_MIPFILTER, new_mip_filter);

  _d3d_device->SetTexture(i, dtc->get_d3d_texture());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
release_texture(TextureContext *tc) {
  DXTextureContext8 *dtc = DCAST(DXTextureContext8, tc);
  delete dtc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::prepare_vertex_buffer
//       Access: Public, Virtual
//  Description: Creates a new retained-mode representation of the
//               given data, and returns a newly-allocated
//               VertexBufferContext pointer to reference it.  It is the
//               responsibility of the calling function to later
//               call release_vertex_buffer() with this same pointer (which
//               will also delete the pointer).
//
//               This function should not be called directly to
//               prepare a buffer.  Instead, call Geom::prepare().
////////////////////////////////////////////////////////////////////
VertexBufferContext *DXGraphicsStateGuardian8::
prepare_vertex_buffer(GeomVertexArrayData *data) {
  DXVertexBufferContext8 *dvbc = new DXVertexBufferContext8(data);
  return dvbc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::apply_vertex_buffer
//       Access: Public
//  Description: Updates the vertex buffer with the current data, and
//               makes it the current vertex buffer for rendering.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
apply_vertex_buffer(VertexBufferContext *vbc) {
  DXVertexBufferContext8 *dvbc = DCAST(DXVertexBufferContext8, vbc);

  if (dvbc->_vbuffer == NULL) {
    // Attempt to create a new vertex buffer.
    if (vertex_buffers &&
        dvbc->get_data()->get_usage_hint() != Geom::UH_client) {
      dvbc->create_vbuffer(*_screen);
    }

    if (dvbc->_vbuffer != NULL) {
      dvbc->upload_data();

      add_to_total_buffer_record(dvbc);
      dvbc->mark_loaded();

      _d3d_device->SetStreamSource
        (0, dvbc->_vbuffer, dvbc->get_data()->get_array_format()->get_stride());
      _active_vbuffer = dvbc;
      _active_ibuffer = NULL;
      add_to_vertex_buffer_record(dvbc);

    } else {
      _active_vbuffer = NULL;
    }

  } else {
    if (dvbc->was_modified()) {
      if (dvbc->changed_size()) {
        // We have to destroy the old vertex buffer and create a new
        // one.
        dvbc->create_vbuffer(*_screen);
      }

      dvbc->upload_data();

      add_to_total_buffer_record(dvbc);
      dvbc->mark_loaded();
      _active_vbuffer = NULL;
    }

    if (_active_vbuffer != dvbc) {
      _d3d_device->SetStreamSource
        (0, dvbc->_vbuffer, dvbc->get_data()->get_array_format()->get_stride());
      _active_vbuffer = dvbc;
      _active_ibuffer = NULL;
      add_to_vertex_buffer_record(dvbc);
    }
  }

  set_vertex_format(dvbc->_fvf);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::release_vertex_buffer
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               data.  This function should never be called
//               directly; instead, call Data::release() (or simply
//               let the Data destruct).
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
release_vertex_buffer(VertexBufferContext *vbc) {
  DXVertexBufferContext8 *dvbc = DCAST(DXVertexBufferContext8, vbc);
  delete dvbc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::prepare_index_buffer
//       Access: Public, Virtual
//  Description: Creates a new retained-mode representation of the
//               given data, and returns a newly-allocated
//               IndexBufferContext pointer to reference it.  It is the
//               responsibility of the calling function to later call
//               release_index_buffer() with this same pointer (which
//               will also delete the pointer).
//
//               This function should not be called directly to
//               prepare a buffer.  Instead, call Geom::prepare().
////////////////////////////////////////////////////////////////////
IndexBufferContext *DXGraphicsStateGuardian8::
prepare_index_buffer(GeomPrimitive *data) {
  DXIndexBufferContext8 *dibc = new DXIndexBufferContext8(data);
  return dibc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::apply_index_buffer
//       Access: Public
//  Description: Updates the index buffer with the current data, and
//               makes it the current index buffer for rendering.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
apply_index_buffer(IndexBufferContext *ibc) {
  DXIndexBufferContext8 *dibc = DCAST(DXIndexBufferContext8, ibc);

  if (dibc->_ibuffer == NULL) {
    // Attempt to create a new index buffer.
    dibc->create_ibuffer(*_screen);

    if (dibc->_ibuffer != NULL) {
      dibc->upload_data();
      add_to_total_buffer_record(dibc);
      dibc->mark_loaded();

      _d3d_device->SetIndices(dibc->_ibuffer, 0);
      _active_ibuffer = dibc;
      add_to_index_buffer_record(dibc);

    } else {
      _d3d_device->SetIndices(NULL, 0);
      _active_ibuffer = NULL;
    }

  } else {
    if (dibc->was_modified()) {
      if (dibc->changed_size()) {
        // We have to destroy the old index buffer and create a new
        // one.
        dibc->create_ibuffer(*_screen);
      }

      dibc->upload_data();

      add_to_total_buffer_record(dibc);
      dibc->mark_loaded();
      _active_ibuffer = NULL;
    }

    if (_active_ibuffer != dibc) {
      _d3d_device->SetIndices(dibc->_ibuffer, 0);
      _active_ibuffer = dibc;
      add_to_index_buffer_record(dibc);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::release_index_buffer
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               data.  This function should never be called
//               directly; instead, call Data::release() (or simply
//               let the Data destruct).
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
release_index_buffer(IndexBufferContext *ibc) {
  DXIndexBufferContext8 *dibc = DCAST(DXIndexBufferContext8, ibc);
  delete dibc;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::make_geom_munger
//       Access: Public, Virtual
//  Description: Creates a new GeomMunger object to munge vertices
//               appropriate to this GSG for the indicated state.
////////////////////////////////////////////////////////////////////
PT(GeomMunger) DXGraphicsStateGuardian8::
make_geom_munger(const RenderState *state) {
  PT(DXGeomMunger8) munger = new DXGeomMunger8(this, state);
  return GeomMunger::register_munger(munger);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_color_clear_value
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
set_color_clear_value(const Colorf& value) {
  _color_clear_value = value;
  _d3dcolor_clear_value =  Colorf_to_D3DCOLOR(value);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_clear
//       Access: Public, Virtual
//  Description: Clears all of the indicated buffers to their assigned
//               colors.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_clear(const RenderBuffer &buffer) {
  nassertv(buffer._gsg == this);
  int buffer_type = buffer._buffer_type;

  DWORD main_flags = 0;
  DWORD aux_flags = 0;

  //set appropriate flags
  if (buffer_type & RenderBuffer::T_back) {
    main_flags |=  D3DCLEAR_TARGET;
  }

  if (buffer_type & RenderBuffer::T_depth) {
    aux_flags |=  D3DCLEAR_ZBUFFER;
    nassertv(_screen->_presentation_params.EnableAutoDepthStencil);
  }

  if (buffer_type & RenderBuffer::T_stencil) {
    aux_flags |=  D3DCLEAR_STENCIL;
    nassertv(_screen->_presentation_params.EnableAutoDepthStencil && IS_STENCIL_FORMAT(_screen->_presentation_params.AutoDepthStencilFormat));
  }

  if ((main_flags | aux_flags) != 0) {
    HRESULT hr = _d3d_device->Clear(0, NULL, main_flags | aux_flags, _d3dcolor_clear_value,
                                    _depth_clear_value, (DWORD)_stencil_clear_value);
    if (FAILED(hr) && main_flags == D3DCLEAR_TARGET && aux_flags != 0) {
      // Maybe there's a problem with the one or more of the auxiliary
      // buffers.
      hr = _d3d_device->Clear(0, NULL, D3DCLEAR_TARGET, _d3dcolor_clear_value,
                              _depth_clear_value, (DWORD)_stencil_clear_value);
      if (!FAILED(hr)) {
        // Yep, it worked without them.  That's a problem.  Which buffer
        // poses the problem?
        if (buffer_type & RenderBuffer::T_depth) {
          aux_flags |=  D3DCLEAR_ZBUFFER;
          HRESULT hr2 = _d3d_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, _d3dcolor_clear_value,
                                           _depth_clear_value, (DWORD)_stencil_clear_value);
          if (FAILED(hr2)) {
            dxgsg8_cat.error()
              << "Unable to clear depth buffer; removing.\n";
            _buffer_mask &= ~RenderBuffer::T_depth;
          }
        }
        if (buffer_type & RenderBuffer::T_stencil) {
          aux_flags |=  D3DCLEAR_STENCIL;
          HRESULT hr2 = _d3d_device->Clear(0, NULL, D3DCLEAR_STENCIL, _d3dcolor_clear_value,
                                           _stencil_clear_value, (DWORD)_stencil_clear_value);
          if (FAILED(hr2)) {
            dxgsg8_cat.error()
              << "Unable to clear stencil buffer; removing.\n";
            _buffer_mask &= ~RenderBuffer::T_stencil;
          }
        }
      }
    }

    if (FAILED(hr)) {
      dxgsg8_cat.error()
        << "clear_buffer failed:  Clear returned " << D3DERRORSTRING(hr);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::prepare_display_region
//       Access: Public, Virtual
//  Description: Prepare a display region for rendering (set up
//       scissor region and viewport)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
prepare_display_region() {
  if (_current_display_region == (DisplayRegion*)0L) {
    dxgsg8_cat.error()
      << "Invalid NULL display region in prepare_display_region()\n";

  } else if (_current_display_region != _actual_display_region) {
    _actual_display_region = _current_display_region;

    int l, u, w, h;
    _actual_display_region->get_region_pixels_i(l, u, w, h);

    // Create the viewport
    D3DVIEWPORT8 vp = { l, u, w, h, 0.0f, 1.0f };
    HRESULT hr = _d3d_device->SetViewport(&vp);
    if (FAILED(hr)) {
      dxgsg8_cat.error()
        << "_screen->_swap_chain = " << _screen->_swap_chain << " _swap_chain = " << _swap_chain << "\n";
      dxgsg8_cat.error()
        << "SetViewport(" << l << ", " << u << ", " << w << ", " << h
        << ") failed" << D3DERRORSTRING(hr);

      D3DVIEWPORT8 vp_old;
      _d3d_device->GetViewport(&vp_old);
      dxgsg8_cat.error()
        << "GetViewport(" << vp_old.X << ", " << vp_old.Y << ", " << vp_old.Width << ", "
        << vp_old.Height << ") returned: Trying to set that vp---->\n";
      hr = _d3d_device->SetViewport(&vp_old);

      if (FAILED(hr)) {
        dxgsg8_cat.error() << "Failed again\n";
        throw_event("panda3d-render-error");
        nassertv(false);
      }
    }
    // Note: for DX9, also change scissor clipping state here
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::prepare_lens
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
bool DXGraphicsStateGuardian8::
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
    _d3d_device->SetTransform(D3DTS_PROJECTION,
                              (D3DMATRIX*)_projection_mat.get_data());
  return SUCCEEDED(hr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::begin_frame
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
bool DXGraphicsStateGuardian8::
begin_frame() {
  return GraphicsStateGuardian::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::begin_scene
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
bool DXGraphicsStateGuardian8::
begin_scene() {
  if (!GraphicsStateGuardian::begin_scene()) {
    return false;
  }

  HRESULT hr = _d3d_device->BeginScene();

  if (FAILED(hr)) {
    if (hr == D3DERR_DEVICELOST) {
      if (dxgsg8_cat.is_debug()) {
        dxgsg8_cat.debug()
          << "BeginScene returns D3DERR_DEVICELOST" << endl;
      }

      check_cooperative_level();

    } else {
      dxgsg8_cat.error()
        << "BeginScene failed, unhandled error hr == "
        << D3DERRORSTRING(hr) << endl;
      throw_event("panda3d-render-error");
    }
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::end_scene
//       Access: Public, Virtual
//  Description: Called between begin_frame() and end_frame() to mark
//               the end of drawing commands for a "scene" (usually a
//               particular DisplayRegion) within a frame.  All 3-D
//               drawing commands, except the clear operation, must be
//               enclosed within begin_scene() .. end_scene().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
end_scene() {
  HRESULT hr = _d3d_device->EndScene();

  if (FAILED(hr)) {
    if (hr == D3DERR_DEVICELOST) {
      if (dxgsg8_cat.is_debug()) {
        dxgsg8_cat.debug()
          << "EndScene returns DeviceLost\n";
      }
      check_cooperative_level();

    } else {
      dxgsg8_cat.error()
        << "EndScene failed, unhandled error hr == " << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
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
void DXGraphicsStateGuardian8::
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
//     Function: DXGraphicsStateGuardian8::begin_draw_primitives
//       Access: Public, Virtual
//  Description: Called before a sequence of draw_primitive()
//               functions are called, this should prepare the vertex
//               data for rendering.  It returns true if the vertices
//               are ok, false to abort this group of primitives.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian8::
begin_draw_primitives(const Geom *geom, const GeomMunger *munger,
                      const GeomVertexData *vertex_data) {
  if (!GraphicsStateGuardian::begin_draw_primitives(geom, munger, vertex_data)) {
    return false;
  }
  nassertr(_vertex_data != (GeomVertexData *)NULL, false);

  const GeomVertexFormat *format = _vertex_data->get_format();

  // The munger should have put the FVF data in the first array.
  const GeomVertexArrayData *data = _vertex_data->get_array(0);

  VertexBufferContext *vbc = ((GeomVertexArrayData *)data)->prepare_now(get_prepared_objects(), this);
  nassertr(vbc != (VertexBufferContext *)NULL, false);
  apply_vertex_buffer(vbc);

  const GeomVertexAnimationSpec &animation = 
    vertex_data->get_format()->get_animation();
  if (animation.get_animation_type() == Geom::AT_hardware) {
    // Set up vertex blending.
    switch (animation.get_num_transforms()) {
    case 1:
      // The MSDN docs suggest we should use D3DVBF_0WEIGHTS here, but
      // that doesn't seem to work at all.  On the other hand,
      // D3DVBF_DISABLE *does* work, because it disables special
      // handling, meaning only the world matrix affects these
      // vertices--and by accident or design, the first matrix,
      // D3DTS_WORLDMATRIX(0), *is* the world matrix.
      _d3d_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
      break;
    case 2:
      _d3d_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);
      break;
    case 3:
      _d3d_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_2WEIGHTS);
      break;
    case 4:
      _d3d_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_3WEIGHTS);
      break;
    }

    if (animation.get_indexed_transforms()) {
      // Set up indexed vertex blending.
      _d3d_device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
    } else {
      _d3d_device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    }

    const TransformTable *table = vertex_data->get_transform_table();
    if (table != (TransformTable *)NULL) {
      for (int i = 0; i < table->get_num_transforms(); i++) {
        LMatrix4f mat;
        table->get_transform(i)->mult_matrix(mat, _internal_transform->get_mat());
        const D3DMATRIX *d3d_mat = (const D3DMATRIX *)mat.get_data();
        _d3d_device->SetTransform(D3DTS_WORLDMATRIX(i), d3d_mat);
      }

      // Setting the first animation matrix steps on the world matrix,
      // so we have to set a flag to reload the world matrix later.
      _transform_stale = true;
    }
    _vertex_blending_enabled = true;

  } else {
    // We're not using vertex blending.
    if (_vertex_blending_enabled) {
      _d3d_device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
      _d3d_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
      _vertex_blending_enabled = false;
    }

    if (_transform_stale && !_vertex_data->is_vertex_transformed()) {
      const D3DMATRIX *d3d_mat = (const D3DMATRIX *)_internal_transform->get_mat().get_data();
      _d3d_device->SetTransform(D3DTS_WORLD, d3d_mat);
      _transform_stale = false;
    }
  }

  if (_vertex_data->is_vertex_transformed()) {
    // If the vertex data claims to be already transformed into clip
    // coordinates, wipe out the current projection and modelview
    // matrix (so we don't attempt to transform it again).

    // It's tempting just to use the D3DFVF_XYZRHW specification on
    // these vertices, but that turns out to be a bigger hammer than
    // we want: that also prevents lighting calculations and user clip
    // planes.
    _d3d_device->SetTransform(D3DTS_WORLD, &_d3d_ident_mat);
    static const LMatrix4f rescale_mat
      (1, 0, 0, 0,
       0, 1, 0, 0,
       0, 0, 0.5, 0,
       0, 0, 0.5, 1);
    _transform_stale = true;

    _d3d_device->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX *)rescale_mat.get_data());
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::draw_triangles
//       Access: Public, Virtual
//  Description: Draws a series of disconnected triangles.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
draw_triangles(const GeomTriangles *primitive) {
  _vertices_tri_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_tri_pcollector.add_level(1);
  if (primitive->is_indexed()) {
    int min_vertex = dx_broken_max_index ? 0 : primitive->get_min_vertex();
    int max_vertex = primitive->get_max_vertex();

    if (_active_vbuffer != NULL) {
      // Indexed, vbuffers.
      IndexBufferContext *ibc = ((GeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
      nassertv(ibc != (IndexBufferContext *)NULL);
      apply_index_buffer(ibc);

      _d3d_device->DrawIndexedPrimitive
        (D3DPT_TRIANGLELIST,
         min_vertex, max_vertex - min_vertex + 1,
         0, primitive->get_num_primitives());

    } else {
      // Indexed, client arrays.
      D3DFORMAT index_type = get_index_type(primitive->get_index_type());

      _d3d_device->DrawIndexedPrimitiveUP
        (D3DPT_TRIANGLELIST,
         min_vertex, max_vertex - min_vertex + 1,
         primitive->get_num_primitives(),
         primitive->get_data(),
         index_type,
         _vertex_data->get_array(0)->get_data(),
         _vertex_data->get_format()->get_array(0)->get_stride());
    }
  } else {
    if (_active_vbuffer != NULL) {
      // Nonindexed, vbuffers.
      _d3d_device->DrawPrimitive
        (D3DPT_TRIANGLELIST,
         primitive->get_first_vertex(),
         primitive->get_num_primitives());

    } else {
      // Nonindexed, client arrays.
      int stride = _vertex_data->get_format()->get_array(0)->get_stride();
      unsigned int first_vertex = primitive->get_first_vertex();
      _d3d_device->DrawPrimitiveUP
        (D3DPT_TRIANGLELIST,
         primitive->get_num_primitives(),
         _vertex_data->get_array(0)->get_data() + stride * first_vertex,
         stride);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::draw_tristrips
//       Access: Public, Virtual
//  Description: Draws a series of triangle strips.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
draw_tristrips(const GeomTristrips *primitive) {
  if (connect_triangle_strips && _current_fill_mode != RenderModeAttrib::M_wireframe) {
    // One long triangle strip, connected by the degenerate vertices
    // that have already been set up within the primitive.
    _vertices_tristrip_pcollector.add_level(primitive->get_num_vertices());
    _primitive_batches_tristrip_pcollector.add_level(1);
    if (primitive->is_indexed()) {
      int min_vertex = dx_broken_max_index ? 0 : primitive->get_min_vertex();
      int max_vertex = primitive->get_max_vertex();

      if (_active_vbuffer != NULL) {
        // Indexed, vbuffers, one line triangle strip.
        IndexBufferContext *ibc = ((GeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
        nassertv(ibc != (IndexBufferContext *)NULL);
        apply_index_buffer(ibc);

        _d3d_device->DrawIndexedPrimitive
          (D3DPT_TRIANGLESTRIP,
           min_vertex, max_vertex - min_vertex + 1,
           0, primitive->get_num_vertices() - 2);

      } else {
        // Indexed, client arrays, one long triangle strip.
        D3DFORMAT index_type = get_index_type(primitive->get_index_type());
        _d3d_device->DrawIndexedPrimitiveUP
          (D3DPT_TRIANGLESTRIP,
           min_vertex, max_vertex - min_vertex + 1,
           primitive->get_num_vertices() - 2,
           primitive->get_data(), index_type,
           _vertex_data->get_array(0)->get_data(),
           _vertex_data->get_format()->get_array(0)->get_stride());
      }
    } else {
      if (_active_vbuffer != NULL) {
        // Nonindexed, vbuffers, one long triangle strip.
        _d3d_device->DrawPrimitive
          (D3DPT_TRIANGLESTRIP,
           primitive->get_first_vertex(),
           primitive->get_num_vertices() - 2);

      } else {
        // Indexed, client arrays, one long triangle strip.
        int stride = _vertex_data->get_format()->get_array(0)->get_stride();
        unsigned int first_vertex = primitive->get_first_vertex();
        _d3d_device->DrawPrimitiveUP
          (D3DPT_TRIANGLESTRIP,
           primitive->get_num_vertices() - 2,
           _vertex_data->get_array(0)->get_data() + stride * first_vertex,
           stride);
      }
    }

  } else {
    // Send the individual triangle strips, stepping over the
    // degenerate vertices.
    CPTA_int ends = primitive->get_ends();
    _primitive_batches_tristrip_pcollector.add_level(ends.size());

    if (primitive->is_indexed()) {
      CPTA_int ends = primitive->get_ends();
      int index_stride = primitive->get_index_stride();
      _primitive_batches_tristrip_pcollector.add_level(ends.size());

      GeomVertexReader mins(primitive->get_mins(), 0);
      GeomVertexReader maxs(primitive->get_maxs(), 0);
      nassertv(primitive->get_mins()->get_num_rows() == (int)ends.size() &&
               primitive->get_maxs()->get_num_rows() == (int)ends.size());

      if (_active_vbuffer != NULL) {
        // Indexed, vbuffers, individual triangle strips.
        IndexBufferContext *ibc = ((GeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
        nassertv(ibc != (IndexBufferContext *)NULL);
        apply_index_buffer(ibc);

        unsigned int start = 0;
        for (size_t i = 0; i < ends.size(); i++) {
          _vertices_tristrip_pcollector.add_level(ends[i] - start);
          unsigned int min = mins.get_data1i();
          unsigned int max = maxs.get_data1i();
          _d3d_device->DrawIndexedPrimitive
            (D3DPT_TRIANGLESTRIP,
             min, max - min + 1,
             start, ends[i] - start - 2);

          start = ends[i] + 2;
        }

      } else {
        // Indexed, client arrays, individual triangle strips.
        CPTA_uchar array_data = _vertex_data->get_array(0)->get_data();
        int stride = _vertex_data->get_format()->get_array(0)->get_stride();
        CPTA_uchar vertices = primitive->get_data();
        D3DFORMAT index_type = get_index_type(primitive->get_index_type());

        unsigned int start = 0;
        for (size_t i = 0; i < ends.size(); i++) {
          _vertices_tristrip_pcollector.add_level(ends[i] - start);
          unsigned int min = mins.get_data1i();
          unsigned int max = maxs.get_data1i();
          _d3d_device->DrawIndexedPrimitiveUP
            (D3DPT_TRIANGLESTRIP,
             min, max - min + 1,
             ends[i] - start - 2,
             vertices + start * index_stride, index_type,
             array_data, stride);

          start = ends[i] + 2;
        }
      }
    } else {
      unsigned int first_vertex = primitive->get_first_vertex();

      if (_active_vbuffer != NULL) {
        // Nonindexed, vbuffers, individual triangle strips.
        unsigned int start = 0;
        for (size_t i = 0; i < ends.size(); i++) {
          _vertices_tristrip_pcollector.add_level(ends[i] - start);
          _d3d_device->DrawPrimitive
            (D3DPT_TRIANGLESTRIP,
             first_vertex + start, ends[i] - start - 2);

          start = ends[i] + 2;
        }

      } else {
        // Nonindexed, client arrays, individual triangle strips.
        CPTA_uchar array_data = _vertex_data->get_array(0)->get_data();
        int stride = _vertex_data->get_format()->get_array(0)->get_stride();

        unsigned int start = 0;
        for (size_t i = 0; i < ends.size(); i++) {
          _vertices_tristrip_pcollector.add_level(ends[i] - start);
          _d3d_device->DrawPrimitiveUP
            (D3DPT_TRIANGLESTRIP,
             ends[i] - start - 2,
             array_data + (first_vertex + start) * stride, stride);

          start = ends[i] + 2;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::draw_trifans
//       Access: Public, Virtual
//  Description: Draws a series of triangle fans.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
draw_trifans(const GeomTrifans *primitive) {
  CPTA_int ends = primitive->get_ends();
  _primitive_batches_trifan_pcollector.add_level(ends.size());

  if (primitive->is_indexed()) {
    int min_vertex = dx_broken_max_index ? 0 : primitive->get_min_vertex();
    int max_vertex = primitive->get_max_vertex();

    // Send the individual triangle fans.  There's no connecting fans
    // with degenerate vertices, so no worries about that.
    int index_stride = primitive->get_index_stride();

    GeomVertexReader mins(primitive->get_mins(), 0);
    GeomVertexReader maxs(primitive->get_maxs(), 0);
    nassertv(primitive->get_mins()->get_num_rows() == (int)ends.size() &&
             primitive->get_maxs()->get_num_rows() == (int)ends.size());

    if (_active_vbuffer != NULL) {
      // Indexed, vbuffers.
      IndexBufferContext *ibc = ((GeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
      nassertv(ibc != (IndexBufferContext *)NULL);
      apply_index_buffer(ibc);

      unsigned int start = 0;
      for (size_t i = 0; i < ends.size(); i++) {
        _vertices_trifan_pcollector.add_level(ends[i] - start);
        unsigned int min = mins.get_data1i();
        unsigned int max = maxs.get_data1i();
        _d3d_device->DrawIndexedPrimitive
          (D3DPT_TRIANGLEFAN,
           min, max - min + 1,
           start, ends[i] - start - 2);

        start = ends[i];
      }

    } else {
      // Indexed, client arrays.
      CPTA_uchar array_data = _vertex_data->get_array(0)->get_data();
      int stride = _vertex_data->get_format()->get_array(0)->get_stride();
      CPTA_uchar vertices = primitive->get_data();
      D3DFORMAT index_type = get_index_type(primitive->get_index_type());

      unsigned int start = 0;
      for (size_t i = 0; i < ends.size(); i++) {
        _vertices_trifan_pcollector.add_level(ends[i] - start);
        unsigned int min = mins.get_data1i();
        unsigned int max = maxs.get_data1i();
        _d3d_device->DrawIndexedPrimitiveUP
          (D3DPT_TRIANGLEFAN,
           min, max - min + 1,
           ends[i] - start - 2,
           vertices + start * index_stride, index_type,
           array_data, stride);

        start = ends[i];
      }
    }
  } else {
    unsigned int first_vertex = primitive->get_first_vertex();

    if (_active_vbuffer != NULL) {
      // Nonindexed, vbuffers.
      unsigned int start = 0;
      for (size_t i = 0; i < ends.size(); i++) {
        _vertices_trifan_pcollector.add_level(ends[i] - start);
        _d3d_device->DrawPrimitive
          (D3DPT_TRIANGLEFAN,
           first_vertex + start, ends[i] - start - 2);

        start = ends[i];
      }

    } else {
      // Nonindexed, client arrays.
      CPTA_uchar array_data = _vertex_data->get_array(0)->get_data();
      int stride = _vertex_data->get_format()->get_array(0)->get_stride();

      unsigned int start = 0;
      for (size_t i = 0; i < ends.size(); i++) {
        _vertices_trifan_pcollector.add_level(ends[i] - start);
        _d3d_device->DrawPrimitiveUP
          (D3DPT_TRIANGLEFAN,
           ends[i] - start - 2,
           array_data + (first_vertex + start) * stride, stride);

        start = ends[i];
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::draw_lines
//       Access: Public, Virtual
//  Description: Draws a series of disconnected line segments.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
draw_lines(const GeomLines *primitive) {
  _vertices_other_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_other_pcollector.add_level(1);

  if (primitive->is_indexed()) {
    int min_vertex = dx_broken_max_index ? 0 : primitive->get_min_vertex();
    int max_vertex = primitive->get_max_vertex();

    if (_active_vbuffer != NULL) {
      // Indexed, vbuffers.
      IndexBufferContext *ibc = ((GeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
      nassertv(ibc != (IndexBufferContext *)NULL);
      apply_index_buffer(ibc);

      _d3d_device->DrawIndexedPrimitive
        (D3DPT_LINELIST,
         min_vertex, max_vertex - min_vertex + 1,
         0, primitive->get_num_primitives());

    } else {
      // Indexed, client arrays.
      D3DFORMAT index_type = get_index_type(primitive->get_index_type());

      _d3d_device->DrawIndexedPrimitiveUP
        (D3DPT_LINELIST,
         min_vertex, max_vertex - min_vertex + 1,
         primitive->get_num_primitives(),
         primitive->get_data(),
         index_type,
         _vertex_data->get_array(0)->get_data(),
         _vertex_data->get_format()->get_array(0)->get_stride());
    }
  } else {
    if (_active_vbuffer != NULL) {
      // Nonindexed, vbuffers.
      _d3d_device->DrawPrimitive
        (D3DPT_LINELIST,
         primitive->get_first_vertex(),
         primitive->get_num_primitives());

    } else {
      // Nonindexed, client arrays.
      int stride = _vertex_data->get_format()->get_array(0)->get_stride();
      unsigned int first_vertex = primitive->get_first_vertex();
      _d3d_device->DrawPrimitiveUP
        (D3DPT_LINELIST,
         primitive->get_num_primitives(),
         _vertex_data->get_array(0)->get_data() + stride * first_vertex,
         stride);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::draw_linestrips
//       Access: Public, Virtual
//  Description: Draws a series of line strips.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
draw_linestrips(const GeomLinestrips *primitive) {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::draw_points
//       Access: Public, Virtual
//  Description: Draws a series of disconnected points.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
draw_points(const GeomPoints *primitive) {
  _vertices_other_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_other_pcollector.add_level(1);

  // The munger should have protected us from indexed points--DirectX
  // doesn't support them.
  nassertv(!primitive->is_indexed());

  if (_active_vbuffer != NULL) {
    // Nonindexed, vbuffers.
    _d3d_device->DrawPrimitive
      (D3DPT_POINTLIST,
       primitive->get_first_vertex(),
       primitive->get_num_primitives());

  } else {
    // Nonindexed, client arrays.
    int stride = _vertex_data->get_format()->get_array(0)->get_stride();
    unsigned int first_vertex = primitive->get_first_vertex();
    _d3d_device->DrawPrimitiveUP
      (D3DPT_POINTLIST,
       primitive->get_num_primitives(),
       _vertex_data->get_array(0)->get_data() + stride * first_vertex,
       stride);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::end_draw_primitives()
//       Access: Public, Virtual
//  Description: Called after a sequence of draw_primitive()
//               functions are called, this should do whatever cleanup
//               is appropriate.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
end_draw_primitives() {
  // Turn off vertex blending--it seems to cause problems if we leave
  // it on.
  if (_vertex_blending_enabled) {
    _d3d_device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    _d3d_device->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    _vertex_blending_enabled = false;
  }

  if (_vertex_data->is_vertex_transformed()) {
    // Restore the projection matrix that we wiped out above.
    prepare_lens();
  }

  GraphicsStateGuardian::end_draw_primitives();
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::framebuffer_copy_to_texture
//       Access: Public, Virtual
//  Description: Copy the pixels within the indicated display
//               region from the framebuffer into texture memory.
//
//               If z > -1, it is the cube map index into which to
//               copy.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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
  DXTextureContext8 *dtc = DCAST(DXTextureContext8, tc);

  nassertv(tex->get_texture_type() == Texture::TT_2d_texture);
  nassertv(dtc->get_d3d_2d_texture() != NULL);

  IDirect3DSurface8 *tex_level_0, *render_target;
  hr = dtc->get_d3d_2d_texture()->GetSurfaceLevel(0, &tex_level_0);
  if (FAILED(hr)) {
    dxgsg8_cat.error() << "GetSurfaceLev failed in copy_texture" << D3DERRORSTRING(hr);
    return;
  }

  hr = _d3d_device->GetRenderTarget(&render_target);
  if (FAILED(hr)) {
    dxgsg8_cat.error() << "GetRenderTgt failed in copy_texture" << D3DERRORSTRING(hr);
    SAFE_RELEASE(tex_level_0);
    return;
  }

  RECT src_rect;

  src_rect.left = xo;
  src_rect.right = xo+w;
  src_rect.top = yo;
  src_rect.bottom = yo+h;

  // now copy from fb to tex
  hr = _d3d_device->CopyRects(render_target, &src_rect, 1, tex_level_0, 0);
  if (FAILED(hr)) {
    dxgsg8_cat.error()
      << "CopyRects failed in copy_texture" << D3DERRORSTRING(hr);
  }

  SAFE_RELEASE(render_target);
  SAFE_RELEASE(tex_level_0);
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
bool DXGraphicsStateGuardian8::
framebuffer_copy_to_ram(Texture *tex, int z, const DisplayRegion *dr, const RenderBuffer &rb) {
  set_read_buffer(rb);

  RECT rect;
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

  Texture::TextureType texture_type;
  if (z >= 0) {
    texture_type = Texture::TT_cube_map;
  } else {
    texture_type = Texture::TT_2d_texture;
  }

  if (tex->get_x_size() != w || tex->get_y_size() != h ||
      tex->get_component_type() != component_type ||
      tex->get_format() != format ||
      tex->get_texture_type() != texture_type) {
    // Re-setup the texture; its properties have changed.
    tex->setup_texture(texture_type, w, h, tex->get_z_size(), 
                       component_type, format);
  }

  rect.top = yo;
  rect.left = xo;
  rect.right = xo + w;
  rect.bottom = yo + h;
  bool copy_inverted = false;

  IDirect3DSurface8 *temp_surface = NULL;
  HRESULT hr;

  // Note if you try to grab the backbuffer and full-screen
  // anti-aliasing is on, the backbuffer might be larger than the
  // window size.  For screenshots it's safer to get the front buffer.
  if (_cur_read_pixel_buffer & RenderBuffer::T_back) {
    IDirect3DSurface8 *backbuffer = NULL;
    // GetRenderTarget() seems to be a little more reliable than
    // GetBackBuffer().  Might just be related to the swap_chain
    // thing.
    hr = _d3d_device->GetRenderTarget(&backbuffer);

    if (FAILED(hr)) {
      dxgsg8_cat.error() << "GetRenderTarget failed" << D3DERRORSTRING(hr);
      return false;
    }

    // Since we might not be able to Lock the back buffer, we will
    // need to copy it to a temporary surface of the appropriate type
    // first.
    hr = _d3d_device->CreateImageSurface(w, h, _screen->_display_mode.Format, 
					 &temp_surface);
    if (FAILED(hr)) {
      dxgsg8_cat.error()
	<< "CreateImageSurface failed in copy_pixel_buffer()"
	<< D3DERRORSTRING(hr);
      backbuffer->Release();
      return false;
    }

    // Now we must copy from the backbuffer to our temporary surface.
    hr = _d3d_device->CopyRects(backbuffer, &rect, 1, temp_surface, NULL);
    if (FAILED(hr)) {
      dxgsg8_cat.error() << "CopyRects failed" << D3DERRORSTRING(hr);
      temp_surface->Release();
      backbuffer->Release();
      return false;
    }

    RELEASE(backbuffer, dxgsg8, "backbuffer", RELEASE_ONCE);

  } else if (_cur_read_pixel_buffer & RenderBuffer::T_front) {

    if (_screen->_presentation_params.Windowed) {
      // GetFrontBuffer() retrieves the entire desktop for a monitor,
      // so we need to reserve space for that.
      
      // We have to use GetMonitorInfo(), since this GSG may not be
      // for the primary monitor.
      MONITORINFO minfo;
      minfo.cbSize = sizeof(MONITORINFO);
      GetMonitorInfo(_screen->_monitor, &minfo);
      
      w = RECT_XSIZE(minfo.rcMonitor);
      h = RECT_YSIZE(minfo.rcMonitor);
      
      // set rect to client area of window in scrn coords
      ClientToScreen(_screen->_window, (POINT*)&rect.left);
      ClientToScreen(_screen->_window, (POINT*)&rect.right);
    }
    
    // For GetFrontBuffer(), we need a temporary surface of type
    // A8R8G8B8.  Unlike GetBackBuffer(), GetFrontBuffer() implicitly
    // performs a copy.
    hr = _d3d_device->CreateImageSurface(w, h, D3DFMT_A8R8G8B8, &temp_surface);
    if (FAILED(hr)) {
      dxgsg8_cat.error()
	<< "CreateImageSurface failed in copy_pixel_buffer()"
	<< D3DERRORSTRING(hr);
      return false;
    }

    hr = _d3d_device->GetFrontBuffer(temp_surface);

    if (hr == D3DERR_DEVICELOST) {
      dxgsg8_cat.error()
	<< "copy_pixel_buffer failed: device lost\n";
      temp_surface->Release();
      return false;
    }

    // For some reason the front buffer comes out inverted, but the
    // back buffer does not.
    copy_inverted = true;

  } else {
    dxgsg8_cat.error()
      << "copy_pixel_buffer: unhandled current_read_pixel_buffer type\n";
    temp_surface->Release();
    return false;
  }

  DXTextureContext8::d3d_surface_to_texture(rect, temp_surface, 
					    copy_inverted, tex, z);

  RELEASE(temp_surface, dxgsg8, "temp_surface", RELEASE_ONCE);

  nassertr(tex->has_ram_image(), false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.  The GraphicsWindow pointer represents a
//               typical window that might be used for this context;
//               it may be required to set up the frame buffer
//               properly the first time.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
reset() {
  GraphicsStateGuardian::reset();

  _auto_rescale_normal = false;

  // overwrite gsg defaults with these values

  // We always have at least a color buffer (the depth and/or stencil
  // buffer flags will be filled in by the window).
  _buffer_mask = RenderBuffer::T_color;

  HRESULT hr;

  // make sure gsg passes all current state down to us
  // set_state_and_transform(RenderState::make_empty(),
  // TransformState::make_identity());
  // want gsg to pass all state settings down so any non-matching defaults we set here get overwritten

  assert(_screen->_d3d8 != NULL);
  assert(_d3d_device != NULL);

  D3DCAPS8 d3d_caps;
  _d3d_device->GetDeviceCaps(&d3d_caps);

  if (dxgsg8_cat.is_debug()) {
    dxgsg8_cat.debug()
      << "\nHwTransformAndLight = " << ((d3d_caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
      << "\nMaxTextureWidth = " << d3d_caps.MaxTextureWidth
      << "\nMaxTextureHeight = " << d3d_caps.MaxTextureHeight
      << "\nMaxVolumeExtent = " << d3d_caps.MaxVolumeExtent
      << "\nMaxTextureAspectRatio = " << d3d_caps.MaxTextureAspectRatio
      << "\nTexCoordCount = " << (d3d_caps.FVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK)
      << "\nMaxTextureBlendStages = " << d3d_caps.MaxTextureBlendStages
      << "\nMaxSimultaneousTextures = " << d3d_caps.MaxSimultaneousTextures
      << "\nMaxActiveLights = " << d3d_caps.MaxActiveLights
      << "\nMaxUserClipPlanes = " << d3d_caps.MaxUserClipPlanes
      << "\nMaxVertexBlendMatrices = " << d3d_caps.MaxVertexBlendMatrices
      << "\nMaxVertexBlendMatrixIndex = " << d3d_caps.MaxVertexBlendMatrixIndex
      << "\nMaxPointSize = " << d3d_caps.MaxPointSize
      << "\nMaxPrimitiveCount = " << d3d_caps.MaxPrimitiveCount
      << "\nMaxVertexIndex = " << d3d_caps.MaxVertexIndex
      << "\nMaxStreams = " << d3d_caps.MaxStreams
      << "\nMaxStreamStride = " << d3d_caps.MaxStreamStride
      << "\nD3DTEXOPCAPS_MULTIPLYADD = " << ((d3d_caps.TextureOpCaps & D3DTEXOPCAPS_MULTIPLYADD) != 0)
      << "\nD3DTEXOPCAPS_LERP = " << ((d3d_caps.TextureOpCaps & D3DTEXOPCAPS_LERP) != 0)
      << "\nD3DPMISCCAPS_TSSARGTEMP = " << ((d3d_caps.PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP) != 0)
      << "\n";
  }

  _max_vertices_per_array = d3d_caps.MaxVertexIndex;
  _max_vertices_per_primitive = d3d_caps.MaxPrimitiveCount;

  _max_texture_stages = d3d_caps.MaxSimultaneousTextures;

  _max_texture_dimension = min(d3d_caps.MaxTextureWidth, d3d_caps.MaxTextureHeight);

  _supports_texture_combine = ((d3d_caps.TextureOpCaps & D3DTEXOPCAPS_LERP) != 0);
  _supports_texture_saved_result = ((d3d_caps.PrimitiveMiscCaps & D3DPMISCCAPS_TSSARGTEMP) != 0);
  _supports_texture_dot3 = true;

  _supports_3d_texture = ((d3d_caps.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP) != 0);
  if (_supports_3d_texture) {
    _max_3d_texture_dimension = d3d_caps.MaxVolumeExtent;
  }
  _supports_cube_map = ((d3d_caps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) != 0);
  if (_supports_cube_map) {
    _max_cube_map_dimension = _max_texture_dimension;
  }

  _max_lights = (int)d3d_caps.MaxActiveLights;
  _max_clip_planes = (int)d3d_caps.MaxUserClipPlanes;
  _max_vertex_transforms = d3d_caps.MaxVertexBlendMatrices;
  _max_vertex_transform_indices = d3d_caps.MaxVertexBlendMatrixIndex;

  ZeroMemory(&_lmodel_ambient, sizeof(Colorf));
  _d3d_device->SetRenderState(D3DRS_AMBIENT, 0x0);

  _clip_plane_bits = 0;
  _d3d_device->SetRenderState(D3DRS_CLIPPLANEENABLE , 0x0);

  _d3d_device->SetRenderState(D3DRS_CLIPPING, true);

  // these both reflect d3d defaults
  _color_writemask = 0xFFFFFFFF;
  _cur_fvf_type = 0x0;

  _d3d_device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

  _depth_test_enabled = true;
  _d3d_device->SetRenderState(D3DRS_ZWRITEENABLE, _depth_test_enabled);

  _d3d_device->SetRenderState(D3DRS_EDGEANTIALIAS, false);

  _color_material_enabled = false;

  _depth_test_enabled = D3DZB_FALSE;
  _d3d_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

  _blend_enabled = false;
  _d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, (DWORD)_blend_enabled);

  // just use whatever d3d defaults to here
  _d3d_device->GetRenderState(D3DRS_SRCBLEND, (DWORD*)&_blend_source_func);
  _d3d_device->GetRenderState(D3DRS_DESTBLEND, (DWORD*)&_blend_dest_func);

  _fog_enabled = false;
  _d3d_device->SetRenderState(D3DRS_FOGENABLE, _fog_enabled);

  _projection_mat = LMatrix4f::ident_mat();
  _has_scene_graph_color = false;

  _last_testcooplevel_result = D3D_OK;

  for(int i = 0; i < MAX_POSSIBLE_TEXFMTS; i++) {
    // look for all possible DX8 texture fmts
    D3DFORMAT_FLAG fmtflag = D3DFORMAT_FLAG(1 << i);
    hr = _screen->_d3d8->CheckDeviceFormat(_screen->_card_id, D3DDEVTYPE_HAL, _screen->_display_mode.Format,
                                          0x0, D3DRTYPE_TEXTURE, g_D3DFORMATmap[fmtflag]);
    if (SUCCEEDED(hr)){
      _screen->_supported_tex_formats_mask |= fmtflag;
    }
  }

  // s3 virge drivers sometimes give crap values for these
  if (_screen->_d3dcaps.MaxTextureWidth == 0)
    _screen->_d3dcaps.MaxTextureWidth = 256;

  if (_screen->_d3dcaps.MaxTextureHeight == 0)
    _screen->_d3dcaps.MaxTextureHeight = 256;

  if (_screen->_d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGTABLE) {
    // Watch out for drivers that emulate per-pixel fog with
    // per-vertex fog (Riva128, Matrox Millen G200).  Some of these
    // require gouraud-shading to be set to work, as if you were using
    // vertex fog
    _do_fog_type = PerPixelFog;
  } else {
    // every card is going to have vertex fog, since it's implemented
    // in d3d runtime.
    assert((_screen->_d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX) != 0);

    // vertex fog may look crappy if you have large polygons in the
    // foreground and they get clipped, so you may want to disable it

    if (dx_no_vertex_fog) {
      _do_fog_type = None;
    } else {
      _do_fog_type = PerVertexFog;

      // range-based fog only works with vertex fog in dx7/8
      if (dx_use_rangebased_fog && (_screen->_d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGRANGE)) {
        _d3d_device->SetRenderState(D3DRS_RANGEFOGENABLE, true);
      }
    }
  }

  _screen->_can_direct_disable_color_writes = ((_screen->_d3dcaps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE) != 0);

  // Lighting, let's turn it off initially.
  _d3d_device->SetRenderState(D3DRS_LIGHTING, false);

  // turn on dithering if the rendertarget is < 8bits/color channel
  _dither_enabled = ((!dx_no_dithering) && IS_16BPP_DISPLAY_FORMAT(_screen->_presentation_params.BackBufferFormat)
                     && (_screen->_d3dcaps.RasterCaps & D3DPRASTERCAPS_DITHER));
  _d3d_device->SetRenderState(D3DRS_DITHERENABLE, _dither_enabled);

  _d3d_device->SetRenderState(D3DRS_CLIPPING, true);

  // Stencil test is off by default
  _d3d_device->SetRenderState(D3DRS_STENCILENABLE, FALSE);

  // Antialiasing.
  _d3d_device->SetRenderState(D3DRS_EDGEANTIALIAS, FALSE);

  _current_fill_mode = RenderModeAttrib::M_filled;
  _d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

  // must do SetTSS here because redundant states are filtered out by
  // our code based on current values above, so initial conditions
  // must be correct
  _texturing_enabled = false;
  _d3d_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);  // disables texturing

  _cull_face_mode = CullFaceAttrib::M_cull_none;
  _d3d_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  _alpha_func = D3DCMP_ALWAYS;
  _alpha_func_refval = 1.0f;
  _d3d_device->SetRenderState(D3DRS_ALPHAFUNC, _alpha_func);
  _d3d_device->SetRenderState(D3DRS_ALPHAREF, (UINT)(_alpha_func_refval*255.0f));
  _alpha_test_enabled = false;
  _d3d_device->SetRenderState(D3DRS_ALPHATESTENABLE, _alpha_test_enabled);

  // this is a new DX8 state that lets you do additional operations other than ADD (e.g. subtract/max/min)
  // must check (_screen->_d3dcaps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP) (yes on GF2/Radeon8500, no on TNT)
  _d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

  PRINT_REFCNT(dxgsg8, _d3d_device);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::apply_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
apply_fog(Fog *fog) {
  if (_do_fog_type == None)
    return;

  Fog::Mode panda_fogmode = fog->get_mode();
  D3DFOGMODE d3dfogmode = get_fog_mode_type(panda_fogmode);

  _d3d_device->SetRenderState((D3DRENDERSTATETYPE)_do_fog_type, d3dfogmode);

  const Colorf &fog_colr = fog->get_color();
  _d3d_device->SetRenderState(D3DRS_FOGCOLOR,
                              MY_D3DRGBA(fog_colr[0], fog_colr[1], fog_colr[2], 0.0f));  // Alpha bits are not used

  // do we need to adjust fog start/end values based on D3DPRASTERCAPS_WFOG/D3DPRASTERCAPS_ZFOG ?
  // if not WFOG, then docs say we need to adjust values to range [0, 1]

  switch (panda_fogmode) {
  case Fog::M_linear:
    {
      float onset, opaque;
      fog->get_linear_range(onset, opaque);

      _d3d_device->SetRenderState(D3DRS_FOGSTART,
                                   *((LPDWORD) (&onset)));
      _d3d_device->SetRenderState(D3DRS_FOGEND,
                                   *((LPDWORD) (&opaque)));
    }
    break;
  case Fog::M_exponential:
  case Fog::M_exponential_squared:
    {
      // Exponential fog is always camera-relative.
      float fog_density = fog->get_exp_density();
      _d3d_device->SetRenderState(D3DRS_FOGDENSITY,
                                   *((LPDWORD) (&fog_density)));
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_transform
//       Access: Protected
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
//
//               This transform is the internal_transform, already
//               converted into the GSG's internal coordinate system.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_transform() {
  const TransformState *transform = _internal_transform;
  DO_PSTATS_STUFF(_transform_state_pcollector.add_level(1));

  const D3DMATRIX *d3d_mat = (const D3DMATRIX *)transform->get_mat().get_data();
  _d3d_device->SetTransform(D3DTS_WORLD, d3d_mat);
  _transform_stale = false;

  if (_auto_rescale_normal) {
    do_auto_rescale_normal();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_alpha_test
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_alpha_test() {
  const AlphaTestAttrib *attrib = _target._alpha_test;
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
//     Function: DXGraphicsStateGuardian8::do_issue_render_mode
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_render_mode() {
  const RenderModeAttrib *attrib = _target._render_mode;
  RenderModeAttrib::Mode mode = attrib->get_mode();

  switch (mode) {
  case RenderModeAttrib::M_unchanged:
  case RenderModeAttrib::M_filled:
    _d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    break;

  case RenderModeAttrib::M_wireframe:
    _d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    break;

  case RenderModeAttrib::M_point:
    _d3d_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
    break;

  default:
    dxgsg8_cat.error()
      << "Unknown render mode " << (int)mode << endl;
  }

  // This might also specify the point size.
  float point_size = attrib->get_thickness();
  _d3d_device->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&point_size));

  if (attrib->get_perspective()) {
    _d3d_device->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);

    LVector3f height(0.0f, point_size, 1.0f);
    height = height * _projection_mat;
    float s = height[1] / point_size;

    float zero = 0.0f;
    float one_over_s2 = 1.0f / (s * s);
    _d3d_device->SetRenderState(D3DRS_POINTSCALE_A, *((DWORD*)&zero));
    _d3d_device->SetRenderState(D3DRS_POINTSCALE_B, *((DWORD*)&zero));
    _d3d_device->SetRenderState(D3DRS_POINTSCALE_C, *((DWORD*)&one_over_s2));

  } else {
    _d3d_device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
  }

  _current_fill_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_rescale_normal
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_rescale_normal() {
  const RescaleNormalAttrib *attrib = _target._rescale_normal;
  RescaleNormalAttrib::Mode mode = attrib->get_mode();

  _auto_rescale_normal = false;

  switch (mode) {
  case RescaleNormalAttrib::M_none:
    _d3d_device->SetRenderState(D3DRS_NORMALIZENORMALS, false);
    break;

  case RescaleNormalAttrib::M_rescale:
  case RescaleNormalAttrib::M_normalize:
    _d3d_device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
    break;

  case RescaleNormalAttrib::M_auto:
    _auto_rescale_normal = true;
    do_auto_rescale_normal();
    break;

  default:
    dxgsg8_cat.error()
      << "Unknown rescale_normal mode " << (int)mode << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_depth_test
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_depth_test() {
  const DepthTestAttrib *attrib = _target._depth_test;
  DepthTestAttrib::PandaCompareFunc mode = attrib->get_mode();
  if (mode == DepthTestAttrib::M_none) {
    _depth_test_enabled = false;
    _d3d_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
  } else {
    _depth_test_enabled = true;
    _d3d_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    _d3d_device->SetRenderState(D3DRS_ZFUNC, (D3DCMPFUNC) mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_depth_write
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_depth_write() {
  const DepthWriteAttrib *attrib = _target._depth_write;
  enable_zwritemask(attrib->get_mode() == DepthWriteAttrib::M_on);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_cull_face
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_cull_face() {
  const CullFaceAttrib *attrib = _target._cull_face;
  _cull_face_mode = attrib->get_effective_mode();

  switch (_cull_face_mode) {
  case CullFaceAttrib::M_cull_none:
    _d3d_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    break;
  case CullFaceAttrib::M_cull_clockwise:
    _d3d_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    break;
  case CullFaceAttrib::M_cull_counter_clockwise:
    _d3d_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    break;
  default:
    dxgsg8_cat.error()
      << "invalid cull face mode " << (int)_cull_face_mode << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_fog
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_fog() {
  const FogAttrib *attrib = _target._fog;
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
//     Function: DXGraphicsStateGuardian8::do_issue_depth_offset
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_depth_offset() {
  const DepthOffsetAttrib *attrib = _target._depth_offset;
  int offset = attrib->get_offset();
  _d3d_device->SetRenderState(D3DRS_ZBIAS, offset);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_shade_model
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_shade_model() {
  const ShadeModelAttrib *attrib = _target._shade_model;
  switch (attrib->get_mode()) {
  case ShadeModelAttrib::M_smooth:
    _d3d_device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    break;

  case ShadeModelAttrib::M_flat:
    _d3d_device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_state_and_transform
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
void DXGraphicsStateGuardian8::
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
    // Antialias not implemented under DX8
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
  
  // Shaders not implemented under DX8
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
//     Function: DXGraphicsStateGuardian8::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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
  D3DLIGHT8 alight;
  alight.Type =  D3DLIGHT_POINT;
  alight.Diffuse  = get_light_color(light_obj);
  alight.Ambient  =  black ;
  alight.Specular = *(D3DCOLORVALUE *)(light_obj->get_specular_color().get_data());

  // Position needs to specify x, y, z, and w
  // w == 1 implies non-infinite position
  alight.Position = *(D3DVECTOR *)pos.get_data();

  alight.Range =  __D3DLIGHT_RANGE_MAX;
  alight.Falloff =  1.0f;

  const LVecBase3f &att = light_obj->get_attenuation();
  alight.Attenuation0 = att[0];
  alight.Attenuation1 = att[1];
  alight.Attenuation2 = att[2];

  HRESULT hr = _d3d_device->SetLight(light_id, &alight);
  if (FAILED(hr)) {
    wdxdisplay8_cat.warning()
      << "Could not set light properties for " << light
      << " to id " << light_id << ": " << D3DERRORSTRING(hr) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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

  D3DLIGHT8 alight;
  ZeroMemory(&alight, sizeof(D3DLIGHT8));

  alight.Type =  D3DLIGHT_DIRECTIONAL;
  alight.Diffuse  = get_light_color(light_obj);
  alight.Ambient  =  black ;
  alight.Specular = *(D3DCOLORVALUE *)(light_obj->get_specular_color().get_data());

  alight.Direction = *(D3DVECTOR *)dir.get_data();

  alight.Range =  __D3DLIGHT_RANGE_MAX;
  alight.Falloff =  1.0f;

  alight.Attenuation0 = 1.0f;       // constant
  alight.Attenuation1 = 0.0f;       // linear
  alight.Attenuation2 = 0.0f;       // quadratic

  HRESULT hr = _d3d_device->SetLight(light_id, &alight);
  if (FAILED(hr)) {
    wdxdisplay8_cat.warning()
      << "Could not set light properties for " << light
      << " to id " << light_id << ": " << D3DERRORSTRING(hr) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::bind_light
//       Access: Public, Virtual
//  Description: Called the first time a particular light has been
//               bound to a given id within a frame, this should set
//               up the associated hardware light with the light's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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

  D3DLIGHT8  alight;
  ZeroMemory(&alight, sizeof(D3DLIGHT8));

  alight.Type =  D3DLIGHT_SPOT;
  alight.Ambient  =  black ;
  alight.Diffuse  = get_light_color(light_obj);
  alight.Specular = *(D3DCOLORVALUE *)(light_obj->get_specular_color().get_data());

  alight.Position = *(D3DVECTOR *)pos.get_data();

  alight.Direction = *(D3DVECTOR *)dir.get_data();

  alight.Range =  __D3DLIGHT_RANGE_MAX;
  alight.Falloff =  1.0f;
  alight.Theta =  0.0f;
  alight.Phi = deg_2_rad(lens->get_hfov());

  const LVecBase3f &att = light_obj->get_attenuation();
  alight.Attenuation0 = att[0];
  alight.Attenuation1 = att[1];
  alight.Attenuation2 = att[2];

  HRESULT hr = _d3d_device->SetLight(light_id, &alight);
  if (FAILED(hr)) {
    wdxdisplay8_cat.warning()
      << "Could not set light properties for " << light
      << " to id " << light_id << ": " << D3DERRORSTRING(hr) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_index_type
//       Access: Protected, Static
//  Description: Maps from the Geom's internal numeric type symbols
//               to DirectX's.
////////////////////////////////////////////////////////////////////
D3DFORMAT DXGraphicsStateGuardian8::
get_index_type(Geom::NumericType numeric_type) {
  switch (numeric_type) {
  case Geom::NT_uint16:
    return D3DFMT_INDEX16;

  case Geom::NT_uint32:
    return D3DFMT_INDEX32;
  }

  dxgsg8_cat.error()
    << "Invalid index NumericType value (" << (int)numeric_type << ")\n";
  return D3DFMT_INDEX16;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_material
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_material() {
  static Material empty;
  const Material *material;
  if (_target._material == (MaterialAttrib *)NULL ||
      _target._material->is_off()) {
    material = &empty;
  } else {
    material = _target._material->get_material();
  }

  D3DMATERIAL8 cur_material;
  cur_material.Diffuse = *(D3DCOLORVALUE *)(material->get_diffuse().get_data());
  cur_material.Ambient = *(D3DCOLORVALUE *)(material->get_ambient().get_data());
  cur_material.Specular = *(D3DCOLORVALUE *)(material->get_specular().get_data());
  cur_material.Emissive = *(D3DCOLORVALUE *)(material->get_emission().get_data());
  cur_material.Power = material->get_shininess();

  if (material->has_diffuse()) {
    // If the material specifies an diffuse color, use it.
    _d3d_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
  } else {
    // Otherwise, the diffuse color comes from the object color.
    if (_has_material_force_color) {
      cur_material.Diffuse = *(D3DCOLORVALUE *)_material_force_color.get_data();
      _d3d_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
    } else {
      _d3d_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    }
  }
  if (material->has_ambient()) {
    // If the material specifies an ambient color, use it.
    _d3d_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
  } else {
    // Otherwise, the ambient color comes from the object color.
    if (_has_material_force_color) {
      cur_material.Ambient = *(D3DCOLORVALUE *)_material_force_color.get_data();
      _d3d_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
    } else {
      _d3d_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    }
  }

  if (material->has_specular()) {
    _d3d_device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
  } else {
    _d3d_device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
  }

  if (material->get_local()) {
    _d3d_device->SetRenderState(D3DRS_LOCALVIEWER, TRUE);
  } else {
    _d3d_device->SetRenderState(D3DRS_LOCALVIEWER, FALSE);
  }

  _d3d_device->SetMaterial(&cur_material);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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
    
    _d3d_device->SetRenderState(D3DRS_POINTSPRITEENABLE, any_point_sprite);

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
//     Function: DXGraphicsStateGuardian8::enable_lighting
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable or disable the use of lighting overall.  This
//               is called by issue_light() according to whether any
//               lights are in use or not.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
enable_lighting(bool enable) {
  _d3d_device->SetRenderState(D3DRS_LIGHTING, (DWORD)enable);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_ambient_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               indicate the color of the ambient light that should
//               be in effect.  This is called by issue_light() after
//               all other lights have been enabled or disabled.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
set_ambient_light(const Colorf &color) {
  Colorf c = color;
  c.set(c[0] * _light_color_scale[0],
        c[1] * _light_color_scale[1],
        c[2] * _light_color_scale[2],
        c[3] * _light_color_scale[3]);

  _d3d_device->SetRenderState(D3DRS_AMBIENT, Colorf_to_D3DCOLOR(c));
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_light
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated light id.  A specific Light will
//               already have been bound to this id via bind_light().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
enable_light(int light_id, bool enable) {
  HRESULT hr = _d3d_device->LightEnable(light_id, enable);

  if (FAILED(hr)) {
    wdxdisplay8_cat.warning()
      << "Could not enable light " << light_id << ": "
      << D3DERRORSTRING(hr) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::enable_clip_plane
//       Access: Protected, Virtual
//  Description: Intended to be overridden by a derived class to
//               enable the indicated clip_plane id.  A specific
//               PlaneNode will already have been bound to this id via
//               bind_clip_plane().
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
enable_clip_plane(int plane_id, bool enable) {
  if (enable) {
    _clip_plane_bits |= ((DWORD)1 << plane_id);
  } else {
    _clip_plane_bits &= ~((DWORD)1 << plane_id);
  }
  _d3d_device->SetRenderState(D3DRS_CLIPPLANEENABLE, _clip_plane_bits);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::bind_clip_plane
//       Access: Protected, Virtual
//  Description: Called the first time a particular clip_plane has been
//               bound to a given id within a frame, this should set
//               up the associated hardware clip_plane with the clip_plane's
//               properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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

  HRESULT hr = _d3d_device->SetClipPlane(plane_id, world_plane.get_data());
  if (FAILED(hr)) {
    wdxdisplay8_cat.warning()
      << "Could not set clip plane for " << plane
      << " to id " << plane_id << ": " << D3DERRORSTRING(hr) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_blend_mode
//       Access: Protected, Virtual
//  Description: Called after any of the things that might change
//               blending state have changed, this function is
//               responsible for setting the appropriate color
//               blending mode based on the current properties.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_blending() {

  // Handle the color_write attrib.  If color_write is off, then
  // all the other blending-related stuff doesn't matter.  If the
  // device doesn't support color-write, we use blending tricks
  // to effectively disable color write.
  if (_target._color_write->get_channels() == ColorWriteAttrib::C_off) {
    if (_target._color_write != _state._color_write) {
      if (_screen->_can_direct_disable_color_writes) {
        enable_blend(false);
        _d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE, (DWORD)0x0);
      } else {
        enable_blend(true);
        call_dxBlendFunc(D3DBLEND_ZERO, D3DBLEND_ONE);
      }
    }
    return;
  } else {
    if (_target._color_write != _state._color_write) {
      if (_screen->_can_direct_disable_color_writes) {
        _d3d_device->SetRenderState(D3DRS_COLORWRITEENABLE, _target._color_write->get_channels());
      }
    }
  }

  CPT(ColorBlendAttrib) color_blend = _target._color_blend;
  ColorBlendAttrib::Mode color_blend_mode = _target._color_blend->get_mode();
  TransparencyAttrib::Mode transparency_mode = _target._transparency->get_mode();

  // Is there a color blend set?
  if (color_blend_mode != ColorBlendAttrib::M_none) {
    enable_blend(true);

    switch (color_blend_mode) {
    case ColorBlendAttrib::M_add:
      _d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
      break;

    case ColorBlendAttrib::M_subtract:
      _d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT);
      break;

    case ColorBlendAttrib::M_inv_subtract:
      _d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
      break;

    case ColorBlendAttrib::M_min:
      _d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MIN);
      break;

    case ColorBlendAttrib::M_max:
      _d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX);
      break;
    }

    call_dxBlendFunc(get_blend_func(color_blend->get_operand_a()),
                     get_blend_func(color_blend->get_operand_b()));
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
    _d3d_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    call_dxBlendFunc(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA);
    return;

  default:
    dxgsg8_cat.error()
      << "invalid transparency mode " << (int)transparency_mode << endl;
    break;
  }

  // Nothing's set, so disable blending.
  enable_blend(false);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::free_nondx_resources
//       Access: Public
//  Description: Frees some memory that was explicitly allocated
//               within the dxgsg.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
free_nondx_resources() {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::free_d3d_device
//       Access: Public
//  Description: setup for re-calling dx_init(), this is not the final
//               exit cleanup routine (see dx_cleanup)
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
free_d3d_device() {
  // dont want a full reset of gsg, just a state clear
  _state_rs = 0;
  _state.clear_to_zero();
  // want gsg to pass all state settings through

  _dx_is_ready = false;

  if (_d3d_device != NULL)
    for(int i = 0;i<D3D_MAXTEXTURESTAGES;i++)
      _d3d_device->SetTexture(i, NULL);  // d3d should release this stuff internally anyway, but whatever

  release_all();

  if (_d3d_device != NULL)
    RELEASE(_d3d_device, dxgsg8, "d3dDevice", RELEASE_DOWN_TO_ZERO);

  free_nondx_resources();

  // obviously we dont release ID3D8, just ID3DDevice8
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_draw_buffer
//       Access: Protected
//  Description: Sets up the glDrawBuffer to render into the buffer
//               indicated by the RenderBuffer object.  This only sets
//               up the color bits; it does not affect the depth,
//               stencil, accum layers.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
set_draw_buffer(const RenderBuffer &rb) {
  dxgsg8_cat.fatal() << "DX set_draw_buffer unimplemented!!!";
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_read_buffer
//       Access: Protected
//  Description: Vestigial analog of glReadBuffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
set_read_buffer(const RenderBuffer &rb) {
  if (rb._buffer_type & RenderBuffer::T_front) {
    _cur_read_pixel_buffer = RenderBuffer::T_front;
  } else  if (rb._buffer_type & RenderBuffer::T_back) {
    _cur_read_pixel_buffer = RenderBuffer::T_back;
  } else {
    dxgsg8_cat.error() << "Invalid or unimplemented Argument to set_read_buffer!\n";
  }
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_auto_rescale_normal
//       Access: Protected
//  Description: Issues the appropriate GL commands to either rescale
//               or normalize the normals according to the current
//               transform.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_auto_rescale_normal() {
  if (_external_transform->has_identity_scale()) {
    // If there's no scale, don't normalize anything.
    _d3d_device->SetRenderState(D3DRS_NORMALIZENORMALS, false);

  } else {
    // If there is a scale, turn on normalization.
    _d3d_device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
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
const D3DCOLORVALUE &DXGraphicsStateGuardian8::
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
//     Function: DXGraphicsStateGuardian8::get_blend_func
//       Access: Protected, Static
//  Description: Maps from ColorBlendAttrib::Operand to D3DBLEND
//               value.
////////////////////////////////////////////////////////////////////
D3DBLEND DXGraphicsStateGuardian8::
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

  dxgsg8_cat.error()
    << "Unknown color blend operand " << (int)operand << endl;
  return D3DBLEND_ZERO;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::report_texmgr_stats
//       Access: Protected
//  Description: Reports the DX texture manager's activity to PStats.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
report_texmgr_stats() {

#ifdef DO_PSTATS
  HRESULT hr;
#ifdef TEXMGRSTATS_USES_GETAVAILVIDMEM
  DWORD dwTexTotal, dwTexFree, dwVidTotal, dwVidFree;

  if (_total_texmem_pcollector.is_active()) {
    DDSCAPS2 ddsCaps;

    ZeroMemory(&ddsCaps, sizeof(ddsCaps));

    ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
    if (FAILED( hr = _d3d_device->GetAvailableVidMem(&ddsCaps, &dwVidTotal, &dwVidFree))) {
      dxgsg8_cat.fatal() << "report_texmgr GetAvailableVidMem for VIDMEM failed : result = " << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
      return;
    }

    ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    if (FAILED( hr = _d3d_device->GetAvailableVidMem(&ddsCaps, &dwTexTotal, &dwTexFree))) {
      dxgsg8_cat.fatal() << "report_texmgr GetAvailableVidMem for TEXTURE failed : result = " << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
      return;
    }
  }
#endif  // TEXMGRSTATS_USES_GETAVAILVIDMEM

  D3DDEVINFO_RESOURCEMANAGER all_resource_stats;
  ZeroMemory(&all_resource_stats, sizeof(D3DDEVINFO_RESOURCEMANAGER));

  if (!_tex_stats_retrieval_impossible) {
    hr = _d3d_device->GetInfo(D3DDEVINFOID_RESOURCEMANAGER, &all_resource_stats, sizeof(D3DDEVINFO_RESOURCEMANAGER));
    if (hr != D3D_OK) {
      if (hr == S_FALSE) {
        static int PrintedMsg = 2;
        if (PrintedMsg>0) {
          if (dxgsg8_cat.is_debug()) {
            dxgsg8_cat.debug()
              << "texstats GetInfo() requires debug DX DLLs to be installed!!\n";
          }
          ZeroMemory(&all_resource_stats, sizeof(D3DDEVINFO_RESOURCEMANAGER));
          _tex_stats_retrieval_impossible = true;
        }
      } else {
        dxgsg8_cat.error() << "GetInfo(RESOURCEMANAGER) failed to get tex stats: result = " << D3DERRORSTRING(hr);
        return;
      }
    }
  }

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
#endif  // TEXMGRSTATS_USES_GETAVAILVIDMEM
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_context
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
set_context(DXScreenData *new_context) {
  nassertv(new_context != NULL);
  _screen = new_context;
  _d3d_device = _screen->_d3d_device;   //copy this one field for speed of deref
  _swap_chain = _screen->_swap_chain;   //copy this one field for speed of deref
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_render_target
//       Access: Protected
//  Description: Set render target to the backbuffer of current swap
//               chain.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
set_render_target() {
  LPDIRECT3DSURFACE8 back = NULL, stencil = NULL;

  if (!_swap_chain)  //maybe fullscreen mode or main/single window
    _d3d_device->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back);
  else
    _swap_chain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back);

  //wdxdisplay8_cat.debug() << "swapchain is " << _swap_chain << "\n";
  //wdxdisplay8_cat.debug() << "back buffer is " << back << "\n";

  _d3d_device->GetDepthStencilSurface(&stencil);
  _d3d_device->SetRenderTarget(back, stencil);
  if (back) {
    back->Release();
  }
  if (stencil) {
    stencil->Release();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_texture_blend_mode
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
set_texture_blend_mode(int i, const TextureStage *stage) {
  switch (stage->get_mode()) {
  case TextureStage::M_modulate:
    // emulates GL_MODULATE glTexEnv mode 
    _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE);
    _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    break;

  case TextureStage::M_decal:
    // emulates GL_DECAL glTexEnv mode
    _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
    _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);

    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
    break;

  case TextureStage::M_replace:
    _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    break;

  case TextureStage::M_add:
    _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_ADD);
    _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);

    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    break;

  case TextureStage::M_blend:
  case TextureStage::M_blend_color_scale:
    {
      _d3d_device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_LERP);
      _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG0, D3DTA_TEXTURE);
      _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
      _d3d_device->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TFACTOR);
      
      _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
      _d3d_device->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    }
    break;

  case TextureStage::M_combine:
    // M_combine mode begins a collection of more sophisticated modes,
    // which match up more closely with DirectX's built-in modes.
    _d3d_device->SetTextureStageState
      (i, D3DTSS_COLOROP, 
       get_texture_operation(stage->get_combine_rgb_mode(),
                             stage->get_rgb_scale()));

    switch (stage->get_num_combine_rgb_operands()) {
    case 3:
      _d3d_device->SetTextureStageState
        (i, D3DTSS_COLORARG0, 
         get_texture_argument(stage->get_combine_rgb_source2(),
                              stage->get_combine_rgb_operand2()));
      // fall through
      
    case 2:
      _d3d_device->SetTextureStageState
        (i, D3DTSS_COLORARG2, 
         get_texture_argument(stage->get_combine_rgb_source1(),
                              stage->get_combine_rgb_operand1()));
      // fall through

    case 1:
      _d3d_device->SetTextureStageState
        (i, D3DTSS_COLORARG1, 
         get_texture_argument(stage->get_combine_rgb_source0(),
                              stage->get_combine_rgb_operand0()));
      // fall through
      
    default:
      break;
    }

    _d3d_device->SetTextureStageState
      (i, D3DTSS_ALPHAOP, 
       get_texture_operation(stage->get_combine_alpha_mode(),
                             stage->get_alpha_scale()));

    switch (stage->get_num_combine_alpha_operands()) {
    case 3:
      _d3d_device->SetTextureStageState
        (i, D3DTSS_ALPHAARG0, 
         get_texture_argument(stage->get_combine_alpha_source2(),
                              stage->get_combine_alpha_operand2()));
      // fall through
      
    case 2:
      _d3d_device->SetTextureStageState
        (i, D3DTSS_ALPHAARG2, 
         get_texture_argument(stage->get_combine_alpha_source1(),
                              stage->get_combine_alpha_operand1()));
      // fall through

    case 1:
      _d3d_device->SetTextureStageState
        (i, D3DTSS_ALPHAARG1, 
         get_texture_argument(stage->get_combine_alpha_source0(),
                              stage->get_combine_alpha_operand0()));
      // fall through
      
    default:
      break;
    }
    break;

  default:
    dxgsg8_cat.error()
      << "Unknown texture mode " << (int)stage->get_mode() << endl;
    break;
  }

  if (stage->get_saved_result()) {
    _d3d_device->SetTextureStageState(i, D3DTSS_RESULTARG, D3DTA_TEMP);
  } else {
    _d3d_device->SetTextureStageState(i, D3DTSS_RESULTARG, D3DTA_CURRENT);
  }

  if (stage->uses_color()) {
    // Set up the constant color for this stage.

    // Actually, DX8 doesn't support a per-stage constant color, but
    // it does support one TEXTUREFACTOR color for the whole pipeline.
    // This does mean you can't have two different blends in effect
    // with different colors on the same object.  However, DX9 does
    // support a per-stage constant color with the D3DTA_CONSTANT
    // argument--so we should implement that when this code gets
    // ported to DX9.

    D3DCOLOR texture_factor;
    if (stage->involves_color_scale() && _color_scale_enabled) {
      Colorf color = stage->get_color();
      color.set(color[0] * _current_color_scale[0],
                color[1] * _current_color_scale[1],
                color[2] * _current_color_scale[2],
                color[3] * _current_color_scale[3]);
      _texture_involves_color_scale = true;
      texture_factor = Colorf_to_D3DCOLOR(color);
    } else {
      texture_factor = Colorf_to_D3DCOLOR(stage->get_color());
    }
    _d3d_device->SetRenderState(D3DRS_TEXTUREFACTOR, texture_factor);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::dx_cleanup
//       Access: Protected
//  Description: Clean up the DirectX environment, accounting for exit()
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
dx_cleanup() {
  if (!_d3d_device) {
    return;
  }

  free_nondx_resources();
  PRINT_REFCNT(dxgsg8, _d3d_device);

  // Do a safe check for releasing the D3DDEVICE. RefCount should be zero.
  // if we're called from exit(), _d3d_device may already have been released
  RELEASE(_d3d_device, dxgsg8, "d3dDevice", RELEASE_DOWN_TO_ZERO);
  _screen->_d3d_device = NULL;

  // Releasing pD3D is now the responsibility of the GraphicsPipe destructor
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::reset_d3d_device
//       Access: Protected
//  Description: This function checks current device's framebuffer
//               dimension against passed p_presentation_params backbuffer
//               dimension to determine a device reset if there is
//               only one window or it is the main window or
//               fullscreen mode then, it resets the device. Finally
//               it returns the new DXScreenData through parameter
//               screen
////////////////////////////////////////////////////////////////////
HRESULT DXGraphicsStateGuardian8::
reset_d3d_device(D3DPRESENT_PARAMETERS *presentation_params, 
                 DXScreenData **screen) {
  HRESULT hr;

  assert(IS_VALID_PTR(presentation_params));
  assert(IS_VALID_PTR(_screen->_d3d8));
  assert(IS_VALID_PTR(_d3d_device));

  // Calling this forces all of the textures and vbuffers to be
  // regenerated.  It appears to be necessary on some cards but not
  // on others.
  //  release_all();
  // On second thought, let's try releasing the vertex buffers only.
  release_all_vertex_buffers();
  release_all_index_buffers();

  // for windowed mode make sure our format matches the desktop fmt,
  // in case the desktop mode has been changed
  _screen->_d3d8->GetAdapterDisplayMode(_screen->_card_id, &_screen->_display_mode);
  presentation_params->BackBufferFormat = _screen->_display_mode.Format;

  // here we have to look at the _presentation_reset frame buffer dimension
  // if current window's dimension is bigger than _presentation_reset
  // we have to reset the device before creating new swapchain.
  // inorder to reset properly, we need to release all swapchains

  if (!(_screen->_swap_chain)
      || (_presentation_reset.BackBufferWidth < presentation_params->BackBufferWidth)
      || (_presentation_reset.BackBufferHeight < presentation_params->BackBufferHeight)) {
    
    if (wdxdisplay8_cat.is_debug()) {
      wdxdisplay8_cat.debug()
        << "swap_chain = " << _screen->_swap_chain << " _presentation_reset = "
        << _presentation_reset.BackBufferWidth << "x" << _presentation_reset.BackBufferHeight
        << " presentation_params = "
        << presentation_params->BackBufferWidth << "x" << presentation_params->BackBufferHeight << "\n";
    }
    
    get_engine()->reset_all_windows(false);// reset old swapchain by releasing
    
    if (_screen->_swap_chain) {  //other windows might be using bigger buffers
      _presentation_reset.BackBufferWidth = max(_presentation_reset.BackBufferWidth, presentation_params->BackBufferWidth);
      _presentation_reset.BackBufferHeight = max(_presentation_reset.BackBufferHeight, presentation_params->BackBufferHeight);

    } else {  // single window, must reset to the new presentation_params dimension
      _presentation_reset.BackBufferWidth = presentation_params->BackBufferWidth;
      _presentation_reset.BackBufferHeight = presentation_params->BackBufferHeight;
    }

    hr = _d3d_device->Reset(&_presentation_reset);
    if (FAILED(hr)) {
      return hr;
    }
    
    get_engine()->reset_all_windows(true);// reset with new swapchains by creating
    if (screen) {
      *screen = NULL;
    }

    if (presentation_params != &_screen->_presentation_params) {
      memcpy(&_screen->_presentation_params, presentation_params, sizeof(D3DPRESENT_PARAMETERS));
    }

    return hr;
  }

  // release the old swapchain and create a new one
  if (_screen && _screen->_swap_chain) {
    _screen->_swap_chain->Release();
    wdxdisplay8_cat.debug() 
      << "swap chain " << _screen->_swap_chain << " is released\n";
    _screen->_swap_chain = NULL;
    hr = _d3d_device->CreateAdditionalSwapChain(presentation_params, &_screen->_swap_chain);
  }
  if (SUCCEEDED(hr)) {
    if (presentation_params != &_screen->_presentation_params) {
      memcpy(&_screen->_presentation_params, presentation_params, sizeof(D3DPRESENT_PARAMETERS));
    }
    if (screen) {
      *screen = _screen;
    }
  }
  return hr;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::check_cooperative_level
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian8::
check_cooperative_level() {
  bool bDoReactivateWindow = false;
  HRESULT hr = _d3d_device->TestCooperativeLevel();

  if (SUCCEEDED(hr)) {
    assert(SUCCEEDED(_last_testcooplevel_result));
    return true;
  }

  switch (hr) {
  case D3DERR_DEVICENOTRESET:
    _dx_is_ready = false;
    hr = reset_d3d_device(&_screen->_presentation_params);
    if (FAILED(hr)) {
      // I think this shouldnt fail unless I've screwed up the
      // _presentation_params from the original working ones somehow
      dxgsg8_cat.error()
        << "check_cooperative_level Reset() failed, hr = " << D3DERRORSTRING(hr);
    }

    hr = _d3d_device->TestCooperativeLevel();
    if (FAILED(hr)) {
      // internal chk, shouldnt fail
      dxgsg8_cat.error()
        << "TestCooperativeLevel following Reset() failed, hr = " << D3DERRORSTRING(hr);

    }

    _dx_is_ready = TRUE;
    break;

  case D3DERR_DEVICELOST:
    if (SUCCEEDED(_last_testcooplevel_result)) {
      if (_dx_is_ready) {
        _dx_is_ready = false;
        if (dxgsg8_cat.is_debug()) {
          dxgsg8_cat.debug() << "D3D Device was Lost, waiting...\n";
        }
      }
    }
  }

  _last_testcooplevel_result = hr;
  return SUCCEEDED(hr);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::show_frame
//       Access: Protected
//  Description: redraw primary buffer
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
show_frame() {
  if (_d3d_device == NULL) {
    return;
  }

  HRESULT hr;

  if (_swap_chain) {
    hr = _swap_chain->Present((CONST RECT*)NULL, (CONST RECT*)NULL, (HWND)NULL, NULL);
  } else {
    hr = _d3d_device->Present((CONST RECT*)NULL, (CONST RECT*)NULL, (HWND)NULL, NULL);
  }

  if (FAILED(hr)) {
    if (hr == D3DERR_DEVICELOST) {
      check_cooperative_level();
    } else {
      dxgsg8_cat.error()
        << "show_frame() - Present() failed" << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::create_swap_chain
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian8::
create_swap_chain(DXScreenData *new_context) {
  // Instead of creating a device and rendering as d3ddevice->present()
  // we should render using SwapChain->present(). This is done to support
  // multiple windows rendering. For that purpose, we need to set additional
  // swap chains here.

  HRESULT hr;
  hr = new_context->_d3d_device->CreateAdditionalSwapChain(&new_context->_presentation_params, &new_context->_swap_chain);
  if (FAILED(hr)) {
    wdxdisplay8_cat.debug() << "Swapchain creation failed :"<<D3DERRORSTRING(hr)<<"\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::release_swap_chain
//       Access: Protected
//  Description: Release the swap chain on this DXScreenData
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian8::
release_swap_chain(DXScreenData *new_context) {
  HRESULT hr;
  if (new_context->_swap_chain) {
    hr = new_context->_swap_chain->Release();
    if (FAILED(hr)) {
      wdxdisplay8_cat.debug() << "Swapchain release failed:" << D3DERRORSTRING(hr) << "\n";
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::copy_pres_reset
//       Access: Protected
//  Description: copies the PresReset from passed DXScreenData
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
copy_pres_reset(DXScreenData *screen) {
  memcpy(&_presentation_reset, &_screen->_presentation_params, sizeof(D3DPRESENT_PARAMETERS));
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_d3d_min_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
D3DTEXTUREFILTERTYPE DXGraphicsStateGuardian8::
get_d3d_min_type(Texture::FilterType filter_type) {
  switch (filter_type) {
  case Texture::FT_nearest:
    return D3DTEXF_POINT;

  case Texture::FT_linear:
    return D3DTEXF_LINEAR;

  case Texture::FT_nearest_mipmap_nearest:
    return D3DTEXF_POINT;

  case Texture::FT_linear_mipmap_nearest:
    return D3DTEXF_LINEAR;

  case Texture::FT_nearest_mipmap_linear:
    return D3DTEXF_POINT;

  case Texture::FT_linear_mipmap_linear:
    return D3DTEXF_LINEAR;
  }

  dxgsg8_cat.error()
    << "Invalid FilterType value (" << (int)filter_type << ")\n";
  return D3DTEXF_POINT;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_d3d_mip_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
D3DTEXTUREFILTERTYPE DXGraphicsStateGuardian8::
get_d3d_mip_type(Texture::FilterType filter_type) {
  switch (filter_type) {
  case Texture::FT_nearest:
    return D3DTEXF_NONE;

  case Texture::FT_linear:
    return D3DTEXF_NONE;

  case Texture::FT_nearest_mipmap_nearest:
    return D3DTEXF_POINT;

  case Texture::FT_linear_mipmap_nearest:
    return D3DTEXF_POINT;

  case Texture::FT_nearest_mipmap_linear:
    return D3DTEXF_LINEAR;

  case Texture::FT_linear_mipmap_linear:
    return D3DTEXF_LINEAR;
  }

  dxgsg8_cat.error()
    << "Invalid FilterType value (" << (int)filter_type << ")\n";
  return D3DTEXF_NONE;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_texture_operation
//       Access: Public, Static
//  Description: Returns the D3DTEXTUREOP value corresponding to the
//               indicated TextureStage::CombineMode enumerated type.
////////////////////////////////////////////////////////////////////
D3DTEXTUREOP DXGraphicsStateGuardian8::
get_texture_operation(TextureStage::CombineMode mode, int scale) {
  switch (mode) {
  case TextureStage::CM_undefined:
  case TextureStage::CM_replace:
    return D3DTOP_SELECTARG1;

  case TextureStage::CM_modulate:
    if (scale < 2) {
      return D3DTOP_MODULATE;
    } else if (scale < 4) {
      return D3DTOP_MODULATE2X;
    } else {
      return D3DTOP_MODULATE4X;
    }

  case TextureStage::CM_add:
    return D3DTOP_ADD;

  case TextureStage::CM_add_signed:
    if (scale < 2) {
      return D3DTOP_ADDSIGNED;
    } else {
      return D3DTOP_ADDSIGNED2X;
    }

  case TextureStage::CM_interpolate:
    return D3DTOP_LERP;

  case TextureStage::CM_subtract:
    return D3DTOP_SUBTRACT;

  case TextureStage::CM_dot3_rgb:
  case TextureStage::CM_dot3_rgba:
    return D3DTOP_DOTPRODUCT3;
  }

  dxgsg8_cat.error()
    << "Invalid TextureStage::CombineMode value (" << (int)mode << ")\n";
  return D3DTOP_DISABLE;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_texture_argument
//       Access: Public, Static
//  Description: Returns the D3DTA value corresponding to the
//               indicated TextureStage::CombineSource and
//               TextureStage::CombineOperand enumerated types.
////////////////////////////////////////////////////////////////////
DWORD DXGraphicsStateGuardian8::
get_texture_argument(TextureStage::CombineSource source,
                     TextureStage::CombineOperand operand) {
  switch (source) {
  case TextureStage::CS_undefined:
  case TextureStage::CS_texture:
    return D3DTA_TEXTURE | get_texture_argument_modifier(operand);

  case TextureStage::CS_constant:
  case TextureStage::CS_constant_color_scale:
    return D3DTA_TFACTOR | get_texture_argument_modifier(operand);

  case TextureStage::CS_primary_color:
    return D3DTA_DIFFUSE | get_texture_argument_modifier(operand);

  case TextureStage::CS_previous:
    return D3DTA_CURRENT | get_texture_argument_modifier(operand);

  case TextureStage::CS_last_saved_result:
    return D3DTA_TEMP | get_texture_argument_modifier(operand);
  }
  dxgsg8_cat.error()
    << "Invalid TextureStage::CombineSource value (" << (int)source << ")\n";
  return D3DTA_CURRENT;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_texture_argument_modifier
//       Access: Public, Static
//  Description: Returns the extra bits that modify the D3DTA
//               argument, according to the indicated
//               TextureStage::CombineOperand enumerated type.
////////////////////////////////////////////////////////////////////
DWORD DXGraphicsStateGuardian8::
get_texture_argument_modifier(TextureStage::CombineOperand operand) {
  switch (operand) {
  case TextureStage::CO_src_color:
    return 0;

  case TextureStage::CO_one_minus_src_color:
    return D3DTA_COMPLEMENT;

  case TextureStage::CO_src_alpha:
    return D3DTA_ALPHAREPLICATE;

  case TextureStage::CO_one_minus_src_alpha:
    return D3DTA_ALPHAREPLICATE | D3DTA_COMPLEMENT;

  case TextureStage::CO_undefined:
    break;
  }
  dxgsg8_cat.error()
    << "Invalid TextureStage::CombineOperand value (" << (int)operand << ")\n";
  return 0;
}

