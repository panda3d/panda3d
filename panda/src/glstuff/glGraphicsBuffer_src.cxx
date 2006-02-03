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
//     Function: glGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsBuffer)::
begin_frame() {
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  return GraphicsBuffer::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsStateGuardian::begin_render_texture
//       Access: Public, Virtual
//  Description: If the GraphicsOutput supports direct render-to-texture,
//               and if any setup needs to be done during begin_frame,
//               then the setup code should go here.  Any textures that
//               can not be rendered to directly should be reflagged
//               as RTM_copy_to_texture.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
begin_render_texture() {
  // glGraphicsStateGuardian *glgsg;
  // DCAST_INTO_V(glgsg, _gsg);

  // Find the color texture, if there is one. That one can be bound to
  // the framebuffer.  All others must be marked RTM_copy_to_texture.
  //  int tex_index = -1;
  //  for (int i=0; i<count_textures(); i++) {
  //    if (get_rtm_mode(i) == RTM_bind_or_copy) {
  //      if ((get_texture(i)->get_format() != Texture::F_depth_component)&&
  //          (get_texture(i)->get_format() != Texture::F_stencil_index)&&
  //          (tex_index < 0)) {
  //        tex_index = i;
  //      } else {
  //        _textures[i]._rtm_mode = RTM_copy_texture;
  //      }
  //    }
  //  }
  //  
  //  if (tex_index >= 0) {
  //    Texture *tex = get_texture(tex_index);
  //    TextureContext *tc = tex->prepare_now(_gsg->get_prepared_objects(), _gsg);
  //    nassertv(tc != (TextureContext *)NULL);
  //    CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
  //    GLenum target = glgsg->get_texture_target(tex->get_texture_type());
  //    if (target == GL_NONE) {
  //      _textures[tex_index]._rtm_mode = RTM_copy_texture;
  //      return;
  //    }
  //    GLP(BindTexture)(target, gtc->_index);
  //    if (_gsg->get_properties().is_single_buffered()) {
  //      glgsg->_glBindTexImageARB(_pbuffer, GL_FRONT_LEFT_ARB);
  //    } else {
  //      glgsg->_glBindTexImageARB(_pbuffer, GL_BACK_LEFT_ARB);
  //    }
  //  }
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
void CLP(GraphicsBuffer)::
end_render_texture() {
  // glGraphicsStateGuardian *glgsg;
  // DCAST_INTO_V(glgsg, _gsg);
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
  // glGraphicsStateGuardian *glgsg;
  // DCAST_INTO_V(glgsg, _gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
make_current() {
  PStatTimer timer(_make_current_pcollector);

  // How to implement this is not obvious.
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

