// Filename: imageBuffer.I
// Created by:  drose (21Nov00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::set_alpha_name
//       Access: Public
//  Description: Sets the name of the file that contains the image's
//               alpha channel contents.  Normally, this is set
//               automatically when the image is loaded, for instance
//               via Texture::read().
//
//               The ImageBuffer's get_name() function, by convention,
//               returns the name of the image file that was loaded
//               into the buffer.  In the case where a texture
//               specified two separate files to load, a 1- or
//               3-channel color image and a 1-channel alpha image,
//               this string is update to contain the name of the
//               image file that was loaded into the buffer's alpha
//               channel.
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
set_alpha_name(const string &alpha_name) {
  _alpha_name = alpha_name;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::clear_alpha_name
//       Access: Public
//  Description: Removes the alpha filename, if it was previously set.
//               See set_alpha_name().
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
clear_alpha_name() {
  _alpha_name = string();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::has_alpha_name
//       Access: Public
//  Description: Returns true if the alpha_name has been set and
//               is available.  See set_alpha_name().
////////////////////////////////////////////////////////////////////
INLINE bool ImageBuffer::
has_alpha_name() const {
  return !_alpha_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::get_alpha_name
//       Access: Public
//  Description: Returns the alpha_name that has been set.  Use
//               this in conjunction with get_name() to get the names
//               of the file(s) that were loaded into the buffer.  See
//               set_alpha_name().
////////////////////////////////////////////////////////////////////
INLINE const string &ImageBuffer::
get_alpha_name() const {
  return _alpha_name;
}
