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

#include "wdxGraphicsPipe8.h"
#include "wdxGraphicsBuffer8.h"
#include "pStatTimer.h"


// ISSUES:
  // size issues
    // texture size must not exeeed the original frame buffer width
    // or height due to the size of the original depth/stencil buffer
  // render to texure format
    // can be specified via the DXGraphicsStateGuardian8 member
    // _render_to_texture_d3d_format  default = D3DFMT_X8R8G8B8

  // LOST DEVICE case for D3DPOOL_DEFAULT buffers
  // should check texture creation with CheckDepthStencilMatch
  // support copy from texture to ram?
    // check D3DCAPS2_DYNAMICTEXTURES

TypeHandle wdxGraphicsBuffer8::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsBuffer8::
wdxGraphicsBuffer8(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  const string &name,
                  int x_size, int y_size) :
  GraphicsBuffer(pipe, gsg, name, x_size, y_size)
{
  // initialize all class members
  _cube_map_index = -1;
  _back_buffer = NULL;
  _z_stencil_buffer = NULL;
  _direct_3d_surface = NULL;
  _dx_texture_context8 = NULL;

// is this correct ???
  // Since the pbuffer never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsBuffer8::
~wdxGraphicsBuffer8() {
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsBuffer8::
begin_frame() {
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  DXGraphicsStateGuardian8 *dxgsg;
  DCAST_INTO_R(dxgsg, _gsg, false);

  return GraphicsBuffer::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsStateGuardian::begin_render_texture
//       Access: Public, Virtual
//  Description: If the GraphicsOutput supports direct render-to-texture,
//               and if any setup needs to be done during begin_frame,
//               then the setup code should go here.  Any textures that
//               can not be rendered to directly should be reflagged
//               as RTM_copy_texture.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer8::
begin_render_texture() {
  DXGraphicsStateGuardian8 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

  HRESULT hr;
  bool state;

  state = false;

  // save render context
  hr = dxgsg -> _d3d_device -> GetRenderTarget (&_back_buffer);
  if (SUCCEEDED (hr)) {
    hr = dxgsg -> _d3d_device -> GetDepthStencilSurface (&_z_stencil_buffer);
    if (SUCCEEDED (hr)) {
      state = true;
    }
  }

  // set render context
  if (state && _dx_texture_context8)
  {
    int tex_index;
    DXTextureContext8 *dx_texture_context8;

    // ***** assume 0 ???
    tex_index = 0;

    Texture *tex = get_texture(tex_index);

    // ***** ??? CAST

    // use saved dx_texture_context8
    dx_texture_context8 = _dx_texture_context8;

    UINT mipmap_level;

    mipmap_level = 0;
    _direct_3d_surface = NULL;

    // render to texture 2D
    IDirect3DTexture8 *direct_3d_texture;

    direct_3d_texture = dx_texture_context8 -> _d3d_2d_texture;
    if (direct_3d_texture) {
      hr = direct_3d_texture -> GetSurfaceLevel (mipmap_level, &_direct_3d_surface);
      if (SUCCEEDED (hr)) {
        hr = dxgsg -> _d3d_device -> SetRenderTarget (_direct_3d_surface,
          _z_stencil_buffer);
        if (SUCCEEDED (hr)) {

        }
      }
    }

/*
    // render to cubemap face
    IDirect3DCubeTexture8 *direct_3d_cube_texture;

    direct_3d_cube_texture = dx_texture_context8 -> _d3d_cube_texture;
    if (direct_3d_cube_texture) {
      if (_cube_map_index >= 0 && _cube_map_index < 6) {
        hr = direct_3d_cube_texture -> GetCubeMapSurface (
          (D3DCUBEMAP_FACES) _cube_map_index, mipmap_level, &_direct_3d_surface);
        if (SUCCEEDED (hr)) {
          hr = dxgsg -> _d3d_device -> SetRenderTarget (_direct_3d_surface,
            _z_stencil_buffer);
          if (SUCCEEDED (hr)) {

          }
        }
      } else {
        // error: invalid cube map face index
      }
    }
*/

  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::end_render_texture
//       Access: Public, Virtual
//  Description: If the GraphicsOutput supports direct render-to-texture,
//               and if any setup needs to be done during end_frame,
//               then the setup code should go here.  Any textures that
//               could not be rendered to directly should be reflagged
//               as RTM_copy_texture.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer8::
end_render_texture() {
  DXGraphicsStateGuardian8 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

// is this really needed ??? seems to work without it though
/*

  // Find the color texture, if there is one. That one can be bound to
  // the framebuffer.  All others must be marked RTM_copy_to_texture.

  int tex_index = -1;
  for (int i=0; i<count_textures(); i++) {
    if (get_rtm_mode(i) == RTM_bind_or_copy) {
      if ((get_texture(i)->get_format() != Texture::F_depth_component)&&
          (get_texture(i)->get_format() != Texture::F_stencil_index)&&
          (tex_index < 0)) {
        tex_index = i;
      } else {
        _textures[i]._rtm_mode = RTM_copy_texture;
      }
    }
  }

  if (tex_index >= 0) {
    Texture *tex = get_texture(tex_index);
    TextureContext *tc = tex->prepare_now(_gsg->get_prepared_objects(), _gsg);
    nassertv(tc != (TextureContext *)NULL);
  }

*/

  // restore render context
  HRESULT hr;

  if (_back_buffer) {
    hr = dxgsg -> _d3d_device -> SetRenderTarget (_back_buffer,
      _z_stencil_buffer);
    if (SUCCEEDED (hr)) {
      _back_buffer -> Release ( );
    }
    _back_buffer = NULL;
  }
  if (_z_stencil_buffer) {
    _z_stencil_buffer -> Release ( );
    _z_stencil_buffer = NULL;
  }
  if (_direct_3d_surface) {
    _direct_3d_surface -> Release ( );
    _direct_3d_surface = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::select_cube_map
//       Access: Public, Virtual
//  Description: Called internally when the window is in
//               render-to-a-texture mode and we are in the process of
//               rendering the six faces of a cube map.  This should
//               do whatever needs to be done to switch the buffer to
//               the indicated face.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer8::
select_cube_map(int cube_map_index) {
  DXGraphicsStateGuardian8 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

  _cube_map_index = cube_map_index;

  HRESULT hr;
  UINT mipmap_level;
  int render_target_index;
  DXTextureContext8 *dx_texture_context8;

  mipmap_level = 0;
  render_target_index = 0;
  dx_texture_context8 = _dx_texture_context8;

  if (_direct_3d_surface) {
      _direct_3d_surface -> Release ( );
      _direct_3d_surface = NULL;
  }

  // render to cubemap face
  IDirect3DCubeTexture8 *direct_3d_cube_texture;

  direct_3d_cube_texture = dx_texture_context8 -> _d3d_cube_texture;
  if (direct_3d_cube_texture) {
    if (_cube_map_index >= 0 && _cube_map_index < 6) {
      hr = direct_3d_cube_texture -> GetCubeMapSurface (
        (D3DCUBEMAP_FACES) _cube_map_index, mipmap_level, &_direct_3d_surface);
      if (SUCCEEDED (hr)) {
        hr = dxgsg -> _d3d_device -> SetRenderTarget (_direct_3d_surface,
          _z_stencil_buffer);
        if (SUCCEEDED (hr)) {

        }
      }
    } else {
      // error: invalid cube map face index
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer8::
make_current() {
  PStatTimer timer(_make_current_pcollector);

  DXGraphicsStateGuardian8 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

  // do nothing here
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer8::
release_gsg() {
  GraphicsBuffer::release_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer8::
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
//     Function: wdxGraphicsBuffer8::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer8::
close_buffer() {

  if (_gsg != (GraphicsStateGuardian *)NULL) {
    DXGraphicsStateGuardian8 *dxgsg;
    DCAST_INTO_V(dxgsg, _gsg);

    DXTextureContext8 *dx_texture_context8;

    dx_texture_context8 = _dx_texture_context8;
    if (dx_texture_context8) {
      // release render texture
      if (dx_texture_context8 -> _d3d_texture) {
        dx_texture_context8 -> _d3d_texture -> Release ( );
      }
      dx_texture_context8 -> _d3d_texture = NULL;
      dx_texture_context8 -> _d3d_2d_texture = NULL;
      dx_texture_context8 -> _d3d_volume_texture = NULL;
      dx_texture_context8 -> _d3d_cube_texture = NULL;
    }
  }

  _cube_map_index = -1;

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsBuffer8::
open_buffer() {
  bool state;
  DXGraphicsStateGuardian8 *dxgsg;
  DCAST_INTO_R(dxgsg, _gsg, false);

  // create texture
  int tex_index;
  UINT texture_depth;
  DXTextureContext8 *dx_texture_context8;

  state = false;
  _is_valid = false;

// **** assume tex_index is 0 ???
  tex_index = 0;
  texture_depth = 1;

  Texture *tex = get_texture(tex_index);
  TextureContext *tc = tex->prepare_now(_gsg->get_prepared_objects(), _gsg);

// ***** ??? CAST
  dx_texture_context8 = (DXTextureContext8 *) tc;

  // release managed texture
  if (dx_texture_context8 -> _d3d_texture) {
    dx_texture_context8 -> _d3d_texture -> Release ( );
  }
  dx_texture_context8 -> _d3d_texture = NULL;
  dx_texture_context8 -> _d3d_2d_texture = NULL;
  dx_texture_context8 -> _d3d_volume_texture = NULL;
  dx_texture_context8 -> _d3d_cube_texture = NULL;

  // save dx_texture_context8
  _dx_texture_context8 = dx_texture_context8;

  // create render texture
  if (wdxGraphicsBuffer8::create_render_texture (
    get_x_size ( ),
    get_y_size ( ),
    texture_depth,
    dxgsg -> _render_to_texture_d3d_format,
    dx_texture_context8,
    dxgsg -> _d3d_device)) {

    // override texture managed format
    dx_texture_context8 -> _d3d_format = dxgsg -> _render_to_texture_d3d_format;
    dx_texture_context8 -> _has_mipmaps = false;

    _is_valid = true;

    state = true;
  }

  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::process_1_event
//       Access: Private, Static
//  Description: Handles one event from the message queue.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer8::
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

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer8::create_render_texture
//       Access: Private
//  Description: Creates a texture that can be rendered into
////////////////////////////////////////////////////////////////////
bool wdxGraphicsBuffer8::
create_render_texture (UINT texture_width, UINT texture_height, UINT texture_depth, D3DFORMAT texture_format, DXTextureContext8 *dx_texture_context8, IDirect3DDevice8 *direct_3d_device)
{
  bool state;
  HRESULT hr;
  DWORD usage;
  D3DPOOL pool;
  UINT mip_level_count;
  D3DFORMAT pixel_format;
  int texture_type;

  state = false;

  texture_type = dx_texture_context8 -> _texture -> get_texture_type ( );

  // assume 1 mip level
  mip_level_count = 1;

  // REQUIRED pool and usage
  pool = D3DPOOL_DEFAULT;
  usage = D3DUSAGE_RENDERTARGET;

// ???
// usage |= D3DUSAGE_DYNAMIC;

  pixel_format = texture_format;

  switch (texture_type)
  {
    case Texture::TT_2d_texture:
        hr = direct_3d_device->CreateTexture
        (texture_width, texture_height, mip_level_count, usage,
        pixel_format, pool, &dx_texture_context8 -> _d3d_2d_texture);
        dx_texture_context8 -> _d3d_texture = dx_texture_context8 -> _d3d_2d_texture;
        break;

    case Texture::TT_3d_texture:
        // DX8 does not support rendering to volume textures so this should fail
        hr = direct_3d_device->CreateVolumeTexture
        (texture_width, texture_height, texture_depth, mip_level_count, usage,
        pixel_format, pool, &dx_texture_context8 -> _d3d_volume_texture);
        dx_texture_context8 -> _d3d_texture = dx_texture_context8 -> _d3d_volume_texture;
        break;

    case Texture::TT_cube_map:
        hr = direct_3d_device->CreateCubeTexture
        (texture_width, mip_level_count, usage,
        pixel_format, pool, &dx_texture_context8 -> _d3d_cube_texture);
        dx_texture_context8 -> _d3d_texture = dx_texture_context8 -> _d3d_cube_texture;
        break;

    default:
        break;
  }

  if (FAILED(hr)) {
    dxgsg8_cat.error()
      << "create_render_texture failed" << D3DERRORSTRING(hr);
  }
  else {
    state = true;
  }

  return state;
}

