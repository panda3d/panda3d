// Filename: palette.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "palette.h"
#include "paletteGroup.h"
#include "pTexture.h"
#include "texturePacking.h"
#include "attribFile.h"
#include "string_utils.h"

#include <notify.h>
#include <pnmImage.h>

bool Palette::TexturePlacement::
intersects(int hleft, int htop, int xsize, int ysize) const {
  int hright = hleft + xsize;
  int hbot = htop + ysize;

  int mright = _left + _xsize;
  int mbot = _top + _ysize;

  return 
    !(hleft >= mright || hright <= _left ||
      htop >= mbot || hbot <= _top);
}

PNMImage *Palette::TexturePlacement::
resize_image(PNMImage *source) const {
  // Compute need_*size to account for the (interior) margins.
  // However, if the size of the texture if very small, we want to
  // make exterior margins, so we won't reduce the size past a certain
  // point.
  int need_xsize = min(_xsize, max(_xsize - 2 * _margin, 8));
  int need_ysize = min(_ysize, max(_ysize - 2 * _margin, 8));

  if (source->get_x_size() == need_xsize && source->get_y_size() == need_ysize) {
    // The image is already the right size.
    return source;
  }

  PNMImage *new_image =
    new PNMImage(need_xsize, need_ysize, source->get_color_type());
  new_image->gaussian_filter_from(0.5, *source);
  delete source;
  return new_image;
}

PNMImage *Palette::TexturePlacement::
add_margins(PNMImage *source) const {
  if (_margin == 0) {
    // No margins to add.
    return source;
  }

  int orig_xsize = source->get_x_size();
  int orig_ysize = source->get_y_size();

  int need_xsize = orig_xsize + _margin * 2;
  int need_ysize = orig_ysize + _margin * 2;

  PNMImage *new_image =
    new PNMImage(need_xsize, need_ysize, source->get_color_type());
  new_image->copy_sub_image(*source, _margin, _margin);
  
  // Now extend the margin out to the sides.
  for (int i = 0; i < _margin; i++) {
    for (int x = 0; x < orig_xsize; x++) {
      // top edge
      new_image->set_xel(x + _margin, i, source->get_xel(x, 0));
      // bottom edge
      new_image->set_xel(x + _margin, need_ysize - 1 - i, 
			 source->get_xel(x, orig_ysize - 1));
    }
    for (int y = 0; y < orig_ysize; y++) {
      // left edge
      new_image->set_xel(i, y + _margin, source->get_xel(0, y));
      // right edge
      new_image->set_xel(need_xsize - 1 - i, y + _margin, 
			 source->get_xel(orig_xsize - 1, y));
    }

    for (int j = 0; j < _margin; j++) {
      // top-left corner
      new_image->set_xel(i, j, source->get_xel(0, 0)); 
      // top-right corner
      new_image->set_xel(need_xsize - 1 - i, j, 
			 source->get_xel(orig_xsize - 1, 0));
      // bottom-left corner
      new_image->set_xel(i, need_ysize - 1 - j, 
			 source->get_xel(0, orig_ysize - 1)); 
      // bottom-right corner
      new_image->set_xel(need_xsize - 1 - i, need_ysize - 1 - j,
			 source->get_xel(orig_xsize - 1, orig_ysize - 1));
    }
  }

  if (source->has_alpha()) {
    // Now do the same thing in the alpha channel.
    for (int i = 0; i < _margin; i++) {
      for (int x = 0; x < orig_xsize; x++) {
	// top edge
	new_image->set_alpha(x + _margin, i, source->get_alpha(x, 0));
	// bottom edge
	new_image->set_alpha(x + _margin, need_ysize - 1 - i, 
			     source->get_alpha(x, orig_ysize - 1));
      }
      for (int y = 0; y < orig_ysize; y++) {
	// left edge
	new_image->set_alpha(i, y + _margin, source->get_alpha(0, y));
	// right edge
	new_image->set_alpha(need_xsize - 1 - i, y + _margin, 
			     source->get_alpha(orig_xsize - 1, y));
      }
      
      for (int j = 0; j < _margin; j++) {
	// top-left corner
	new_image->set_alpha(i, j, source->get_alpha(0, 0)); 
	// top-right corner
	new_image->set_alpha(need_xsize - 1 - i, j, 
			     source->get_alpha(orig_xsize - 1, 0));
	// bottom-left corner
	new_image->set_alpha(i, need_ysize - 1 - j, 
			     source->get_alpha(0, orig_ysize - 1)); 
	// bottom-right corner
	new_image->set_alpha(need_xsize - 1 - i, need_ysize - 1 - j,
			     source->get_alpha(orig_xsize - 1, orig_ysize - 1));
      }
    }
  }

  delete source;
  return new_image;
}
  


Palette::
Palette(const Filename &filename, PaletteGroup *group,
	int xsize, int ysize, int components, AttribFile *attrib_file) :
  _filename(filename),
  _group(group),
  _xsize(xsize),
  _ysize(ysize),
  _components(components),
  _attrib_file(attrib_file)
{
  _index = -1;
  _palette_changed = false;
  _new_palette = false;
}

Palette::
Palette(PaletteGroup *group, int index,
	int xsize, int ysize, int components, AttribFile *attrib_file) :
  _group(group),
  _index(index),
  _xsize(xsize),
  _ysize(ysize),
  _components(components),
  _attrib_file(attrib_file)
{
  _palette_changed = false;
  _new_palette = true;
}

Palette::
~Palette() {
  // Unmark any textures we've had packed.
  TexPlace::iterator ti;
  for (ti = _texplace.begin(); ti != _texplace.end(); ++ti) {
    (*ti)._packing->mark_unpacked();
  }
}

Filename Palette::
get_filename() const {
  return _filename;
}

Filename Palette::
get_basename() const {
  return _basename;
}

bool Palette::
changed() const {
  return _palette_changed;
}

bool Palette::
new_palette() const {
  return _new_palette;
}

int Palette::
get_num_textures() const {
  return _texplace.size();
}

bool Palette::
check_uses_alpha() const {
  // Returns true if any texture in the palette uses alpha.
  TexPlace::const_iterator ti;
  for (ti = _texplace.begin(); ti != _texplace.end(); ++ti) {
    if ((*ti)._packing->uses_alpha()) {
      return true;
    }
  }

  return false;
}

void Palette::
get_size(int &xsize, int &ysize) const {
  xsize = _xsize;
  ysize = _ysize;
}
  

void Palette::
place_texture_at(TexturePacking *packing, int left, int top,
		 int xsize, int ysize, int margin) {
  nassertv(_group != (PaletteGroup *)NULL);
  TexturePlacement tp;
  tp._packing = packing;
  tp._left = left;
  tp._top = top;
  tp._xsize = xsize;
  tp._ysize = ysize;
  tp._margin = margin;
  _texplace.push_back(tp);

  packing->mark_pack_location(this, left, top, xsize, ysize, margin);
}

bool Palette::
pack_texture(TexturePacking *packing) {
  PTexture *texture = packing->get_texture();
  int xsize, ysize;
  if (!texture->get_req(xsize, ysize)) {
    return false;
  }
  
  int left, top;
  if (find_home(left, top, xsize, ysize)) {
    place_texture_at(packing, left, top, xsize, ysize, texture->get_margin());
    _palette_changed = true;
    return true;
  }
  return false;
}

bool Palette::
unpack_texture(TexturePacking *packing) {
  TexPlace::iterator ti;
  for (ti = _texplace.begin(); ti != _texplace.end(); ++ti) {
    if ((*ti)._packing == packing) {
      _texplace.erase(ti);
      packing->mark_unpacked();
      return true;
    }
  }
  return false;
}

// Attempt to shrink the palette down to the smallest possible (power
// of 2) size that includes all its textures.
void Palette::
optimal_resize() {
  if (_texplace.size() < 2) {
    // If we don't have at least two textures, there's no point in
    // shrinking the palette because we won't be using it anyway.
    // Might as well keep it full-sized so we can better pack future
    // textures.
    return;
  }

  int max_y = get_max_y();
  assert(max_y > 0);
  bool resized = false;

  while (max_y <= _ysize / 2) {
    // If at least half the palette is empty, we can try resizing.
    if (max_y <= _ysize / 4) {
      // Wow, three-quarters of the palette is empty.
      Palette *np = try_resize(_xsize / 2, _ysize / 2);
      if (np != NULL) {
	// Excellent!  We just reduced the palette size by half in
	// both dimensions.
	_texplace = np->_texplace;
	_xsize = _xsize / 2;
	_ysize = _ysize / 2;
	_palette_changed = true;
	delete np;
      } else {
	// Well, we couldn't reduce in both dimensions, but we can
	// always reduce twice in the y dimension.
	_ysize = _ysize / 4;
      }
    } else {
      _ysize = _ysize / 2;
    }
    resized = true;
    max_y = get_max_y();
  }

  assert(get_max_y() <= _ysize);

  if (resized) {
    nout << "Resizing " << get_basename() << " to " << _xsize << " " 
	 << _ysize << "\n";
    // If we've resized the palette, all the egg files that reference
    // these textures will need to be rebuilt.
    TexPlace::iterator ti;
    for (ti = _texplace.begin(); ti != _texplace.end(); ++ti) {
      (*ti)._packing->set_changed(true);
    }

    // And we'll have to mark the palette as a new image.
    _new_palette = true;
  }
}


void Palette::
finalize_palette() {
  if (_filename.empty()) {
    // Generate a filename based on the index number.
    char index_str[128];
    sprintf(index_str, "%03d", _index);
    _basename = _group->get_name() + "-palette." + index_str + ".rgb";

    Filename dirname = _group->get_full_dirname(_attrib_file);
    _filename = _basename;
    _filename.set_dirname(dirname.get_fullpath());
  } else {
    _basename = _filename.get_basename();
  }

  _components = check_uses_alpha() ? 4 : 3;

  if (_texplace.size() == 1) {
    // If we packed exactly one texture, never mind.
    TexturePacking *packing = (*_texplace.begin())._packing;

    // This is a little odd: we mark the texture as being omitted, but
    // we don't actually unpack it.  That way it will still be
    // recorded as belonging to this palette (for future
    // palettizations), but it will also be copied to the map
    // directory, and any egg files that reference it will use the
    // texture and not the palette.
    packing->set_omit(OR_solitary);
  }
}

void Palette::
write(ostream &out) const {
  out << "palette " << _attrib_file->write_pi_filename(_filename)
      << " in " << _group->get_name()
      << " size " << _xsize << " " << _ysize << " " << _components
      << "\n";
  TexPlace::const_iterator ti;
  for (ti = _texplace.begin(); ti != _texplace.end(); ++ti) {
    out << "  " << (*ti)._packing->get_texture()->get_name()
	<< " at " << (*ti)._left << " " << (*ti)._top
	<< " size " << (*ti)._xsize << " " << (*ti)._ysize
	<< " margin " << (*ti)._margin
	<< "\n";
  }
}

bool Palette::
generate_image() {
  bool okflag = true;

  PNMImage palette(_xsize, _ysize, _components);

  nout << "Generating " << _filename << "\n";

  TexPlace::const_iterator ti;
  for (ti = _texplace.begin(); ti != _texplace.end(); ++ti) {
    TexturePacking *packing = (*ti)._packing;
    PTexture *texture = packing->get_texture();
    nout << "  " << texture->get_name() << "\n";
    okflag = copy_texture_image(palette, *ti) && okflag;
  }

  _filename.make_dir();
  if (!palette.write(_filename)) {
    nout << "Error in writing.\n";
    okflag = false;
  }

  return okflag;
}

bool Palette::
refresh_image() {
  if (!_filename.exists()) {
    nout << "Palette image " << _filename << " does not exist, rebuilding.\n";
    return generate_image();
  }

  bool okflag = true;

  PNMImage palette;
  if (!palette.read(_filename)) {
    nout << "Unable to read old palette image " << _filename 
	 << ", rebuilding.\n";
    return generate_image();
  }

  bool any_changed = false;

  if (_components == 4 && !palette.has_alpha()) {
    palette.add_alpha();
    palette.alpha_fill(1.0);
    any_changed = true;

  } else if (_components == 3 && palette.has_alpha()) {
    palette.remove_alpha();
    any_changed = true;
  }

  TexPlace::const_iterator ti;
  for (ti = _texplace.begin(); ti != _texplace.end(); ++ti) {
    TexturePacking *packing = (*ti)._packing;
    PTexture *texture = packing->get_texture();
    if (packing->needs_refresh()) {
      if (!any_changed) {
	nout << "Refreshing " << _filename << "\n";
	any_changed = true;
      }

      nout << "  " << texture->get_name() << "\n";
      okflag = copy_texture_image(palette, *ti) && okflag;
    }
  }

  if (any_changed) {
    if (!palette.write(_filename)) {
      nout << "Error in writing.\n";
      okflag = false;
    }
  }

  return okflag;
}


Palette *Palette::
try_resize(int new_xsize, int new_ysize) const {
  Palette *np =
    new Palette(_group, _index, new_xsize, new_ysize,
		_components, _attrib_file);

  bool okflag = true;
  TexPlace::const_iterator ti;
  for (ti = _texplace.begin(); 
       ti != _texplace.end() && okflag;
       ++ti) {
    okflag = np->pack_texture((*ti)._packing);
  }

  if (okflag) {
    return np;
  } else {
    delete np;
    return NULL;
  }
}

int Palette::
get_max_y() const {
  int max_y = 0;

  TexPlace::const_iterator ti;
  for (ti = _texplace.begin(); ti != _texplace.end(); ++ti) {
    max_y = max(max_y, (*ti)._top + (*ti)._ysize);
  }

  return max_y;
}

bool Palette::
find_home(int &left, int &top, int xsize, int ysize) const {
  top = 0;
  while (top + ysize <= _ysize) {
    int next_y = _ysize;
    // Scan along the row at 'top'.
    left = 0;
    while (left + xsize <= _xsize) {
      int next_x = left;
      // Consider the spot at left, top.

      // Can we place it here?
      bool place_ok = true;
      TexPlace::const_iterator ti;
      for (ti = _texplace.begin(); 
	   ti != _texplace.end() && place_ok;
	   ++ti) {
	if ((*ti).intersects(left, top, xsize, ysize)) {
	  // Nope.
	  place_ok = false;
	  next_x = (*ti)._left + (*ti)._xsize;
	  next_y = min(next_y, (*ti)._top + (*ti)._ysize);
	}
      }
      
      if (place_ok) {
	// Hooray!
	return true;
      }

      assert(next_x > left);
      left = next_x;
    }

    assert(next_y > top);
    top = next_y;
  }

  // Nope, wouldn't fit anywhere.
  return false;
}

bool Palette::
copy_texture_image(PNMImage &palette, const TexturePlacement &tp) {
  bool okflag = true;
  PTexture *texture = tp._packing->get_texture();
  PNMImage *image = texture->read_image();
  if (image == NULL) {
    nout << "  *** Unable to read " << texture->get_name() << "\n";
    okflag = false;
    
    // Create a solid red texture for images we can't read.
    image = new PNMImage(tp._xsize, tp._ysize);
    image->fill(1.0, 0.0, 0.0);
    
  } else {
    image = tp.add_margins(tp.resize_image(image));
  }
  
  if (_components == 4 && !image->has_alpha()) {
    // We need to add an alpha channel for the image if the
    // palette has an alpha channel.
    image->add_alpha();
    image->alpha_fill(1.0);
  }
  palette.copy_sub_image(*image, tp._left, tp._top);
  delete image;

  return okflag;
}
