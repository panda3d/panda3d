// Filename: imageInfo.h
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

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include "pandatoolbase.h"

#include "programBase.h"

////////////////////////////////////////////////////////////////////
//       Class : ImageInfo
// Description : This program reads the headers of a series of one or
//               more images and reports their sizes to standard
//               output.
////////////////////////////////////////////////////////////////////
class ImageInfo : public ProgramBase {
public:
  ImageInfo();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  bool is_power_2(int value) const;

  Args _filenames;
  bool _report_power_2;
};

#endif

