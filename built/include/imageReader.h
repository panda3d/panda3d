/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageReader.h
 * @author drose
 * @date 2000-06-19
 */

#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include "pandatoolbase.h"

#include "imageBase.h"

/**
 * This is the base class for a program that reads an image file, but doesn't
 * write an image file.
 */
class ImageReader : virtual public ImageBase {
public:
  ImageReader();

protected:
  virtual bool handle_args(Args &args);

};

#endif
