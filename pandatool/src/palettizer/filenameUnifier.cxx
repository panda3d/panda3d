// Filename: filenameUnifier.cxx
// Created by:  drose (05Dec00)
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

#include "filenameUnifier.h"

#include "executionEnvironment.h"

Filename FilenameUnifier::_txa_filename;
Filename FilenameUnifier::_txa_dir;
Filename FilenameUnifier::_rel_dirname;

FilenameUnifier::CanonicalFilenames FilenameUnifier::_canonical_filenames;

////////////////////////////////////////////////////////////////////
//     Function: FilenameUnifier::set_txa_filename
//       Access: Public, Static
//  Description: Notes the filename the .txa file was found in.  This
//               may have come from the command line, or it may have
//               been implicitly located.  This has other implications
//               for the FilenameUnifier, particularly in locating the bam
//               file that saves the filenameUnifier state from last
//               session.
////////////////////////////////////////////////////////////////////
void FilenameUnifier::
set_txa_filename(const Filename &txa_filename) {
  _txa_filename = txa_filename;
  _txa_dir = txa_filename.get_dirname();
  if (_txa_dir.empty()) {
    _txa_dir = ".";
  }
  make_canonical(_txa_dir);
}

////////////////////////////////////////////////////////////////////
//     Function: FilenameUnifier::set_rel_dirname
//       Access: Public, Static
//  Description: Sets the name of the directory that texture filenames
//               will be written relative to, when generating egg
//               files.  This is not the directory the textures are
//               actually written to (see set_map_dirname()), but
//               rather is the name of some directory above that,
//               which will be the starting point for the pathnames
//               written to the egg files.  If this is empty, the full
//               pathnames will be written to the egg files.
////////////////////////////////////////////////////////////////////
void FilenameUnifier::
set_rel_dirname(const Filename &rel_dirname) {
  _rel_dirname = rel_dirname;
  if (!_rel_dirname.empty()) {
    make_canonical(_rel_dirname);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FilenameUnifier::make_bam_filename
//       Access: Public, Static
//  Description: Returns a new filename that's made relative to the
//               bam file itself, suitable for writing to the bam file.
////////////////////////////////////////////////////////////////////
Filename FilenameUnifier::
make_bam_filename(Filename filename) {
  make_canonical(filename);
  filename.make_relative_to(_txa_dir);
  return filename;
}

////////////////////////////////////////////////////////////////////
//     Function: FilenameUnifier::get_bam_filename
//       Access: Public, Static
//  Description: Returns an absolute pathname based on the given
//               relative pathname, presumably read from the bam file
//               and relative to the bam file.
////////////////////////////////////////////////////////////////////
Filename FilenameUnifier::
get_bam_filename(Filename filename) {
  if (!filename.empty()) {
    filename.make_absolute(_txa_dir);
  }
  return filename;
}

////////////////////////////////////////////////////////////////////
//     Function: FilenameUnifier::make_egg_filename
//       Access: Public, Static
//  Description: Returns a new filename that's made relative to the
//               rel_directory, suitable for writing out within egg
//               files.
////////////////////////////////////////////////////////////////////
Filename FilenameUnifier::
make_egg_filename(Filename filename) {
  if (!filename.empty()) {
    make_canonical(filename);
    filename.make_relative_to(_rel_dirname);
  }
  return filename;
}

////////////////////////////////////////////////////////////////////
//     Function: FilenameUnifier::make_user_filename
//       Access: Public, Static
//  Description: Returns a new filename that's made relative to the
//               current directory, suitable for reporting to the
//               user.
////////////////////////////////////////////////////////////////////
Filename FilenameUnifier::
make_user_filename(Filename filename) {
  if (!filename.empty()) {
    make_canonical(filename);
    filename.make_relative_to(ExecutionEnvironment::get_cwd());
  }
  return filename;
}

////////////////////////////////////////////////////////////////////
//     Function: FilenameUnifier::make_canonical
//       Access: Public, Static
//  Description: Does the same thing as Filename::make_canonical()--it
//               converts the filename to its canonical form--but
//               caches the operation so that repeated calls to
//               filenames in the same directory will tend to be
//               faster.
////////////////////////////////////////////////////////////////////
void FilenameUnifier::
make_canonical(Filename &filename) {
  if (filename.empty()) {
    return;
  }

  Filename orig_dirname = filename.get_dirname();

  CanonicalFilenames::iterator fi;
  fi = _canonical_filenames.find(orig_dirname);
  if (fi != _canonical_filenames.end()) {
    filename.set_dirname((*fi).second);
    return;
  }

  Filename new_dirname = orig_dirname;
  if (new_dirname.empty()) {
    new_dirname = ".";
  }
  new_dirname.make_canonical();
  filename.set_dirname(new_dirname);

  _canonical_filenames.insert(CanonicalFilenames::value_type(orig_dirname, new_dirname));
}
