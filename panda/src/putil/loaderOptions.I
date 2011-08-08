// Filename: loaderOptions.I
// Created by:  drose (05Oct05)
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
//     Function: LoaderOptions::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LoaderOptions::
LoaderOptions(int flags, int texture_flags) : 
  _flags(flags), 
  _texture_flags(texture_flags),
  _texture_num_views(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LoaderOptions::
LoaderOptions(const LoaderOptions &copy) :
  _flags(copy._flags),
  _texture_flags(copy._texture_flags),
  _texture_num_views(copy._texture_num_views)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LoaderOptions::
operator = (const LoaderOptions &copy) {
  _flags = copy._flags;
  _texture_flags = copy._texture_flags;
  _texture_num_views = copy._texture_num_views;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::set_flags
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LoaderOptions::
set_flags(int flags) {
  _flags = flags;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::get_flags
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int LoaderOptions::
get_flags() const {
  return _flags;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::set_texture_flags
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LoaderOptions::
set_texture_flags(int texture_flags) {
  _texture_flags = texture_flags;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::get_texture_flags
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int LoaderOptions::
get_texture_flags() const {
  return _texture_flags;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::set_texture_num_views
//       Access: Published
//  Description: Specifies the expected number of views to load for
//               the texture.  This is ignored unless TF_multiview is
//               included in texture_flags.  This must be specified
//               when loading a 3-d multiview texture, in which case
//               it is used to differentiate z levels from separate
//               views; it may be zero in the case of 2-d textures or
//               cube maps, in which case the number of views can be
//               inferred from the number of images found on disk.
////////////////////////////////////////////////////////////////////
INLINE void LoaderOptions::
set_texture_num_views(int texture_num_views) {
  _texture_num_views = texture_num_views;
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::get_texture_num_views
//       Access: Published
//  Description: See set_texture_num_views().
////////////////////////////////////////////////////////////////////
INLINE int LoaderOptions::
get_texture_num_views() const {
  return _texture_num_views;
}
