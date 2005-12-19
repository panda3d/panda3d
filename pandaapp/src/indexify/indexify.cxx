// Filename: indexify.cxx
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

#include "indexify.h"
#include "rollDirectory.h"
#include "notify.h"
#include "pnmTextMaker.h"
#include "default_font.h"
#include "default_index_icons.h"
#include "indexParameters.h"
#include "string_utils.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: Indexify::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Indexify::
Indexify() {
  clear_runlines();
  add_runline("[opts] roll1-dir roll2-dir [roll3-dir ...]");
  add_runline("[opts] full/*");

  set_program_description
    ("This program reads a collection of directories containing photo "
     "archives (typically JPEG files), and will generate a number of "
     "thumbnail images and a series of HTML pages to browse them.  It is "
     "especially useful in preparation for burning the photo archives to "
     "CD.\n\n"

     "A number of directories is named on the command line; each "
     "directory must contain a number of image files, and all directories "
     "should be within the same parent directory.  Each directory is "
     "considered a \"roll\", which may or may not correspond to a physical "
     "roll of film, and the photos within each directory are grouped "
     "correspondingly on the generated HTML pages.  One common special case "
     "is in which all the roll directories are found in the subdirectory "
     "named \"full\" (e.g., the second example above) or \"reduced\".  This "
     "keeps the root directory nice and clean.\n\n"

     "If a file exists by the same name as an image file but with the "
     "extension \"cm\", that file is taken to be a HTML comment about that "
     "particular image and is inserted the HTML page for that image.  "
     "Similarly, if there is a file within a roll directory with the same "
     "name as the directory itself (but with the extension \"cm\"), that file "
     "is inserted into the front page to introduce that particular roll.  "
     "Finally, a file with the name of the directory, but with the extension "
     "\"ds\" may contain a brief one-line description of the directory, for "
     "the toplevel index page.\n\n"

     "Normally, all image files with the specified extension (normally "
     "\"jpg\") within a roll directory are included in the index, and sorted "
     "into alphabetical (or numerical) order.  If you wish to specify a "
     "different order, or use only a subset of the images in a directory, "
     "create a file in the roll directory with the same name as the "
     "directory itself, and the extension \"ls\".  This file should "
     "simply list the filenames (with or without extension) within the "
     "roll directory in the order they should be listed.  If the ls "
     "file exists but is empty, it indicates that the files should be "
     "listed in reverse order, as from a camera that loads its film "
     "upside-down.");

  add_option
    ("t", "title", 0,
     "Specifies the title to give to the front HTML page.",
     &Indexify::dispatch_string, NULL, &_front_title);

  add_option
    ("a", "archive-dir", 0,
     "Write the generated files to the indicated directory, instead of "
     "the directory above roll1-dir.",
     &Indexify::dispatch_filename, NULL, &archive_dir);

  add_option
    ("r", "relative-dir", 0,
     "When -a is specified to place the generated html files in a directory "
     "other than the default, you may need "
     "to specify how the html files will address the roll directories.  This "
     "parameter specifies the relative path to the directory above the roll "
     "directories, from the directory named by -a.",
     &Indexify::dispatch_filename, NULL, &_roll_dir_root);

  add_option
    ("f", "", 0,
     "Forces the regeneration of all reduced and thumbnail images, even if "
     "image files already exist that seem to be newer than the source "
     "image files.",
     &Indexify::dispatch_none, &force_regenerate);

  add_option
    ("r", "", 0,
     "Specifies that roll directory names are encoded using the Rose "
     "convention of six digits: mmyyss, where mm and yy are the month and "
     "year, and ss is a sequence number of the roll within the month.  This "
     "name will be reformatted to m-yy/s for output.",
     &Indexify::dispatch_none, &format_rose);

  add_option
    ("s", "", 0,
     "When used in conjunction with -r, requests sorting of the roll "
     "directory names by date.",
     &Indexify::dispatch_none, &sort_date);

  add_option
    ("d", "", 0,
     "Run in \"dummy\" mode; don't load any images, but instead just "
     "draw an empty box indicating where the thumbnails will be.",
     &Indexify::dispatch_none, &dummy_mode);

  add_option
    ("slide", "", 0,
     "Draw a frame, like a slide mount, around each thumbnail image.",
     &Indexify::dispatch_none, &draw_frames);

  add_option
    ("pe", "extension", 0,
     "Specifies the filename extension (without a leading dot) to identify "
     "photo files within the roll directories.  This is normally jpg.",
     &Indexify::dispatch_string, NULL, &_photo_extension);

  add_option
    ("me", "extension", 0,
     "Specifies the filename extension (without a leading dot) to identify "
     "movie files within the roll directories.  This is normally mov.  If "
     "a file exists with the same name as a given photo but with this "
     "extension, it is taken to be a movie associated with the photo, and "
     "a link will be generated to play the movie.",
     &Indexify::dispatch_string, NULL, &_movie_extension);

  add_option
    ("se", "extension", 0,
     "Specifies the filename extension (without a leading dot) to identify "
     "sound files within the roll directories.  This is normally mp3.  If "
     "a file exists with the same name as a given photo but with this "
     "extension, it is taken to be a sound clip associated with the photo, "
     "and a link will be generated to play the clip.",
     &Indexify::dispatch_string, NULL, &_sound_extension);

  add_option
    ("i", "", 0,
     "Indicates that default navigation icon images should be generated "
     "into a directory called \"icons\" which will be created within the "
     "directory named by -a.  This is meaningful only if -iprev, -inext, "
     "-iup, -imovie, and -isound are not explicitly specified.",
     &Indexify::dispatch_none, &_generate_icons);

  add_option
    ("omit-rh", "", 0,
     "Omits roll headers introducing each roll directory, including any "
     "headers defined in roll.cm files.",
     &Indexify::dispatch_none, &omit_roll_headers);

  add_option
    ("cmdir", "director", 0,
     "Searches in the named directory for .cm files before searching within "
     "the source archive.  This option may be repeated.",
     &Indexify::dispatch_search_path, NULL, &cm_search);

  add_option
    ("omit-full", "", 0,
     "Omits links to the full-size images.",
     &Indexify::dispatch_none, &omit_full_links);

  add_option
    ("omit-complete", "", 0,
     "Omits the complete.htm index file that lists all thumbnails at once.",
     &Indexify::dispatch_none, &omit_complete);

  add_option
    ("caption", "size[,spacing]", 0,
     "Specifies the font size in pixels of the thumbnail captions.  If the "
     "optional spacing parameter is included, it is the number of pixels "
     "below each thumbnail that the caption should be placed.  Specify "
     "-caption 0 to disable thumbnail captions.",
     &Indexify::dispatch_caption, NULL);

  add_option
    ("fnum", "", 0,
     "Writes the frame number of each thumbnail image into the caption "
     "on the index page, instead of the image filename.  This only works "
     "if the photo image filenames consist of the roll directory name "
     "concatenated with a frame number.",
     &Indexify::dispatch_none, &caption_frame_numbers);

  add_option
    ("font", "fontname", 0,
     "Specifies the filename of the font to use to generate the thumbnail "
     "captions.",
     &Indexify::dispatch_filename, NULL, &_font_filename);

  add_option
    ("fontaa", "factor", 0,
     "Specifies a scale factor to apply to the fonts used for captioning "
     "when generating text for the purpose of antialiasing the fonts a "
     "little better than FreeType can do by itself.  The letters are "
     "generated large and then scaled to their proper size.  Normally this "
     "should be a number in the range 3 to 4 for best effect.",
     &Indexify::dispatch_double, NULL, &_font_aa_factor);

  add_option
    ("thumb", "x,y", 0,
     "Specifies the size in pixels of the thumbnail images.",
     &Indexify::dispatch_int_pair, NULL, &thumb_width);

  add_option
    ("reduced", "x,y", 0,
     "Specifies the size in pixels of reduced images (images presented after "
     "the first click on a thumbnail).",
     &Indexify::dispatch_int_pair, NULL, &reduced_width);

  add_option
    ("space", "x,y", 0,
     "Specifies the x,y spacing between thumbnail images, in pixels.",
     &Indexify::dispatch_int_pair, NULL, &thumb_x_space);

  add_option
    ("index", "x,y", 0,
     "Specifies the size in pixels of the index images (the images that "
     "contain an index of thumbnails).",
     &Indexify::dispatch_int_pair, NULL, &max_index_width);

  add_option
    ("iprev", "filename", 0,
     "Specifies the relative pathname from the archive directory (or "
     "absolute pathname) to the \"previous\" icon.",
     &Indexify::dispatch_filename, NULL, &prev_icon);

  add_option
    ("inext", "filename", 0,
     "Specifies the relative pathname from the archive directory (or "
     "absolute pathname) to the \"next\" icon.",
     &Indexify::dispatch_filename, NULL, &next_icon);

  add_option
    ("iup", "filename", 0,
     "Specifies the relative pathname from the archive directory (or "
     "absolute pathname) to the \"up\" icon.",
     &Indexify::dispatch_filename, NULL, &up_icon);

  add_option
    ("imovie", "filename", 0,
     "Specifies the relative pathname from the archive directory (or "
     "absolute pathname) to the \"movie\" icon.  This is used only if "
     "there are one or more movie files found in the directory.",
     &Indexify::dispatch_filename, NULL, &movie_icon);

  add_option
    ("isound", "filename", 0,
     "Specifies the relative pathname from the archive directory (or "
     "absolute pathname) to the \"sound\" icon.  This is used only if "
     "there are one or more sound files found in the directory.",
     &Indexify::dispatch_filename, NULL, &sound_icon);

  add_option
    ("copyreduced", "", 0,
     "Instead of generating index files, copy key files (such as "
     "*.cm, *.ds) from the full image directory into the reduced "
     "image directory, so that the full image directory may be removed "
     "for generating a reduced-size archive.",
     &Indexify::dispatch_none, &_copy_reduced);

  _photo_extension = "jpg";
  _movie_extension = "mov";
  _sound_extension = "mp3";
  _text_maker = (PNMTextMaker *)NULL;
  _font_aa_factor = 4.0;
}

////////////////////////////////////////////////////////////////////
//     Function: Indexify::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Indexify::
~Indexify() {
  RollDirs::iterator di;
  for (di = _roll_dirs.begin(); di != _roll_dirs.end(); ++di) {
    RollDirectory *roll_dir = (*di);
    delete roll_dir;
  }

  if (_text_maker != (PNMTextMaker *)NULL) {
    delete _text_maker;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Indexify::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool Indexify::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the roll directories containing archive photos on the command line.\n";
    return false;
  }

  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    Filename filename = Filename::from_os_specific(*ai);
    filename.standardize();
    if (filename.is_directory()) {
      string basename = filename.get_basename();
      if (basename == "icons" || basename == "html" || 
	  basename == "reduced" || basename == "thumbs") {
	nout << "Ignoring " << filename << "; indexify-generated directory.\n";

      } else {
	RollDirectory *roll_dir = new RollDirectory(filename);
	_roll_dirs.push_back(roll_dir);
      }

    } else if (filename.exists()) {
      nout << "Ignoring " << filename << "; not a directory.\n";

    } else {
      nout << filename << " does not exist.\n";
      return false;
    }
  }

  return true;
}

class SortRollDirs {
public:
  bool operator () (const RollDirectory *a, const RollDirectory *b) const {
    return a->sort_date_before(*b);
  }
};

////////////////////////////////////////////////////////////////////
//     Function: Indexify::post_command_line
//       Access: Protected, Virtual
//  Description: This is called after the command line has been
//               completely processed, and it gives the program a
//               chance to do some last-minute processing and
//               validation of the options and arguments.  It should
//               return true if everything is fine, false if there is
//               an error.
////////////////////////////////////////////////////////////////////
bool Indexify::
post_command_line() {
  if (_roll_dirs.empty()) {
    nout << "No roll directories.\n";
    return false;
  }

  if (archive_dir.empty()) {
    // Choose a default archive directory, above the first roll directory.
    archive_dir = _roll_dirs.front()->get_dir().get_dirname();
    string parent_dirname = archive_dir.get_basename();
    if (parent_dirname == "full" || parent_dirname == "reduced") {
      // As a special case, if the subdirectory name is "full" or
      // "reduced", use the directory above that.
      archive_dir = archive_dir.get_dirname();
    }
    if (archive_dir.empty()) {
      archive_dir = ".";
    }
  }
  archive_dir.standardize();

  if (!_roll_dir_root.empty()) {
    _roll_dir_root.standardize();
  }

  // Sort the roll directories, if specified.
  if (format_rose && sort_date) {
    sort(_roll_dirs.begin(), _roll_dirs.end(), SortRollDirs());
  }

  // Update the next/prev pointers in each roll directory so we can
  // browse from one to the other.
  RollDirectory *prev_roll_dir = (RollDirectory *)NULL;
  RollDirs::iterator di;
  for (di = _roll_dirs.begin(); di != _roll_dirs.end(); ++di) {
    RollDirectory *roll_dir = (*di);
    if (prev_roll_dir != (RollDirectory *)NULL) {
      roll_dir->_prev = prev_roll_dir;
      prev_roll_dir->_next = roll_dir;
    }
    prev_roll_dir = roll_dir;
  }

  if (_front_title.empty()) {
    // Supply a default title.
    if (_roll_dirs.size() == 1) {
      _front_title = _roll_dirs.front()->get_name();
    } else {
      _front_title = _roll_dirs.front()->get_name() + " through " + _roll_dirs.back()->get_name();
    }
  }
    
  if (caption_font_size != 0) {
    if (!_font_filename.empty()) {
      _text_maker = new PNMTextMaker(_font_filename, 0);
      if (!_text_maker->is_valid()) {
	delete _text_maker;
	_text_maker = (PNMTextMaker *)NULL;
      }
    }
    
    if (_text_maker == (PNMTextMaker *)NULL) {
      _text_maker = new PNMTextMaker((const char *)default_font, 
                                     default_font_size, 0);
      if (!_text_maker->is_valid()) {
	nout << "Unable to open default font.\n";
	delete _text_maker;
	_text_maker = (PNMTextMaker *)NULL;
      }
    }
    
    if (_text_maker != (PNMTextMaker *)NULL) {
      _text_maker->set_scale_factor(_font_aa_factor);
      _text_maker->set_pixel_size(caption_font_size);
      _text_maker->set_align(PNMTextMaker::A_center);
    }
  }

  if (_generate_icons) {
    if (prev_icon.empty()) {
      prev_icon = Filename("icons", default_left_icon_filename);
      Filename icon_filename(archive_dir, prev_icon);

      if (force_regenerate || !icon_filename.exists()) {
	nout << "Generating " << icon_filename << "\n";
	icon_filename.make_dir();
	icon_filename.set_binary();

	ofstream output;
	if (!icon_filename.open_write(output)) {
	  nout << "Unable to write to " << icon_filename << "\n";
	  exit(1);
	}
	output.write((const char *)default_left_icon, default_left_icon_len);
      }
    }
    if (next_icon.empty()) {
      next_icon = Filename("icons", default_right_icon_filename);
      Filename icon_filename(archive_dir, next_icon);
      if (force_regenerate || !icon_filename.exists()) {
	nout << "Generating " << icon_filename << "\n";
	icon_filename.make_dir();
	icon_filename.set_binary();
	
	ofstream output;
	if (!icon_filename.open_write(output)) {
	  nout << "Unable to write to " << icon_filename << "\n";
	  exit(1);
	}
	output.write((const char *)default_right_icon, default_right_icon_len);
      }
    }
    if (up_icon.empty()) {
      up_icon = Filename("icons", default_up_icon_filename);
      Filename icon_filename(archive_dir, up_icon);
      if (force_regenerate || !icon_filename.exists()) {
	nout << "Generating " << icon_filename << "\n";
	icon_filename.make_dir();
	icon_filename.set_binary();
      
	ofstream output;
	if (!icon_filename.open_write(output)) {
	  nout << "Unable to write to " << icon_filename << "\n";
	  exit(1);
	}
	output.write((const char *)default_up_icon, default_up_icon_len);
      }
    }
    if (movie_icon.empty()) {
      movie_icon = Filename("icons", default_movie_icon_filename);
      Filename icon_filename(archive_dir, movie_icon);
      if (force_regenerate || !icon_filename.exists()) {
	nout << "Generating " << icon_filename << "\n";
	icon_filename.make_dir();
	icon_filename.set_binary();
      
	ofstream output;
	if (!icon_filename.open_write(output)) {
	  nout << "Unable to write to " << icon_filename << "\n";
	  exit(1);
	}
	output.write((const char *)default_movie_icon, default_movie_icon_len);
      }
    }
    if (sound_icon.empty()) {
      sound_icon = Filename("icons", default_sound_icon_filename);
      Filename icon_filename(archive_dir, sound_icon);
      if (force_regenerate || !icon_filename.exists()) {
	nout << "Generating " << icon_filename << "\n";
	icon_filename.make_dir();
	icon_filename.set_binary();
      
	ofstream output;
	if (!icon_filename.open_write(output)) {
	  nout << "Unable to write to " << icon_filename << "\n";
	  exit(1);
	}
	output.write((const char *)default_sound_icon, default_sound_icon_len);
      }
    }
  }

  finalize_parameters();


  return ProgramBase::post_command_line();
}


////////////////////////////////////////////////////////////////////
//     Function: Indexify::dispatch_caption
//       Access: Protected, Static
//  Description: Dispatch function for the -caption parameter, which
//               takes either one or two numbers separated by a comma,
//               representing the caption font size and the optional
//               pixel spacing of the caption under the image.
////////////////////////////////////////////////////////////////////
bool Indexify::
dispatch_caption(const string &opt, const string &arg, void *) {
  vector_string words;
  tokenize(arg, words, ",");

  int caption_spacing = 0;

  bool okflag = false;
  if (words.size() == 1) {
    okflag =
      string_to_int(words[0], caption_font_size);

  } else if (words.size() == 2) {
    okflag =
      string_to_int(words[0], caption_font_size) &&
      string_to_int(words[1], caption_spacing);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires one or two integers separated by a comma.\n";
    return false;
  }

  thumb_caption_height = caption_font_size + caption_spacing;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Indexify::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Indexify::
run() {
  bool all_ok = true;

  RollDirs::iterator di;
  for (di = _roll_dirs.begin(); di != _roll_dirs.end(); ++di) {
    RollDirectory *roll_dir = (*di);
    if (!roll_dir->scan(_photo_extension, _movie_extension, _sound_extension)) {
      nout << "Unable to read " << *roll_dir << "\n";
      all_ok = false;
    }
    roll_dir->collect_index_images();
  }

  if (!all_ok) {
    exit(1);
  }

  if (_copy_reduced) {
    do_copy_reduced();
  } else {
    do_generate_images();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Indexify::do_generate_images
//       Access: Public
//  Description: The main work function.  Generates all of the reduced
//               images and the html code.
////////////////////////////////////////////////////////////////////
void Indexify::
do_generate_images() {
  // First, generate all the images.
  RollDirs::iterator di;
  for (di = _roll_dirs.begin(); di != _roll_dirs.end(); ++di) {
    RollDirectory *roll_dir = (*di);
    if (!roll_dir->generate_images(archive_dir, _text_maker)) {
      nout << "Failure.\n";
      exit(1);
    }
  }

  // Then go back and generate the HTML.
  for (di = _roll_dirs.begin(); di != _roll_dirs.end(); ++di) {
    RollDirectory *roll_dir = (*di);
    if (!roll_dir->generate_html(archive_dir, _roll_dir_root)) {
      nout << "Failure.\n";
      exit(1);
    }
  }

  if (!omit_complete) {
    // Generate the complete index that browses all the roll directories
    // at once.
    Filename complete_filename(archive_dir, "html/complete.htm");
    nout << "Generating " << complete_filename << "\n";
    complete_filename.set_text();
    ofstream complete_html;
    if (!complete_filename.open_write(complete_html)) {
      nout << "Unable to write to " << complete_filename << "\n";
      exit(1);
    }

    complete_html
      << "<html>\n"
      << "<head>\n"
      << "<title>" << _front_title << "</title>\n"
      << "</head>\n"
      << "<body>\n"
      << "<h1>" << _front_title << "</h1>\n";
    
    for (di = _roll_dirs.begin(); di != _roll_dirs.end(); ++di) {
      RollDirectory *roll_dir = (*di);
      complete_html
        << roll_dir->get_comment_html()
        << roll_dir->get_index_html();
    }
    
    complete_html << "<p>\n";
    if (!up_icon.empty()) {
      // Use an icon to go up.
      Filename up_icon_href = compose_href("..", up_icon);
      complete_html 
        << "<a href=\"../index.htm\"><img src=\"" << up_icon_href
        << "\" alt=\"return to index\"></a>\n";
    } else {
      // No up icon; use text to go up.
      complete_html
        << "<br><a href=\"../index.htm\">Return to index</a>\n";
    }
    complete_html << "</p>\n";
    
    complete_html
      << "</body>\n"
      << "</html>\n";
  }
    
  // And finally, generate the index HTML file that sits on the top of
  // all of this.
  Filename index_filename(archive_dir, "index.htm");
  nout << "Generating " << index_filename << "\n";
  index_filename.set_text();
  ofstream index_html;
  if (!index_filename.open_write(index_html)) {
    nout << "Unable to write to " << index_filename << "\n";
    exit(1);
  }

  index_html
    << "<html>\n"
    << "<head>\n"
    << "<title>" << _front_title << "</title>\n"
    << "</head>\n"
    << "<body>\n"
    << "<h1>" << _front_title << "</h1>\n"
    << "<ul>\n";

  for (di = _roll_dirs.begin(); di != _roll_dirs.end(); ++di) {
    RollDirectory *roll_dir = (*di);
    if (!roll_dir->is_empty()) {
      index_html
	<< "<a name=\"" << roll_dir->get_basename() << "\">\n"
	<< "<li><a href=\"html/" << roll_dir->get_basename() << ".htm\">"
	<< roll_dir->get_name() << " " << escape_html(roll_dir->get_desc())
	<< "</a></li>\n";
    }
  }

  index_html
    << "</ul>\n";

  if (!omit_complete) {
    index_html
      << "<a href=\"html/complete.htm\">(complete archive)</a>\n";
  }

  index_html
    << "</body>\n"
    << "</html>\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Indexify::do_copy_reduced
//       Access: Public
//  Description: The main work function in -copyreduced mode.  Simply
//               copies key files from the full directory to the
//               reduced directory.
////////////////////////////////////////////////////////////////////
void Indexify::
do_copy_reduced() {
  RollDirs::iterator di;
  for (di = _roll_dirs.begin(); di != _roll_dirs.end(); ++di) {
    RollDirectory *roll_dir = (*di);
    if (!roll_dir->copy_reduced(archive_dir)) {
      nout << "Failure.\n";
      exit(1);
    }
  }
}


int main(int argc, char *argv[]) {
  Indexify prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
