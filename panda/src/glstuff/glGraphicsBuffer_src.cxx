// Filename: glGraphicsBuffer_src.cxx
// Created by:  jyelon (15Jan06)
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

TypeHandle CLP(GraphicsBuffer)::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLP(GraphicsBuffer)::
CLP(GraphicsBuffer)(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  const string &name,
                  int x_size, int y_size) :
  GraphicsBuffer(pipe, gsg, name, x_size, y_size) 
{
  // Since the FBO never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CLP(GraphicsBuffer)::
~CLP(GraphicsBuffer)() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
begin_frame(FrameMode mode) {
  PStatTimer timer(_make_current_pcollector);

  begin_frame_spam();
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  if (!_host->begin_frame(FM_parasite)) {
    return false;
  }

  if (_track_host_size) {
    if ((_host->get_x_size() != _x_size)||
        (_host->get_y_size() != _y_size)) {
      set_size_and_recalc(_host->get_x_size(),
                          _host->get_y_size());
    }
  }

  // This function also contains the code to *reopen* the window
  // in the event that configuration parameters (ie, size, texture
  // bindings) have changed.
  open_window();
  
  // bind the FBO

  if (mode == FM_render) {
    begin_render_texture();
    clear_cube_map_selection();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
end_frame(FrameMode mode) {
  end_frame_spam();
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    end_render_texture();
    copy_to_textures();
  }

  // Unbind the FBO
  
  _host->end_frame(FM_parasite);

  if (mode == FM_render) {
    trigger_flip();
    if (_one_shot) {
      prepare_for_deletion();
    }
    clear_cube_map_selection();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::select_cube_map
//       Access: Public, Virtual
//  Description: Called internally when the window is in
//               render-to-a-texture mode and we are in the process of
//               rendering the six faces of a cube map.  This should
//               do whatever needs to be done to switch the buffer to
//               the indicated face.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
select_cube_map(int cube_map_index) {
  open_buffer();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
//
//               This particular version of open_buffer is also
//               capable of reopening the buffer, which is necessary
//               if the texture bindings, size, or cube face has
//               changed.  Caution: since this function is called
//               at every cube-map switch, it needs to be reasonably
//               fast in the case that little is changing.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
open_buffer() {
  // Make sure the FBO is allocated.
  
  // Figure out the desired size of the FBO
  int desired_x = _x_size;
  int desired_y = _y_size;
  if (!_gsg->get_supports_tex_non_pow2()) {
    desired_x = Texture::up_to_power_2(desired_x);
    desired_y = Texture::up_to_power_2(desired_y);
  }

  // Scan the textures list to see which textures should be attached.
  Texture *attach_color = 0;
  Texture *attach_depth = 0;
  Texture *attach_stencil = 0;
  

  // For all slots containing textures, detach if:
  //   - they aren't supposed to be attached any more, or
  //   - there is a size mismatch, or
  //   - there is a cube-face mismatch
  
  // For all renderbuffers, detach and deallocate if:
  //   - there is a size mismatch, or,
  //   - there is a texture to be attached at that point
  
  // For all to-be-attached textures, attach if not already
  // attached.  During attachment process, resize and reformat
  // if needed.
  
  // For all unfilled slots, allocate and attach render buffers.
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
close_buffer() {
  // Detach all renderbuffers
  // Detach all textures
  // Deallocate the FBO itself.
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
release_gsg() {
  GraphicsBuffer::release_gsg();
}

