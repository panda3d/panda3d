// Filename: eggData.cxx
// Created by:  drose (20Jan99)
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

#include "eggData.h"
#include "eggCoordinateSystem.h"
#include "eggTextureCollection.h"
#include "eggMaterialCollection.h"
#include "eggComment.h"
#include "eggPoolUniquifier.h"
#include "config_egg.h"

#include "config_util.h"
#include "config_express.h"
#include "string_utils.h"
#include "dSearchPath.h"
#include "virtualFileSystem.h"

extern int eggyyparse(void);
#include "parserDefs.h"
#include "lexerDefs.h"

TypeHandle EggData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggData::resolve_egg_filename
//       Access: Public, Static
//  Description: Looks for the indicated filename, first along the
//               indicated searchpath, and then along the egg_path and
//               finally along the model_path.  If found, updates the
//               filename to the full path and returns true;
//               otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool EggData::
resolve_egg_filename(Filename &egg_filename, const DSearchPath &searchpath) {
  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

    if (egg_filename.is_fully_qualified() && vfs->exists(egg_filename)) {
      return true;
    }

    vfs->resolve_filename(egg_filename, searchpath, "egg") ||
      vfs->resolve_filename(egg_filename, get_egg_path(), "egg") ||
      vfs->resolve_filename(egg_filename, get_model_path(), "egg");

    return vfs->exists(egg_filename);

  } else {
    if (egg_filename.is_fully_qualified() && egg_filename.exists()) {
      return true;
    }

    egg_filename.resolve_filename(searchpath, "egg") ||
      egg_filename.resolve_filename(get_egg_path(), "egg") ||
      egg_filename.resolve_filename(get_model_path(), "egg");
    
    return egg_filename.exists();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::read
//       Access: Public
//  Description: Opens the indicated filename and reads the egg data
//               contents from it.  Returns true if the file was
//               successfully opened and read, false if there were
//               some errors, in which case the data may be partially
//               read.
//
//               error is the output stream to which to write error
//               messages.
////////////////////////////////////////////////////////////////////
bool EggData::
read(Filename filename, string display_name) {
  filename.set_text();
  set_egg_filename(filename);

  if (display_name.empty()) {
    display_name = filename;
  }

  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    
    istream *file = vfs->open_read_file(filename);
    if (file == (istream *)NULL) {
      egg_cat.error() << "Unable to open " << display_name << "\n";
      return false;
    }
    
    egg_cat.info()
      << "Reading " << display_name << "\n";

    bool read_ok = read(*file);
    delete file;
    return read_ok;

  } else {
    ifstream file;
    if (!filename.open_read(file)) {
      egg_cat.error() << "Unable to open " << display_name << "\n";
      return false;
    }
    
    egg_cat.info()
      << "Reading " << display_name << "\n";
    
    return read(file);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::read
//       Access: Public
//  Description: Parses the egg syntax contained in the indicated
//               input stream.  Returns true if the stream was a
//               completely valid egg file, false if there were some
//               errors, in which case the data may be partially read.
//
//               Before you call this routine, you should probably
//               call set_egg_filename() to set the name of the egg
//               file we're processing, if at all possible.  If there
//               is no such filename, you may set it to the empty
//               string.
////////////////////////////////////////////////////////////////////
bool EggData::
read(istream &in) {
  // First, dispense with any children we had previously.  We will
  // replace them with the new data.
  clear();

  // Create a temporary EggData structure to read into.  We initialize
  // it with a copy of ourselves, so that it will get our _coordsys
  // value, if the user set it.
  PT(EggData) data = new EggData(*this);
  egg_init_parser(in, get_egg_filename(), data, data);
  eggyyparse();
  egg_cleanup_parser();

  data->post_read();

  steal_children(*data);
  (*this) = *data;

  return (egg_error_count() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::merge
//       Access: Public
//  Description: Appends the other egg structure to the end of this
//               one.  The other egg structure is invalidated.
////////////////////////////////////////////////////////////////////
void EggData::
merge(EggData &other) {
  if (get_coordinate_system() == CS_default) {
    // If we haven't specified a coordinate system yet, we inherit the
    // other one's.
    set_coordinate_system(other.get_coordinate_system());

  } else {
    // Otherwise, the other one is forced into our coordinate system
    // before we merge.
    other.set_coordinate_system(get_coordinate_system());
  }
  steal_children(other);
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::load_externals
//       Access: Public
//  Description: Loads up all the egg files referenced by <File>
//               entries within the egg structure, and inserts their
//               contents in place of the <File> entries.  Searches
//               for files in the searchpath, if not found directly,
//               and writes error messages to the indicated output
//               stream.  Returns true if all externals were loaded
//               successfully, false otherwise.
////////////////////////////////////////////////////////////////////
bool EggData::
load_externals(const DSearchPath &searchpath) {
  return
    r_load_externals(searchpath, get_coordinate_system());
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::collapse_equivalent_textures
//       Access: Public
//  Description: Removes duplicate references to the same texture
//               image with the same properties.  Considers two
//               texture references with identical properties, but
//               different tref names, to be equivalent, and collapses
//               them, choosing one tref name to keep arbitrarily.
//               Returns the number of textures removed.
////////////////////////////////////////////////////////////////////
int EggData::
collapse_equivalent_textures() {
  EggTextureCollection textures;
  textures.find_used_textures(this);
  return
    textures.collapse_equivalent_textures(~EggTexture::E_tref_name, this);
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::collapse_equivalent_materials
//       Access: Public
//  Description: Removes duplicate references to the same material
//               with the same properties.  Considers two material
//               references with identical properties, but different
//               mref names, to be equivalent, and collapses them,
//               choosing one mref name to keep arbitrarily.  Returns
//               the number of materials removed.
////////////////////////////////////////////////////////////////////
int EggData::
collapse_equivalent_materials() {
  EggMaterialCollection materials;
  materials.find_used_materials(this);
  return
    materials.collapse_equivalent_materials(~EggMaterial::E_mref_name, this);
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::write_egg
//       Access: Public
//  Description: The main interface for writing complete egg files.
////////////////////////////////////////////////////////////////////
bool EggData::
write_egg(Filename filename) {
  filename.set_text();
  filename.unlink();

  ofstream file;
  if (!filename.open_write(file)) {
    egg_cat.error() << "Unable to open " << filename << " for writing.\n";
    return false;
  }

  return write_egg(file);
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::write_egg
//       Access: Public
//  Description: The main interface for writing complete egg files.
////////////////////////////////////////////////////////////////////
bool EggData::
write_egg(ostream &out) {
  pre_write();
  write(out, 0);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::set_coordinate_system
//       Access: Public
//  Description: Changes the coordinate system of the EggData.  If the
//               coordinate system was previously different, this may
//               result in a conversion of the data.
////////////////////////////////////////////////////////////////////
void EggData::
set_coordinate_system(CoordinateSystem new_coordsys) {
  if (new_coordsys == CS_default) {
    new_coordsys = default_coordinate_system;
  }
  if (new_coordsys != _coordsys &&
      (_coordsys != CS_default && _coordsys != CS_invalid)) {
    // Time to convert the data.
    r_transform(LMatrix4d::convert_mat(_coordsys, new_coordsys),
                LMatrix4d::convert_mat(new_coordsys, _coordsys),
                new_coordsys);

    // Now we have to update the under_flags to ensure that all the
    // cached relative matrices are correct.
    update_under(0);
  }

  _coordsys = new_coordsys;
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::write
//       Access: Protected, Virtual
//  Description: Writes the egg data out to the indicated output
//               stream.
////////////////////////////////////////////////////////////////////
void EggData::
write(ostream &out, int indent_level) const {
  EggCoordinateSystem ecs(_coordsys);
  ecs.write(out, indent_level);
  EggGroupNode::write(out, indent_level);
  out << flush;
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::post_read
//       Access: Private
//  Description: Does whatever processing is appropriate after reading
//               the data in from an egg file.
////////////////////////////////////////////////////////////////////
void EggData::
post_read() {
  CoordinateSystem old_coordsys = _coordsys;
  _coordsys = find_coordsys_entry();

  if (_coordsys == CS_default) {
    // If the egg file didn't contain a <CoordinateSystem> entry,
    // assume it's Y-up, by convention.
    _coordsys = CS_yup_right;

  } else if (_coordsys == CS_invalid) {
    egg_cat.warning()
      << "Contradictory <CoordinateSystem> entries encountered.\n";
    _coordsys = CS_yup_right;
  }

  r_mark_coordsys(_coordsys);

  if (old_coordsys != CS_default) {
    // Now if we had a previous definition, enforce it.  This might
    // convert the data to the given coordinate system.
    set_coordinate_system(old_coordsys);
  }

  if (get_auto_resolve_externals()) {
    // Resolve filenames that are relative to the egg file.
    DSearchPath dir;
    dir.append_directory(get_egg_filename().get_dirname());
    resolve_filenames(dir);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::pre_write
//       Access: Private
//  Description: Does whatever processing is appropriate just before
//               writing the data out to an egg file.  This includes
//               verifying that vertex pool names are unique, etc.
////////////////////////////////////////////////////////////////////
void EggData::
pre_write() {
  // Pull out all of the texture definitions in the file and massage
  // them a bit.
  EggTextureCollection textures;
  textures.extract_textures(this);

  // Remove any textures that aren't being used.
  textures.remove_unused_textures(this);

  // Collapse out any textures that are completely equivalent.  For
  // this purpose, we consider two textures with identical properties
  // but different tref names to be different.
  textures.collapse_equivalent_textures(~0, this);

  // Make sure all of the textures have unique TRef names.
  textures.uniquify_trefs();
  textures.sort_by_tref();

  // Do the same thing with the materials.
  EggMaterialCollection materials;
  materials.extract_materials(this);
  materials.remove_unused_materials(this);
  materials.collapse_equivalent_materials(~0, this);
  materials.uniquify_mrefs();
  materials.sort_by_mref();

  // Now put them all back at the head of the file, after any initial
  // comment records.
  iterator ci = begin();
  while (ci != end() && (*ci)->is_of_type(EggComment::get_class_type())) {
    ++ci;
  }
  textures.insert_textures(this, ci);
  materials.insert_materials(this, ci);

  // Also make sure that the vertex pools are uniquely named.  This
  // also checks textures and materials, which is kind of redundant
  // since we just did that, but we don't mind.
  EggPoolUniquifier pu;
  pu.uniquify(this);
}
