// Filename: indexImage.cxx
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

#include "indexImage.h"
#include "rollDirectory.h"
#include "photo.h"
#include "pnmTextMaker.h"
#include "indexParameters.h"
#include "pnmImage.h"
#include "pnmReader.h"

#include "string_utils.h"

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
IndexImage::
IndexImage(RollDirectory *dir, int index) :
  _dir(dir),
  _index(index)
{
  _name = _dir->get_basename() + "_" + format_string(index + 1);
  _index_x_size = 0;
  _index_y_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
IndexImage::
~IndexImage() {
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::add_photo
//       Access: Public
//  Description: Adds the nth photo in the roll to the index image.
//               Returns true if the photo is added, or false if there
//               was no more room.
////////////////////////////////////////////////////////////////////
bool IndexImage::
add_photo(int photo_index) {
  int count = _photos.size();
  if (count >= max_thumbs) {
    return false;
  }

  int yi = count / thumb_count_x;
  int xi = count - (yi * thumb_count_x);

  int x = thumb_x_space + xi * (thumb_width + thumb_x_space);
  int y = thumb_y_space + yi * (thumb_width + thumb_caption_height + thumb_x_space);

  PhotoInfo pinfo;
  pinfo._photo_index = photo_index;
  pinfo._x_place = x;
  pinfo._y_place = y;
  _photos.push_back(pinfo);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::generate_images
//       Access: Public
//  Description: Computes the thumbnail image file and writes it to
//               the indicated directory; generates all reduced images
//               associated with these photos into the appropriate
//               subdirectory.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool IndexImage::
generate_images(const Filename &archive_dir, PNMTextMaker *text_maker) {
  int count = _photos.size();
  int num_rows = (count + thumb_count_x - 1) / thumb_count_x;
  int actual_index_height = thumb_y_space + num_rows * (thumb_height + thumb_caption_height + thumb_y_space);

  PNMImage index_image;

  Filename reduced_dir(archive_dir, "reduced/" + _dir->get_basename());
  Filename thumbnail_dir(archive_dir, "thumbs");
  Filename output_filename(thumbnail_dir, _name);
  output_filename.set_extension("jpg");

  Photos::const_iterator pi;

  // First, scan the image files to see if we can avoid regenerating
  // the index image.
  bool generate_index_image = true;
  if (!dummy_mode && !force_regenerate && output_filename.exists()) {
    // Maybe we don't need to renegerate the index.
    generate_index_image = false;

    const Filename &newest_contributing_filename = 
      _dir->get_newest_contributing_filename();
    if (!newest_contributing_filename.empty()) {
      generate_index_image = 
        (output_filename.compare_timestamps(newest_contributing_filename) < 0);
    }

    for (pi = _photos.begin(); 
         pi != _photos.end() && !generate_index_image; 
         ++pi) {
      const PhotoInfo &pinfo = (*pi);
      Photo *photo = _dir->get_photo(pinfo._photo_index);
      Filename photo_filename(_dir->get_dir(), photo->get_basename());

      // If any of the source photos are newer than the index image,
      // we must regenerate it.
      generate_index_image = 
        (output_filename.compare_timestamps(photo_filename) < 0);
    }
  }

  // If we don't need to regenerate the index, we do need to at least
  // scan the header.
  if (!generate_index_image) {
    if (index_image.read_header(output_filename)) {
      if (index_image.get_x_size() != actual_index_width ||
          index_image.get_y_size() != actual_index_height) {
        // If the index exists but is the wrong size, we'd better
        // regenerate it.
        generate_index_image = true;
      }

    } else {
      // If we couldn't even read the header, we'd better regenerate it
      // after all.
      nout << "Couldn't read " << output_filename << "; regenerating.\n";
      generate_index_image = true;
    }
  }

  if (generate_index_image) {
    index_image.clear(actual_index_width, actual_index_height);
    index_image.fill(1.0, 1.0, 1.0);
  }

  // Now go back through and read whichever images are necessary, or
  // just read the headers if we can get away with it.
  for (pi = _photos.begin(); pi != _photos.end(); ++pi) {
    const PhotoInfo &pinfo = (*pi);
    Photo *photo = _dir->get_photo(pinfo._photo_index);
    Filename photo_filename(_dir->get_dir(), photo->get_basename());
    Filename reduced_filename(reduced_dir, photo->get_basename());
    photo_filename.standardize();
    reduced_filename.standardize();
    PNMImage reduced_image;

    if (!dummy_mode && photo_filename != reduced_filename &&
        (force_regenerate || 
         reduced_filename.compare_timestamps(photo_filename) < 0)) {
      // If the reduced filename does not exist or is older than the
      // source filename, we must read the complete source filename to
      // generate the reduced image.
      PNMImage photo_image;
      PNMReader *reader = photo_image.make_reader(photo_filename);
      if (reader == (PNMReader *)NULL) {        
        nout << "Unable to read " << photo_filename << ".\n";
        return false;
      }
      photo_image.copy_header_from(*reader);
      
      photo->_full_x_size = photo_image.get_x_size();
      photo->_full_y_size = photo_image.get_y_size();
      
      // Generate a reduced image for the photo.
      compute_reduction(photo_image, reduced_image, 
                        reduced_width, reduced_height);

      photo->_reduced_x_size = reduced_image.get_x_size();
      photo->_reduced_y_size = reduced_image.get_y_size();

      // Only bother making a reduced version if it would actually be
      // smaller than the original.
      if (photo->_reduced_x_size < photo->_full_x_size ||
          photo->_reduced_y_size < photo->_full_y_size) {
	nout << "Reading " << photo_filename << "\n";
        if (!photo_image.read(reader)) {
          nout << "Unable to read.\n";
          return false;
        }
	reader = NULL;

        reduced_image.quick_filter_from(photo_image);
        reduced_filename.make_dir();
        nout << "Writing " << reduced_filename << "\n";
        if (!reduced_image.write(reduced_filename)) {
          nout << "Unable to write.\n";
          delete reader;
          return false;
        }
	photo->_has_reduced = true;

      } else {
	// We're not making a reduced version.  But maybe we still
	// need to read the original so we can make a thumbnail.
	reduced_filename = photo_filename;
	reduced_image.copy_header_from(photo_image);

	if (!dummy_mode && generate_index_image) {
	  nout << "Reading " << photo_filename << "\n";
	  if (!reduced_image.read(reader)) {
	    nout << "Unable to read image.\n";
	    return false;
	  }
	  reader = NULL;
	}  
      }
      if (reader != (PNMReader *)NULL) {
	delete reader;
      }

    } else {
      // If the reduced image already exists and is newer than the
      // source image, use it.

      // We still read the image header to determine its size.
      PNMImageHeader photo_image;
      if (!photo_image.read_header(photo_filename)) {
        nout << "Unable to read " << photo_filename << "\n";
        return false;
      }
      
      photo->_full_x_size = photo_image.get_x_size();
      photo->_full_y_size = photo_image.get_y_size();
      photo->_has_reduced = true;

      if (dummy_mode) {
        // In dummy mode, we may or may not actually have a reduced
        // image.  In either case, ignore the file and compute its
        // appropriate size from the source image.
        compute_reduction(photo_image, reduced_image, reduced_width, reduced_height);
        photo->_reduced_x_size = reduced_image.get_x_size();
        photo->_reduced_y_size = reduced_image.get_y_size();

      } else if (generate_index_image) {
        // Now read the reduced image from disk, so we can put it on
        // the index image.
        nout << "Reading " << reduced_filename << "\n";
        
        if (!reduced_image.read(reduced_filename)) {
          nout << "Unable to read.\n";
          return false;
        }

        photo->_reduced_x_size = reduced_image.get_x_size();
        photo->_reduced_y_size = reduced_image.get_y_size();

      } else {
        // If we're not generating an index image, we don't even need
        // the reduced image--just scan its header to get its size.
        if (!reduced_image.read_header(reduced_filename)) {
          nout << "Unable to read " << reduced_filename << "\n";
          return false;
        }
        photo->_reduced_x_size = reduced_image.get_x_size();
        photo->_reduced_y_size = reduced_image.get_y_size();
      }
    }

    if (generate_index_image) {
      // Generate a thumbnail image for the photo.
      PNMImage thumbnail_image;
      compute_reduction(reduced_image, thumbnail_image, 
                        thumb_interior_width, thumb_interior_height);

      if (dummy_mode) {
        draw_box(thumbnail_image);
      } else {
        thumbnail_image.quick_filter_from(reduced_image);
      }
      // Center the thumbnail image within its box.
      int x_center = (thumb_width - thumbnail_image.get_x_size()) / 2;
      int y_center = (thumb_height - thumbnail_image.get_y_size()) / 2;

      if (draw_frames) {
        draw_frame(index_image, 
                   pinfo._x_place, pinfo._y_place,
                   thumb_width, thumb_height,
                   pinfo._x_place + x_center, pinfo._y_place + y_center,
                   thumbnail_image.get_x_size(), thumbnail_image.get_y_size());
      }

      thumbnail_image.set_color_type(index_image.get_color_type());
      index_image.copy_sub_image(thumbnail_image, 
                                 pinfo._x_place + x_center, 
                                 pinfo._y_place + y_center);
      
      if (text_maker != (PNMTextMaker *)NULL) {
        text_maker->generate_into(photo->get_frame_number(), index_image, 
                                  pinfo._x_place + thumb_width / 2, 
                                  pinfo._y_place + thumb_height + thumb_caption_height);
      }
    }
  }

  if (generate_index_image) {
    nout << "Writing " << output_filename << "\n";
    output_filename.make_dir();
    if (!index_image.write(output_filename)) {
      nout << "Unable to write.\n";
      return false;
    }
  }

  _index_x_size = index_image.get_x_size();
  _index_y_size = index_image.get_y_size();
  _index_basename = output_filename.get_basename();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::generate_html
//       Access: Public
//  Description: Generates all HTML files associated with reduced
//               images into the appropriate subdirectory, and
//               generates root-level HTML code to access the
//               thumbnails on the index image into the indicated
//               stream.
////////////////////////////////////////////////////////////////////
bool IndexImage::
generate_html(ostream &root_html, const Filename &archive_dir,
              const Filename &roll_dir_root) {
  root_html
    << "<a name=\"" << _name << "\">\n"
    << "<img src=\"../thumbs/" << _index_basename
    << "\" width=" << _index_x_size << " height=" << _index_y_size
    << " ismap usemap=\"#" << _name << "\">\n"
    << "<map name=\"" << _name << "\"><br>\n";

  Filename html_dir(archive_dir, "html");

  Photos::const_iterator pi;
  for (pi = _photos.begin(); pi != _photos.end(); ++pi) {
    const PhotoInfo &pinfo = (*pi);
    int photo_index = pinfo._photo_index;
    Photo *photo = _dir->get_photo(pinfo._photo_index);
    Filename html_relname(_dir->get_basename(), photo->get_basename());
    html_relname.set_extension("htm");

    Filename html_filename(html_dir, html_relname);
    html_filename.make_dir();
    html_filename.set_text();
    ofstream reduced_html;
    if (!html_filename.open_write(reduced_html)) {
      nout << "Unable to write to " << html_filename << "\n";
      return false;
    }
    if (!generate_reduced_html(reduced_html, photo, photo_index,
                               pi - _photos.begin(), roll_dir_root)) {
      return false;
    }

    root_html
      << "  <area shape=\"RECT\" href=\"" << html_relname 
      << "\" coords=\""
      << pinfo._x_place << "," << pinfo._y_place << ","
      << pinfo._x_place + thumb_width << "," 
      << pinfo._y_place + thumb_height + thumb_caption_height
      << "\">\n";
  }

  root_html
    << "</map>\n";

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void IndexImage::
output(ostream &out) const {
  out << _name;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void IndexImage::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _name << "\n";
  Photos::const_iterator pi;
  for (pi = _photos.begin(); pi != _photos.end(); ++pi) {
    const PhotoInfo &pinfo = (*pi);
    indent(out, indent_level + 2)
      << *_dir->get_photo(pinfo._photo_index) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::generate_reduced_html
//       Access: Public
//  Description: Generates HTML code for rendering the indicated
//               reduced-image photo into the given ostream.
////////////////////////////////////////////////////////////////////
bool IndexImage::
generate_reduced_html(ostream &html, Photo *photo, int photo_index, int pi,
                      const Filename &roll_dir_root) {
  Filename full_dir;
  if (roll_dir_root.empty()) {
    full_dir = Filename("../..", _dir->get_dir());
  } else {
    compose_href("../..", roll_dir_root, _dir->get_basename());
  }
  Filename full(full_dir, photo->get_basename());

  Filename reduced_dir("../../reduced", _dir->get_basename());
  Filename reduced(reduced_dir, photo->get_basename());
  if (!photo->_has_reduced) {
    reduced_dir = full_dir;
    reduced = full;
  }

  string up_href = "../" + _dir->get_basename() + ".htm#" + _name;

  Filename prev_photo_filename;
  Filename next_photo_filename;

  if (pi != 0) {
    // If this is not the first photo on the index, "back" will take
    // us to the previous photo on the same index.
    Photo *prev_photo = _dir->get_photo(_photos[pi - 1]._photo_index);
    prev_photo_filename = Filename(prev_photo->get_basename());
    prev_photo_filename.set_extension("htm");

  } else {
    // If this is the first photo on the index, "back" will take us to
    // the last photo on the previous index.  Or even to the last
    // photo on the previous roll.
    RollDirectory *roll_dir = _dir;
    int index = _index - 1;
    while (index < 0 && roll_dir->_prev != (RollDirectory *)NULL) {
      roll_dir = roll_dir->_prev;
      index = roll_dir->get_num_index_images() - 1;
    }

    if (index >= 0) {
      IndexImage *prev_index = roll_dir->get_index_image(index);
      nassertr(!prev_index->_photos.empty(), false);
      Photo *prev_photo = 
        roll_dir->get_photo(prev_index->_photos.back()._photo_index);
      Filename other_dir("..", roll_dir->get_basename());
      prev_photo_filename = Filename(other_dir, prev_photo->get_basename());
      prev_photo_filename.set_extension("htm");
    } 
  }

  if (pi + 1 < (int)_photos.size()) {
    // If this is not the last photo, "next" will take us to the next
    // photo on the same index.
    Photo *next_photo = _dir->get_photo(_photos[pi + 1]._photo_index);
    next_photo_filename = Filename(next_photo->get_basename());
    next_photo_filename.set_extension("htm");

  } else {
    // If this is the last photo on the index, "next" will take us to
    // the first photo on the next index.  Or even to the first photo
    // on the next roll.

    RollDirectory *roll_dir = _dir;
    int index = _index + 1;
    while (index >= roll_dir->get_num_index_images() && 
           roll_dir->_next != (RollDirectory *)NULL) {
      roll_dir = roll_dir->_next;
      index = 0;
    }

    if (index < roll_dir->get_num_index_images()) {
      IndexImage *next_index = roll_dir->get_index_image(index);
      nassertr(!next_index->_photos.empty(), false);
      Photo *next_photo = 
        roll_dir->get_photo(next_index->_photos.front()._photo_index);
      Filename other_dir("..", roll_dir->get_basename());
      next_photo_filename = Filename(other_dir, next_photo->get_basename());
      next_photo_filename.set_extension("htm");
    }
  }

  html 
    << "<html>\n"
    << "<head>\n"
    << "<title>" << photo->get_name() << " (" << photo_index + 1 << " of "
    << _dir->get_num_photos() << " on " << _dir->get_basename() 
    << ")</title>\n"
    << "</head>\n"
    << "<body>\n"
    << "<h1>" << photo->get_name() << " (" << photo_index + 1 << " of "
    << _dir->get_num_photos() << ")</h1>";

  generate_nav_buttons(html, prev_photo_filename, next_photo_filename,
                       up_href);

  Filename cm_filename(_dir->get_dir(), photo->get_basename());
  cm_filename.set_extension("cm");
  if (cm_filename.exists()) {
    // If a comment file for the photo exists, insert its contents
    // here, right above the photo.
    if (!RollDirectory::insert_html_comment(html, cm_filename)) {
      return false;
    }
  }

  html
    << "<p><a href=\"" << reduced << "\">"
    << "<img src=\"" << reduced << "\" width=" << photo->_reduced_x_size
    << " height=" << photo->_reduced_y_size << " alt=\"" << photo->get_name()
    << "\"></a></p>\n";

  if (!omit_full_links && photo->_has_reduced) {
    html
      << "<p><a href=\"" << full << "\">View full size image ("
      << photo->_full_x_size << " x " << photo->_full_y_size << ")</a></p>";
  }

  generate_nav_buttons(html, prev_photo_filename, next_photo_filename,
                       up_href);

  html
    << "</body>\n"
    << "</html>\n";

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::generate_nav_buttons
//       Access: Private
//  Description: Outputs the HTML code to generate the next, prev,
//               up buttons when viewing each reduced image.
////////////////////////////////////////////////////////////////////
void IndexImage::
generate_nav_buttons(ostream &html, const Filename &prev_photo_filename,
                     const Filename &next_photo_filename, 
                     const string &up_href) {
  html << "<p>\n";

  bool first_icons = false;
  if (!prev_icon.empty() && !next_icon.empty()) {
    first_icons = true;
    // Use icons to go forward and back.
    Filename prev_icon_href = compose_href("../..", prev_icon);
    if (prev_photo_filename.empty()) {
      html << "<img src=\"" << prev_icon_href << "\" alt=\"No previous photo\">\n";
    } else {
      html << "<a href=\"" << prev_photo_filename
           << "\"><img src=\"" << prev_icon_href << "\" alt=\"previous\"></a>\n";
    }

    Filename next_icon_href = compose_href("../..", next_icon);
    if (next_photo_filename.empty()) {
      html << "<img src=\"" << next_icon_href << "\" alt=\"No next photo\">\n";
    } else {
      html << "<a href=\"" << next_photo_filename
           << "\"><img src=\"" << next_icon_href << "\" alt=\"next\"></a>\n";
    }

  } else {
    // No prev/next icons; use text to go forward and back.
    if (prev_photo_filename.empty()) {
      html << "(This is the first photo.)\n";
    } else {
      html << "<a href=\"" << prev_photo_filename
           << "\">Back to previous photo</a>\n";
    }
    
    if (next_photo_filename.empty()) {
      html << "<br>(This is the last photo.)\n";
    } else {
      html << "<br><a href=\"" << next_photo_filename
           << "\">On to next photo</a>\n";
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
      Filename up_icon_href = compose_href("../..", up_icon);
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
//     Function: IndexImage::compute_reduction
//       Access: Private, Static
//  Description: Computes the amount by which the source image must be
//               scaled to fit within the indicated box, and clears
//               the dest_image to the computed size.
////////////////////////////////////////////////////////////////////
void IndexImage::
compute_reduction(const PNMImageHeader &source_image, PNMImage &dest_image,
                  int x_size, int y_size) {
  double x_scale = (double)x_size / (double)source_image.get_x_size();
  double y_scale = (double)y_size / (double)source_image.get_y_size();
  double scale = min(x_scale, y_scale);

  // Don't ever enlarge an image to fit the rectangle; if the image is
  // smaller than the rectangle, just leave it small.
  scale = min(scale, 1.0);

  int new_x_size = (int)(source_image.get_x_size() * scale + 0.5);
  int new_y_size = (int)(source_image.get_y_size() * scale + 0.5);

  dest_image.clear(new_x_size, new_y_size,
                   source_image.get_num_channels(),
                   source_image.get_maxval());
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::draw_box
//       Access: Private, Static
//  Description: Called in dummy mode to draw a little box in black
//               around the border of the empty thumbnail image.
////////////////////////////////////////////////////////////////////
void IndexImage::
draw_box(PNMImage &image) {
  // First, fill it in white.
  image.fill(1, 1, 1);

  if (!draw_frames) {
    // Now make the border pixel black.  We only need to do this if we
    // aren't drawing frames, since the frames will reveal the shape
    // of the image too.
    int x_size = image.get_x_size();
    int y_size = image.get_y_size();
    for (int xi = 0; xi < x_size; xi++) {
      image.set_xel(xi, 0, 0, 0, 0);
      image.set_xel(xi, y_size - 1, 0, 0, 0);
    }
    
    for (int yi = 1; yi < y_size - 1; yi++) {
      image.set_xel(0, yi, 0, 0, 0);
      image.set_xel(x_size - 1, yi, 0, 0, 0);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IndexImage::draw_frame
//       Access: Private, Static
//  Description: Called in draw_frames mode to draw a slide mount in
//               gray on the index image before drawing the thumbnail
//               image in the center.
////////////////////////////////////////////////////////////////////
void IndexImage::
draw_frame(PNMImage &image,
           int frame_left, int frame_top, int frame_width, int frame_height,
           int hole_left, int hole_top, int hole_width, int hole_height) {
  // Gray levels.
  static const RGBColord mid(0.5, 0.5, 0.5);
  static const RGBColord light(0.7, 0.7, 0.7);
  static const RGBColord lighter(0.9, 0.9, 0.9);
  static const RGBColord dark(0.3, 0.3, 0.3);
  static const RGBColord darker(0.1, 0.1, 0.1);

  // First, fill in the whole rectangle in gray.
  int xi, yi;
  for (yi = 0; yi < frame_height; yi++) {
    for (xi = 0; xi < frame_width; xi++) {
      image.set_xel(xi + frame_left, yi + frame_top, mid);
    }
  }

  // Now draw the bevel.
  for (xi = 0; xi < frame_outer_bevel; xi++) {
    for (yi = xi; yi < frame_height - xi; yi++) { 
      // Left edge.
      image.set_xel(xi + frame_left, yi + frame_top, light);
      // Right edge.
      image.set_xel(frame_width - 1 - xi + frame_left, yi + frame_top, dark);
    }
  }
  for (yi = 0; yi < frame_outer_bevel; yi++) {
    for (xi = yi; xi < frame_width - yi; xi++) { 
      // Top edge.
      image.set_xel(xi + frame_left, yi + frame_top, lighter);
      // Bottom edge.
      image.set_xel(xi + frame_left, frame_height - 1 - yi + frame_top, darker);
    }
  }

  // Interior bevel.
  for (xi = -1; xi >= -frame_inner_bevel; xi--) {
    for (yi = xi; yi < hole_height - xi; yi++) { 
      // Left edge.
      image.set_xel(xi + hole_left, yi + hole_top, dark);
      // Right edge.
      image.set_xel(hole_width - 1 - xi + hole_left, yi + hole_top, light);
    }
  }
  for (yi = -1; yi >= -frame_inner_bevel; yi--) {
    for (xi = yi; xi < hole_width - yi; xi++) { 
      // Top edge.
      image.set_xel(xi + hole_left, yi + hole_top, darker);
      // Bottom edge.
      image.set_xel(xi + hole_left, hole_height - 1 - yi + hole_top, lighter);
    }
  }

  // We don't have to cut out the hole, since the thumbnail image will
  // do that when it is placed.
}
