// Filename: indexParameters.h
// Created by:  drose (04Apr02)
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

#ifndef INDEXPARAMETERS_H
#define INDEXPARAMETERS_H

#include "pandatoolbase.h"

#include "filename.h"
#include "dSearchPath.h"

// Some of these constants may be modified by command-line parameters
// from the user.

// The maximum size of the index image.  It will shrink vertically to
// fit the images it contains, and it will shrink horizontally to fit
// a complete row of images (even if it does not contain a complete
// row).  It will never be larger than this.
extern int max_index_width;
extern int max_index_height;

// The size of the individual thumbnail images, including the frames
// (if present).  Thumbnail images are scaled to fit within this box.
extern int thumb_width;
extern int thumb_height;

// The total number of pixels reserved for the caption under each
// thumbnail image.  This is the caption_font_size plus whatever
// spacing should be included between the caption and the image.
extern int thumb_caption_height;

// The size in pixels of the caption font.  This depends on the point
// size of the font as reported by FreeType, so the actual height of
// the letters might be slightly lower or higher than this, depending
// on the font.
extern int caption_font_size;

// The amount of space, in pixels, between each two neighboring
// thumbnail images, and around the overall index image.
extern int thumb_x_space;
extern int thumb_y_space;

// The ratio by which the thumbnail images are reduced further when
// frames are drawn, to allow room for a frame that resembles a slide
// mount.
extern double frame_reduction_factor;

// The number of pixels of thickness to draw for the frames' outer
// bevels and inner bevels, respectively.
extern int frame_outer_bevel;
extern int frame_inner_bevel;

// The size of the reduced images on the individual image pages.  The
// source image will be scaled to fit within this rectangle.
extern int reduced_width;
extern int reduced_height;

// The directory at the root of the output hierarchy (or as specified
// by -a on the command line).
extern Filename archive_dir;

// The filenames (or URLS) to the icon images for navigating the
// individual image pages.
extern Filename prev_icon;
extern Filename next_icon;
extern Filename up_icon;
extern Filename movie_icon;

const PNMImage &get_movie_icon();

// True to regenerate every image, whether it appears to need it or
// not.
extern bool force_regenerate;

// True to use the Rose formatting convention for roll directory names.
extern bool format_rose;

// True to sort roll directory names by date.  Useful only with -r.
extern bool sort_date;

// True to place dummy thumbnails instead of loading actual images.
extern bool dummy_mode;

// True to draw frames (slide mounts) around each thumbnail image.
extern bool draw_frames;

// True to avoid introducing each roll directory on the index page
// with its own little header.  This also ignored the roll.cm file if
// it exists.
extern bool omit_roll_headers;
extern DSearchPath cm_search;

// True to omit links to the full-size source images.
extern bool omit_full_links;

// True to caption photos with just a frame number instead of the
// whole image basename.  This only works if the photo image filenames
// consist of the roll directory name concatenated with a frame number.
extern bool caption_frame_numbers;


void finalize_parameters();



// The following parameters are all computed based on the above.

// The number of thumbnail images that fit across an index image,
// horizontally and vertically.
extern int thumb_count_x;
extern int thumb_count_y;

// The total number of thumbnail images within each index image.
extern int max_thumbs;

// The number of pixels wide each index image will actually be, based
// on thumb_count_x.
extern int actual_index_width;

// The actual size of the rectangle each thumbnail image must be
// scaled into, accounting for the presence of a frame.
extern int thumb_interior_width;
extern int thumb_interior_height;

Filename compose_href(const Filename &rel_dir, const Filename &user_prefix,
		      const Filename &basename = Filename());

string escape_html(const string &input);

#endif


