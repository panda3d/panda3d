// Filename: photo.h
// Created by:  drose (03Apr02)
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

#ifndef PHOTO_H
#define PHOTO_H

#include "pandatoolbase.h"

#include "filename.h"

class RollDirectory;

////////////////////////////////////////////////////////////////////
//       Class : Photo
// Description : A single photo image within a roll directory.
////////////////////////////////////////////////////////////////////
class Photo {
public:
  Photo(RollDirectory *dir, const Filename &basename, 
        const string &movie_extension);

  const Filename &get_basename() const;
  const Filename &get_movie() const;
  const Filename &get_cm() const;
  const string &get_name() const;
  const string &get_frame_number() const;

  void output(ostream &out) const;

public:
  int _full_x_size;
  int _full_y_size;
  int _reduced_x_size;
  int _reduced_y_size;
  bool _has_reduced;
  bool _has_movie;
  bool _has_cm;

private:
  RollDirectory *_dir;
  Filename _basename;
  Filename _movie;
  Filename _cm;
  string _name;
  string _frame_number;
};

INLINE ostream &operator << (ostream &out, const Photo &p) {
  p.output(out);
  return out;
}

#endif
