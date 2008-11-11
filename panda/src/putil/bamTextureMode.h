// Filename: bamTextureMode.h
// Created by:  drose (14Mar06)
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

#ifndef BAMTEXTUREMODE_H
#define BAMTEXTUREMODE_H

#include "pandabase.h"

BEGIN_PUBLISH
// This enum is used to control how textures are written to a bam
// stream.
enum BamTextureMode {
  BTM_unchanged,
  BTM_fullpath,
  BTM_relative,
  BTM_basename,
  BTM_rawdata
};
END_PUBLISH

EXPCL_PANDA_PUTIL ostream &operator << (ostream &out, BamTextureMode btm);
EXPCL_PANDA_PUTIL istream &operator >> (istream &in, BamTextureMode &btm);

#endif
