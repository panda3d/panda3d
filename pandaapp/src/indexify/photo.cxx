// Filename: photo.cxx
// Created by:  drose (03Apr02)
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

#include "photo.h"
#include "indexParameters.h"
#include "rollDirectory.h"


////////////////////////////////////////////////////////////////////
//     Function: Photo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Photo::
Photo(RollDirectory *dir, const Filename &basename, const string &movie_extension) :
  _dir(dir),
  _basename(basename)
{
  _name = _basename.get_basename_wo_extension();
  _frame_number = _name;
  if (caption_frame_numbers) {
    const string &dirname = _dir->get_basename();

    // If the initial prefix of the filename is the same as the roll
    // directory, lop it off when caption_frame_numbers is in effect.
    if (_frame_number.substr(0, dirname.length()) == dirname) {
      _frame_number = _frame_number.substr(dirname.length());
      while (_frame_number.length() > 1 && _frame_number[0] == '0') {
	_frame_number = _frame_number.substr(1);
      }
    }
  }
  
  _full_x_size = 0;
  _full_y_size = 0;
  _reduced_x_size = 0;
  _reduced_y_size = 0;
  _has_reduced = false;

  _has_movie = false;
  Filename movie_filename(_dir->get_dir(), _basename);
  movie_filename.set_extension(movie_extension);
  if (movie_filename.exists()) {
    _movie = movie_filename.get_basename();
    _has_movie = true;
  }

  _has_cm = false;
  Filename cm_filename(_dir->get_dir(), _basename);
  cm_filename.set_extension("cm");
  if (cm_filename.exists()) {
    _cm = cm_filename.get_basename();
    _has_cm = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::get_basename
//       Access: Public
//  Description: Returns the filename of the photo within its roll
//               directory.
////////////////////////////////////////////////////////////////////
const Filename &Photo::
get_basename() const {
  return _basename;
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::get_movie
//       Access: Public
//  Description: Returns the filename of the movie associated with the
//               photo, if any.
////////////////////////////////////////////////////////////////////
const Filename &Photo::
get_movie() const {
  return _movie;
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::get_cm
//       Access: Public
//  Description: Returns the filename of the comment file associated
//               with the photo, if any.
////////////////////////////////////////////////////////////////////
const Filename &Photo::
get_cm() const {
  return _cm;
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::get_name
//       Access: Public
//  Description: Returns the name of the photo without its filename
//               extension.
////////////////////////////////////////////////////////////////////
const string &Photo::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::get_frame_number
//       Access: Public
//  Description: Returns the string describing the photo within the
//               roll directory.  This is usually the same string
//               reported by get_name(), but it may be just the frame
//               number if caption_frame_numbers is in effect.
////////////////////////////////////////////////////////////////////
const string &Photo::
get_frame_number() const {
  return _frame_number;
}

////////////////////////////////////////////////////////////////////
//     Function: Photo::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Photo::
output(ostream &out) const {
  out << _basename;
}
