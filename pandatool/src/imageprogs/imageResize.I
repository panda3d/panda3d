// Filename: imageResize.I
// Created by:  drose (13Mar03)
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
//     Function: ImageResize::SizeRequest::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ImageResize::SizeRequest::
SizeRequest() {
  _type = RT_none;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageResize::SizeRequest::get_type
//       Access: Public
//  Description: Returns the type of the size request, or RT_none if
//               the request has not been specified.
////////////////////////////////////////////////////////////////////
INLINE ImageResize::RequestType ImageResize::SizeRequest::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageResize::SizeRequest::set_pixel_size
//       Access: Public
//  Description: Sets the size request to store an explicit pixel
//               size.
////////////////////////////////////////////////////////////////////
INLINE void ImageResize::SizeRequest::
set_pixel_size(int pixel_size) {
  _type = RT_pixel_size;
  _e._pixel_size = pixel_size;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageResize::SizeRequest::get_pixel_size
//       Access: Public
//  Description: Returns the explicit pixel size stored within the
//               size request.
////////////////////////////////////////////////////////////////////
INLINE int ImageResize::SizeRequest::
get_pixel_size() const {
  nassertr(_type == RT_pixel_size, 0);
  return _e._pixel_size;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageResize::SizeRequest::get_pixel_size
//       Access: Public
//  Description: Returns the explicit pixel size stored within the
//               size request, or if a ratio has been stored, returns
//               the computed pixel size based on the original size.
////////////////////////////////////////////////////////////////////
INLINE int ImageResize::SizeRequest::
get_pixel_size(int orig_pixel_size) const {
  switch (_type) {
  case RT_pixel_size:
    return _e._pixel_size;
  case RT_ratio:
    return (int)(_e._ratio * orig_pixel_size + 0.5);
  default:
    return orig_pixel_size;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ImageResize::SizeRequest::set_ratio
//       Access: Public
//  Description: Sets the size request to store a specific ratio.
////////////////////////////////////////////////////////////////////
INLINE void ImageResize::SizeRequest::
set_ratio(double ratio) {
  _type = RT_ratio;
  _e._ratio = ratio;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageResize::SizeRequest::get_ratio
//       Access: Public
//  Description: Returns the specific ratio stored within the
//               size request.
////////////////////////////////////////////////////////////////////
INLINE double ImageResize::SizeRequest::
get_ratio() const {
  nassertr(_type == RT_ratio, 0);
  return _e._ratio;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageResize::SizeRequest::get_ratio
//       Access: Public
//  Description: Returns the specific ratio stored within the
//               size request, or if a pixel size has been stored,
//               returns the computed ratio based on the original
//               size.
////////////////////////////////////////////////////////////////////
INLINE double ImageResize::SizeRequest::
get_ratio(int orig_pixel_size) const {
  switch (_type) {
  case RT_ratio:
    return _e._ratio;
  case RT_pixel_size:
    return (double)_e._pixel_size / (double)orig_pixel_size;
  default:
    return 1.0;
  }
}
