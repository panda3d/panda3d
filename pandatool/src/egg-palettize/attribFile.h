// Filename: attribFile.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef ATTRIBFILE_H
#define ATTRIBFILE_H

#include <pandatoolbase.h>

#include <filename.h>

#include <map>
#include <vector>

class UserAttribLine;
class PTexture;
class TexturePacking;
class PaletteGroup;
class Palette;
class SourceEgg;
class EggPalettize;

////////////////////////////////////////////////////////////////////
// 	 Class : AttribFile
// Description : 
////////////////////////////////////////////////////////////////////
class AttribFile {
public:
  AttribFile(const Filename &filename);

  string get_name() const;

  bool open_and_lock(bool lock);
  bool close_and_unlock();

  bool read(bool force_redo_all);
  bool write();

  void update_params(EggPalettize *prog);

  PaletteGroup *get_group(const string &group_name);
  void set_default_group(PaletteGroup *default_group);
  PaletteGroup *get_default_group();

  void get_egg_group_requests();
  void get_size_requests();
  void update_texture_flags();

  void repack_all_textures();
  void repack_some_textures();
  void optimal_resize();
  void finalize_palettes();
  void remove_unused_lines();
  bool prepare_repack(bool force_optimal);

  void touch_dirty_egg_files(bool force_redo_all,
			     bool eggs_include_images);

  PTexture *get_texture(const string &name);
  void get_eligible_textures(vector<TexturePacking *> &textures);
  SourceEgg *get_egg(Filename name);

  bool generate_palette_images();
  bool transfer_unplaced_images(bool force_redo_all);

  void check_dup_textures(map<string, PTexture *> &textures,
			  map<string, int> &dup_textures) const;

  void collect_statistics(int &num_textures, int &num_placed,
			  int &num_palettes,
			  int &orig_size, int &resized_size, 
			  int &palette_size, int &unplaced_size) const;

private:
  typedef vector<UserAttribLine *> UserLines;
  UserLines _user_lines;

  typedef map<string, SourceEgg *> Eggs;
  Eggs _eggs;

  typedef map<string, PaletteGroup *> Groups;
  Groups _groups;

  typedef map<string, PTexture *> PTextures;
  PTextures _textures;

  typedef vector<TexturePacking *> Packing;
  Packing _packing;

  bool read_txa(istream &outfile);
  bool read_pi(istream &outfile, bool force_redo_all);
  bool write_txa(ostream &outfile) const;
  bool write_pi(ostream &outfile) const;

  bool parse_params(const vector<string> &words, istream &infile, 
		    string &line, int &line_num);
  bool parse_packing(const vector<string> &words, istream &infile, 
		     string &line, int &line_num);
  bool parse_texture(const vector<string> &words, istream &infile, 
		     string &line, int &line_num);
  bool parse_pathname(const vector<string> &words, istream &infile, 
		      string &line, int &line_num);
  bool parse_egg(const vector<string> &words, istream &infile, 
		 string &line, int &line_num, bool force_redo_all);
  bool parse_group(const vector<string> &words, istream &infile, 
		   string &line, int &line_num);
  bool parse_palette(const vector<string> &words, istream &infile, 
		     string &line, int &line_num);
  bool parse_unplaced(const vector<string> &words, istream &infile, 
		      string &line, int &line_num);
  bool parse_surprises(const vector<string> &words, istream &infile, 
		       string &line, int &line_num);

  bool _optimal;
  bool _txa_needs_rewrite;

  string _name;
  Filename _txa_filename;
  Filename _pi_filename;

  PaletteGroup *_default_group;

public:
  // These parameter values come from the command line, or from the
  // .pi file if omitted from the command line.  These are the
  // parameter values that specifically refer to textures and
  // palettes, and thus should be stored in the .pi file for future
  // reference.
  string _map_dirname;
  int _pal_xsize, _pal_ysize;
  int _default_margin;
  bool _force_power_2;
  bool _aggressively_clean_mapdir;

  int _txa_fd;
  fstream _txa_fstrm;

  friend class PTexture;
};

#endif
