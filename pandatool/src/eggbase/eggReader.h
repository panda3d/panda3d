// Filename: eggReader.h
// Created by:  drose (14Feb00)
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

#ifndef EGGREADER_H
#define EGGREADER_H

#include "pandatoolbase.h"

#include "eggSingleBase.h"
#include "filename.h"

class PNMFileType;

////////////////////////////////////////////////////////////////////
//       Class : EggReader
// Description : This is the base class for a program that reads egg
//               files, but doesn't write an egg file.
////////////////////////////////////////////////////////////////////
class EggReader : virtual public EggSingleBase {
public:
  EggReader();

  void add_texture_options();
  void add_delod_options(double default_delod = -1.0);
  
  virtual EggReader *as_reader();
  virtual void pre_process_egg_file();

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  bool do_reader_options();

private:
  bool copy_textures();
  bool do_delod(EggNode *node);

protected:
  bool _force_complete;

private:
  Filename _tex_dirname;
  bool _got_tex_dirname;
  string _tex_extension;
  bool _got_tex_extension;
  PNMFileType *_tex_type;
  double _delod;
};

#endif


