// Filename: eggFile.cxx
// Created by:  drose (29Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "eggFile.h"
#include "textureImage.h"
#include "paletteGroup.h"
#include "texturePlacement.h"
#include "palettizer.h"

#include <eggData.h>
#include <eggTextureCollection.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle EggFile::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggFile::
EggFile() {
  _data = (EggData *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::from_command_line
//       Access: Public
//  Description: Accepts the information about the egg file as
//               supplied from the command line.
////////////////////////////////////////////////////////////////////
void EggFile::
from_command_line(EggData *data,
		  const Filename &source_filename, 
		  const Filename &dest_filename) {
  _data = data;
  _source_filename = source_filename;
  _dest_filename = dest_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::scan_textures
//       Access: Public
//  Description: Scans the egg file for texture references and updates
//               the _textures list appropriately.  This assumes the
//               egg file was supplied on the command line and thus
//               the _data member is available.
////////////////////////////////////////////////////////////////////
void EggFile::
scan_textures() {
  nassertv(_data != (EggData *)NULL);

  EggTextureCollection tc;
  tc.find_used_textures(_data);

  // Remove the old TextureReference objects.
  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    delete (*ti);
  }
  _textures.clear();

  EggTextureCollection::iterator eti;
  for (eti = tc.begin(); eti != tc.end(); ++eti) {
    EggTexture *egg_tex = (*eti);

    TextureReference *ref = new TextureReference;
    ref->from_egg(_data, egg_tex);

    _textures.push_back(ref);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::get_textures
//       Access: Public
//  Description: Fills up the indicated set with the set of textures
//               referenced by this egg file.  It is the user's
//               responsibility to ensure the set is empty before
//               making this call; otherwise, the new textures will be
//               appended to the existing set.
////////////////////////////////////////////////////////////////////
void EggFile::
get_textures(set<TextureImage *> &result) const {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    result.insert((*ti)->get_texture());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::post_txa_file
//       Access: Public
//  Description: Once the egg file has been matched against all of the
//               matching lines the .txa file, do whatever adjustment
//               is necessary.
////////////////////////////////////////////////////////////////////
void EggFile::
post_txa_file() {
  if (_assigned_groups.empty()) {
    // If the egg file has been assigned to no groups, we have to
    // assign it to something.
    _assigned_groups.insert(pal->get_default_group());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::get_groups
//       Access: Public
//  Description: Returns the set of PaletteGroups that the egg file
//               has been explicitly assigned to in the .txa file.
////////////////////////////////////////////////////////////////////
const PaletteGroups &EggFile::
get_groups() const {
  return _assigned_groups;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::build_cross_links
//       Access: Public
//  Description: Calls TextureImage::note_egg_file() for each texture
//               the egg file references, and
//               PaletteGroup::increment_egg_count() for each palette
//               group it wants.  This sets up some of the back
//               references to support determining an ideal texture
//               assignment.
////////////////////////////////////////////////////////////////////
void EggFile::
build_cross_links() {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    (*ti)->get_texture()->note_egg_file(this);
  }

  _assigned_groups.make_complete(_assigned_groups);

  PaletteGroups::const_iterator gi;
  for (gi = _assigned_groups.begin();
       gi != _assigned_groups.end();
       ++gi) {
    (*gi)->increment_egg_count();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::choose_placements
//       Access: Public
//  Description: Once all the textures have been assigned to groups
//               (but before they may actually be placed), chooses a
//               suitable TexturePlacement for each texture that
//               appears in the egg file.  This will be necessary to
//               do at some point before writing out the egg file
//               anyway, and doing it before the textures are placed
//               allows us to decide what the necessary UV range is
//               for each to-be-placed texture.
////////////////////////////////////////////////////////////////////
void EggFile::
choose_placements() {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    TextureImage *texture = reference->get_texture();

    if (reference->get_placement() != (TexturePlacement *)NULL &&
	texture->get_groups().count(reference->get_placement()->get_group()) != 0) {
      // The egg file is already using a TexturePlacement that is
      // suitable.  Don't bother changing it.

    } else {
      // We need to select a new TexturePlacement.
      PaletteGroups groups;
      groups.make_intersection(get_groups(), texture->get_groups());
      
      // Now groups is the set of groups that the egg file requires,
      // which also happen to include the texture.  It better not be
      // empty.
      nassertv(!groups.empty());
      
      // It doesn't really matter which group in the set we choose, so
      // we arbitrarily choose the first one.
      PaletteGroup *group = (*groups.begin());

      // Now get the TexturePlacement object that corresponds to the
      // placement of this texture into this group.
      TexturePlacement *placement = texture->get_placement(group);
      nassertv(placement != (TexturePlacement *)NULL);
      
      reference->set_placement(placement);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::update_egg
//       Access: Public
//  Description: Once all textures have been placed appropriately,
//               updates the egg file with all the information to
//               reference the new textures.
////////////////////////////////////////////////////////////////////
void EggFile::
update_egg() {
  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    reference->update_egg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::write_texture_refs
//       Access: Public
//  Description: Writes the list of texture references to the
//               indicated output stream, one per line.
////////////////////////////////////////////////////////////////////
void EggFile::
write_texture_refs(ostream &out, int indent_level) const {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    TextureReference *reference = (*ti);
    reference->write(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void EggFile::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_EggFile);
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::write_datagram
//       Access: Public, Virtual
//  Description: Fills the indicated datagram up with a binary
//               representation of the current object, in preparation
//               for writing to a Bam file.
////////////////////////////////////////////////////////////////////
void EggFile::
write_datagram(BamWriter *writer, Datagram &datagram) {
  datagram.add_string(get_name());

  // We don't write out _data; that needs to be reread each session.

  datagram.add_string(_source_filename);
  datagram.add_string(_dest_filename);

  datagram.add_uint32(_textures.size());
  Textures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    writer->write_pointer(datagram, (*ti));
  }

  _assigned_groups.write_datagram(writer, datagram);
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::complete_pointers
//       Access: Public, Virtual
//  Description: Called after the object is otherwise completely read
//               from a Bam file, this function's job is to store the
//               pointers that were retrieved from the Bam file for
//               each pointer object written.  The return value is the
//               number of pointers processed from the list.
////////////////////////////////////////////////////////////////////
int EggFile::
complete_pointers(vector_typedWriteable &plist, BamReader *manager) {
  nassertr((int)plist.size() >= _num_textures, 0);
  int index = 0;

  int i;
  _textures.reserve(_num_textures);
  for (i = 0; i < _num_textures; i++) {
    TextureReference *texture;
    DCAST_INTO_R(texture, plist[index], index);
    _textures.push_back(texture);
    index++;
  }

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::make_EggFile
//       Access: Protected
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
////////////////////////////////////////////////////////////////////
TypedWriteable* EggFile::
make_EggFile(const FactoryParams &params) {
  EggFile *me = new EggFile;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: EggFile::fillin
//       Access: Protected
//  Description: Reads the binary data from the given datagram
//               iterator, which was written by a previous call to
//               write_datagram().
////////////////////////////////////////////////////////////////////
void EggFile::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  _source_filename = scan.get_string();
  _dest_filename = scan.get_string();

  _num_textures = scan.get_uint32();
  manager->read_pointers(scan, this, _num_textures);

  _assigned_groups.fillin(scan, manager);
}
