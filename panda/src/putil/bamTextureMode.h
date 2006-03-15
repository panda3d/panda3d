// Filename: bamTextureMode.h
// Created by:  drose (14Mar06)
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

#ifndef BAMTEXTUREMODE_H
#define BAMTEXTUREMODE_H

#include "pandabase.h"

// This enum is used to control how textures are written to a bam
// stream.
enum BamTextureMode {
  BTM_unchanged,
  BTM_fullpath,
  BTM_relative,
  BTM_basename,
  BTM_rawdata
};
EXPCL_PANDA ostream &operator << (ostream &out, BamTextureMode btm);
EXPCL_PANDA istream &operator >> (istream &in, BamTextureMode &btm);

#endif
