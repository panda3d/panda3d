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
#include "qpgeomVertexFormat.h"
#include "qpgeomVertexData.h"
#include "qpgeomTriangles.h"
#include "qpgeomTristrips.h"
#include "qpgeomTrifans.h"
#include "qpgeomLines.h"
#include "qpgeomLinestrips.h"
#include "qpgeomPoints.h"
#include "qpGeomVertexReader.h"
#include "dxGeomMunger8.h"
#include "config_gobj.h"
#include "dxVertexBufferContext8.h"
#include "dxIndexBufferContext8.h"
#include "pStatTimer.h"
#include "pStatCollector.h"

#include <d3dx8.h>
#include <mmsystem.h>

TypeHandle DXGraphicsStateGuardian8::_type_handle;

static D3DMATRIX matIdentity;

#define __D3DLIGHT_RANGE_MAX ((float)sqrt(FLT_MAX))  //for some reason this is missing in dx8 hdrs

#define MY_D3DRGBA(r, g, b, a) ((D3DCOLOR) D3DCOLOR_COLORVALUE(r, g, b, a))

static bool tex_stats_retrieval_impossible = false;

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian8::
DXGraphicsStateGuardian8(const FrameBufferProperties &properties) :
  GraphicsStateGuardian(properties, CS_yup_left)
{
  _pScrn = NULL;
  _pD3DDevice = NULL;

  _bDXisReady = false;
  _transform_stale = false;
  _vertex_blending_enabled = false;
  _overlay_windows_supported = false;

  _active_vbuffer = NULL;
  _active_ibuffer = NULL;

  // non-dx obj values inited here should not change if resize is
  // called and dx objects need to be recreated (otherwise they
  // belong in dx_init, with other renderstate

  ZeroMemory(&matIdentity, sizeof(D3DMATRIX));
  matIdentity._11 = matIdentity._22 = matIdentity._33 = matIdentity._44 = 1.0f;

  _cur_read_pixel_buffer=RenderBuffer::T_front;
  set_color_clear_value(_color_clear_value);

  // DirectX drivers seem to consistently invert the texture when
  // they copy framebuffer-to-texture.  Ok.
  _copy_texture_inverted = true;

  // D3DRS_POINTSPRITEENABLE doesn't seem to support remapping the
  // texture coordinates via a texture matrix, so we don't advertise
  // GR_point_sprite_tex_matrix.
  _supported_geom_rendering =
    qpGeom::GR_point | qpGeom::GR_point_uniform_size |
    qpGeom::GR_point_perspective | qpGeom::GR_point_sprite |
    qpGeom::GR_triangle_strip | qpGeom::GR_triangle_fan |
    qpGeom::GR_flat_first_vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXGraphicsStateGuardian8::
~DXGraphicsStateGuardian8() {
  if (IS_VALID_PTR(_pD3DDevice))
    _pD3DDevice->SetTexture(0, NULL);  // this frees reference to the old texture
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
  if (dtc->create_texture(*_pScrn) == NULL) {
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
  if (tc == NULL) {
    // The texture wasn't bound properly or something, so ensure
    // texturing is disabled and just return.
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
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

      if (dtc->create_texture(*_pScrn) == NULL) {
        // Oops, we can't re-create the texture for some reason.
        dxgsg8_cat.error()
          << "Unable to re-create texture " << *dtc->_texture << endl;
        _pD3DDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        return;
      }
    }
  }

  Texture *tex = tc->_texture;
  Texture::WrapMode wrapU, wrapV;
  wrapU=tex->get_wrap_u();
  wrapV=tex->get_wrap_v();

  _pD3DDevice->SetTextureStageState(i, D3DTSS_ADDRESSU, get_texture_wrap_mode(wrapU));
  _pD3DDevice->SetTextureStageState(i, D3DTSS_ADDRESSV, get_texture_wrap_mode(wrapV));

  uint aniso_degree=tex->get_anisotropic_degree();
  Texture::FilterType ft=tex->get_magfilter();

  _pD3DDevice->SetTextureStageState(i, D3DTSS_MAXANISOTROPY, aniso_degree);

  D3DTEXTUREFILTERTYPE newMagFilter;
  if (aniso_degree<=1) {
    newMagFilter=((ft!=Texture::FT_nearest) ? D3DTEXF_LINEAR : D3DTEXF_POINT);
  } else {
    newMagFilter=D3DTEXF_ANISOTROPIC;
  }

  _pD3DDevice->SetTextureStageState(i, D3DTSS_MAGFILTER, newMagFilter);

  // map Panda composite min+mip filter types to d3d's separate min & mip filter types
  D3DTEXTUREFILTERTYPE newMinFilter = get_d3d_min_type(tex->get_minfilter());
  D3DTEXTUREFILTERTYPE newMipFilter = get_d3d_mip_type(tex->get_minfilter());

  if (!tex->might_have_ram_image()) {
    // If the texture is completely dynamic, don't try to issue
    // mipmaps--pandadx doesn't support auto-generated mipmaps at this
    // point.
    newMipFilter = D3DTEXF_NONE;
  }

#ifndef NDEBUG
  // sanity check
  if ((!dtc->has_mipmaps()) && (newMipFilter != D3DTEXF_NONE)) {
    dxgsg8_cat.error()
      << "Trying to set mipmap filtering for texture with no generated mipmaps!! texname["
      << tex->get_name() << "], filter("
      << tex->get_minfilter() << ")\n";
    newMipFilter=D3DTEXF_NONE;
  }
#endif

  if (aniso_degree>=2) {
    newMinFilter=D3DTEXF_ANISOTROPIC;
  }

  _pD3DDevice->SetTextureStageState(i, D3DTSS_MINFILTER, newMinFilter);
  _pD3DDevice->SetTextureStageState(i, D3DTSS_MIPFILTER, newMipFilter);

  _pD3DDevice->SetTexture(i, dtc->get_d3d_texture());
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::release_texture
//       Access: Public, Virtual
//  Description: Frees the GL resources previously allocated for the
//               texture.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
release_texture(TextureContext *tc) {
  DXTextureContext8 *gtc = DCAST(DXTextureContext8, tc);
  delete gtc;
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
prepare_vertex_buffer(qpGeomVertexArrayData *data) {
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
        dvbc->get_data()->get_usage_hint() != qpGeom::UH_client) {
      dvbc->create_vbuffer(*_pScrn);
    }

    if (dvbc->_vbuffer != NULL) {
      dvbc->upload_data();

      add_to_total_buffer_record(dvbc);
      dvbc->mark_loaded();

      _pD3DDevice->SetStreamSource
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
        dvbc->create_vbuffer(*_pScrn);
      }

      dvbc->upload_data();

      add_to_total_buffer_record(dvbc);
      dvbc->mark_loaded();
      _active_vbuffer = NULL;
    }

    if (_active_vbuffer != dvbc) {
      _pD3DDevice->SetStreamSource
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
prepare_index_buffer(qpGeomPrimitive *data) {
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
    dibc->create_ibuffer(*_pScrn);

    if (dibc->_ibuffer != NULL) {
      dibc->upload_data();
      add_to_total_buffer_record(dibc);
      dibc->mark_loaded();

      _pD3DDevice->SetIndices(dibc->_ibuffer, 0);
      _active_ibuffer = dibc;
      add_to_index_buffer_record(dibc);

    } else {
      _pD3DDevice->SetIndices(NULL, 0);
      _active_ibuffer = NULL;
    }

  } else {
    if (dibc->was_modified()) {
      if (dibc->changed_size()) {
        // We have to destroy the old index buffer and create a new
        // one.
        dibc->create_ibuffer(*_pScrn);
      }

      dibc->upload_data();

      add_to_total_buffer_record(dibc);
      dibc->mark_loaded();
      _active_ibuffer = NULL;
    }

    if (_active_ibuffer != dibc) {
      _pD3DDevice->SetIndices(dibc->_ibuffer, 0);
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
PT(qpGeomMunger) DXGraphicsStateGuardian8::
make_geom_munger(const RenderState *state) {
  PT(DXGeomMunger8) munger = new DXGeomMunger8(this, state);
  return qpGeomMunger::register_munger(munger);
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
    nassertv(_pScrn->PresParams.EnableAutoDepthStencil);
  }

  if (buffer_type & RenderBuffer::T_stencil) {
    aux_flags |=  D3DCLEAR_STENCIL;
    nassertv(_pScrn->PresParams.EnableAutoDepthStencil && IS_STENCIL_FORMAT(_pScrn->PresParams.AutoDepthStencilFormat));
  }

  if ((main_flags | aux_flags) != 0) {
    HRESULT hr = _pD3DDevice->Clear(0, NULL, main_flags | aux_flags, _d3dcolor_clear_value,
                                    _depth_clear_value, (DWORD)_stencil_clear_value);
    if (FAILED(hr) && main_flags == D3DCLEAR_TARGET && aux_flags != 0) {
      // Maybe there's a problem with the one or more of the auxiliary
      // buffers.
      hr = _pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, _d3dcolor_clear_value,
                              _depth_clear_value, (DWORD)_stencil_clear_value);
      if (!FAILED(hr)) {
        // Yep, it worked without them.  That's a problem.  Which buffer
        // poses the problem?
        if (buffer_type & RenderBuffer::T_depth) {
          aux_flags |=  D3DCLEAR_ZBUFFER;
          HRESULT hr2 = _pD3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, _d3dcolor_clear_value,
                                           _depth_clear_value, (DWORD)_stencil_clear_value);
          if (FAILED(hr2)) {
            dxgsg8_cat.error()
              << "Unable to clear depth buffer; removing.\n";
            _buffer_mask &= ~RenderBuffer::T_depth;
          }
        }
        if (buffer_type & RenderBuffer::T_stencil) {
          aux_flags |=  D3DCLEAR_STENCIL;
          HRESULT hr2 = _pD3DDevice->Clear(0, NULL, D3DCLEAR_STENCIL, _d3dcolor_clear_value,
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
    HRESULT hr = _pD3DDevice->SetViewport(&vp);
    if (FAILED(hr)) {
      dxgsg8_cat.error()
        << "pScrn_SwapChain = " << _pScrn->pSwapChain << " SwapChain = " << _pSwapChain << "\n";
      dxgsg8_cat.error()
        << "SetViewport(" << l << ", " << u << ", " << w << ", " << h
        << ") failed" << D3DERRORSTRING(hr);

      D3DVIEWPORT8 vp_old;
      _pD3DDevice->GetViewport(&vp_old);
      dxgsg8_cat.error()
        << "GetViewport(" << vp_old.X << ", " << vp_old.Y << ", " << vp_old.Width << ", "
        << vp_old.Height << ") returned: Trying to set that vp---->\n";
      hr = _pD3DDevice->SetViewport(&vp_old);

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
    _pD3DDevice->SetTransform(D3DTS_PROJECTION,
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

  HRESULT hr = _pD3DDevice->BeginScene();

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
  HRESULT hr = _pD3DDevice->EndScene();

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
//     Function: DXGraphicsStateGuardian8::begin_draw_primitives
//       Access: Public, Virtual
//  Description: Called before a sequence of draw_primitive()
//               functions are called, this should prepare the vertex
//               data for rendering.  It returns true if the vertices
//               are ok, false to abort this group of primitives.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian8::
begin_draw_primitives(const qpGeom *geom, const qpGeomMunger *munger,
                      const qpGeomVertexData *vertex_data) {
  if (!GraphicsStateGuardian::begin_draw_primitives(geom, munger, vertex_data)) {
    return false;
  }
  nassertr(_vertex_data != (qpGeomVertexData *)NULL, false);

  const qpGeomVertexFormat *format = _vertex_data->get_format();

  // The munger should have put the FVF data in the first array.
  const qpGeomVertexArrayData *data = _vertex_data->get_array(0);

  VertexBufferContext *vbc = ((qpGeomVertexArrayData *)data)->prepare_now(get_prepared_objects(), this);
  nassertr(vbc != (VertexBufferContext *)NULL, false);
  apply_vertex_buffer(vbc);

  const qpGeomVertexAnimationSpec &animation =
    vertex_data->get_format()->get_animation();
  if (animation.get_animation_type() == qpGeom::AT_hardware) {
    // Set up vertex blending.
    switch (animation.get_num_transforms()) {
    case 1:
      _pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS);
      break;
    case 2:
      _pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);
      break;
    case 3:
      _pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_2WEIGHTS);
      break;
    case 4:
      _pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_3WEIGHTS);
      break;
    }

    if (animation.get_indexed_transforms()) {
      // Set up indexed vertex blending.
      _pD3DDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE);
    } else {
      _pD3DDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    }

    const TransformTable *table = vertex_data->get_transform_table();
    if (table != (TransformTable *)NULL) {
      for (int i = 0; i < table->get_num_transforms(); i++) {
        LMatrix4f mat;
        table->get_transform(i)->mult_matrix(mat, _internal_transform->get_mat());
        const D3DMATRIX *d3d_mat = (const D3DMATRIX *)mat.get_data();
        _pD3DDevice->SetTransform(D3DTS_WORLDMATRIX(i), d3d_mat);
      }

      // Setting the first animation matrix steps on the world matrix,
      // so we have to set a flag to reload the world matrix later.
      _transform_stale = true;
    }
    _vertex_blending_enabled = true;

  } else {
    // We're not using vertex blending.
    if (_vertex_blending_enabled) {
      _pD3DDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
      _pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
      _vertex_blending_enabled = false;
    }

    if (_transform_stale && !_vertex_data->is_vertex_transformed()) {
      const D3DMATRIX *d3d_mat = (const D3DMATRIX *)_internal_transform->get_mat().get_data();
      _pD3DDevice->SetTransform(D3DTS_WORLD, d3d_mat);
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
    _pD3DDevice->SetTransform(D3DTS_WORLD, &matIdentity);
    static const LMatrix4f rescale_mat
      (1, 0, 0, 0,
       0, 1, 0, 0,
       0, 0, 0.5, 0,
       0, 0, 0.5, 1);
    _transform_stale = true;

    _pD3DDevice->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX *)rescale_mat.get_data());
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::draw_triangles
//       Access: Public, Virtual
//  Description: Draws a series of disconnected triangles.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
draw_triangles(const qpGeomTriangles *primitive) {
  _vertices_tri_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_tri_pcollector.add_level(1);
  if (primitive->is_indexed()) {
    int min_vertex = dx_broken_max_index ? 0 : primitive->get_min_vertex();
    int max_vertex = primitive->get_max_vertex();

    if (_active_vbuffer != NULL) {
      // Indexed, vbuffers.
      IndexBufferContext *ibc = ((qpGeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
      nassertv(ibc != (IndexBufferContext *)NULL);
      apply_index_buffer(ibc);

      _pD3DDevice->DrawIndexedPrimitive
        (D3DPT_TRIANGLELIST,
         min_vertex, max_vertex - min_vertex + 1,
         0, primitive->get_num_primitives());

    } else {
      // Indexed, client arrays.
      D3DFORMAT index_type = get_index_type(primitive->get_index_type());

      _pD3DDevice->DrawIndexedPrimitiveUP
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
      _pD3DDevice->DrawPrimitive
        (D3DPT_TRIANGLELIST,
         primitive->get_first_vertex(),
         primitive->get_num_primitives());

    } else {
      // Nonindexed, client arrays.
      int stride = _vertex_data->get_format()->get_array(0)->get_stride();
      unsigned int first_vertex = primitive->get_first_vertex();
      _pD3DDevice->DrawPrimitiveUP
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
draw_tristrips(const qpGeomTristrips *primitive) {
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
        IndexBufferContext *ibc = ((qpGeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
        nassertv(ibc != (IndexBufferContext *)NULL);
        apply_index_buffer(ibc);

        _pD3DDevice->DrawIndexedPrimitive
          (D3DPT_TRIANGLESTRIP,
           min_vertex, max_vertex - min_vertex + 1,
           0, primitive->get_num_vertices() - 2);

      } else {
        // Indexed, client arrays, one long triangle strip.
        D3DFORMAT index_type = get_index_type(primitive->get_index_type());
        _pD3DDevice->DrawIndexedPrimitiveUP
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
        _pD3DDevice->DrawPrimitive
          (D3DPT_TRIANGLESTRIP,
           primitive->get_first_vertex(),
           primitive->get_num_vertices() - 2);

      } else {
        // Indexed, client arrays, one long triangle strip.
        int stride = _vertex_data->get_format()->get_array(0)->get_stride();
        unsigned int first_vertex = primitive->get_first_vertex();
        _pD3DDevice->DrawPrimitiveUP
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

      qpGeomVertexReader mins(primitive->get_mins(), 0);
      qpGeomVertexReader maxs(primitive->get_maxs(), 0);
      nassertv(primitive->get_mins()->get_num_rows() == (int)ends.size() &&
               primitive->get_maxs()->get_num_rows() == (int)ends.size());

      if (_active_vbuffer != NULL) {
        // Indexed, vbuffers, individual triangle strips.
        IndexBufferContext *ibc = ((qpGeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
        nassertv(ibc != (IndexBufferContext *)NULL);
        apply_index_buffer(ibc);

        unsigned int start = 0;
        for (size_t i = 0; i < ends.size(); i++) {
          _vertices_tristrip_pcollector.add_level(ends[i] - start);
          unsigned int min = mins.get_data1i();
          unsigned int max = maxs.get_data1i();
          _pD3DDevice->DrawIndexedPrimitive
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
          _pD3DDevice->DrawIndexedPrimitiveUP
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
          _pD3DDevice->DrawPrimitive
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
          _pD3DDevice->DrawPrimitiveUP
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
draw_trifans(const qpGeomTrifans *primitive) {
  CPTA_int ends = primitive->get_ends();
  _primitive_batches_trifan_pcollector.add_level(ends.size());

  if (primitive->is_indexed()) {
    int min_vertex = dx_broken_max_index ? 0 : primitive->get_min_vertex();
    int max_vertex = primitive->get_max_vertex();

    // Send the individual triangle fans.  There's no connecting fans
    // with degenerate vertices, so no worries about that.
    int index_stride = primitive->get_index_stride();

    qpGeomVertexReader mins(primitive->get_mins(), 0);
    qpGeomVertexReader maxs(primitive->get_maxs(), 0);
    nassertv(primitive->get_mins()->get_num_rows() == (int)ends.size() &&
             primitive->get_maxs()->get_num_rows() == (int)ends.size());

    if (_active_vbuffer != NULL) {
      // Indexed, vbuffers.
      IndexBufferContext *ibc = ((qpGeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
      nassertv(ibc != (IndexBufferContext *)NULL);
      apply_index_buffer(ibc);

      unsigned int start = 0;
      for (size_t i = 0; i < ends.size(); i++) {
        _vertices_trifan_pcollector.add_level(ends[i] - start);
        unsigned int min = mins.get_data1i();
        unsigned int max = maxs.get_data1i();
        _pD3DDevice->DrawIndexedPrimitive
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
        _pD3DDevice->DrawIndexedPrimitiveUP
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
        _pD3DDevice->DrawPrimitive
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
        _pD3DDevice->DrawPrimitiveUP
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
draw_lines(const qpGeomLines *primitive) {
  _vertices_other_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_other_pcollector.add_level(1);

  if (primitive->is_indexed()) {
    int min_vertex = dx_broken_max_index ? 0 : primitive->get_min_vertex();
    int max_vertex = primitive->get_max_vertex();

    if (_active_vbuffer != NULL) {
      // Indexed, vbuffers.
      IndexBufferContext *ibc = ((qpGeomPrimitive *)primitive)->prepare_now(get_prepared_objects(), this);
      nassertv(ibc != (IndexBufferContext *)NULL);
      apply_index_buffer(ibc);

      _pD3DDevice->DrawIndexedPrimitive
        (D3DPT_LINELIST,
         min_vertex, max_vertex - min_vertex + 1,
         0, primitive->get_num_primitives());

    } else {
      // Indexed, client arrays.
      D3DFORMAT index_type = get_index_type(primitive->get_index_type());

      _pD3DDevice->DrawIndexedPrimitiveUP
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
      _pD3DDevice->DrawPrimitive
        (D3DPT_LINELIST,
         primitive->get_first_vertex(),
         primitive->get_num_primitives());

    } else {
      // Nonindexed, client arrays.
      int stride = _vertex_data->get_format()->get_array(0)->get_stride();
      unsigned int first_vertex = primitive->get_first_vertex();
      _pD3DDevice->DrawPrimitiveUP
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
draw_linestrips(const qpGeomLinestrips *primitive) {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::draw_points
//       Access: Public, Virtual
//  Description: Draws a series of disconnected points.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
draw_points(const qpGeomPoints *primitive) {
  _vertices_other_pcollector.add_level(primitive->get_num_vertices());
  _primitive_batches_other_pcollector.add_level(1);

  // The munger should have protected us from indexed points--DirectX
  // doesn't support them.
  nassertv(!primitive->is_indexed());

  if (_active_vbuffer != NULL) {
    // Nonindexed, vbuffers.
    _pD3DDevice->DrawPrimitive
      (D3DPT_POINTLIST,
       primitive->get_first_vertex(),
       primitive->get_num_primitives());

  } else {
    // Nonindexed, client arrays.
    int stride = _vertex_data->get_format()->get_array(0)->get_stride();
    unsigned int first_vertex = primitive->get_first_vertex();
    _pD3DDevice->DrawPrimitiveUP
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
    _pD3DDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    _pD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
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

  IDirect3DSurface8 *pTexSurfaceLev0, *pCurRenderTarget;
  hr = dtc->get_d3d_texture()->GetSurfaceLevel(0, &pTexSurfaceLev0);
  if (FAILED(hr)) {
    dxgsg8_cat.error() << "GetSurfaceLev failed in copy_texture" << D3DERRORSTRING(hr);
    return;
  }

  hr = _pD3DDevice->GetRenderTarget(&pCurRenderTarget);
  if (FAILED(hr)) {
    dxgsg8_cat.error() << "GetRenderTgt failed in copy_texture" << D3DERRORSTRING(hr);
    SAFE_RELEASE(pTexSurfaceLev0);
    return;
  }

  RECT SrcRect;

  SrcRect.left = xo;
  SrcRect.right = xo+w;
  SrcRect.top = yo;
  SrcRect.bottom = yo+h;

  // now copy from fb to tex
  hr = _pD3DDevice->CopyRects(pCurRenderTarget, &SrcRect, 1, pTexSurfaceLev0, 0);
  if (FAILED(hr)) {
    dxgsg8_cat.error()
      << "CopyRects failed in copy_texture" << D3DERRORSTRING(hr);
  }

  SAFE_RELEASE(pCurRenderTarget);
  SAFE_RELEASE(pTexSurfaceLev0);
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

  RECT SrcCopyRect;
  nassertr(tex != NULL && dr != NULL, false);

  int xo, yo, w, h;
  dr->get_region_pixels_i(xo, yo, w, h);

  tex->setup_2d_texture(w, h, Texture::T_unsigned_byte, Texture::F_rgb);

  SrcCopyRect.top = yo;
  SrcCopyRect.left = xo;
  SrcCopyRect.right = xo + w;
  SrcCopyRect.bottom = yo + h;

  IDirect3DSurface8 *pD3DSurf;
  HRESULT hr;

  if (_cur_read_pixel_buffer & RenderBuffer::T_back) {
    hr=_pD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pD3DSurf);

    if (FAILED(hr)) {
      dxgsg8_cat.error() << "GetBackBuffer failed" << D3DERRORSTRING(hr);
      return false;
    }

    // note if you try to grab the backbuffer and full-screen anti-aliasing is on,
    // the backbuffer might be larger than the window size.  for screenshots its safer to get the front buffer.

  } else if (_cur_read_pixel_buffer & RenderBuffer::T_front) {
    // must create a A8R8G8B8 sysmem surface for GetFrontBuffer to copy to

    DWORD TmpSurfXsize, TmpSurfYsize;

    if (_pScrn->PresParams.Windowed) {
      // GetFrontBuffer retrieves the entire desktop for a monitor, so
      // need space for that

      MONITORINFO minfo;
      minfo.cbSize = sizeof(MONITORINFO);
      GetMonitorInfo(_pScrn->hMon, &minfo);   // have to use GetMonitorInfo, since this gsg may not be for primary monitor

      TmpSurfXsize = RECT_XSIZE(minfo.rcMonitor);
      TmpSurfYsize = RECT_YSIZE(minfo.rcMonitor);

      // set SrcCopyRect to client area of window in scrn coords
      ClientToScreen(_pScrn->hWnd, (POINT*)&SrcCopyRect.left);
      ClientToScreen(_pScrn->hWnd, (POINT*)&SrcCopyRect.right);

    } else {
      RECT WindRect;
      GetWindowRect(_pScrn->hWnd, &WindRect);
      TmpSurfXsize = RECT_XSIZE(WindRect);
      TmpSurfYsize = RECT_YSIZE(WindRect);
    }

    hr=_pD3DDevice->CreateImageSurface(TmpSurfXsize, TmpSurfYsize, D3DFMT_A8R8G8B8, &pD3DSurf);
    if (FAILED(hr)) {
      dxgsg8_cat.error() << "CreateImageSurface failed in copy_pixel_buffer()" << D3DERRORSTRING(hr);
      return false;
    }

    hr=_pD3DDevice->GetFrontBuffer(pD3DSurf);

    if (hr==D3DERR_DEVICELOST) {
      pD3DSurf->Release();
      dxgsg8_cat.error() << "copy_pixel_buffer failed: device lost\n";
      return false;
    }

  } else {
    dxgsg8_cat.error() << "copy_pixel_buffer: unhandled current_read_pixel_buffer type\n";
    return false;
  }

  DXTextureContext8::d3d_surface_to_texture(SrcCopyRect, pD3DSurf, tex);

  RELEASE(pD3DSurf, dxgsg8, "pD3DSurf", RELEASE_ONCE);

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
  set_state(RenderState::make_empty());
  // want gsg to pass all state settings down so any non-matching defaults we set here get overwritten

  assert(_pScrn->pD3D8!=NULL);
  assert(_pD3DDevice!=NULL);

  D3DCAPS8 d3dCaps;
  _pD3DDevice->GetDeviceCaps(&d3dCaps);

  if (dxgsg8_cat.is_debug()) {
    dxgsg8_cat.debug()
      << "\nHwTransformAndLight = " << ((d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
      << "\nMaxTextureWidth = " << d3dCaps.MaxTextureWidth
      << "\nMaxTextureHeight = " << d3dCaps.MaxTextureHeight
      << "\nMaxVolumeExtent = " << d3dCaps.MaxVolumeExtent
      << "\nMaxTextureAspectRatio = " << d3dCaps.MaxTextureAspectRatio
      << "\nTexCoordCount = " << (d3dCaps.FVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK)
      << "\nMaxTextureBlendStages = " << d3dCaps.MaxTextureBlendStages
      << "\nMaxSimultaneousTextures = " << d3dCaps.MaxSimultaneousTextures
      << "\nMaxActiveLights = " << d3dCaps.MaxActiveLights
      << "\nMaxUserClipPlanes = " << d3dCaps.MaxUserClipPlanes
      << "\nMaxVertexBlendMatrices = " << d3dCaps.MaxVertexBlendMatrices
      << "\nMaxVertexBlendMatrixIndex = " << d3dCaps.MaxVertexBlendMatrixIndex
      << "\nMaxPointSize = " << d3dCaps.MaxPointSize
      << "\nMaxPrimitiveCount = " << d3dCaps.MaxPrimitiveCount
      << "\nMaxVertexIndex = " << d3dCaps.MaxVertexIndex
      << "\nMaxStreams = " << d3dCaps.MaxStreams
      << "\nMaxStreamStride = " << d3dCaps.MaxStreamStride
      << "\n";
  }

  _max_lights = d3dCaps.MaxActiveLights;
  _max_clip_planes = d3dCaps.MaxUserClipPlanes;
  _max_vertex_transforms = d3dCaps.MaxVertexBlendMatrices;
  _max_vertex_transform_indices = d3dCaps.MaxVertexBlendMatrixIndex;

  ZeroMemory(&_lmodel_ambient, sizeof(Colorf));
  _pD3DDevice->SetRenderState(D3DRS_AMBIENT, 0x0);

  _clip_plane_bits = 0;
  _pD3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE , 0x0);

  _pD3DDevice->SetRenderState(D3DRS_CLIPPING, true);

  // these both reflect d3d defaults
  _color_writemask = 0xFFFFFFFF;
  _CurFVFType = 0x0;  // guards SetVertexShader fmt

  _pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

  _depth_test_enabled = true;
  _pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, _depth_test_enabled);

  _pD3DDevice->SetRenderState(D3DRS_EDGEANTIALIAS, false);

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

  _projection_mat = LMatrix4f::ident_mat();
  _has_scene_graph_color = false;

  // Apply a default material when materials are turned off.
  _pending_material = NULL;
  do_issue_material();

  _last_testcooplevel_result = D3D_OK;

  for(int i=0;i<MAX_POSSIBLE_TEXFMTS;i++) {
    // look for all possible DX8 texture fmts
    D3DFORMAT_FLAG fmtflag=D3DFORMAT_FLAG(1<<i);
    hr = _pScrn->pD3D8->CheckDeviceFormat(_pScrn->CardIDNum, D3DDEVTYPE_HAL, _pScrn->DisplayMode.Format,
                                          0x0, D3DRTYPE_TEXTURE, g_D3DFORMATmap[fmtflag]);
    if (SUCCEEDED(hr)){
      _pScrn->SupportedTexFmtsMask|=fmtflag;
    }
  }

  // s3 virge drivers sometimes give crap values for these
  if (_pScrn->d3dcaps.MaxTextureWidth==0)
    _pScrn->d3dcaps.MaxTextureWidth=256;

  if (_pScrn->d3dcaps.MaxTextureHeight==0)
    _pScrn->d3dcaps.MaxTextureHeight=256;

  if (_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGTABLE) {
    // watch out for drivers that emulate per-pixel fog with per-vertex fog (Riva128, Matrox Millen G200)
    // some of these require gouraud-shading to be set to work, as if you were using vertex fog
    _doFogType=PerPixelFog;
  } else {
    // every card is going to have vertex fog, since it's implemented in d3d runtime
    assert((_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGVERTEX)!=0);

    // vtx fog may look crappy if you have large polygons in the foreground and they get clipped,
    // so you may want to disable it

    if (dx_no_vertex_fog) {
      _doFogType = None;
    } else {
      _doFogType = PerVertexFog;

      // range-based fog only works with vertex fog in dx7/8
      if (dx_use_rangebased_fog && (_pScrn->d3dcaps.RasterCaps & D3DPRASTERCAPS_FOGRANGE))
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

  _pD3DDevice->SetRenderState(D3DRS_CLIPPING, true);

  // Stencil test is off by default
  _pD3DDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);

  // Antialiasing.
  _pD3DDevice->SetRenderState(D3DRS_EDGEANTIALIAS, FALSE);

  _current_fill_mode = RenderModeAttrib::M_filled;
  _pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

  // must do SetTSS here because redundant states are filtered out by
  // our code based on current values above, so initial conditions
  // must be correct
  _texturing_enabled = false;
  _pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);  // disables texturing

  _cull_face_mode = CullFaceAttrib::M_cull_none;
  _pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  _alpha_func = D3DCMP_ALWAYS;
  _alpha_func_refval = 1.0f;
  _pD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, _alpha_func);
  _pD3DDevice->SetRenderState(D3DRS_ALPHAREF, (UINT)(_alpha_func_refval*255.0f));
  _alpha_test_enabled = false;
  _pD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, _alpha_test_enabled);

  // this is a new DX8 state that lets you do additional operations other than ADD (e.g. subtract/max/min)
  // must check (_pScrn->d3dcaps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP) (yes on GF2/Radeon8500, no on TNT)
  _pD3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

  PRINT_REFCNT(dxgsg8, _pD3DDevice);

  // Make sure the DX state matches all of our initial attribute states.
  CPT(RenderAttrib) dta = DepthTestAttrib::make(DepthTestAttrib::M_less);
  CPT(RenderAttrib) dwa = DepthWriteAttrib::make(DepthWriteAttrib::M_on);
  CPT(RenderAttrib) cfa = CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise);

  dta->issue(this);
  dwa->issue(this);
  cfa->issue(this);

  PRINT_REFCNT(dxgsg8, _pD3DDevice);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::apply_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
apply_fog(Fog *fog) {

  if (_doFogType==None)
    return;

  Fog::Mode panda_fogmode = fog->get_mode();
  D3DFOGMODE d3dfogmode = get_fog_mode_type(panda_fogmode);

  _pD3DDevice->SetRenderState((D3DRENDERSTATETYPE)_doFogType, d3dfogmode);

  const Colorf &fog_colr = fog->get_color();
  _pD3DDevice->SetRenderState(D3DRS_FOGCOLOR,
                              MY_D3DRGBA(fog_colr[0], fog_colr[1], fog_colr[2], 0.0f));  // Alpha bits are not used

  // do we need to adjust fog start/end values based on D3DPRASTERCAPS_WFOG/D3DPRASTERCAPS_ZFOG ?
  // if not WFOG, then docs say we need to adjust values to range [0, 1]

  switch (panda_fogmode) {
  case Fog::M_linear:
    {
      float onset, opaque;
      fog->get_linear_range(onset, opaque);

      _pD3DDevice->SetRenderState(D3DRS_FOGSTART,
                                   *((LPDWORD) (&onset)));
      _pD3DDevice->SetRenderState(D3DRS_FOGEND,
                                   *((LPDWORD) (&opaque)));
    }
    break;
  case Fog::M_exponential:
  case Fog::M_exponential_squared:
    {
      // Exponential fog is always camera-relative.
      float fog_density = fog->get_exp_density();
      _pD3DDevice->SetRenderState(D3DRS_FOGDENSITY,
                                   *((LPDWORD) (&fog_density)));
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_transform
//       Access: Public, Virtual
//  Description: Sends the indicated transform matrix to the graphics
//               API to be applied to future vertices.
//
//               This transform is the internal_transform, already
//               converted into the GSG's internal coordinate system.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
issue_transform(const TransformState *transform) {
  DO_PSTATS_STUFF(_transform_state_pcollector.add_level(1));

  const D3DMATRIX *d3d_mat = (const D3DMATRIX *)transform->get_mat().get_data();
  _pD3DDevice->SetTransform(D3DTS_WORLD, d3d_mat);
  _transform_stale = false;

  if (_auto_rescale_normal) {
    do_auto_rescale_normal();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_alpha_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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
//     Function: DXGraphicsStateGuardian8::issue_render_mode
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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
    dxgsg8_cat.error()
      << "Unknown render mode " << (int)mode << endl;
  }

  // This might also specify the point size.
  float point_size = attrib->get_thickness();
  _pD3DDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&point_size));

  if (attrib->get_perspective()) {
    _pD3DDevice->SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);

    LVector3f height(0.0f, point_size, 1.0f);
    height = height * _projection_mat;
    float s = height[1] / point_size;

    float zero = 0.0f;
    float one_over_s2 = 1.0f / (s * s);
    _pD3DDevice->SetRenderState(D3DRS_POINTSCALE_A, *((DWORD*)&zero));
    _pD3DDevice->SetRenderState(D3DRS_POINTSCALE_B, *((DWORD*)&zero));
    _pD3DDevice->SetRenderState(D3DRS_POINTSCALE_C, *((DWORD*)&one_over_s2));

  } else {
    _pD3DDevice->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
  }

  _current_fill_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_rescale_normal
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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
    dxgsg8_cat.error()
      << "Unknown rescale_normal mode " << (int)mode << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_color_write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
issue_color_write(const ColorWriteAttrib *attrib) {
  _color_write_mode = attrib->get_mode();
  set_color_writemask((_color_write_mode ==ColorWriteAttrib::M_on) ? 0xFFFFFFFF : 0x0);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_depth_test
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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
//     Function: DXGraphicsStateGuardian8::issue_depth_write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
issue_depth_write(const DepthWriteAttrib *attrib) {
  enable_zwritemask(attrib->get_mode() == DepthWriteAttrib::M_on);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_cull_face
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
issue_cull_face(const CullFaceAttrib *attrib) {
  _cull_face_mode = attrib->get_effective_mode();

  switch (_cull_face_mode) {
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
    dxgsg8_cat.error()
      << "invalid cull face mode " << (int)_cull_face_mode << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_fog
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
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
//     Function: DXGraphicsStateGuardian8::issue_depth_offset
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
issue_depth_offset(const DepthOffsetAttrib *attrib) {
  int offset = attrib->get_offset();
  _pD3DDevice->SetRenderState(D3DRS_ZBIAS, offset);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_tex_gen
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
issue_tex_gen(const TexGenAttrib *attrib) {
  TexGenAttrib::Mode mode = attrib->get_mode(TextureStage::get_default());
  switch (mode) {
  case TexGenAttrib::M_off:
    _pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    _pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
    break;

  case TexGenAttrib::M_eye_sphere_map:
    _pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX,
                                      D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
    _pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
    break;

  case TexGenAttrib::M_point_sprite:
    _pD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    _pD3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::issue_shade_model
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
issue_shade_model(const ShadeModelAttrib *attrib) {
  switch (attrib->get_mode()) {
  case ShadeModelAttrib::M_smooth:
    _pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    break;

  case ShadeModelAttrib::M_flat:
    _pD3DDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
    break;
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

  HRESULT hr = _pD3DDevice->SetLight(light_id, &alight);
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

  HRESULT hr = _pD3DDevice->SetLight(light_id, &alight);
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

  HRESULT hr = _pD3DDevice->SetLight(light_id, &alight);
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
get_index_type(qpGeom::NumericType numeric_type) {
  switch (numeric_type) {
  case qpGeom::NT_uint16:
    return D3DFMT_INDEX16;

  case qpGeom::NT_uint32:
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
  if (_pending_material == (MaterialAttrib *)NULL ||
      _pending_material->is_off()) {
    material = &empty;
  } else {
    material = _pending_material->get_material();
  }

  D3DMATERIAL8 cur_material;
  cur_material.Diffuse = *(D3DCOLORVALUE *)(material->get_diffuse().get_data());
  cur_material.Ambient = *(D3DCOLORVALUE *)(material->get_ambient().get_data());
  cur_material.Specular = *(D3DCOLORVALUE *)(material->get_specular().get_data());
  cur_material.Emissive = *(D3DCOLORVALUE *)(material->get_emission().get_data());
  cur_material.Power = material->get_shininess();

  if (material->has_diffuse()) {
    // If the material specifies an diffuse color, use it.
    _pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
  } else {
    // Otherwise, the diffuse color comes from the object color.
    if (_has_material_force_color) {
      cur_material.Diffuse = *(D3DCOLORVALUE *)_material_force_color.get_data();
      _pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
    } else {
      _pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    }
  }
  if (material->has_ambient()) {
    // If the material specifies an ambient color, use it.
    _pD3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
  } else {
    // Otherwise, the ambient color comes from the object color.
    if (_has_material_force_color) {
      cur_material.Ambient = *(D3DCOLORVALUE *)_material_force_color.get_data();
      _pD3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
    } else {
      _pD3DDevice->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
    }
  }

  _pD3DDevice->SetMaterial(&cur_material);

  if (material->get_local()) {
    _pD3DDevice->SetRenderState(D3DRS_LOCALVIEWER, TRUE);
  } else {
    _pD3DDevice->SetRenderState(D3DRS_LOCALVIEWER, FALSE);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::do_issue_texture
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
do_issue_texture() {
  DO_PSTATS_STUFF(_texture_state_pcollector.add_level(1));

  CPT(TextureAttrib) new_texture = _pending_texture->filter_to_max(_max_texture_stages);

  int num_stages = new_texture->get_num_on_stages();
  int num_old_stages = _current_texture->get_num_on_stages();

  nassertv(num_stages <= _max_texture_stages &&
           num_old_stages <= _max_texture_stages);

  _texture_involves_color_scale = false;

  int i;
  for (i = 0; i < num_stages; i++) {
    TextureStage *stage = new_texture->get_on_stage(i);
    Texture *texture = new_texture->get_on_texture(stage);
    nassertv(texture != (Texture *)NULL);

    if (i >= num_old_stages ||
        stage != _current_texture->get_on_stage(i) ||
        texture != _current_texture->get_on_texture(stage) ||
        stage->involves_color_scale()) {
      // Stage i has changed.  Issue the texture on this stage.
      TextureContext *tc = texture->prepare_now(_prepared_objects, this);
      apply_texture(i, tc);

      set_texture_blend_mode(i, stage);

      if (_current_tex_mat->has_stage(stage)) {
        // We have to reorder the elements of the matrix for some reason.
        const LMatrix4f &m = _current_tex_mat->get_mat(stage);
        LMatrix4f dm(m(0, 0), m(0, 1), m(0, 3), 0.0f,
                     m(1, 0), m(1, 1), m(1, 3), 0.0f,
                     m(3, 0), m(3, 1), m(3, 3), 0.0f,
                     0.0f, 0.0f, 0.0f, 1.0f);
        _pD3DDevice->SetTransform(get_tex_mat_sym(i), (D3DMATRIX *)dm.get_data());
        _pD3DDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS,
                                          D3DTTFF_COUNT2);
      } else {
        _pD3DDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS,
                                          D3DTTFF_DISABLE);
        // For some reason, "disabling" texture coordinate transforms
        // doesn't seem to be sufficient.  We'll load an identity matrix
        // to underscore the point.
        _pD3DDevice->SetTransform(get_tex_mat_sym(i), &matIdentity);
      }
    }
  }

  // Disable the texture stages that are no longer used.
  for (i = num_stages; i < num_old_stages; i++) {
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
  }

  _current_texture = new_texture;

  // Changing the set of texture stages will require us to reissue the
  // texgen and texmat attribs.
  _needs_tex_gen = true;
  _needs_tex_mat = true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::slot_new_light
//       Access: Protected, Virtual
//  Description: This will be called by the base class before a
//               particular light id will be used for the first time.
//               It is intended to allow the derived class to reserve
//               any additional resources, if required, for the new
//               light; and also to indicate whether the hardware
//               supports this many simultaneous lights.
//
//               The return value should be true if the additional
//               light is supported, or false if it is not.
////////////////////////////////////////////////////////////////////
bool DXGraphicsStateGuardian8::
slot_new_light(int light_id) {
  return ((unsigned int)light_id < _max_lights);
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
  _pD3DDevice->SetRenderState(D3DRS_LIGHTING, (DWORD)enable);
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

  _pD3DDevice->SetRenderState(D3DRS_AMBIENT, Colorf_to_D3DCOLOR(c));
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
  HRESULT hr = _pD3DDevice->LightEnable(light_id, enable);

  if (FAILED(hr)) {
    wdxdisplay8_cat.warning()
      << "Could not enable light " << light_id << ": "
      << D3DERRORSTRING(hr) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::slot_new_clip_plane
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
bool DXGraphicsStateGuardian8::
slot_new_clip_plane(int plane_id) {
  return ((unsigned int)plane_id < _max_clip_planes);
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
  _pD3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, _clip_plane_bits);
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

  HRESULT hr = _pD3DDevice->SetClipPlane(plane_id, world_plane.get_data());
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
set_blend_mode() {

  if ((_color_write_mode == ColorWriteAttrib::M_off) && !_pScrn->bCanDirectDisableColorWrites) {
    // need !_pScrn->bCanDirectDisableColorWrites guard because other
    // issue_colorblend, issue_transp will come this way, and they
    // should ignore the colorwriteattrib value since it's been
    // handled separately in set_color_writemask
    enable_blend(true);
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
    dxgsg8_cat.error()
      << "invalid transparency mode " << (int)_transparency_mode << endl;
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
  set_state(RenderState::make_empty());
  // want gsg to pass all state settings through

  _bDXisReady = false;

  if (_pD3DDevice!=NULL)
    for(int i=0;i<D3D_MAXTEXTURESTAGES;i++)
      _pD3DDevice->SetTexture(i, NULL);  // d3d should release this stuff internally anyway, but whatever

  release_all();

  if (_pD3DDevice!=NULL)
    RELEASE(_pD3DDevice, dxgsg8, "d3dDevice", RELEASE_DOWN_TO_ZERO);

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
    _cur_read_pixel_buffer=RenderBuffer::T_front;
  } else  if (rb._buffer_type & RenderBuffer::T_back) {
    _cur_read_pixel_buffer=RenderBuffer::T_back;
  } else {
    dxgsg8_cat.error() << "Invalid or unimplemented Argument to set_read_buffer!\n";
  }
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_smooth_state
//       Access: Protected, Static
//  Description: Returns a RenderState object that represents
//               smooth, per-vertex shading.
////////////////////////////////////////////////////////////////////
CPT(RenderState) DXGraphicsStateGuardian8::
get_smooth_state() {
  static CPT(RenderState) state;
  if (state == (RenderState *)NULL) {
    state = RenderState::make(ShadeModelAttrib::make(ShadeModelAttrib::M_smooth));
  }
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::get_flat_state
//       Access: Protected, Static
//  Description: Returns a RenderState object that represents
//               flat, per-primitive shading.
////////////////////////////////////////////////////////////////////
CPT(RenderState) DXGraphicsStateGuardian8::
get_flat_state() {
  static CPT(RenderState) state;
  if (state == (RenderState *)NULL) {
    state = RenderState::make(ShadeModelAttrib::make(ShadeModelAttrib::M_flat));
  }
  return state;
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
    _pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, false);

  } else {
    // If there is a scale, turn on normalization.
    _pD3DDevice->SetRenderState(D3DRS_NORMALIZENORMALS, true);
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
    if (FAILED( hr = _pD3DDevice->GetAvailableVidMem(&ddsCaps, &dwVidTotal, &dwVidFree))) {
      dxgsg8_cat.fatal() << "report_texmgr GetAvailableVidMem for VIDMEM failed : result = " << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
      return;
    }

    ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    if (FAILED( hr = _pD3DDevice->GetAvailableVidMem(&ddsCaps, &dwTexTotal, &dwTexFree))) {
      dxgsg8_cat.fatal() << "report_texmgr GetAvailableVidMem for TEXTURE failed : result = " << D3DERRORSTRING(hr);
      throw_event("panda3d-render-error");
      return;
    }
  }
#endif  // TEXMGRSTATS_USES_GETAVAILVIDMEM

  D3DDEVINFO_RESOURCEMANAGER all_resource_stats;
  ZeroMemory(&all_resource_stats, sizeof(D3DDEVINFO_RESOURCEMANAGER));

  if (!tex_stats_retrieval_impossible) {
    hr = _pD3DDevice->GetInfo(D3DDEVINFOID_RESOURCEMANAGER, &all_resource_stats, sizeof(D3DDEVINFO_RESOURCEMANAGER));
    if (hr!=D3D_OK) {
      if (hr==S_FALSE) {
        static int PrintedMsg=2;
        if (PrintedMsg>0) {
          if (dxgsg8_cat.is_debug()) {
            dxgsg8_cat.debug()
              << "texstats GetInfo() requires debug DX DLLs to be installed!!\n";
          }
          ZeroMemory(&all_resource_stats, sizeof(D3DDEVINFO_RESOURCEMANAGER));
          tex_stats_retrieval_impossible = true;
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
  _pScrn = new_context;
  _pD3DDevice = _pScrn->pD3DDevice;   //copy this one field for speed of deref
  _pSwapChain = _pScrn->pSwapChain;   //copy this one field for speed of deref
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::set_render_target
//       Access: Protected
//  Description: Set render target to the backbuffer of current swap
//               chain.
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
set_render_target() {
  LPDIRECT3DSURFACE8 pBack=NULL, pStencil=NULL;

  if (!_pSwapChain)  //maybe fullscreen mode or main/single window
    _pD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBack);
  else
    _pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBack);

  //wdxdisplay8_cat.debug() << "swapchain is " << _pSwapChain << "\n";
  //wdxdisplay8_cat.debug() << "back buffer is " << pBack << "\n";

  _pD3DDevice->GetDepthStencilSurface(&pStencil);
  _pD3DDevice->SetRenderTarget(pBack, pStencil);
  if (pBack) {
    pBack->Release();
  }
  if (pStencil) {
    pStencil->Release();
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
    // want to multiply tex-color*pixel color to emulate GL modulate blend (see glTexEnv)
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    break;

  case TextureStage::M_decal:
    // emulates GL_DECAL glTexEnv mode
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    break;

  case TextureStage::M_replace:
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    break;

  case TextureStage::M_add:
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_ADD);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    // since I'm making up 'add' mode, use modulate.  "adding" alpha
    // never makes sense right?
    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    _pD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    break;

  case TextureStage::M_blend:
    dxgsg8_cat.error()
      << "Impossible to emulate GL_BLEND in DX exactly.\n";
    break;

  default:
    dxgsg8_cat.error()
      << "Unknown texture blend mode " << (int)stage->get_mode() << endl;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::dx_cleanup
//       Access: Protected
//  Description: Clean up the DirectX environment, accounting for exit()
////////////////////////////////////////////////////////////////////
void DXGraphicsStateGuardian8::
dx_cleanup() {
  if (!_pD3DDevice)
    return;

  free_nondx_resources();
  PRINT_REFCNT(dxgsg8, _pD3DDevice);

  // Do a safe check for releasing the D3DDEVICE. RefCount should be zero.
  // if we're called from exit(), _pD3DDevice may already have been released
  RELEASE(_pD3DDevice, dxgsg8, "d3dDevice", RELEASE_DOWN_TO_ZERO);
  _pScrn->pD3DDevice = NULL;

  // Releasing pD3D is now the responsibility of the GraphicsPipe destructor
}

////////////////////////////////////////////////////////////////////
//     Function: DXGraphicsStateGuardian8::reset_d3d_device
//       Access: Protected
//  Description: This function checks current device's framebuffer
//               dimension against passed pPresParams backbuffer
//               dimension to determine a device reset if there is
//               only one window or it is the main window or
//               fullscreen mode then, it resets the device. Finally
//               it returns the new DXScreenData through parameter
//               pScrn
////////////////////////////////////////////////////////////////////
HRESULT DXGraphicsStateGuardian8::
reset_d3d_device(D3DPRESENT_PARAMETERS *pPresParams, DXScreenData **pScrn) {
  HRESULT hr;

  assert(IS_VALID_PTR(pPresParams));
  assert(IS_VALID_PTR(_pScrn->pD3D8));
  assert(IS_VALID_PTR(_pD3DDevice));

  release_all();

  // for windowed mode make sure our format matches the desktop fmt,
  // in case the desktop mode has been changed
  _pScrn->pD3D8->GetAdapterDisplayMode(_pScrn->CardIDNum, &_pScrn->DisplayMode);
  pPresParams->BackBufferFormat = _pScrn->DisplayMode.Format;

  // here we have to look at the _PresReset frame buffer dimension
  // if current window's dimension is bigger than _PresReset
  // we have to reset the device before creating new swapchain.
  // inorder to reset properly, we need to release all swapchains

  if (!(_pScrn->pSwapChain)
       || (_PresReset.BackBufferWidth < pPresParams->BackBufferWidth)
       || (_PresReset.BackBufferHeight < pPresParams->BackBufferHeight)) {

    wdxdisplay8_cat.debug() << "Swpachain = " << _pScrn->pSwapChain << " _PresReset = "
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
    if (pPresParams!=&_pScrn->PresParams)
      memcpy(&_pScrn->PresParams, pPresParams, sizeof(D3DPRESENT_PARAMETERS));
    return hr;
  }

  // release the old swapchain and create a new one
  if (_pScrn && _pScrn->pSwapChain) {
    _pScrn->pSwapChain->Release();
    wdxdisplay8_cat.debug() << "SwapChain " << _pScrn->pSwapChain << " is released\n";
    _pScrn->pSwapChain = NULL;
    hr=_pD3DDevice->CreateAdditionalSwapChain(pPresParams, &_pScrn->pSwapChain);
  }
  if (SUCCEEDED(hr)) {
    if (pPresParams!=&_pScrn->PresParams)
      memcpy(&_pScrn->PresParams, pPresParams, sizeof(D3DPRESENT_PARAMETERS));
    if (pScrn)
      *pScrn = _pScrn;
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
  HRESULT hr = _pD3DDevice->TestCooperativeLevel();

  if (SUCCEEDED(hr)) {
    assert(SUCCEEDED(_last_testcooplevel_result));
    return true;
  }

  switch(hr) {
  case D3DERR_DEVICENOTRESET:
    _bDXisReady = false;
    hr=reset_d3d_device(&_pScrn->PresParams);
    if (FAILED(hr)) {
      // I think this shouldnt fail unless I've screwed up the PresParams from the original working ones somehow
      dxgsg8_cat.error()
        << "check_cooperative_level Reset() failed, hr = " << D3DERRORSTRING(hr);
    }

    hr = _pD3DDevice->TestCooperativeLevel();
    if (FAILED(hr)) {
      // internal chk, shouldnt fail
      dxgsg8_cat.error()
        << "TestCooperativeLevel following Reset() failed, hr = " << D3DERRORSTRING(hr);

    }

    _bDXisReady = TRUE;
    break;

  case D3DERR_DEVICELOST:
    if (SUCCEEDED(_last_testcooplevel_result)) {
      if (_bDXisReady) {
        _bDXisReady = false;
        if (dxgsg8_cat.is_debug())
          dxgsg8_cat.debug() << "D3D Device was Lost, waiting...\n";
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
  if (_pD3DDevice == NULL) {
    return;
  }

  HRESULT hr;

  if (_pSwapChain) {
    hr = _pSwapChain->Present((CONST RECT*)NULL, (CONST RECT*)NULL, (HWND)NULL, NULL);
  } else {
    hr = _pD3DDevice->Present((CONST RECT*)NULL, (CONST RECT*)NULL, (HWND)NULL, NULL);
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
  hr = new_context->pD3DDevice->CreateAdditionalSwapChain(&new_context->PresParams, &new_context->pSwapChain);
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
  if (new_context->pSwapChain) {
    hr = new_context->pSwapChain->Release();
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
copy_pres_reset(DXScreenData *pScrn) {
  memcpy(&_PresReset, &_pScrn->PresParams, sizeof(D3DPRESENT_PARAMETERS));
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
//     Function: DXGraphicsStateGuardian8::get_tex_color_op1
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
D3DTEXTUREOP DXGraphicsStateGuardian8::
get_tex_color_op1(TextureStage::Mode mode) {
  switch (mode) {
  case TextureStage::M_modulate:
    return D3DTOP_MODULATE;

  case TextureStage::M_decal:
    return D3DTOP_BLENDTEXTUREALPHA;

  case TextureStage::M_blend:
    return D3DTOP_MODULATE;

  case TextureStage::M_replace:
    return D3DTOP_SELECTARG1;

  case TextureStage::M_add:
    return D3DTOP_ADD;
  }

  dxgsg8_cat.error()
    << "Invalid TextureStage::Mode value (" << (int)mode << ")\n";
  return D3DTOP_MODULATE;
}
