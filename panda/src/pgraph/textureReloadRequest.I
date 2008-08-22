// Filename: textureReloadRequest.I
// Created by:  drose (12Aug08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TextureReloadRequest::Constructor
//       Access: Published
//  Description: Create a new TextureReloadRequest, and add it to the loader
//               via load_async(), to begin an asynchronous load.
////////////////////////////////////////////////////////////////////
INLINE TextureReloadRequest::
TextureReloadRequest(TextureContext *tc, bool allow_compressed) :
  _texture_context(tc),
  _allow_compressed(allow_compressed),
  _is_ready(false)
{
  nassertv(_texture_context != (TextureContext *)NULL);
  _texture = _texture_context->get_texture();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureReloadRequest::get_texture_context
//       Access: Published
//  Description: Returns the TextureContext object associated with
//               this asynchronous TextureReloadRequest.
////////////////////////////////////////////////////////////////////
INLINE TextureContext *TextureReloadRequest::
get_texture_context() const {
  return _texture_context;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureReloadRequest::get_allow_compressed
//       Access: Published
//  Description: Returns the "allow compressed" flag associated with
//               this asynchronous TextureReloadRequest.
////////////////////////////////////////////////////////////////////
INLINE bool TextureReloadRequest::
get_allow_compressed() const {
  return _allow_compressed;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureReloadRequest::is_ready
//       Access: Published
//  Description: Returns true if this request has completed, false if
//               it is still pending.
////////////////////////////////////////////////////////////////////
INLINE bool TextureReloadRequest::
is_ready() const {
  return _is_ready;
}
