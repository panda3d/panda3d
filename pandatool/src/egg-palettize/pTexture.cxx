// Filename: pTexture.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "pTexture.h"
#include "palette.h"
#include "attribFile.h"

#include <pnmImage.h>
#include <pnmReader.h>


PTexture::
PTexture(AttribFile *af, const Filename &name) : 
  _name(name),
  _attrib_file(af)
{
  _got_filename = false;
  _file_exists = false;
  _texture_changed = false;
  _unused = true;
  _matched_anything = false;
  _got_size = false;
  _got_req = false;
  _got_last_req = false;
  _margin = _attrib_file->_default_margin;
  _read_header = false;
  _omit = OR_none;
  _uses_alpha = false;
  _is_packed = false;
  _palette = NULL;
}

Filename PTexture::
get_name() const {
  return _name;
}
  
void PTexture::
add_filename(const Filename &filename) {
  if (!filename.exists()) {
    // Store the filename, even though it doesn't exist.
    if (!_got_filename) {
      _got_filename = true;
      _filename = filename;
      _file_exists = false;
    }

  } else {
    bool inserted = _filenames.insert(filename).second;

    if (!_got_filename || !_file_exists) {
      // This was the first filename we encountered; no sweat.
      _got_filename = true;
      _file_exists = true;
      _filename = filename;
      
      check_size();

    } else if (inserted) {
      // We had been using a different filename previously.  Now we've
      // got a new one.  Maybe this one is better?
      
      // First, read the headers.  We'll consider the larger image to
      // be the better choice.
      if (!_got_size) {
	read_header();
      }

      int oxsize, oysize;
      bool got_other_size =
	read_image_header(filename, oxsize, oysize);

      if (got_other_size) {
	if (!_got_size || oxsize * oysize > _xsize * _ysize) {
	  _filename = filename;
	  _xsize = oxsize;
	  _ysize = oysize;
	  _got_size = true;
	  _texture_changed = true;
	  
	} else if (oxsize * oysize == _xsize * _ysize) {
	  // If the sizes are the same, we'll consider the newer image
	  // to be the better choice.
	  if (filename.compare_timestamps(_filename, false, false) > 0) {
	    _filename = filename;
	    _texture_changed = true;
	  }
	}
      }
    }
  }
}

Filename PTexture::
get_filename() const {
  Filename filename = _name;
  filename.set_dirname(_attrib_file->_map_dirname);
  return filename;
}

Filename PTexture::
get_basename() const {
  return _name;
}

bool PTexture::
get_size(int &xsize, int &ysize) {
  if (!_got_size) {
    read_header();
  }

  if (!_got_size) {
    return false;
  }

  xsize = _xsize;
  ysize = _ysize;
  return true;
}

void PTexture::
set_size(int xsize, int ysize) {
  // If we've already read the file header, don't let anyone tell us
  // differently.
  if (!_read_header) {
    _xsize = xsize;
    _ysize = ysize;
    _got_size = true;
  }
}

bool PTexture::
get_req(int &xsize, int &ysize) {
  if (!_got_req) {
    return get_size(xsize, ysize);
  }
  xsize = _req_xsize;
  ysize = _req_ysize;
  return true;
}

bool PTexture::
get_last_req(int &xsize, int &ysize) {
  if (!_got_last_req) {
    return get_size(xsize, ysize);
  }
  xsize = _last_req_xsize;
  ysize = _last_req_ysize;
  return true;
}

void PTexture::
set_last_req(int xsize, int ysize) {
  _last_req_xsize = xsize;
  _last_req_ysize = ysize;
  _got_last_req = true;
}

void PTexture::
reset_req(int xsize, int ysize) {
  if (_got_last_req && 
      (_last_req_xsize != xsize || _last_req_ysize != ysize)) {
    // We've changed the requested size from the last time we ran.
    // That constitutes a change to the texture.
    _texture_changed = true;
  }

  _req_xsize = xsize;
  _req_ysize = ysize;
  _got_req = true;
}

void PTexture::
scale_req(double scale_pct) {
  if (!_got_size) {
    read_header();
  }
  if (_got_size) {
    reset_req(_xsize * scale_pct / 100.0,
	      _ysize * scale_pct / 100.0);
  }
}

void PTexture::
clear_req() {
  _got_req = false;
  _margin = _attrib_file->_default_margin;

  if (_omit != OR_cmdline) {
    // If we previously omitted this texture from the command line,
    // preserve that property.
    _omit = OR_none;
  }
}

double PTexture::
get_scale_pct() {
  if (!_got_size) {
    read_header();
  }
  if (_got_size && _got_req) {
    return 
      100.0 * (((double)_req_xsize / (double)_xsize) +
	       ((double)_req_ysize / (double)_ysize)) / 2.0;
  } else {
    return 100.0;
  }
}

int PTexture::
get_margin() const {
  return _margin;
}

void PTexture::
set_margin(int margin) {
  _margin = margin;
}

PTexture::OmitReason PTexture::
get_omit() const {
  return _omit;
}

void PTexture::
set_omit(OmitReason omit) {
  _omit = omit;
}

bool PTexture::
needs_refresh() {
  if (!_texture_changed) {
    // We consider the texture to be out-of-date if it's moved around
    // in the palette.
    _texture_changed = packing_changed();
  }

  if (!_texture_changed && _file_exists) {
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
	_texture_changed = true;
      }

    } else {
      // Compare to the resized file.
      target_filename = get_filename();
    }

    if (!_texture_changed) {
      _texture_changed = 
	(target_filename.compare_timestamps(_filename, true, false) < 0);
    }
  }

  return _texture_changed;
}

void PTexture::
set_changed(bool changed) {
  _texture_changed = changed;
}

bool PTexture::
unused() const {
  return _unused;
}

void PTexture::
set_unused(bool unused) {
  _unused = unused;
}

bool PTexture::
matched_anything() const {
  return _matched_anything;
}

void PTexture::
set_matched_anything(bool matched_anything) {
  _matched_anything = matched_anything;
}

bool PTexture::
uses_alpha() const {
  return _uses_alpha;
}

void PTexture::
set_uses_alpha(bool uses_alpha) {
  _uses_alpha = uses_alpha;
}

void PTexture::
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

void PTexture::
mark_unpacked() {
  _is_packed = false;
}

bool PTexture::
is_packed() const {
  return _is_packed;
}

// Returns the same thing as is_packed(), except it doesn't consider a
// texture that has been left alone on a palette to be packed.
bool PTexture::
is_really_packed() const {
  return _is_packed && _omit != OR_solitary;
}

Palette *PTexture::
get_palette() const {
  return _is_packed ? _palette : (Palette *)NULL;
}

bool PTexture::
get_packed_location(int &left, int &top) const {
  left = _pleft;
  top = _ptop;
  return _is_packed;
}

bool PTexture::
get_packed_size(int &xsize, int &ysize, int &margin) const {
  xsize = _pxsize;
  ysize = _pysize;
  margin = _pmargin;
  return _is_packed;
}

void PTexture::
record_orig_state() {
  // Records the current packing state, storing it aside as the state
  // at load time.  Later, when the packing state may have changed,
  // packing_changed() will return true if it has or false if it
  // has not.
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

bool PTexture::
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

void PTexture::
write_size(ostream &out) {
  if (_omit != OR_unused) {
    if (!_got_size) {
      read_header();
    }
    out << "  " << _name;
    if (_got_size) {
      out << " orig " << _xsize << " " << _ysize;
    }
    if (_got_req) {
      out << " new " << _req_xsize << " " << _req_ysize;
    }
    out << " " << get_scale_pct() << "%\n";
  }
}

void PTexture::
write_pathname(ostream &out) const {
  if (_got_filename && _omit != OR_unused) {
    if (!_file_exists) {
      nout << "Warning: texture " << _filename << " does not exist.\n";
    }

    out << "  " << _name << " " << _filename << "\n";

    // Also write out all the alternate filenames.
    Filenames::const_iterator fi;
    for (fi = _filenames.begin(); fi != _filenames.end(); ++fi) {
      if ((*fi) != _filename) {
	// Indent by the same number of spaces to line up the filenames.
	for (int i = 0; i < (int)_name.length() + 3; i++) {
	  out << ' ';
	}
	out << (*fi) << "\n";
      }
    }
  }
}

void PTexture::
write_unplaced(ostream &out) const {
  if (_omit != OR_none && _omit != OR_unused) {
    out << "unplaced " << get_name() << " because ";
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
    case OR_cmdline:
      out << "cmdline";
      break;
    default:
      nout << "Invalid type: " << (int)_omit << "\n";
      abort();
    }
    out << "\n";
  }
}

bool PTexture::
transfer() {
  bool okflag = true;

  Filename new_filename = get_filename();
  if (new_filename == _filename) {
    nout << "*** PTexture " << _name << " is already in the map directory!\n"
	 << "    Cannot modify texture in place!\n";
    return false;
  }

  int nx, ny;
  if (!get_req(nx, ny)) {
    nout << "Unknown size for image " << _name << "\n";
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
 
  PNMImage *image = read_image();
  if (image == NULL) {
    nout << "*** Unable to read " << _name << "\n";
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
    
  if (!image->write(new_filename)) {
    nout << "Error in writing.\n";
    okflag = false;
  }
  delete image;

  return okflag;
}

PNMImage *PTexture::
read_image() {
  if (!_got_filename || !_file_exists) {
    return NULL;
  }

  PNMImage *image = new PNMImage;
  if (image->read(_filename)) {
    return image;
  }

  // Hmm, it wasn't able to read the image successfully.  Oh well.
  delete image;
  return NULL;
}

void PTexture::
check_size() {
  // Make sure the file has the size it claims to have.
  if (_got_size) {
    int old_xsize = _xsize;
    int old_ysize = _ysize;
    read_header();
    if (_xsize != old_xsize && _ysize != old_ysize) {
      nout << "Source texture " << _name << " has changed size, from "
	   << old_xsize << " " << old_ysize << " to "
	   << _xsize << " " << _ysize << "\n";
      _texture_changed = true;
    }
  }
}

void PTexture::
read_header() {
  // Open the file and read its header to determine its size.
  if (_got_filename && _file_exists) {
    if (!_read_header) {
      _read_header = true;
      _got_size = read_image_header(_filename, _xsize, _ysize);
    }
    _read_header = true;
  }
}

bool PTexture::
read_image_header(const Filename &filename, int &xsize, int &ysize) {
  PNMImageHeader header;
  if (!header.read_header(filename)) {
    nout << "Warning: cannot read texture " << filename << "\n";
    return false;
  }

  xsize = header.get_x_size();
  ysize = header.get_y_size();
  return true;
}

int PTexture::
to_power_2(int value) {
  int x = 1;
  while ((x << 1) <= value) {
    x = (x << 1);
  }
  return x;
}
