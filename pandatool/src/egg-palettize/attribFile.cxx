// Filename: attribFile.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "attribFile.h"
#include "userAttribLine.h"
#include "eggPalettize.h"
#include "string_utils.h"
#include "pTexture.h"
#include "palette.h"
#include "sourceEgg.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <map>
#include <set>
#include <fcntl.h>

AttribFile::
AttribFile(const Filename &filename) {
  _name = filename.get_basename_wo_extension();
  _txa_filename = filename;
  _txa_filename.set_extension("txa");
  _pi_filename = filename;
  _pi_filename.set_extension("pi");
  _txa_fd = -1;

  _optimal = false;
  _txa_needs_rewrite = false;

  _palette_prefix = _name + "-palette.";
  _pal_xsize = 512;
  _pal_ysize = 512;
  _default_margin = 2;
  _force_power_2 = false;
  _aggressively_clean_mapdir = false;
}

string AttribFile::
get_name() const {
  return _name;
}

bool AttribFile::
grab_lock() {
  if (!_txa_filename.exists()) {
    nout << "Attributes file " << _txa_filename << " does not exist.\n";
  }

  _txa_fd = open(_txa_filename.c_str(), O_RDWR | O_CREAT, 0666);
  if (_txa_fd < 0) {
    perror(_txa_filename.c_str());
    return false;
  }

  struct flock fl;
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;

  if (fcntl(_txa_fd, F_SETLK, &fl) < 0) {
    nout << "Waiting for lock on " << _txa_filename << "\n";
    while (fcntl(_txa_fd, F_SETLKW, &fl) < 0) {
      if (errno != EINTR) {
	perror(_txa_filename.c_str());
	return false;
      }
    }
  }

  _txa_fstrm.attach(_txa_fd);

  return true;
}

bool AttribFile::
release_lock() {
  // Closing the fstream will close the fd, and thus release all the
  // file locks.
  _txa_fstrm.close();
  _txa_fd = -1;

  return true;
}

bool AttribFile::
read() {
  bool okflag = true;

  okflag = read_txa(_txa_fstrm);

  {
    if (!_pi_filename.exists()) {
      nout << "Palette information file " << _pi_filename << " does not exist.\n";
    } else {
      ifstream infile(_pi_filename.c_str());
      if (!infile) {
	nout << "Palette information file " << _pi_filename << " exists, but cannot be read.\n";
	return false;
      }

      okflag = read_pi(infile);
    }
  }

  return okflag;
}

bool AttribFile::
write() {
  bool okflag = true;

  if (_txa_needs_rewrite) {
    // Rewind and truncate the file for writing.
    _txa_fstrm.clear();
    _txa_fstrm.seekp(0, ios::beg);
    ftruncate(_txa_fd, 0);

    okflag = write_txa(_txa_fstrm) && okflag;
    _txa_fstrm << flush;
  }

  {
    ofstream outfile(_pi_filename.c_str(), ios::out, 0666);
    if (!outfile) {
      nout << "Unable to write file " << _pi_filename << "\n";
      return false;
    }
    
    okflag = write_pi(outfile) && okflag;
  }

  return okflag;
}

void AttribFile::
update_params(EggPalettize *prog) {
  if (prog->_got_map_dirname) {
    _map_dirname = prog->_map_dirname;
  }
  if (prog->_got_palette_size) {
    _pal_xsize = prog->_pal_size[0];
    _pal_ysize = prog->_pal_size[1];
  }
  if (prog->_got_default_margin) {
    _default_margin = prog->_default_margin;
  }
  if (prog->_got_force_power_2) {
    _force_power_2 = prog->_force_power_2;
  }
  if (prog->_got_aggressively_clean_mapdir) {
    _aggressively_clean_mapdir = prog->_aggressively_clean_mapdir;
  }
}

void AttribFile::
get_req_sizes() {
  PTextures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *tex = (*ti).second;
    tex->clear_req();

    int margin = _default_margin;
    UserLines::const_iterator ui;

    bool matched = false;
    for (ui = _user_lines.begin(); 
	 ui != _user_lines.end() && !matched;
	 ++ui) {
      matched = (*ui)->match_texture(tex, margin);
    }

    if (matched) {
      tex->set_matched_anything(true);
    }
  }
}

// Update the unused flags on all textures to accurately reflect
// those that are unused by any egg files.  Omit unused textures
// from the palettizing set.
void AttribFile::
update_texture_flags() {
  // First, clear all the flags.
  PTextures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *tex = (*ti).second;
    tex->set_unused(true);
    tex->set_uses_alpha(false);
  }

  // Then, for each egg file, mark all the textures it's known to be
  // using, and update the repeat and alpha flags.
  Eggs::const_iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    SourceEgg *egg = (*ei).second;
    egg->mark_texture_flags();
  }

  // Now go back through and omit any unused textures.  This is also a
  // fine time to mark the textures' original packing state, so we can
  // check later to see if they've been repacked elsewhere.
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *tex = (*ti).second;
    tex->record_orig_state();

    if (tex->unused()) {
      tex->set_omit(PTexture::OR_unused);
    }
  }
}

// Clear out all the old packing order and start again from the top.
// This should get as nearly optimal a packing as this poor little
// algorithm can manage.
void AttribFile::
repack_all_textures() {
  // First, delete all the existing palettes.
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    // Remove the old palette file?
    if (_aggressively_clean_mapdir && 
	!(*pi)->get_filename().empty()) {
      if (access((*pi)->get_filename().c_str(), F_OK) == 0) {
	nout << "Deleting " << (*pi)->get_filename() << "\n";
	unlink((*pi)->get_filename().c_str());
      }
    }

    delete (*pi);
  }
  _palettes.clear();

  // Reorder the textures in descending order by height and width for
  // optimal packing.
  vector<PTexture *> textures;
  get_eligible_textures(textures);
  
  // Now pack all the textures.  This will create new palettes.
  vector<PTexture *>::iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    pack_texture(*ti);
  }

  _optimal = true;
}

// Add new textures into the palettes without disturbing whatever was
// already there.  This won't generate an optimal palette, but it
// won't require rebuilding every egg file that already uses this
// palette.
void AttribFile::
repack_some_textures() {
  bool empty_before = _palettes.empty();
  bool any_added = false;

  // Reorder the textures in descending order by height and width for
  // optimal packing.
  vector<PTexture *> textures;
  get_eligible_textures(textures);
  
  // Now pack whatever textures are currently unpacked.
  vector<PTexture *>::iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    PTexture *tex = (*ti);
    if (!tex->is_packed()) {
      if (pack_texture(tex)) {
	any_added = true;
      }
    }
  }

  _optimal = (empty_before || !any_added);
}

void AttribFile::
optimal_resize() {
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    (*pi)->optimal_resize();
  }
}

void AttribFile::
finalize_palettes() {
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    (*pi)->finalize_palette();
  }
}

void AttribFile::
remove_unused_lines() {
  UserLines::iterator read, write;

  read = _user_lines.begin();
  write = _user_lines.begin();
  while (read != _user_lines.end()) {
    if ((*read)->was_used()) {
      (*write++) = (*read++);
    } else {
      delete (*read);
      _txa_needs_rewrite = true;
      read++;
    }
  }
  _user_lines.erase(write, _user_lines.end());
}

// Checks that each texture that wants packing has been packed, that
// no textures that don't need packing have been packed, and that all
// textures are packed at their correct sizes.  Returns true if no
// changes need to be made, false otherwise.
bool AttribFile::
check_packing(bool force_optimal) {
  bool all_ok = true;

  PTextures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *texture = (*ti).second;

    if (texture->get_omit() == PTexture::OR_none) {
      // Here's a texture that thinks it wants to be packed.  Does it?
      int xsize, ysize;
      if (!texture->get_req(xsize, ysize)) {
	// If we don't know the texture's size, we can't place it.
	nout << "Warning!  Can't determine size of " << texture->get_name()
	     << "\n";
	texture->set_omit(PTexture::OR_unknown);

      } else if ((xsize > _pal_xsize || ysize > _pal_ysize) ||
		 (xsize == _pal_xsize && ysize == _pal_ysize)) {
	// If the texture is too big for the palette (or exactly fills the
	// palette), we can't place it.
	texture->set_omit(PTexture::OR_size);

      } else {
	// Ok, this texture really does want to be packed.  Is it?
	int px, py, m;
	if (texture->get_packed_size(px, py, m)) {
	  // The texture is packed.  Does it have the right size?
	  if (px != xsize || py != ysize) {
	    // Oops, we'll have to repack it.
	    unpack_texture(texture);
	    _optimal = false;
	    all_ok = false;
	  }
	  if (m != texture->get_margin()) {
	    // The margin has changed, although not the size.  We
	    // won't have to repack it, but we do need to update it.
	    texture->set_changed(true);
	  }
	} else {
	  // The texture isn't packed.  Need to pack it.
	  all_ok = false;
	}
      }
    }

    if (texture->get_omit() != PTexture::OR_none) {
      // Here's a texture that doesn't want to be packed.  Is it?
      if (unpack_texture(texture)) {
	// It was!  Not any more.
	_optimal = false;
	all_ok = false;
      }
    }
  }

  if (force_optimal && !_optimal) {
    // If the user wants to insist on an optimal packing, we'll have
    // to give it to him.
    all_ok = false;
  }

  return all_ok;
}


bool AttribFile::
pack_texture(PTexture *texture) {
  // Now try to place it in each of our existing palettes.
  Palettes::iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    if ((*pi)->pack_texture(texture)) {
      return true;
    }
  }

  // It didn't place anywhere; create a new palette for it.
  Palette *palette = 
    new Palette(_palettes.size() + 1, _pal_xsize, _pal_ysize, 0, this);
  if (!palette->pack_texture(texture)) {
    // Hmm, it didn't fit on an empty palette.  Must be too big.
    texture->set_omit(PTexture::OR_size);
    delete palette;
    return false;
  }
  _palettes.push_back(palette);

  return true;
}

bool AttribFile::
unpack_texture(PTexture *texture) {
  if (texture->is_packed()) {
    bool unpacked = texture->get_palette()->unpack_texture(texture);
    assert(unpacked);
    return true;
  }

  // It wasn't packed.
  return false;
}

// Updates the timestamp on each egg file that will need to be
// rebuilt, so that a future make process will pick it up.  This is
// only necessary to update egg files that may not have been included
// on the command line, and which we don't have direct access to.
void AttribFile::
touch_dirty_egg_files(bool force_redo_all,
		      bool eggs_include_images) {
  Eggs::iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    SourceEgg *egg = (*ei).second;

    if (egg->needs_rebuild(force_redo_all, eggs_include_images)) {
      Filename filename = egg->get_egg_filename();
      filename.set_extension("pt");
      nout << "Touching " << filename << "\n";
      if (!filename.touch()) {
	nout << "unable to touch " << filename << "\n";
      }
    }
  }
}


PTexture *AttribFile::
get_texture(const string &name) {
  PTextures::iterator ti;
  ti = _textures.find(name);
  if (ti != _textures.end()) {
    return (*ti).second;
  }

  PTexture *texture = new PTexture(this, name);
  _textures[name] = texture;
  return texture;
}

void AttribFile::
get_eligible_textures(vector<PTexture *> &textures) {
  // First, copy the texture pointers into this map structure to sort
  // them in descending order by size.  This is a 2-d map such that
  // each map[ysize][xsize] is a set of texture pointers.
  typedef map<int, map<int, set<PTexture *> > > TexBySize;
  TexBySize tex_by_size;
  int num_textures = 0;

  PTextures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *texture = (*ti).second;

    if (texture->get_omit() == PTexture::OR_none) {
      int xsize, ysize;
      if (texture->get_req(xsize, ysize)) {
	tex_by_size[-ysize][-xsize].insert(texture);
	num_textures++;
      }
    }
  }

  // Now walk through this map and get out our textures, nicely sorted
  // in descending order by height and width.
  textures.clear();
  textures.reserve(num_textures);

  TexBySize::const_iterator t1;
  for (t1 = tex_by_size.begin(); t1 != tex_by_size.end(); ++t1) {
    map<int, set<PTexture *> >::const_iterator t2;
    for (t2 = (*t1).second.begin(); t2 != (*t1).second.end(); ++t2) {
      set<PTexture *>::const_iterator t3;
      for (t3 = (*t2).second.begin(); t3 != (*t2).second.end(); ++t3) {
	textures.push_back(*t3);
      }
    }
  }
}

SourceEgg *AttribFile::
get_egg(Filename name) {
  Eggs::iterator ei;
  ei = _eggs.find(name);
  if (ei != _eggs.end()) {
    return (*ei).second;
  }

  SourceEgg *egg = new SourceEgg();
  egg->resolve_egg_filename(name);
  egg->set_egg_filename(name);
  _eggs[name] = egg;
  return egg;
}

bool AttribFile::
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

bool AttribFile::
transfer_unplaced_images(bool force_redo_all) {
  bool okflag = true;

  PTextures::iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *texture = (*ti).second;

    if (texture->get_omit() != PTexture::OR_none &&
	texture->get_omit() != PTexture::OR_unused) {
      // Here's a texture that needs to be moved to our mapdir.  But
      // maybe it's already there and hasn't changed recently.
      if (force_redo_all || texture->needs_refresh()) {
	// Nope, needs to be updated.
	okflag = texture->transfer() && okflag;
      }
    } else {
      if (_aggressively_clean_mapdir) {
	if (access(texture->get_filename().c_str(), F_OK) == 0) {
	  nout << "Deleting " << texture->get_filename() << "\n";
	  unlink(texture->get_filename().c_str());
	}
      }
    }
  }

  return okflag;
}


void AttribFile::
check_dup_textures(map<string, PTexture *> &textures,
		   map<string, int> &dup_textures) const {
  PTextures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *texture = (*ti).second;
    string name = texture->get_name();
      
    map<string, PTexture *>::iterator mi = textures.find(name);
    if (mi == textures.end()) {
      // This texture hasn't been used yet.
      textures[name] = texture;
      
    } else {
      // This texture has already been used in another palette.  The
      // smaller of the two is considered wasted space.
      PTexture *other = (*mi).second;
      
      if (!other->is_really_packed() && !texture->is_really_packed()) {
	// No, neither one is packed, so it's not wasted space.
      
      } else {
	int txsize, tysize;
	int oxsize, oysize;
	int wasted_size = 0;
      
	if (other->is_really_packed() != texture->is_really_packed()) {
	  // If one texture is packed and the other isn't, the packed
	  // one is considered wasted space.
	  if (other->is_really_packed()) {
	    if (other->get_req(oxsize, oysize)) {
	      wasted_size = oxsize * oysize;
	    }
	    (*mi).second = texture;
	  } else {
	    if (texture->get_req(txsize, tysize)) {
	      wasted_size = txsize * tysize;
	    }
	  }

	} else {
	  // Both textures are packed.  The smaller one is considered
	  // wasted space.
	  assert(other->is_really_packed() && texture->is_really_packed());

	  if (texture->get_req(txsize, tysize) && 
	      other->get_req(oxsize, oysize)) {
	    if (txsize * tysize <= oxsize * oysize) {
	      wasted_size = txsize * tysize;
	    } else {
	      wasted_size = oxsize * oysize;
	      (*mi).second = texture;
	    }
	  }
	}
	
	// Now update the wasted space total for this texture.
	map<string, int>::iterator di = dup_textures.find(name);
	if (di != dup_textures.end()) {
	  (*di).second += wasted_size;
	} else {
	  dup_textures[name] = wasted_size;
	}
      }
    }
  }
}

void AttribFile::
collect_statistics(int &num_textures, int &num_placed, int &num_palettes,
		   int &orig_size, int &resized_size, 
		   int &palette_size, int &unplaced_size) const {
  num_textures = _textures.size();
  num_palettes = _palettes.size();
  num_placed = 0;
  orig_size = 0;
  resized_size = 0;
  palette_size = 0;
  unplaced_size = 0;

  PTextures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *texture = (*ti).second;

    int xsize, ysize;
    int rxsize, rysize;
    int rsize = 0;
    if (texture->get_size(xsize, ysize) && 
	texture->get_last_req(rxsize, rysize)) {
      orig_size += xsize * ysize;
      resized_size += rxsize * rysize;
      rsize = rxsize * rysize;
    }
    
    if (texture->is_really_packed()) {
      num_placed++;
    } else {
      unplaced_size += rsize;
    }
  }

  Palettes::const_iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    Palette *palette = (*pi);
    if (palette->get_num_textures() > 1) {
      int xsize, ysize;
      palette->get_size(xsize, ysize);
      palette_size += xsize * ysize;
    }
  }    
}  


bool AttribFile::
read_txa(istream &infile) {
  string line;

  getline(infile, line);
  int line_num = 1;

  while (!infile.eof()) {
    UserAttribLine *ul = new UserAttribLine(line, this);
    if (!ul->is_valid()) {
      nout << "Error at line " << line_num << " of " << _txa_filename << "\n";
      return false;
    }
    if (ul->is_old_style()) {
      _txa_needs_rewrite = true;
    }
    _user_lines.push_back(ul);

    getline(infile, line);
    line_num++;
  }
  return true;
}

bool AttribFile::
read_pi(istream &infile) {
  string line;

  getline(infile, line);
  int line_num = 1;

  while (!infile.eof()) {
    // First, strip off the comment.
    if (!line.empty()) {
      if (line[0] == '#') {
	line = "";
      } else {
	size_t pos = line.find(" #");
	if (pos != string::npos) {
	  line = line.substr(0, pos - 1);
	}
      }
    }

    vector<string> words = extract_words(line);
    bool okflag = true;

    if (words.empty()) {
      getline(infile, line);
      line_num++;

    } else if (words[0] == "params") {
      okflag = parse_params(words, infile, line, line_num);

    } else if (words[0] == "packing") {
      okflag = parse_packing(words, infile, line, line_num);

    } else if (words[0] == "textures") {
      okflag = parse_texture(words, infile, line, line_num);

    } else if (words[0] == "pathnames") {
      okflag = parse_pathname(words, infile, line, line_num);

    } else if (words[0] == "egg") {
      okflag = parse_egg(words, infile, line, line_num);

    } else if (words[0] == "palette") {
      okflag = parse_palette(words, infile, line, line_num);

    } else if (words[0] == "unplaced") {
      okflag = parse_unplaced(words, infile, line, line_num);

    } else if (words[0] == "surprises") {
      okflag = parse_surprises(words, infile, line, line_num);

    } else {
      nout << "Invalid keyword: " << words[0] << "\n";
      okflag = false;
    }

    if (!okflag) {
      nout << "Error at line " << line_num << " of " << _pi_filename << "\n";
      return false;
    }
  }

  return true;
}

bool AttribFile::
write_txa(ostream &out) const {
  UserLines::const_iterator ui;

  for (ui = _user_lines.begin(); ui != _user_lines.end(); ++ui) {
    (*ui)->write(out);
  }

  if (!out) {
    nout << "I/O error when writing to " << _txa_filename << "\n";
    return false;
  }
  return true;
}

bool AttribFile::
write_pi(ostream &out) const {
  out << 
    "# This file was generated by egg-palettize.  Edit it at your own peril.\n";

  out << "\nparams\n"
      << "  map_directory " << _map_dirname << "\n"
      << "  pal_xsize " << _pal_xsize << "\n"
      << "  pal_ysize " << _pal_ysize << "\n"
      << "  default_margin " << _default_margin << "\n"
      << "  force_power_2 " << _force_power_2 << "\n"
      << "  aggressively_clean_mapdir " << _aggressively_clean_mapdir << "\n";

  if (_optimal) {
    out << "\npacking is optimal\n";
    // Well, as nearly as this program can do it, anyway.
  } else {
    out << "\npacking is suboptimal\n";
  }

  out << "\npathnames\n";
  PTextures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    (*ti).second->write_pathname(out);
  }

  Eggs::const_iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    out << "\n";
    (*ei).second->write_pi(out);
  }

  Palettes::const_iterator pi;
  for (pi = _palettes.begin(); pi != _palettes.end(); ++pi) {
    out << "\n";
    (*pi)->write(out);
  }

  out << "\n";
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    (*ti).second->write_unplaced(out);
  }

  // Sort textures in descending order by scale percent.
  typedef multimap<double, PTexture *> SortPTextures;
  SortPTextures sort_textures;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *texture = (*ti).second;
    sort_textures.insert(SortPTextures::value_type(-texture->get_scale_pct(),
						  texture));
  }

  bool any_surprises = false;

  out << "\ntextures\n";
  SortPTextures::const_iterator sti;
  for (sti = sort_textures.begin(); sti != sort_textures.end(); ++sti) {
    PTexture *texture = (*sti).second;
    texture->write_size(out);
    any_surprises = any_surprises || !texture->matched_anything();
  }

  if (any_surprises) {
    // Some textures didn't match any commands; they're "surprise"
    // textures.
    out << "\nsurprises\n";
    for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
      PTexture *texture = (*ti).second;
      if (!texture->matched_anything()) {
	out << "  " << texture->get_name() << "\n";
      }
    }
  }

  if (!out) {
    nout << "I/O error when writing to " << _pi_filename << "\n";
    return false;
  }

  return true;
}

bool AttribFile::
parse_params(const vector<string> &words, istream &infile, 
	     string &line, int &line_num) {
  if (words.size() != 1) {
    nout << "Unexpected keywords on line.\n";
    return false;
  }

  getline(infile, line);
  line = trim_right(line);
  line_num++;
  while (!infile.eof() && !line.empty() && isspace(line[0])) {
    string param, value;
    extract_param_value(line, param, value);

    if (param == "map_directory") {
      _map_dirname = value;

      // These are all deprecated.
    } else if (param == "converted_directory") {
    } else if (param == "convert_extension") {
    } else if (param == "convert_command") {
    } else if (param == "palette_prefix") {
    } else if (param == "map_prefix") {

    } else if (param == "pal_xsize") {
      _pal_xsize = atoi(value.c_str());
    } else if (param == "pal_ysize") {
      _pal_ysize = atoi(value.c_str());
    } else if (param == "default_margin") {
      _default_margin = atoi(value.c_str());
    } else if (param == "force_power_2") {
      _force_power_2 = atoi(value.c_str());
    } else if (param == "aggressively_clean_mapdir") {
      _aggressively_clean_mapdir = atoi(value.c_str());
    } else {
      nout << "Unexpected keyword: " << param << "\n";
      return false;
    }

    getline(infile, line);
    line = trim_right(line);
    line_num++;
  }

  return true;
}

bool AttribFile::
parse_packing(const vector<string> &words, istream &infile, 
	      string &line, int &line_num) {
  if (!(words.size() == 3 && words[1] == "is" &&
	(words[2] == "optimal" || words[2] == "suboptimal"))) {
    nout << "Expected 'packing is {optimal|suboptimal}'\n";
    return false;
  }

  _optimal = (words[2] == "optimal");

  getline(infile, line);
  line_num++;
  return true;
}


bool AttribFile::
parse_texture(const vector<string> &words, istream &infile, 
	      string &line, int &line_num) {
  if (words.size() != 1) {
    nout << "Unexpected words on line.\n";
    return false;
  }
  
  getline(infile, line);
  line = trim_right(line);
  line_num++;
  while (!infile.eof() && !line.empty() && isspace(line[0])) {
    vector<string> twords = extract_words(line);
    if (twords.size() < 1) {
      nout << "Expected texture name and additional parameters.\n";
      return false;
    }
    PTexture *texture = get_texture(twords[0]);

    int kw = 1;
    while (kw < (int)twords.size()) {
      if (kw + 3 <= (int)twords.size() && twords[kw] == "orig") {
	texture->set_size(atoi(twords[kw + 1].c_str()),
			  atoi(twords[kw + 2].c_str()));
	kw += 3;

      } else if (kw + 3 <= (int)twords.size() && twords[kw] == "new") {
	texture->set_last_req(atoi(twords[kw + 1].c_str()),
			      atoi(twords[kw + 2].c_str()));
	kw += 3;

      } else if (twords[kw].find('%') != string::npos) {
	// Ignore scale percentage.
	kw++;

      } else {
	nout << "Unexpected keyword: " << twords[kw] << "\n";
      }
    }

    getline(infile, line);
    line = trim_right(line);
    line_num++;
  }

  return true;
}

bool AttribFile::
parse_pathname(const vector<string> &words, istream &infile, 
	       string &line, int &line_num) {
  if (words.size() != 1) {
    nout << "Unexpected words on line.\n";
    return false;
  }
  
  getline(infile, line);
  line = trim_right(line);
  line_num++;
  PTexture *texture = NULL;

  while (!infile.eof() && !line.empty() && isspace(line[0])) {
    vector<string> twords = extract_words(line);
    if (twords.size() == 1) {
      // Only one word on the line means it's an alternate filename
      // for the previous texture.
      if (texture == NULL) {
	nout << "Expected texture name and pathname.\n";
	return false;
      }
      texture->add_filename(twords[0]);

    } else if (twords.size() == 2) {
      // Two words on the line means it's a texture name and filename.
      texture = get_texture(twords[0]);
      texture->add_filename(twords[1]);

    } else {
      // Anything else is a mistake.
      nout << "Expected texture name and pathname.\n";
      return false;
    }

    getline(infile, line);
    line = trim_right(line);
    line_num++;
  }

  return true;
}

bool AttribFile::
parse_egg(const vector<string> &words, istream &infile, 
	  string &line, int &line_num) {
  if (words.size() != 2) {
    nout << "Egg filename expected.\n";
    return false;
  }
  
  SourceEgg *egg = get_egg(words[1]);

  getline(infile, line);
  line = trim_right(line);
  line_num++;
  while (!infile.eof() && !line.empty() && isspace(line[0])) {
    vector<string> twords = extract_words(line);
    if (twords.size() < 1) {
      nout << "Expected texture name\n";
      return false;
    }

    string name = twords[0];
    bool repeats = false;
    bool alpha = false;

    int kw = 1;
    while (kw < (int)twords.size()) {
      if (twords[kw] == "repeats") {
	repeats = true;
	kw++;

      } else if (twords[kw] == "alpha") {
	alpha = true;
	kw++;

      } else {
	nout << "Unexpected keyword " << twords[kw] << "\n";
	return false;
      }
    }

    egg->add_texture(get_texture(name), repeats, alpha);

    getline(infile, line);
    line = trim_right(line);
    line_num++;
  }

  return true;
}
  


bool AttribFile::
parse_palette(const vector<string> &words, istream &infile, 
	      string &line, int &line_num) {
  if (words.size() != 6) {
    nout << "Palette filename, size, and number of components expected.\n";
    return false;
  }

  string filename = words[1];
  if (!(words[2] == "size")) {
    nout << "Expected keyword 'size'\n";
    return false;
  }
  int xsize = atoi(words[3].c_str());
  int ysize = atoi(words[4].c_str());
  int components = atoi(words[5].c_str());

  Palette *palette = new Palette(filename, xsize, ysize, components, this);
  _palettes.push_back(palette);

  getline(infile, line);
  line = trim_right(line);
  line_num++;
  while (!infile.eof() && !line.empty() && isspace(line[0])) {
    vector<string> twords = extract_words(line);
    if (twords.size() != 9) {
      nout << "Expected texture placement line.\n";
      return false;
    }

    PTexture *texture = get_texture(twords[0]);
    
    if (!(twords[1] == "at")) {
      nout << "Expected keyword 'at'\n";
      return false;
    }
    int left = atoi(twords[2].c_str());
    int top = atoi(twords[3].c_str());
    
    if (!(twords[4] == "size")) {
      nout << "Expected keyword 'size'\n";
      return false;
    }
    int xsize = atoi(twords[5].c_str());
    int ysize = atoi(twords[6].c_str());
    
    if (!(twords[7] == "margin")) {
      nout << "Expected keyword 'margin'\n";
      return false;
    }
    int margin = atoi(twords[8].c_str());

    palette->place_texture_at(texture, left, top, xsize, ysize, margin);

    getline(infile, line);
    line = trim_right(line);
    line_num++;
  }

  return true;
}


  
bool AttribFile::
parse_unplaced(const vector<string> &words, istream &infile, 
	       string &line, int &line_num) {
  if (words.size() != 4) {
    nout << "Unplaced texture description expected.\n";
    return false;
  }

  PTexture *texture = get_texture(words[1]);

  if (!(words[2] == "because")) {
    nout << "Expected keyword 'because'\n";
    return false;
  }
  
  if (words[3] == "size") {
    texture->set_omit(PTexture::OR_size);
  } else if (words[3] == "repeats") {
    texture->set_omit(PTexture::OR_repeats);
  } else if (words[3] == "omitted") {
    texture->set_omit(PTexture::OR_omitted);
  } else if (words[3] == "unused") {
    texture->set_omit(PTexture::OR_unused);
  } else if (words[3] == "unknown") {
    texture->set_omit(PTexture::OR_unknown);
  } else if (words[3] == "solitary") {
    texture->set_omit(PTexture::OR_solitary);
  } else if (words[3] == "cmdline") {
    texture->set_omit(PTexture::OR_cmdline);
  } else {
    nout << "Unknown keyword " << words[3] << "\n";
    return false;
  }

  getline(infile, line);
  line_num++;
  return true;
}

bool AttribFile::
parse_surprises(const vector<string> &words, istream &infile, 
		string &line, int &line_num) {
  if (words.size() != 1) {
    nout << "Unexpected words on line.\n";
    return false;
  }

  // This is just the list of surprise textures from last time.  Its
  // only purpose is to inform the user; we can completely ignore it.
  
  getline(infile, line);
  line = trim_right(line);
  line_num++;
  while (!infile.eof() && !line.empty() && isspace(line[0])) {
    getline(infile, line);
    line = trim_right(line);
    line_num++;
  }

  return true;
}
