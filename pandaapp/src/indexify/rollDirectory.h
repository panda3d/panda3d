// Filename: rollDirectory.h
// Created by:  drose (03Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef ROLLDIRECTORY_H
#define ROLLDIRECTORY_H

#include "pandatoolbase.h"

#include "filename.h"
#include "pvector.h"

class Photo;
class IndexImage;
class PNMTextMaker;

////////////////////////////////////////////////////////////////////
//       Class : RollDirectory
// Description : A representation of a single directory containing one
//               or more photos.
////////////////////////////////////////////////////////////////////
class RollDirectory {
public:
  RollDirectory(const Filename &dir);
  ~RollDirectory();

  const Filename &get_dir() const;
  const string &get_basename() const;
  const string &get_name() const;
  const string &get_desc() const;
  bool scan(const string &extension);
  void collect_index_images();

  bool sort_date_before(const RollDirectory &other) const;

  const Filename &get_newest_contributing_filename() const;

  bool is_empty() const;
  int get_num_photos() const;
  Photo *get_photo(int n) const;

  int get_num_index_images() const;
  IndexImage *get_index_image(int n) const;

  bool generate_images(const Filename &archive_dir, PNMTextMaker *text_maker);
  bool generate_html(const Filename &archive_dir,
		     const Filename &roll_dir_root);
  const string &get_comment_html() const;
  const string &get_index_html() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

  static bool insert_html_comment(ostream &html, Filename cm_filename);

private:
  void add_contributing_filename(const Filename &filename);
  static bool insert_html_comment_body(ostream &html, istream &cm);
  static string format_basename(const string &basename);
  void generate_nav_buttons(ostream &html, 
			    const Filename &prev_roll_filename,
			    const Filename &next_roll_filename, 
			    const string &up_href);
  static bool compare_filenames(const string &a, const string &b);

public:
  RollDirectory *_prev;
  RollDirectory *_next;

private:
  Filename _dir;
  string _basename;
  string _name;
  string _desc;
  typedef pvector<Photo *> Photos;
  Photos _photos;
  
  Filename _newest_contributing_filename;

  typedef pvector<IndexImage *> IndexImages;
  IndexImages _index_images;

  string _comment_html;
  string _index_html;
};

INLINE ostream &operator << (ostream &out, const RollDirectory &d) {
  d.output(out);
  return out;
}

#endif
