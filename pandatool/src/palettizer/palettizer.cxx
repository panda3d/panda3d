/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file palettizer.cxx
 * @author drose
 * @date 2000-12-01
 */

#include "palettizer.h"
#include "eggFile.h"
#include "textureImage.h"
#include "pal_string_utils.h"
#include "paletteGroup.h"
#include "filenameUnifier.h"
#include "textureMemoryCounter.h"

#include "pnmImage.h"
#include "pnmFileTypeRegistry.h"
#include "pnmFileType.h"
#include "eggData.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"

using std::cout;
using std::string;

Palettizer *pal = nullptr;

// This number is written out as the first number to the pi file, to indicate
// the version of egg-palettize that wrote it out.  This allows us to easily
// update egg-palettize to write out additional information to its pi file,
// without having it increment the bam version number for all bam and boo
// files anywhere in the world.
int Palettizer::_pi_version = 20;
/*
 * Updated to version 8 on 32003 to remove extensions from texture key names.
 * Updated to version 9 on 41303 to add a few properties in various places.
 * Updated to version 10 on 41503 to add _alpha_file_channel.  Updated to
 * version 11 on 43003 to add TextureReference::_tref_name.  Updated to
 * version 12 on 91103 to add _generated_image_pattern.  Updated to version 13
 * on 91303 to add _keep_format and _background.  Updated to version 14 on
 * 72605 to add _omit_everything.  Updated to version 15 on 80105 to make
 * TextureImages be case-insensitive.  Updated to version 16 on 40306 to add
 * Palettizer::_cutout_mode et al.  Updated to version 17 on 30207 to add
 * TextureImage::_txa_wrap_u etc.  Updated to version 18 on 51308 to add
 * TextureProperties::_quality_level.  Updated to version 19 on 71609 to add
 * PaletteGroup::_override_margin Updated to version 20 on 72709 to add
 * TexturePlacement::_swapTextures
 */

int Palettizer::_min_pi_version = 8;
// Dropped support for versions 7 and below on 71403.

int Palettizer::_read_pi_version = 0;

TypeHandle Palettizer::_type_handle;

std::ostream &operator << (std::ostream &out, Palettizer::RemapUV remap) {
  switch (remap) {
  case Palettizer::RU_never:
    return out << "never";

  case Palettizer::RU_group:
    return out << "per group";

  case Palettizer::RU_poly:
    return out << "per polygon";

  case Palettizer::RU_invalid:
    return out << "(invalid)";
  }

  return out << "**invalid**(" << (int)remap << ")";
}


// This STL function object is used in report_statistics(), below.
class SortGroupsByDependencyOrder {
public:
  bool operator ()(PaletteGroup *a, PaletteGroup *b) {
    if (a->get_dependency_order() != b->get_dependency_order()) {
      return a->get_dependency_order() < b->get_dependency_order();
    }
    return a->get_name() < b->get_name();
  }
};

// And this one is used in report_pi().
class SortGroupsByPreference {
public:
  bool operator ()(PaletteGroup *a, PaletteGroup *b) {
    return !a->is_preferred_over(*b);
  }
};

/**
 *
 */
Palettizer::
Palettizer() {
  _is_valid = true;
  _noabs = false;

  _generated_image_pattern = "%g_palette_%p_%i";
  _map_dirname = "%g";
  _shadow_dirname = "shadow";
  _margin = 2;
  _omit_solitary = false;
  _omit_everything = false;
  _coverage_threshold = 2.5;
  _aggressively_clean_mapdir = true;
  _force_power_2 = true;
  _color_type = PNMFileTypeRegistry::get_global_ptr()->get_type_from_extension("png");
  _alpha_type = nullptr;
  _shadow_color_type = nullptr;
  _shadow_alpha_type = nullptr;
  _pal_x_size = _pal_y_size = 512;
  _background.set(0.0, 0.0, 0.0, 0.0);
  _cutout_mode = EggRenderMode::AM_dual;
  _cutout_ratio = 0.3;

  _round_uvs = true;
  _round_unit = 0.1;
  _round_fuzz = 0.01;
  _remap_uv = RU_poly;
  _remap_char_uv = RU_poly;

  get_palette_group("null");
}

/**
 * Returns the current setting of the noabs flag.  See set_noabs().
 */
bool Palettizer::
get_noabs() const {
  return _noabs;
}

/**
 * Changes the current setting of the noabs flag.
 *
 * If this flag is true, then it is an error to process an egg file that
 * contains absolute pathname references.  This flag is intended to help
 * detect egg files that are incorrectly built within a model tree (which
 * should use entirely relative pathnames).
 *
 * This flag must be set before any egg files are processed.
 */
void Palettizer::
set_noabs(bool noabs) {
  _noabs = noabs;
}

/**
 * Returns true if the palette information file was read correctly, or false
 * if there was some error and the palettization can't continue.
 */
bool Palettizer::
is_valid() const {
  return _is_valid;
}

/**
 * Output a verbose description of all the palettization information to
 * standard output, for the user's perusal.
 */
void Palettizer::
report_pi() const {
  // Start out with the cross links and back counts; some of these are nice to
  // report.
  EggFiles::const_iterator efi;
  for (efi = _egg_files.begin(); efi != _egg_files.end(); ++efi) {
    (*efi).second->build_cross_links();
  }

  cout
    << "\nparams\n"
    << "  generated image pattern: " << _generated_image_pattern << "\n"
    << "  map directory: " << _map_dirname << "\n"
    << "  shadow directory: "
    << FilenameUnifier::make_user_filename(_shadow_dirname) << "\n"
    << "  egg relative directory: "
    << FilenameUnifier::make_user_filename(_rel_dirname) << "\n"
    << "  palettize size: " << _pal_x_size << " by " << _pal_y_size << "\n"
    << "  background: " << _background << "\n"
    << "  margin: " << _margin << "\n"
    << "  coverage threshold: " << _coverage_threshold << "\n"
    << "  force textures to power of 2: " << yesno(_force_power_2) << "\n"
    << "  aggressively clean the map directory: "
    << yesno(_aggressively_clean_mapdir) << "\n"
    << "  omit everything: " << yesno(_omit_everything) << "\n"
    << "  round UV area: " << yesno(_round_uvs) << "\n";
  if (_round_uvs) {
    cout << "  round UV area to nearest " << _round_unit << " with fuzz "
         << _round_fuzz << "\n";
  }
  cout << "  remap UV's: " << _remap_uv << "\n"
       << "  remap UV's for characters: " << _remap_char_uv << "\n";
  cout << "  alpha cutouts: " << _cutout_mode << " " << _cutout_ratio << "\n";

  if (_color_type != nullptr) {
    cout << "  generate image files of type: "
         << _color_type->get_suggested_extension();
    if (_alpha_type != nullptr) {
      cout << "," << _alpha_type->get_suggested_extension();
    }
    cout << "\n";
  }

  if (_shadow_color_type != nullptr) {
    cout << "  generate shadow palette files of type: "
         << _shadow_color_type->get_suggested_extension();
    if (_shadow_alpha_type != nullptr) {
      cout << "," << _shadow_alpha_type->get_suggested_extension();
    }
    cout << "\n";
  }

  cout << "\ntexture source pathnames and assignments\n";
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureImage *texture = (*ti).second;
    if (texture->is_used()) {
      cout << "  " << texture->get_name() << ":\n";
      texture->write_source_pathnames(cout, 4);
    }
  }

  cout << "\negg files and textures referenced\n";
  EggFiles::const_iterator ei;
  for (ei = _egg_files.begin(); ei != _egg_files.end(); ++ei) {
    EggFile *egg_file = (*ei).second;
    egg_file->write_description(cout, 2);
    egg_file->write_texture_refs(cout, 4);
  }

  // Sort the palette groups into order of preference, so that the more
  // specific ones appear at the bottom.
  pvector<PaletteGroup *> sorted_groups;
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    sorted_groups.push_back((*gi).second);
  }
  sort(sorted_groups.begin(), sorted_groups.end(),
       SortGroupsByPreference());

  cout << "\npalette groups\n";
  pvector<PaletteGroup *>::iterator si;
  for (si = sorted_groups.begin(); si != sorted_groups.end(); ++si) {
    PaletteGroup *group = (*si);
    if (si != sorted_groups.begin()) {
      cout << "\n";
    }
    cout << "  " << group->get_name()
      // << " (" << group->get_dirname_order() << "," <<
      // group->get_dependency_order() << ")"
         << ": " << group->get_groups() << "\n";
    group->write_image_info(cout, 4);
  }

  cout << "\ntextures\n";
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureImage *texture = (*ti).second;
    texture->write_scale_info(cout, 2);
  }

  cout << "\nsurprises\n";
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureImage *texture = (*ti).second;
    if (texture->is_surprise()) {
      cout << "  " << texture->get_name() << "\n";
    }
  }
  for (ei = _egg_files.begin(); ei != _egg_files.end(); ++ei) {
    EggFile *egg_file = (*ei).second;
    if (egg_file->is_surprise()) {
      cout << "  " << egg_file->get_name() << "\n";
    }
  }

  cout << "\n";
}

/**
 * Output a report of the palettization effectiveness, texture memory
 * utilization, and so on.
 */
void Palettizer::
report_statistics() const {
  // Sort the groups into order by dependency order, for the user's
  // convenience.
  pvector<PaletteGroup *> sorted_groups;

  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    sorted_groups.push_back((*gi).second);
  }

  sort(sorted_groups.begin(), sorted_groups.end(),
       SortGroupsByDependencyOrder());

  Placements overall_placements;

  pvector<PaletteGroup *>::const_iterator si;
  for (si = sorted_groups.begin();
       si != sorted_groups.end();
       ++si) {
    PaletteGroup *group = (*si);

    Placements placements;
    group->get_placements(placements);
    if (!placements.empty()) {
      group->get_placements(overall_placements);

      cout << "\n" << group->get_name() << ", by itself:\n";
      compute_statistics(cout, 2, placements);

      PaletteGroups complete;
      complete.make_complete(group->get_groups());

      if (complete.size() > 1) {
        Placements complete_placements;
        group->get_complete_placements(complete_placements);
        if (complete_placements.size() != placements.size()) {
          cout << "\n" << group->get_name()
               << ", with dependents (" << complete << "):\n";
          compute_statistics(cout, 2, complete_placements);
        }
      }
    }
  }

  cout << "\nOverall:\n";
  compute_statistics(cout, 2, overall_placements);

  cout << "\n";
}


/**
 * Reads in the .txa file and keeps it ready for matching textures and egg
 * files.
 */
void Palettizer::
read_txa_file(std::istream &txa_file, const string &txa_filename) {
  // Clear out the group dependencies, in preparation for reading them again
  // from the .txa file.
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->clear_depends();
    group->set_dirname("");
  }

  // Also reset _shadow_color_type.
  _shadow_color_type = nullptr;
  _shadow_alpha_type = nullptr;

  if (!_txa_file.read(txa_file, txa_filename)) {
    exit(1);
  }

  if (_color_type == nullptr) {
    nout << "No valid output image file type available; cannot run.\n"
         << "Use :imagetype command in .txa file.\n";
    exit(1);
  }

  // Compute the correct dependency level and order for each group.  This will
  // help us when we assign the textures to their groups.
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->reset_dependency_level();
  }

  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->set_dependency_level(1);
  }

  bool any_changed;
  do {
    any_changed = false;
    for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
      PaletteGroup *group = (*gi).second;
      if (group->set_dependency_order()) {
        any_changed = true;
      }
    }
  } while (any_changed);
}

/**
 * Called after all command line parameters have been set up, this is a hook
 * to do whatever initialization is necessary.
 */
void Palettizer::
all_params_set() {
  // Make sure the palettes have their shadow images set up properly.
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->setup_shadow_images();
  }
}

/**
 * Processes all the textures named in the _command_line_eggs, placing them on
 * the appropriate palettes or whatever needs to be done with them.
 *
 * If force_texture_read is true, it forces each texture image file to be read
 * (and thus legitimately checked for grayscaleness etc.) before placing.
 */
void Palettizer::
process_command_line_eggs(bool force_texture_read, const Filename &state_filename) {
  _command_line_textures.clear();

  // Start by scanning all the egg files we read up on the command line.
  CommandLineEggs::const_iterator ei;
  for (ei = _command_line_eggs.begin();
       ei != _command_line_eggs.end();
       ++ei) {
    EggFile *egg_file = (*ei);

    egg_file->scan_textures();
    egg_file->get_textures(_command_line_textures);

    egg_file->pre_txa_file();
    _txa_file.match_egg(egg_file);
    egg_file->post_txa_file();
  }

  // Now that all of our egg files are read in, build in all the cross links
  // and back pointers and stuff.
  EggFiles::const_iterator efi;
  for (efi = _egg_files.begin(); efi != _egg_files.end(); ++efi) {
    (*efi).second->build_cross_links();
  }

  // Now match each of the textures mentioned in those egg files against a
  // line in the .txa file.
  CommandLineTextures::iterator ti;
  for (ti = _command_line_textures.begin();
       ti != _command_line_textures.end();
       ++ti) {
    TextureImage *texture = *ti;

    if (force_texture_read || texture->is_newer_than(state_filename)) {
      // If we're forcing a redo, or the texture image has changed, re-read
      // the complete image.
      texture->read_source_image();
    } else {
      // Otherwise, just the header is sufficient.
      texture->read_header();
    }

    texture->mark_texture_named();
    texture->pre_txa_file();
    _txa_file.match_texture(texture);
    texture->post_txa_file();
  }

  // And now, assign each of the current set of textures to an appropriate
  // group or groups.
  for (ti = _command_line_textures.begin();
       ti != _command_line_textures.end();
       ++ti) {
    TextureImage *texture = *ti;
    texture->assign_groups();
  }

  // And then the egg files need to sign up for a particular TexturePlacement,
  // so we can determine some more properties about how the textures are
  // placed (for instance, how big the UV range is for a particular
  // TexturePlacement).
  for (efi = _egg_files.begin(); efi != _egg_files.end(); ++efi) {
    (*efi).second->choose_placements();
  }

  // Now that *that's* done, we need to make sure the various
  // TexturePlacements require the right size for their textures.
  for (ti = _command_line_textures.begin();
       ti != _command_line_textures.end();
       ++ti) {
    TextureImage *texture = *ti;
    texture->determine_placement_size();
  }

  // Now that each texture has been assigned to a suitable group, make sure
  // the textures are placed on specific PaletteImages.
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->update_unknown_textures(_txa_file);
    group->place_all();
  }
}

/**
 * Reprocesses all textures known.
 *
 * If force_texture_read is true, it forces each texture image file to be read
 * (and thus legitimately checked for grayscaleness etc.) before placing.
 */
void Palettizer::
process_all(bool force_texture_read, const Filename &state_filename) {
  // First, clear all the basic properties on the source texture images, so we
  // can reapply them from the complete set of egg files and thereby ensure
  // they are up-to-date.
  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureImage *texture = (*ti).second;
    texture->clear_source_basic_properties();
  }

  // If there *were* any egg files on the command line, deal with them.
  CommandLineEggs::const_iterator ei;
  for (ei = _command_line_eggs.begin();
       ei != _command_line_eggs.end();
       ++ei) {
    EggFile *egg_file = (*ei);

    egg_file->scan_textures();
    egg_file->get_textures(_command_line_textures);
  }

  // Then match up all the egg files we know about with the .txa file.
  EggFiles::const_iterator efi;
  for (efi = _egg_files.begin(); efi != _egg_files.end(); ++efi) {
    EggFile *egg_file = (*efi).second;
    egg_file->pre_txa_file();
    _txa_file.match_egg(egg_file);
    egg_file->post_txa_file();
  }

  // Now that all of our egg files are read in, build in all the cross links
  // and back pointers and stuff.
  for (efi = _egg_files.begin(); efi != _egg_files.end(); ++efi) {
    (*efi).second->build_cross_links();

    // Also make sure each egg file's properties are applied to the source
    // image (since we reset all the source image properties, above).
    (*efi).second->apply_properties_to_source();
  }

  // Now match each of the textures in the world against a line in the .txa
  // file.
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureImage *texture = (*ti).second;
    if (force_texture_read || texture->is_newer_than(state_filename)) {
      texture->read_source_image();
    }

    texture->mark_texture_named();
    texture->pre_txa_file();
    _txa_file.match_texture(texture);
    texture->post_txa_file();

    // We need to do this to avoid bloating memory.
    texture->release_source_image();
  }

  // And now, assign each texture to an appropriate group or groups.
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureImage *texture = (*ti).second;
    texture->assign_groups();
  }

  // And then the egg files need to sign up for a particular TexturePlacement,
  // so we can determine some more properties about how the textures are
  // placed (for instance, how big the UV range is for a particular
  // TexturePlacement).
  for (efi = _egg_files.begin(); efi != _egg_files.end(); ++efi) {
    (*efi).second->choose_placements();
  }

  // Now that *that's* done, we need to make sure the various
  // TexturePlacements require the right size for their textures.
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureImage *texture = (*ti).second;
    texture->determine_placement_size();
  }

  // Now that each texture has been assigned to a suitable group, make sure
  // the textures are placed on specific PaletteImages.
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->update_unknown_textures(_txa_file);
    group->place_all();
  }
}

/**
 * Attempts to resize each PalettteImage down to its smallest possible size.
 */
void Palettizer::
optimal_resize() {
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->optimal_resize();
  }
}

/**
 * Throws away all of the current PaletteImages, so that new ones may be
 * created (and the packing made more optimal).
 */
void Palettizer::
reset_images() {
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->reset_images();
  }
}

/**
 * Actually generates the appropriate palette and unplaced texture images into
 * the map directories.  If redo_all is true, this forces a regeneration of
 * each image file.
 */
void Palettizer::
generate_images(bool redo_all) {
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    group->update_images(redo_all);
  }

  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureImage *texture = (*ti).second;
    texture->copy_unplaced(redo_all);
  }
}

/**
 * Reads in any egg file that is known to be stale, even if it was not listed
 * on the command line, so that it may be updated and written out when
 * write_eggs() is called.  If redo_all is true, this even reads egg files
 * that were not flagged as stale.
 *
 * Returns true if successful, or false if there was some error.
 */
bool Palettizer::
read_stale_eggs(bool redo_all) {
  bool okflag = true;

  pvector<EggFiles::iterator> invalid_eggs;

  EggFiles::iterator ei;
  for (ei = _egg_files.begin(); ei != _egg_files.end(); ++ei) {
    EggFile *egg_file = (*ei).second;
    if (!egg_file->had_data() &&
        (egg_file->is_stale() || redo_all)) {
      if (!egg_file->read_egg(_noabs)) {
        invalid_eggs.push_back(ei);

      } else {
        egg_file->scan_textures();
        egg_file->choose_placements();
        egg_file->release_egg_data();
      }
    }
  }

  // Now eliminate all the invalid egg files.
  pvector<EggFiles::iterator>::iterator ii;
  for (ii = invalid_eggs.begin(); ii != invalid_eggs.end(); ++ii) {
    EggFiles::iterator ei = (*ii);
    EggFile *egg_file = (*ei).second;
    if (egg_file->get_source_filename().exists()) {
      // If there is an invalid egg file, remove it; hopefully it will get
      // rebuilt properly next time.
      nout << "Removing invalid egg file: "
           << FilenameUnifier::make_user_filename(egg_file->get_source_filename())
           << "\n";

      egg_file->get_source_filename().unlink();
      okflag = false;

    } else {
      // If the egg file is simply missing, quietly remove any record of it
      // from the database.
      egg_file->remove_egg();
      _egg_files.erase(ei);
    }
  }

  if (!okflag) {
    nout << "\n"
         << "Some errors in egg files encountered.\n"
         << "Re-run make install or make opt-pal to try to regenerate these.\n\n";
  }

  return okflag;
}

/**
 * Adjusts the egg files to reference the newly generated textures, and writes
 * them out.  Returns true if successful, or false if there was some error.
 */
bool Palettizer::
write_eggs() {
  bool okflag = true;

  EggFiles::iterator ei;
  for (ei = _egg_files.begin(); ei != _egg_files.end(); ++ei) {
    EggFile *egg_file = (*ei).second;
    if (egg_file->had_data()) {
      if (!egg_file->has_data()) {
        // Re-read the egg file.
        bool read_ok = egg_file->read_egg(_noabs);
        if (!read_ok) {
          nout << "Error!  Unable to re-read egg file.\n";
          okflag = false;
        }
      }

      if (egg_file->has_data()) {
        egg_file->update_egg();
        if (!egg_file->write_egg()) {
          okflag = false;
        }
        egg_file->release_egg_data();
      }
    }
  }

  return okflag;
}

/**
 * Returns the EggFile with the given name.  If there is no EggFile with the
 * indicated name, creates one.  This is the key name used to sort the egg
 * files, which is typically the basename of the filename.
 */
EggFile *Palettizer::
get_egg_file(const string &name) {
  EggFiles::iterator ei = _egg_files.find(name);
  if (ei != _egg_files.end()) {
    return (*ei).second;
  }

  EggFile *file = new EggFile;
  file->set_name(name);
  _egg_files.insert(EggFiles::value_type(name, file));
  return file;
}

/**
 * Removes the named egg file from the database, if it exists.  Returns true
 * if the egg file was found, false if it was not.
 */
bool Palettizer::
remove_egg_file(const string &name) {
  EggFiles::iterator ei = _egg_files.find(name);
  if (ei != _egg_files.end()) {
    EggFile *file = (*ei).second;
    file->remove_egg();
    _egg_files.erase(ei);
    return true;
  }

  return false;
}

/**
 * Adds the indicated EggFile to the list of eggs that are considered to have
 * been read on the command line.  These will be processed by
 * process_command_line_eggs().
 */
void Palettizer::
add_command_line_egg(EggFile *egg_file) {
  _command_line_eggs.push_back(egg_file);
}

/**
 * Returns the PaletteGroup with the given name.  If there is no PaletteGroup
 * with the indicated name, creates one.
 */
PaletteGroup *Palettizer::
get_palette_group(const string &name) {
  Groups::iterator gi = _groups.find(name);
  if (gi != _groups.end()) {
    return (*gi).second;
  }

  PaletteGroup *group = new PaletteGroup;
  group->set_name(name);
  _groups.insert(Groups::value_type(name, group));
  return group;
}

/**
 * Returns the PaletteGroup with the given name.  If there is no PaletteGroup
 * with the indicated name, returns NULL.
 */
PaletteGroup *Palettizer::
test_palette_group(const string &name) const {
  Groups::const_iterator gi = _groups.find(name);
  if (gi != _groups.end()) {
    return (*gi).second;
  }

  return nullptr;
}

/**
 * Returns the default group to which an egg file should be assigned if it is
 * not mentioned in the .txa file.
 */
PaletteGroup *Palettizer::
get_default_group() {
  PaletteGroup *default_group = get_palette_group(_default_groupname);
  if (!_default_groupdir.empty() && !default_group->has_dirname()) {
    default_group->set_dirname(_default_groupdir);
  }
  return default_group;
}

/**
 * Returns the TextureImage with the given name.  If there is no TextureImage
 * with the indicated name, creates one.  This is the key name used to sort
 * the textures, which is typically the basename of the primary filename.
 */
TextureImage *Palettizer::
get_texture(const string &name) {
  // Look first in the same-case name, just in case it happens to be there
  // (from an older version of egg-palettize that did this).
  Textures::iterator ti = _textures.find(name);
  if (ti != _textures.end()) {
    return (*ti).second;
  }

  // Then look in the downcase name, since we nowadays index textures only by
  // their downcase names (to implement case insensitivity).
  string downcase_name = downcase(name);
  ti = _textures.find(downcase_name);
  if (ti != _textures.end()) {
    return (*ti).second;
  }

  TextureImage *image = new TextureImage;
  image->set_name(name);
  // image->set_filename(name);
  _textures.insert(Textures::value_type(downcase_name, image));

  return image;
}

/**
 * A silly function to return "yes" or "no" based on a bool flag for nicely
 * formatted output.
 */
const char *Palettizer::
yesno(bool flag) {
  return flag ? "yes" : "no";
}

/**
 * Returns the RemapUV code corresponding to the indicated string, or
 * RU_invalid if the string is invalid.
 */
Palettizer::RemapUV Palettizer::
string_remap(const string &str) {
  if (str == "never") {
    return RU_never;

  } else if (str == "group") {
    return RU_group;

  } else if (str == "poly") {
    return RU_poly;

  } else {
    return RU_invalid;
  }
}

/**
 * Determines how much memory, etc.  is required by the indicated set of
 * texture placements, and reports this to the indicated output stream.
 */
void Palettizer::
compute_statistics(std::ostream &out, int indent_level,
                   const Palettizer::Placements &placements) const {
  TextureMemoryCounter counter;

  Placements::const_iterator pi;
  for (pi = placements.begin(); pi != placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);
    counter.add_placement(placement);
  }

  counter.report(out, indent_level);
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void Palettizer::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_Palettizer);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void Palettizer::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);

  datagram.add_int32(_pi_version);
  datagram.add_string(_generated_image_pattern);
  datagram.add_string(_map_dirname);
  datagram.add_string(FilenameUnifier::make_bam_filename(_shadow_dirname));
  datagram.add_string(FilenameUnifier::make_bam_filename(_rel_dirname));
  datagram.add_int32(_pal_x_size);
  datagram.add_int32(_pal_y_size);
  datagram.add_float64(_background[0]);
  datagram.add_float64(_background[1]);
  datagram.add_float64(_background[2]);
  datagram.add_float64(_background[3]);
  datagram.add_int32(_margin);
  datagram.add_bool(_omit_solitary);
  datagram.add_bool(_omit_everything);
  datagram.add_float64(_coverage_threshold);
  datagram.add_bool(_force_power_2);
  datagram.add_bool(_aggressively_clean_mapdir);
  datagram.add_bool(_round_uvs);
  datagram.add_float64(_round_unit);
  datagram.add_float64(_round_fuzz);
  datagram.add_int32((int)_remap_uv);
  datagram.add_int32((int)_remap_char_uv);
  datagram.add_uint8((int)_cutout_mode);
  datagram.add_float64(_cutout_ratio);

  writer->write_pointer(datagram, _color_type);
  writer->write_pointer(datagram, _alpha_type);
  writer->write_pointer(datagram, _shadow_color_type);
  writer->write_pointer(datagram, _shadow_alpha_type);

  datagram.add_int32(_egg_files.size());
  EggFiles::const_iterator ei;
  for (ei = _egg_files.begin(); ei != _egg_files.end(); ++ei) {
    writer->write_pointer(datagram, (*ei).second);
  }

  // We don't write _command_line_eggs; that's specific to each session.

  datagram.add_int32(_groups.size());
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    writer->write_pointer(datagram, (*gi).second);
  }

  datagram.add_int32(_textures.size());
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    writer->write_pointer(datagram, (*ti).second);
  }
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int Palettizer::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int index = TypedWritable::complete_pointers(p_list, manager);

  if (p_list[index] != nullptr) {
    DCAST_INTO_R(_color_type, p_list[index], index);
  }
  index++;

  if (p_list[index] != nullptr) {
    DCAST_INTO_R(_alpha_type, p_list[index], index);
  }
  index++;

  if (p_list[index] != nullptr) {
    DCAST_INTO_R(_shadow_color_type, p_list[index], index);
  }
  index++;

  if (p_list[index] != nullptr) {
    DCAST_INTO_R(_shadow_alpha_type, p_list[index], index);
  }
  index++;

  int i;
  for (i = 0; i < _num_egg_files; i++) {
    EggFile *egg_file;
    DCAST_INTO_R(egg_file, p_list[index], index);
    _egg_files.insert(EggFiles::value_type(egg_file->get_name(), egg_file));
    index++;
  }

  for (i = 0; i < _num_groups; i++) {
    PaletteGroup *group;
    DCAST_INTO_R(group, p_list[index], index);
    _groups.insert(Groups::value_type(group->get_name(), group));
    index++;
  }

  for (i = 0; i < _num_textures; i++) {
    TextureImage *texture;
    DCAST_INTO_R(texture, p_list[index], index);

    string name = downcase(texture->get_name());
    std::pair<Textures::iterator, bool> result = _textures.insert(Textures::value_type(name, texture));
    if (!result.second) {
      // Two textures mapped to the same slot--probably a case error (since we
      // just changed this rule).
      _texture_conflicts.push_back(texture);
    }
    index++;
  }

  return index;
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void Palettizer::
finalize(BamReader *manager) {
  // Walk through the list of texture names that were in conflict.  These can
  // only happen if there were two different names that different only in
  // case, which means the textures.boo file was created before we introduced
  // the rule that case is insignificant.
  TextureConflicts::iterator ci;
  for (ci = _texture_conflicts.begin();
       ci != _texture_conflicts.end();
       ++ci) {
    TextureImage *texture_b = (*ci);
    string downcase_name = downcase(texture_b->get_name());

    Textures::iterator ti = _textures.find(downcase_name);
    nassertv(ti != _textures.end());
    TextureImage *texture_a = (*ti).second;
    _textures.erase(ti);

    if (!texture_b->is_used() || !texture_a->is_used()) {
      // If either texture is not used, there's not really a conflict--the
      // other one wins.
      if (texture_a->is_used()) {
        bool inserted1 = _textures.insert(Textures::value_type(downcase_name, texture_a)).second;
        nassertd(inserted1) { }

      } else if (texture_b->is_used()) {
        bool inserted2 = _textures.insert(Textures::value_type(downcase_name, texture_b)).second;
        nassertd(inserted2) { }
      }

    } else {
      // If both textures are used, there *is* a conflict.
      nout << "Texture name conflict: \"" << texture_a->get_name()
           << "\" vs. \"" << texture_b->get_name() << "\"\n";
      if (texture_a->get_name() != downcase_name &&
          texture_b->get_name() != downcase_name) {
        // Arbitrarily pick texture_a to get the right case.
        bool inserted1 = _textures.insert(Textures::value_type(downcase_name, texture_a)).second;
        bool inserted2 = _textures.insert(Textures::value_type(texture_b->get_name(), texture_b)).second;
        nassertd(inserted1 && inserted2) { }

      } else {
        // One of them is already the right case.
        bool inserted1 = _textures.insert(Textures::value_type(texture_a->get_name(), texture_a)).second;
        bool inserted2 = _textures.insert(Textures::value_type(texture_b->get_name(), texture_b)).second;
        nassertd(inserted1 && inserted2) { }
      }
    }
  }
}


/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable* Palettizer::
make_Palettizer(const FactoryParams &params) {
  Palettizer *me = new Palettizer;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  manager->register_finalize(me);

  return me;
}

/**
 * Reads the binary data from the given datagram iterator, which was written
 * by a previous call to write_datagram().
 */
void Palettizer::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  _read_pi_version = scan.get_int32();
  if (_read_pi_version > _pi_version || _read_pi_version < _min_pi_version) {
    // Oops, we don't know how to read this palette information file.
    _is_valid = false;
    return;
  }
  if (_read_pi_version >= 12) {
    _generated_image_pattern = scan.get_string();
  }
  _map_dirname = scan.get_string();
  _shadow_dirname = FilenameUnifier::get_bam_filename(scan.get_string());
  _rel_dirname = FilenameUnifier::get_bam_filename(scan.get_string());
  FilenameUnifier::set_rel_dirname(_rel_dirname);
  _pal_x_size = scan.get_int32();
  _pal_y_size = scan.get_int32();
  if (_read_pi_version >= 13) {
    _background[0] = scan.get_float64();
    _background[1] = scan.get_float64();
    _background[2] = scan.get_float64();
    _background[3] = scan.get_float64();
  }
  _margin = scan.get_int32();
  _omit_solitary = scan.get_bool();
  if (_read_pi_version >= 14) {
    _omit_everything = scan.get_bool();
  }
  _coverage_threshold = scan.get_float64();
  _force_power_2 = scan.get_bool();
  _aggressively_clean_mapdir = scan.get_bool();
  _round_uvs = scan.get_bool();
  _round_unit = scan.get_float64();
  _round_fuzz = scan.get_float64();
  _remap_uv = (RemapUV)scan.get_int32();
  _remap_char_uv = (RemapUV)scan.get_int32();
  if (_read_pi_version >= 16) {
    _cutout_mode = (EggRenderMode::AlphaMode)scan.get_uint8();
    _cutout_ratio = scan.get_float64();
  }

  manager->read_pointer(scan);  // _color_type
  manager->read_pointer(scan);  // _alpha_type
  manager->read_pointer(scan);  // _shadow_color_type
  manager->read_pointer(scan);  // _shadow_alpha_type

  _num_egg_files = scan.get_int32();
  manager->read_pointers(scan, _num_egg_files);

  _num_groups = scan.get_int32();
  manager->read_pointers(scan, _num_groups);

  _num_textures = scan.get_int32();
  manager->read_pointers(scan, _num_textures);
}
