/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file txoConverter.h
 * @author RegDogg
 * @date 2025-11-10
 */

#ifndef TXOCONVERTER_H
#define TXOCONVERTER_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "filename.h"
#include "withOutputFile.h"
#include "textureAttrib.h"

/**
 *
 */
class TxoConverter : public ProgramBase, public WithOutputFile {
public:
  TxoConverter();
    
  void run();

  protected:
  virtual bool handle_args(Args &args);

  private:
  void convert_txo(Texture *tex);

  Filename _image_filename;
  bool _got_rgb_filename;
  Filename _rgb_filename;
};

#endif
