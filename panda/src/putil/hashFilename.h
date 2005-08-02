// Filename: hashFilename.h
// Created by:  drose (02Aug05)
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

#ifndef HASHFILENAME_H
#define HASHFILENAME_H

#include "pandabase.h"
#include "filename.h"

////////////////////////////////////////////////////////////////////
//       Class : HashFilename
// Description : This is a specialization on Filename that is intended
//               to be used to record a filename pattern for reading a
//               numeric sequence of filenames in a directory; for
//               instance, as used by TexturePool::load_cube_map().
//
//               The Filename may include a string of one or more hash
//               marks ('#') in the basename or extension part.  These
//               will be filled in with digits when
//               get_filename_index() is called.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA HashFilename : public Filename {
PUBLISHED:
  INLINE HashFilename(const string &filename_pattern = string());
  INLINE HashFilename(const HashFilename &copy);
  INLINE void operator = (const HashFilename &copy);
  INLINE void operator = (const Filename &copy);
  INLINE ~HashFilename();

  INLINE bool has_hash() const;
  Filename get_filename_index(int index) const;

  INLINE string get_hash_to_end() const;
  void set_hash_to_end(const string &s);
  
private:
  void locate_hash();

  size_t _hash_start;
  size_t _hash_end;
};

#include "hashFilename.I"

#endif
