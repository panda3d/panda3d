// Filename: wdxGraphicsBuffer9.cxx
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
#include "pStatTimer.h"


TypeHandle wdxGraphicsBuffer9::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsBuffer9::
wdxGraphicsBuffer9(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  const string &name,
                  int x_size, int y_size) :
  GraphicsBuffer(pipe, gsg, name, x_size, y_size)
{
  // initialize all class members
  _cube_map_index = -1;
  _back_buffer = NULL;
  _z_stencil_buffer = NULL;

  // Since the pbuffer never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsBuffer9::
~wdxGraphicsBuffer9() {
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
begin_frame() {
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  DXGraphicsStateGuardian9 *dxgsg;
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
void wdxGraphicsBuffer9::
begin_render_texture() {
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

HRESULT hr;
bool state;
int render_target_index;

  state = false;
  render_target_index = 0;

  // save render context
  hr = dxgsg -> _d3d_device -> GetRenderTarget (render_target_index, &_back_buffer);
  if (SUCCEEDED (hr)) {
    hr = dxgsg -> _d3d_device -> GetDepthStencilSurface (&_z_stencil_buffer);
    if (SUCCEEDED (hr)) {
      state = TRUE;
    }
  }
  // set render context
  if (state && _dx_texture_context9)
  {

  int tex_index;
  DXTextureContext9 *dx_texture_context9;

// **** ??? assume 0
  tex_index = 0;

  Texture *tex = get_texture(tex_index);

// ***** ??? CAST

  dx_texture_context9 = _dx_texture_context9;

    UINT mipmap_level;

    mipmap_level = 0;
    _direct_3d_surface = NULL;

IDirect3DTexture9 *direct_3d_texture;

direct_3d_texture = dx_texture_context9 -> _d3d_2d_texture;

    if (direct_3d_texture) {
      hr = direct_3d_texture -> GetSurfaceLevel (mipmap_level, &_direct_3d_surface);
      if (SUCCEEDED (hr)) {
        hr = dxgsg -> _d3d_device -> SetRenderTarget (render_target_index, _direct_3d_surface);
        if (SUCCEEDED (hr)) {

        }
      }
    }

IDirect3DCubeTexture9 *direct_3d_cube_texture;

direct_3d_cube_texture = dx_texture_context9 -> _d3d_cube_texture;

    if (direct_3d_cube_texture) {

// ***** check for legal cubemap index

      hr = direct_3d_cube_texture -> GetCubeMapSurface ((D3DCUBEMAP_FACES) _cube_map_index, mipmap_level, &_direct_3d_surface);
      if (SUCCEEDED (hr)) {
        hr = dxgsg -> _d3d_device -> SetRenderTarget (render_target_index, _direct_3d_surface);
        if (SUCCEEDED (hr)) {

        }
      }
    }
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
void wdxGraphicsBuffer9::
end_render_texture() {
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

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

// ???
  if (tex_index >= 0) {
    Texture *tex = get_texture(tex_index);
    TextureContext *tc = tex->prepare_now(_gsg->get_prepared_objects(), _gsg);
    nassertv(tc != (TextureContext *)NULL);
  }

  // restore render context
  HRESULT hr;
  int render_target_index;
  render_target_index = 0;

  if (_back_buffer) {
    hr = dxgsg -> _d3d_device -> SetRenderTarget (render_target_index, _back_buffer);
    if (SUCCEEDED (hr)) {
      _back_buffer -> Release ( );
    }
    _back_buffer = NULL;
  }
  if (_z_stencil_buffer) {
    hr = dxgsg -> _d3d_device -> SetDepthStencilSurface (_z_stencil_buffer);
    if (SUCCEEDED (hr)) {
      _z_stencil_buffer -> Release ( );
    }
    _z_stencil_buffer = NULL;
  }
  if (_direct_3d_surface) {
    _direct_3d_surface -> Release ( );
    _direct_3d_surface = NULL;
  }
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
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

  _cube_map_index = cube_map_index;

}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer9::
make_current() {
  PStatTimer timer(_make_current_pcollector);

  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);

}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsBuffer9::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsBuffer9::
release_gsg() {
  GraphicsBuffer::release_gsg();
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
    DXGraphicsStateGuardian9 *dxgsg;
    DCAST_INTO_V(dxgsg, _gsg);

// ***** release render texture



  }

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
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_R(dxgsg, _gsg, false);

  // create texture
  int tex_index;
  UINT texture_depth;
  DXTextureContext9 *dx_texture_context9;

// **** ??? assume 0
  tex_index = 0;
  texture_depth = 1;

  Texture *tex = get_texture(tex_index);
  TextureContext *tc = tex->prepare_now(_gsg->get_prepared_objects(), _gsg);

// free managed texture ???



// ***** ??? CAST
  dx_texture_context9 = (DXTextureContext9 *) tc;

// cache dx_texture_context9
_dx_texture_context9 = dx_texture_context9;

  // create render texture
  create_render_texture (get_x_size ( ), get_y_size ( ), texture_depth,
    dxgsg -> _render_to_texture_d3d_format, dx_texture_context9,
    dxgsg -> _d3d_device);

  dx_texture_context9 -> _d3d_format = dxgsg -> _render_to_texture_d3d_format;

  _is_valid = true;

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


bool wdxGraphicsBuffer9::
create_render_texture (UINT texture_width, UINT texture_height, UINT texture_depth, D3DFORMAT texture_format, DXTextureContext9 *dx_texture_context9, IDirect3DDevice9 *direct_3d_device)
{
  bool state;
  HRESULT hr;
  DWORD usage;
  D3DPOOL pool;
  UINT mip_level_count;
  D3DFORMAT pixel_format;
  int texture_type;

  state = false;

  texture_type = dx_texture_context9 -> _texture -> get_texture_type ( );

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
        pixel_format, pool, &dx_texture_context9 -> _d3d_2d_texture, NULL);
        dx_texture_context9 -> _d3d_texture = dx_texture_context9 -> _d3d_2d_texture;
        break;

    case Texture::TT_3d_texture:
        hr = direct_3d_device->CreateVolumeTexture
        (texture_width, texture_height, texture_depth, mip_level_count, usage,
        pixel_format, pool, &dx_texture_context9 -> _d3d_volume_texture, NULL);
        dx_texture_context9 -> _d3d_texture = dx_texture_context9 -> _d3d_volume_texture;
        break;

    case Texture::TT_cube_map:
        hr = direct_3d_device->CreateCubeTexture
        (texture_width, mip_level_count, usage,
        pixel_format, pool, &dx_texture_context9 -> _d3d_cube_texture, NULL);
        dx_texture_context9 -> _d3d_texture = dx_texture_context9 -> _d3d_cube_texture;
        break;

    default:
        break;
  }

  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "create_render_texture failed" << D3DERRORSTRING(hr);
  }
  else {
    state = true;
  }

  return state;
}

// ISSUES:
    // check size issues
        // original frame buffer width, height
    // render to texure format
    // bind / set texture with which texture stage
    // LOST DEVICE case for D3DPOOL_DEFAULT buffers
    // CheckDepthStencilMatch
    // ? check D3DCAPS2_DYNAMICTEXTURES
