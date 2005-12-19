// Filename: rollDirectory.cxx
// Created by:  drose (03Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
  _prev = (RollDirectory *)NULL;
  _next = (RollDirectory *)NULL;

  _basename = _dir.get_basename();
  if (format_rose) {
    _name = format_basename(_basename);
  } else {
    _name = _basename;
  }

  _got_ds_file = false;
  _got_ls_file = false;
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
//     Function: RollDirectory::get_desc
//       Access: Public
//  Description: Returns the one-line description for the directory.
////////////////////////////////////////////////////////////////////
const string &RollDirectory::
get_desc() const {
  return _desc;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::scan
//       Access: Public
//  Description: Scans the directory for all the listed photos.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
scan(const string &photo_extension, const string &movie_extension,
     const string &sound_extension) {
  bool reverse_order = false;
  bool explicit_list = false;

  // Check for a .ds file, which contains a one-line description of
  // the contents of the directory.
  _ds_filename = Filename(_basename);
  _ds_filename.set_extension("ds");
  if (cm_search.is_empty() || !_ds_filename.resolve_filename(cm_search)) {
    // If the ds file isn't found along the search path specified
    // via -cmdir on the command line, then look for it in the
    // appropriate source directory.
    _ds_filename = Filename(_dir, _ds_filename);
  }
  if (_ds_filename.exists()) {
    _got_ds_file = true;
    _ds_filename.set_text();
    ifstream ds;
    if (!_ds_filename.open_read(ds)) {
      nout << "Could not read " << _ds_filename << "\n";
    } else {
      // Get the words out one at a time and put just one space
      // between them.
      string word;
      ds >> word;
      while (!ds.eof() && !ds.fail()) {
	if (!_desc.empty()) {
	  _desc += ' ';
	}
	_desc += word;
	word = string();
	ds >> word;
      }
      if (!word.empty()) {
	if (!_desc.empty()) {
	  _desc += ' ';
	}
	_desc += word;
      }
    }
  }

  // Check for an .ls file in the roll directory, which may give an
  // explicit ordering, or if empty, it specifies reverse ordering.
  _ls_filename = Filename(_dir, _basename);
  _ls_filename.set_extension("ls");
  if (_ls_filename.exists()) {
    _got_ls_file = true;
    add_contributing_filename(_ls_filename);
    _ls_filename.set_text();
    ifstream ls;
    if (!_ls_filename.open_read(ls)) {
      nout << "Could not read " << _ls_filename << "\n";
    } else {
      bool any_words = false;
      string word;
      ls >> word;
      while (!ls.eof() && !ls.fail()) {
	any_words = true;
	Filename try_filename(_dir, word);
	if (!try_filename.exists()) {
	  try_filename = Filename(_dir, word + "." + photo_extension);
	}
	if (!try_filename.exists()) {
	  try_filename = Filename(_dir, _basename + word + "." + photo_extension);
	}
	if (!try_filename.exists()) {
	  try_filename = Filename(_dir, _basename + "0" + word + "." + photo_extension);
	}
	if (try_filename.exists()) {
          add_photo(try_filename.get_basename(), movie_extension, 
		    sound_extension);
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

    // We want to sort the filenames in our own way, so that
    // underscore is deemed to fall before anything else.  This allows
    // the underscore, the only non-alphnumeric character we can have
    // in a filename for an iso9660 image, to stand for "before 0".
    sort(contents.begin(), contents.end(), compare_filenames);

    if (reverse_order) {
      reverse(contents.begin(), contents.end());
    }

    vector_string::iterator ci;
    for (ci = contents.begin(); ci != contents.end(); ++ci) {
      Filename basename = (*ci);
      if (basename.get_extension() == photo_extension) {
        add_photo(basename, movie_extension, sound_extension);
      }
    }
  }

  if (_photos.empty()) {
    nout << _dir << " contains no photos.\n";
    // This is not an error, just a notice.
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

  if (is_empty()) {
    return;
  }

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
//     Function: RollDirectory::sort_date_before
//       Access: Public
//  Description: Returns true if the given directory name should sort
//               before the other one, assuming the Rose naming
//               convention of mmyyss is in place.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
sort_date_before(const RollDirectory &other) const {
  if (_name == _basename && other._name == other._basename) {
    // If Rose naming convention is not in place in either case, sort
    // alphabetically.
    return _basename < other._basename;

  } else if (_name == _basename) {
    // If Rose naming convention is in place on this one and not the
    // other, it sorts first.
    return true;

  } else if (other._name == other._basename) {
    // And vice-versa.
    return false;

  } else {
    // Rose naming convention holds.  Sort based on year first.  Years
    // above 90 are deemed to belong to the previous century.
    string yy = _basename.substr(2, 2);
    string other_yy = other._basename.substr(2, 2);
    int year = atoi(yy.c_str());
    int other_year = atoi(other_yy.c_str());
    if (year < 90) {
      year += 100;
    }
    if (other_year < 90) {
      other_year += 100;
    }
    if (year != other_year) {
      return year < other_year;
    }

    // After year, sort alphabetically.
    return _basename < other._basename;
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
//     Function: RollDirectory::is_empty
//       Access: Public
//  Description: Returns true if the directory is empty (has no
//               photos).
////////////////////////////////////////////////////////////////////
bool RollDirectory::
is_empty() const {
  return _photos.empty();
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
  if (is_empty()) {
    return true;
  }
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
//               into the html strings (retrieved by
//               get_comment_html() and get_index_html()).
////////////////////////////////////////////////////////////////////
bool RollDirectory::
generate_html(const Filename &archive_dir, const Filename &roll_dir_root) {
  if (is_empty()) {
    return true;
  }
  nassertr(!_index_images.empty(), false);

  ostringstream comment_strm;
  comment_strm
    << "<a name=\"" << _basename << "\">\n";

  if (!omit_roll_headers) {
    Filename cm_filename(_basename);
    cm_filename.set_extension("cm");
    if (cm_search.is_empty() || !cm_filename.resolve_filename(cm_search)) {
      // If the cm file isn't found along the search path specified
      // via -cmdir on the command line, then look for it in the
      // appropriate source directory.
      cm_filename = Filename(_dir, cm_filename);
    }

    if (cm_filename.exists()) {
      // If the comment file for the roll exists, insert its contents
      // here instead of the generic header.
      if (!insert_html_comment(comment_strm, cm_filename)) {
	return false;
      }
      
    } else {
      comment_strm
	<< "<h2>" << _name << "</h2>\n";
      if (!_desc.empty()) {
	comment_strm << "<p>" << escape_html(_desc) << ".</p>\n";
      }
    }
  }
  _comment_html = comment_strm.str();

  Filename html_dir(archive_dir, "html");
  nout << "Generating " << Filename(html_dir, _basename) << "/*\n";

  ostringstream index_strm;
  index_strm << "<p>\n";
  IndexImages::iterator ii;
  for (ii = _index_images.begin(); ii != _index_images.end(); ++ii) {
    IndexImage *index_image = (*ii);
    if (!index_image->generate_html(index_strm, archive_dir, roll_dir_root)) {
      return false;
    }
  }
  index_strm << "</p>\n";
  _index_html = index_strm.str();

  // Also generate the index html for this directory.
  Filename html_filename(html_dir, _basename);
  html_filename.set_extension("htm");
  nout << "Generating " << html_filename << "\n";
  html_filename.set_text();
  ofstream index_html;
  if (!html_filename.open_write(index_html)) {
    nout << "Unable to write to " << html_filename << "\n";
    exit(1);
  }

  string up_href = "../index.htm#" + _basename;

  Filename prev_roll_filename;
  Filename next_roll_filename;

  if (_prev != (RollDirectory *)NULL) {
    prev_roll_filename = _prev->_basename;
    prev_roll_filename.set_extension("htm");
  }
  if (_next != (RollDirectory *)NULL) {
    next_roll_filename = _next->_basename;
    next_roll_filename.set_extension("htm");
  }

  index_html
    << "<html>\n"
    << "<head>\n";
  if (_desc.empty()) {
    index_html
      << "<title>" << _name << "</title>\n";
  } else {
    index_html
      << "<title>" << _name << " " << escape_html(_desc) << "</title>\n";
  }
  index_html
    << "</head>\n"
    << "<body>\n"
    << get_comment_html();

  generate_nav_buttons(index_html, prev_roll_filename, next_roll_filename,
                       up_href);
  index_html << get_index_html();
  generate_nav_buttons(index_html, prev_roll_filename, next_roll_filename,
                       up_href);

  if (!omit_complete) {
    index_html
      << "<a href=\"complete.htm#" << _basename
      << "\">(complete archive)</a>\n";
  }

  index_html
    << "</body>\n"
    << "</html>\n";

  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_comment_html
//       Access: Public
//  Description: Returns the HTML text that describes this directory's
//               index.  This is set when generate_html() returns
//               true.
//
//               This text may be inserted into the middle of a HTML
//               page to include the imagemap that references each of
//               the images in this directory.
////////////////////////////////////////////////////////////////////
const string &RollDirectory::
get_comment_html() const {
  return _comment_html;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::get_index_html
//       Access: Public
//  Description: Returns the HTML text that describes this directory's
//               index.  This is set when generate_html() returns
//               true.
//
//               This text may be inserted into the middle of a HTML
//               page to include the imagemap that references each of
//               the images in this directory.
////////////////////////////////////////////////////////////////////
const string &RollDirectory::
get_index_html() const {
  return _index_html;
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::copy_reduced
//       Access: Public
//  Description: Copies key files from the full directory into the
//               reduced directory.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
copy_reduced(const Filename &archive_dir) {
  if (is_empty()) {
    return true;
  }
  nassertr(!_index_images.empty(), false);

  Filename reduced_dir(archive_dir, "reduced/" + _dir.get_basename());

  if (!reduced_dir.exists()) {
    cerr << "Ignoring " << reduced_dir << "; does not exist.\n";
    return true;
  }

  if (_got_ds_file) {
    if (!copy_file(_ds_filename, reduced_dir)) {
      return false;
    }
  }

  if (_got_ls_file) {
    if (!copy_file(_ls_filename, reduced_dir)) {
      return false;
    }
  }

  IndexImages::iterator ii;
  for (ii = _index_images.begin(); ii != _index_images.end(); ++ii) {
    IndexImage *index_image = (*ii);
    if (!index_image->copy_reduced(archive_dir)) {
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
//     Function: RollDirectory::add_photo
//       Access: Private
//  Description: Adds the photo with the indicated basename to the
//               list.
////////////////////////////////////////////////////////////////////
void RollDirectory::
add_photo(const Filename &basename, const string &movie_extension,
	  const string &sound_extension) {
  Photo *photo = new Photo(this, basename, movie_extension, sound_extension);
  _photos.push_back(photo);
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

  // The first two characters must be alphanumeric.
  if (!isalnum(basename[0]) || !isalnum(basename[1])) {
    return basename;
  }

  // The next two characters must be digits.
  if (!isdigit(basename[2]) || !isdigit(basename[3])) {
    return basename;
  }

  // If the first two were digits as well as being alphanumeric, then
  // we have mm-yy/sequence.  Otherwise, we just have xxyy/sequence.
  bool mm_is_month = (isdigit(basename[0]) && isdigit(basename[1]));

  string mm = basename.substr(0, 2);
  string yy = basename.substr(2, 2);
  string ss = basename.substr(4);

  if (mm_is_month && mm[0] == '0') {
    mm = mm[1];
  }
  while (ss.length() > 1 && ss[0] == '0') {
    ss = ss.substr(1);
  }

  if (mm_is_month) {
    return mm + "-" + yy + "/" + ss;
  } else {
    return mm + yy + "/" + ss;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::generate_nav_buttons
//       Access: Private, Static
//  Description: Outputs the HTML code to generate the next, prev,
//               up buttons when viewing each reduced image.
////////////////////////////////////////////////////////////////////
void RollDirectory::
generate_nav_buttons(ostream &html, const Filename &prev_roll_filename,
                     const Filename &next_roll_filename, 
                     const string &up_href) {
  html << "<p>\n";

  bool first_icons = false;
  if (!prev_icon.empty() && !next_icon.empty()) {
    first_icons = true;
    // Use icons to go forward and back.
    Filename prev_icon_href = compose_href("..", prev_icon);
    if (prev_roll_filename.empty()) {
      html << "<img src=\"" << prev_icon_href << "\" alt=\"No previous roll\">\n";
    } else {
      html << "<a href=\"" << prev_roll_filename
           << "\"><img src=\"" << prev_icon_href << "\" alt=\"previous\"></a>\n";
    }

    Filename next_icon_href = compose_href("..", next_icon);
    if (next_roll_filename.empty()) {
      html << "<img src=\"" << next_icon_href << "\" alt=\"No next roll\">\n";
    } else {
      html << "<a href=\"" << next_roll_filename
           << "\"><img src=\"" << next_icon_href << "\" alt=\"next\"></a>\n";
    }

  } else {
    // No prev/next icons; use text to go forward and back.
    if (prev_roll_filename.empty()) {
      html << "(This is the first roll.)\n";
    } else {
      html << "<a href=\"" << prev_roll_filename
           << "\">Back to previous roll</a>\n";
    }
    
    if (next_roll_filename.empty()) {
      html << "<br>(This is the last roll.)\n";
    } else {
      html << "<br><a href=\"" << next_roll_filename
           << "\">On to next roll</a>\n";
    }
  }

  if (!up_href.empty()) {
    if (!up_icon.empty()) {
      // Use an icon to go up.
      if (!first_icons) {
        html << "<br>";
      } else {
        html << "&nbsp;&nbsp;&nbsp;";
      }
      Filename up_icon_href = compose_href("..", up_icon);
      html << "<a href=\"" << up_href
           << "\"><img src=\"" << up_icon_href << "\" alt=\"return to index\"></a>\n";
    } else {
      // No up icon; use text to go up.
      html << "<br><a href=\"" << up_href
           << "\">Return to index</a>\n";
    }
  }
  html << "</p>\n";
}

////////////////////////////////////////////////////////////////////
//     Function: RollDirectory::compare_filenames
//       Access: Private, Static
//  Description: Returns true if filename a sorts before filename b,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool RollDirectory::
compare_filenames(const string &a, const string &b) {
  size_t i = 0;
  size_t min_length = min(a.length(), b.length());
  while (i < min_length) {
    char achar = a[i];
    char bchar = b[i];
    if (achar != bchar) {
      if (achar == '_') {
        return true;
      } else if (bchar == '_') {
        return false;
      } else {
        return achar < bchar;
      }
    }
    ++i;
  }

  return a.length() < b.length();
}

