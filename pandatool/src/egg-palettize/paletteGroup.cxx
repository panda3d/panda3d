// Filename: paletteGroup.cxx
// Created by:  drose (06Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "paletteGroup.h"
#include "texturePacking.h"
#include "palette.h"
#include "attribFile.h"

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PaletteGroup::
PaletteGroup(const string &name) : Namable(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PaletteGroup::
~PaletteGroup() {
  clear_palettes();
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::get_num_parents
//       Access: Public
//  Description: Returns the number of dependent PaletteGroup this
//               PaletteGroup can share its textures with.  See
//               get_parent().
////////////////////////////////////////////////////////////////////
int PaletteGroup::
get_num_parents() const {
  return _parents.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::get_parent
//       Access: Public
//  Description: Returns the nth dependent PaletteGroup this
//               PaletteGroup can share its textures with.  If a
//               texture is added to this group that already appears
//               one of these groups, the reference to the shared
//               texture is used instead of adding a new texture.
////////////////////////////////////////////////////////////////////
PaletteGroup *PaletteGroup::
get_parent(int n) const {
  nassertr(n >= 0 && n < (int)_parents.size(), (PaletteGroup *)NULL);
  return _parents[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::add_parent
//       Access: Public
//  Description: Adds a new PaletteGroup to the set of PaletteGroups
//               this one can share its textures with.  See
//               get_parent().
////////////////////////////////////////////////////////////////////
void PaletteGroup::
add_parent(PaletteGroup *parent) {
  _parents.push_back(parent);
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::get_dirname
//       Access: Public
//  Description: Returns the directory name to which palettes and
//               textures associated with this group will be written.
////////////////////////////////////////////////////////////////////
const string &PaletteGroup::
get_dirname() const {
  return _dirname;
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::set_dirname
//       Access: Public
//  Description: Sets the directory name to which palettes and
//               textures associated with this group will be written.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
set_dirname(const string &dirname) {
  _dirname = dirname;
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::pack_texture
//       Access: Public
//  Description: Adds the texture to some suitable palette image
//               within the PaletteGroup.  Returns true if
//               successfully packed, false otherwise.
////////////////////////////////////////////////////////////////////
bool PaletteGroup::
pack_texture(TexturePacking *packing, AttribFile *attrib_file) {
  // Now try to place it in each of our existing palettes.
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    if ((*pi)->pack_texture(packing)) {
      return true;
    }
  }

  // It didn't place anywhere; create a new palette for it.
  Palette *palette = 
    new Palette(this, _palettes.size() + 1, 
		attrib_file->_pal_xsize, attrib_file->_pal_ysize,
		0, attrib_file);
  if (!palette->pack_texture(packing)) {
    // Hmm, it didn't fit on an empty palette.  Must be too big.
    packing->set_omit(OR_size);
    delete palette;
    return false;
  }
  _palettes.push_back(palette);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::generate_palette_images
//       Access: Public
//  Description: After all of the textures has been placed, generates
//               the actual image files for each palette image within
//               the group.  Returns true if successful, false on
//               error.
////////////////////////////////////////////////////////////////////
bool PaletteGroup::
generate_palette_images() {
  bool okflag = true;

  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    Palette *palette = (*pi);
    if (palette->new_palette()) {
      // If the palette is a new palette, we'll have to generate a new
      // image from scratch.
      okflag = palette->generate_image() && okflag;
    } else {
      // Otherwise, we can probably get by with just updating
      // whichever images, if any, have changed.
      okflag = palette->refresh_image() && okflag;
    }
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::optimal_resize
//       Access: Public
//  Description: Resizes each palette image within the group downward,
//               if possible, to the smallest power-of-two size that
//               holds all of its images.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
optimal_resize() {
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    (*pi)->optimal_resize();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::finalize_palettes
//       Access: Public
//  Description: Performs some finalization of the palette images,
//               such as generating filenames.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
finalize_palettes() {
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    (*pi)->finalize_palette();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::add_palette
//       Access: Public
//  Description: Adds the indicated already-created Palette image to
//               the group.  This is mainly intended to be called from
//               AttribFile when reading in a previously-built .pi
//               file.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
add_palette(Palette *palette) {
  _palettes.push_back(palette);
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::remove_palette_files
//       Access: Public
//  Description: Removes all the image files generated for the Palette
//               images within the group.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
remove_palette_files() {
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    // Remove the old palette file?
    Palette *palette = *pi;
    if (!palette->get_filename().empty()) {
      if (palette->get_filename().exists()) {
	nout << "Deleting " << palette->get_filename() << "\n";
	palette->get_filename().unlink();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::clear_palettes
//       Access: Public
//  Description: Deletes all of the Palette objects within the group.
//               This loses all information about where textures are
//               packed.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
clear_palettes() {
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    delete *pi;
  }
  _palettes.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::complete_groups
//       Access: Public, Static
//  Description: Expands the set of PaletteGroups to include all
//               parents and parents of parents.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
complete_groups(PaletteGroups &groups) {
  // Make a copy so we can safely modify the original set as we
  // traverse the copy.
  PaletteGroups groups_copy = groups;
  PaletteGroups::const_iterator gi;
  for (gi = groups_copy.begin(); gi != groups_copy.end(); ++gi) {
    (*gi)->add_ancestors(groups);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::add_ancestors
//       Access: Public
//  Description: Adds all the ancestors of this PaletteGroup to the
//               indicated set.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
add_ancestors(PaletteGroups &groups) {
  Parents::const_iterator pri;
  for (pri = _parents.begin(); pri != _parents.end(); ++pri) {
    PaletteGroup *parent = *pri;
    bool inserted = groups.insert(parent).second;
    if (inserted) {
      parent->add_ancestors(groups);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PaletteGroup::write
//       Access: Public
//  Description: Writes out a .pi file description of the palette
//               group and all of its nested Palette images.
////////////////////////////////////////////////////////////////////
void PaletteGroup::
write(ostream &out) const {
  Palettes::const_iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    out << "\n";
    (*pi)->write(out);
  }
}
