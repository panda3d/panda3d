// Filename: palettePage.cxx
// Created by:  drose (01Dec00)
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

#include "palettePage.h"
#include "texturePlacement.h"
#include "textureImage.h"
#include "paletteImage.h"
#include "paletteGroup.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

#include <algorithm>

TypeHandle PalettePage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::Default Constructor
//       Access: Private
//  Description: The default constructor is only for the convenience
//               of the Bam reader.
////////////////////////////////////////////////////////////////////
PalettePage::
PalettePage() {
  _group = (PaletteGroup *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PalettePage::
PalettePage(PaletteGroup *group, const TextureProperties &properties) :
  Namable(properties.get_string()),
  _group(group),
  _properties(properties)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::get_group
//       Access: Public
//  Description: Returns the group this particular PalettePage belongs
//               to.
////////////////////////////////////////////////////////////////////
PaletteGroup *PalettePage::
get_group() const {
  return _group;
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::get_properties
//       Access: Public
//  Description: Returns the texture grouping properties that all
//               textures in this page share.
////////////////////////////////////////////////////////////////////
const TextureProperties &PalettePage::
get_properties() const {
  return _properties;
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::assign
//       Access: Public
//  Description: Adds the indicated texture to the list of textures to
//               consider placing on the page.
////////////////////////////////////////////////////////////////////
void PalettePage::
assign(TexturePlacement *placement) {
  _assigned.push_back(placement);
}


////////////////////////////////////////////////////////////////////
//     Function: PalettePage::place_all
//       Access: Public
//  Description: Assigns all the textures to their final home in a
//               PaletteImage somewhere.
////////////////////////////////////////////////////////////////////
void PalettePage::
place_all() {
  // Sort the textures to be placed in order from biggest to smallest,
  // as an aid to optimal packing.
  sort(_assigned.begin(), _assigned.end(), SortPlacementBySize());

  Assigned::const_iterator ai;
  for (ai = _assigned.begin(); ai != _assigned.end(); ++ai) {
    TexturePlacement *placement = (*ai);
    place(placement);
  }

  _assigned.clear();

  // Now, look for solitary images; these are left placed, but flagged
  // with OR_solitary, so they won't go into egg references.  There's
  // no real point in referencing these.
  Images::iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    PaletteImage *image = (*ii);
    image->check_solitary();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::place
//       Access: Public
//  Description: Assigns the particular TexturePlacement to a
//               PaletteImage where it fits.
////////////////////////////////////////////////////////////////////
void PalettePage::
place(TexturePlacement *placement) {
  nassertv(placement->get_omit_reason() == OR_working);

  // First, try to place it in one of our existing PaletteImages.
  Images::iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    PaletteImage *image = (*ii);
    if (image->place(placement)) {
      return;
    }
  }

  // No good?  Then we need to create a new PaletteImage for it.
  PaletteImage *image = new PaletteImage(this, _images.size());
  _images.push_back(image);

  bool placed = image->place(placement);

  // This should have stuck.
  nassertv(placed);
}


////////////////////////////////////////////////////////////////////
//     Function: PalettePage::unplace
//       Access: Public
//  Description: Removes the TexturePlacement from wherever it has
//               been placed.
////////////////////////////////////////////////////////////////////
void PalettePage::
unplace(TexturePlacement *placement) {
  nassertv(placement->is_placed() && placement->get_page() == this);
  placement->get_image()->unplace(placement);
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::write_image_info
//       Access: Public
//  Description: Writes a list of the PaletteImages associated with
//               this page, and all of their textures, to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
void PalettePage::
write_image_info(ostream &out, int indent_level) const {
  Images::const_iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    PaletteImage *image = (*ii);
    if (!image->is_empty()) {
      indent(out, indent_level);
      image->output_filename(out);
      out << "\n";
      image->write_placements(out, indent_level + 2);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::optimal_resize
//       Access: Public
//  Description: Attempts to resize each PalettteImage down to its
//               smallest possible size.
////////////////////////////////////////////////////////////////////
void PalettePage::
optimal_resize() {
  Images::iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    PaletteImage *image = (*ii);
    image->optimal_resize();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::reset_images
//       Access: Public
//  Description: Throws away all of the current PaletteImages, so that
//               new ones may be created (and the packing made more
//               optimal).
////////////////////////////////////////////////////////////////////
void PalettePage::
reset_images() {
  Images::iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    PaletteImage *image = (*ii);
    image->reset_image();
    delete image;
  }

  _images.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::setup_shadow_images
//       Access: Public
//  Description: Ensures that each PaletteImage's _shadow_image has
//               the correct filename and image types, based on what
//               was supplied on the command line and in the .txa
//               file.
////////////////////////////////////////////////////////////////////
void PalettePage::
setup_shadow_images() {
  Images::iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    PaletteImage *image = (*ii);
    image->setup_shadow_image();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::update_images
//       Access: Public
//  Description: Regenerates each PaletteImage on this page that needs
//               it.
////////////////////////////////////////////////////////////////////
void PalettePage::
update_images(bool redo_all) {
  Images::iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    PaletteImage *image = (*ii);
    image->update_image(redo_all);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void PalettePage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PalettePage);
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::write_datagram
//       Access: Public, Virtual
//  Description: Fills the indicated datagram up with a binary
//               representation of the current object, in preparation
//               for writing to a Bam file.
////////////////////////////////////////////////////////////////////
void PalettePage::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);
  datagram.add_string(get_name());

  writer->write_pointer(datagram, _group);
  _properties.write_datagram(writer, datagram);

  // We don't write out _assigned, since that's rebuilt each session.

  datagram.add_uint32(_images.size());
  Images::const_iterator ii;
  for (ii = _images.begin(); ii != _images.end(); ++ii) {
    writer->write_pointer(datagram, *ii);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::complete_pointers
//       Access: Public, Virtual
//  Description: Called after the object is otherwise completely read
//               from a Bam file, this function's job is to store the
//               pointers that were retrieved from the Bam file for
//               each pointer object written.  The return value is the
//               number of pointers processed from the list.
////////////////////////////////////////////////////////////////////
int PalettePage::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  if (p_list[pi] != (TypedWritable *)NULL) {
    DCAST_INTO_R(_group, p_list[pi], pi);
  }
  pi++;

  pi += _properties.complete_pointers(p_list + pi, manager);

  int i;
  _images.reserve(_num_images);
  for (i = 0; i < _num_images; i++) {
    PaletteImage *image;
    DCAST_INTO_R(image, p_list[pi++], pi);
    _images.push_back(image);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::make_PalettePage
//       Access: Protected
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
////////////////////////////////////////////////////////////////////
TypedWritable* PalettePage::
make_PalettePage(const FactoryParams &params) {
  PalettePage *me = new PalettePage;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: PalettePage::fillin
//       Access: Protected
//  Description: Reads the binary data from the given datagram
//               iterator, which was written by a previous call to
//               write_datagram().
////////////////////////////////////////////////////////////////////
void PalettePage::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  set_name(scan.get_string());

  manager->read_pointer(scan);  // _group
  _properties.fillin(scan, manager);

  _num_images = scan.get_uint32();
  manager->read_pointers(scan, _num_images);
}
