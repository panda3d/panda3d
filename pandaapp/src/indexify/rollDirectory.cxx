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
//     Function: RollDirectory::scan
//       Access: Public
//  Description: Scans the directory for all the listed photos.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
scan(const string &extension) {
  vector_string contents;
  if (!_dir.scan_directory(contents)) {
    nout << "Could not read " << _dir << "\n";
    return false;
  }

  vector_string::iterator ci;
  for (ci = contents.begin(); ci != contents.end(); ++ci) {
    Filename basename = (*ci);
    if (basename.get_extension() == extension) {
      _photos.push_back(new Photo(this, basename));
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
generate_images(const Filename &archive_dir, TextMaker *text_maker) {
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
      << "<h2>" << _basename << "</h2>\n";
  }

  IndexImages::iterator ii;
  for (ii = _index_images.begin(); ii != _index_images.end(); ++ii) {
    IndexImage *index_image = (*ii);
    if (!index_image->generate_html(root_html, archive_dir, roll_dir_root)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void RollDirectory::
output(ostream &out) const {
  out << _basename;
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
  cm.seekg(0);
  int ch = cm.get();
  while (ch != EOF) {
    html.put(ch);
    ch = cm.get();
  }    

  return true;
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
