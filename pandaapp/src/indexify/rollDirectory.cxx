// Filename: rollDirectory.cxx
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

#include "rollDirectory.h"
#include "photo.h"
#include "indexImage.h"
#include "indent.h"
#include "notify.h"
#include "string_utils.h"
#include "indexParameters.h"

#include <ctype.h>
#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
RollDirectory::
RollDirectory(const Filename &dir) :
  _dir(dir)
{
  _basename = _dir.get_basename();
  if (format_rose) {
    _name = format_basename(_basename);
  } else {
    _name = _basename;
  }
  _prev = (RollDirectory *)NULL;
  _next = (RollDirectory *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
RollDirectory::
~RollDirectory() {
  IndexImages::iterator ii;
  for (ii = _index_images.begin(); ii != _index_images.end(); ++ii) {
    IndexImage *index_image = (*ii);
    delete index_image;
  }

  Photos::iterator pi;
  for (pi = _photos.begin(); pi != _photos.end(); ++pi) {
    Photo *photo = (*pi);
    delete photo;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_dir
//       Access: Public
//  Description: Returns the filename of the directory.
////////////////////////////////////////////////////////////////////
const Filename &RollDirectory::
get_dir() const {
  return _dir;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_basename
//       Access: Public
//  Description: Returns the base name of the directory.
////////////////////////////////////////////////////////////////////
const string &RollDirectory::
get_basename() const {
  return _basename;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_name
//       Access: Public
//  Description: Returns the formatted name of the directory.  This is
//               often the same as the basename, but if -r is
//               specified on the command line it might be a somewhat
//               different, reformatted name.  In any case, it is the
//               name of the roll as it should be presented to the
//               user.
////////////////////////////////////////////////////////////////////
const string &RollDirectory::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::scan
//       Access: Public
//  Description: Scans the directory for all the listed photos.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
scan(const string &extension) {
  bool reverse_order = false;
  bool explicit_list = false;

  // Check for an .ls file in the roll directory, which may give an
  // explicit ordering, or if empty, it specifies reverse ordering.
  Filename ls_filename(_dir, _basename + ".ls");
  if (ls_filename.exists()) {
    add_contributing_filename(ls_filename);
    ls_filename.set_text();
    ifstream ls;
    if (!ls_filename.open_read(ls)) {
      nout << "Could not read " << ls_filename << "\n";
    } else {
      bool any_words = false;
      string word;
      ls >> word;
      while (!ls.eof() && !ls.fail()) {
	any_words = true;
	Filename try_filename(_dir, word);
	if (!try_filename.exists()) {
	  try_filename = Filename(_dir, word + "." + extension);
	}
	if (!try_filename.exists()) {
	  try_filename = Filename(_dir, _basename + word + "." + extension);
	}
	if (!try_filename.exists()) {
	  try_filename = Filename(_dir, _basename + "0" + word + "." + extension);
	}
	if (try_filename.exists()) {
	  _photos.push_back(new Photo(this, try_filename.get_basename()));
	} else {
	  nout << "Frame " << word << " not found in " << _name << "\n";
	}
	ls >> word;
      }

      if (!any_words) {
	// An empty .ls file just means reverse the order.
	reverse_order = true;
      } else {
	// A non-empty .ls file has listed all the files we need; no
	// need to scan the directory.
	explicit_list = true;
      }
    }
  }

  if (!explicit_list) {
    vector_string contents;
    
    if (!_dir.scan_directory(contents)) {
      nout << "Could not read " << _dir << "\n";
      return false;
    }

    if (reverse_order) {
      reverse(contents.begin(), contents.end());
    }

    vector_string::iterator ci;
    for (ci = contents.begin(); ci != contents.end(); ++ci) {
      Filename basename = (*ci);
      if (basename.get_extension() == extension) {
	_photos.push_back(new Photo(this, basename));
      }
    }
  }

  if (_photos.empty()) {
    nout << _dir << " contains no photos.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::collect_index_images
//       Access: Public
//  Description: Groups the photos into one or more IndexImages.
////////////////////////////////////////////////////////////////////
void RollDirectory::
collect_index_images() {
  // Don't call this twice.
  nassertv(_index_images.empty());

  IndexImage *index_image = new IndexImage(this, _index_images.size());
  _index_images.push_back(index_image);

  size_t i;
  for (i = 0; i < _photos.size(); i++) {
    if (!index_image->add_photo(i)) {
      // Oops, that one's full.
      index_image = new IndexImage(this, _index_images.size());
      _index_images.push_back(index_image);
      bool result = index_image->add_photo(i);
      nassertv(result);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_newest_contributing_filename
//       Access: Public
//  Description: Returns the Filename with the newest timestamp that
//               contributes to the contents of the index images in
//               this directory, if any (other than the images
//               themselves).  This is used by the IndexImage code to
//               determine if it needs to regenerate the index image.
////////////////////////////////////////////////////////////////////
const Filename &RollDirectory::
get_newest_contributing_filename() const {
  return _newest_contributing_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_num_photos
//       Access: Public
//  Description: Returns the number of photos in the roll.
////////////////////////////////////////////////////////////////////
int RollDirectory::
get_num_photos() const {
  return _photos.size();
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_photo
//       Access: Public
//  Description: Returns the nth photo in the roll.
////////////////////////////////////////////////////////////////////
Photo *RollDirectory::
get_photo(int n) const {
  nassertr(n >= 0 && n < (int)_photos.size(), NULL);
  return _photos[n];
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_num_index_images
//       Access: Public
//  Description: Returns the number of index images in the roll.
////////////////////////////////////////////////////////////////////
int RollDirectory::
get_num_index_images() const {
  return _index_images.size();
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_index_image
//       Access: Public
//  Description: Returns the nth index image in the roll.
////////////////////////////////////////////////////////////////////
IndexImage *RollDirectory::
get_index_image(int n) const {
  nassertr(n >= 0 && n < (int)_index_images.size(), NULL);
  return _index_images[n];
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::generate_images
//       Access: Public
//  Description: Generates image files appropriate to this directory
//               to the indicated archive dir.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
generate_images(const Filename &archive_dir, PNMTextMaker *text_maker) {
  nassertr(!_index_images.empty(), false);

  IndexImages::iterator ii;
  for (ii = _index_images.begin(); ii != _index_images.end(); ++ii) {
    IndexImage *index_image = (*ii);
    if (!index_image->generate_images(archive_dir, text_maker)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::generate_html
//       Access: Public
//  Description: Generates all appropriate HTML files for this
//               directory, and generate the appropriate HTML code
//               into the root_html file.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
generate_html(ostream &root_html, const Filename &archive_dir, 
              const Filename &roll_dir_root) {
  nassertr(!_index_images.empty(), false);

  if (!omit_roll_headers) {
    Filename cm_filename(_dir, _basename);
    cm_filename.set_extension("cm");
    if (cm_filename.exists()) {
      // If the comment file for the roll exists, insert its contents
      // here instead of the generic header.
      if (!insert_html_comment(root_html, cm_filename)) {
	return false;
      }
      
    } else {
      root_html
	<< "<h2>" << _name << "</h2>\n";
    }
  }

  nout << "Generating " << Filename(archive_dir, "html/")
       << _basename << "/*\n";

  root_html << "<p>\n";
  IndexImages::iterator ii;
  for (ii = _index_images.begin(); ii != _index_images.end(); ++ii) {
    IndexImage *index_image = (*ii);
    if (!index_image->generate_html(root_html, archive_dir, roll_dir_root)) {
      return false;
    }
  }
  root_html << "</p>\n";

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void RollDirectory::
output(ostream &out) const {
  out << _name;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void RollDirectory::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _dir << "\n";

  if (!_index_images.empty()) {
    // If we have any index images, output those.
    IndexImages::const_iterator ii;
    for (ii = _index_images.begin(); ii != _index_images.end(); ++ii) {
      const IndexImage *index_image = (*ii);
      index_image->write(out, indent_level + 2);
    }

  } else {
    // Otherwise, just output the photos.
    Photos::const_iterator pi;
    for (pi = _photos.begin(); pi != _photos.end(); ++pi) {
      const Photo *photo = (*pi);
      indent(out, indent_level + 2)
        << *photo << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::insert_html_comment
//       Access: Public, Static
//  Description: Reads the indicated comment file and inserts its
//               contents (after the header) within the indicated
//               ostream at the current point.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
insert_html_comment(ostream &html, Filename cm_filename) {
  cm_filename.set_text();
  ifstream cm;
  if (!cm_filename.open_read(cm)) {
    nout << "Unable to read " << cm_filename << "\n";
    return false;
  }

  // First, scan through the file looking for <BODY> or <body>.
  string line;
  getline(cm, line);
  while (!cm.eof() && !cm.fail()) {
    string lower = downcase(line);
    size_t body = lower.find("<body>");
    if (body != string::npos) {
      return insert_html_comment_body(html, cm);
    }
    getline(cm, line);
  }

  // We didn't find it.  Insert the whole file.
  cm.clear();
  cm.seekg(0);
  int ch = cm.get();
  while (ch != EOF) {
    html.put(ch);
    ch = cm.get();
  }    

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::add_contributing_filename
//       Access: Private
//  Description: Specifies an additional filename that contributes to
//               the index image.
////////////////////////////////////////////////////////////////////
void RollDirectory::
add_contributing_filename(const Filename &filename) {
  if (_newest_contributing_filename.empty() ||
      _newest_contributing_filename.compare_timestamps(filename) < 0) {
    _newest_contributing_filename = filename;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::insert_html_comment_body
//       Access: Private, Static
//  Description: Copies from the comment file up until the next
//               </body>, or until the end of file.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
insert_html_comment_body(ostream &html, istream &cm) {
  string line;
  getline(cm, line);
  while (!cm.eof() && !cm.fail()) {
    string lower = downcase(line);
    size_t body = lower.find("</body>");
    if (body != string::npos) {
      // Here's the end; we're done.
      return true;
    }
    html << line << "\n";
    getline(cm, line);
  }

  // Reached end of file; good enough.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::format_basename
//       Access: Private, Static
//  Description: Reformats the roll directory into a user-friendly
//               name based on its encoded directory name.
////////////////////////////////////////////////////////////////////
string RollDirectory::
format_basename(const string &basename) {
  if (basename.length() <= 4) {
    return basename;
  }

  // The first four characters must be digits.
  for (size_t i = 0; i < 4; i++) {
    if (!isdigit(basename[i])) {
      return basename;
    }
  }

  string mm = basename.substr(0, 2);
  string yy = basename.substr(2, 2);
  string ss = basename.substr(4);

  if (mm[0] == '0') {
    mm = mm[1];
  }
  while (ss.length() > 1 && ss[0] == '0') {
    ss = ss.substr(1);
  }

  return mm + "-" + yy + "/" + ss;
}
