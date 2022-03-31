/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageTransformColors.h
 * @author drose
 * @date 2009-03-25
 */

#ifndef IMAGETRANSFORMCOLORS_H
#define IMAGETRANSFORMCOLORS_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "pvector.h"
#include "filename.h"
#include "luse.h"

class PNMImage;

/**
 * This program can apply a 4x4 color transform to all of the colors in the
 * pixels of a series of images.
 */
class ImageTransformColors : public ProgramBase {
public:
  ImageTransformColors();

  void run();

protected:
  static bool dispatch_mat4(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_mat3(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_range(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_scale(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_add(const std::string &opt, const std::string &arg, void *var);

  virtual bool handle_args(Args &args);
  Filename get_output_filename(const Filename &source_filename) const;

  void process_image(PNMImage &image);

private:
  bool _hls;
  LMatrix4d _mat;

  bool _got_output_filename;
  Filename _output_filename;
  bool _got_output_dirname;
  Filename _output_dirname;
  bool _inplace;

  typedef pvector<Filename> Filenames;
  Filenames _filenames;
};

#include "imageTransformColors.I"

#endif
