// Filename: fadeImagePool.h
// Created by:  drose (30Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef FADEIMAGEPOOL_H
#define FADEIMAGEPOOL_H

#include "pandaappbase.h"

#include "filename.h"
#include "pvector.h"
#include "pmap.h"

class PNMImage;

////////////////////////////////////////////////////////////////////
//       Class : FadeImagePool
// Description : This maintains a list of images loaded up as "fade"
//               images--that is, grayscale images whose only purpose
//               is to adjust the source image to dark at the edges.
//               It guarantees that each named image is only loaded
//               once.
//
//               Images are never freed from this pool.  The
//               assumption is that you have plenty of RAM for dealing
//               with images.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FadeImagePool {
public:
  INLINE static const PNMImage *get_image(const Filename &filename, 
                                          int x_size, int y_size);

private:
  FadeImagePool();

  const PNMImage *ns_get_image(const Filename &filename, int x_size, int y_size);

  static FadeImagePool *get_ptr();

  typedef pvector<PNMImage *> ImageSizes;
  typedef pmap<Filename, ImageSizes> Images;
  Images _images;

  static FadeImagePool *_global_ptr;
};

#include "fadeImagePool.I"

#endif


