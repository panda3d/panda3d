// Filename: fltCopy.h
// Created by:  drose (01Nov00)
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

#ifndef FLTCOPY_H
#define FLTCOPY_H

#include "pandatoolbase.h"

#include "cvsCopy.h"

#include "dSearchPath.h"
#include "pointerTo.h"

#include "pset.h"

class FltRecord;
class FltTexture;
class FltExternalReference;

////////////////////////////////////////////////////////////////////
//       Class : FltCopy
// Description : A program to copy Multigen .flt files into the cvs
//               tree.  It copies the base file plus all externally
//               referenced files as well as all textures.
////////////////////////////////////////////////////////////////////
class FltCopy : public CVSCopy {
public:
  FltCopy();

  void run();

protected:
  virtual bool copy_file(const Filename &source, const Filename &dest,
                         CVSSourceDirectory *dir, void *extra_data,
                         bool new_file);

private:
  enum FileType {
    FT_flt,
    FT_texture
  };

  class ExtraData {
  public:
    FileType _type;
    FltTexture *_texture;
  };

  bool copy_flt_file(const Filename &source, const Filename &dest,
                     CVSSourceDirectory *dir);
  bool copy_texture(const Filename &source, const Filename &dest,
                    CVSSourceDirectory *dir, FltTexture *tex,
                    bool new_file);


  typedef pset< PT(FltExternalReference) > Refs;
  typedef pset< PT(FltTexture) > Textures;

  void scan_flt(FltRecord *record, Refs &refs, Textures &textures);
};

#endif
