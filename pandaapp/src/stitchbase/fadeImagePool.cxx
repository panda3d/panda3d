// Filename: fadeImagePool.cxx
// Created by:  drose (30Jul01)
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
////////////////////////////////////////////////////////////////////

#include "fadeImagePool.h"
#include "pnmImage.h"

FadeImagePool *FadeImagePool::_global_ptr = (FadeImagePool *)NULL;

////////////////////////////////////////////////////////////////////
//     Function: FadeImagePool::Constructor
//       Access: Private
//  Description: The constructor is never called explicitly; there is
//               only one FadeImagePool and it constructs itself.
////////////////////////////////////////////////////////////////////
FadeImagePool::
FadeImagePool() {
}

////////////////////////////////////////////////////////////////////
//     Function: FadeImagePool::ns_get_image
//       Access: Private
//  Description: The nonstatic implementation of get_image().  This
//               loads up the image if it is not already, scales it to
//               the indicated size if it is not already, and returns
//               it.
////////////////////////////////////////////////////////////////////
const PNMImage *FadeImagePool::
ns_get_image(const Filename &filename, int x_size, int y_size) {
  Images::iterator ii = _images.find(filename);
  if (ii == _images.end()) {
    // The image has not yet been loaded.  Load it.
    cerr << "Reading fade image " << filename << "\n";
    PNMImage *image = new PNMImage(filename);
    if (!image->is_valid()) {
      cerr << "Unable to read fade image.\n";
      delete image;
      return (const PNMImage *)NULL;
    }

    // Make sure it's a grayscale image.  This will save a bit of time
    // later.
    image->set_color_type(PNMImage::CT_grayscale);

    ii = _images.insert(Images::value_type(filename, ImageSizes())).first;
    (*ii).second.push_back(image);
  }

  // Now see if we have a fade image of the requested size.
  ImageSizes &sizes = (*ii).second;
  ImageSizes::iterator si;
  for (si = sizes.begin(); si != sizes.end(); ++si) {
    PNMImage *image = (*si);
    if (image->get_x_size() == x_size && image->get_y_size() == y_size) {
      // Here's one that suits!
      return image;
    }
  }

  // None of our images were of a suitable size, so make one.
  nassertr(!sizes.empty(), NULL);

  PNMImage *orig_image = sizes.front();
  cerr << "Resizing fade image to " << x_size << " by " << y_size << "\n";
  PNMImage *resized_image = 
    new PNMImage(x_size, y_size, PNMImage::CT_grayscale,
                 orig_image->get_maxval());
  resized_image->quick_filter_from(*orig_image);

  sizes.push_back(resized_image);
  return resized_image;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeImagePool::get_ptr
//       Access: Private, Static
//  Description: Returns the global FadeImagePool pointer.
////////////////////////////////////////////////////////////////////
FadeImagePool *FadeImagePool::
get_ptr() {
  if (_global_ptr == (FadeImagePool *)NULL) {
    _global_ptr = new FadeImagePool;
  }
  return _global_ptr;
}

