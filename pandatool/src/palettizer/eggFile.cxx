/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggFile.cxx
 * @author drose
 * @date 2000-11-29
 */

#include "eggFile.h"
#include "textureImage.h"
#include "paletteGroup.h"
#include "texturePlacement.h"
#include "textureReference.h"
#include "sourceTextureImage.h"
#include "palettizer.h"
#include "filenameUnifier.h"

#include "eggData.h"
#include "eggGroup.h"
#include "eggTextureCollection.h"
#include "eggComment.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "executionEnvironment.h"
#include "dSearchPath.h"
#include "indirectLess.h"

#include <algorithm>

TypeHandle EggFile::_type_handle;

/**
 *
 */
EggFile::
EggFile() {
  _data = nullptr;
  _first_txa_match = false;
  _default_group = nullptr;
  _is_surprise = true;
  _is_stale = true;
  _had_data = false;
}

/**
 * Accepts the information about the egg file as supplied from the command
 * line.  Returns true if the egg file is valid, false otherwise.
 */
bool EggFile::
from_command_line(EggData *data,
                  const Filename &source_filename,
                  const Filename &dest_filename,
                  const std::string &egg_comment) {
  _data = data;
  _had_data = true;
  remove_backstage(_data);

  // We save the current directory at the time the egg file appeared on the
  // command line, so that we'll later be able to properly resolve external
  // references (like textures) that might be relative to this directory.
  _current_directory = ExecutionEnvironment::get_cwd();
  _source_filename = source_filename;
  _source_filename.make_absolute();
  _dest_filename = dest_filename;
  _dest_filename.make_absolute();

  // We also save the command line that loaded this egg file, so we can
  // continue to write it as a comment to the beginning of the egg file,
  // should we need to rewrite it later.
  _egg_comment = egg_comment;

  // We save the default PaletteGroup at this point, because the egg file
  // inherits the default group that was in effect when it was specified on
  // the command line.
  _default_group = pal->get_default_group();

  return true;
}

/**
 * Returns the filename this egg file was read from.
 */
const Filename &EggFile::
get_source_filename() const {
  return _source_filename;
}


/**
 * Scans the egg file for texture references and updates the _textures list
 * appropriately.  This assumes the egg file was supplied on the command line
 * and thus the _data member is available.
 */
void EggFile::
scan_textures() {
  nassertv(_data != nullptr);

  // Extract the set of textures referenced by this egg file.
  EggTextureCollection tc;
  tc.find_used_textures(_data);

  // Make sure each tref name is unique within a given file.
  tc.uniquify_trefs();

  // Now build up a list of new TextureReference objects that represent the
  // textures actually used and their uv range, etc.
  Textures new_textures;

  EggTextureCollection::iterator eti;
  for (eti = tc.begin(); eti != tc.end(); ++eti) {
    EggTexture *egg_tex = (*eti);

    TextureReference *ref = new TextureReference;
    ref->from_egg(this, _data, egg_tex);

    if (!ref->has_uvs()) {
      // This texture isn't *really* referenced.  (Usually this happens if the
      // texture is only referenced by "backstage" geometry, which we don't
      // care about.)
      delete ref;

    } else {
      new_textures.push_back(ref);
    }
  }

  // Sort the new references into order so we can compare them with the
  // original references.
  sort(new_textures.begin(), new_textures.end(),
       IndirectLess<TextureReference>());

  // Sort the original references too.  This should already be sorted from the
  // previous run, but we might as well be neurotic about it.
  sort(_textures.begin(), _textures.end(),
       IndirectLess<TextureReference>());

  // Now go through and merge the lists.
  Textures combined_textures;
  Textures::const_iterator ai = _textures.begin();
  Textures::const_iterator bi = new_textures.begin();

  while (ai != _textures.end() && bi != new_textures.end()) {
    TextureReference *aref = (*ai);
    TextureReference *bref = (*bi);

    if ((*aref) < (*bref)) {
      // Here's a texture reference in the original list, but not in the new
      // list.  Remove it.
      delete aref;
      ++ai;

    } else if ((*bref) < (*aref)) {
      // Here's a texture reference in the new list, but not in the original
      // list.  Add it.
      combined_textures.push_back(bref);
      ++bi;

    } else { // (*bref) == (*aref)
      // Here's a texture reference that was in both lists.  Compare it.
      if (aref->is_equivalent(*bref)) {
        // It hasn't changed substantially, so keep the original (which still
        // has the placement references from a previous pass).
        aref->from_egg_quick(*bref);
        combined_textures.push_back(aref);
        delete bref;

      } else {
        // It has changed, so drop the original and keep the new one.
        combined_textures.push_back(bref);
        delete aref;
      }
      ++ai;
      ++bi;
    }
  }

  while (bi != new_textures.end()) {
    TextureReference *bref = (*bi);
    // Here's a texture reference in the new list, but not in the original
    // list.  Add it.
    combined_textures.push_back(bref);
    ++bi;
  }

  while (ai != _textures.end()) {
    TextureReference *aref = (*ai);
    // Here's a texture reference in the original list, but not in the new
    // list.  Remove it.
    delete aref;
    ++ai;
  }

  _textures.swap(combined_textures);
}

/**
 * Fills up the indicated set with the set of textures referenced by this egg
 * file.  It is the user's responsibility to ensure the set is empty before
 * making this call; otherwise, the new textures will be appended to the
 * existing set.
 */
void EggFile::
get_textures(pset<TextureImage *> &result) const {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    result.insert((*ti)->get_texture());
  }
}

/**
 * Does some processing prior to scanning the .txa file.
 */
void EggFile::
pre_txa_file() {
  _is_surprise = true;
  _first_txa_match = true;
}

/**
 * Adds the indicated set of groups, read from the .txa file, to the set of
 * groups to which the egg file is assigned.
 */
void EggFile::
match_txa_groups(const PaletteGroups &groups) {
  if (_first_txa_match) {
    // If this is the first line we matched in the .txa file, clear the set of
    // groups we'd matched from before.  We don't clear until we match a line
    // in the .txa file, because if we don't match any lines we still want to
    // remember what groups we used to be assigned to.
    _explicitly_assigned_groups.clear();
    _first_txa_match = false;
  }

  _explicitly_assigned_groups.make_union(_explicitly_assigned_groups, groups);
}

/**
 * Once the egg file has been matched against all of the matching lines the
 * .txa file, do whatever adjustment is necessary.
 */
void EggFile::
post_txa_file() {
}

/**
 * Returns the set of PaletteGroups that the egg file has been explicitly
 * assigned to in the .txa file.
 */
const PaletteGroups &EggFile::
get_explicit_groups() const {
  return _explicitly_assigned_groups;
}

/**
 * Returns the PaletteGroup that was specified as the default group on the
 * command line at the time the egg file last appeared on the command line.
 */
PaletteGroup *EggFile::
get_default_group() const {
  return _default_group;
}

/**
 * Returns the complete set of PaletteGroups that the egg file is assigned to.
 * This is the set of all the groups it is explicitly assigned to, plus all
 * the groups that these groups depend on.
 */
const PaletteGroups &EggFile::
get_complete_groups() const {
  return _complete_groups;
}

/**
 * Removes the 'surprise' flag; this file has been successfully matched
 * against a line in the .txa file.
 */
void EggFile::
clear_surprise() {
  _is_surprise = false;
}

/**
 * Returns true if this particular egg file is a 'surprise', i.e.  it wasn't
 * matched by a line in the .txa file that didn't include the keyword 'cont'.
 */
bool EggFile::
is_surprise() const {
  return _is_surprise;
}

/**
 * Marks this particular egg file as stale, meaning that something has
 * changed, such as the location of a texture within its palette, which causes
 * the egg file to need to be regenerated.
 */
void EggFile::
mark_stale() {
  _is_stale = true;
}

/**
 * Returns true if the egg file needs to be updated, i.e.  some palettizations
 * have changed affecting it, or false otherwise.
 */
bool EggFile::
is_stale() const {
  return _is_stale;
}

/**
 * Calls TextureImage::note_egg_file() and
 * SourceTextureImage::increment_egg_count() for each texture the egg file
 * references, and PaletteGroup::increment_egg_count() for each palette group
 * it wants.  This sets up some of the back references to support determining
 * an ideal texture assignment.
 */
void EggFile::
build_cross_links() {
  if (_explicitly_assigned_groups.empty()) {
    // If the egg file has been assigned to no groups, we have to assign it to
    // something.
    _complete_groups.clear();
    _complete_groups.insert(_default_group);
    _complete_groups.make_complete(_complete_groups);

  } else {
    _complete_groups.make_complete(_explicitly_assigned_groups);
  }

  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    TextureImage *texture = reference->get_texture();
    nassertv(texture != nullptr);
    texture->note_egg_file(this);

    // Actually, this may count the same egg file multiple times for a
    // particular SourceTextureImage, since a given texture may be referenced
    // multiples times within an egg file.  No harm done, however.
    reference->get_source()->increment_egg_count();
  }

  PaletteGroups::const_iterator gi;
  for (gi = _complete_groups.begin();
       gi != _complete_groups.end();
       ++gi) {
    (*gi)->increment_egg_count();
  }
}

/**
 * Calls apply_properties_to_source() for each texture reference, updating all
 * the referenced source textures with the complete set of property
 * information from this egg file.
 */
void EggFile::
apply_properties_to_source() {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    reference->apply_properties_to_source();
  }
}

/**
 * Once all the textures have been assigned to groups (but before they may
 * actually be placed), chooses a suitable TexturePlacement for each texture
 * that appears in the egg file.  This will be necessary to do at some point
 * before writing out the egg file anyway, and doing it before the textures
 * are placed allows us to decide what the necessary UV range is for each to-
 * be-placed texture.
 */
void EggFile::
choose_placements() {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    TextureImage *texture = reference->get_texture();

    if (reference->get_placement() != nullptr &&
        texture->get_groups().count(reference->get_placement()->get_group()) != 0) {
      // The egg file is already using a TexturePlacement that is suitable.
      // Don't bother changing it.

    } else {
      // We need to select a new TexturePlacement.
      PaletteGroups groups;
      groups.make_intersection(get_complete_groups(), texture->get_groups());

      // Now groups is the set of groups that the egg file requires, which
      // also happen to include the texture.

      if (groups.empty()) {
        // It might be empty if the egg file was assigned only to the "null"
        // group (since this group is not propagated to the textures).  In
        // this case, choose from the wider set of groups available to the
        // texture.
        groups = texture->get_groups();
      }

      if (!groups.empty()) {
        // It doesn't really matter which group in the set we choose, so we
        // arbitrarily choose the first one.
        PaletteGroup *group = (*groups.begin());

        // Now get the TexturePlacement object that corresponds to the
        // placement of this texture into this group.
        TexturePlacement *placement = texture->get_placement(group);
        nassertv(placement != nullptr);

        reference->set_placement(placement);
      }
    }
  }
}

/**
 * Returns true if the EggData for this EggFile has been loaded, and not yet
 * released.
 */
bool EggFile::
has_data() const {
  return (_data != nullptr);
}

/**
 * Returns true if the EggData for this EggFile has ever been loaded in this
 * session.
 */
bool EggFile::
had_data() const {
  return _had_data;
}

/**
 * Once all textures have been placed appropriately, updates the egg file with
 * all the information to reference the new textures.
 */
void EggFile::
update_egg() {
  nassertv(_data != nullptr);

  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    reference->update_egg();
  }
}

/**
 * Removes this egg file from all things that reference it, in preparation for
 * removing it from the database.
 */
void EggFile::
remove_egg() {
  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    TexturePlacement *placement = reference->get_placement();
    placement->remove_egg(reference);
  }
}

/**
 * Reads in the egg file from its _source_filename.  It is only valid to call
 * this if it has not already been read in, e.g.  from the command line.
 * Returns true if successful, false if there is an error.
 *
 * This may also be called after a previous call to release_egg_data(), in
 * order to re-read the same egg file.
 */
bool EggFile::
read_egg(bool noabs) {
  nassertr(_data == nullptr, false);
  nassertr(!_source_filename.empty(), false);

  Filename user_source_filename =
    FilenameUnifier::make_user_filename(_source_filename);

  if (!_source_filename.exists()) {
    nout << user_source_filename << " does not exist.\n";
    return false;
  }

  PT(EggData) data = new EggData;
  if (!data->read(_source_filename, user_source_filename)) {
    // Failure reading.
    return false;
  }

  if (noabs && data->original_had_absolute_pathnames()) {
    nout << _source_filename.get_basename()
         << " references textures using absolute pathnames!\n";
    return false;
  }

  // Extract the set of textures referenced by this egg file.
  EggTextureCollection tc;
  tc.find_used_textures(data);

  // Make sure each tref name is unique within a given file.
  tc.uniquify_trefs();

  // Now build up a list of new TextureReference objects that represent the
  // textures actually used and their uv range, etc.
  Textures new_textures;

  // We want to search for filenames based on the egg directory, and also on
  // our current directory from which we originally loaded the egg file.  This
  // is important because it's possible the egg file referenced some textures
  // or something relative to that directory.
  DSearchPath dir;
  dir.append_directory(_source_filename.get_dirname());
  dir.append_directory(_current_directory);
  data->resolve_filenames(dir);

  // If any relative filenames remain, they are relative to the source
  // directory, by convention.
  data->force_filenames(_current_directory);

  if (!data->load_externals()) {
    // Failure reading an external.
    return false;
  }

  _data = data;
  _had_data = true;
  remove_backstage(_data);

  // Insert a comment that shows how we first generated the egg file.
  PT(EggNode) comment = new EggComment("", _egg_comment);
  _data->insert(_data->begin(), comment);

  if (!_textures.empty()) {
    // If we already have textures, assume we're re-reading the file.
    rescan_textures();
  }

  return true;
}

/**
 * Releases the memory that was loaded by a previous call to read_egg().
 */
void EggFile::
release_egg_data() {
  if (_data != nullptr) {
    _data = nullptr;
  }
  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    reference->release_egg_data();
  }
}

/**
 * Writes out the egg file to its _dest_filename.  Returns true if successful,
 * false if there is an error.
 */
bool EggFile::
write_egg() {
  nassertr(_data != nullptr, false);
  nassertr(!_dest_filename.empty(), false);

  _dest_filename.make_dir();
  nout << "Writing " << FilenameUnifier::make_user_filename(_dest_filename)
       << "\n";
  if (!_data->write_egg(_dest_filename)) {
    // Some error while writing.  Most unusual.
    _is_stale = true;
    return false;
  }

  _is_stale = false;
  return true;
}

/**
 * Writes a one-line description of the egg file and its group assignments to
 * the indicated output stream.
 */
void EggFile::
write_description(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << get_name() << ": ";
  if (_explicitly_assigned_groups.empty()) {
    if (_default_group != nullptr) {
      out << _default_group->get_name();
    }
  } else {
    out << _explicitly_assigned_groups;
  }

  if (is_stale()) {
    out << " (needs update)";
  }
  out << "\n";
}

/**
 * Writes the list of texture references to the indicated output stream, one
 * per line.
 */
void EggFile::
write_texture_refs(std::ostream &out, int indent_level) const {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    reference->write(out, indent_level);
  }
}

/**
 * Recursively walks the egg hierarchy and removes any "backstage" nodes found
 * from the scene graph completely.  These aren't part of the egg scene
 * anyway, and removing them early helps reduce confusion.
 */
void EggFile::
remove_backstage(EggGroupNode *node) {
  EggGroupNode::iterator ci;
  ci = node->begin();
  while (ci != node->end()) {
    EggNode *child = (*ci);
    bool remove_child = false;

    if (child->is_of_type(EggGroup::get_class_type())) {
      EggGroup *egg_group;
      DCAST_INTO_V(egg_group, child);
      remove_child = egg_group->has_object_type("backstage");
    }

    if (remove_child) {
      ci = node->erase(ci);
    } else {
      if (child->is_of_type(EggGroupNode::get_class_type())) {
        // Recurse on children.
        remove_backstage(DCAST(EggGroupNode, child));
      }
      ++ci;
    }
  }
}

/**
 * After reloading the egg file for the second time in a given session,
 * rematches the texture pointers with the TextureReference objects.
 */
void EggFile::
rescan_textures() {
  nassertv(_data != nullptr);

  // Extract the set of textures referenced by this egg file.
  EggTextureCollection tc;
  tc.find_used_textures(_data);

  // Make sure each tref name is unique within a given file.
  tc.uniquify_trefs();

  typedef pmap<std::string, TextureReference *> ByTRefName;
  ByTRefName by_tref_name;
  for (Textures::const_iterator ti = _textures.begin();
       ti != _textures.end();
       ++ti) {
    TextureReference *ref = (*ti);
    by_tref_name[ref->get_tref_name()] = ref;
  }

  EggTextureCollection::iterator eti;
  for (eti = tc.begin(); eti != tc.end(); ++eti) {
    EggTexture *egg_tex = (*eti);

    ByTRefName::const_iterator tni = by_tref_name.find(egg_tex->get_name());
    if (tni == by_tref_name.end()) {
      // We didn't find this TRef name last time around!
      nout << _source_filename.get_basename()
           << " modified during session--TRef " << egg_tex->get_name()
           << " is new!\n";

    } else {
      TextureReference *ref = (*tni).second;
      ref->rebind_egg_data(_data, egg_tex);
    }
  }
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void EggFile::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_EggFile);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void EggFile::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);
  datagram.add_string(get_name());

  // We don't write out _data; that needs to be reread each session.

  datagram.add_string(FilenameUnifier::make_bam_filename(_current_directory));
  datagram.add_string(FilenameUnifier::make_bam_filename(_source_filename));
  datagram.add_string(FilenameUnifier::make_bam_filename(_dest_filename));
  datagram.add_string(_egg_comment);

  datagram.add_uint32(_textures.size());
  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    writer->write_pointer(datagram, (*ti));
  }

  _explicitly_assigned_groups.write_datagram(writer, datagram);
  writer->write_pointer(datagram, _default_group);

  // We don't write out _complete_groups; that is recomputed each session.

  datagram.add_bool(_is_surprise);
  datagram.add_bool(_is_stale);
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int EggFile::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  int i;
  _textures.reserve(_num_textures);
  for (i = 0; i < _num_textures; i++) {
    TextureReference *texture;
    DCAST_INTO_R(texture, p_list[pi], pi);
    _textures.push_back(texture);
    pi++;
  }

  pi += _explicitly_assigned_groups.complete_pointers(p_list + pi, manager);

  if (p_list[pi] != nullptr) {
    DCAST_INTO_R(_default_group, p_list[pi], pi);
  }
  pi++;

  return pi;
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable* EggFile::
make_EggFile(const FactoryParams &params) {
  EggFile *me = new EggFile();
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
void EggFile::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  set_name(scan.get_string());
  _current_directory = FilenameUnifier::get_bam_filename(scan.get_string());
  _source_filename = FilenameUnifier::get_bam_filename(scan.get_string());
  _dest_filename = FilenameUnifier::get_bam_filename(scan.get_string());
  if (Palettizer::_read_pi_version >= 9) {
    _egg_comment = scan.get_string();
  }

  _num_textures = scan.get_uint32();
  manager->read_pointers(scan, _num_textures);

  _explicitly_assigned_groups.fillin(scan, manager);
  manager->read_pointer(scan);  // _default_group

  _is_surprise = scan.get_bool();
  _is_stale = scan.get_bool();

  if (Palettizer::_read_pi_version < 11) {
    // If this file was written by a version of egg-palettize prior to 11, we
    // didn't store the tref names on the texture references.  Since we need
    // that information now, it follows that every egg file is stale.
    _is_stale = true;
  }
}
