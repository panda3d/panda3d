// Filename: wdxGraphicsBuffer8.cxx
// Created by:  drose (08Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2005, Disney Enterprises, Inc.  All rights reserved
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

#include "wdxGraphicsPipe9.h"
#include "wdxGraphicsBuffer9.h"
#include "dxGraphicsStateGuardian9.h"
#include "pStatTimer.h"


#define FL << "\n" << __FILE__ << " " << __LINE__ << "\n"

TypeHandle wdxGraphicsBuffer9::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsBuffer9::
wdxGraphicsBuffer9(GraphicsPipe *pipe,
                   const string &name,
                   const FrameBufferProperties &fb_prop,
                   const WindowProperties &win_prop,
                   int flags,
                   GraphicsStateGuardian *gsg,
                   GraphicsOutput *host):
  GraphicsBuffer(pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  // initialize all class members
  _cube_map_index = -1;
  _saved_color_buffer = NULL;
  _saved_depth_buffer = NULL;
  _color_backing_store = NULL;
  _depth_backing_store = NULL;

  // is this correct ???
  // Since the pbuffer never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;

  if (_gsg) {
    // save to GSG list to handle device lost issues
    DXGraphicsStateGuardian9 *dxgsg;

    dxgsg = DCAST (DXGraphicsStateGuardian9, _gsg);
    dxgsg -> _graphics_buffer_list.push_back(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsBuffer9::
~wdxGraphicsBuffer9() {

  if (_gsg) {
    // remove from GSG list
    DXGraphicsStateGuardian9 *dxgsg;

    dxgsg = DCAST (DXGraphicsStateGuardian9, _gsg);
    dxgsg -> _graphics_buffer_list.remove(this);
  }

  this -> close_buffer ( );
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsBuffer9::
begin_frame(FrameMode mode, Thread *current_thread) {

  begin_frame_spam();
  if (_gsg == (GraphicsStateGuardian *)NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer9::
end_frame(FrameMode mode, Thread *current_thread) {

  end_frame_spam();
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    if (_one_shot) {
      prepare_for_deletion();
    }
    clear_cube_map_selection();
    restore_bitplanes();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::save_bitplanes
//       Access: Public
//  Description: After rendering, d3d_device will need to be restored
//               to its initial state.  This function saves the state.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsBuffer9::
save_bitplanes() {
  HRESULT hr;

  hr = _dxgsg -> _d3d_device -> GetRenderTarget (0, &_saved_color_buffer);
  if (!SUCCEEDED (hr)) {
    dxgsg9_cat.error ( ) << "GetRenderTarget " << D3DERRORSTRING(hr) FL;
    return false;
  }
  hr = _dxgsg -> _d3d_device -> GetDepthStencilSurface (&_saved_depth_buffer);
  if (!SUCCEEDED (hr)) {
    dxgsg9_cat.error ( ) << "GetDepthStencilSurface " << D3DERRORSTRING(hr) FL;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::restore_bitplanes
//       Access: Public
//  Description: After rendering, d3d_device will need to be restored
//               to its initial state.  This function restores the state.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer9::
restore_bitplanes() {
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

  HRESULT hr;

  hr = dxgsg -> _d3d_device ->
    SetRenderTarget (0, _saved_color_buffer);
  if (!SUCCEEDED (hr)) {
    dxgsg9_cat.error ( ) << "SetRenderTarget " << D3DERRORSTRING(hr) FL;
  }
  hr = _dxgsg -> _d3d_device -> SetDepthStencilSurface (_saved_depth_buffer);
  if (!SUCCEEDED (hr)) {
    dxgsg9_cat.error ( ) << "SetDepthStencilSurface " << D3DERRORSTRING(hr) FL;
  }

  _saved_color_buffer->Release();
  _saved_depth_buffer->Release();
  _saved_color_buffer = NULL;
  _saved_depth_buffer = NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::rebuild_bitplanes
//       Access: Public
//  Description: If necessary, reallocates (or allocates) the
//               bitplanes for the buffer.
////////////////////////////////////////////////////////////////////
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

  // Decide how big the bitplanes should be.

  int bitplane_x = _x_size;
  int bitplane_y = _y_size;
  if (!_gsg->get_supports_tex_non_pow2()) {
    bitplane_x = Texture::up_to_power_2(bitplane_x);
    bitplane_y = Texture::up_to_power_2(bitplane_y);
  }

  // Find the color and depth textures.  Either may be present,
  // or neither.
  //
  // NOTE: Currently, depth-stencil textures are not implemented,
  // but since it's coming soon, we're structuring for it.

  int color_tex_index = -1;
  int depth_tex_index = -1;
  for (int i=0; i<count_textures(); i++) {
    if (get_rtm_mode(i) == RTM_bind_or_copy) {
      if ((get_texture(i)->get_format() != Texture::F_depth_stencil)&&
          (color_tex_index < 0)) {
        color_tex_index = i;
      } else {
        _textures[i]._rtm_mode = RTM_copy_texture;
      }
    }
  }


  if (color_tex_index < 0) {
    // Maintain the backing color surface.
    if ((_color_backing_store)&&
        ((bitplane_x != _backing_sizex)||(bitplane_y != _backing_sizey))) {
      _color_backing_store->Release();
      _color_backing_store = NULL;
    }
    if (!_color_backing_store) {
      hr = _dxgsg -> _d3d_device ->
        CreateOffscreenPlainSurface(bitplane_x, bitplane_y, _saved_color_desc.Format,
                                    D3DPOOL_DEFAULT, &_color_backing_store, NULL);
      if (!SUCCEEDED(hr)) {
        dxgsg9_cat.error ( ) << "CreateImageSurface " << D3DERRORSTRING(hr) FL;
      }
    }
    color_surf = _color_backing_store;
  } else {
    // Maintain the color texture.
    if (_color_backing_store) {
      _color_backing_store->Release();
      _color_backing_store = NULL;
    }
    color_tex = get_texture(color_tex_index);
    color_tex->set_x_size(bitplane_x);
    color_tex->set_y_size(bitplane_y);
    color_tex->set_format(Texture::F_rgba);
    color_ctx =
      DCAST(DXTextureContext9,
            color_tex->prepare_now(_gsg->get_prepared_objects(), _gsg));
    if (color_ctx) {
      if (color_tex->get_texture_type() == Texture::TT_2d_texture) {
        color_d3d_tex = color_ctx->_d3d_2d_texture;
        nassertr(color_d3d_tex != 0, false);
        hr = color_d3d_tex -> GetSurfaceLevel(0, &color_surf);
        if (!SUCCEEDED(hr)) {
          dxgsg9_cat.error ( ) << "GetSurfaceLevel " << D3DERRORSTRING(hr) FL;
        }
      } else {
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

  if (depth_tex_index < 0) {
    // Maintain the backing depth surface.
    if ((_depth_backing_store)&&
        ((bitplane_x != _backing_sizex)||(bitplane_y != _backing_sizey))) {
      _depth_backing_store->Release();
      _depth_backing_store = NULL;
    }
    if (!_depth_backing_store) {
      hr = _dxgsg -> _d3d_device ->
        CreateDepthStencilSurface (bitplane_x, bitplane_y, _saved_depth_desc.Format,
                                   _saved_depth_desc.MultiSampleType, _saved_depth_desc.MultiSampleQuality,
                                   false, &_depth_backing_store, NULL);
      if (!SUCCEEDED(hr)) {
        dxgsg9_cat.error ( ) << "CreateDepthStencilSurface " << D3DERRORSTRING(hr) FL;
      }
    }
    depth_surf = _depth_backing_store;
  } else {
    // Maintain the depth texture.
    if (_depth_backing_store) {
      _depth_backing_store->Release();
      _depth_backing_store = NULL;
    }
    depth_tex = get_texture(depth_tex_index);
    depth_tex->set_x_size(bitplane_x);
    depth_tex->set_y_size(bitplane_y);
    depth_tex->set_format(Texture::F_depth_stencil);
    depth_ctx =
      DCAST(DXTextureContext9,
            depth_tex->prepare_now(_gsg->get_prepared_objects(), _gsg));
    if (depth_ctx) {
      if (depth_tex->get_texture_type() == Texture::TT_2d_texture) {
        depth_d3d_tex = depth_ctx->_d3d_2d_texture;
        nassertr(depth_d3d_tex != 0, false);
        hr = color_d3d_tex -> GetSurfaceLevel(0, &depth_surf);
        if (!SUCCEEDED(hr)) {
          dxgsg9_cat.error ( ) << "GetSurfaceLevel " << D3DERRORSTRING(hr) FL;
        }
      } else {
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
    hr = _dxgsg -> _d3d_device -> SetRenderTarget (0, color_surf);
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

  // Decrement the reference counts on these surfaces. The refcounts
  // were incremented earlier when we called GetSurfaceLevel.

  if ((color_surf != 0)&&(color_surf != _color_backing_store)) {
    color_surf->Release();
  }
  if ((depth_surf != 0)&&(depth_surf != _depth_backing_store)) {
    depth_surf->Release();
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::select_cube_map
//       Access: Public, Virtual
//  Description: Called internally when the window is in
//               render-to-a-texture mode and we are in the process of
//               rendering the six faces of a cube map.  This should
//               do whatever needs to be done to switch the buffer to
//               the indicated face.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer9::
select_cube_map(int cube_map_index) {

  _cube_map_index = cube_map_index;

  HRESULT hr;
  Texture *color_tex = 0;
  DXTextureContext9 *color_ctx = 0;
  IDirect3DCubeTexture9 *color_cube = 0;
  IDirect3DSurface9 *color_surf = 0;
  int color_tex_index = -1;

  for (int i=0; i<count_textures(); i++) {
    if (get_rtm_mode(i) == RTM_bind_or_copy) {
      if ((get_texture(i)->get_format() != Texture::F_depth_stencil)&&
          (color_tex_index < 0)) {
        color_tex_index = i;
      } else {
        _textures[i]._rtm_mode = RTM_copy_texture;
      }
    }
  }

  color_tex = get_texture(color_tex_index);
  if (color_tex) {
    color_ctx =
      DCAST(DXTextureContext9,
            color_tex->prepare_now(_gsg->get_prepared_objects(), _gsg));

    color_cube = color_ctx->_d3d_cube_texture;

    if (color_cube && _cube_map_index >= 0 && _cube_map_index < 6) {
      hr = color_cube -> GetCubeMapSurface ((D3DCUBEMAP_FACES) _cube_map_index, 0, &color_surf);
      if (!SUCCEEDED(hr)) {
        dxgsg9_cat.error ( ) << "GetCubeMapSurface " << D3DERRORSTRING(hr) FL;
      }

      hr = _dxgsg -> _d3d_device -> SetRenderTarget (0, color_surf);
      if (!SUCCEEDED (hr)) {
        dxgsg9_cat.error ( ) << "SetRenderTarget " << D3DERRORSTRING(hr) FL;
      }
      else {
        color_surf->Release();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer9::
process_events() {
  GraphicsBuffer::process_events();

  MSG msg;

  // Handle all the messages on the queue in a row.  Some of these
  // might be for another window, but they will get dispatched
  // appropriately.
  while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
    process_1_event();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer9::
close_buffer() {

  if (_gsg != (GraphicsStateGuardian *)NULL) {
    _gsg.clear();
  }

  if (_color_backing_store) {
    _color_backing_store->Release();
    _color_backing_store = NULL;
  }
  if (_depth_backing_store) {
    _depth_backing_store->Release();
    _depth_backing_store = NULL;
  }

  _active = false;
  _cube_map_index = -1;
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsBuffer9::
open_buffer() {

  // GSG creation/initialization.
  if (_gsg == 0) {
    _dxgsg = new DXGraphicsStateGuardian9(_pipe);
    _gsg = _dxgsg;
  } else {
    DCAST_INTO_R(_dxgsg, _gsg, false);
  }

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

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::process_1_event
//       Access: Private, Static
//  Description: Handles one event from the message queue.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer9::
process_1_event() {
  MSG msg;

  if (!GetMessage(&msg, NULL, 0, 0)) {
    // WM_QUIT received.  We need a cleaner way to deal with this.
    //    DestroyAllWindows(false);
    exit(msg.wParam);  // this will invoke AtExitFn
  }

  // Translate virtual key messages
  TranslateMessage(&msg);
  // Call window_proc
  DispatchMessage(&msg);
}
