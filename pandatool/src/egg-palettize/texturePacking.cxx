// Filename: texturePacking.cxx
// Created by:  drose (06Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "texturePacking.h"
#include "paletteGroup.h"
#include "attribFile.h"
#include "palette.h"
#include "pTexture.h"

#include <pnmImage.h>


////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TexturePacking::
TexturePacking(PTexture *texture, PaletteGroup *group) :
  _texture(texture),
  _group(group)
{
  _attrib_file = _texture->_attrib_file;
  _omit = _texture->_omit ? OR_omitted : OR_none;
  _unused = true;
  _uses_alpha = false;
  _is_packed = false;
  _palette = NULL;
  _packing_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TexturePacking::
~TexturePacking() {
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_texture
//       Access: Public
//  Description: Returns the texture this TexturePacking object refers
//               to.
////////////////////////////////////////////////////////////////////
PTexture *TexturePacking::
get_texture() const {
  return _texture;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_group
//       Access: Public
//  Description: Returns the palette group this TexturePacking object
//               refers to.
////////////////////////////////////////////////////////////////////
PaletteGroup *TexturePacking::
get_group() const {
  return _group;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_omit
//       Access: Public
//  Description: Returns the reason this texture was omitted from the
//               particular palette group, if it was.
////////////////////////////////////////////////////////////////////
TextureOmitReason TexturePacking::
get_omit() const {
  return _omit;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::set_omit
//       Access: Public
//  Description: Indicates the reason this texture was omitted from
//               the particular palette group, if it was.
////////////////////////////////////////////////////////////////////
void TexturePacking::
set_omit(TextureOmitReason omit) {
  _omit = omit;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::unused
//       Access: Public
//  Description: Returns true if this particular texture/group
//               combination seems to be unused by any egg files,
//               false if at least one egg file uses it.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
unused() const {
  return _unused;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::set_unused
//       Access: Public
//  Description: Sets the state of the unused flag.  See unused().
////////////////////////////////////////////////////////////////////
void TexturePacking::
set_unused(bool unused) {
  _unused = unused;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::uses_alpha
//       Access: Public
//  Description: Returns true if this texture seems to require alpha;
//               that is, at least one egg file that references the
//               texture specifies an alpha mode.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
uses_alpha() const {
  return _uses_alpha;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::set_uses_alpha
//       Access: Public
//  Description: Sets the state of the uses_alpha flag.  See
//               uses_alpha().
////////////////////////////////////////////////////////////////////
void TexturePacking::
set_uses_alpha(bool uses_alpha) {
  _uses_alpha = uses_alpha;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::pack
//       Access: Public
//  Description: Adds this texture to its appropriate palette group,
//               if it has not already been packed.  Returns true if
//               anything has changed (e.g. it has been packed), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
pack() {
  if (!_is_packed) {
    return _group->pack_texture(this, _attrib_file);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::unpack
//       Access: Public
//  Description: Removes this texture from its palette image if it is
//               on one.  Returns true if anything has changed
//               (e.g. it has been unpacked), false otherwise.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
unpack() {
  if (_is_packed) {
    return _palette->unpack_texture(this);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::prepare_repack
//       Access: Public
//  Description: Checks if the texture needs to be repacked into a
//               different location on the palette (for instance,
//               because it has changed size).  If so, unpacks it and
//               returns true; otherwise, leaves it alone and returns
//               false.
//
//               If unpacking it will leave a hole or some similar
//               nonsense, also sets optimal to false.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
prepare_repack(bool &optimal) {
  bool needs_repack = false;

  if (get_omit() == OR_none) {
    // Here's a texture that thinks it wants to be packed.  Does it?
    int xsize, ysize;
    int pal_xsize = _attrib_file->_pal_xsize;
    int pal_ysize = _attrib_file->_pal_ysize;
    if (!_texture->get_req(xsize, ysize)) {
      // If we don't know the texture's size, we can't place it.
      nout << "Warning!  Can't determine size of " << _texture->get_name()
	   << "\n";
      set_omit(OR_unknown);
	
    } else if ((xsize > pal_xsize || ysize > pal_ysize) ||
	       (xsize == pal_xsize && ysize == pal_ysize)) {
      // If the texture is too big for the palette (or exactly fills the
      // palette), we can't place it.
      set_omit(OR_size);
	
    } else {
      // Ok, this texture really does want to be packed.  Is it?
      int px, py, m;
      if (get_packed_size(px, py, m)) {
	// The texture is packed.  Does it have the right size?
	if (px != xsize || py != ysize) {
	  // Oops, we'll have to repack it.
	  unpack();
	  optimal = false;
	  needs_repack = true;
	}
	if (m != _texture->get_margin()) {
	  // The margin has changed, although not the size.  We
	  // won't have to repack it, but we do need to update it.
	  _texture->set_changed(true);
	}
      } else {
	// The texture isn't packed.  Need to pack it.
	needs_repack = true;
      }
    }
  }

  if (get_omit() != OR_none) {
    // Here's a texture that doesn't want to be packed.  Is it?
    if (unpack()) {
      // It was!  Not any more.
      optimal = false;
    }
  }

  return needs_repack;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::mark_pack_location
//       Access: Public
//  Description: Records the location at which the texture has been
//               packed.
////////////////////////////////////////////////////////////////////
void TexturePacking::
mark_pack_location(Palette *palette, int left, int top,
		   int xsize, int ysize, int margin) {
  _is_packed = true;
  _palette = palette;
  _pleft = left;
  _ptop = top;
  _pxsize = xsize;
  _pysize = ysize;
  _pmargin = margin;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::mark_unpacked
//       Access: Public
//  Description: Records that the texture has not been packed.
////////////////////////////////////////////////////////////////////
void TexturePacking::
mark_unpacked() {
  _is_packed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::is_packed
//       Access: Public
//  Description: Returns true if the texture has been packed, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
is_packed() const {
  return _is_packed;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::is_really_packed
//       Access: Public
//  Description: Returns the same thing as is_packed(), except it
//               doesn't consider a texture that has been left alone
//               on a palette to be packed.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
is_really_packed() const {
  return _is_packed && _omit != OR_solitary;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_palette
//       Access: Public
//  Description: Returns the particular palette image the texture has
//               been packed into, or NULL if the texture is unpacked.
////////////////////////////////////////////////////////////////////
Palette *TexturePacking::
get_palette() const {
  return _is_packed ? _palette : (Palette *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_packed_location
//       Access: Public
//  Description: Fills left and top with the upper-left corner of the
//               rectangle in which the texture has been packed within
//               its Palette image.  Returns true if the texture has
//               been packed, false otherwise.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
get_packed_location(int &left, int &top) const {
  left = _pleft;
  top = _ptop;
  return _is_packed;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_packed_size
//       Access: Public
//  Description: Fills xsize, ysize, and margin with the size of the
//               rectangle in which the texture has been packed within
//               its Palette image.  The margin is an interior margin.
//               Returns true if the texture has been packed, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
get_packed_size(int &xsize, int &ysize, int &margin) const {
  xsize = _pxsize;
  ysize = _pysize;
  margin = _pmargin;
  return _is_packed;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::record_orig_state
//       Access: Public
//  Description: Records the current packing state, storing it aside
//               as the state at load time.  Later, when the packing
//               state may have changed, packing_changed() will return
//               true if it has or false if it has not.
////////////////////////////////////////////////////////////////////
void TexturePacking::
record_orig_state() {
  _orig_is_packed = _is_packed;
  if (_is_packed) {
    _orig_palette_name = _palette->get_filename();
    _opleft = _pleft;
    _optop = _ptop;
    _opxsize = _pxsize;
    _opysize = _pysize;
    _opmargin = _pmargin;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::packing_changed
//       Access: Public
//  Description: Returns true if the packing has changed in any way
//               since the last call to record_orig_state(), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
packing_changed() const {
  if (_orig_is_packed != _is_packed) {
    return true;
  }
  if (_is_packed) {
    return _orig_palette_name != _palette->get_filename() ||
      _opleft != _pleft ||
      _optop != _ptop ||
      _opxsize != _pxsize ||
      _opysize != _pysize ||
      _opmargin != _pmargin;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::set_changed
//       Access: Public
//  Description: Sets the state of the changed flag.  If this is true,
//               the state of this texture on this group is known to
//               have changed in some way such that files that depend
//               on it will need to be rebuilt.
////////////////////////////////////////////////////////////////////
void TexturePacking::
set_changed(bool changed) {
  _packing_changed = changed;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::needs_refresh
//       Access: Public
//  Description: Returns true if the texture has changed in any
//               significant way and the palette it's placed on needs
//               to be regenerated.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
needs_refresh() {
  bool any_change = 
    _texture->_texture_changed || _packing_changed;

  if (!any_change) {
    // We consider the texture to be out-of-date if it's moved around
    // in the palette.
    any_change = packing_changed();
  }

  if (!any_change && _texture->_file_exists) {
    // Compare the texture's timestamp to that of its palette (or
    // resized copy).  If it's newer, it's changed and must be
    // replaced.

    Filename target_filename;
    if (is_packed() && _omit == OR_none) {
      // Compare to the palette file.
      target_filename = _palette->get_filename();
      if (_palette->new_palette()) {
	// It's a brand new palette; don't even bother comparing
	// timestamps.
	any_change = true;
      }

    } else {
      // Compare to the resized file.
      target_filename = get_new_filename();

      if (has_alpha_filename()) {
	// If we have an alpha file, compare to the older of the two
	// files.
	Filename alpha_filename = get_alpha_filename();
	if (target_filename.compare_timestamps(alpha_filename, true, true) > 0) {
	  target_filename = alpha_filename;
	}
      }
    }

    if (!any_change) {
      any_change = 
	(target_filename.compare_timestamps(get_old_filename(), true, false) < 0);
    }
  }

  return any_change;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::write_unplaced
//       Access: Public
//  Description: Writes an "unplaced" entry to the .pi file if this
//               texture has not been placed on the group, describing
//               the reason for the failure.
////////////////////////////////////////////////////////////////////
void TexturePacking::
write_unplaced(ostream &out) const {
  if (_omit != OR_none && _omit != OR_unused) {
    out << "unplaced " << _texture->get_name()
	<< " in " << _group->get_name() << " because ";
    switch (_omit) {
    case OR_size:
      out << "size";
      break;
    case OR_repeats:
      out << "repeats";
      break;
    case OR_omitted:
      out << "omitted";
      break;
    case OR_unused:
      out << "unused";
      break;
    case OR_unknown:
      out << "unknown";
      break;
    case OR_solitary:
      out << "solitary";
      break;
    default:
      nout << "Invalid type: " << (int)_omit << "\n";
      abort();
    }
    out << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_new_filename
//       Access: Public
//  Description: Returns the filename to which this texture will be
//               copied, assuming it is not placed on a palette.
////////////////////////////////////////////////////////////////////
Filename TexturePacking::
get_new_filename() const {
  Filename dirname = _group->get_full_dirname(_attrib_file);
  Filename new_filename(dirname, _texture->get_name());
  new_filename = _attrib_file->make_color_filename(new_filename);
  new_filename.standardize();
  return new_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::has_alpha_filename
//       Access: Public
//  Description: Returns true if this texture requires a separate
//               alpha image file.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
has_alpha_filename() const {
  return _uses_alpha && (_attrib_file->_alpha_type != (PNMFileType *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_alpha_filename
//       Access: Public
//  Description: Returns the filename for the alpha channel to which
//               this texture will be copied, assuming it is not
//               placed on a palette.
////////////////////////////////////////////////////////////////////
Filename TexturePacking::
get_alpha_filename() const {
  return _attrib_file->make_alpha_filename(get_new_filename());
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::get_old_filename
//       Access: Public
//  Description: Returns the original filename of this texture.
////////////////////////////////////////////////////////////////////
Filename TexturePacking::
get_old_filename() const {
  Filename old_filename = _texture->_filename;
  old_filename.standardize();
  return old_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::transfer
//       Access: Public
//  Description: Copies an unpalettized image to the install
//               directory, if it is not already there.  The
//               particular directory it is installed into may depend
//               on the PaletteGroup to which it has been added.
//               Returns true if successful, false if there is an
//               error.
////////////////////////////////////////////////////////////////////
bool TexturePacking::
transfer() {
  bool okflag = true;

  Filename old_filename = get_old_filename();
  Filename new_filename = get_new_filename();
  if (new_filename == old_filename) {
    nout << "*** Texture " << _texture->get_name()
	 << " is already in the map directory!\n"
	 << "    Cannot modify texture in place!\n";
    return false;
  }

  int nx, ny;
  if (!_texture->get_req(nx, ny)) {
    nout << "Unknown size for image " << _texture->get_name() << "\n";
    nx = 16;
    ny = 16;
  }

  if (_attrib_file->_force_power_2) {
    int newx = to_power_2(nx);
    int newy = to_power_2(ny);
    if (newx != nx || newy != ny) {
      nx = newx;
      ny = newy;
    }
  }
 
  PNMImage *image = _texture->read_image();
  if (image == NULL) {
    nout << "*** Unable to read " << _texture->get_name() << "\n";
    okflag = false;

    // Create a solid red texture for images we can't read.
    image = new PNMImage(nx, ny);
    image->fill(1.0, 0.0, 0.0);

  } else {
    // Should we scale it?
    if (nx != image->get_x_size() && ny != image->get_y_size()) {
      nout << "Resizing " << new_filename << " to " 
	   << nx << " " << ny << "\n";
      PNMImage *new_image =
	new PNMImage(nx, ny, image->get_color_type());
      new_image->gaussian_filter_from(0.5, *image);
      delete image;
      image = new_image;
      
    } else {
      nout << "Copying " << new_filename
	   << " (size " << nx << " " << ny << ")\n";
    }
  }
  
  new_filename.make_dir();
  if (!_attrib_file->write_image_file(*image, new_filename, get_alpha_filename())) {
    nout << "Error in writing.\n";
    okflag = false;
  }
  delete image;

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: TexturePacking::to_power_2
//       Access: Public, Static
//  Description: Returns the largest power of 2 less than or equal to
//               value.
////////////////////////////////////////////////////////////////////
int TexturePacking::
to_power_2(int value) {
  int x = 1;
  while ((x << 1) <= value) {
    x = (x << 1);
  }
  return x;
}
