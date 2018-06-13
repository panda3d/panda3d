/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wdxGraphicsBuffer9.cxx
 * @author drose
 * @date 2004-02-08
 */

#include "wdxGraphicsPipe9.h"
#include "wdxGraphicsBuffer9.h"
#include "dxGraphicsStateGuardian9.h"
#include "pStatTimer.h"


#define FL << "\n" << __FILE__ << " " << __LINE__ << "\n"

using std::cout;
using std::endl;

TypeHandle wdxGraphicsBuffer9::_type_handle;


/**
 *
 */
wdxGraphicsBuffer9::
wdxGraphicsBuffer9(GraphicsEngine *engine, GraphicsPipe *pipe,
                   const std::string &name,
                   const FrameBufferProperties &fb_prop,
                   const WindowProperties &win_prop,
                   int flags,
                   GraphicsStateGuardian *gsg,
                   GraphicsOutput *host):
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  // initialize all class members
  _cube_map_index = -1;
  _saved_color_buffer = nullptr;
  _saved_depth_buffer = nullptr;
  _color_backing_store = nullptr;
  _depth_backing_store = nullptr;

  // is this correct ??? Since the pbuffer never gets flipped, we get
  // screenshots from the same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;

  _shared_depth_buffer = 0;
  _debug = 0;
  _this = 0;

  if (_debug) {
    cout << "+++++ wdxGraphicsBuffer9 constructor " << this << " " << this -> get_name ( ) << "\n";
  }

  if (_gsg) {
    // save to GSG list to handle device lost issues
    DXGraphicsStateGuardian9 *dxgsg;

    dxgsg = DCAST (DXGraphicsStateGuardian9, _gsg);
    _this = new (wdxGraphicsBuffer9 *);
    *_this = this;
    dxgsg -> _graphics_buffer_list.push_back(_this);
  }
}

/**
 *
 */
wdxGraphicsBuffer9::
~wdxGraphicsBuffer9() {

  if (_debug) {
    cout << "----- wdxGraphicsBuffer9 destructor " << this << " " << this -> get_name ( ) << "\n";
  }

  if (_gsg) {
    // remove from GSG list
    DXGraphicsStateGuardian9 *dxgsg;

    dxgsg = DCAST (DXGraphicsStateGuardian9, _gsg);
    if (_this) {
      dxgsg -> _graphics_buffer_list.remove(_this);
    }
    _this = 0;
    _gsg.clear();
    _gsg = 0;
  }

  // unshare shared depth buffer if any
  this -> unshare_depth_buffer();

  // unshare all buffers that are sharing this object's depth buffer
  {
    wdxGraphicsBuffer9 *graphics_buffer;
    std::list <wdxGraphicsBuffer9 *>::iterator graphics_buffer_iterator;

    graphics_buffer_iterator = _shared_depth_buffer_list.begin( );
    while (graphics_buffer_iterator != _shared_depth_buffer_list.end( )) {
      graphics_buffer = (*graphics_buffer_iterator);
      if (graphics_buffer) {
        // this call removes the entry from the list
        graphics_buffer -> unshare_depth_buffer();
      }
      graphics_buffer_iterator = _shared_depth_buffer_list.begin( );
    }
  }

  this -> close_buffer ( );
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool wdxGraphicsBuffer9::
begin_frame(FrameMode mode, Thread *current_thread) {

  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }
  if (_dxgsg -> _d3d_device == 0) {
    return false;
  }

  if (mode == FM_render) {
    if (!save_bitplanes()) {
      return false;
    }
    if (!rebuild_bitplanes()) {
      restore_bitplanes();
      return false;
    }
    clear_cube_map_selection();
  }

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void wdxGraphicsBuffer9::
end_frame(FrameMode mode, Thread *current_thread) {

  end_frame_spam(mode);
  nassertv(_gsg != nullptr);

  if (mode == FM_render) {
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
    restore_bitplanes();
  }
}

/**
 * After rendering, d3d_device will need to be restored to its initial state.
 * This function saves the state.
 */
bool wdxGraphicsBuffer9::
save_bitplanes() {
  HRESULT hr;
  DWORD render_target_index;

  render_target_index = 0;

  hr = _dxgsg -> _d3d_device -> GetRenderTarget (render_target_index, &_saved_color_buffer);
  if (!SUCCEEDED (hr)) {
    dxgsg9_cat.error ( ) << "GetRenderTarget " << D3DERRORSTRING(hr) FL;
    return false;
  }

  _saved_depth_buffer = 0;
  hr = _dxgsg -> _d3d_device -> GetDepthStencilSurface (&_saved_depth_buffer);
  if (hr == D3DERR_NOTFOUND) {
    // this case is actually ok
  }
  else {
    if (!SUCCEEDED (hr)) {
      dxgsg9_cat.error ( ) << "GetDepthStencilSurface " << D3DERRORSTRING(hr) FL;
      return false;
    }
  }
  return true;
}

/**
 * After rendering, d3d_device will need to be restored to its initial state.
 * This function restores the state.
 */
void wdxGraphicsBuffer9::
restore_bitplanes() {
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

  HRESULT hr;
  DWORD render_target_index;

  render_target_index = 0;

  hr = dxgsg -> _d3d_device ->
    SetRenderTarget (render_target_index, _saved_color_buffer);
  if (!SUCCEEDED (hr)) {
    dxgsg9_cat.error ( ) << "SetRenderTarget " << D3DERRORSTRING(hr) FL;
  }
  if (_saved_depth_buffer) {
    hr = _dxgsg -> _d3d_device -> SetDepthStencilSurface (_saved_depth_buffer);
    if (!SUCCEEDED (hr)) {
      dxgsg9_cat.error ( ) << "SetDepthStencilSurface " << D3DERRORSTRING(hr) FL;
    }
  }

  // clear all render targets, except for the main render target
  for (int i = 1; i<count_textures(); i++) {
    hr = _dxgsg -> _d3d_device -> SetRenderTarget (i, nullptr);
    if (!SUCCEEDED (hr)) {
      dxgsg9_cat.error ( ) << "SetRenderTarget " << i << " " << D3DERRORSTRING(hr) FL;
    }
  }

  _saved_color_buffer->Release();
  if (_saved_depth_buffer) {
    _saved_depth_buffer->Release();
  }
  _saved_color_buffer = nullptr;
  _saved_depth_buffer = nullptr;
}



/**
 * If necessary, reallocates (or allocates) the bitplanes for the buffer.
 */
bool wdxGraphicsBuffer9::
rebuild_bitplanes() {
  HRESULT hr;
  Texture *color_tex = 0;
  Texture *depth_tex = 0;
  DXTextureContext9 *color_ctx = 0;
  DXTextureContext9 *depth_ctx = 0;
  IDirect3DTexture9 *color_d3d_tex = 0;
  IDirect3DTexture9 *depth_d3d_tex = 0;
  IDirect3DCubeTexture9 *color_cube = 0;
  IDirect3DCubeTexture9 *depth_cube = 0;
  IDirect3DSurface9 *color_surf = 0;
  IDirect3DSurface9 *depth_surf = 0;
  DWORD render_target_index;

  render_target_index = 0;

  // Decide how big the bitplanes should be.

  if ((_host != 0)&&(_creation_flags & GraphicsPipe::BF_size_track_host)) {
    if (_host->get_size() != _size) {
      set_size_and_recalc(_host->get_x_size(),
                          _host->get_y_size());
    }
  }
  int bitplane_x = get_x_size();
  int bitplane_y = get_y_size();
  if (Texture::get_textures_power_2() != ATS_none) {
    bitplane_x = Texture::up_to_power_2(bitplane_x);
    bitplane_y = Texture::up_to_power_2(bitplane_y);
  }

  // Find the color and depth textures.  Either may be present, or neither.

  int color_tex_index = -1;
  int depth_tex_index = -1;
  {
    CDLockedReader cdata(_cycler);
    for (size_t i = 0; i != cdata->_textures.size(); ++i) {
      const RenderTexture &rt = cdata->_textures[i];
      RenderTextureMode rtm_mode = rt._rtm_mode;
      if (rtm_mode == RTM_bind_or_copy) {
        RenderTexturePlane plane = rt._plane;

        switch (plane) {
        case RTP_color:
          color_tex_index = i;
          break;

        case RTP_depth:
        case RTP_depth_stencil:
          depth_tex_index = i;
          break;

        case RTP_aux_rgba_0:
        case RTP_aux_rgba_1:
        case RTP_aux_rgba_2:
        case RTP_aux_rgba_3:
        case RTP_aux_hrgba_0:
        case RTP_aux_hrgba_1:
        case RTP_aux_hrgba_2:
        case RTP_aux_hrgba_3:
        case RTP_aux_float_0:
        case RTP_aux_float_1:
        case RTP_aux_float_2:
        case RTP_aux_float_3:
          {
            CDWriter cdataw(_cycler, cdata, false);
            cdataw->_textures[i]._rtm_mode = RTM_none;
          }
          // Creating the CDWriter invalidated the CDLockedReader.
          cdata = CDLockedReader(_cycler);
          break;
        default:
          {
            CDWriter cdataw(_cycler, cdata, false);
            cdataw->_textures[i]._rtm_mode = RTM_copy_texture;
          }
          // Creating the CDWriter invalidated the CDLockedReader.
          cdata = CDLockedReader(_cycler);
          break;
        }
      }
    }
  }

  if (color_tex_index < 0) {
    // Maintain the backing color surface.
    if ((_color_backing_store)&&
        ((bitplane_x != _backing_sizex)||(bitplane_y != _backing_sizey))) {
      _color_backing_store->Release();
      _color_backing_store = nullptr;
    }
    if (!_color_backing_store) {
      hr = _dxgsg->_d3d_device->CreateRenderTarget(bitplane_x, bitplane_y,
                                                   _saved_color_desc.Format,
                                                   _saved_color_desc.MultiSampleType,
                                                   _saved_color_desc.MultiSampleQuality,
                                                   FALSE,
                                                   &_color_backing_store,
                                                   nullptr);
      if (!SUCCEEDED(hr)) {
        dxgsg9_cat.error ( ) << "CreateRenderTarget " << D3DERRORSTRING(hr) FL;
      }
    }
    color_surf = _color_backing_store;
  } else {
    // Maintain the color texture.
    if (_color_backing_store) {
      _color_backing_store->Release();
      _color_backing_store = nullptr;
    }
    color_tex = get_texture(color_tex_index);
    color_tex->set_size_padded(get_x_size(), get_y_size());
// color_tex->set_format(Texture::F_rgba);
    color_ctx =
      DCAST(DXTextureContext9,
            color_tex->prepare_now(0, _gsg->get_prepared_objects(), _gsg));

    if (color_ctx) {
      if (!color_ctx->create_texture(*_dxgsg->_screen)) {
        dxgsg9_cat.error()
          << "Unable to re-create texture " << *color_ctx->get_texture() << endl;
        return false;
      }
      if (color_tex->get_texture_type() == Texture::TT_2d_texture) {
        color_d3d_tex = color_ctx->_d3d_2d_texture;
        nassertr(color_d3d_tex != 0, false);
        hr = color_d3d_tex -> GetSurfaceLevel(0, &color_surf);
        if (!SUCCEEDED(hr)) {
          dxgsg9_cat.error ( ) << "GetSurfaceLevel " << D3DERRORSTRING(hr) FL;
        }
      }
      if (color_tex->get_texture_type() == Texture::TT_cube_map) {
        color_cube = color_ctx->_d3d_cube_texture;
        nassertr(color_cube != 0, false);

        if (_cube_map_index >= 0 && _cube_map_index < 6) {
          hr = color_cube -> GetCubeMapSurface ((D3DCUBEMAP_FACES) _cube_map_index, 0, &color_surf);
          if (!SUCCEEDED(hr)) {
            dxgsg9_cat.error ( ) << "GetCubeMapSurface " << D3DERRORSTRING(hr) FL;
          }
        }
      }
    }
  }

  bool release_depth;

  release_depth = true;
  if (depth_tex_index < 0) {
    if (_shared_depth_buffer) {
      if (_shared_depth_buffer -> _depth_backing_store) {
        if (_debug) {
          printf ("SHARE DEPTH BUFFER\n");
        }
        depth_surf = _shared_depth_buffer -> _depth_backing_store;
        release_depth = false;
      }
    }
    if (depth_surf == 0) {
      // Maintain the backing depth surface.
      if ((_depth_backing_store)&&
          ((bitplane_x != _backing_sizex)||(bitplane_y != _backing_sizey))) {
        _depth_backing_store->Release();
        _depth_backing_store = nullptr;
      }
      if (!_depth_backing_store) {
        hr = _dxgsg -> _d3d_device ->
          CreateDepthStencilSurface (bitplane_x, bitplane_y, _saved_depth_desc.Format,
                                     _saved_depth_desc.MultiSampleType, _saved_depth_desc.MultiSampleQuality,
                                     false, &_depth_backing_store, nullptr);
        if (!SUCCEEDED(hr)) {
          dxgsg9_cat.error ( ) << "CreateDepthStencilSurface " << D3DERRORSTRING(hr) FL;
        }
      }
      depth_surf = _depth_backing_store;
    }
  } else {
    // Maintain the depth texture.
    if (_depth_backing_store) {
      _depth_backing_store->Release();
      _depth_backing_store = nullptr;
    }

    if (_shared_depth_buffer) {
      depth_tex = _shared_depth_buffer -> get_texture(depth_tex_index);
    }
    if (depth_tex == 0) {
      depth_tex = get_texture(depth_tex_index);
    }

    depth_tex->set_size_padded(get_x_size(), get_y_size());
    depth_tex->set_format(Texture::F_depth_stencil);
    depth_ctx =
      DCAST(DXTextureContext9,
            depth_tex->prepare_now(0, _gsg->get_prepared_objects(), _gsg));
    if (depth_ctx) {
      if (!depth_ctx->create_texture(*_dxgsg->_screen)) {
        dxgsg9_cat.error()
          << "Unable to re-create texture " << *depth_ctx->get_texture() << endl;
        return false;
      }
      if (depth_tex->get_texture_type() == Texture::TT_2d_texture) {
        depth_d3d_tex = depth_ctx->_d3d_2d_texture;
        nassertr(depth_d3d_tex != 0, false);
        hr = depth_d3d_tex -> GetSurfaceLevel(0, &depth_surf);
        if (!SUCCEEDED(hr)) {
          dxgsg9_cat.error ( ) << "GetSurfaceLevel " << D3DERRORSTRING(hr) FL;
        }
      }
      if (depth_tex->get_texture_type() == Texture::TT_cube_map) {
        depth_cube = depth_ctx->_d3d_cube_texture;
        nassertr(depth_cube != 0, false);
        hr = depth_cube -> GetCubeMapSurface ((D3DCUBEMAP_FACES) _cube_map_index, 0, &depth_surf);
        if (!SUCCEEDED(hr)) {
          dxgsg9_cat.error ( ) << "GetCubeMapSurface " << D3DERRORSTRING(hr) FL;
        }
      }
    }
  }

  _backing_sizex = bitplane_x;
  _backing_sizey = bitplane_y;

  // Load up the bitplanes.
  if (color_surf) {
    hr = _dxgsg -> _d3d_device -> SetRenderTarget (render_target_index, color_surf);
    if (!SUCCEEDED (hr)) {
      dxgsg9_cat.error ( ) << "SetRenderTarget " << D3DERRORSTRING(hr) FL;
    }
  }

  if (depth_surf) {
    hr = _dxgsg -> _d3d_device -> SetDepthStencilSurface (depth_surf);
    if (!SUCCEEDED (hr)) {
      dxgsg9_cat.error ( ) << "SetDepthStencilSurface " << D3DERRORSTRING(hr) FL;
    }
  }

  render_target_index = 1;
  for (int i=0; i<count_textures(); i++) {

    Texture *tex = get_texture(i);
    RenderTexturePlane plane = get_texture_plane(i);

    if (_debug) {
// printf ("i = %d, RenderTexturePlane = %d \n", i, plane);
    }

    switch (plane) {
      case RTP_color:
        break;
      case RTP_aux_rgba_0:
      case RTP_aux_rgba_1:
      case RTP_aux_rgba_2:
      case RTP_aux_rgba_3:
      case RTP_aux_hrgba_0:
      case RTP_aux_hrgba_1:
      case RTP_aux_hrgba_2:
      case RTP_aux_hrgba_3:
      case RTP_aux_float_0:
      case RTP_aux_float_1:
      case RTP_aux_float_2:
      case RTP_aux_float_3:
        {
          DXTextureContext9 *color_ctx = 0;
          IDirect3DTexture9 *color_d3d_tex = 0;
          IDirect3DSurface9 *color_surf = 0;
          IDirect3DCubeTexture9 *color_cube = 0;

          color_ctx = DCAST(DXTextureContext9, tex->prepare_now(0, _gsg->get_prepared_objects(), _gsg));
          if (color_ctx) {
            if (!color_ctx->create_texture(*_dxgsg->_screen)) {
              dxgsg9_cat.error()
                << "Unable to re-create texture " << *color_ctx->get_texture() << endl;
              return false;
            }
            if (tex->get_texture_type() == Texture::TT_2d_texture) {
              color_d3d_tex = color_ctx->_d3d_2d_texture;
              nassertr(color_d3d_tex != 0, false);

              hr = color_d3d_tex -> GetSurfaceLevel(0, &color_surf);
              if (!SUCCEEDED(hr)) {
                dxgsg9_cat.error ( ) << "GetSurfaceLevel " << D3DERRORSTRING(hr) FL;
              }
              if (color_surf) {
                hr = _dxgsg -> _d3d_device -> SetRenderTarget (render_target_index, color_surf);
                if (SUCCEEDED (hr)) {
                  render_target_index++;
                } else {
                  dxgsg9_cat.error ( ) << "SetRenderTarget " << render_target_index << " " << D3DERRORSTRING(hr) FL;
                }
                color_surf->Release();
              }
            }
          }
        }
        break;

      default:
        break;
    }
  }

  // Decrement the reference counts on these surfaces.  The refcounts were
  // incremented earlier when we called GetSurfaceLevel.

  if ((color_surf != 0)&&(color_surf != _color_backing_store)) {
    color_surf->Release();
  }

  if (release_depth) {
    if ((depth_surf != 0)&&(depth_surf != _depth_backing_store)) {
      depth_surf->Release();
    }
  }

  return true;
}

/**
 * Called internally when the window is in render-to-a-texture mode and we are
 * in the process of rendering the six faces of a cube map.  This should do
 * whatever needs to be done to switch the buffer to the indicated face.
 */
void wdxGraphicsBuffer9::
select_target_tex_page(int page) {

  DWORD render_target_index;

  render_target_index = 0;

  _cube_map_index = page;

  HRESULT hr;
  Texture *color_tex = 0;
  DXTextureContext9 *color_ctx = 0;
  IDirect3DCubeTexture9 *color_cube = 0;
  IDirect3DSurface9 *color_surf = 0;
  int color_tex_index = -1;

  {
    CDLockedReader cdata(_cycler);
    for (size_t i = 0; i != cdata->_textures.size(); ++i) {
      const RenderTexture &rt = cdata->_textures[i];
      RenderTextureMode rtm_mode = rt._rtm_mode;
      if (rtm_mode == RTM_bind_or_copy) {
        Texture *tex = rt._texture;
        if ((tex->get_format() != Texture::F_depth_stencil)&&
            (tex->get_format() != Texture::F_depth_component)&&
            (color_tex_index < 0)) {
          color_tex_index = i;
        } else {
          CDWriter cdataw(_cycler, cdata, false);
          nassertv(cdata->_textures.size() == cdataw->_textures.size());
          cdataw->_textures[i]._rtm_mode = RTM_copy_texture;
        }
      }
    }
  }

  color_tex = get_texture(color_tex_index);
  if (color_tex) {
    color_ctx =
      DCAST(DXTextureContext9,
            color_tex->prepare_now(0, _gsg->get_prepared_objects(), _gsg));
    color_cube = color_ctx->_d3d_cube_texture;
    if (color_cube && _cube_map_index >= 0 && _cube_map_index < 6) {
      hr = color_cube -> GetCubeMapSurface ((D3DCUBEMAP_FACES) _cube_map_index, 0, &color_surf);
      if (!SUCCEEDED(hr)) {
        dxgsg9_cat.error ( ) << "GetCubeMapSurface " << D3DERRORSTRING(hr) FL;
      }

      hr = _dxgsg -> _d3d_device -> SetRenderTarget (render_target_index, color_surf);
      if (!SUCCEEDED (hr)) {
        dxgsg9_cat.error ( ) << "SetRenderTarget " << D3DERRORSTRING(hr) FL;
      }
      else {
        color_surf->Release();
      }
    }
  }

  render_target_index = 1;
  for (int i=0; i<count_textures(); i++) {

    Texture *tex = get_texture(i);
    RenderTexturePlane plane = get_texture_plane(i);

    switch (plane) {
      case RTP_color:
        break;
      case RTP_aux_rgba_0:
      case RTP_aux_rgba_1:
      case RTP_aux_rgba_2:
      case RTP_aux_rgba_3:
      case RTP_aux_hrgba_0:
      case RTP_aux_hrgba_1:
      case RTP_aux_hrgba_2:
      case RTP_aux_hrgba_3:
      case RTP_aux_float_0:
      case RTP_aux_float_1:
      case RTP_aux_float_2:
      case RTP_aux_float_3:
        {
          DXTextureContext9 *color_ctx = 0;
          IDirect3DSurface9 *color_surf = 0;
          IDirect3DCubeTexture9 *color_cube = 0;

          color_ctx = DCAST(DXTextureContext9, tex->prepare_now(0, _gsg->get_prepared_objects(), _gsg));
          if (color_ctx) {
            if (tex->get_texture_type() == Texture::TT_cube_map) {

              if (_debug) {
                printf ("CUBEMAP i = %d, RenderTexturePlane = %d, _cube_map_index %d \n", i, plane, _cube_map_index);
              }

              color_cube = color_ctx->_d3d_cube_texture;
              if (color_cube && _cube_map_index >= 0 && _cube_map_index < 6) {
                hr = color_cube -> GetCubeMapSurface ((D3DCUBEMAP_FACES) _cube_map_index, 0, &color_surf);
                if (!SUCCEEDED(hr)) {
                  dxgsg9_cat.error ( ) << "GetCubeMapSurface " << D3DERRORSTRING(hr) FL;
                }
                if (color_surf) {
                  hr = _dxgsg -> _d3d_device -> SetRenderTarget (render_target_index, color_surf);
                  if (SUCCEEDED (hr)) {
                    render_target_index++;
                  } else {
                    dxgsg9_cat.error ( ) << "cube map SetRenderTarget " << render_target_index << " " << D3DERRORSTRING(hr) FL;
                  }
                  color_surf->Release();
                }
              }
            }
          }
        }
        break;

      default:
        break;
    }
  }
}

/**
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties()
 *
 * This function is called only within the window thread.
 */
void wdxGraphicsBuffer9::
process_events() {
  GraphicsBuffer::process_events();

  MSG msg;

  // Handle all the messages on the queue in a row.  Some of these might be
  // for another window, but they will get dispatched appropriately.
  while (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)) {
    process_1_event();
  }
}

/**
 * Closes the buffer right now.  Called from the window thread.
 */
void wdxGraphicsBuffer9::
close_buffer() {

  if (_color_backing_store) {
    _color_backing_store->Release();
    _color_backing_store = nullptr;
  }
  if (_depth_backing_store) {
    _depth_backing_store->Release();
    _depth_backing_store = nullptr;
  }

  _cube_map_index = -1;
  _is_valid = false;
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool wdxGraphicsBuffer9::
open_buffer() {

  // GSG creationinitialization.
  if (_gsg == 0) {
    // The code below doesn't support creating a GSG on the fly.  Just error
    // out for now.  _dxgsg = new DXGraphicsStateGuardian9(_engine, _pipe);
    // _gsg = _dxgsg;
    return false;
  }

  DCAST_INTO_R(_dxgsg, _gsg, false);

  if (!save_bitplanes()) {
    return false;
  }

  HRESULT hr;
  hr = _saved_color_buffer -> GetDesc (&_saved_color_desc);
  if (!SUCCEEDED (hr)) {
    dxgsg9_cat.error ( ) << "GetDesc " << D3DERRORSTRING(hr) FL;
    return false;
  }
  hr = _saved_depth_buffer -> GetDesc (&_saved_depth_desc);
  if (!SUCCEEDED (hr)) {
    dxgsg9_cat.error ( ) << "GetDesc " << D3DERRORSTRING(hr) FL;
    return false;
  }
  _fb_properties = _dxgsg->
    calc_fb_properties(_saved_color_desc.Format,
                       _saved_depth_desc.Format,
                       _saved_depth_desc.MultiSampleType,
                       _saved_depth_desc.MultiSampleQuality);
  _fb_properties.set_force_hardware(1); // Wild guess.


  if (!rebuild_bitplanes()) {
    restore_bitplanes();
    return false;
  }

  restore_bitplanes();
  return true;
}

/**
 * Handles one event from the message queue.
 */
void wdxGraphicsBuffer9::
process_1_event() {
  MSG msg;

  if (!GetMessage(&msg, nullptr, 0, 0)) {
    // WM_QUIT received.  We need a cleaner way to deal with this.
    // DestroyAllWindows(false);
    exit(msg.wParam);  // this will invoke AtExitFn
  }

  // Translate virtual key messages
  TranslateMessage(&msg);
  // Call window_proc
  DispatchMessage(&msg);
}




/**
 * Will attempt to use the depth buffer of the input graphics_output.  The
 * buffer sizes must be exactly the same.
 */
bool wdxGraphicsBuffer9::
share_depth_buffer(GraphicsOutput *graphics_output) {

  bool state;
  wdxGraphicsBuffer9 *input_graphics_output;

  state = false;
  input_graphics_output = DCAST (wdxGraphicsBuffer9, graphics_output);
  if (this != input_graphics_output && input_graphics_output) {

    state = true;
    this -> unshare_depth_buffer();

    if (_debug) {
      printf ("share_depth_buffer\n");
    }

    // check buffer sizes
    if (this -> get_x_size() != input_graphics_output -> get_x_size()) {
      if (_debug) {
        printf ("ERROR: share_depth_buffer: non matching width \n");
      }
      state = false;
    }

    if (this -> get_y_size() != input_graphics_output -> get_y_size()) {
      if (_debug) {
        printf ("ERROR: share_depth_buffer: non matching height \n");
      }
      state = false;
    }

    if (state) {
      // let the input GraphicsOutput know that there is an object sharing its
      // depth buffer
      input_graphics_output -> register_shared_depth_buffer(this);
      _shared_depth_buffer = input_graphics_output;
      state = true;
    }
  }

  return state;
}

/**
 * Discontinue sharing the depth buffer.
 */
void wdxGraphicsBuffer9::
unshare_depth_buffer() {
  if (_shared_depth_buffer) {
    if (_debug) {
      printf ("wdxGraphicsBuffer9 unshare_depth_buffer \n");
    }

    // let the GraphicsOutput know that this object is no longer sharing its
    // depth buffer
    _shared_depth_buffer -> unregister_shared_depth_buffer(this);
    _shared_depth_buffer = 0;
  }
}

/**
 * Register/save who is sharing the depth buffer.
 */
void wdxGraphicsBuffer9::
register_shared_depth_buffer(GraphicsOutput *graphics_output) {
  wdxGraphicsBuffer9 *input_graphics_output;

  input_graphics_output = DCAST (wdxGraphicsBuffer9, graphics_output);
  if (input_graphics_output) {
    // add to list
    _shared_depth_buffer_list.push_back(input_graphics_output);
  }
}

/**
 * Unregister who is sharing the depth buffer.
 */
void wdxGraphicsBuffer9::
unregister_shared_depth_buffer(GraphicsOutput *graphics_output) {
  wdxGraphicsBuffer9 *input_graphics_output;

  input_graphics_output = DCAST (wdxGraphicsBuffer9, graphics_output);
  if (input_graphics_output) {
    // remove from list
    _shared_depth_buffer_list.remove(input_graphics_output);
  }
}
