// Filename: mayaCopy.h
// Created by:  drose (10May02)
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

#ifndef MAYACOPY_H
#define MAYACOPY_H

#include "pandatoolbase.h"

#include "cvsCopy.h"

#include "dSearchPath.h"
#include "pointerTo.h"

#include "pset.h"

class MayaShader;

////////////////////////////////////////////////////////////////////
//       Class : MayaCopy
// Description : A program to copy Maya .mb files into the cvs
//               tree.
////////////////////////////////////////////////////////////////////
class MayaCopy : public CVSCopy {
public:
  MayaCopy();

  void run();

protected:
  virtual bool copy_file(const Filename &source, const Filename &dest,
                         CVSSourceDirectory *dir, void *extra_data,
                         bool new_file);

private:
  enum FileType {
    FT_maya,
    FT_texture
  };

  class ExtraData {
  public:
    FileType _type;
    MayaShader *_shader;
  };

  bool copy_maya_file(const Filename &source, const Filename &dest,
                     CVSSourceDirectory *dir);
  bool copy_texture(const Filename &source, const Filename &dest,
                    CVSSourceDirectory *dir);


  typedef pset< PT(MayaExternalReference) > Refs;
  typedef pset< PT(MayaTexture) > Textures;

  void scan_maya(MayaRecord *record, Refs &refs, Textures &textures);
};

#endif
