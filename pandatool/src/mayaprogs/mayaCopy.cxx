// Filename: mayaCopy.cxx
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

#include "mayaCopy.h"

#include "cvsSourceDirectory.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MayaCopy::
MayaCopy() {
  set_program_description
    ("mayacopy copies one or more Maya .mb files into a "
     "CVS source hierarchy.  "
     "Rather than copying the named files immediately into the current "
     "directory, it first scans the entire source hierarchy, identifying all "
     "the already-existing files.  If the named file to copy matches the "
     "name of an already-existing file in the current directory or elsewhere "
     "in the hierarchy, that file is overwritten.  Other .mb files, as "
     "well as texture files, that are externally referenced by the "
     "named .mb file(s) are similarly copied.");

  clear_runlines();
  add_runline("[opts] file.mb [file.mb ... ]");
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void MayaCopy::
run() {
  SourceFiles::iterator fi;
  for (fi = _source_files.begin(); fi != _source_files.end(); ++fi) {
    ExtraData ed;
    ed._type = FT_maya;

    CVSSourceDirectory *dest = import(*fi, &ed, _model_dir);
    if (dest == (CVSSourceDirectory *)NULL) {
      exit(1);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::copy_file
//       Access: Protected, Virtual
//  Description: Called by import() if verify_file() indicates that a
//               file needs to be copied.  This does the actual copy
//               of a file from source to destination.  If new_file is
//               true, then dest does not already exist.
////////////////////////////////////////////////////////////////////
bool MayaCopy::
copy_file(const Filename &source, const Filename &dest,
          CVSSourceDirectory *dir, void *extra_data, bool new_file) {
  ExtraData *ed = (ExtraData *)extra_data;
  switch (ed->_type) {
  case FT_maya:
    return copy_maya_file(source, dest, dir);

  case FT_texture:
    return copy_texture(source, dest, dir);
  }

  nout << "Internal error: invalid type " << (int)ed->_type << "\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::copy_maya_file
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool MayaCopy::
copy_maya_file(const Filename &source, const Filename &dest,
               CVSSourceDirectory *dir) {
  if (!_maya->read(source)) {
    mayaegg_cat.error()
      << "Unable to read " << source << "\n";
    return false;
  }

  if (!_maya->write(dest)) {
    mayaegg_cat.error()
      << "Cannot write " << dest << "\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::copy_texture
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool MayaCopy::
copy_texture(const Filename &source, const Filename &dest,
             CVSSourceDirectory *dir) {
  if (!copy_binary_file(source, dest)) {
    return false;
  }

  return true;
}

/*
////////////////////////////////////////////////////////////////////
//     Function: MayaCopy::scan_maya
//       Access: Private
//  Description: Recursively walks through the maya file hierarchy,
//               looking for texture references and external maya file
//               references.
////////////////////////////////////////////////////////////////////
void MayaCopy::
scan_maya(MayaRecord *record, MayaCopy::Refs &refs, MayaCopy::Textures &textures) {
  if (record->is_of_type(MayaFace::get_class_type())) {
    MayaFace *face;
    DCAST_INTO_V(face, record);
    if (face->has_texture()) {
      textures.insert(face->get_texture());
    }

  } else if (record->is_of_type(MayaExternalReference::get_class_type())) {
    MayaExternalReference *ref;
    DCAST_INTO_V(ref, record);

    refs.insert(ref);
  }

  int i;
  int num_subfaces = record->get_num_subfaces();
  for (i = 0; i < num_subfaces; i++) {
    scan_maya(record->get_subface(i), refs, textures);
  }

  int num_children = record->get_num_children();
  for (i = 0; i < num_children; i++) {
    scan_maya(record->get_child(i), refs, textures);
  }
}
*/


int main(int argc, char *argv[]) {
  MayaCopy prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
