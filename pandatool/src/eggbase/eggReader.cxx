// Filename: eggReader.cxx
// Created by:  drose (14Feb00)
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

#include "eggReader.h"

#include "pnmImage.h"
#include "config_util.h"
#include "eggTextureCollection.h"

////////////////////////////////////////////////////////////////////
//     Function: EggReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggReader::
EggReader() {
  clear_runlines();
  add_runline("[opts] input.egg");

  redescribe_option
    ("cs",
     "Specify the coordinate system to operate in.  This may be "
     " one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default "
     "is the coordinate system of the input egg file.");

  add_option
    ("f", "", 80,
     "Force complete loading: load up the egg file along with all of its "
     "external references.",
     &EggReader::dispatch_none, &_force_complete);

  _tex_type = (PNMFileType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRead::add_texture_options
//       Access: Public
//  Description: Adds -td, -te, etc. as valid options for this
//               program.  If the user specifies one of the options on
//               the command line, the textures will be copied and
//               converted as each egg file is read.
////////////////////////////////////////////////////////////////////
void EggReader::
add_texture_options() {
  add_option
    ("td", "dirname", 40,
     "Copy textures to the indicated directory.  The copy is performed "
     "only if the destination file does not exist or is older than the "
     "source file.",
     &EggReader::dispatch_filename, &_got_tex_dirname, &_tex_dirname);

  add_option
    ("te", "ext", 40,
     "Rename textures to have the indicated extension.  This also "
     "automatically copies them to the new filename (possibly in a "
     "different directory if -td is also specified), and may implicitly "
     "convert to a different image format according to the extension.",
     &EggReader::dispatch_string, &_got_tex_extension, &_tex_extension);

  add_option
    ("tt", "type", 40,
     "Explicitly specifies the image format to convert textures to "
     "when copying them via -td or -te.  Normally, this is unnecessary as "
     "the image format can be determined by the extension, but sometimes "
     "the extension is insufficient to unambiguously specify an image "
     "type.",
     &EggReader::dispatch_image_type, NULL, &_tex_type);
}

////////////////////////////////////////////////////////////////////
//     Function: EggReader::as_reader
//       Access: Public, Virtual
//  Description: Returns this object as an EggReader pointer, if it is
//               in fact an EggReader, or NULL if it is not.
//
//               This is intended to work around the C++ limitation
//               that prevents downcasts past virtual inheritance.
//               Since both EggReader and EggWriter inherit virtually
//               from EggBase, we need functions like this to downcast
//               to the appropriate pointer.
////////////////////////////////////////////////////////////////////
EggReader *EggReader::
as_reader() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggReader::handle_args
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool EggReader::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the egg file(s) to read on the command line.\n";
    return false;
  }

  // Any separate egg files that are listed on the command line will
  // get implicitly loaded up into one big egg file.

  DSearchPath local_path(".");

  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    Filename filename = *ai;
    // First, we always try to resolve a filename from the current
    // directory.  This means a local filename will always be found
    // before the model path is searched.
    filename.resolve_filename(local_path);

    if (!_data.read(filename)) {
      // Rather than returning false, we simply exit here, so the
      // ProgramBase won't try to tell the user how to run the program
      // just because we got a bad egg file.
      exit(1);
    }

    if (_force_complete) {
      if (!_data.resolve_externals()) {
        exit(1);
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggReader::post_command_line
//       Access: Protected, Virtual
//  Description: This is called after the command line has been
//               completely processed, and it gives the program a
//               chance to do some last-minute processing and
//               validation of the options and arguments.  It should
//               return true if everything is fine, false if there is
//               an error.
////////////////////////////////////////////////////////////////////
bool EggReader::
post_command_line() {
  if (!copy_textures()) {
    exit(1);
  }

  return EggBase::post_command_line();
}

////////////////////////////////////////////////////////////////////
//     Function: EggReader::copy_textures
//       Access: Protected
//  Description: Renames and copies the textures referenced in the egg
//               file, if so specified by the -td and -te options.
//               Returns true if all textures are copied successfully,
//               false if any one of them failed.
////////////////////////////////////////////////////////////////////
bool EggReader::
copy_textures() {
  if (!_got_tex_dirname && !_got_tex_extension) {
    return true;
  }

  bool success = true;
  EggTextureCollection textures;
  textures.find_used_textures(&_data);

  EggTextureCollection::const_iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    EggTexture *tex = (*ti);
    Filename orig_filename = tex->get_filename();
    if (!orig_filename.exists()) {
      bool found = 
        orig_filename.resolve_filename(get_texture_path()) ||
        orig_filename.resolve_filename(get_model_path());
      if (!found) {
        nout << "Cannot find " << orig_filename << "\n";
        success = false;
        continue;
      }
    }

    Filename new_filename = orig_filename;
    if (_got_tex_dirname) {
      new_filename.set_dirname(_tex_dirname);
    }
    if (_got_tex_extension) {
      new_filename.set_extension(_tex_extension);
    }

    if (orig_filename != new_filename) {
      tex->set_filename(new_filename);

      // The new filename is different; does it need copying?
      int compare = 
        orig_filename.compare_timestamps(new_filename, true, true);
      if (compare > 0) {
        // Yes, it does.  Copy it!
        nout << "Reading " << orig_filename << "\n";
        PNMImage image;
        if (!image.read(orig_filename)) {
          nout << "  unable to read!\n";
          success = false;
        } else {
          nout << "Writing " << new_filename << "\n";
          if (!image.write(new_filename, _tex_type)) {
            nout << "  unable to write!\n";
            success = false;
          }
        }
      }
    }
  }

  return success;
}
