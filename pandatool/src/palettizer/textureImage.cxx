/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureImage.cxx
 * @author drose
 * @date 2000-11-29
 */

#include "textureImage.h"
#include "sourceTextureImage.h"
#include "destTextureImage.h"
#include "eggFile.h"
#include "paletteGroup.h"
#include "paletteImage.h"
#include "texturePlacement.h"
#include "filenameUnifier.h"
#include "string_utils.h"
#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pnmFileType.h"
#include "indirectCompareNames.h"
#include "pvector.h"

#include <iterator>

using std::string;

TypeHandle TextureImage::_type_handle;

/**
 *
 */
TextureImage::
TextureImage() {
  _preferred_source = nullptr;
  _read_source_image = false;
  _allow_release_source_image = true;
  _is_surprise = true;
  _ever_read_image = false;
  _forced_grayscale = false;
  _alpha_bits = 0;
  _mid_pixel_ratio = 0.0;
  _is_cutout = false;
  _alpha_mode = EggRenderMode::AM_unspecified;
  _txa_wrap_u = EggTexture::WM_unspecified;
  _txa_wrap_v = EggTexture::WM_unspecified;
  _texture_named = false;
  _got_txa_file = false;
}

/**
 * Records that a particular egg file references this texture.  This is
 * essential to know when deciding how to assign the TextureImage to the
 * various PaletteGroups.
 */
void TextureImage::
note_egg_file(EggFile *egg_file) {
  nassertv(!egg_file->get_complete_groups().empty());
  _egg_files.insert(egg_file);
}

/**
 * Assigns the texture to all of the PaletteGroups the various egg files that
 * use it need.  Attempts to choose the minimum set of PaletteGroups that
 * satisfies all of the egg files.
 */
void TextureImage::
assign_groups() {
  if (_egg_files.empty()) {
    // If we're not referenced by any egg files any more, assign us to no
    // groups.
    PaletteGroups empty;
    assign_to_groups(empty);
    return;
  }

  PaletteGroups definitely_in;

  // First, we need to eliminate from consideration all the egg files that are
  // already taken care of by the user's explicit group assignments for this
  // texture.
  WorkingEggs needed_eggs;

  if (_explicitly_assigned_groups.empty()) {
    // If we have no explicit group assignments, we must consider all the egg
    // files.
    std::copy(_egg_files.begin(), _egg_files.end(), std::back_inserter(needed_eggs));

  } else {
    // Otherwise, we only need to consider the egg files that don't have any
    // groups in common with our explicit assignments.

    EggFiles::const_iterator ei;
    for (ei = _egg_files.begin(); ei != _egg_files.end(); ++ei) {
      PaletteGroups intersect;
      intersect.make_intersection(_explicitly_assigned_groups, (*ei)->get_complete_groups());
      if (!intersect.empty()) {
        // This egg file is satisfied by one of the texture's explicit
        // assignments.

        // We must use at least one of the explicitly-assigned groups that
        // satisfied the egg file.  We don't need to use all of them, however,
        // and we choose the first one arbitrarily.
        definitely_in.insert(*intersect.begin());

      } else {
        // This egg file was not satisfied by any of the texture's explicit
        // assignments.  Therefore, we'll need to choose some additional group
        // to assign the texture to, to make the egg file happy.  Defer this a
        // bit.
        needed_eggs.push_back(*ei);
      }
    }
  }

  while (!needed_eggs.empty()) {
    // We need to know the complete set of groups that we need to consider
    // adding the texture to.  This is the union of all the egg files'
    // requested groups.
    PaletteGroups total;
    WorkingEggs::const_iterator ei;
    for (ei = needed_eggs.begin(); ei != needed_eggs.end(); ++ei) {
      total.make_union(total, (*ei)->get_complete_groups());
    }

    // We don't count the "null" group for texture assignment.
    total.remove_null();
    if (total.empty()) {
      break;
    }

    // Now, find the group that will satisfy the most egg files.  If two
    // groups satisfy the same number of egg files, choose (a) the most
    // specific one, i.e.  with the lowest dirname_level, or the lowest
    // dependency_level if the dirname_levels are equal, and (b) the one that
    // has the fewest egg files sharing it.
    PaletteGroups::iterator gi = total.begin();
    PaletteGroup *best = (*gi);
    int best_egg_count = compute_egg_count(best, needed_eggs);
    ++gi;
    while (gi != total.end()) {
      PaletteGroup *group = (*gi);

      // Do we prefer this group to our current 'best'?
      bool prefer_group = false;
      int group_egg_count = compute_egg_count(group, needed_eggs);
      if (group_egg_count != best_egg_count) {
        prefer_group = (group_egg_count > best_egg_count);

      } else {
        prefer_group = group->is_preferred_over(*best);
      }

      if (prefer_group) {
        best = group;
        best_egg_count = group_egg_count;
      }
      ++gi;
    }

    // Okay, now we've picked the best group.  Eliminate all the eggs from
    // consideration that are satisfied by this group, and repeat.
    definitely_in.insert(best);

    WorkingEggs next_needed_eggs;
    for (ei = needed_eggs.begin(); ei != needed_eggs.end(); ++ei) {
      if ((*ei)->get_complete_groups().count(best) == 0) {
        // This one wasn't eliminated.
        next_needed_eggs.push_back(*ei);
      }
    }
    needed_eggs.swap(next_needed_eggs);
  }

  // Finally, now that we've computed the set of groups we need to assign the
  // texture to, we need to reconcile this with the set of groups we've
  // assigned the texture to previously.
  assign_to_groups(definitely_in);
}

/**
 * Once assign_groups() has been called, this returns the actual set of groups
 * the TextureImage has been assigned to.
 */
const PaletteGroups &TextureImage::
get_groups() const {
  return _actual_assigned_groups;
}

/**
 * Gets the TexturePlacement object which represents the assignment of this
 * texture to the indicated group.  If the texture has not been assigned to
 * the indicated group, returns NULL.
 */
TexturePlacement *TextureImage::
get_placement(PaletteGroup *group) const {
  Placement::const_iterator pi;
  pi = _placement.find(group);
  if (pi == _placement.end()) {
    return nullptr;
  }

  return (*pi).second;
}

/**
 * Removes the texture from any PaletteImages it is assigned to, but does not
 * remove it from the groups.  It will be re-placed within each group when
 * PaletteGroup::place_all() is called.
 */
void TextureImage::
force_replace() {
  Placement::iterator pi;
  for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
    (*pi).second->force_replace();
  }
}

/**
 * Marks all the egg files that reference this texture stale.  Should be
 * called only when the texture properties change in some catastrophic way
 * that will require every egg file referencing it to be regenerated, even if
 * it is not palettized.
 */
void TextureImage::
mark_eggs_stale() {
  Placement::iterator pi;
  for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
    (*pi).second->mark_eggs_stale();
  }
}

/**
 * Indicates that this particular texture has been named by the user for
 * processing this session, normally by listing an egg file on the command
 * line that references it.
 */
void TextureImage::
mark_texture_named() {
  _texture_named = true;
}

/**
 * Returns true if this particular texture has been named by the user for
 * procession this session, for instance by listing an egg file on the command
 * line that references it.
 */
bool TextureImage::
is_texture_named() const {
  return _texture_named;
}

/**
 * Updates any internal state prior to reading the .txa file.
 */
void TextureImage::
pre_txa_file() {
  // Save our current properties, so we can note if they change.
  _pre_txa_properties = _properties;

  // Get our properties from the actual image for this texture.  It's possible
  // the .txa file will update them further.
  SourceTextureImage *source = get_preferred_source();
  if (source != nullptr) {
    _properties = source->get_properties();
  }

  _pre_txa_alpha_mode = _alpha_mode;
  _alpha_mode = EggRenderMode::AM_unspecified;

  _request.pre_txa_file();
  _is_surprise = true;
}

/**
 * Once the .txa file has been read and the TextureImage matched against it,
 * considers applying the requested size change.  Updates the TextureImage's
 * size with the size the texture ought to be, if this can be determined.
 */
void TextureImage::
post_txa_file() {
  _got_txa_file = true;

  // First, get the actual size of the texture.
  SourceTextureImage *source = get_preferred_source();
  if (source != nullptr) {
    if (source->get_size()) {
      _size_known = true;
      _x_size = source->get_x_size();
      _y_size = source->get_y_size();
      _properties.set_num_channels(source->get_num_channels());
    }
  }

  // Now update this with a particularly requested size.
  if (_request._got_size) {
    _size_known = true;
    _x_size = _request._x_size;
    _y_size = _request._y_size;
  }

  if (_txa_wrap_u != _request._wrap_u ||
      _txa_wrap_v != _request._wrap_v) {
    _txa_wrap_u = _request._wrap_u;
    _txa_wrap_v = _request._wrap_v;

    // If the explicit wrap mode changes, we may need to regenerate the egg
    // files, andor refill the palettes.
    mark_eggs_stale();

    Placement::iterator pi;
    for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
      TexturePlacement *placement = (*pi).second;
      placement->mark_unfilled();
    }
  }

  if (_properties.has_num_channels() && !_request._keep_format) {
    int num_channels = _properties.get_num_channels();
    // Examine the image to determine if we can downgrade the number of
    // channels, for instance from color to grayscale.
    if (num_channels == 3 || num_channels == 4) {
      consider_grayscale();
    }

    // Also consider the alpha properties, and whether we should downgrade
    // from alpha to non-alpha.
    if (num_channels == 2 || num_channels == 4) {
      consider_alpha();
    }
  }

  // However, if we got an explicit request for channels, honor that.
  if (_request._got_num_channels) {
    _properties.set_num_channels(_request._num_channels);
  }

  _properties._generic_format = _request._generic_format;
  _properties._keep_format = _request._keep_format;

  if (_request._format != EggTexture::F_unspecified) {
    _properties._format = _request._format;
    _properties._force_format = _request._force_format;
  }

  if (_request._minfilter != EggTexture::FT_unspecified) {
    _properties._minfilter = _request._minfilter;
  }
  if (_request._magfilter != EggTexture::FT_unspecified) {
    _properties._magfilter = _request._magfilter;
  }

  _properties._anisotropic_degree = _request._anisotropic_degree;

  if (_properties._color_type == nullptr) {
    _properties._color_type = _request._properties._color_type;
    _properties._alpha_type = _request._properties._alpha_type;
  }

  // Finally, make sure our properties are fully defined.
  _properties.fully_define();

  // Now, if our properties have changed in all that from our previous
  // session, we need to re-place ourself in all palette groups.
  if (_properties != _pre_txa_properties) {
    force_replace();

    // The above will mark the egg files stale when the texture is palettized
    // (since the UV's will certainly need to be recomputed), but sometimes we
    // need to mark the egg files stale even when the texture is not
    // palettized (if a critical property has changed).  The following
    // accomplishes this:
    if (!_properties.egg_properties_match(_pre_txa_properties)) {
      mark_eggs_stale();
    }
  }

  // The alpha mode isn't stored in the properties, because it doesn't affect
  // which textures may be associated into a common palette.
  if (_request._alpha_mode != EggRenderMode::AM_unspecified) {
    _alpha_mode = _request._alpha_mode;
  }

  // On the other hand, if we don't have an alpha channel, we shouldn't have
  // an alpha mode.
  if (_properties.has_num_channels()) {
    int num_channels = _properties.get_num_channels();
    if (num_channels == 1 || num_channels == 3) {
      _alpha_mode = EggRenderMode::AM_unspecified;
    }
  }

  // If we've changed the alpha mode, we should also mark the eggs stale.
  if (_pre_txa_alpha_mode != _alpha_mode) {
    mark_eggs_stale();
  }
}

/**
 * Returns true if this TextureImage has been looked up in the .txa file this
 * session, false otherwise.
 */
bool TextureImage::
got_txa_file() const {
  return _got_txa_file;
}

/**
 * Calls determine_size() on each TexturePlacement for the texture, to ensure
 * that each TexturePlacement is still requesting the best possible size for
 * the texture.
 */
void TextureImage::
determine_placement_size() {
  Placement::iterator pi;
  for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
    TexturePlacement *placement = (*pi).second;
    placement->determine_size();
  }
}

/**
 * Returns true if the user specifically requested to omit this texture via
 * the "omit" keyword in the .txa file, or false otherwise.
 */
bool TextureImage::
get_omit() const {
  return _request._omit;
}

/**
 * Returns the appropriate coverage threshold for this texture.  This is
 * either the Palettizer::_coverage_threshold parameter, given globally via
 * -r, or a particular value for this texture as supplied by the "coverage"
 * keyword in the .txa file.
 */
double TextureImage::
get_coverage_threshold() const {
  return _request._coverage_threshold;
}

/**
 * Returns the appropriate margin for this texture.  This is either the
 * Palettizer::_margin parameter, or a particular value for this texture as
 * supplied by the "margin" keyword in the .txa file.
 */
int TextureImage::
get_margin() const {
  return _request._margin;
}

/**
 * Returns true if this particular texture is a 'surprise', i.e.  it wasn't
 * matched by a line in the .txa file that didn't include the keyword 'cont'.
 */
bool TextureImage::
is_surprise() const {
  if (_placement.empty()) {
    // A texture that is not actually placed anywhere is not considered a
    // surprise.
    return false;
  }

  return _is_surprise;
}

/**
 * Returns true if this particular texture has been placed somewhere,
 * anywhere, or false if it is not used.
 */
bool TextureImage::
is_used() const {
  return !_placement.empty();
}

/**
 * Returns the alpha mode that should be used to render objects with this
 * texture, as specified by the user or as determined from examining the
 * texture's alpha channel.
 */
EggRenderMode::AlphaMode TextureImage::
get_alpha_mode() const {
  return _alpha_mode;
}

/**
 * Returns the wrap mode specified in the u direction in the txa file, or
 * WM_unspecified.
 */
EggTexture::WrapMode TextureImage::
get_txa_wrap_u() const {
  return _txa_wrap_u;
}

/**
 * Returns the wrap mode specified in the v direction in the txa file, or
 * WM_unspecified.
 */
EggTexture::WrapMode TextureImage::
get_txa_wrap_v() const {
  return _txa_wrap_v;
}


/**
 * Returns the SourceTextureImage corresponding to the given filename(s).  If
 * the given filename has never been used as a SourceTexture for this
 * particular texture, creates a new SourceTextureImage and returns that.
 */
SourceTextureImage *TextureImage::
get_source(const Filename &filename, const Filename &alpha_filename,
           int alpha_file_channel) {
  string key = get_source_key(filename, alpha_filename, alpha_file_channel);

  Sources::iterator si;
  si = _sources.find(key);
  if (si != _sources.end()) {
    return (*si).second;
  }

  SourceTextureImage *source =
    new SourceTextureImage(this, filename, alpha_filename, alpha_file_channel);
  _sources.insert(Sources::value_type(key, source));

  // Clear out the preferred source image to force us to rederive this next
  // time someone asks.
  _preferred_source = nullptr;
  _read_source_image = false;

  return source;
}

/**
 * Determines the preferred source image for examining size and reading
 * pixels, etc.  This is the largest and most recent of all the available
 * source images.
 */
SourceTextureImage *TextureImage::
get_preferred_source() {
  if (_preferred_source != nullptr) {
    return _preferred_source;
  }

  // Now examine all of the various source images available to us and pick the
  // most suitable.  We base this on the following criteria:

  // (1) A suitable source image must be referenced by at least one egg file,
  // unless no source images are referenced by any egg file.

  // (2) A larger source image is preferable to a smaller one.

  // (3) Given two source images of the same size, the more recent one is
  // preferable.

  // Are any source images referenced by an egg file?

  bool any_referenced = false;
  Sources::iterator si;
  for (si = _sources.begin(); si != _sources.end() && !any_referenced; ++si) {
    SourceTextureImage *source = (*si).second;
    if (source->get_egg_count() > 0) {
      any_referenced = true;
    }
  }

  SourceTextureImage *best = nullptr;
  int best_size = 0;

  for (si = _sources.begin(); si != _sources.end(); ++si) {
    SourceTextureImage *source = (*si).second;

    if (source->get_egg_count() > 0 || !any_referenced) {
      // Rule (1) passes.

      if (source->exists() && source->get_size()) {
        int source_size = source->get_x_size() * source->get_y_size();
        if (best == nullptr) {
          best = source;
          best_size = source_size;

        } else if (source_size > best_size) {
          // Rule (2) passes.
          best = source;
          best_size = source_size;

        } else if (source_size == best_size &&
                   source->get_filename().compare_timestamps(best->get_filename()) > 0) {
          // Rule (3) passes.
          best = source;
          best_size = source_size;
        }
      }
    }
  }

  if (best == nullptr && !_sources.empty()) {
    // If we didn't pick any that pass, it must be that all of them are
    // unreadable.  In this case, it really doesn't matter which one we pick,
    // but we should at least pick one that has an egg reference, if any of
    // them do.
    if (any_referenced) {
      for (si = _sources.begin();
           si != _sources.end() && best == nullptr;
           ++si) {
        SourceTextureImage *source = (*si).second;
        if (source->get_egg_count() > 0) {
          best = source;
        }
      }
    } else {
      best = (*_sources.begin()).second;
    }
  }

  _preferred_source = best;
  return _preferred_source;
}

/**
 * Calls clear_basic_properties() on each source texture image used by this
 * texture, to reset the properties in preparation for re-applying them from
 * the set of all known egg files.
 */
void TextureImage::
clear_source_basic_properties() {
  Sources::iterator si;
  for (si = _sources.begin(); si != _sources.end(); ++si) {
    SourceTextureImage *source = (*si).second;
    source->clear_basic_properties();
  }
}

/**
 * Copies the texture to whichever destination directories are appropriate for
 * the groups in which it has been unplaced.  Also removes the old filenames
 * for previous sessions where it was unplaced, but is no longer.
 *
 * If redo_all is true, this recopies the texture whether it needed to or not.
 */
void TextureImage::
copy_unplaced(bool redo_all) {
  // First, we need to build up the set of DestTextureImages that represents
  // the files we need to generate.
  Dests generate;

  // Go through all the TexturePlacements and note the ones for which we're
  // unplaced.  We check get_omit_reason() and not is_placed(), because we
  // want to consider solitary images to be unplaced in this case.
  Placement::iterator pi;
  for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
    TexturePlacement *placement = (*pi).second;
    if (placement->get_omit_reason() != OR_none &&
        placement->get_omit_reason() != OR_unknown) {
      DestTextureImage *dest = new DestTextureImage(placement);
      Filename filename = dest->get_filename();
      FilenameUnifier::make_canonical(filename);

      std::pair<Dests::iterator, bool> insert_result = generate.insert
        (Dests::value_type(filename, dest));
      if (!insert_result.second) {
        // At least two DestTextureImages map to the same filename, no sweat.
        delete dest;
        dest = (*insert_result.first).second;
      }

      placement->set_dest(dest);

    } else {
      placement->set_dest(nullptr);
    }
  }

  if (redo_all) {
    // If we're redoing everything, we remove everything first and then recopy
    // it again.
    Dests empty;
    remove_old_dests(empty, _dests);
    copy_new_dests(generate, empty);

  } else {
    // Otherwise, we only remove and recopy the things that changed between
    // this time and last time.
    remove_old_dests(generate, _dests);
    copy_new_dests(generate, _dests);
  }

  // Clean up the old set.
  Dests::iterator di;
  for (di = _dests.begin(); di != _dests.end(); ++di) {
    delete (*di).second;
  }

  _dests.swap(generate);
}

/**
 * Reads in the original image, if it has not already been read, and returns
 * it.
 */
const PNMImage &TextureImage::
read_source_image() {
  if (!_read_source_image) {
    SourceTextureImage *source = get_preferred_source();
    if (source != nullptr) {
      source->read(_source_image);
    }
    _read_source_image = true;
    _allow_release_source_image = true;
    _ever_read_image = true;
  }

  return _source_image;
}

/**
 * Frees the memory that was allocated by a previous call to
 * read_source_image().  The next time read_source_image() is called, it will
 * have to read the disk again.
 */
void TextureImage::
release_source_image() {
  if (_read_source_image && _allow_release_source_image) {
    _source_image.clear();
    _read_source_image = false;
  }
}

/**
 * Accepts the indicated source image as if it had been read from disk.  This
 * image is copied into the structure, and will be returned by future calls to
 * read_source_image().
 */
void TextureImage::
set_source_image(const PNMImage &image) {
  _source_image = image;
  _allow_release_source_image = false;
  _read_source_image = true;
  _ever_read_image = true;
}

/**
 * Causes the header part of the image to be reread, usually to confirm that
 * its image properties (size, number of channels, etc.) haven't changed.
 */
void TextureImage::
read_header() {
  if (!_read_source_image) {
    SourceTextureImage *source = get_preferred_source();
    if (source != nullptr) {
      source->read_header();
    }
  }
}

/**
 * Returns true if the source image is newer than the indicated file, false
 * otherwise.  If the image has already been read, this always returns false.
 */
bool TextureImage::
is_newer_than(const Filename &reference_filename) {
  if (!_read_source_image) {
    SourceTextureImage *source = get_preferred_source();
    if (source != nullptr) {
      const Filename &source_filename = source->get_filename();
      return source_filename.compare_timestamps(reference_filename) >= 0;
    }
  }

  return false;
}

/**
 * Writes the list of source pathnames that might contribute to this texture
 * to the indicated output stream, one per line.
 */
void TextureImage::
write_source_pathnames(std::ostream &out, int indent_level) const {
  Sources::const_iterator si;
  for (si = _sources.begin(); si != _sources.end(); ++si) {
    SourceTextureImage *source = (*si).second;

    if (source->get_egg_count() > 0) {
      indent(out, indent_level);
      source->output_filename(out);
      if (!source->is_size_known()) {
        out << " (unknown size)";

      } else {
        out << " " << source->get_x_size() << " "
            << source->get_y_size();

        if (source->get_properties().has_num_channels()) {
          out << " " << source->get_properties().get_num_channels();
        }
      }
      out << "\n";
    }
  }

  if (_is_cutout) {
    indent(out, indent_level)
      << "Cutout image (ratio " << (PN_stdfloat)_mid_pixel_ratio << ")\n";
  }

  // Now write out the group assignments.
  if (!_egg_files.empty()) {
    // Sort the egg files into order by name for output.
    pvector<EggFile *> egg_vector;
    egg_vector.reserve(_egg_files.size());
    EggFiles::const_iterator ei;
    for (ei = _egg_files.begin(); ei != _egg_files.end(); ++ei) {
      egg_vector.push_back(*ei);
    }
    sort(egg_vector.begin(), egg_vector.end(),
         IndirectCompareNames<EggFile>());

    indent(out, indent_level)
      << "Used by:\n";
    pvector<EggFile *>::const_iterator evi;
    for (evi = egg_vector.begin(); evi != egg_vector.end(); ++evi) {
      EggFile *egg = (*evi);
      indent(out, indent_level + 2)
        << egg->get_name() << " (";
      if (egg->get_explicit_groups().empty()) {
        out << *egg->get_default_group();
      } else {
        out << egg->get_explicit_groups();
      }
      out << ")\n";
    }
  }
  if (!_explicitly_assigned_groups.empty()) {
    indent(out, indent_level)
      << "Explicitly assigned to " << _explicitly_assigned_groups << " in .txa\n";
  }

  if (_placement.empty()) {
    indent(out, indent_level)
      << "Not used.\n";
  } else {
    indent(out, indent_level)
      << "Assigned to " << _actual_assigned_groups << "\n";
  }
}

/**
 * Writes the information about the texture's size and placement.
 */
void TextureImage::
write_scale_info(std::ostream &out, int indent_level) {
  SourceTextureImage *source = get_preferred_source();
  indent(out, indent_level) << get_name();

  // Write the list of groups we're placed in.
  if (_placement.empty()) {
    out << " (not used)";
  } else {
    Placement::const_iterator pi;
    pi = _placement.begin();
    out << " (" << (*pi).second->get_group()->get_name();
    ++pi;
    while (pi != _placement.end()) {
      out << " " << (*pi).second->get_group()->get_name();
      ++pi;
    }
    out << ")";
  }

  out << " orig ";

  if (source == nullptr ||
      !source->is_size_known()) {
    out << "unknown";
  } else {
    out << source->get_x_size() << " " << source->get_y_size()
        << " " << source->get_num_channels();
  }

  if (!_placement.empty() && is_size_known()) {
    out << " new " << get_x_size() << " " << get_y_size()
        << " " << get_num_channels();

    if (source != nullptr &&
        source->is_size_known()) {
      double scale =
        100.0 * (((double)get_x_size() / (double)source->get_x_size()) +
                 ((double)get_y_size() / (double)source->get_y_size())) / 2.0;
      out << " scale " << scale << "%";
    }
  }
  out << "\n";

  // Also cross-reference the placed and unplaced information.
  Placement::iterator pi;
  for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
    TexturePlacement *placement = (*pi).second;
    if (placement->get_omit_reason() == OR_none) {
      PaletteImage *image = placement->get_image();
      nassertv(image != nullptr);
      indent(out, indent_level + 2)
        << "placed on "
        << FilenameUnifier::make_user_filename(image->get_filename())
        << "\n";

    } else if (placement->get_omit_reason() == OR_unknown) {
      indent(out, indent_level + 2)
        << "not placed because unknown.\n";

    } else {
      DestTextureImage *image = placement->get_dest();
      nassertv(image != nullptr);
      indent(out, indent_level + 2)
        << "copied to "
        << FilenameUnifier::make_user_filename(image->get_filename());
      if (image->is_size_known() && is_size_known() &&
          (image->get_x_size() != get_x_size() ||
           image->get_y_size() != get_y_size())) {
        out << " at size " << image->get_x_size() << " "
            << image->get_y_size();
        if (source != nullptr &&
            source->is_size_known()) {
          double scale =
            100.0 * (((double)image->get_x_size() / (double)source->get_x_size()) +
                     ((double)image->get_y_size() / (double)source->get_y_size())) / 2.0;
          out << " scale " << scale << "%";
        }
      }
      out << "\n";
    }
  }
}

/**
 * Counts the number of egg files in the indicated set that will be satisfied
 * if a texture is assigned to the indicated group.
 */
int TextureImage::
compute_egg_count(PaletteGroup *group,
                  const TextureImage::WorkingEggs &egg_files) {
  int count = 0;

  WorkingEggs::const_iterator ei;
  for (ei = egg_files.begin(); ei != egg_files.end(); ++ei) {
    if ((*ei)->get_complete_groups().count(group) != 0) {
      count++;
    }
  }

  return count;
}

/**
 * Assigns the texture to the indicated set of groups.  If the texture was
 * previously assigned to any of these groups, keeps the same TexturePlacement
 * object for the assignment; at the same time, deletes any TexturePlacement
 * objects that represent groups we are no longer assigned to.
 */
void TextureImage::
assign_to_groups(const PaletteGroups &groups) {
  PaletteGroups::const_iterator gi;
  Placement::const_iterator pi;

  Placement new_placement;

  gi = groups.begin();
  pi = _placement.begin();

  while (gi != groups.end() && pi != _placement.end()) {
    PaletteGroup *a = (*gi);
    PaletteGroup *b = (*pi).first;

    if (a < b) {
      // Here's a group we're now assigned to that we weren't assigned to
      // previously.
      TexturePlacement *place = a->prepare(this);
      new_placement.insert
        (new_placement.end(), Placement::value_type(a, place));
      ++gi;

    } else if (b < a) {
      // Here's a group we're no longer assigned to.
      TexturePlacement *place = (*pi).second;
      delete place;
      ++pi;

    } else { // b == a
      // Here's a group we're still assigned to.
      TexturePlacement *place = (*pi).second;
      new_placement.insert
        (new_placement.end(), Placement::value_type(a, place));
      ++gi;
      ++pi;
    }
  }

  while (gi != groups.end()) {
    // Here's a group we're now assigned to that we weren't assigned to
    // previously.
    PaletteGroup *a = (*gi);
    TexturePlacement *place = a->prepare(this);
    new_placement.insert
      (new_placement.end(), Placement::value_type(a, place));
    ++gi;
  }

  while (pi != _placement.end()) {
    // Here's a group we're no longer assigned to.
    TexturePlacement *place = (*pi).second;
    delete place;
    ++pi;
  }

  _placement.swap(new_placement);
  _actual_assigned_groups = groups;
}

/**
 * Examines the actual contents of the image to determine if it should maybe
 * be considered a grayscale image (even though it has separate rgb
 * components).
 */
void TextureImage::
consider_grayscale() {
  // Since this isn't likely to change for a particular texture after its
  // creation, we save a bit of time by not performing this check unless this
  // is the first time we've ever seen this texture.  This will save us from
  // having to load the texture images each time we look at them.  On the
  // other hand, if we've already loaded up the image, then go ahead.
  if (!_read_source_image && _ever_read_image) {
    if (_forced_grayscale) {
      _properties.force_grayscale();
    }
    return;
  }

  const PNMImage &source = read_source_image();
  if (!source.is_valid()) {
    return;
  }

  for (int y = 0; y < source.get_y_size(); y++) {
    for (int x = 0; x < source.get_x_size(); x++) {
      const xel &v = source.get_xel_val(x, y);
      if (PPM_GETR(v) != PPM_GETG(v) || PPM_GETR(v) != PPM_GETB(v)) {
        // Here's a colored pixel.  We can't go grayscale.
        _forced_grayscale = false;
        return;
      }
    }
  }

  // All pixels in the image were grayscale!
  _properties.force_grayscale();
  _forced_grayscale = true;
}

/**
 * Examines the actual contents of the image to determine what alpha
 * properties it has.
 */
void TextureImage::
consider_alpha() {
  // As above, we don't bother doing this if we've already done this in a
  // previous session.

  // _alpha_bits == -1 indicates we have read an older textures.boo file that
  // didn't define these bits.
  if (_read_source_image || !_ever_read_image || _alpha_bits == -1) {
    _alpha_bits = 0;
    int num_mid_pixels = 0;

    const PNMImage &source = read_source_image();
    if (source.is_valid() && source.has_alpha()) {
      xelval maxval = source.get_maxval();
      for (int y = 0; y < source.get_y_size(); y++) {
        for (int x = 0; x < source.get_x_size(); x++) {
          xelval alpha_val = source.get_alpha_val(x, y);
          if (alpha_val == 0) {
            _alpha_bits |= AB_zero;
          } else if (alpha_val == maxval) {
            _alpha_bits |= AB_one;
          } else {
            _alpha_bits |= AB_mid;
            ++num_mid_pixels;
          }
        }
      }
    }

    int num_pixels = source.get_x_size() * source.get_y_size();
    _mid_pixel_ratio = 0.0;
    if (num_pixels != 0) {
      _mid_pixel_ratio = (double)num_mid_pixels / (double)num_pixels;
    }
  }

  _is_cutout = false;

  if (_alpha_bits != 0) {
    if (_alpha_bits == AB_one) {
      // All alpha pixels are white; drop the alpha channel.
      _properties.force_nonalpha();

    } else if (_alpha_bits == AB_zero) {
      // All alpha pixels are invisible; this is probably a mistake.  Drop the
      // alpha channel and complain.
      _properties.force_nonalpha();
      if (_read_source_image) {
        nout << *this << " has an all-zero alpha channel; dropping alpha.\n";
      }

    } else if (_alpha_mode == EggRenderMode::AM_unspecified) {
      // Consider fiddling with the alpha mode, if the user hasn't specified a
      // particular alpha mode in the txa file.
      if ((_alpha_bits & AB_mid) == 0) {
        // No middle range bits: a binary alpha image.
        _alpha_mode = EggRenderMode::AM_binary;

      } else if ((_alpha_bits & AB_one) != 0 && _mid_pixel_ratio < pal->_cutout_ratio) {
        // At least some opaque bits, and relatively few middle range bits: a
        // cutout image.
        _alpha_mode = pal->_cutout_mode;
        _is_cutout = true;

      } else {
        // No opaque bits; just use regular alpha blending.
        _alpha_mode = EggRenderMode::AM_blend;
      }
    }
  }
}

/**
 * Removes all of the filenames named in b that are not also named in a.
 */
void TextureImage::
remove_old_dests(const TextureImage::Dests &a, const TextureImage::Dests &b) {
  Dests::const_iterator ai = a.begin();
  Dests::const_iterator bi = b.begin();

  while (ai != a.end() && bi != b.end()) {
    const string &astr = (*ai).first;
    const string &bstr = (*bi).first;

    if (astr < bstr) {
      // Here's a filename in a, not in b.
      ++ai;

    } else if (bstr < astr) {
      // Here's a filename in b, not in a.
      (*bi).second->unlink();
      ++bi;

    } else { // bstr == astr
      // Here's a filename in both a and b.
      ++ai;
      ++bi;
    }
  }

  while (bi != b.end()) {
    // Here's a filename in b, not in a.
    (*bi).second->unlink();
    ++bi;
  }

  while (ai != a.end()) {
    ++ai;
  }
}

/**
 * Copies a resized texture into each filename named in a that is not also
 * listed in b, or whose corresponding listing in b is out of date.
 */
void TextureImage::
copy_new_dests(const TextureImage::Dests &a, const TextureImage::Dests &b) {
  Dests::const_iterator ai = a.begin();
  Dests::const_iterator bi = b.begin();

  while (ai != a.end() && bi != b.end()) {
    const string &astr = (*ai).first;
    const string &bstr = (*bi).first;

    if (astr < bstr) {
      // Here's a filename in a, not in b.
      (*ai).second->copy(this);
      ++ai;

    } else if (bstr < astr) {
      // Here's a filename in b, not in a.
      ++bi;

    } else { // bstr == astr
      // Here's a filename in both a and b.
      (*ai).second->copy_if_stale((*bi).second, this);
      ++ai;
      ++bi;
    }
  }

  while (ai != a.end()) {
    // Here's a filename in a, not in b.
    (*ai).second->copy(this);
    ++ai;
  }
}

/**
 * Returns the key that a SourceTextureImage should be stored in, given its
 * one or two filenames.
 */
string TextureImage::
get_source_key(const Filename &filename, const Filename &alpha_filename,
               int alpha_file_channel) {
  Filename f = FilenameUnifier::make_bam_filename(filename);
  Filename a = FilenameUnifier::make_bam_filename(alpha_filename);

  return f.get_fullpath() + ":" + a.get_fullpath() + ":" +
    format_string(alpha_file_channel);
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void TextureImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_TextureImage);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void TextureImage::
write_datagram(BamWriter *writer, Datagram &datagram) {
  ImageFile::write_datagram(writer, datagram);
  datagram.add_string(get_name());

  // We don't write out _request; this is re-read from the .txa file each
  // time.

  // We don't write out _pre_txa_properties; this is transitional.

  // We don't write out _preferred_source; this is redetermined each session.

  datagram.add_bool(_is_surprise);
  datagram.add_bool(_ever_read_image);
  datagram.add_bool(_forced_grayscale);
  datagram.add_uint8(_alpha_bits);
  datagram.add_int16((int)_alpha_mode);
  datagram.add_float64(_mid_pixel_ratio);
  datagram.add_bool(_is_cutout);
  datagram.add_uint8((int)_txa_wrap_u);
  datagram.add_uint8((int)_txa_wrap_v);

  // We don't write out _explicitly_assigned_groups; this is re-read from the
  // .txa file each time.

  _actual_assigned_groups.write_datagram(writer, datagram);

  // We don't write out _egg_files; this is redetermined each session.

  datagram.add_uint32(_placement.size());
  Placement::const_iterator pi;
  for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
    writer->write_pointer(datagram, (*pi).first);
    writer->write_pointer(datagram, (*pi).second);
  }

  datagram.add_uint32(_sources.size());
  Sources::const_iterator si;
  for (si = _sources.begin(); si != _sources.end(); ++si) {
    writer->write_pointer(datagram, (*si).second);
  }

  datagram.add_uint32(_dests.size());
  Dests::const_iterator di;
  for (di = _dests.begin(); di != _dests.end(); ++di) {
    writer->write_pointer(datagram, (*di).second);
  }
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int TextureImage::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ImageFile::complete_pointers(p_list, manager);

  pi += _actual_assigned_groups.complete_pointers(p_list + pi, manager);

  int i;
  for (i = 0; i < _num_placement; i++) {
    PaletteGroup *group;
    TexturePlacement *placement;
    DCAST_INTO_R(group, p_list[pi++], pi);
    DCAST_INTO_R(placement, p_list[pi++], pi);
    _placement.insert(Placement::value_type(group, placement));
  }

  for (i = 0; i < _num_sources; i++) {
    SourceTextureImage *source;
    DCAST_INTO_R(source, p_list[pi++], pi);
    string key = get_source_key(source->get_filename(),
                                source->get_alpha_filename(),
                                source->get_alpha_file_channel());

    bool inserted = _sources.insert(Sources::value_type(key, source)).second;
    if (!inserted) {
      nout << "Warning: texture key " << key
           << " is nonunique; texture lost.\n";
    }
  }

  for (i = 0; i < _num_dests; i++) {
    DestTextureImage *dest;
    DCAST_INTO_R(dest, p_list[pi++], pi);
    bool inserted = _dests.insert(Dests::value_type(dest->get_filename(), dest)).second;
    if (!inserted) {
      nout << "Warning: dest filename " << dest->get_filename()
           << " is nonunique; texture lost.\n";
    }
  }

  return pi;
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable *TextureImage::
make_TextureImage(const FactoryParams &params) {
  TextureImage *me = new TextureImage;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Reads the binary data from the given datagram iterator, which was written
 * by a previous call to write_datagram().
 */
void TextureImage::
fillin(DatagramIterator &scan, BamReader *manager) {
  ImageFile::fillin(scan, manager);
  set_name(scan.get_string());

  _is_surprise = scan.get_bool();
  _ever_read_image = scan.get_bool();
  _forced_grayscale = scan.get_bool();
  _alpha_bits = scan.get_uint8();
  _alpha_mode = (EggRenderMode::AlphaMode)scan.get_int16();
  if (pal->_read_pi_version >= 16) {
    _mid_pixel_ratio = scan.get_float64();
    _is_cutout = scan.get_bool();
  } else {
    // Force a re-read of the image if we are upgrading to pi version 16.
    _ever_read_image = false;
    _mid_pixel_ratio = 0.0;
    _is_cutout = false;
  }
  if (pal->_read_pi_version >= 17) {
    _txa_wrap_u = (EggTexture::WrapMode)scan.get_uint8();
    _txa_wrap_v = (EggTexture::WrapMode)scan.get_uint8();
  }

  _actual_assigned_groups.fillin(scan, manager);

  _num_placement = scan.get_uint32();
  manager->read_pointers(scan, _num_placement * 2);

  _num_sources = scan.get_uint32();
  manager->read_pointers(scan, _num_sources);
  _num_dests = scan.get_uint32();
  manager->read_pointers(scan, _num_dests);
}
