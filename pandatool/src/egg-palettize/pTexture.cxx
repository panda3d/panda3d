// Filename: pTexture.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "pTexture.h"
#include "texturePacking.h"
#include "palette.h"
#include "attribFile.h"

#include <pnmImage.h>
#include <pnmReader.h>

////////////////////////////////////////////////////////////////////
//     Function: PTexture::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PTexture::
PTexture(AttribFile *attrib_file, const Filename &name) : 
  _name(name),
  _attrib_file(attrib_file)
{
  _got_filename = false;
  _file_exists = false;
  _texture_changed = false;
  _matched_anything = false;
  _got_size = false;
  _got_req = false;
  _got_last_req = false;
  _margin = _attrib_file->_default_margin;
  _omit = false;
  _read_header = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PTexture::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PTexture::
~PTexture() {
  Packing::iterator pi;
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    delete (*pi).second;
  }
}

Filename PTexture::
get_name() const {
  return _name;
}
  
void PTexture::
add_filename(Filename filename) {
  filename.make_absolute();

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

      int oxsize, oysize, ozsize;
      bool got_other_size =
	read_image_header(filename, oxsize, oysize, ozsize);

      if (got_other_size) {
	if (!_got_size || oxsize * oysize > _xsize * _ysize) {
	  _filename = filename;
	  _xsize = oxsize;
	  _ysize = oysize;
	  _zsize = ozsize;
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

bool PTexture::
get_size(int &xsize, int &ysize, int &zsize) {
  if (!_got_size) {
    read_header();
  }

  if (!_got_size) {
    return false;
  }

  xsize = _xsize;
  ysize = _ysize;
  zsize = _zsize;
  return true;
}

void PTexture::
set_size(int xsize, int ysize, int zsize) {
  // If we've already read the file header, don't let anyone tell us
  // differently.
  if (!_read_header) {
    _xsize = xsize;
    _ysize = ysize;
    _zsize = zsize;
    _got_size = true;
  }
}

bool PTexture::
get_req(int &xsize, int &ysize) {
  if (!_got_req) {
    int zsize;
    return get_size(xsize, ysize, zsize);
  }
  xsize = _req_xsize;
  ysize = _req_ysize;
  return true;
}

bool PTexture::
get_last_req(int &xsize, int &ysize) {
  if (!_got_last_req) {
    int zsize;
    return get_size(xsize, ysize, zsize);
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

////////////////////////////////////////////////////////////////////
//     Function: PTexture::user_omit
//       Access: Public
//  Description: Omits the texture from all palettes, based on a
//               user-supplied "omit" parameter (presumably read from
//               the .txa file).
////////////////////////////////////////////////////////////////////
void PTexture::
user_omit() {
  _omit = true;
  
  // Also omit each of our already-packed textures.
  Packing::iterator pi;
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    (*pi).second->set_omit(OR_omitted);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PTexture::add_to_group
//       Access: Public
//  Description: Adds the texture to the indicated PaletteGroup, if it
//               has not already been added.  Returns the
//               TexturePacking pointer that represents the addition.
////////////////////////////////////////////////////////////////////
TexturePacking *PTexture::
add_to_group(PaletteGroup *group) {
  Packing::iterator pi;
  pi = _packing.find(group);
  if (pi != _packing.end()) {
    return (*pi).second;
  }

  TexturePacking *packing = new TexturePacking(this, group);
  _packing[group] = packing;
  _attrib_file->_packing.push_back(packing);
  return packing;
}

////////////////////////////////////////////////////////////////////
//     Function: PTexture::check_group
//       Access: Public
//  Description: If the texture has already been added to the
//               indicated PaletteGroup, returns the associated
//               TexturePacking object.  If it is not yet been added,
//               returns NULL.
////////////////////////////////////////////////////////////////////
TexturePacking *PTexture::
check_group(PaletteGroup *group) const {
  Packing::const_iterator pi;
  pi = _packing.find(group);
  if (pi != _packing.end()) {
    return (*pi).second;
  }

  return (TexturePacking *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PTexture::set_changed
//       Access: Public
//  Description: Sets the state of the changed flag.  If this is true,
//               the texture is known to have changed in some way such
//               that files that depend on it will need to be rebuilt.
////////////////////////////////////////////////////////////////////
void PTexture::
set_changed(bool changed) {
  _texture_changed = changed;
}

////////////////////////////////////////////////////////////////////
//     Function: PTexture::matched_anything
//       Access: Public
//  Description: Returns true if the texture matched at least one line
//               in the .txa file, false otherwise.
////////////////////////////////////////////////////////////////////
bool PTexture::
matched_anything() const {
  return _matched_anything;
}

////////////////////////////////////////////////////////////////////
//     Function: PTexture::set_matched_anything
//       Access: Public
//  Description: Sets the state of the matched_anything flag.  See
//               matched_anything().
////////////////////////////////////////////////////////////////////
void PTexture::
set_matched_anything(bool matched_anything) {
  _matched_anything = matched_anything;
}

////////////////////////////////////////////////////////////////////
//     Function: PTexture::is_unused
//       Access: Public
//  Description: Returns true if the texture is not used by any egg
//               file in any palette group, false if it is used by at
//               least one in at least one group.
////////////////////////////////////////////////////////////////////
bool PTexture::
is_unused() const {
  Packing::const_iterator pi;
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    if (!(*pi).second->unused()) {
      return false;
    }
  }

  return true;
}


void PTexture::
write_size(ostream &out) {
  if (!is_unused()) {
    if (!_got_size) {
      read_header();
    }
    out << "  " << _name;
    if (_got_size) {
      out << " orig " << _xsize << " " << _ysize << " " << _zsize;
    }
    if (_got_req) {
      out << " new " << _req_xsize << " " << _req_ysize;
    }
    out << " " << get_scale_pct() << "%\n";
  }
}

void PTexture::
write_pathname(ostream &out) const {
  if (_got_filename && !is_unused()) {
    if (!_file_exists) {
      nout << "Warning: texture " << _filename << " does not exist.\n";
    }

    out << "  " << _name << " " << _attrib_file->write_pi_filename(_filename)
	<< "\n";

    // Also write out all the alternate filenames.
    Filenames::const_iterator fi;
    for (fi = _filenames.begin(); fi != _filenames.end(); ++fi) {
      if ((*fi) != _filename) {
	// Indent by the same number of spaces to line up the filenames.
	for (int i = 0; i < (int)_name.length() + 3; i++) {
	  out << ' ';
	}
	out << _attrib_file->write_pi_filename(*fi) << "\n";
      }
    }
  }
}

PNMImage *PTexture::
read_image() {
  if (!_got_filename || !_file_exists) {
    return NULL;
  }

  PNMImage *image = new PNMImage;
  if (_attrib_file->read_image_file(*image, _filename, Filename())) {
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
      _got_size = read_image_header(_filename, _xsize, _ysize, _zsize);
    }
    _read_header = true;
  }
}

bool PTexture::
read_image_header(const Filename &filename, int &xsize, int &ysize,
		  int &zsize) {
  PNMImageHeader header;
  if (!header.read_header(filename)) {
    nout << "Warning: cannot read texture " << filename << "\n";
    return false;
  }

  xsize = header.get_x_size();
  ysize = header.get_y_size();
  zsize = header.get_num_channels();

  return true;
}
