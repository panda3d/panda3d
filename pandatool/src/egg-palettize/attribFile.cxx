// Filename: attribFile.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "attribFile.h"
#include "userAttribLine.h"
#include "eggPalettize.h"
#include "string_utils.h"
#include "pTexture.h"
#include "texturePacking.h"
#include "paletteGroup.h"
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

  _default_group = (PaletteGroup *)NULL;

  _optimal = false;
  _txa_needs_rewrite = false;

  _map_dirname = "%s";
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
open_and_lock(bool lock) {
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

  if (lock) {
    if (fcntl(_txa_fd, F_SETLK, &fl) < 0) {
      nout << "Waiting for lock on " << _txa_filename << "\n";
      while (fcntl(_txa_fd, F_SETLKW, &fl) < 0) {
	if (errno != EINTR) {
	  perror(_txa_filename.c_str());
	  return false;
	}
      }
    }
  }

  _txa_fstrm.attach(_txa_fd);

  return true;
}

bool AttribFile::
close_and_unlock() {
  // Closing the fstream will close the fd, and thus release all the
  // file locks.
  _txa_fstrm.close();
  _txa_fd = -1;

  return true;
}

bool AttribFile::
read(bool force_redo_all) {
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

      okflag = read_pi(infile, force_redo_all);
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

////////////////////////////////////////////////////////////////////
//     Function: AttribFile::get_group
//       Access: Public
//  Description: Returns a pointer to the PaletteGroup object with the
//               given name.  If there is not yet any such
//               PaletteGroup, creates one.
////////////////////////////////////////////////////////////////////
PaletteGroup *AttribFile::
get_group(const string &group_name) {
  Groups::iterator gi;
  gi = _groups.find(group_name);
  if (gi != _groups.end()) {
    return (*gi).second;
  }

  PaletteGroup *new_group = new PaletteGroup(group_name);
  _groups[group_name] = new_group;
  return new_group;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribFile::set_default_group
//       Access: Public
//  Description: Sets the PaletteGroup that should be associated
//               with any textures or egg files not explicitly placed
//               in a different group.
////////////////////////////////////////////////////////////////////
void AttribFile::
set_default_group(PaletteGroup *default_group) {
  _default_group = default_group;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribFile::get_default_group
//       Access: Public
//  Description: Returns the PaletteGroup that should be associated
//               with any textures or egg files not explicitly placed
//               in a different group.
////////////////////////////////////////////////////////////////////
PaletteGroup *AttribFile::
get_default_group() {
  if (_default_group == (PaletteGroup *)NULL) {
    _default_group = get_group(_name);
  }
  return _default_group;
}

////////////////////////////////////////////////////////////////////
//     Function: AttribFile::get_egg_group_requests
//       Access: Public
//  Description: Checks all of the known egg filenames against the egg
//               files named in the .txa file, looking to see which
//               egg files as assigned to which groups.
////////////////////////////////////////////////////////////////////
void AttribFile::
get_egg_group_requests() {
  Eggs::iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    SourceEgg *egg = (*ei).second;
    UserLines::const_iterator ui;

    bool matched = false;
    for (ui = _user_lines.begin(); 
	 ui != _user_lines.end() && !matched;
	 ++ui) {
      matched = (*ui)->get_group_request(egg);
    }

    if (matched) {
      egg->set_matched_anything(true);
    }
  }    

  // Now go back and make sure that all textures are assigned to
  // *something*.
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    SourceEgg *egg = (*ei).second;
    egg->all_textures_assigned();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AttribFile::get_size_requests
//       Access: Public
//  Description: Determines the size/scaling requested for each
//               texture by scanning the .txa file.
////////////////////////////////////////////////////////////////////
void AttribFile::
get_size_requests() {
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
      matched = (*ui)->get_size_request(tex, margin);
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
  Packing::iterator pi;
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    TexturePacking *packing = (*pi);
    packing->set_unused(true);
    packing->set_uses_alpha(false);
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
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    TexturePacking *packing = (*pi);
    packing->record_orig_state();

    if (packing->unused()) {
      packing->set_omit(OR_unused);
    }
  }
}

// Clear out all the old packing order and start again from the top.
// This should get as nearly optimal a packing as this poor little
// algorithm can manage.
void AttribFile::
repack_all_textures() {
  // First, empty all the existing palette groups.
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    if (_aggressively_clean_mapdir) {
      group->remove_palette_files();
    }
    group->clear_palettes();
  }

  // Reorder the textures in descending order by height and width for
  // optimal packing.
  vector<TexturePacking *> textures;
  get_eligible_textures(textures);
  
  // Now pack all the textures.  This will create new palettes.
  vector<TexturePacking *>::iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    (*ti)->pack();
  }

  _optimal = true;
}

// Add new textures into the palettes without disturbing whatever was
// already there.  This won't generate an optimal palette, but it
// won't require rebuilding every egg file that already uses this
// palette.
void AttribFile::
repack_some_textures() {
  bool empty_before = _groups.empty();
  bool any_added = false;

  // Reorder the textures in descending order by height and width for
  // optimal packing.
  vector<TexturePacking *> textures;
  get_eligible_textures(textures);
  
  // Now pack whatever textures are currently unpacked.
  vector<TexturePacking *>::iterator ti;
  for (ti = textures.begin(); ti != textures.end(); ++ti) {
    TexturePacking *packing = (*ti);
    if (packing->pack()) {
      any_added = true;
    }
  }

  _optimal = (empty_before || !any_added);
}

void AttribFile::
optimal_resize() {
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    (*gi).second->optimal_resize();
  }
}

void AttribFile::
finalize_palettes() {
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    (*gi).second->finalize_palettes();
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

////////////////////////////////////////////////////////////////////
//     Function: AttribFile::prepare_repack
//       Access: Public
//  Description: Checks if any texture needs to be repacked into a
//               different location on the palette (for instance,
//               because it has changed size).  If so, unpacks it and
//               returns true; otherwise, leaves it alone and returns
//               false.
//
//               If force_optimal is true, returns true if anything
//               has changed at all that would result in a suboptimal
//               palette.
////////////////////////////////////////////////////////////////////
bool AttribFile::
prepare_repack(bool force_optimal) {
  bool needs_repack = false;

  Packing::iterator pi;
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    TexturePacking *packing = (*pi);
    if (packing->prepare_repack(_optimal)) {
      needs_repack = true;
    }
  }

  if (force_optimal && !_optimal) {
    // If the user wants to insist on an optimal packing, we'll have
    // to give it to him.
    needs_repack = true;
  }

  return needs_repack;
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
get_eligible_textures(vector<TexturePacking *> &textures) {
  // First, copy the texture pointers into this map structure to sort
  // them in descending order by size.  This is a 2-d map such that
  // each map[ysize][xsize] is a set of texture pointers.
  typedef map<int, map<int, set<TexturePacking *> > > TexBySize;
  TexBySize tex_by_size;
  int num_textures = 0;

  Packing::iterator pi;
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    TexturePacking *packing = (*pi);
    PTexture *texture = packing->get_texture();

    if (packing->get_omit() == OR_none) {
      int xsize, ysize;
      if (texture->get_req(xsize, ysize)) {
	tex_by_size[-ysize][-xsize].insert(packing);
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
    map<int, set<TexturePacking *> >::const_iterator t2;
    for (t2 = (*t1).second.begin(); t2 != (*t1).second.end(); ++t2) {
      set<TexturePacking *>::const_iterator t3;
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

  SourceEgg *egg = new SourceEgg(this);
  egg->resolve_egg_filename(name);
  egg->set_egg_filename(name);
  _eggs[name] = egg;
  return egg;
}

bool AttribFile::
generate_palette_images() {
  bool okflag = true;

  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    PaletteGroup *group = (*gi).second;
    okflag = group->generate_palette_images() && okflag;
  }

  return okflag;
}

bool AttribFile::
transfer_unplaced_images(bool force_redo_all) {
  bool okflag = true;

  Packing::iterator pi;
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    TexturePacking *packing = (*pi);
    PTexture *texture = packing->get_texture();

    if (packing->get_omit() != OR_none &&
	packing->get_omit() != OR_unused) {
      // Here's a texture that needs to be moved to our mapdir.  But
      // maybe it's already there and hasn't changed recently.
      if (force_redo_all || packing->needs_refresh()) {
	// Nope, needs to be updated.
	okflag = packing->transfer() && okflag;
      }
    } else {
      if (_aggressively_clean_mapdir && texture->is_unused()) {
	Filename new_filename = packing->get_new_filename();
	if (new_filename.exists()) {
	  nout << "Deleting " << new_filename << "\n";
	  new_filename.unlink();
	}
      }
    }
  }

  return okflag;
}


void AttribFile::
check_dup_textures(map<string, PTexture *> &textures,
		   map<string, int> &dup_textures) const {
  /*
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
  */
}

void AttribFile::
collect_statistics(int &num_textures, int &num_placed, int &num_palettes,
		   int &orig_size, int &resized_size, 
		   int &palette_size, int &unplaced_size) const {
  /*
  num_textures = _textures.size();
  num_palettes = 0; //_palettes.size();
  num_placed = 0;
  orig_size = 0;
  resized_size = 0;
  palette_size = 0;
  unplaced_size = 0;

  PTextures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *texture = (*ti).second;

    int xsize, ysize, zsize;
    int rxsize, rysize;
    int rsize = 0;
    if (texture->get_size(xsize, ysize, zsize) && 
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
      int xsize, ysize, zsize;
      palette->get_size(xsize, ysize, zsize);
      palette_size += xsize * ysize;
    }
  } 
  */
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
read_pi(istream &infile, bool force_redo_all) {
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
      okflag = parse_egg(words, infile, line, line_num, force_redo_all);

    } else if (words[0] == "group") {
      okflag = parse_group(words, infile, line, line_num);

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
  bool any_surprises = false;

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
    SourceEgg *egg = (*ei).second;
    out << "\n";
    egg->write_pi(out);
    any_surprises = any_surprises || !egg->matched_anything();
  }

  out << "\n";
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    (*gi).second->write_pi(out);
  }
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    (*gi).second->write_palettes_pi(out);
  }

  out << "\n";
  Packing::const_iterator pi;
  for (pi = _packing.begin(); pi != _packing.end(); ++pi) {
    (*pi)->write_unplaced(out);
  }

  // Sort textures in descending order by scale percent.
  typedef multimap<double, PTexture *> SortPTextures;
  SortPTextures sort_textures;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    PTexture *texture = (*ti).second;
    sort_textures.insert(SortPTextures::value_type(-texture->get_scale_pct(),
						  texture));
  }

  out << "\ntextures\n";
  SortPTextures::const_iterator sti;
  for (sti = sort_textures.begin(); sti != sort_textures.end(); ++sti) {
    PTexture *texture = (*sti).second;
    texture->write_size(out);
    any_surprises = any_surprises || !texture->matched_anything();
  }

  if (any_surprises) {
    // Some textures or egg files didn't match any commands; they're
    // "surprises".
    out << "\nsurprises\n";
    for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
      PTexture *texture = (*ti).second;
      if (!texture->matched_anything()) {
	out << "  " << texture->get_name() << "\n";
      }
    }
    Eggs::const_iterator ei;
    for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
      SourceEgg *egg = (*ei).second;
      if (!egg->matched_anything()) {
	out << "  " << egg->get_egg_filename().get_basename() << "\n";
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
			  atoi(twords[kw + 2].c_str()),
			  atoi(twords[kw + 3].c_str()));
	kw += 4;

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
	  string &line, int &line_num, bool force_redo_all) {
  if (words.size() < 2) {
    nout << "Egg filename expected.\n";
    return false;
  }
  
  SourceEgg *egg = get_egg(words[1]);

  if (words.size() > 2 && words[2] == "in") {
    // Get the group names.
    for (int i = 3; i < (int)words.size(); i++) {
      egg->add_group(get_group(words[i]));
    }
  }

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
    PaletteGroup *group = (PaletteGroup *)NULL;

    int kw = 1;
    while (kw < (int)twords.size()) {
      if (twords[kw] == "in") {
	kw++;
	if (kw >= (int)twords.size()) {
	  nout << "Expected group name\n";
	  return false;
	}
	group = get_group(twords[kw]);
	kw++;

      } else if (twords[kw] == "repeats") {
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

    PTexture *texture = get_texture(name);
    TexturePacking *packing = (TexturePacking *)NULL;

    if (!force_redo_all) {
      packing = texture->add_to_group(group);
    }

    egg->add_texture(texture, packing, repeats, alpha);

    getline(infile, line);
    line = trim_right(line);
    line_num++;
  }

  return true;
}
  

bool AttribFile::
parse_group(const vector<string> &words, istream &infile, 
	    string &line, int &line_num) {
  if (words.size() == 2) {
    // Just a group name by itself; ignore it.
    return true;
  }

  if (words.size() != 4) {
    nout << "Group dirname expected.\n";
    return false;
  }

  if (!(words[2] == "dir")) {
    nout << "Expected keyword 'dir'\n";
    return false;
  }
  PaletteGroup *group = get_group(words[1]);

  group->set_dirname(words[3]);

  getline(infile, line);
  line_num++;
  return true;
}

bool AttribFile::
parse_palette(const vector<string> &words, istream &infile, 
	      string &line, int &line_num) {
  if (words.size() != 8) {
    nout << "Palette filename, group, size, and number of components expected.\n";
    return false;
  }

  string filename = words[1];
  if (!(words[2] == "in")) {
    nout << "Expected keyword 'in'\n";
    return false;
  }
  PaletteGroup *group = get_group(words[3]);

  if (!(words[4] == "size")) {
    nout << "Expected keyword 'size'\n";
    return false;
  }
  int xsize = atoi(words[5].c_str());
  int ysize = atoi(words[6].c_str());
  int components = atoi(words[7].c_str());

  Palette *palette = 
    new Palette(filename, group, xsize, ysize, components, this);
  group->add_palette(palette);

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
    TexturePacking *packing = texture->add_to_group(group);
    
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
    
    palette->place_texture_at(packing, left, top, xsize, ysize, margin);

    getline(infile, line);
    line = trim_right(line);
    line_num++;
  }

  return true;
}


  
bool AttribFile::
parse_unplaced(const vector<string> &words, istream &infile, 
	       string &line, int &line_num) {
  if (words.size() != 6) {
    nout << "Unplaced texture description expected.\n";
    return false;
  }

  PTexture *texture = get_texture(words[1]);

  if (!(words[2] == "in")) {
    nout << "Expected keyword 'in'\n";
    return false;
  }

  PaletteGroup *group = get_group(words[3]);
  TexturePacking *packing = texture->add_to_group(group);

  if (!(words[4] == "because")) {
    nout << "Expected keyword 'because'\n";
    return false;
  }
  
  if (words[5] == "size") {
    packing->set_omit(OR_size);
  } else if (words[5] == "repeats") {
    packing->set_omit(OR_repeats);
  } else if (words[5] == "omitted") {
    packing->set_omit(OR_omitted);
  } else if (words[5] == "unused") {
    packing->set_omit(OR_unused);
  } else if (words[5] == "unknown") {
    packing->set_omit(OR_unknown);
  } else if (words[5] == "solitary") {
    packing->set_omit(OR_solitary);
  } else {
    nout << "Unknown keyword " << words[5] << "\n";
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
