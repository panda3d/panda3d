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
TextureReloadRequest(const string &name,
                     PreparedGraphicsObjects *pgo, Texture *texture, 
                     bool allow_compressed) :
  AsyncTask(name),
  _pgo(pgo),
  _texture(texture),
  _allow_compressed(allow_compressed),
  _is_ready(false)
{
  nassertv(_pgo != (PreparedGraphicsObjects *)NULL);
  nassertv(_texture != (Texture *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureReloadRequest::get_prepared_graphics_objects
//       Access: Published
//  Description: Returns the PreparedGraphicsObjects object associated with
//               this asynchronous TextureReloadRequest.
////////////////////////////////////////////////////////////////////
INLINE PreparedGraphicsObjects *TextureReloadRequest::
get_prepared_graphics_objects() const {
  return _pgo;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureReloadRequest::get_texture
//       Access: Published
//  Description: Returns the Texture object associated with
//               this asynchronous TextureReloadRequest.
////////////////////////////////////////////////////////////////////
INLINE Texture *TextureReloadRequest::
get_texture() const {
  return _texture;
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
