// Filename: textureImage.cxx
// Created by:  drose (29Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "textureImage.h"
#include "sourceTextureImage.h"
#include "eggFile.h"
#include "paletteGroup.h"
#include "texturePlacement.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle TextureImage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextureImage::
TextureImage() {
  _preferred_source = (SourceTextureImage *)NULL;
  _read_source_image = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::note_egg_file
//       Access: Public
//  Description: Records that a particular egg file references this
//               texture.  This is essential to know when deciding how
//               to assign the TextureImage to the various
//               PaletteGroups.
////////////////////////////////////////////////////////////////////
void TextureImage::
note_egg_file(EggFile *egg_file) {
  nassertv(!egg_file->get_groups().empty());
  _egg_files.insert(egg_file);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::assign_groups
//       Access: Public
//  Description: Assigns the texture to all of the PaletteGroups the
//               various egg files that use it need.  Attempts to
//               choose the minimum set of PaletteGroups that
//               satisfies all of the egg files.
////////////////////////////////////////////////////////////////////
void TextureImage::
assign_groups() {
  nassertv(!_egg_files.empty());

  PaletteGroups definitely_in;

  // First, we need to eliminate from consideration all the egg files
  // that are already taken care of by the user's explicit group
  // assignments for this texture.
  WorkingEggs needed_eggs;

  if (_explicitly_assigned_groups.empty()) {
    // If we have no explicit group assignments, we must consider all
    // the egg files.
    copy(_egg_files.begin(), _egg_files.end(), back_inserter(needed_eggs));
  
  } else {
    // Otherwise, we only need to consider the egg files that don't
    // have any groups in common with our explicit assignments.

    EggFiles::const_iterator ei;
    for (ei = _egg_files.begin(); ei != _egg_files.end(); ++ei) {
      PaletteGroups intersect;
      intersect.make_intersection(_explicitly_assigned_groups, (*ei)->get_groups());
      if (!intersect.empty()) {
	// This egg file is satisfied by one of the texture's explicit
	// assignments.

	// We must use at least one of the explicitly-assigned groups
	// that satisfied the egg file.  We don't need to use all of
	// them, however, and we choose the first one arbitrarily.
	definitely_in.insert(*intersect.begin());

      } else {
	// This egg file was not satisfied by any of the texture's
	// explicit assignments.  Therefore, we'll need to choose some
	// additional group to assign the texture to, to make the egg
	// file happy.  Defer this a bit.
	needed_eggs.push_back(*ei);
      }
    }
  }

  while (!needed_eggs.empty()) {
    // We need to know the complete set of groups that we need to
    // consider adding the texture to.  This is the union of all the egg
    // files' requested groups.
    PaletteGroups total;
    WorkingEggs::const_iterator ei;
    for (ei = needed_eggs.begin(); ei != needed_eggs.end(); ++ei) {
      total.make_union(total, (*ei)->get_groups());
    }
    
    // Now, find the group that will satisfy the most egg files.  If two
    // groups satisfy the same number of egg files, choose the one that
    // has the fewest egg files sharing it.
    nassertv(!total.empty());
    PaletteGroups::iterator gi = total.begin();
    PaletteGroup *best = (*gi);
    int best_egg_count = compute_egg_count(best, needed_eggs);
    ++gi;
    while (gi != total.end()) {
      PaletteGroup *group = (*gi);
      int group_egg_count = compute_egg_count(group, needed_eggs);
      if (group_egg_count > best_egg_count ||
	  (group_egg_count == best_egg_count &&
	   group->get_egg_count() < best->get_egg_count())) {
	best = group;
	best_egg_count = group_egg_count;
      }
      ++gi;
    }
    
    // Okay, now we've picked the best group.  Eliminate all the eggs
    // from consideration that are satisfied by this group, and repeat.
    definitely_in.insert(best);
    
    WorkingEggs next_needed_eggs;
    for (ei = needed_eggs.begin(); ei != needed_eggs.end(); ++ei) {
      if ((*ei)->get_groups().count(best) == 0) {
	// This one wasn't eliminated.
	next_needed_eggs.push_back(*ei);
      }
    }
    needed_eggs.swap(next_needed_eggs);
  }

  // Finally, now that we've computed the set of groups we need to
  // assign the texture to, we need to reconcile this with the set of
  // groups we've assigned the texture to previously.
  assign_to_groups(definitely_in);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::get_groups
//       Access: Public
//  Description: Once get_groups() has been called, this returns the
//               actual set of groups the TextureImage has been
//               assigned to.
////////////////////////////////////////////////////////////////////
const PaletteGroups &TextureImage::
get_groups() const {
  return _actual_assigned_groups;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::get_placement
//       Access: Public
//  Description: Gets the TexturePlacement object which represents the
//               assignment of this texture to the indicated group.
//               If the texture has not been assigned to the indicated
//               group, returns NULL.
////////////////////////////////////////////////////////////////////
TexturePlacement *TextureImage::
get_placement(PaletteGroup *group) const {
  Placement::const_iterator pi;
  pi = _placement.find(group);
  if (pi == _placement.end()) {
    return (TexturePlacement *)NULL;
  }

  return (*pi).second;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::force_replace
//       Access: Public
//  Description: Removes the texture from any PaletteImages it is
//               assigned to, but does not remove it from the groups.
//               It will be re-placed within each group when
//               PaletteGroup::place_all() is called.
////////////////////////////////////////////////////////////////////
void TextureImage::
force_replace() {
  Placement::iterator pi;
  for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
    (*pi).second->force_replace();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::pre_txa_file
//       Access: Public
//  Description: Updates any internal state prior to reading the .txa
//               file.
////////////////////////////////////////////////////////////////////
void TextureImage::
pre_txa_file() {
  // Save our current properties, so we can note if they change.
  _pre_txa_properties = _properties;

  // Update our properties from the egg files that reference this
  // texture.  It's possible the .txa file will update them further.
  SourceTextureImage *source = get_preferred_source();
  if (source != (SourceTextureImage *)NULL) {
    _properties = source->get_properties();
  }

  _request.pre_txa_file();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::post_txa_file
//       Access: Public
//  Description: Once the .txa file has been read and the TextureImage
//               matched against it, considers applying the requested
//               size change.  Updates the TextureImage's size with
//               the size the texture ought to be, if this can be
//               determined.
////////////////////////////////////////////////////////////////////
void TextureImage::
post_txa_file() {
  // First, get the actual size of the texture.
  SourceTextureImage *source = get_preferred_source();
  if (source != (SourceTextureImage *)NULL) {
    if (source->get_size()) {
      _size_known = true;
      _x_size = source->get_x_size();
      _y_size = source->get_y_size();
      _properties._got_num_channels = true;
      _properties._num_channels = source->get_num_channels();
    }
  }

  // Now update this with a particularly requested size.
  if (_request._got_size) {
    _size_known = true;
    _x_size = _request._x_size;
    _y_size = _request._y_size;
  }
    
  if (_request._got_num_channels) {
    _properties._got_num_channels = true;
    _properties._num_channels = _request._num_channels;

  } else {
    // If we didn't request a particular number of channels, examine
    // the image to determine if we can downgrade it, for instance
    // from color to grayscale.
    if (_properties._got_num_channels &&
	(_properties._num_channels == 3 || _properties._num_channels == 4)) {
      consider_grayscale();
    }
  }

  if (_request._format != EggTexture::F_unspecified) {
    _properties._format = _request._format;
  }
  if (_request._minfilter != EggTexture::FT_unspecified) {
    _properties._minfilter = _request._minfilter;
  }
  if (_request._magfilter != EggTexture::FT_unspecified) {
    _properties._magfilter = _request._magfilter;
  }

  // Finally, make sure our properties are fully defined.
  _properties.fully_define();

  // Now, if our properties have changed in all that from our previous
  // session, we need to re-place ourself in all palette groups.
  if (_properties != _pre_txa_properties) {
    force_replace();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::determine_placement_size
//       Access: Public
//  Description: Calls determine_size() on each TexturePlacement for
//               the texture, to ensure that each TexturePlacement is
//               still requesting the best possible size for the
//               texture.
////////////////////////////////////////////////////////////////////
void TextureImage::
determine_placement_size() {
  Placement::iterator pi;
  for (pi = _placement.begin(); pi != _placement.end(); ++pi) {
    TexturePlacement *placement = (*pi).second;
    placement->determine_size();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::get_omit
//       Access: Public
//  Description: Returns true if the user specifically requested to
//               omit this texture via the "omit" keyword in the .txa
//               file, or false otherwise.
////////////////////////////////////////////////////////////////////
bool TextureImage::
get_omit() const {
  return _request._omit;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::get_repeat_threshold
//       Access: Public
//  Description: Returns the suitable repeat threshold for this
//               texture.  This is either the
//               Palettizer::_repeat_threshold parameter, given
//               globally via -r, or a particular value for this
//               texture as supplied by the "repeat" keyword in the
//               .txa file.
////////////////////////////////////////////////////////////////////
double TextureImage::
get_repeat_threshold() const {
  return _request._repeat_threshold;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::get_margin
//       Access: Public
//  Description: Returns the suitable repeat threshold for this
//               texture.  This is either the Palettizer::_margin
//               parameter, or a particular value for this texture as
//               supplied by the "margin" keyword in the .txa file.
////////////////////////////////////////////////////////////////////
int TextureImage::
get_margin() const {
  return _request._margin;
}


////////////////////////////////////////////////////////////////////
//     Function: TextureImage::get_source
//       Access: Public
//  Description: Returns the SourceTextureImage corresponding to the
//               given filename(s).  If the given filename has never
//               been used as a SourceTexture for this particular
//               texture, creates a new SourceTextureImage and returns
//               that.
////////////////////////////////////////////////////////////////////
SourceTextureImage *TextureImage::
get_source(const Filename &filename, const Filename &alpha_filename) {
  string key = filename.get_fullpath() + ":" + alpha_filename.get_fullpath();
  Sources::iterator si;
  si = _sources.find(key);
  if (si != _sources.end()) {
    return (*si).second;
  }

  SourceTextureImage *source = 
    new SourceTextureImage(this, filename, alpha_filename);
  _sources.insert(Sources::value_type(key, source));

  // Clear out the preferred source image to force us to rederive this
  // next time someone asks.
  _preferred_source = (SourceTextureImage *)NULL;
  _read_source_image = false;

  return source;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::get_preferred_source
//       Access: Public
//  Description: Determines the preferred source image for examining
//               size and reading pixels, etc.  This is the largest
//               and most recent of all the available source images.
////////////////////////////////////////////////////////////////////
SourceTextureImage *TextureImage::
get_preferred_source() {
  if (_preferred_source != (SourceTextureImage *)NULL) {
    return _preferred_source;
  }

  // Now examine all of the various source images available to us and
  // pick the most suitable.

  // **** For now, we arbitrarily pick the first one.
  if (!_sources.empty()) {
    _preferred_source = (*_sources.begin()).second;
  }

  return _preferred_source;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::read_source_image
//       Access: Public
//  Description: Reads in the original image, if it has not already
//               been read, and returns it.
////////////////////////////////////////////////////////////////////
const PNMImage &TextureImage::
read_source_image() {
  if (!_read_source_image) {
    SourceTextureImage *source = get_preferred_source();
    if (source != (SourceTextureImage *)NULL) {
      source->read(_source_image);
    }
    _read_source_image = true;
  }

  return _source_image;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::write_source_pathnames
//       Access: Public
//  Description: Writes the list of source pathnames that might
//               contribute to this texture to the indicated output
//               stream, one per line.
////////////////////////////////////////////////////////////////////
void TextureImage::
write_source_pathnames(ostream &out, int indent_level) const {
  Sources::const_iterator si;
  for (si = _sources.begin(); si != _sources.end(); ++si) {
    SourceTextureImage *source = (*si).second;

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

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::write_scale_info
//       Access: Public
//  Description: Writes the list of source pathnames that might
//               contribute to this texture to the indicated output
//               stream, one per line.
////////////////////////////////////////////////////////////////////
void TextureImage::
write_scale_info(ostream &out, int indent_level) {
  SourceTextureImage *source = get_preferred_source();
  indent(out, indent_level) << get_name() << " orig ";

  if (source == (SourceTextureImage *)NULL ||
      !source->is_size_known()) {
    out << "unknown";
  } else {
    out << source->get_x_size() << " " << source->get_y_size()
	<< " " << source->get_num_channels();
  }

  out << " new " << get_x_size() << " " << get_y_size()
      << " " << get_num_channels();

  if (source != (SourceTextureImage *)NULL &&
      source->is_size_known()) {
    double scale = 
      100.0 * (((double)get_x_size() / (double)source->get_x_size()) +
	       ((double)get_y_size() / (double)source->get_y_size())) / 2.0;
    out << " " << scale << "%";
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::compute_egg_count
//       Access: Private
//  Description: Counts the number of egg files in the indicated set
//               that will be satisfied if a texture is assigned to
//               the indicated group.
////////////////////////////////////////////////////////////////////
int TextureImage::
compute_egg_count(PaletteGroup *group, 
		  const TextureImage::WorkingEggs &egg_files) {
  int count = 0;

  WorkingEggs::const_iterator ei;
  for (ei = egg_files.begin(); ei != egg_files.end(); ++ei) {
    if ((*ei)->get_groups().count(group) != 0) {
      count++;
    }
  }

  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::assign_to_groups
//       Access: Private
//  Description: Assigns the texture to the indicated set of groups.
//               If the texture was previously assigned to any of
//               these groups, keeps the same TexturePlacement object
//               for the assignment; at the same time, deletes any
//               TexturePlacement objects that represent groups we are
//               no longer assigned to.
////////////////////////////////////////////////////////////////////
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
      // Here's a group we're now assigned to that we weren't assigned
      // to previously.
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
    // Here's a group we're now assigned to that we weren't assigned
    // to previously.
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

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::consider_grayscale
//       Access: Private
//  Description: Examines the actual contents of the image to
//               determine if it should maybe be considered a
//               grayscale image (even though it has separate rgb
//               components).
////////////////////////////////////////////////////////////////////
void TextureImage::
consider_grayscale() {
  const PNMImage &source = read_source_image();
  if (!source.is_valid()) {
    return;
  }

  for (int y = 0; y < source.get_y_size(); y++) {
    for (int x = 0; x < source.get_x_size(); x++) {
      const xel &v = source.get_xel_val(x, y);
      if (PPM_GETR(v) != PPM_GETG(v) || PPM_GETR(v) != PPM_GETB(v)) {
	// Here's a colored pixel.  We can't go grayscale.
	return;
      }
    }
  }

  // All pixels in the image were grayscale!
  _properties._num_channels -= 2;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void TextureImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_TextureImage);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::write_datagram
//       Access: Public, Virtual
//  Description: Fills the indicated datagram up with a binary
//               representation of the current object, in preparation
//               for writing to a Bam file.
////////////////////////////////////////////////////////////////////
void TextureImage::
write_datagram(BamWriter *writer, Datagram &datagram) {
  ImageFile::write_datagram(writer, datagram);
  datagram.add_string(get_name());

  // We don't write out _request; this is re-read from the .txa file
  // each time.

  // We don't write out _pre_txa_properties; this is transitional.

  // We don't write out _preferred_source; this is redetermined each
  // session.

  // We don't write out _explicitly_assigned_groups; this is re-read
  // from the .txa file each time.

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

  // We don't write out _read_source_image or _source_image; this must
  // be reread each session.
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::complete_pointers
//       Access: Public, Virtual
//  Description: Called after the object is otherwise completely read
//               from a Bam file, this function's job is to store the
//               pointers that were retrieved from the Bam file for
//               each pointer object written.  The return value is the
//               number of pointers processed from the list.
////////////////////////////////////////////////////////////////////
int TextureImage::
complete_pointers(vector_typedWriteable &plist, BamReader *manager) {
  nassertr((int)plist.size() >= _num_placement * 2 + _num_sources, 0);
  int index = 0;

  int i;
  for (i = 0; i < _num_placement; i++) {
    PaletteGroup *group;
    TexturePlacement *placement;
    DCAST_INTO_R(group, plist[index], index);
    index++;
    DCAST_INTO_R(placement, plist[index], index);
    index++;
    _placement.insert(Placement::value_type(group, placement));
  }

  for (i = 0; i < _num_sources; i++) {
    SourceTextureImage *source;
    DCAST_INTO_R(source, plist[index], index);
    string key = source->get_filename().get_fullpath() + ":" + 
      source->get_alpha_filename().get_fullpath();

    _sources.insert(Sources::value_type(key, source));
    index++;
  }

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::make_TextureImage
//       Access: Protected
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
////////////////////////////////////////////////////////////////////
TypedWriteable* TextureImage::
make_TextureImage(const FactoryParams &params) {
  TextureImage *me = new TextureImage;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureImage::fillin
//       Access: Protected
//  Description: Reads the binary data from the given datagram
//               iterator, which was written by a previous call to
//               write_datagram().
////////////////////////////////////////////////////////////////////
void TextureImage::
fillin(DatagramIterator &scan, BamReader *manager) {
  ImageFile::fillin(scan, manager);
  set_name(scan.get_string());

  _actual_assigned_groups.fillin(scan, manager);

  _num_placement = scan.get_uint32();
  manager->read_pointers(scan, this, _num_placement * 2);

  _num_sources = scan.get_uint32();
  manager->read_pointers(scan, this, _num_sources);
}
